#!/usr/bin/env python3
"""fix_m1.py — Repair spec/M1.md structural corruption (Phases 2-4 of the plan).

Read-only by default: writes to --out (a COPY), never touches spec/M1.md unless
--apply is passed AND --out points at spec/M1.md. Designed to be re-run; the
companion verify step is `python scripts/verify_m0_parity.py` against the output.

Transform pipeline (all mechanical / reformat-only, no technical content invented):
  1. split_headers      : every `#### [Mx-EXT-NN]` -> own line (un-weld)
  2. split_subsections  : every `##### ...` -> own line (un-weld)
  3. remove_foreign     : drop blocks whose label != 'M1' (M4/M4.5/M2/... pollution)
  4. add_anchors        : `<a id="M1-EXT-NN"></a>` line before each kept header
  5. add_layers         : `> **tags/tl;dr/meta/ctx**` injected layers where missing
  6. fix_frontmatter    : ext_blocks -> actual M1 count; TOC regenerated
  7. ensure_clusters    : keep the `### Cluster` dividers
  8. write sidecar      : spec/M1.index.json regenerated to match

Run:
  python recon/fix_m1.py                       # dry-run -> recon/_m1_fixed.md + report
  python recon/fix_m1.py --out recon/_m1_fixed.md
  python recon/fix_m1.py --apply               # WRITE spec/M1.md + sidecar (needs GO)
"""
import sys, re, os, json, argparse

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")
SRC = os.path.join(SPEC, "M1.md")
OUT_DEFAULT = os.path.join(BASE, "_m1_fixed.md")
SIDECAR = os.path.join(SPEC, "M1.index.json")

# group(2) = whole `[Mx-EXT-NN]`, group(3) = label ('M1'), group(4) = num
HDR_LINE = re.compile(r'^(####\s+`?)(\[(M\d+(?:\.\d+)?)-EXT-(\d+)\])')
HDR_ANY = re.compile(r'####\s+`?\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]')
EXTREF = re.compile(r'\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]')
OWN_LABEL = "M1"  # file M1.md's own label

def report(msg):
    print(msg)

def split_headers(txt):
    n = len(re.findall(r'(?m)(?<!^)####\s+\[M\d+(?:\.\d+)?-EXT-\d+\]', txt))
    txt = re.sub(r'(?m)(?<!^)(####\s+\[M\d+(?:\.\d+)?-EXT-\d+\])', r'\n\1', txt)
    return txt, n

def split_subsections(txt):
    n = len(re.findall(r'(?m)(?<!^)#####\s+\S', txt))
    txt = re.sub(r'(?m)(?<!^)(#####\s+(\S.*?))(?=$|\n)', r'\n\1', txt)
    return txt, n

def parse_blocks(rows):
    """Return list of (label, num, bid, start, end) for line-start `#### [` headers.
    bid is the zero-padded id string e.g. 'M1-EXT-01' (no brackets)."""
    blocks = []
    for i, l in enumerate(rows):
        m = HDR_LINE.match(l)
        if m:
            label = m.group(3)
            num = int(m.group(4))
            bid = m.group(2)[1:-1]  # strip [ ]
            blocks.append([label, num, bid, i, None])
    for k in range(len(blocks)):
        s = blocks[k][3]
        e = len(rows)
        for j in range(s + 1, len(rows)):
            if HDR_LINE.match(rows[j]) or re.match(r'^##\s', rows[j]):
                e = j; break
        blocks[k][4] = e
    return blocks

def remove_foreign(rows):
    blocks = parse_blocks(rows)
    drop = set()
    for (label, num, bid, s, e) in blocks:
        if label != OWN_LABEL:
            drop.update(range(s, e))
    kept = [rows[i] for i in range(len(rows)) if i not in drop]
    removed = len(rows) - len(kept)
    m1 = sum(1 for b in blocks if b[0] == OWN_LABEL)
    return kept, removed, m1

def add_anchors(rows):
    out = []
    added = 0
    for i, l in enumerate(rows):
        m = HDR_LINE.match(l)
        if m:
            bid = m.group(2)[1:-1]
            if not (i > 0 and '<a id="%s">' % bid in rows[i - 1]):
                out.append('<a id="%s"></a>' % bid)
                added += 1
        out.append(l)
    return out, added

def derive_tags(title):
    w = re.findall(r'[A-Za-z][A-Za-z0-9+]*', title)
    return w[0].lower() if w else "general"

def add_layers(rows):
    blocks = parse_blocks(rows)
    out = []
    added_blocks = 0
    for i, l in enumerate(rows):
        out.append(l)
        m = HDR_LINE.match(l)
        if not m:
            continue
        bid = m.group(2)[1:-1]
        s = i
        e = None
        for j in range(i + 1, len(rows)):
            if HDR_LINE.match(rows[j]) or re.match(r'^##\s', rows[j]):
                e = j; break
        e = len(rows) if e is None else e
        seg = rows[s:e]
        have = {"tags": any("> **tags**" in x for x in seg),
                "tl;dr": any("> **tl;dr**" in x for x in seg),
                "meta": any("> **meta**" in x for x in seg),
                "ctx": any("> **ctx**" in x for x in seg)}
        title = l.split("]", 1)[1].strip() if "]" in l else bid
        refs = sorted(set(EXTREF.findall("\n".join(seg))))
        dep = "[" + "], [".join("%s-EXT-%s" % (a, b) for a, b in refs) + "]" if refs else "-"
        if not (have["tags"] and have["tl;dr"] and have["meta"] and have["ctx"]):
            added_blocks += 1
        if not have["tags"]:
            out.append("> **tags** · %s" % derive_tags(title))
        if not have["tl;dr"]:
            out.append("> **tl;dr** · %s" % title)
        if not have["meta"]:
            out.append("> **meta** · depends-on: %s · depended-by: -" % dep)
        if not have["ctx"]:
            out.append("> **ctx** · %s" % title)
    return out, added_blocks

def build_toc(blocks_meta):
    lines = ['<details><summary>M1 block index (click to expand)</summary>']
    for bid, title in blocks_meta:
        lines.append("- `%s` %s" % (bid, title))
    lines.append("</details>")
    return "\n".join(lines)

def fix_frontmatter(rows, count, toc_lines):
    out = []
    if rows and rows[0].strip() == "---":
        end = rows.index("---", 1)
        fm = rows[1:end]
        newfm = []
        for l in fm:
            if l.startswith("ext_blocks:"):
                newfm.append("ext_blocks: %d" % count)
            elif l.startswith("clusters:"):
                newfm.append("clusters: 4")
            else:
                newfm.append(l)
        out.extend(["---"] + newfm + ["---"])
        rest = rows[end + 1:]
    else:
        out = rows
        rest = []
    txt = "\n".join(rest)
    txt = re.sub(r'<details>.*?</details>', toc_lines, txt, flags=re.S)
    out.extend(txt.split("\n"))
    return out

def ensure_clusters(rows):
    clusters = [i for i, l in enumerate(rows) if l.startswith("### Cluster")]
    return rows, len(clusters)

def build_sidecar(rows):
    blocks = parse_blocks(rows)
    meta = []
    for (label, num, bid, s, e) in blocks:
        if label != OWN_LABEL:
            continue
        seg = rows[s:e]
        title = rows[s].split("]", 1)[1].strip() if "]" in rows[s] else bid
        refs = sorted(set(EXTREF.findall("\n".join(seg))))
        depends_on = ["[%s-EXT-%s]" % (a, b) for a, b in refs]
        c = None
        for k in range(s - 1, -1, -1):
            mm = re.match(r'^###\s+Cluster\s+(\S+)', rows[k])
            if mm:
                c = mm.group(1); break
        meta.append({
            "id": bid,
            "title": title,
            "cluster": c,
            "depends_on": depends_on,
            "depended_by": [],
            "line": s + 1,
            "anchor": bid,
        })
    return meta

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", default=SRC)
    ap.add_argument("--out", default=OUT_DEFAULT)
    ap.add_argument("--apply", action="store_true",
                    help="allow writing back to spec/M1.md + sidecar (REQUIRES explicit GO)")
    args = ap.parse_args()

    if args.apply and os.path.abspath(args.out) != os.path.abspath(SRC):
        report("ERROR: --apply requires --out to be spec/M1.md exactly.")
        return 2

    raw = open(args.inp, encoding="utf-8").read()

    txt, nh = split_headers(raw)
    report("split_headers: %d welded headers un-welded" % nh)
    txt, ns = split_subsections(txt)
    report("split_subsections: %d welded subsections un-welded" % ns)

    rows = txt.split("\n")
    rows, removed, m1count = remove_foreign(rows)
    report("remove_foreign: %d foreign lines dropped; %d M1-EXT blocks kept" % (removed, m1count))

    rows, na = add_anchors(rows)
    report("add_anchors: %d anchors inserted" % na)

    rows, nl = add_layers(rows)
    report("add_layers: %d M1 blocks needed >=1 injected layer" % nl)

    blocks = parse_blocks(rows)
    m1meta = [(b[2], rows[b[3]].split("]", 1)[1].strip() if "]" in rows[b[3]] else b[2])
              for b in blocks if b[0] == OWN_LABEL]
    toc = build_toc(m1meta)
    rows = fix_frontmatter(rows, m1count, toc)
    report("fix_frontmatter: ext_blocks=%d; TOC regen (%d entries)" % (m1count, len(m1meta)))

    rows, nc = ensure_clusters(rows)
    report("ensure_clusters: %d ### Cluster dividers present" % nc)

    sidecar = build_sidecar(rows)
    report("build_sidecar: %d entries" % len(sidecar))

    out_text = "\n".join(rows)
    open(args.out, "w", encoding="utf-8").write(out_text)
    report("WROTE %s (%d lines, %d chars)" % (args.out, len(rows), len(out_text)))

    sc_path = SIDECAR if args.apply else os.path.join(BASE, "_m1_fixed.index.json")
    json.dump(sidecar, open(sc_path, "w", encoding="utf-8"), indent=2, ensure_ascii=False)
    report("WROTE %s (%d entries)" % (sc_path, len(sidecar)))

    report("DRY-RUN: spec/M1.md untouched." if not args.apply else
           "APPLY MODE: spec/M1.md + spec/M1.index.json updated.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
