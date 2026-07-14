import io, re, os

ROOT = r"C:\ZombieEngine\milestones_M0-M13_antigravity"
HEAD = re.compile(r"^####\s+\[([A-Z0-9.]+-EXT-\d+)\]", re.M)
ADDED = ["M1-EXT-18","M1-EXT-19","M1-EXT-20","M1-EXT-21","M1-EXT-22","M1-EXT-23",
         "M1-EXT-24","M1-EXT-25","M1-EXT-26","M4-EXT-22","M4.5-EXT-25"]
fof = {"M1-EXT-18":"01_M1.md","M1-EXT-19":"01_M1.md","M1-EXT-20":"01_M1.md","M1-EXT-21":"01_M1.md",
       "M1-EXT-22":"01_M1.md","M1-EXT-23":"01_M1.md","M1-EXT-24":"01_M1.md","M1-EXT-25":"01_M1.md",
       "M1-EXT-26":"01_M1.md","M4-EXT-22":"08_M4.md","M4.5-EXT-25":"09_M4_5.md"}

print("=== VERIFY: per-ID heading count in owning file (expect exactly 1 each) ===")
per = {}
for fn in os.listdir(ROOT):
    if not fn.endswith(".md"): continue
    for mid in HEAD.findall(io.open(os.path.join(ROOT,fn),"r",encoding="utf-8",newline="").read()):
        per.setdefault(mid,{}).setdefault(fn,0); per[mid][fn]+=1

allok = True
for tid in ADDED:
    c = per.get(tid,{}).get(fof[tid],0)
    flag = "OK" if c==1 else "FAIL"
    if c!=1: allok=False
    print(f"  {tid:14s} in {fof[tid]:12s} defs={c} [{flag}]")

print(f"\n=== VERIFY: duplicate heading definitions across ALL {len(per)} distinct IDs ===")
dups=[f"{i}@{f}x{c}" for i,m in per.items() for f,c in m.items() if c>1]
print("  duplicates:", dups if dups else "NONE (0)")
if dups: allok=False

print("\n=== VERIFY: required subsections present per added ID ===")
for tid in ADDED:
    t=io.open(os.path.join(ROOT,fof[tid]),"r",encoding="utf-8",newline="").read()
    m=re.search(r"^####\s+\["+re.escape(tid)+r"\]",t,re.M)
    blk=t[m.end():]
    nxt=re.search(r"^####\s+\[",blk,re.M); blk=blk[:nxt.start() if nxt else len(blk)]
    miss=[s for s in ["Systems Touched","How It Works","Player-Facing Impact"] if f"##### {s}" not in blk]
    print(f"  {tid:14s} missing={miss if miss else 'none'} [{'OK' if not miss else 'FAIL'}]")
    if miss: allok=False

print("\n=== VERIFY: M1 occlusion base-step bullet annotated (not deleted) ===")
m1=io.open(os.path.join(ROOT,"01_M1.md"),"r",encoding="utf-8",newline="").read()
present = "Occlusion query double-buffering for Hi-Z culling" in m1
annot   = "superseded by `[M4.5-EXT-08]`" in m1
print(f"  present={present} annotated={annot} [{'OK' if present and annot else 'FAIL'}]")
if not (present and annot): allok=False

print("\n=== VERIFY: M3-EXT-10 intact (prior batch no regression) ===")
m3ok = bool(re.search(r"^####\s+\[M3-EXT-10\]", io.open(os.path.join(ROOT,"07_M3.md"),"r",encoding="utf-8",newline="").read(), re.M))
print(f"  M3-EXT-10 present={m3ok} [{'OK' if m3ok else 'FAIL'}]")
if not m3ok: allok=False

print("\n=== VERIFY: EXT-26 conditional (compaction grep of source) ===")
src=io.open(r"C:\ZombieEngine\src\render\TriangleRenderer.cpp","r",encoding="utf-8",errors="ignore").read()
di = "vkCmdDrawIndexedIndirectCount" in src
fb = "vkCmdFillBuffer" in src
print(f"  DrawIndexedIndirectCount={di} FillBuffer={fb} -> EXT-26 skip={'correct' if di and fb else 'WRONG'}")
if not (di and fb): allok=False

print("\n=== RESULT ===")
print("PASS" if allok else "FAIL")
