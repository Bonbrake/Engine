import re, os, shutil, collections

SRC = r"C:\ZombieEngine\MASTER_PLAN_ENDLESS_QUARANTINE.md"
OUT = r"C:\ZombieEngine\spec"
os.makedirs(OUT, exist_ok=True)

# backup original
shutil.copy2(SRC, os.path.join(OUT, "_v79_presplit.md"))

rows = open(SRC, encoding="utf-8").read().split("\n")
N = len(rows)

EXTID = re.compile(r'^(#{2,6})\s+`?\[((?:M?\d[\d.]*|K|L)-EXT-\d+)\]`?')
H2 = re.compile(r'^##\s+(.*)$')
MILE = re.compile(r'^##\s+(M\d+(?:\.\d+)?)\s+—')
APPEND_TAIL = re.compile(r'^##\s+Milestone\s+(M[\d.]+)\s+—\s+Appendix EXT additions')
APPK = re.compile(r'^##\s+Appendix K')
APPL = re.compile(r'^##\s+Appendix L')
APPM = re.compile(r'^##\s+Appendix M')
NOISE = re.compile(r'^\*?\(?(?:gap-file ref|RECONSTRUCTED FROM CITATION)')
FENCE = re.compile(r'^```')

# 1) Split into top-level ## sections
sections = []  # (header_text, start_idx, end_idx)
i = 0
while i < N:
    if H2.match(rows[i]) and not EXTID.match(rows[i]):
        s = i
        j = i + 1
        while j < N and not (H2.match(rows[j]) and not EXTID.match(rows[j])):
            j += 1
        sections.append((rows[s], s, j))
        i = j
    else:
        i += 1

first_mile_idx = next((s for h, s, e in sections if MILE.match(h)), None)

# 2) Route sections into per-file chunk lists
files = collections.defaultdict(list)  # filename -> [text chunks]

def norm_block_text(lines):
    """Normalize a chunk of lines: EXT headers -> H4, strip noise, blank hygiene."""
    out = []
    for ln in lines:
        if NOISE.search(ln.strip()):
            continue
        m = EXTID.match(ln)
        if m and len(m.group(1)) in (2, 3):  # ## or ### EXT header -> ####
            ln = "#### " + ln[m.end(1):]  # keep ' [ID] Title'
            # ensure '#### ' prefix
            ln = re.sub(r'^#{2,6}\s+', '#### ', ln)
        out.append(ln)
    # collapse 3+ blank lines
    res = []
    blank = 0
    for ln in out:
        if ln.strip() == "":
            blank += 1
            if blank <= 2:
                res.append(ln)
        else:
            blank = 0
            res.append(ln)
    return res

def milestone_file(label):
    return "%s.md" % label  # label already starts with 'M' (e.g. M0, M2.7)

for h, s, e in sections:
    body = rows[s:e]
    if MILE.match(h):
        label = MILE.match(h).group(1)
        files[milestone_file(label)].append("\n".join(body))
    elif APPEND_TAIL.match(h):
        label = APPEND_TAIL.match(h).group(1)
        # extract only EXT blocks, normalize, drop wrapper
        blk = norm_block_text(body)
        files[milestone_file(label)].append(
            "\n\n## %s — Appendix EXT additions\n\n" % label + "\n".join(blk))
    elif EXTID.match(h):
        # standalone EXT section: ## [Mx-EXT-NN] Title
        m = EXTID.match(h)
        prefix = re.match(r'(M?\d[\d.]*)', m.group(2)).group(1)
        blk = norm_block_text(body)
        files[milestone_file(prefix)].append("\n".join(blk))
    elif APPK.match(h):
        files["APPENDIX_K.md"].append("\n".join(body))
    elif APPL.match(h):
        files["APPENDIX_L.md"].append("\n".join(body))
    elif APPM.match(h):
        files["APPENDIX_M.md"].append("\n".join(body))
    elif s < first_mile_idx:
        files["00_PROTOCOL.md"].append("\n".join(body))
    else:
        files["APPENDICES.md"].append("\n".join(body))

# 3) Write files
written = {}
for fn, chunks in files.items():
    text = "\n\n".join(chunks).strip() + "\n"
    # final normalize pass for EXT headers in case any slipped
    text = re.sub(r'(?m)^#{2,3}\s+(\[(?:M?\d[\d.]*|K|L)-EXT-\d+\])', r'#### \1', text)
    text = re.sub(r'(?m)^(?:\*?\(?(?:gap-file ref|RECONSTRUCTED FROM CITATION)[^\n]*\n)+', '', text)
    p = os.path.join(OUT, fn)
    open(p, "w", encoding="utf-8").write(text)
    written[fn] = len(text.split("\n"))

# 4) Build _INDEX.md by scanning all written files for EXT ids
idx = collections.defaultdict(list)
for fn in sorted(written):
    p = os.path.join(OUT, fn)
    for n, ln in enumerate(open(p, encoding="utf-8").read().split("\n"), 1):
        m = EXTID.match(ln)
        if m:
            idx[m.group(2)].append((fn, n))

lines = ["# ZombieEngine EXT Index", "",
         "> 609 EXT blocks across %d files. Each `ID | file | line`." % len(written), ""]
by_ms = collections.defaultdict(list)
for idv, locs in idx.items():
    by_ms[idv.split("-EXT")[0]].append((idv, locs[0]))
for ms in sorted(by_ms):
    lines.append("## %s" % ms)
    for idv, (fn, n) in sorted(by_ms[ms], key=lambda x: int(re.search(r'(\d+)$', x[0]).group(1))):
        lines.append("- `%s` → `%s` (L%d)" % (idv, fn, n))
    lines.append("")
open(os.path.join(OUT, "_INDEX.md"), "w", encoding="utf-8").write("\n".join(lines))

# 5) Verification summary
all_ids = list(idx.keys())
print("=== SPLIT COMPLETE ===")
print("output dir:", OUT)
print("files written: %d" % len(written))
for fn in sorted(written):
    print("  %-18s %5d lines" % (fn, written[fn]))
print("INDEX entries (unique EXT ids):", len(all_ids))
dups = [k for k, v in idx.items() if len(v) > 1]
print("duplicate IDs across files:", len(dups), dups[:10])
print("total EXT-id header occurrences:", sum(len(v) for v in idx.values()))
