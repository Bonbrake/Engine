#!/usr/bin/env python3
"""Free offline-friendly research fetcher for ZombieEngine planning."""

from pathlib import Path
import urllib.parse
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
    
    # Test new helpers
    print("\n=== Testing new research helpers ===")
    try:
        abstract = fetch_arxiv_abstract("2109.06780")
        print(f"  arxiv_abstract got {len(abstract)} bytes")
    except Exception as e:
        print(f"  arxiv_abstract ERR: {e}")
    
    try:
        search_html = search_arxiv("vehicle physics game")
        print(f"  arxiv_search got {len(search_html)} bytes")
    except Exception as e:
        print(f"  arxiv_search ERR: {e}")
    
    try:
        doi_html = fetch_doi_redirect("10.1145/2793107.2793120")
        print(f"  doi_redirect got {len(doi_html)} bytes")
    except Exception as e:
        print(f"  doi_redirect ERR: {e}")
    
    return 0




def fetch_arxiv_abstract(paper_id: str, timeout: int = 20) -> str:
    path = CACHE / f"arxiv_{paper_id}.html"
    if path.exists():
        return path.read_text(encoding="utf-8", errors="ignore")
    url = f"https://arxiv.org/abs/{paper_id}"
    req = urllib.request.Request(url, headers={"User-Agent": "ZombieEngine-research/1.0"})
    data = urllib.request.urlopen(req, timeout=timeout).read().decode("utf-8", errors="ignore")
    path.write_text(data, encoding="utf-8")
    return data


def search_arxiv(query: str, timeout: int = 20) -> str:
    """Search arXiv and cache the HTML results page."""
    slug = re.sub(r"[^a-z0-9]+", "_", query.lower()).strip("_")[:80]
    path = CACHE / f"arxiv_search_{slug}.html"
    if path.exists():
        return path.read_text(encoding="utf-8", errors="ignore")
    url = f"https://arxiv.org/search/?searchtype=all&query={urllib.parse.quote(query)}"
    req = urllib.request.Request(url, headers={"User-Agent": "ZombieEngine-research/1.0"})
    data = urllib.request.urlopen(req, timeout=timeout).read().decode("utf-8", errors="ignore")
    path.write_text(data, encoding="utf-8")
    return data


def fetch_doi_redirect(doi: str, timeout: int = 20) -> str:
    path = CACHE / f"doi_{doi.replace('/', '_')}.html"
    if path.exists():
        return path.read_text(encoding="utf-8", errors="ignore")
    url = f"https://doi.org/{doi}"
    req = urllib.request.Request(url, headers={
        "User-Agent": "ZombieEngine-research/1.0",
        "Accept": "text/html"
    })
    try:
        data = urllib.request.urlopen(req, timeout=timeout).read().decode("utf-8", errors="ignore")
        path.write_text(data, encoding="utf-8")
        return data
    except Exception as e:
        return f"ERROR: {e}"

if __name__ == "__main__":
    raise SystemExit(main())
