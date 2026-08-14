# ZombieEngine — M0 Handoff (session state)

> This file is the single source of truth for resuming M0. It is written for a
> NEW agent session that has NO context from prior turns. Read it top-to-bottom.

## What this project is
**ZombieEngine** ("The Endless Quarantine") — an original C++/Vulkan 1.4
open-world zombie-survival engine, built from scratch. You are building the
**M0 milestone: Vulkan 1.4 native bootstrap + capability tiering**.

> **NOTE (2026-08-13):** The reconciled plan index is `C:\ZombieEngine\PLAN.md`.
> The v79 master spec at `C:\Users\jakeb\Desktop\ZombieEngine-WorkingFiles\ZombieEngine-Plan\USETHISITSV79.md`
> is the **historical origin** the milestones were split from — the **live working set** is
> `spec/` (e.g. `spec/M0.md` for the M0 span). The 50-item M0 plan is at
> `.hermes\plans\2026-07-24_M0-vulkan-bootstrap.md`.

## Hard rules (enforced on every step)
1. **Assume your own work is wrong.** Fact-check every Vulkan struct/feature/
   extension name against the ACTUAL SDK headers
   (`C:\VulkanSDK\1.4.350.0\Include\vulkan\*.hpp`) AND authoritative sources
   before compiling. Do not trust the doc's names — it has known bugs.
2. **One concern at a time.** Implement, build, run, verify — THEN move on.
   Do not batch unverified changes.
3. **Verify on real hardware.** Build + run `EndlessQuarantine.exe --headless`
   and assert `HEADLESS_BOOT_OK` + exit 0. Headed rendering needs a real
   desktop (no display in the agent env) — it degrades gracefully to exit 1.
4. **Commit as each phase verifies** (already done through C2).

## Environment (Windows 11)
- VS2022 Community, MSVC **19.44.35228** x64.
- Vulkan SDK **1.4.350** at `C:\VulkanSDK\1.4.350.0`.
- vcpkg packages (in `vcpkg_installed/x64-windows`): `sdl3`, `vulkan-memory-allocator`, `volk`, `enkiTS`, `spdlog`, `fmt`, `nlohmann-json`.
- CMake 4.3.4 + Ninja. Project enables **C and CXX** (volk.c is C).
- **Run all `cmd`/MSVC work through `scripts\\build_ze.cmd`** (canonical; `tools\\build_skeleton.bat` is broken — it `cd`s to a nonexistent path). Sets vcvarsall
  + VULKAN_SDK + VCPKG_INSTALLED_DIR + CMAKE_PREFIX_PATH, then cmake+Ninja).
- For a **headed** run, copy `vcpkg_installed/x64-windows/bin/*.dll` next to
  `build/EndlessQuarantine.exe` (SDL3.dll etc. must be on the path).

## Build / run / verify
```
cd C:\Users\jakeb\ZombieEngine
# from a git-bash terminal (NOT cmd), suppress noise with >/dev/null (NEVER >nul)
cmd /c "scripts\build_ze.cmd" >/dev/null 2>&1
cp -f vcpkg_installed/x64-windows/bin/*.dll build/ 2>/dev/null
cd build && cmd /c "EndlessQuarantine.exe --headless"   # expect HEADLESS_BOOT_OK, exit 0
```
Ad-hoc verify scripts go in `%TEMP%` (e.g. `hermes-verify-*.bat`), run, then delete.

## M0 progress (committed)
- **Phase A** (Tasks 1–7): repo + CMake + Vulkan SDK wiring. ✅
- **Phase B** (Tasks 8–14): logging, settings, CapabilityTier on real GPU. ✅
- **Phase C2** (Tasks 19–23): SDL3 window + surface + swapchain (explicit
  OUT_OF_DATE / SUBOPTIMAL handling) + dynamic-rendering clear pass. ✅
  Verified: headless boot exit 0 on **RTX 2070 SUPER**, Vulkan 1.4, tier 2,
  all capability bits resolve correctly.
- **Phase C3** (Tasks 24–27): pipeline cache, VMA allocator, robustness/headless
  extensions. ⬜ NOT STARTED.
- **Phase D** (Tasks 28–36): enkiTS loop, fixed-timestep MainLoop, frame pacing,
  adaptive quality, input, deterministic replay. ⬜
- **Phase E** (Tasks 37–44): assets, files, resilience. ⬜
- **Phase F** (Tasks 45–50): verification, determinism, headless CI. ⬜

## Key facts already discovered (don't re-derive)
- **Vulkan-Hpp + volk dispatcher** (issue #377 pattern): `VK_NO_PROTOTYPES` +
  `VULKAN_HPP_NO_EXCEPTIONS`; emit the default dynamic dispatcher STORAGE by
  invoking `VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE` in EXACTLY ONE
  TU (`VulkanContext.cpp`) — defining it in 2+ TUs = duplicate-symbol link error.
  Load the dispatcher from volk BEFORE any vk:: call:
  `vk::detail::defaultDispatchLoaderDynamic = vk::detail::DispatchLoaderDynamic(vkGetInstanceProcAddr);`
- **Doc naming bugs already corrected in code** (keep these fixes):
  `VK_EXT_SHADER_OBJECT_EXTENSION_NAME` (doc said KHR),
  `VK_EXT_MESH_SHADER_EXTENSION_NAME` (doc said KHR),
  feature member `fragmentShaderPixelInterlock` (not `fragmentShaderInterlock`),
  `attachmentFeedbackLoopLayout`, `opacityMicromap` member = `micromap`,
  `unifiedImageLayouts` is a KHR ext (NOT 1.4 core).
- **Windows gotchas fixed**: `CreateWindow` collides with the Win32 macro →
  renamed `CreateSdlWindow`; `NOMINMAX` before windows.h for `std::min/max/clamp`;
  `Shutdown()` made idempotent (was called twice → double device-destroy crash).
- **Instance/device extensions are probed** against
  `enumerate*ExtensionProperties` and only enabled if present (doc M0 item 5) —
  `VK_EXT_memory_budget` and `VK_EXT_debug_utils` are unavailable on this box and
  are skipped gracefully.

## Files of interest
- `src/engine/vulkan/VulkanContext.{h,cpp}` — instance/device/surface/swapchain,
  dispatcher wiring, OOD/SUBOPTIMAL acquire/present.
- `src/engine/vulkan/Renderer.{h,cpp}` — command pool + dynamic-rendering clear.
- `src/engine/core/CapabilityTier.{h,cpp}` — capability detection + tier assert.
- `src/engine/Engine.{h,cpp}` — facade + frame loop + CLI (`--headless/--headed/--dev/--force-tier0`).
- `src/CMakeLists.txt`, `CMakeLists.txt` — build (vcpkg, volk.c glob, VK_NO_PROTOTYPES).
- `tools/build_skeleton.bat`, `tools/rm_build.bat`, `tools/rm_nul.bat`.

## Next task = Phase C3 (Tasks 24–27)
Start with **Task 24 (pipeline cache)** then **25 (VMA)**, **26 (bindless gate)**,
**27 (Synchronization2 barrier helper)**. Read the plan section for each before
coding. Verify each on the headless boot + a cache-file round-trip.
