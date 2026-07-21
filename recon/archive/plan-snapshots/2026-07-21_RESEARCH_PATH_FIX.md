# ZombieEngine: Research Path Fix And Source Tracking

## Purpose

This document makes online research reproducible on this machine. It names the
working free path, the known broken path, and the exact script to run.

## Current verified path

- Direct GitHub raw content: working
- Direct GitHub HTML pages: working
- Direct official sites that allow keyless fetch: working
- Direct Wikipedia HTML: blocked with 403 from this environment
- Brave Free via Hermes built-in web tools: blocked with 402 despite correct config

## Free unlimited workaround

Use the bundled retrieval script below. It does direct GitHub, direct official
docs, and keyless HTML extraction. No API key is required. Output goes to
the Hermes web cache.

## Retrieval script

Use `scripts/fetch_research.py` from the repo root. If needed, create it under
`C:\ZombieEngine\scripts\fetch_research.py` with this behavior:

```python
#!/usr/bin/env python3
"""Free offline-friendly research fetcher for ZombieEngine planning."""

from pathlib import Path
import urllib.request
import json
import re
import sys

CACHE = Path.home() / "AppData" / "Local" / "hermes" / "cache" / "web"
CACHE.mkdir(parents=True, exist_ok=True)

def clean(text: str) -> str:
    return re.sub(r'\s+', ' ', text)

def fetch(name, url, timeout=20):
    path = CACHE / f"{name}.html"
    if path.exists():
        return path.read_text(encoding="utf-8", errors="ignore")
    req = urllib.request.Request(url, headers={"User-Agent":"Mozilla/5.0"})
    data = urllib.request.urlopen(req, timeout=timeout).read().decode("utf-8", errors="ignore")
    path.write_text(data, encoding="utf-8")
    return data

SOURCES = {
    "gns": "https://github.com/ValveSoftware/GameNetworkingSockets",
    "imgui": "https://github.com/ocornut/imgui",
    "ue4ss": "https://github.com/UE4SS/UE4SS",
    "vulkan": "https://www.khronos.org/vulkan/",
    "ue5_docs": "https://dev.epicgames.com/documentation/en-us/unreal-engine/",
    "gdc_vault": "https://www.gdcvault.com/free/video",
}

if __name__ == "__main__":
    for name, url in SOURCES.items():
        print(f"Fetching {name}: {url}")
        txt = fetch(name, url)
        print(f"  Got {len(txt)} bytes")
```

## Verified source batch

| ID | Source | URL | Status |
|----|--------|-----|--------|
| S-01 | GameNetworkingSockets | https://github.com/ValveSoftware/GameNetworkingSockets | Fetched |
| S-02 | Dear ImGui | https://github.com/ocornut/imgui | Fetched |
| S-03 | UE4SS | https://github.com/UE4SS/UE4SS | Fetched |
| S-04 | Vulkan homepage | https://www.khronos.org/vulkan/ | Fetched |
| S-05 | UE5 docs | https://dev.epicgames.com/documentation/en-us/unreal-engine/ | Fetched |
| S-06 | GDC Vault free index | https://www.gdcvault.com/free/video | Fetched |
| S-07 | Game Developer blogs | https://www.gamasutra.com/blogs/ | Fetched |
| S-08 | Wikipedia Frostbite | https://en.wikipedia.org/wiki/Frostbite_(game_engine) | 403 blocked |
| S-09 | Wikipedia Decima | https://en.wikipedia.org/wiki/Decima_(game_engine) | 403 blocked |
| S-10 | Wikipedia CryEngine | https://en.wikipedia.org/wiki/CryEngine | 403 blocked |
| S-11 | Wikipedia Snowdrop | https://en.wikipedia.org/wiki/Snowdrop_(game_engine) | 403 blocked |
| S-12 | Wikipedia Anvil | https://en.wikipedia.org/wiki/Anvil_(game_engine) | 403 blocked |
| S-13 | DDG HTML search | https://html.duckduckgo.com/html/ | Returns 202 no-results page |
| S-14 | DDG JSON API | https://api.duckduckgo.com/ | Returns 0 Topics |
| S-15 | Brave Free search via Hermes | built-in backend | 402 from provider |

## Known issues

1. Brave-free returns 402; Hermes free-tools skill suggests `universal_free`,
   but the effective search provider still fails in this session.
2. Wikipedia blocks direct fetches with 403.
3. DDG search returns empty results in this environment.

Workaround: rely on direct fetches to GitHub + official docs + cached pages.

## Next actions

1. Run `scripts/fetch_research.py` at session start to refresh cache.
2. Re-run web_search if provider quota resets or config changes.
3. Add new source URLs to `SOURCES` in the script as they are verified.
