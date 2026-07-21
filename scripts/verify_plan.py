#!/usr/bin/env python3
"""Verify the professional plan and project state against actual files."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
plan_path = ROOT / "recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md"
exec_map_path = ROOT / "recon/plans/APPENDIX_EXECUTION_MAP.md"
cmake_path = ROOT / "CMakeLists.txt"
arch_path = ROOT / "docs/architecture/README.md"
guides_path = ROOT / "docs/guides/README.md"
fetch_path = ROOT / "scripts/fetch_research.py"
sanity_path = ROOT / "sanity_suite_harness/sanity.cpp"
cache = Path.home() / "AppData/Local/hermes/cache/web"

def read(p):
    return p.read_text(encoding="utf-8", errors="ignore") if p.exists() else ""

plan = read(plan_path)
exec_map = read(exec_map_path)
cmake = read(cmake_path)
arch = read(arch_path)
guides = read(guides_path)
fetch = read(fetch_path)
sanity = read(sanity_path)

checks = []

def require(label, condition):
    status = "PASS" if condition else "FAIL"
    checks.append((label, status))
    return condition

# Section presence
require("1. Pass 4 section exists", "## A. Pass 4 — task-to-paper mapping audit" in plan)
require("2. Pass 5 section exists", "## B. Pass 5 — EXT ID collision audit" in plan)
require("3. Sanity section exists", "## C. Standalone sanity suite result" in plan)
require("4. GSound section exists", "## D. Remaining [X] blocker fetch evidence" in plan)
require("5. 5-pass status exists", "## E. 5-pass P5 status" in plan)

# Paper rows fixed
require("6. Paper 26 corrected to 2023", "Breaking Good: Fracture Modes for Realtime Destruction | 2023 | [cache: crossref_10.1145_3549540.json]" in plan)
require("7. Paper 35 uses arxiv_1804.02717.html", "arxiv_1804.02717.html" in plan)

# Case study downgrades
require("8. UE5 marked [X]", "UE5 Nanite + Lumen — Epic Games [X]" in plan)
require("9. Frostbite marked [X]", "Frostbite — DICE / EA [X]" in plan)
require("10. Unity DOTS marked [X]", "Unity DOTS + HDRP — Unity Technologies [X]" in plan)

# Syntax integrity
require("11. No ||| malformed rows", "|||" not in plan)
require("12. T-70 present", "| T-70 |" in plan)

# Execution map
require("13. Exec map has evidence table", "| ID | Title | Readiness | Evidence | Blocker | Source |" in exec_map)
require("14. Exec map notes 51 IDs", "51 unique provisional EXT IDs" in exec_map)

# Build wiring
require("15. CMake has sanity_suite target", "add_executable(sanity_suite" in cmake)
require("16. CMake has tests enabled", "enable_testing()" in cmake)

# Documentation
require("17. Arch README has research cache", "## Research cache" in arch)
require("18. Guides README has research cache", "### Research cache" in guides)

# Scripts
require("19. fetch_research.py has fetchers", "def fetch_vehicle_sources()" in fetch and "def fetch_gsound_sources()" in fetch)

# Artifacts
require("20. sanity_suite.exe exists", (ROOT / "build-ninja/sanity_suite.exe").exists())

passed = sum(1 for _, s in checks if s == "PASS")
failed = [(l, s) for l, s in checks if s == "FAIL"]

print(f"VERIFY_PLAN: {passed}/{len(checks)} checks passed")
for label, status in checks:
    print(f"  [{status}] {label}")

if failed:
    print("\nFAILED:")
    for label, _ in failed:
        print(f"  - {label}")
    sys.exit(1)
else:
    print("\nALL CHECKS PASSED")
    sys.exit(0)
