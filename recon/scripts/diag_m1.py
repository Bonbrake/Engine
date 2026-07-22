#!/usr/bin/env python3
"""diag_m1.py — READ-ONLY structural measurement of spec/M1.md.
Characterizes every defect the fix script must handle. No file is modified.
Run: python recon/diag_m1.py [path-to-M1.md]
"""
import sys, re, os, json

PATH = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "spec", "M1.md")
txt = open(PATH, encoding="utf-8").read()
rows = txt.split("\n")
N = len(rows)

HDR = re.compile(r'^(####\s+`?\[(M\d+(?:\.\d+)?)-EXT-(\d+)\])')
HDR_ANY = re.compile(r'####\s+`?\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]')
# group(1)=full [..], group(2)=label, group(3)=num  -- for the anchored matcher we re-use HDR_ANY
SUB = re.compile(r'^#####\s+(.*?)\s*$')
FENCE = re.compile(r'^\s*```')

def find_blocks():
    """Return list of (label, num, start_idx, end_idx, header_text) by scanning
    for '#### [' at the START of a line. Mid-line headers are handled later."""
    blocks = []
    for i, l in enumerate(rows):
        m = HDR.match(l)
        if m:
            blocks.append([m.group(1), int(m.group(3)), i, None, l])
    # compute end as next header or '## '
    for k in range(len(blocks)):
        s = blocks[k][2]
        e = N
        for j in range(s + 1, N):
            if HDR.match(rows[j]) or re.match(r'^##\s', rows[j]):
                e = j; break
        blocks[k][3] = e
    return blocks

blocks = find_blocks()

# Also count ALL header occurrences (incl mid-line welded) position-independently
all_occ = HDR_ANY.findall(txt)

print("=" * 70)
print("FILE:", os.path.relpath(PATH))
print("lines:", N, "chars:", len(txt))
print("=" * 70)

# frontmatter
fm_blocks = None
if rows[0].strip() == "---":
    end = rows.index("---", 1)
    fm = "\n".join(rows[1:end])
    m = re.search(r'ext_blocks:\s*(\d+)', fm)
    fm_blocks = int(m.group(1)) if m else None
    print("frontmatter ext_blocks:", fm_blocks)

# label counts (all occurrences, incl welded)
from collections import Counter
labels = Counter(l for l, n in all_occ)
print("ALL header occurrences by label:", dict(labels))

# weld detection: are '#### [' occurrences preceded by newline (own line) or mid-line?
welded = 0
for m in HDR_ANY.finditer(txt):
    pos = m.start()
    if pos > 0 and txt[pos - 1] != "\n":
        welded += 1
print("mid-line (welded) header occurrences:", welded)

print("-" * 70)
print("PER-BLOCK ANALYSIS (line-start headers only):")
print("%-12s %5s %7s %4s %4s %4s %4s %4s %4s %4s  fence" % (
    "id", "line", "anchor", "tldr", "meta", "ctx", "tags", "Sys", "Math", "Ref"))
REQ = {"Systems Touched", "Math", "How It Works", "Reference Implementation", "Player-Facing Impact"}
stub_count = 0
missing_skel = 0
no_anchor = 0
for (label, num, s, e, htext) in blocks:
    seg = rows[s:e]
    anchor_ok = (s > 0 and '<a id="%s">' % label in rows[s-1])
    if not anchor_ok:
        no_anchor += 1
    has = {"tags": any("> **tags**" in ln for ln in seg),
           "tl;dr": any("> **tl;dr**" in ln for ln in seg),
           "meta": any("> **meta**" in ln for ln in seg),
           "ctx": any("> **ctx**" in ln for ln in seg)}
    subs = set()
    for ln in seg:
        mm = SUB.match(ln)
        if mm:
            subs.add(mm.group(1).split(":")[0].strip())
    missing = REQ - subs
    is_stub = any("> **STUB**" in ln for ln in seg)
    if is_stub:
        stub_count += 1
    if missing and not is_stub:
        missing_skel += 1
    # fence balance (only '```' lines)
    fcount = sum(1 for ln in seg if FENCE.match(ln))
    fbal = "ok" if fcount % 2 == 0 else "UNBAL(%d)" % fcount
    print("%-12s %5d %7s %4s %4s %4s %4s %4s %4s %4s  %s" % (
        label, s + 1, "Y" if anchor_ok else "N",
        "Y" if has["tags"] else "-", "Y" if has["tl;dr"] else "-",
        "Y" if has["meta"] else "-", "Y" if has["ctx"] else "-",
        "Y" if "Systems Touched" in subs else "-",
        "Y" if "Math" in subs else "-",
        "Y" if "Reference Implementation" in subs else "-",
        fbal))
print("-" * 70)
print("blocks (line-start headers):", len(blocks))
print("  stub blocks:", stub_count)
print("  non-stub blocks MISSING some of 5-subsection skeleton:", missing_skel)
print("  blocks missing preceding anchor:", no_anchor)

# cluster dividers
clusters = [i for i, l in enumerate(rows) if l.startswith("### Cluster")]
print("### Cluster dividers:", len(clusters), "at lines", [c+1 for c in clusters])

# TOC analysis
m = re.search(r'<details>.*?</details>', txt, re.S)
if m:
    toc = m.group(0)
    toc_ids = re.findall(r'M1-EXT-(\d+)', toc)
    print("TOC lists M1-EXT entries:", len(toc_ids), "range",
          (min(int(x) for x in toc_ids), max(int(x) for x in toc_ids)) if toc_ids else "-")
    actual_ids = [num for (label, num, s, e, h) in blocks if label == "M1-EXT"]
    print("actual M1-EXT block ids:", len(actual_ids))
    tocset, actset = set(toc_ids), set(str(x) for x in actual_ids)
    print("  in TOC not in file:", sorted(tocset - actset))
    print("  in file not in TOC:", sorted(actset - tocset))
else:
    print("NO <details> TOC found")

# foreign blocks: list them
foreign = [(label, num, s) for (label, num, s, e, h) in blocks if label in ("M4-EXT", "M4.5-EXT")]
print("-" * 70)
print("FOREIGN blocks (to be removed):", len(foreign))
print("  M4-EXT:", sum(1 for l, n, s in foreign if l == "M4-EXT"),
      " M4.5-EXT:", sum(1 for l, n, s in foreign if l == "M4.5-EXT"))

# global fence balance
gf = sum(1 for l in rows if FENCE.match(l))
print("GLOBAL fence count:", gf, "balance:", "ok" if gf % 2 == 0 else "UNBALANCED")
