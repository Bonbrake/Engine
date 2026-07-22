#!/usr/bin/env python3
"""
recon/ti_debug.py — TI-gospel design-by-contract DEBUG gate (agent-first).

Single source of truth: claim->EXT->status is PARSED from the plan prose, so the
checked mapping can never drift from what the plan actually SAYS.

WHY THIS NAME: always grep-able as `ti_debug`; always TI-referenced; obviously a
debug tool. This is THE tool to run after any TI-plan edit — `python recon/ti_debug.py`.

OUTPUT CONTRACT (default when piped / no TTY = canonical JSON, schema "ti-gate/v1"):
  An LLM agent (Hermes / hy3) consumes the JSON directly — every finding is
  self-contained: stable `id`+`kind`, exact `missing_keywords`, `evidence` (spec block
  snippet + plan line text), `locs` (file:line for spec AND plan), and a `fix_hint`.
  No surrounding context needed to act. Human text is opt-in (--human / TTY).

Run:
  python recon/ti_debug.py                 # JSON (piped) or human (TTY)
  python recon/ti_debug.py --human         # force human view
  python recon/ti_debug.py --jsonl         # NDJSON: one object per finding + summary
  python recon/ti_debug.py --claim R7      # narrow to one claim (debug isolation)
  python recon/ti_debug.py --kind unbacked # narrow to one finding kind
  python recon/ti_debug.py --gha           # GitHub Actions annotations (::error file=::)
  python recon/ti_debug.py --selftest      # self-verify (retires temp verify scripts)
  python recon/ti_debug.py --schema        # print embedded SCHEMA, exit 0
  python recon/ti_debug.py --plan P --spec D --strict

Exit 0 = every COVERED/PARTIAL claim backed by >=1 spec block; no collision; no false
        cover; no bogus GAP target.
Exit 1 = at least one contract violation.
Exit 2 = self-test failed / internal error.
"""
import os, re, sys, json, io, contextlib
from collections import defaultdict

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")
PLAN = os.path.join(BASE, "PLAN_threat_interactive_gospel_2026-07-16.md")

EXT = r"(M\d+(?:\.\d+)?-EXT-\d+)"
STATUS = r"(COVERED|PARTIAL|GAP)"

# ---- CONTRACT: capability -> keyword groups. A claim is BACKED iff ANY ONE of its
# named EXT blocks satisfies ANY group (all keywords in that group present in the block).
CAP_REQUIREMENTS = {
    "R1":  [["burley", "lambert"]],
    "R7":  [["silhouette", "edge", "depth"], ["hysteresis", "variance"]],
    "R8":  [["shadow"]],
    "R9":  [["virtual texture", "page"]],
    "R10": [["beer", "lambert"]],
    "R12": [["lod"]],
    "T6":  [["ambient", "obscurance", "ssao"]],
    "T9":  [["illumination", "sh"]],
    "T14": [["voxel", "raster"]],
    "T15": [["volumetric", "cone"]],
}
# a COVERED claim whose body LACKS these keywords (the capability the claim needs but is
# absent) -> FALSE COVER.
FORBIDDEN_COVERED = {
    "T1": ["mipmap", "trilinear", "aniso"],
    "T3": ["msaa", "stencil"],
    "T4": ["rgb10a2", "rgb10"],
    "T5": ["stencil", "shadow mask"],
    "T8": ["subsurface", "lut"],
}

PROPOSED_NEW = []

SCHEMA = {
    "type": "object",
    "required": ["schema", "tool", "consumer", "exit", "summary", "findings", "claims"],
    "properties": {
        "schema": {"type": "string"},
        "tool": {"type": "string"},
        "consumer": {"type": "array", "items": {"type": "string"}},
        "exit": {"type": "integer"},
        "summary": {"type": "object"},
        "findings": {"type": "array", "items": {"type": "object"}},
        "claims": {"type": "array", "items": {"type": "object"}},
        "schema_valid": {"type": "boolean"},
    },
}

# try jsonschema; degrade gracefully if absent (we still emit the contract)
try:
    import jsonschema
    _HAVE_JSC = True
except Exception:
    _HAVE_JSC = False


# --------------------------------------------------------------------------- parse
def parse_claims(plan):
    """Parse (claim_id, status, ext_id, line_no) from plan prose.
    Handles both '-> [GAP] EXT' and '-> EXT. [COVERED]' orderings, multi-line T#
    technique claims (title on one line, '-> [STATUS] EXT' on the next), and R# SPEC
    lines. Uses the preceding '^R\\d ' line as the claim id (not incidental R-digit
    mentions inside SPEC prose like 'with R3')."""
    out, last_rule = [], None
    raw = plan.splitlines()
    lines = []
    i = 0
    while i < len(raw):
        line = raw[i]
        if re.match(r"^T\d+\s+\*\*", line) and "→" not in line:
            buf = [line]; j = i + 1
            while j < len(raw) and raw[j].strip() \
                    and not re.match(r"^(R\d+|T\d+)\b", raw[j]):
                buf.append(raw[j])
                if "→" in raw[j]:
                    break
                j += 1
            lines.append((" ".join(buf), i + 1)); i = j
        else:
            lines.append((line, i + 1)); i += 1

    for i, (line, lno) in enumerate(lines):
        rm = re.search(r"^(R\d+)\s", line)
        if rm:
            last_rule = rm.group(1)

        m = re.search(rf"(T\d+)\s+\*\*.+?\*\*\s*.*?→\s*\[?{STATUS}\]?\s*{EXT}", line)
        if m:
            out.append((m.group(1), m.group(2), m.group(3), lno)); continue

        m = re.search(rf"(T\d+)\s+\*\*.+?\*\*\s*.*?→\s*{EXT}", line)
        if m:
            ext = m.group(2)
            s = re.search(STATUS, line[m.end():]) if m.end() < len(line) else None
            if s:
                out.append((m.group(1), s.group(1), ext, lno)); continue
            if i + 1 < len(lines):
                s2 = re.search(STATUS, lines[i + 1][0])
                if s2:
                    out.append((m.group(1), s2.group(1), ext, lno)); continue
            out.append((m.group(1), None, ext, lno)); continue

        m = re.search(rf"SPEC:\s*{STATUS}.*?{EXT}", line)
        if m:
            cands = [m.group(2)]
            if i + 1 < len(lines):
                m3 = re.search(EXT, lines[i + 1][0])
                if m3: cands.append(m3.group(1))
            cid = last_rule
            for ex in cands:
                if cid: out.append((cid, m.group(1), ex, lno))
            continue

        m2 = re.search(rf"SPEC:\s*{STATUS}", line)
        if m2 and i + 1 < len(lines):
            m3 = re.search(EXT, lines[i + 1][0])
            if m3 and last_rule:
                out.append((last_rule, m2.group(1), m3.group(1), lno))

    seen, res = set(), []
    for p in out:
        if p not in seen:
            seen.add(p); res.append(p)
    return res


def _satisfies(body, req_groups):
    if not req_groups:
        return False
    if isinstance(req_groups[0], str):
        req_groups = [req_groups]
    low = body.lower()
    return any(all(k in low for k in grp) for grp in req_groups)


_BLOCK_CACHE = {}

def load_block(ext_id):
    """Return (body, block_start_line) or (error_str, None). Cached."""
    if ext_id in _BLOCK_CACHE:
        return _BLOCK_CACHE[ext_id]
    m = re.match(r"(M\d+(?:\.\d+)?)-EXT-(\d+)", ext_id)
    if not m:
        res = (f"__BADID__:{ext_id}", None)
        _BLOCK_CACHE[ext_id] = res
        return res
    path = os.path.join(SPEC, f"{m.group(1)}.md")
    if not os.path.exists(path):
        res = (f"__NOFILE__:{path}", None)
        _BLOCK_CACHE[ext_id] = res
        return res
    txt = open(path, encoding="utf-8").read()
    hdr = f"#### [{ext_id}]"
    i = txt.find(hdr)
    if i < 0:
        res = (f"__NOBLOCK__:{ext_id}", None)
        _BLOCK_CACHE[ext_id] = res
        return res
    start = txt.count("\n", 0, i) + 1
    nxt = txt.find("\n#### [", i + len(hdr))
    res = (txt[i: nxt if nxt > 0 else len(txt)], start)
    _BLOCK_CACHE[ext_id] = res
    return res


# --------------------------------------------------------------------------- core
def run(plan_path=PLAN, spec_dir=SPEC, strict=False, claim=None, kind=None):
    """Run the gate. `claim` (e.g. 'R7') and `kind` (e.g. 'unbacked') narrow the
    emitted findings+claims for debugging a single item."""
    plan = open(plan_path, encoding="utf-8").read()
    pairs = parse_claims(plan)
    findings, claims = [], []
    by_claim = defaultdict(list)
    for cid, status, ext, lno in pairs:
        by_claim[cid].append((status, ext, lno))

    covered_backed = 0
    for cid, items in by_claim.items():
        req = CAP_REQUIREMENTS.get(cid)
        forbid = FORBIDDEN_COVERED.get(cid)
        backed = False
        saw = False
        for status, ext, lno in items:
            body, sline = load_block(ext)
            missing = isinstance(body, str) and body.startswith("__")
            spec_file = os.path.relpath(os.path.join(spec_dir,
                          f"{re.match(EXT, ext).group(1)}.md"), BASE) if re.match(EXT, ext) else "?"
            if status is None:
                findings.append(_f("UNCLASSIFIED", "unclassified", "warning", cid, None, ext,
                    f"EXT referenced, NO STATUS parsed", [],
                    (body[:200] if not missing else body, sline),
                    (plan_path, lno),
                    "Add [GAP]/[COVERED]/[PARTIAL] status to this claim line."))
                continue
            saw = True
            if status == "GAP":
                if missing:
                    if ext not in PROPOSED_NEW:
                        findings.append(_f("WRONG_GAP", "wrong_gap", "error", cid, status, ext,
                            "GAP target does NOT exist AND is not in PROPOSED_NEW (typo/drift)",
                            [], (body, sline), (plan_path, lno),
                            "Fix the EXT id or add it to PROPOSED_NEW before pasting the gap."))
                elif req and _satisfies(body, req):
                    findings.append(_f("WRONG_GAP", "wrong_gap", "error", cid, status, ext,
                        f"block EXISTS and already satisfies {req} (already implemented)",
                        [], (body[:200], sline), (plan_path, lno),
                        "Drop the GAP or target a different EXT — capability already present."))
                claims.append({"claim": cid, "status": status, "ext": ext,
                               "backed": None, "spec_line": sline})
                continue

            # COVERED / PARTIAL
            if missing:
                findings.append(_f("UNBACKED", "unbacked", "error", cid, status, ext,
                    f"BLOCK NOT FOUND ({body})", [], (body, sline), (plan_path, lno),
                    "Point the claim at a real EXT or reclassify as GAP."))
                claims.append({"claim": cid, "status": status, "ext": ext,
                               "backed": False, "spec_line": None})
                continue
            sat = _satisfies(body, req)
            if sat:
                backed = True
            claims.append({"claim": cid, "status": status, "ext": ext,
                           "backed": sat, "spec_line": sline})
            if forbid and status == "COVERED":
                miss = [k for k in forbid if k not in body.lower()]
                if miss:
                    findings.append(_f("FALSE_COVER", "false_cover", "error", cid, status, ext,
                        f"body lacks {miss} (FALSE COVER)", miss, (body[:200], sline),
                        (plan_path, lno),
                        "COVERED claim's block lacks the required capability; reclassify GAP."))
        if saw and req and not backed:
            findings.append(_f("UNBACKED", "unbacked", "error", cid, "COVERED/PARTIAL", None,
                f"no named block satisfies {req}", [], (None, None), (plan_path, None),
                "Add the capability to a named EXT or reclassify as GAP."))
        elif backed:
            covered_backed += 1

    collisions = []
    for ext in PROPOSED_NEW:
        body, _ = load_block(ext)
        if not (isinstance(body, str) and body.startswith("__")):
            collisions.append(_f("COLLISION", "collision", "error", None, None, ext,
                "proposed NEW but already exists in spec", [], (body[:200], _),
                (plan_path, None),
                "Pick a different EXT id (cluster already taken)."))

    gaps = sum(1 for c in claims if c["status"] == "GAP")
    has_err = any(f["severity"] == "error" for f in findings) or collisions
    if strict and any(f["severity"] == "warning" for f in findings):
        has_err = True
    exit_code = 1 if has_err else 0

    # debug filters: narrow findings+claims to one claim or finding kind
    if claim:
        claim = claim.upper()
        findings = [f for f in findings if (f.get("claim") or "").upper() == claim]
        claims = [c for c in claims if c["claim"].upper() == claim]
    if kind:
        kind = kind.lower()
        findings = [f for f in findings if f["kind"].lower() == kind]

    summary = {
        "claims_parsed": len(pairs),
        "covered_partial_backed": covered_backed,
        "gaps": gaps,
        "violations": len(findings),
        "collisions": len(collisions),
        "warnings": sum(1 for f in findings if f["severity"] == "warning"),
    }
    contract = {
        "schema": "ti-gate/v1",
        "tool": "ti_debug.py",
        "consumer": ["hermes", "hy3"],
        "exit": exit_code,
        "summary": summary,
        "findings": findings + collisions,
        "claims": claims,
        "proposed_new": PROPOSED_NEW,
        "schema_valid": True,
    }
    if _HAVE_JSC:
        try:
            jsonschema.validate(contract, SCHEMA)
        except Exception as e:  # never emit a broken contract
            contract = {"schema": "ti-gate/v1", "tool": "ti_debug.py",
                        "consumer": ["hermes", "hy3"], "exit": 2,
                        "schema_valid": False, "error": str(e)}
            exit_code = 2
    return contract, exit_code


def _f(fid, kind, severity, claim, status, ext, detail, missing, evidence, locs, fix):
    ev = {}
    if evidence[0] is not None:
        ev["spec_block"] = evidence[0]
    if locs[1] is not None:
        ev["plan_line_text"] = _plan_line(locs[0], locs[1])
    return {
        "id": fid, "kind": kind, "severity": severity,
        "claim": claim, "status": status, "ext": ext, "detail": detail,
        "missing_keywords": missing, "evidence": ev,
        "locs": {"spec": {"file": _spec_file(ext), "line": evidence[1]} if evidence[1] else None,
                 "plan": {"file": os.path.relpath(locs[0], BASE), "line": locs[1]} if locs[1] else None},
        "fix_hint": fix,
    }


def _spec_file(ext):
    m = re.match(EXT, ext)
    return f"spec/{m.group(1)}.md" if m else "?"


def _plan_line(path, lno):
    try:
        return open(path, encoding="utf-8").read().splitlines()[lno - 1].strip()
    except Exception:
        return ""


# --------------------------------------------------------------------------- views
def _human(contract):
    print("=" * 64)
    print("TI-GOSPEL DEBUG GATE  (ti_debug.py — parse-from-prose + contract)")
    print("=" * 64)
    s = contract["summary"]
    print(f"claims parsed: {s['claims_parsed']} | backed: {s['covered_partial_backed']} | "
          f"gaps: {s['gaps']} | proposed-new check: {len(contract.get('proposed_new', []))}")
    if contract.get("claims"):
        print(f"\nMATRIX ({len(contract['claims'])}):")
        for c in contract["claims"]:
            b = c.get("backed")
            if c["status"] == "PARTIAL":
                bstr = "partial"
            elif b is True:
                bstr = "backed"
            elif b is False:
                bstr = "NO"
            else:
                bstr = "-"
            print(f"  {c['claim']:<4} {c['status']:<8} {c['ext']:<14} {bstr}")
    if contract.get("findings"):
        print(f"\nFINDINGS ({len(contract['findings'])}):")
        for f in contract["findings"]:
            tag = "!" if f["severity"] == "error" else "~"
            loc = f["locs"]["plan"] or f["locs"]["spec"] or {}
            where = f"{loc.get('file','?')}:{loc.get('line','?')}" if loc else ""
            print(f"  {tag} {f['id']} {f.get('claim') or ''}->{f.get('ext') or ''} ({where})")
            print(f"      {f['detail']}")
            if f.get("fix_hint"):
                print(f"      fix: {f['fix_hint']}")
    if contract["exit"] == 0:
        print("\nGATE: PASS (exit 0) — COVERED/PARTIAL backed; GAPs correct; no collisions.")
    else:
        print(f"\nGATE: FAIL (exit {contract['exit']}) — fix findings before declaring done.")


# --------------------------------------------------------------------------- selftest
def selftest():
    import tempfile
    ok = True

    def check(name, cond):
        nonlocal ok
        if cond:
            print(f"  PASS: {name}")
        else:
            ok = False
            print(f"  FAIL: {name}")

    # positive: real plan gates exit 0
    c, rc = run()
    check("positive plan exit 0", rc == 0)
    check("positive schema_valid", c.get("schema_valid") is True)

    # negative: bogus GAP (non-existent + not proposed) -> FAIL
    plan = open(PLAN, encoding="utf-8").read()
    bad = plan + "\nR99  Bogus.\n    SPEC: GAP — M4.5-EXT-99 (typo)\n"
    tf = tempfile.NamedTemporaryFile(mode="w", suffix=".md", delete=False, encoding="utf-8")
    tf.write(bad); tf.close()
    c2, rc2 = run(plan_path=tf.name)
    os.unlink(tf.name)
    check("bogus GAP -> FAIL", rc2 == 1 and any(f["id"] == "WRONG_GAP" for f in c2["findings"]))

    # collision: proposed id that exists -> FAIL
    c3, rc3 = run()
    # force a collision by monkeypatching PROPOSED_NEW
    global PROPOSED_NEW
    old = PROPOSED_NEW
    PROPOSED_NEW = ["M4.5-EXT-20"]
    c4, rc4 = run()
    PROPOSED_NEW = old
    check("collision -> FAIL", rc4 == 1 and any(f["id"] == "COLLISION" for f in c4["findings"]))

    # T15 is a legitimately COVERED+backed claim (EXT-01 has volumetric+cone) — NOT unclassified
    t15 = [c for c in c["claims"] if c["claim"] == "T15"]
    check("T15 is COVERED and backed (not spurious unclassified)",
          any(x["status"] == "COVERED" and x["backed"] for x in t15))

    # JSON contract valid shape
    check("contract has findings+claims", "findings" in c and "claims" in c)
    check("schema field present", c.get("schema") == "ti-gate/v1")

    print(f"\nSELFTEST: {'PASS' if ok else 'FAIL'} (exit {'0' if ok else '2'})")
    return 0 if ok else 2


# --------------------------------------------------------------------------- cli
def main(argv=None):
    argv = argv if argv is not None else sys.argv[1:]
    args = {"human": False, "jsonl": False, "selftest": False, "schema": False,
            "gha": False, "plan": PLAN, "spec": SPEC, "strict": False,
            "claim": None, "kind": None}
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--human": args["human"] = True
        elif a == "--jsonl": args["jsonl"] = True
        elif a == "--selftest": args["selftest"] = True
        elif a == "--schema": args["schema"] = True
        elif a == "--gha": args["gha"] = True
        elif a == "--strict": args["strict"] = True
        elif a == "--claim": i += 1; args["claim"] = argv[i]
        elif a == "--kind": i += 1; args["kind"] = argv[i]
        elif a == "--plan": i += 1; args["plan"] = argv[i]
        elif a == "--spec": i += 1; args["spec"] = argv[i]
        elif a in ("-h", "--help"):
            print(__doc__); return 0
        i += 1

    if args["selftest"]:
        return selftest()
    if args["schema"]:
        print(json.dumps(SCHEMA, indent=2)); return 0

    contract, rc = run(args["plan"], args["spec"], args["strict"],
                       claim=args["claim"], kind=args["kind"])

    if args["gha"]:
        for f in contract["findings"]:
            sev = "error" if f["severity"] == "error" else "warning"
            loc = f["locs"]["plan"] or f["locs"]["spec"] or {}
            fn = loc.get("file", "recon/PLAN_threat_interactive_gospel_2026-07-16.md")
            ln = loc.get("line", 1)
            msg = f"{f['id']} {f.get('claim') or ''}->{f.get('ext') or ''}: {f['detail']}"
            print(f"::{sev} file={fn},line={ln},title={f['id']}::{msg}")
        if rc == 0:
            print("::notice title=ti_debug::GATE PASS — all claims backed")
        return rc

    use_human = args["human"] or (sys.stdout.isatty() and not args["jsonl"])
    if args["jsonl"]:
        for f in contract["findings"]:
            print(json.dumps(f, ensure_ascii=False))
        print(json.dumps({"schema": contract["schema"], "exit": rc,
                          "summary": contract["summary"]}, ensure_ascii=False))
    elif use_human:
        _human(contract)
    else:
        print(json.dumps(contract, ensure_ascii=False))
    return rc


if __name__ == "__main__":
    sys.exit(main())
