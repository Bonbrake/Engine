#!/usr/bin/env python3
"""fix_m1.py — Repair spec/M1.md structural corruption (Phases 2-4 of the plan).

Read-only by default: writes to --out (a COPY), never touches spec/M1.md unless
--apply is passed AND --out points at spec/M1.md. Designed to be re-run; the
companion verify step is `python scripts/verify_m0_parity.py` against the output.

KEY FACT (measured from spec/M1.md HEAD): M1's own block headers are `### [M1-EXT-NN]`
(H3), NOT `#### [M1-EXT-NN]` (H4). The M0-parity verifier expects H4. So this script
detects M1 headers at H3 OR H4, KEEPS only M1-label segments, and PROMOTES them to H4.
Foreign pollution uses `### [Mx-EXT]` (other labels), `#### [Mx-EXT]`, and
`## Milestone ...` appendix blocks — all dropped.

Transform pipeline (mechanical / reformat-only, no technical content invented):
  1. split_headers      : un-weld every `####`/`### [Mx-EXT]` and `## Milestone` -> own line
  2. split_subsections  : un-weld every `##### ...` -> own line
  3. remove_foreign     : keep only M1-label block segments; drop all other labels + appendices
  4. promote_headers    : `### [M1-EXT-NN]` -> `#### [M1-EXT-NN]` (H4 standard)
  5. add_anchors        : `<a id="M1-EXT-NN"></a>` line before each kept header
  6. add_layers         : `> **tags/tl;dr/meta/ctx**` injected layers where missing
  7. fix_frontmatter    : ext_blocks -> actual M1 count; TOC regenerated
  8. build_sidecar      : spec/M1.index.json regenerated to match

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
OWN_LABEL = "M1"

# M1 block header at H3 OR H4:  ### [M1-EXT-NN]  /  #### [M1-EXT-NN]
M1_HDR = re.compile(r'^(#{3,4})\s+`?\[(M1)-EXT-(\d+)\]')
# Any block header (own or foreign) at H3/H4, plus milestone appendix markers.
SEG_HDR = re.compile(
    r'^(#{3,4})\s+`?\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]'
    r'|^(##\s+Milestone\s+\S+)')
EXTREF = re.compile(r'\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]')

def report(msg):
    print(msg)

# ---- 1. un-weld headers ----
def split_headers(txt):
    total = 0
    # H4 block headers first (most specific).
    for pat in (r'####\s+\[M\d+(?:\.\d+)?-EXT-\d+\]',):
        n = len(re.findall(r'(?m)(?<!^)' + pat, txt))
        txt = re.sub(r'(?m)(?<!^)(' + pat + r')', r'\n\1', txt)
        total += n
    # H3 block headers, but NOT when preceded by an extra '#' (would match inside '####').
    for pat in (r'###\s+\[M\d+(?:\.\d+)?-EXT-\d+\]',):
        n = len(re.findall(r'(?m)(?<!^)(?<!#)' + pat, txt))
        txt = re.sub(r'(?m)(?<!^)(?<!#)(' + pat + r')', r'\n\1', txt)
        total += n
    # Milestone appendix markers.
    for pat in (r'##\s+Milestone\s+\S+',):
        n = len(re.findall(r'(?m)(?<!^)' + pat, txt))
        txt = re.sub(r'(?m)(?<!^)(' + pat + r')', r'\n\1', txt)
        total += n
    return txt, total

# ---- 2. un-weld subsections ----
def split_subsections(txt):
    n = len(re.findall(r'(?m)(?<!^)#####\s+\S', txt))
    txt = re.sub(r'(?m)(?<!^)(#####\s+(\S.*?))(?=$|\n)', r'\n\1', txt)
    return txt, n

# ---- 3. remove foreign (keep only M1-label segments + prologue before first M1) ----
def remove_foreign(rows):
    bounds = [i for i, l in enumerate(rows) if SEG_HDR.match(l)]
    if not bounds:
        return rows, 0, 0
    first = bounds[0]
    prologue = rows[:first]          # frontmatter + intro + TOC precede first M1 block
    bounds.append(len(rows))
    keep = list(prologue)
    m1 = 0
    removed = 0
    for k in range(len(bounds) - 1):
        s, e = bounds[k], bounds[k + 1]
        m = SEG_HDR.match(rows[s])
        opener = m.group(0)
        is_m1 = bool(re.match(r'^#{3,4}\s+`?\[M1-EXT-', opener))
        if is_m1:
            keep.extend(rows[s:e])
            m1 += 1
        else:
            removed += (e - s)
    return keep, removed, m1

# ---- 4. promote M1 headers H3 -> H4 ----
def promote_headers(rows):
    out = []
    promoted = 0
    for l in rows:
        m = M1_HDR.match(l)
        if m and m.group(1) == "###":
            out.append("####" + l[3:])
            promoted += 1
        else:
            out.append(l)
    return out, promoted

# ---- 5. anchors ----
def add_anchors(rows):
    out, added = [], 0
    for i, l in enumerate(rows):
        m = M1_HDR.match(l)
        if m:
            bid = "M1-EXT-" + m.group(3)
            if not (i > 0 and '<a id="%s">' % bid in rows[i - 1]):
                out.append('<a id="%s"></a>' % bid)
                added += 1
        out.append(l)
    return out, added

# ---- 6. layers ----
def derive_tags(title):
    w = re.findall(r'[A-Za-z][A-Za-z0-9+]*', title)
    return w[0].lower() if w else "general"

def add_layers(rows):
    # segment by M1 headers (now H4 after promote)
    bounds = [i for i, l in enumerate(rows) if M1_HDR.match(l)]
    bounds.append(len(rows))
    out, added_blocks = [], 0
    for i, l in enumerate(rows):
        out.append(l)
        m = M1_HDR.match(l)
        if not m:
            continue
        bid = "M1-EXT-" + m.group(3)
        s, e = i, len(rows)
        for j in range(i + 1, len(rows)):
            if M1_HDR.match(rows[j]):
                e = j; break
        seg = rows[s:e]
        have = {k: any("> **%s**" % k in x for x in seg)
                for k in ("tags", "tl;dr", "meta", "ctx")}
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

# ---- 7. frontmatter + TOC ----
def build_toc(meta):
    lines = ['<details><summary>M1 block index (click to expand)</summary>']
    for bid, title in meta:
        lines.append("- `%s` %s" % (bid, title))
    lines.append("</details>")
    return "\n".join(lines)

def fix_frontmatter(rows, count, toc_lines):
    out = []
    if rows and rows[0].strip() == "---":
        end = rows.index("---", 1)
        newfm = []
        for l in rows[1:end]:
            if l.startswith("ext_blocks:"):
                newfm.append("ext_blocks: %d" % count)
            elif l.startswith("clusters:"):
                newfm.append("clusters: 4")
            else:
                newfm.append(l)
        out.extend(["---"] + newfm + ["---"])
        rest = rows[end + 1:]
    else:
        out, rest = rows, []
    txt = "\n".join(rest)
    txt = re.sub(r'<details>.*?</details>', toc_lines, txt, flags=re.S)
    out.extend(txt.split("\n"))
    return out

# ---- 8. sidecar ----
def build_sidecar(rows):
    bounds = [i for i, l in enumerate(rows) if M1_HDR.match(l)]
    bounds.append(len(rows))
    meta = []
    for k in range(len(bounds) - 1):
        s, e = bounds[k], bounds[k + 1]
        m = M1_HDR.match(rows[s])
        bid = "M1-EXT-" + m.group(3)
        seg = rows[s:e]
        title = rows[s].split("]", 1)[1].strip() if "]" in rows[s] else bid
        refs = sorted(set(EXTREF.findall("\n".join(seg))))
        depends_on = ["[%s-EXT-%s]" % (a, b) for a, b in refs]
        c = None
        for j in range(s - 1, -1, -1):
            mm = re.match(r'^###\s+Cluster\s+(\S+)', rows[j])
            if mm:
                c = mm.group(1); break
        meta.append({
            "id": bid, "title": title, "cluster": c,
            "depends_on": depends_on, "depended_by": [],
            "line": s + 1, "anchor": bid,
        })
    return meta

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", default=SRC)
    ap.add_argument("--out", default=OUT_DEFAULT)
    ap.add_argument("--apply", action="store_true",
                    help="write back to spec/M1.md + sidecar (REQUIRES explicit GO)")
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

    rows, np_ = promote_headers(rows)
    report("promote_headers: %d M1 headers H3->H4" % np_)

    rows, na = add_anchors(rows)
    report("add_anchors: %d anchors inserted" % na)

    rows, nl = add_layers(rows)
    report("add_layers: %d M1 blocks needed >=1 injected layer" % nl)

    bounds = [i for i, l in enumerate(rows) if M1_HDR.match(l)]
    bounds.append(len(rows))
    m1meta = []
    for k in range(len(bounds) - 1):
        s, e = bounds[k], bounds[k + 1]
        bid = "M1-EXT-" + M1_HDR.match(rows[s]).group(3)
        title = rows[s].split("]", 1)[1].strip() if "]" in rows[s] else bid
        m1meta.append((bid, title))
    toc = build_toc(m1meta)
    rows = fix_frontmatter(rows, m1count, toc)
    report("fix_frontmatter: ext_blocks=%d; TOC regen (%d entries)" % (m1count, len(m1meta)))

    clusters = [i for i, l in enumerate(rows) if l.startswith("### Cluster")]
    report("ensure_clusters: %d ### Cluster dividers present" % len(clusters))

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
