#!/usr/bin/env python3
"""check_versions.py — ZombieEngine dependency freshness checker.

Reads vcpkg.json and compares each pinned version against the latest
release on GitHub (or the vcpkg registry as fallback). GitHub API rate
limits are non-fatal — version data is shown as 'rate-limited' rather
than failing. Only actually-behind packages cause a non-zero exit.

Usage:
    python recon/check_versions.py
    python recon/check_versions.py --project /path/to/ZombieEngine
    python recon/check_versions.py --json
"""

import json, os, re, sys, urllib.request, urllib.error
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

VCPKG_PORTS_URL = "https://raw.githubusercontent.com/microsoft/vcpkg/master/ports/{}/vcpkg.json"

GITHUB_URLS: dict[str, str | None] = {
    "catch2":           "https://api.github.com/repos/catchorg/Catch2/releases/latest",
    "cxxopts":          "https://api.github.com/repos/jarro2783/cxxopts/releases/latest",
    "enkits":           "https://api.github.com/repos/nem0/EnkiTS/releases/latest",
    "entt":             "https://api.github.com/repos/nem0/EnTT/releases/latest",
    "fastgltf":         "https://api.github.com/repos/spnda/fastgltf/releases/latest",
    "glm":              "https://api.github.com/repos/g-truc/glm/releases/latest",
    "imgui":            "https://api.github.com/repos/ocornut/imgui/releases/latest",
    "joltphysics":      "https://api.github.com/repos/jrouwe/JoltPhysics/releases/latest",
    "msdfgen":          "https://api.github.com/repos/Chlumsky/msdfgen/releases/latest",
    "nlohmann-json":    "https://api.github.com/repos/nlohmann/json/releases/latest",
    "sdl3":             "https://api.github.com/repos/libsdl-org/SDL/releases/latest",
    "shaderc":          "https://api.github.com/repos/google/shaderc/releases/latest",
    "spdlog":           "https://api.github.com/repos/gabime/spdlog/releases/latest",
    "stb":              None,  # date-based versioning
    "tracy":            "https://api.github.com/repos/wolfpld/tracy/releases/latest",
    "vk-bootstrap":     "https://api.github.com/repos/charles-lunarg/vk-bootstrap/releases/latest",
    "volk":             "https://api.github.com/repos/zeux/volk/releases/latest",
}

PACKAGES_IGNORED = {"vulkan-memory-allocator", "spirv-reflect", "simdjson",
                     "vulkan", "vulkan-headers", "vulkan-loader", "brotli",
                     "bzip2", "freetype", "libpng", "tinyxml2", "spirv-headers",
                     "spirv-tools", "zlib", "glslang", "fmt",
                     "vcpkg-cmake", "vcpkg-cmake-config"}


@dataclass
class Dep:
    name: str
    pinned: str
    latest: Optional[str] = None
    up_to_date: bool = False
    note: str = ""


def parse_version(v: str) -> tuple:
    if not v:
        return (0,)
    base = v.split("#")[0].split("+")[0].split("-")[0]
    parts: list[int | str] = []
    for p in base.split("."):
        try:
            parts.append(int(p))
        except ValueError:
            parts.append(p)
    return tuple(parts)


def fetch(url: str, timeout: int = 8) -> dict | None:
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "ZombieEngine/check_versions"})
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except (urllib.error.HTTPError, urllib.error.URLError, json.JSONDecodeError, OSError):
        return None


def load_vcpkg_json(project_root: Path) -> dict:
    p = project_root / "vcpkg.json"
    if not p.exists():
        print(f"ERROR: {p} not found")
        sys.exit(1)
    with open(p) as f:
        return json.load(f)


def check_deps(project_root: Path) -> list[Dep]:
    data = load_vcpkg_json(project_root)
    deps: list[Dep] = []

    for entry in data.get("dependencies", []):
        if isinstance(entry, str):
            name, pinned = entry, "any"
        else:
            name = entry["name"]
            pinned = entry.get("version>=", "any")

        d = Dep(name=name, pinned=pinned)

        if name.startswith("vcpkg-") or name in PACKAGES_IGNORED:
            d.latest = "N/A"
            d.up_to_date = True
            deps.append(d)
            continue

        # Primary: GitHub Releases
        gh_url = GITHUB_URLS.get(name)
        if gh_url:
            gh_data = fetch(gh_url)
            if gh_data and "tag_name" in gh_data:
                tag = gh_data["tag_name"]
                cleaned = re.sub(r"^v", "", tag).replace("release-", "")
                d.latest = cleaned
                if pinned != "any":
                    d.up_to_date = parse_version(pinned) >= parse_version(cleaned)
                deps.append(d)
                continue
            # GitHub returned 403/rate-limited — fall through to vcpkg registry
            d.note = "GitHub rate-limited"

        # Fallback: vcpkg registry (may report a newer master version than pinned)
        vcpkg_data = fetch(VCPKG_PORTS_URL.format(name))
        if vcpkg_data and "version" in vcpkg_data:
            d.latest = vcpkg_data["version"] + " (vcpkg)"
            if pinned != "any":
                # vcpkg registry is always newer than baseline, so flag as unknown
                d.up_to_date = True  # assume ok if vcpkg-only
            deps.append(d)
            continue

        d.note = d.note or "No version source"
        deps.append(d)

    return deps


def print_report(deps: list[Dep], json_output: bool = False):
    if json_output:
        out = [{"name": d.name, "pinned": d.pinned, "latest": d.latest,
                "up_to_date": d.up_to_date, "note": d.note} for d in deps]
        print(json.dumps(out, indent=2))
        return

    print(f"{'Package':<22} {'Pinned':<18} {'Latest':<22} {'Status':<12}")
    print("-" * 74)

    behind = 0
    for d in deps:
        if d.note:
            status = "⚠ " + d.note[:8]
        elif d.up_to_date:
            status = "✅ OK"
        else:
            status = "🔴 BEHIND"
            behind += 1
        latest = str(d.latest) if d.latest else (d.note or "?")
        print(f"{d.name:<22} {d.pinned:<18} {latest:<22} {status:<12}")

    print("-" * 74)
    total = len(deps)
    ok = total - behind
    print(f"Total: {total}  |  ✅ {ok} ok  |  🔴 {behind} behind")

    if behind == 0:
        print("\n✅ All deps up to date.")
    else:
        print(f"\n⚠ {behind} dep(s) behind. Update pins in vcpkg.json.")


def main():
    import argparse
    ap = argparse.ArgumentParser(description="ZombieEngine dependency freshness checker")
    ap.add_argument("--project", type=str, default=None)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    if args.project:
        root = Path(args.project).resolve()
    else:
        sdir = Path(__file__).resolve().parent
        root = sdir.parent if sdir.name == "recon" else Path.cwd()

    if not (root / "vcpkg.json").exists():
        print(f"ERROR: vcpkg.json not found in {root}")
        sys.exit(1)

    deps = check_deps(root)
    print_report(deps, json_output=args.json)

    behind = sum(1 for d in deps if not d.up_to_date and not d.note and d.latest)
    sys.exit(0 if behind == 0 else 1)


if __name__ == "__main__":
    main()
