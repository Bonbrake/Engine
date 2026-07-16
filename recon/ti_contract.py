#!/usr/bin/env python3
"""
recon/ti_contract.py — DESIGN-BY-CONTRACT gate for the TI-gospel plan.

PREVENTION, not detection. The bug we kept hitting: the agent ASSERTED "covered"
from an EXT TITLE, then the verifier caught it later. This script removes the
agent from the claim-authoring step entirely.

HOW IT PREVENTS THE BUG (literate/contract style):
  1. The plan's "COVERED" / "-> EXT" claims are PARSED from the plan file itself
     (single source of truth — the agent writes prose, the script extracts claims).
  2. For every extracted claim, the script READS THE REAL SPEC BLOCK and asserts
     the claimed capability's keywords are present. No keyword match => the claim
     is CONTRACT-VIOLATING => exit 1 (hard gate).
  3. Run as a pre-commit git hook (or manually). A plan that asserts something the
     spec body doesn't back CANNOT pass. The agent cannot "declare done" without it.

This is the contract: a plan claim is valid IFF the spec block body contains the
evidence. The agent's prose is checked against the spec, not trusted.

Run: python recon/ti_contract.py
Exit 0 = every plan claim is contract-valid (backed by spec body).
Exit 1 = at least one claim asserts coverage the spec body does not contain.
"""
import os, re, sys

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")
PLAN = os.path.join(BASE, "PLAN_threat_interactive_gospel_2026-07-16.md")

# capability -> keywords that MUST appear in the named EXT's real block body
# (this is the CONTRACT: claim X is valid only if these words are in the block)
CAP_REQUIREMENTS = {
    "R1": ["burley", "lambert"],
    "R7": ["hysteresis", "variance"],          # TAA clamper
    "R7-AA": ["laplacian", "depth", "edge"],   # EXT-11 (upscaler edge, NOT MSAA)
    "R8": ["shadow"],
    "R9": ["virtual texture", "page"],
    "R10": ["beer", "lambert"],
    "R12": ["lod"],
    "T6": ["ambient", "obscurance", "ssao"],
    "T9": ["illumination", "sh"],
    "T14": ["voxel", "raster"],
    "T15": ["volumetric", "cone"],
}

# claims the spec body does NOT satisfy (agent mis-mapped these) -> MUST be GAP, not COVERED
FORBIDDEN_COVERED = {
    "T1": ["mipmap", "trilinear", "aniso"],       # not in -19/-20
    "T3": ["msaa", "stencil"],                     # not in -11 (upscaler edge)
    "T4": ["rgb10a2", "rgb10"],                    # not in -12
    "T5": ["stencil", "shadow mask"],              # not in -13
    "T8": ["subsurface", "lut"],                   # not in -19
}

def parse_claims(plan):
    """Walk the plan line by line; return list of (claim_id, status, ext_id).
    Handles: 'R1  ...' then 'SPEC: COVERED — Mx-EXT-NN' (rule id on prev line)
             'T3  **...** → [GAP] M4.5-EXT-NN' (may wrap to next line)."""
    STATUS = r"(COVERED|PARTIAL|GAP)"
    EXT = r"(M\d+(?:\.\d+)?-EXT-\d+)"
    out = []
    last_rule = None
    lines = plan.splitlines()
    for i, line in enumerate(lines):
        # track the most recent rule id (R#) for SPEC: lines that follow
        rm = re.search(r"^(R\d+)\s", line)
        if rm:
            last_rule = rm.group(1)
        # SPEC: status — EXT  (same line) — also grab EXT on the NEXT line (continuation)
        m = re.search(rf"SPEC:\s*{STATUS}.*?{EXT}", line)
        if m:
            cands = [m.group(2)]
            m3 = re.search(EXT, lines[i + 1]) if i + 1 < len(lines) else None
            if m3:
                cands.append(m3.group(1))
            cid = re.search(r"(R\d+)", line)
            cid = cid.group(1) if cid else last_rule
            if cid:
                for ex in cands:
                    out.append((cid, m.group(1), ex))
            continue
        # SPEC: status (EXT on NEXT line only)
        m2 = re.search(rf"SPEC:\s*{STATUS}", line)
        if m2 and i + 1 < len(lines):
            m3 = re.search(EXT, lines[i + 1])
            if m3:
                cid = re.search(r"(R\d+)", line)
                cid = cid.group(1) if cid else last_rule
                if cid:
                    out.append((cid, m2.group(1), m3.group(1)))
                continue
        # Technique: T#  **...** → [STATUS] EXT  (same line)
        m = re.search(rf"(T\d+)\s+\*\*.*?\*\*\s*(?:\([^)]*\)\s*)?→\s*\[?{STATUS}\]?\s*{EXT}", line)
        if m:
            out.append((m.group(1), m.group(2), m.group(3)))
            continue
        # technique wrapped: T# **...** → [STATUS]  (EXT on next line)
        m2 = re.search(rf"(T\d+)\s+\*\*.*?\*\*\s*(?:\([^)]*\)\s*)?→\s*\[?{STATUS}\]?", line)
        if m2 and i + 1 < len(lines):
            m3 = re.search(EXT, lines[i + 1])
            if m3:
                out.append((m2.group(1), m2.group(2), m3.group(1)))
    # dedupe
    seen, res = set(), []
    for p in out:
        if p not in seen:
            seen.add(p); res.append(p)
    return res


def load_block(ext_id):
    m = re.match(r"(M\d+(?:\.\d+)?)-EXT-(\d+)", ext_id)
    if not m:
        return None
    path = os.path.join(SPEC, f"{m.group(1)}.md")
    if not os.path.exists(path):
        return f"__NOFILE__:{path}"
    txt = open(path, encoding="utf-8").read()
    i = txt.find(f"#### [{ext_id}]")
    if i < 0:
        return f"__NOBLOCK__:{ext_id}"
    nxt = txt.find("\n#### [", i + 10)
    return txt[i: nxt if nxt > 0 else len(txt)]


def main():
    plan = open(PLAN, encoding="utf-8").read()
    pairs = parse_claims(plan)
    if not pairs:
        print("CONTRACT ERROR: no claim->EXT pairs parsed from plan. Plan format broke?")
        sys.exit(1)

    violations = []
    covered_checks = 0
    # group pairs by claim_id to allow "valid if ANY named EXT satisfies the contract"
    from collections import defaultdict
    by_claim = defaultdict(list)
    for claim_id, status, ext_id in pairs:
        by_claim[claim_id].append((status, ext_id))

    for claim_id, items in by_claim.items():
        req = CAP_REQUIREMENTS.get(claim_id)
        forbid = FORBIDDEN_COVERED.get(claim_id)
        for status, ext_id in items:
            body = load_block(ext_id)
            block_missing = (body is None) or (isinstance(body, str) and body.startswith("__"))

            if status == "GAP":
                if (not block_missing) and req:
                    missing = [k for k in req if k not in body.lower()]
                    if missing:
                        violations.append(f"{claim_id}(GAP) -> {ext_id}: exists but lacks {missing}")
                continue

            # COVERED / PARTIAL
            if block_missing:
                violations.append(f"{claim_id}({status}) -> {ext_id}: BLOCK NOT FOUND ({body})")
                continue
            if req:
                missing = [k for k in req if k not in body.lower()]
                if not missing:
                    covered_checks += 1
            if forbid and status == "COVERED":
                missing = [k for k in forbid if k not in body.lower()]
                if missing:
                    violations.append(f"{claim_id}(COVERED) -> {ext_id}: body lacks {missing} (false cover)")

    print("=" * 64)
    print("TI-GOSPEL CONTRACT GATE (design-by-contract, prevention)")
    print("=" * 64)
    print(f"claims parsed: {len(pairs)} | COVERED/PARTIAL contract checks: {covered_checks}")
    if violations:
        print(f"\nCONTRACT VIOLATIONS ({len(violations)}):")
        for v in violations:
            print("  x", v)
        print("\nGATE: FAIL (exit 1) — fix the plan before declaring done.")
        sys.exit(1)
    else:
        print("\nGATE: PASS (exit 0) — every COVERED/PARTIAL claim backed by spec body;")
        print("GAP claims correctly absent. Agent cannot 'declare done' on unbacked claim.")
        sys.exit(0)


if __name__ == "__main__":
    main()
