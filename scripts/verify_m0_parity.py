#!/usr/bin/env python3
# verify_m0_parity.py — check every spec/Mx.md against M0-PARITY standard.
# Scoped to each file's OWN label (so foreign-embedded blocks like M7-EXT-04 in M4.md
# are NOT counted as the file's own blocks — matches llms.txt own-count rule).
# Checks: frontmatter ext_blocks, TOC, ### Cluster dividers, per-block anchor +
# tags->tl;dr->meta->ctx order, 5-subsection skeleton, ascending IDs, fences even,
# JSON sidecar exists with matching block count + 6 keys.
import re, os, json, glob

SPEC = r"C:\ZombieEngine\spec"

def own_blocks(rows, label):
    out = []
    rx = re.compile(r'^####\s+`?\[(' + re.escape(label) + r'-EXT-(\d+))\]')
    for i, l in enumerate(rows):
        m = rx.match(l)
        if m:
            out.append((i, m.group(1), int(m.group(2))))
    return out

def main():
    results = {}
    for path in sorted(glob.glob(os.path.join(SPEC, "M*.md"))):
        fn = os.path.basename(path)
        label = fn[:-3]
        rows = open(path, encoding="utf-8").read().split("\n")
        issues = []

        # frontmatter
        fm_blocks = None
        if rows and rows[0].strip() == "---":
            try:
                end = rows.index("---", 1)
                fm = "\n".join(rows[1:end])
                m = re.search(r'ext_blocks:\s*(\d+)', fm)
                if m: fm_blocks = int(m.group(1))
                else: issues.append("no ext_blocks in frontmatter")
            except ValueError:
                issues.append("unclosed frontmatter (no trailing ---)")

        blocks = own_blocks(rows, label)
        n = len(blocks)
        if fm_blocks is not None and fm_blocks != n:
            issues.append("frontmatter ext_blocks=%s != actual %d" % (fm_blocks, n))

        # ascending order
        nums = [b[2] for b in blocks]
        if nums != sorted(nums):
            issues.append("IDs not ascending: %s" % nums)

        # per-block deep layers + subsections
        REQ = {"Systems Touched","Math","How It Works","Reference Implementation","Player-Facing Impact"}
        sub_rx = re.compile(r'^#####\s+(.*?)\s*$')
        for idx, (b, bid, num) in enumerate(blocks):
            # find block end
            e = len(rows)
            for j in range(b+1, len(rows)):
                if re.match(r'^####\s+`?\[' + re.escape(label) + r'-EXT', rows[j]) or re.match(r'^##\s', rows[j]):
                    e = j; break
            seg = rows[b:e]
            # anchor must immediately precede header
            anchor_ok = (b > 0 and ('<a id="%s"' % bid) in rows[b-1])
            if not anchor_ok:
                issues.append("%s: missing/!preceding anchor" % bid)
            # injected layer order tags->tl;dr->meta->ctx
            inj = []
            for ln in seg[1:]:
                s = ln.strip()
                if s.startswith("> **tags**"): inj.append("tags")
                elif s.startswith("> **tl;dr**"): inj.append("tl;dr")
                elif s.startswith("> **meta**"): inj.append("meta")
                elif s.startswith("> **ctx**"): inj.append("ctx")
                elif s.startswith("### Cluster") or s.startswith("## "): break
                elif re.match(r'^####\s', ln): break
            if inj != ["tags","tl;dr","meta","ctx"]:
                issues.append("%s: layer order %s" % (bid, inj))
            # 5-subsection skeleton (skip STUB)
            is_stub = any("> **STUB**" in ln for ln in seg)
            if not is_stub:
                subs = set()
                for ln in seg:
                    mm = sub_rx.match(ln)
                    if mm: subs.add(mm.group(1).split(":")[0].strip())
                missing = REQ - subs
                if missing:
                    issues.append("%s: missing subsections %s" % (bid, sorted(missing)))

        # TOC
        if not any("<details>" in l and "block index" in l for l in rows[:60]):
            issues.append("no collapsible block-index TOC")
        # clusters (prose-only 0-block milestones are exempt — e.g. M2.6, M8.7)
        if n > 0:
            clusters = [l for l in rows if l.startswith("### Cluster")]
            if not clusters:
                issues.append("no ### Cluster dividers")

        # fences
        fc = sum(1 for l in rows if l.strip().startswith("```"))
        if fc % 2 != 0:
            issues.append("fences unbalanced (%d)" % fc)

        # JSON sidecar
        jp = path.replace(".md", ".index.json")
        if not os.path.exists(jp):
            issues.append("no JSON sidecar")
        else:
            try:
                data = json.load(open(jp, encoding="utf-8"))
                if len(data) != n:
                    issues.append("sidecar entries %d != blocks %d" % (len(data), n))
                for k in ("id","title","cluster","depends_on","depended_by","line","anchor"):
                    if any(k not in d for d in data):
                        issues.append("sidecar missing key '%s'" % k); break
            except Exception as ex:
                issues.append("sidecar parse error: %s" % ex)

        results[label] = (n, issues)

    print("%-9s | %3s | issues" % ("file","blk"))
    for label,(n,issues) in results.items():
        flag = "OK" if not issues else "FIX(%d)" % len(issues)
        print("%-9s | %3d | %s" % (label, n, flag))
        for it in issues:
            print("            - %s" % it)

if __name__ == "__main__":
    main()
