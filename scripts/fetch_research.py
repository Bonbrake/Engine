#!/usr/bin/env python3
"""Free offline-friendly research fetcher for ZombieEngine planning."""

from pathlib import Path
import urllib.request
import re

CACHE = Path.home() / "AppData" / "Local" / "hermes" / "cache" / "web"
CACHE.mkdir(parents=True, exist_ok=True)


def fetch(name: str, url: str, timeout: int = 20) -> str:
    path = CACHE / f"{name}.html"
    if path.exists():
        return path.read_text(encoding="utf-8", errors="ignore")
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    data = urllib.request.urlopen(req, timeout=timeout).read().decode("utf-8", errors="ignore")
    path.write_text(data, encoding="utf-8")
    return data


def main() -> int:
    sources = {
        "gns": "https://github.com/ValveSoftware/GameNetworkingSockets",
        "imgui": "https://github.com/ocornut/imgui",
        "ue4ss": "https://github.com/UE4SS/UE4SS",
        "vulkan": "https://www.khronos.org/vulkan/",
        "ue5_docs": "https://dev.epicgames.com/documentation/en-us/unreal-engine/",
        "gdc_vault": "https://www.gdcvault.com/free/video",
        "gamedev_blogs": "https://www.gamasutra.com/blogs/",
    }
    for name, url in sources.items():
        print(f"Fetching {name}: {url}")
        try:
            txt = fetch(name, url)
            print(f"  Got {len(txt)} bytes")
        except Exception as e:
            print(f"  ERR: {e}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
