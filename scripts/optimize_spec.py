import re, glob, os
SPEC = r"C:\ZombieEngine\spec"
SKIP = {"_INDEX.md", "_v79_presplit.md"}
NOISE = re.compile(r'^\*?\(?(?:RECONSTRUCTED FROM CITATION CONTEXT[^)]*|gap-file ref:[^)]*)\)?\*?$')
FENCE = re.compile(r'^\s*```')
HR = re.compile(r'^-{3,}$')

def optimize(text):
    out = []
    in_fence = False
    for ln in text.split("\n"):
        if FENCE.match(ln):
            in_fence = not in_fence
            out.append(ln); continue
        if in_fence:
            out.append(ln); continue
        if NOISE.match(ln.strip()):
            continue
        if HR.match(ln.strip()):
            continue
        out.append(ln)
    # collapse 3+ blank lines -> 2 (keep paragraph gap, drop book-paper whitespace)
    merged = []
    blank = 0
    for ln in out:
        if ln.strip() == "":
            blank += 1
            if blank <= 2:
                merged.append(ln)
        else:
            blank = 0
            merged.append(ln)
    return "\n".join(merged).strip() + "\n"

for p in glob.glob(os.path.join(SPEC, "*.md")):
    fn = os.path.basename(p)
    if fn in SKIP:
        continue
    text = open(p, encoding="utf-8").read()
    new = optimize(text)
    if new != text:
        open(p, "w", encoding="utf-8").write(new)
        print("optimized %-16s %6d -> %6d bytes" % (fn, len(text), len(new)))

# Rebuild _INDEX.md WITH titles (fast topic lookup, no file open needed)
EXTID = re.compile(r'^####\s+`?\[((?:M?\d[\d.]*|K|L)-EXT-\d+)\]`?\s*(.*)$')
by_ms = {}
for p in glob.glob(os.path.join(SPEC, "*.md")):
    fn = os.path.basename(p)
    if fn in SKIP:
        continue
    for n, ln in enumerate(open(p, encoding="utf-8").read().split("\n"), 1):
        m = EXTID.match(ln)
        if m:
            idv = m.group(1)
            title = m.group(2).strip() or "(untitled)"
            by_ms.setdefault(idv.split("-EXT")[0], []).append((idv, title, fn, n))
lines = ["# ZombieEngine EXT Index", "",
         "> 609 EXT blocks. Each entry: `ID` Title → file (L#). Grep this file to locate a topic, then open the one file.",
         ""]
for ms in sorted(by_ms):
    lines.append("## %s" % ms)
    for idv, title, fn, n in sorted(by_ms[ms], key=lambda x: int(re.search(r'(\d+)$', x[0]).group(1))):
        t = title if len(title) <= 70 else title[:67] + "..."
        lines.append("- `%s` %s → `%s` (L%d)" % (idv, t, fn, n))
    lines.append("")
open(os.path.join(SPEC, "_INDEX.md"), "w", encoding="utf-8").write("\n".join(lines))
print("rebuilt _INDEX.md with titles")
