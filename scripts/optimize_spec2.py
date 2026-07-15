import re, glob, os
SPEC = r"C:\ZombieEngine\spec"
SKIP = {"_INDEX.md", "_v79_presplit.md", "llms.txt"}

# 1) Strip inline process-noise tags from headers/lines across all working files.
NOISE = re.compile(r'\s*\*?\((?:RECONSTRUCTED FROM CITATION CONTEXT[^)]*|gap-file ref:[^)]*)\)?\*?')
FENCE = re.compile(r'^\s*```')
HR = re.compile(r'^-{3,}$')

def clean(text):
    out=[]; in_fence=False
    for ln in text.split("\n"):
        if FENCE.match(ln):
            in_fence=not in_fence; out.append(ln); continue
        if in_fence:
            out.append(ln); continue
        ln = NOISE.sub("", ln).strip()
        if ln=="":
            # keep, collapse later
            out.append(ln); continue
        if HR.match(ln):
            continue
        out.append(ln)
    # collapse 3+ blank lines -> 2
    res=[]; b=0
    for ln in out:
        if ln=="":
            b+=1
            if b<=2: res.append(ln)
        else:
            b=0; res.append(ln)
    return "\n".join(res).strip()+"\n"

changed=0
for p in glob.glob(os.path.join(SPEC,"*.md")):
    fn=os.path.basename(p)
    if fn in SKIP: continue
    t=open(p,encoding="utf-8").read()
    n=clean(t)
    if n!=t:
        open(p,"w",encoding="utf-8").write(n); changed+=1
        print("cleaned %-16s -> %d bytes (was %d)"%(fn,len(n),len(t)))
print("files cleaned:",changed)

# 2) Build standards-compliant llms.txt (H1 -> blockquote summary -> H2 file lists).
EXTID = re.compile(r'^####\s+`?\[((?:M?\d[\d.]*|K|L)-EXT-\d+)\]`?\s*(.*)$')
sections = {
 "Milestones (core engine)": ["M0.md","M1.md","M2.md","M2.6.md","M2.7.md","M2.8.md","M2.9.md",
    "M3.md","M4.md","M4.5.md","M4.6.md","M5.md","M5.1.md","M5.2.md","M5.3.md","M5.4.md",
    "M6.md","M6.5.md","M7.md","M8.md","M8.5.md","M8.6.md","M8.7.md","M9.md","M10.md","M11.md","M12.md","M13.md"],
 "Gap-fill appendices (AAA parity)": ["APPENDIX_K.md","APPENDIX_L.md","APPENDIX_M.md"],
 "Reference & protocol": ["00_PROTOCOL.md","APPENDICES.md"],
}
# count blocks per file
blockcount={}
for p in glob.glob(os.path.join(SPEC,"*.md")):
    fn=os.path.basename(p)
    if fn in SKIP: continue
    c=sum(1 for l in open(p,encoding="utf-8").read().split("\n") if EXTID.match(l))
    if c: blockcount[fn]=c

L=[]
L.append("# ZombieEngine Master Specification")
L.append("")
L.append("> AI-facing index for the ZombieEngine spec (609 EXT design blocks, Vulkan 1.4 + 100% procedural + modding engine). Each file is self-contained: load ONLY the milestone you are working on. Every `#### [ID]` block is a complete, independently-retrievable unit (Systems Touched / Math / How It Works / Reference Implementation / Player-Facing Impact). Jump to any block via grep on this index, then open the one file. Full pre-split original preserved at `spec/_v79_presplit.md`.")
L.append("")
L.append("This index follows the llms.txt standard (llmstxt.org). Open files directly; do not load the whole corpus.")
L.append("")
for sec, files in sections.items():
    L.append("## %s"%sec)
    for fn in files:
        if fn in blockcount:
            L.append("- [%s](%s): milestone spec, %d EXT blocks"%(fn.replace('.md',''), fn, blockcount[fn]))
    L.append("")
open(os.path.join(SPEC,"llms.txt"),"w",encoding="utf-8").write("\n".join(L))
print("wrote llms.txt")
