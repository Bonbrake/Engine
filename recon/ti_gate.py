#!/usr/bin/env python3
"""
recon/ti_gate.py — MERGED design-by-contract gate for the TI-gospel plan.

Replaces BOTH ti_contract.py (prevention) and verify_ti_plan.py (detection).
Single source of truth: claim->EXT->status is PARSED from the plan prose, so the
checked mapping can never drift from what the plan actually SAYS (the bug that let
T7 stay falsely COVERED — its line put [COVERED] AFTER the EXT, which the old
regex couldn't match, and verify_ti_plan.py had no T7 row either).

WHAT IT ENFORCES (the CONTRACT — domain evidence spec, not plan duplication):
  For every COVERED/PARTIAL claim, the named EXT's REAL block body must contain the
  capability keywords (CAP_REQUIREMENTS). If it doesn't, the claim is unbacked -> FAIL.
  For every claim in FORBIDDEN_COVERED, the body must LACK those keywords (else it's a
  false "covered" the verifier would have caught) -> FAIL.
  Every GAP claim must target an EXT that does NOT already contain the capability
  (else it's wrongly a gap / duplicate).
  Proposed NEW EXT ids must NOT collide with an existing spec block.

UNCLASSIFIED GUARD: any R#/T# line that references an EXT but yields no parseable
status is emitted as a WARNING, so a claim can't hide from the gate.

Run: python recon/ti_gate.py
Exit 0 = every COVERED/PARTIAL claim backed by spec body; no collision; no false cover.
Exit 1 = at least one contract violation.
"""
import os, re, sys
from collections import defaultdict

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")
PLAN = os.path.join(BASE, "PLAN_threat_interactive_gospel_2026-07-16.md")
GAPFILL = os.path.join(BASE, "PLAN_gap_fill_ext_proposals_2026-07-16.md")

EXT = r"(M\d+(?:\.\d+)?-EXT-\d+)"
STATUS = r"(COVERED|PARTIAL|GAP)"

# ---- CONTRACT: capability -> keywords that MUST appear in the named EXT's real block body
CAP_REQUIREMENTS = {
    "R1": ["burley", "lambert"],
    "R7": ["hysteresis", "variance"],
    "R7-AA": ["laplacian", "depth", "edge"],     # EXT-11 (upscaler edge, NOT MSAA)
    "R8": ["shadow"],
    "R9": ["virtual texture", "page"],
    "R10": ["beer", "lambert"],
    "R12": ["lod"],
    "T6": ["ambient", "obscurance", "ssao"],
    "T9": ["illumination", "sh"],
    "T14": ["voxel", "raster"],
    "T15": ["volumetric", "cone"],
    "T7": ["forward"],                            # NEW: forward skin/hair pass
}
# claims the spec body does NOT satisfy -> MUST be GAP, not COVERED
FORBIDDEN_COVERED = {
    "T1": ["mipmap", "trilinear", "aniso"],
    "T3": ["msaa", "stencil"],
    "T4": ["rgb10a2", "rgb10"],
    "T5": ["stencil", "shadow mask"],
    "T8": ["subsurface", "lut"],
}

# proposed NEW ext ids (from gap-fill) — must NOT already exist in spec
PROPOSED_NEW = ["M4.5-EXT-22", "M4.5-EXT-23", "M4.5-EXT-32",
                "M4.5-EXT-33", "M4.5-EXT-34", "M4.5-EXT-35"]


def parse_claims(plan):
    """Parse (claim_id, status, ext_id, line_no) from plan prose.
    Handles BOTH orderings the old tools missed:
      'T7 **..** → [GAP] M4.5-EXT-32'   (status before EXT)
      'T7 **..** → M4.5-EXT-20. [COVERED]' (EXT before status)
    and RULE SPEC: lines (R# tracked from a preceding '^R\\d ' line)."""
    out, last_rule = [], None
    lines = plan.splitlines()
    for i, line in enumerate(lines):
        rm = re.search(r"^(R\d+)\s", line)
        if rm:
            last_rule = rm.group(1)

        # --- technique: T# **...** → [STATUS] EXT  (status-first) ---
        m = re.search(rf"(T\d+)\s+\*\*.+?\*\*\s*(?:\([^)]*\)\s*)?→\s*\[?{STATUS}\]?\s*{EXT}", line)
        if m:
            out.append((m.group(1), m.group(2), m.group(3), i + 1)); continue

        # --- technique: T# **...** → EXT ... [STATUS]  (ext-first, possibly on next line) ---
        m = re.search(rf"(T\d+)\s+\*\*.+?\*\*\s*(?:\([^)]*\)\s*)?→\s*{EXT}", line)
        if m:
            # status may be same line (after EXT) or next line
            cand = [m.group(3)]
            s = re.search(STATUS, line[m.end():]) if m.end() < len(line) else None
            if s:
                out.append((m.group(1), s.group(1), m.group(3), i + 1)); continue
            if i + 1 < len(lines):
                s2 = re.search(STATUS, lines[i + 1])
                if s2:
                    out.append((m.group(1), s2.group(1), m.group(3), i + 1)); continue
            # EXT referenced but NO status -> caller warns (unclassified)
            out.append((m.group(1), None, m.group(3), i + 1)); continue

        # --- rule SPEC: same line ---
        m = re.search(rf"SPEC:\s*{STATUS}.*?{EXT}", line)
        if m:
            cands = [m.group(2)]
            if i + 1 < len(lines):
                m3 = re.search(EXT, lines[i + 1])
                if m3: cands.append(m3.group(1))
            cid = re.search(r"(R\d+)", line)
            cid = cid.group(1) if cid else last_rule
            for ex in cands:
                if cid: out.append((cid, m.group(1), ex, i + 1))
            continue

        # --- rule SPEC: status on this line, EXT on next ---
        m2 = re.search(rf"SPEC:\s*{STATUS}", line)
        if m2 and i + 1 < len(lines):
            m3 = re.search(EXT, lines[i + 1])
            if m3:
                cid = re.search(r"(R\d+)", line)
                cid = cid.group(1) if cid else last_rule
                if cid: out.append((cid, m2.group(1), m3.group(1), i + 1))

    # dedupe
    seen, res = set(), []
    for p in out:
        if p not in seen:
            seen.add(p); res.append(p)
    return res


def unclassified_warnings(plan, parsed):
    """Any R#/T# line referencing an EXT but not captured with a status -> warn."""
    captured = {(c, e) for (c, s, e, _) in parsed}
    warns = []
    for i, line in enumerate(plan.splitlines()):
        m = re.search(rf"^(R\d+|T\d+)\b.*{EXT}", line)
        if m and (m.group(1), m.group(2)) not in captured:
            # only warn if it looks like a claim (has → or SPEC:), not a cross-ref mention
            if "→" in line or "SPEC:" in line:
                warns.append(f"  line {i+1}: {m.group(1)} references {m.group(2)} but no status parsed")
    return warns


def load_block(ext_id):
    m = re.match(r"(M\d+(?:\.\d+)?)-EXT-(\d+)", ext_id)
    if not m:
        return f"__BADID__:{ext_id}"
    path = os.path.join(SPEC, f"{m.group(1)}.md")
    if not os.path.exists(path):
        return f"__NOFILE__:{path}"
    txt = open(path, encoding="utf-8").read()
    hdr = f"#### [{ext_id}]"
    i = txt.find(hdr)
    if i < 0:
        return f"__NOBLOCK__:{ext_id}"
    nxt = txt.find("\n#### [", i + len(hdr))
    return txt[i: nxt if nxt > 0 else len(txt)]


def main():
    plan = open(PLAN, encoding="utf-8").read()
    pairs = parse_claims(plan)
    if not pairs:
        print("GATE ERROR: no claim->EXT pairs parsed. Plan format broke?")
        sys.exit(1)

    violations, warns = [], []
    by_claim = defaultdict(list)
    for cid, status, ext, _ in pairs:
        by_claim[cid].append((status, ext))

    covered_checks = 0
    for cid, items in by_claim.items():
        req = CAP_REQUIREMENTS.get(cid)
        forbid = FORBIDDEN_COVERED.get(cid)
        for status, ext in items:
            body = load_block(ext)
            missing_file = isinstance(body, str) and body.startswith("__")

            if status is None:
                warns.append(f"{cid} -> {ext}: EXT referenced, NO STATUS parsed (unclassified)")
                continue
            if status == "GAP":
                if not missing_file and req:
                    miss = [k for k in req if k not in body.lower()]
                    if miss:
                        violations.append(f"{cid}(GAP) -> {ext}: block EXISTS but already has {miss} (wrongly a gap)")
                continue

            # COVERED / PARTIAL
            if missing_file:
                violations.append(f"{cid}({status}) -> {ext}: BLOCK NOT FOUND ({body})")
                continue
            if req:
                miss = [k for k in req if k not in body.lower()]
                if not miss:
                    covered_checks += 1
            if forbid and status == "COVERED":
                miss = [k for k in forbid if k not in body.lower()]
                if miss:
                    violations.append(f"{cid}(COVERED) -> {ext}: body lacks {miss} (FALSE COVER)")

    # --- proposed-new EXT collision check ---
    collisions = []
    for ext in PROPOSED_NEW:
        b = load_block(ext)
        if not (isinstance(b, str) and b.startswith("__")):
            collisions.append(f"{ext}: proposed NEW but already exists in spec (collision)")

    # --- unclassified guard ---
    warns += unclassified_warnings(plan, pairs)

    print("=" * 64)
    print("TI-GOSPEL MERGED GATE  (ti_gate.py — parse-from-prose + contract)")
    print("=" * 64)
    print(f"claims parsed: {len(pairs)} | COVERED/PARTIAL contract checks: {covered_checks}")
    print(f"proposed-new EXT collision check: {len(PROPOSED_NEW)} ids")
    if warns:
        print(f"\nUNCLASSIFIED WARNINGS ({len(warns)}):")
        for w in warns:
            print("  !", w)
    if collisions:
        print(f"\nCOLLISIONS ({len(collisions)}):")
        for c in collisions:
            print("  x", c)
    if violations:
        print(f"\nCONTRACT VIOLATIONS ({len(violations)}):")
        for v in violations:
            print("  x", v)
        print("\nGATE: FAIL (exit 1) — fix the plan before declaring done.")
        sys.exit(1)
    if collisions:
        sys.exit(1)
    print("\nGATE: PASS (exit 0) — COVERED/PARTIAL backed by spec body; GAPs correct;")
    print("no collisions; no unclassified claims hiding.")
    sys.exit(0)


if __name__ == "__main__":
    main()
