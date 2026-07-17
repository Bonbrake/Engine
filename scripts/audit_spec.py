#!/usr/bin/env python3
"""
audit_spec.py - single source of truth for ZombieEngine master-spec integrity.
Replaces the 7 overlapping/broken audit_* + hermes-verify-* scripts.

Method (sound, header-anchored):
  A header ID is a line starting with '#' (1-6), optional backtick,
  [ID], optional backtick. BOTH '#### [M0-EXT-13]' and
  '#### `[M0-EXT-54]`' forms are accepted by one regex.
  References are any '[ID]' anywhere. We then cross-check.

ID grammar: (M?digit[.digit]* | K | L) - EXT - digit+

Run:  python audit_spec.py
Exit 0 = PASS, 1 = FAIL. Prints a compact report.
"""
import re, sys, collections, os

DEFAULT = r"C:\ZombieEngine\spec\M0.md"  # spec was split into per-milestone files; point at the gold-standard file by default. For full-corpus audit, prefer scripts/verify_m0_parity.py.

HDR = re.compile(r'^#{1,6}\s+`?\[((?:M?\d[\d.]*|K|L)-EXT-\d+)\]`?')
REF = re.compile(r'\[((?:M?\d[\d.]*|K|L)-EXT-\d+)\]')

def audit(path):
    rows = open(path, encoding="utf-8").read().split("\n")
    # header -> list of line numbers (dedupe by line)
    headers = collections.defaultdict(list)
    for n, ln in enumerate(rows, 1):
        m = HDR.match(ln)
        if m:
            headers[m.group(1)].append(n)
    # references -> count
    refs = collections.Counter()
    for ln in rows:
        for mm in REF.finditer(ln):
            refs[mm.group(1)] += 1

    defined_ids = set(headers)
    ref_ids = set(refs)
    dups = {k: v for k, v in headers.items() if len(v) > 1}
    dangling = sorted(ref_ids - defined_ids)
    orphan = sorted(defined_ids - ref_ids)
    return {
        "path": path,
        "defined": len(defined_ids),
        "referenced": len(ref_ids),
        "dups": dups,
        "dangling": dangling,
        "orphan": orphan,
    }

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT
    if not os.path.exists(path):
        print(f"ERROR: {path} not found"); sys.exit(2)
    r = audit(path)
    print("=== SPEC INTEGRITY AUDIT (canonical) ===")
    print(f"  file              : {r['path']}")
    print(f"  defined IDs       : {r['defined']}")
    print(f"  referenced IDs   : {r['referenced']}")
    print(f"  duplicate defs   : {len(r['dups'])}")
    if r['dups']:
        for k, v in sorted(r['dups'].items()):
            print(f"      {k}: lines {v}")
    print(f"  dangling refs    : {len(r['dangling'])}")
    if r['dangling']:
        print("      " + ", ".join(r['dangling']))
    print(f"  orphan blocks    : {len(r['orphan'])}")
    if r['orphan']:
        print("      " + ", ".join(r['orphan']))
    ok = (r['defined'] > 0 and not r['dups'] and not r['dangling'] and not r['orphan'])
    print(f"  VERDICT           : {'PASS' if ok else 'FAIL'}")
    sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
