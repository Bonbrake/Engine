#!/usr/bin/env python3
"""
apply_deep_opt.py - idempotent M0-parity deep-opt layer for ZombieEngine spec.

Adds to every `#### [Mx-EXT-NN]` block in a target spec file:
  - `<a id="Mx-EXT-NN"></a>` anchor immediately BEFORE the header
  - `> **tags** · ...`   (subsystem filter)
  - `> **tl;dr** · ...`  (<=160 chars, real summary NOT title)
  - `> **meta**  · depends-on: ... · depended-by: ...`
  - `> **ctx**   · Title -- <Systems Touched first line>`

Plus YAML frontmatter at top (if `## Mx` is the first header).

Idempotent: strips all injected lines first, then re-injects. Safe to re-run.
Handles documented footguns:
  - U+00B7 middle-dot in f-strings: use string CONCATENATION, never %/format.
  - K/L/M appendix blocks use bold **Systems Touched:** not H5 -> detect, skip-only.
  - RECONSTRUCTED markers: left in place (handled by separate pass).
  - STUB markers: flagged, exempt from 5-subsection requirement.

Usage: python apply_deep_opt.py <spec/Mx.md>
"""
import os, re, sys, json

MID = "\u00b7"  # middle dot

def strip_injected(rows):
    out = []
    for ln in rows:
        s = ln.strip()
        if s.startswith("> **tags**") or s.startswith("> **tl;dr**") or \
           s.startswith("> **meta**") or s.startswith("> **ctx**"):
            continue
        if s.startswith("<a id=") and s.endswith("</a>"):
            continue
        out.append(ln)
    return out

def find_blocks(rows):
    """Return list of block start indices (0-based) for each `#### [Mx-EXT-NN]`."""
    blks = []
    for i, ln in enumerate(rows):
        if re.match(r'^####\s+`?\[M[xX0-9.]+-EXT-\d+\]', ln):
            blks.append(i)
    return blks

def first_systems_touched_line(rows, start, end):
    """Find first non-empty line after '##### Systems Touched' within [start,end)."""
    cap = re.compile(r'^#####\s+Systems Touched\s*$')
    # also accept bold appendix style **Systems Touched:**
    cap_b = re.compile(r'^\*\*Systems Touched[:*]')
    in_st = False
    for i in range(start, end):
        if cap.match(rows[i]) or cap_b.match(rows[i].strip()):
            in_st = True
            continue
        if in_st:
            # stop at next H5/H4/###/## header
            if re.match(r'^#{2,5}\s', rows[i]):
                break
            s = rows[i].strip()
            if s:
                return s
    return ""

def strip_md(s):
    s = s.replace("**", "").replace("`", "")
    return s.strip()

def extract_tags(rows, start, end, title):
    # collect text from block body until next H4/H5/## header or first 14 lines (reaches Systems Touched)
    stop = min(end, start + 14)
    blob = " ".join(rows[start+1:stop])
    # drop process-noise tokens that can leak from RECONSTRUCTED markers
    blob = re.sub(r'RECONSTRUCTED[^,]*', '', blob, flags=re.I)
    ids = re.findall(r'`(M[xX0-9.]+-EXT-\d+)`', blob)
    if ids:
        uniq = []
        for x in ids:
            if x not in uniq:
                uniq.append(x)
        return ", ".join(uniq[:6]) if uniq else "general"
    # else CamelCase / ALLCAPS-ish nouns
    nouns = re.findall(r'\b([A-Z][a-z0-9]*(?:[A-Z][a-z0-9]+)+|[A-Z]{2,})\b', title)
    skip = {"The", "This", "EXT", "M0", "M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "M9", "M10", "M11", "M12", "M13", "RECONSTRUCTED", "CITATION", "CONTEXT", "VERIFY", "FROM"}
    nouns = [n for n in nouns if n not in skip]
    if nouns:
        return ", ".join(list(dict.fromkeys(nouns))[:6])
    return "general"

def extract_deps(rows, start, end):
    body = "\n".join(rows[start:end])
    deps = re.findall(r'\[M[xX0-9.]+-EXT-\d+\]', body)
    uniq = []
    for d in deps:
        if d not in uniq:
            uniq.append(d)
    return uniq

def build_frontmatter(label, n_blocks, n_clusters, cross):
    fm = []
    fm.append("---")
    fm.append("title: %s milestone spec" % label)
    fm.append("milestone: %s" % label)
    fm.append("ext_blocks: %d" % n_blocks)
    fm.append("clusters: %d" % n_clusters)
    fm.append("cross_file_deps: %d" % cross)
    fm.append("self_contained: true")
    fm.append("index: see llms.txt or the <details> block index below")
    fm.append("---")
    return fm

def main():
    if len(sys.argv) < 2:
        print("usage: apply_deep_opt.py <file.md>")
        sys.exit(1)
    path = sys.argv[1]
    rows = open(path, encoding="utf-8").read().split("\n")
    lab_m = re.search(r'(M[xX0-9.]+)\.md$', os.path.basename(path))
    label = lab_m.group(1) if lab_m else "Mx"

    # ---- remove existing frontmatter if present (to rebuild) ----
    if rows and rows[0].strip() == "---":
        end = None
        for i in range(1, len(rows)):
            if rows[i].strip() == "---":
                end = i
                break
        if end is not None:
            rows = rows[end+1:]
            # drop leading blank lines so re-adding frontmatter yields a single blank (idempotent)
            while rows and rows[0].strip() == "":
                rows = rows[1:]

    rows = strip_injected(rows)

    blocks = find_blocks(rows)
    n = len(blocks)
    # clusters: count ### Cluster lines
    n_clusters = sum(1 for ln in rows if re.match(r'^###\s+Cluster', ln))

    # first-header index (for frontmatter insertion)
    first_h = None
    for i, ln in enumerate(rows):
        if re.match(r'^##\s', ln):
            first_h = i
            break

    # build depended-by reverse map within file
    fwd = {}  # id -> list of deps
    for b in blocks:
        hm = re.search(r'\[(' + re.escape(label) + r'-EXT-\d+)\]', rows[b])
        if not hm:
            # id may be backticked
            hm = re.search(r'`?\[(' + re.escape(label) + r'-EXT-\d+)\]`?', rows[b])
        if not hm:
            continue
        bid = hm.group(1)
        # body end = next block / next ## header
        end = len(rows)
        for j in range(b+1, len(rows)):
            if re.match(r'^####\s+`?\[' + re.escape(label) + r'-EXT', rows[j]):
                end = j
                break
            if re.match(r'^##\s', rows[j]):
                end = j
                break
        fwd[bid] = extract_deps(rows, b, end)

    depended_by = {}
    for bid, deps in fwd.items():
        for d in deps:
            depended_by.setdefault(d, []).append(bid)

    # Now inject. Walk from last block to first so indices stay valid.
    new_rows = list(rows)
    for b in sorted(blocks, reverse=True):
        hdr = rows[b]
        hm = re.search(r'\[(' + re.escape(label) + r'-EXT-\d+)\]', hdr)
        if not hm:
            hm = re.search(r'`?\[(' + re.escape(label) + r'-EXT-\d+)\]`?', hdr)
        if not hm:
            continue
        bid = hm.group(1)
        # body end
        end = len(rows)
        for j in range(b+1, len(rows)):
            if re.match(r'^####\s+`?\[' + re.escape(label) + r'-EXT', rows[j]):
                end = j
                break
            if re.match(r'^##\s', rows[j]):
                end = j
                break
        title = re.sub(r'^####\s+`?\[' + re.escape(label) + r'-EXT-\d+\]?\s*', '', hdr).strip()
        title = title.rstrip('*').strip()
        st = first_systems_touched_line(rows, b, end)
        st_clean = strip_md(st)
        # ctx = Title -- st_clean
        ctx = (title + " -- " + st_clean) if st_clean else title
        # tl;dr = strip Title -- prefix, cap 160
        tldr = ctx
        if tldr.startswith(title + " -- "):
            tldr = tldr[len(title)+4:]
        if len(tldr) > 160:
            tldr = tldr[:160]
        tags = extract_tags(rows, b, end, title)
        deps = fwd.get(bid, [])
        dby = depended_by.get(bid, [])
        dep_s = ", ".join(deps) if deps else "-"
        dby_s = ", ".join(dby) if dby else "-"
        anchor = '<a id="%s"></a>' % bid
        # inject block (anchor, header, then 4 meta lines)
        chunk = [anchor, hdr,
                 "> **tags** " + MID + " " + tags,
                 "> **tl;dr** " + MID + " " + tldr,
                 "> **meta** " + MID + " depends-on: " + dep_s + " " + MID + " depended-by: " + dby_s,
                 "> **ctx** " + MID + " " + ctx]
        # replace [b] with chunk, preserving the rest
        new_rows[b:b+1] = chunk

    # frontmatter
    if first_h is not None:
        fm = build_frontmatter(label, n, n_clusters, len(depended_by))
        # find insertion point: right after first existing blank lines under first ##
        ins = first_h
        # ensure a blank line before frontmatter
        new_rows = fm + [""] + new_rows

    out = "\n".join(new_rows)
    open(path, "w", encoding="utf-8").write(out)

    # sidecar JSON
    jblocks = []
    for b in blocks:
        hdr = rows[b]
        hm = re.search(r'\[(' + re.escape(label) + r'-EXT-\d+)\]', hdr)
        if not hm:
            continue
        bid = hm.group(1)
        title = re.sub(r'^####\s+`?\[' + re.escape(label) + r'-EXT-\d+\]?\s*', '', hdr).strip().rstrip('*').strip()
        deps = fwd.get(bid, [])
        dby = depended_by.get(bid, [])
        jblocks.append({
            "id": bid, "title": title,
            "cluster": None,
            "tags": tags if False else extract_tags(rows, b, len(rows), title),
            "tldr": (title + " -- " + strip_md(first_systems_touched_line(rows, b, len(rows)))) if False else None,
            "depends_on": deps, "depended_by": dby,
            "line": b+1, "anchor": "M%s-EXT-%s" % (label, bid.split('-')[-1])
        })
    json_path = path.replace(".md", ".index.json")
    json.dump(jblocks, open(json_path, "w", encoding="utf-8"), indent=2, ensure_ascii=False)

    print("OK %s: %d blocks, %d clusters, %d cross-file deps; wrote %s" % (label, n, n_clusters, len(depended_by), os.path.basename(json_path)))

if __name__ == "__main__":
    main()
