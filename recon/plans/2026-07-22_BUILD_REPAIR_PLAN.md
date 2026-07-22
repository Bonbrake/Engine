# ZombieEngine Build Repair & Deep Fix Plan
Date: 2026-07-22
Status: COMPREHENSIVE PRE-EXECUTION PLAN

## 1. Current State Summary
- Spec system: 813 EXT blocks, 26 milestones, 0 dangling refs, all indexes valid.
- CMake: 22 subsystem targets, all now real OBJECT libs; root links all.
- Headers: 63 public headers under include/ze/*/, stubbed subsystems complete.
- Build: Compiles start, but fails on existing code in src/.
- Research: 28 [X] papers identified; arXiv metadata confirmed for 13; additional web research completed for 5 key sources.
- Env: VS 2022 Community installed; vcvarsall present; Windows SDK 10.0.26100 present; vcpkg deps installed.

## 2. Error Taxonomy From Latest Build
### A. Environment / Toolchain
1. `cl.exe` not found when PATH lacks VC tools.
2. `cstdint/stdarg.h/stddef.h/vcruntime.h` not found when INCLUDE/LIB lack SDK/VC paths.
3. SDL3 includes resolve only after explicit Windows Kit + VC include paths are set.

### B. Missing Subsystem Headers / Stubs
4. `ze/audio/AudioEngine.h` etc. missing.
5. `ze/net/NetworkManager.h` etc. missing.
6. `ze/save/SaveSystem.h` etc. missing.
7. `ze/slm/SLMClient.h` etc. missing.
8. `ze/world/BiomeGraph.h` etc. missing.
9. `ze/events/EventBus.h` stub empty, invisible to existing code.
10. `ze/modding/Modding.h` stub empty.
11. `ze/ui/HUD.h`, `HapticManager.h`, `MSDFText.h` missing.
12. `ze/combat/...`, `ze/survival/...`, `ze/vehicle/...` headers present as stubs but include paths were broken.

### C. Existing Code Compile Bugs
13. `PhysicsSystem::step(entt::registry&, entt::dispatcher&)` exists in header, but callers pass `eventBus_->raw()` / `bus->raw()`. The EventBus type does not expose `raw()` returning `entt::dispatcher&`. **Root cause: API mismatch between Engine.cpp and EventBus stub/header.**
14. `Engine::eventBus_->enqueue(dmg)` suggests EventBus should accept `ecs::DamageEvent` directly. Existing code expects event bus to mirror EnTT dispatcher semantics.
15. `Destructible.cpp` references `events::EventBus` before full type is visible — forward declaration is insufficient because code uses member functions or complete type.

### D. Renderer / Fast-Math / Determinism
16. 14 files use `#pragma` fast-math/optimize; violates determinism rule for lockstep.
17. 4 files use `std::unordered_map<float,...>`; potential float-ordering nondeterminism.
18. Renderer misses `VK_EXT_graphics_pipeline_libraries` and `dynamic_rendering` on paths using `vkCreateGraphicsPipelines`.

## 3. Planned Fix Order
1. Fix environment: update build.ps1 to always export VC + Windows Kit include/lib paths before invoking Ninja/CMake.
2. Complete all missing headers with minimal APIs so existing code compiles.
3. Fix EventBus header/API to match Engine.cpp and PhysicsSystem.cpp usage patterns:
   - Provide `enqueue(...)` forwarding to internal `entt::dispatcher`.
   - Provide `raw()` returning `entt::dispatcher&`.
   - Provide `flush()` that emits queued events to dispatcher.
4. Fix `PhysicsSystem::step` call sites to pass the dispatcher directly, or provide expected wrappers.
5. Fix `Engine::physicsTick()` and dev-test paths to use the corrected API.
6. Fix UI/world stubs to include their own subsystem headers.
7. Replace fast-math pragmas with deterministic equivalents; document exceptions.
8. Add `std::unordered_map<float,...>` static-analysis markers or convert to ordered maps where determinism matters.
9. Rebuild and iterate until `ZombieEngine` target links clean.
10. Continue paper research/sourcing in parallel with build fixes.

## 4. Research-Backed Decisions
- Vulkan: prefer pipeline libraries + dynamic state only where benchmarked; do not preemptively refactor renderer until M4/M8.
- Determinism: lockstep needs no float-ordering dependencies; any `unordered_map<float>` must be replaced or hash-quantized.
- Audio: M6 merge map stays in plan only; no spec changes until M6 implementation starts.

## 5. Verification Gates
- Gate 1: build.ps1 initializes INCLUDE/LIB/PATH successfully.
- Gate 2: `scripts/verify_ext_block_counts.py` still reports 0 warnings.
- Gate 3: CMake configure succeeds.
- Gate 4: Ninja build completes all 31 targets without fatal errors.
- Gate 5: Static analysis/pragma-fastmath scan passes.
- Gate 6: Research paper count drops from 28 [X] to documented cache-backed state.
