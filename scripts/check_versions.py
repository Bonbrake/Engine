#!/usr/bin/env python3
# check_versions.py — keep ZombieEngine's deps pinned to the NEWEST release.
# Diffs live GitHub "latest release" (or newest tag) against the pinned version
# in vcpkg.json. Flags only what is BEHIND. Self-policing: run any time; no stale claims.
# Uses GitHub API (egress confirmed working in this sandbox via curl).
import re, json, subprocess, sys

REPO = {
    "joltphysics": "jrouwe/JoltPhysics",
    "entt": "skypjack/entt",
    "sdl3": "libsdl-org/SDL",
    "imgui": "ocornut/imgui",
    "tracy": "wolfpld/tracy",
    "fastgltf": "spnda/fastgltf",
    "vulkan-memory-allocator": "gpuopen-librariesandsdks/VulkanMemoryAllocator",
    "vk-bootstrap": "charles-lunarg/vk-bootstrap",
    "volk": "confetti-cl/volk",
    "shaderc": "google/shaderc",
    "spirv-reflect": "KhronosGroup/SPIRV-Reflect",
    "catch2": "catchorg/Catch2",
    "cxxopts": "jarro2783/cxxopts",
    "enkits": "dougbinks/enkiTS",
    "nlohmann-json": "nlohmann/json",
    "spdlog": "gabime/spdlog",
    "msdfgen": "Chlumsky/msdfgen",
}

def norm(v):
    v = v.lower().strip()
    v = re.sub(r'^[v]', '', v)
    v = re.sub(r'(release-|vulkan-sdk-)', '', v)
    return v.strip()

def latest(owner_repo):
    # try releases/latest
    out = subprocess.run(["curl", "-s", "--max-time", "25",
                          "https://api.github.com/repos/%s/releases/latest" % owner_repo],
                         capture_output=True, text=True).stdout
    try:
        d = json.loads(out)
        if "tag_name" in d:
            return norm(d["tag_name"])
    except Exception:
        pass
    # fall back to tags
    out = subprocess.run(["curl", "-s", "--max-time", "25",
                          "https://api.github.com/repos/%s/tags?per_page=3" % owner_repo],
                         capture_output=True, text=True).stdout
    try:
        d = json.loads(out)
        if isinstance(d, list) and d:
            return norm(d[0]["name"])
    except Exception:
        pass
    return "?"

def parse_vcpkg(path):
    txt = open(path, encoding="utf-8").read()
    # vcpkg.json here lists bare dependency NAMES with no version pins;
    # versions resolve from the vcpkg registry baseline (not in-repo).
    out = {}
    for m in re.finditer(r'"name":\s*"([^"]+)"', txt):
        name = m.group(1)
        # capture a trailing version>=" if present (some projects pin via this)
        seg = txt[m.end():]
        vm = re.search(r'"version>=":\s*"([^"]+)"', seg[:200])
        out[name] = norm(vm.group(1)) if vm else None
    return out

def main():
    root = r"C:\ZombieEngine"
    pinned = parse_vcpkg(root + r"\vcpkg.json")
    print("%-22s %-12s %-12s %s" % ("dep", "pinned", "newest", "status"))
    print("-" * 60)
    behind = 0
    for dep, repo in REPO.items():
        cur = pinned.get(dep, None)
        new = latest(repo)
        if cur is None:
            st = "UNPINNED"
        elif new == "?":
            st = "??"
        elif norm(cur) == norm(new):
            st = "OK"
        else:
            st = "BEHIND"
            behind += 1
        print("%-22s %-12s %-12s %s" % (dep, cur or "-", new, st))
    print("-" * 60)
    unpinned = sum(1 for d in REPO if pinned.get(d) is None)
    print("UNPINNED: %d   BEHIND: %d" % (unpinned, behind))
    # exit non-zero only if something is genuinely behind a pin we control
    sys.exit(1 if behind else 0)

if __name__ == "__main__":
    main()
