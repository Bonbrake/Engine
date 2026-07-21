# ZombieEngine Developer Guides

This directory contains operational guides for building, debugging, testing,
profiling, and extending ZombieEngine. Every step below was verified against
the actual repo state on the `spec/m0-parity-reformat` branch.

## Build from Source

Prerequisites: Visual Studio 2022 Community with MSVC 14.44, Windows SDK
10.0.26100, Ninja, vcpkg in manifest mode. The canonical build entry point is
`scripts/build_ze.cmd`; it calls `vcvars64.bat`, sets the vcpkg toolchain, and
invokes CMake + Ninja. From the repo root:

```
cmd.exe /c scripts/build_ze.cmd
```

That produces `build/` by default. For an ASAN build:

```
cmd.exe /c scripts/build_ze.cmd build-asan
```

Both configurations have been verified green: 49 targets compile, tests pass,
and the headless smoke test exits cleanly. The ASAN preset is also wired into
`CMakePresets.json` as `windows-asan`.

## Debug

Enable Vulkan validation layers in Debug builds. The engine logs validation
warnings to the standard output. Use `focus_probe.exe` for quick renderer
smoke tests without launching the full engine. MSI Afterburner + RTSS can hook
Vulkan; because RTSS injects `STORAGE_BIT` into the swapchain, the renderer
uses `R8G8B8A8_UNORM` plus an explicit `STORAGE_BIT` request as a workaround
for validation error 01778.

## Tests

Test runner is `ZombieEngineTests.exe`, built with Catch2. From the build
directory:

```
cmd.exe /c tests\ZombieEngineTests.exe
```

Current baseline: 20 passed, 1 skipped, 83/83 assertions green on both Debug
and ASAN configurations. New tests belong in `tests/unit/Test_*.cpp` and are
auto-discovered by the Catch2 single-include registration in `tests/CMakeLists.txt`.

## Test runner and sanity gate

Test runner is `ZombieEngineTests.exe`, built with Catch2. From the build
directory:

```
cmd.exe /c tests\\ZombieEngineTests.exe
```

Current baseline: 20 passed, 1 skipped, 83/83 assertions green on both Debug
and ASAN configurations. New tests belong in `tests/unit/Test_*.cpp` and are
auto-discovered by the Catch2 single-include registration in `tests/CMakeLists.txt`.

### Standalone sanity suite

Before merge, run the standalone sandboxed verification suite from
`spec/APPENDICES.md §5.10`. It is a zero-dependency C++ harness compiled and
run independent of the engine, using local mock structs so it never collides
with canonical engine types. It validates WFC recovery, Saint-Venant flux
stencil, SPSC queue logic, cache-line alignment, and basic determinism. Use it
as a pre-merge gate before trusting a new version's appendix block behavior.

### Research cache

Use `scripts/fetch_research.py` to resolve remaining `[X]` blockers without
leaving the terminal. It caches fetches under your Hermes cache directory so
re-runs are offline-friendly. Example:

```
python scripts/fetch_research.py
```

It probes `search_arxiv("query")`, `fetch_arxiv_abstract("<id>")`, and
`fetch_doi_redirect("<doi>")`. Exact fetch evidence, including HTTP status and
byte counts, is what closes a blocker; do not mark a source verified from
memory alone.

## Profiling

Tracy is integrated into the engine. RTSS captures frame-time overlays; pair
it with Afterburner for GPU utilization and temperature telemetry during
profile runs. The 60 FPS frame budget is enforced at 1080p on the RTX 2070
SUPER with an 8 GB VRAM ceiling. Any new system that threatens that budget
must surface a Tracy profile before merge.

## Shaders

Shaders compile to SPIR-V via `glslc`. The build system caches compiled shaders
in `pipeline_cache.bin`; the headless smoke test confirms the cache is written
on shutdown. In Debug, watch for shader-compile timeouts: `M0-EXT-26` defines
the safe-mode boot fallback if compilation exceeds the watchdog threshold.
