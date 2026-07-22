import json, glob, os, re, html, sys

OUT = os.path.join(os.path.dirname(__file__), 'transcripts')
files = sorted(glob.glob(os.path.join(OUT, 'yt_*.en.json3')))
print(f"converting {len(files)} json3 files -> .txt")
for f in files:
    with open(f, encoding='utf-8') as fh:
        data = json.load(fh)
    lines = []
    for ev in data.get('events', []):
        segs = ev.get('segs') or []
        txt = ''.join(s.get('utf8', '') for s in segs)
        txt = txt.replace('\n', ' ').strip()
        if txt:
            lines.append(txt)
    base = f[:-len('.en.json3')]
    out = base + '.txt'
    with open(out, 'w', encoding='utf-8') as fh:
        fh.write('\n'.join(lines))
    print(f"  {os.path.basename(out)}: {len(lines)} lines, {os.path.getsize(out)} bytes")
