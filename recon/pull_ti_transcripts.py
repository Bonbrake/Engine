#!/usr/bin/env python
"""
Standard Threat-Interactive transcript puller.
METHOD (per user instruction): youtube-transcript-api (NO KEY) is the primary.
Fallback: yt-dlp (also NO KEY) if the API is IP-blocked (YouTube 429/hot-IP).

YouTube caption endpoints IP-sign their URLs and rate-limit heavy egress (HTTP 429).
If the whole egress IP is blocked, BOTH methods fail until the cooldown passes
(typically minutes->an hour). This script paces requests and retries with backoff,
so just re-run it (or let the background run finish) once YouTube lifts the block.

Usage: python pull_ti_transcripts.py
Pulls every video id in ti_all_vids.txt -> transcripts_api/<VID>.txt (skips existing).
"""
import subprocess, os, sys, json, time
from youtube_transcript_api import YouTubeTranscriptApi

HERE = os.path.dirname(os.path.abspath(__file__))
# Write directly into transcripts/ with the yt_ prefix that analyze_transcripts.py
# reads (it globs recon/transcripts/yt_*.txt). Previously wrote to transcripts_api/
# which analyze never saw — a silent pipeline seam. Existing yt_*.txt are skipped.
OUT = os.path.join(HERE, "transcripts")
os.makedirs(OUT, exist_ok=True)

ids = [l.strip() for l in open(os.path.join(HERE, "ti_all_vids.txt"), encoding="utf-8") if l.strip()]
ids = [v for v in ids if not os.path.exists(os.path.join(OUT, f"yt_{v}.txt"))]
print(f"remaining to fetch: {len(ids)}")

def via_api(v):
    t = YouTubeTranscriptApi().fetch(v, languages=['en'])
    snips = getattr(t, 'snippets', [])
    return " ".join(s.text for s in snips)

def via_ytdlp(v):
    subprocess.run([sys.executable, "-m", "yt_dlp", "--skip-download",
        "--write-auto-subs", "--sub-langs", "en", "--sub-format", "json3",
        "--quiet", "-o", os.path.join(OUT, f"yt_{v}"),
        f"https://www.youtube.com/watch?v={v}"], check=True, capture_output=True, timeout=180)
    jf = os.path.join(OUT, f"yt_{v}.en.json3")
    if not os.path.exists(jf):
        return None
    data = json.load(open(jf, encoding="utf-8"))
    lines = [("".join(s.get('utf8', '') for s in (ev.get('segs') or []))).replace("\n", " ").strip()
             for ev in data.get("events", [])]
    text = "\n".join(l for l in lines if l)
    os.remove(jf)
    return text

def main():
    done = 0
    for v in ids:
        text = None
        for attempt in range(6):
            try:
                text = via_api(v)
                break
            except Exception as e:
                err = type(e).__name__
                if err == "IpBlocked":
                    time.sleep(20 * (attempt + 1))  # backoff
                    continue
                # try yt-dlp fallback once per attempt
                try:
                    text = via_ytdlp(v)
                    break
                except Exception:
                    time.sleep(10)
                    continue
        if text:
            open(os.path.join(OUT, f"yt_{v}.txt"), "w", encoding="utf-8").write(text)
            done += 1
            print(f"  OK {v} ({len(text)} chars)")
        else:
            print(f"  FAIL {v} (ip-blocked/cooldown needed)")
        time.sleep(3)
    print(f"DONE this run: {done}/{len(ids)}")

if __name__ == "__main__":
    main()
