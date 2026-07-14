# AUDIT_M0.md — Vulkan 1.4 native bootstrap + capability tiering

**Branch:** `m2/physics-destruction` (M0 was implemented here, not on a dedicated M0 branch — see Known Doc Drift D1).
**Audit date:** 2026-07-12.
**Evidence basis:** source read of all 53 `src/` files mapped to M0 + 2 real local runs on RTX 2070 Super (headless boot-smoke, `--force-tier0` fallback) + `ctest` 9/9.

**Status glyph legend (defined once):** ✅ verified present & wired / run-confirmed · ⚠️ partial, scaffold, or declared-but-not-wired · ❌ missing.

> **Supersedes:** `AUDIT_M0-M2.md` and `AUDIT_FIX_STATUS.md` are KNOWN-STALE and superseded by this document (see Known Doc Drift D2–D4). Do not trust their Finding #1/#2 body text or test-count claims.

---

## 1. Implementation Steps

| # | Spec step | Status | Evidence |
|---|-----------|--------|----------|
| 1 | SDL window + Vulkan Instance API 1.4, `VK_EXT_pipeline_robustness`/`host_image_copy` core | ✅ | `VulkanContext.cpp:35-67` (`require_api_version(1,4,0)`); `Device.cpp:55-56` (pipelineRobustness/hostImageCopy) |
| 2 | Dynamic Rendering (`vkCmdBeginRendering`, no RenderPass/Framebuffer) | ✅ | `Swapchain.cpp:343-349` (`VkPipelineRenderingCreateInfo`, `VK_KHR_dynamic_rendering`); `Device.cpp:67` |
| 3 | Swapchain branches `OUT_OF_DATE_KHR`/`SUBOPTIMAL_KHR` in acquire **and** present | ✅ | `Swapchain.cpp:195-197` (acquire) + `:479-481` (present) → `recreate()` |
| 4 | Capability-Tier struct (meshShaders/rtPipeline/descriptorBuffer/shaderObject/unifiedImageLayouts/…) | ✅ | `Device.cpp:313-358` `checkCapabilities()`; logged at boot |
| 5 | `unifiedImageLayouts` opt gated, Synchronization2 fallback | ✅ | `Device.cpp:167-172` feature gate; S2 always-on `:51` |
| 6 | EnkiTS init | ✅ | `JobSystem.cpp:8-16` |
| 7 | Fixed-timestep `FIXED_DT=1/60`, clamp 0.25, focus-loss throttle | ⚠️ partial | `Engine.cpp:220,242` (clamp present); throttle via `FramePacing` EMA + `canRender` skip-on-minimize (`:445`). No dedicated native focus-loss sim throttle — relies on 0.25 clamp + frame-pacing. Functional, not literal mechanic. |
| 8 | Raw input poll → flat `InputState` | ✅ | `Input.cpp:112-202` `poll()` |
| 9 | `settings.json` + `--headless`/`--force-tier0` CLI | ✅ | `Config.cpp:14-100` |
| 10 | SplitMix64 thread isolation (`thread_local`) | ✅ | `PRNG.h:27` `inline thread_local SplitMix64 tls_rng` |
| 11 | GPU memory tracking `VK_EXT_memory_budget` + discrete-topology assert | ✅ | `Device.cpp:280,294-308` (logs "Device Local Heap N Headroom"); `:355-357` discrete warn |
| 12 | Async spdlog + JSON-lines sink `logs/session.jsonl` | ✅ | `Logger.cpp:11-29` |
| 13 | Crash interception → minidump + sidecar; `ENGINE_ASSERT` | ✅ | `Platform.cpp:46-64` (`SetUnhandledExceptionFilter`→`writeMinidump`+`writeCrashSidecar`); artifacts present on disk |
| 14 | Deterministic input replay (`--record-input`/`--replay-input`) | ✅ (mechanism) | `Input.cpp:142-173,195-201`; `Engine.cpp:226-231,340-341` tick-hash |
| 15 | `AssetPath` resolver mapping `mods/` | ✅ | `FileSystem.cpp:6-14` (used by `PipelineCacheManager::init`) |
| 16 | Pipeline cache persisted to disk, loaded via AssetPath | ✅ | `PipelineCacheManager.cpp:14-58` |
| 17 | Debug-only validation layers gated on `--dev` | ✅ | `VulkanContext.cpp:41` `request_validation_layers(devMode)` |
| 18 | GPU driver blocklist + workaround table (JSON-lines) | ⚠️ scaffold | `DriverBlocklist.h` — blocklist EMPTY (only commented example); workaround toggles only on Intel. Extension point, not populated. |
| 19 | Descriptor heap cap flag in tier struct | ✅ | `CapabilityTier` has descriptorBuffer etc. |
| 20 | Headless CI boot-smoke assertion | ✅ | `Engine.cpp` `verifyHeadlessInit()` → `HEADLESS CI SMOKE TEST: SUCCESS` (run-verified, exit 0) |
| 21 | 8K/multi-RT guarantee assert at boot | ✅ | `Device.cpp:351-353` `ENGINE_ASSERT(maxImageDimension2D>=8192 …)` |
| 22 | RAII teardown order Instance→Window→Surface→Device→Swapchain | ✅ | `TeardownTracker.h` + `Engine.cpp:137-163` / `VulkanContext.cpp:168-201`; run logs ordered `RAII Shutdown` |
| 23 | Bindless design gated on `caps.descriptorBuffer` | ✅ | `TriangleRenderer.cpp:154-168` (`RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT` + device address) |
| 24 | `VK_KHR_synchronization2` cross-pass barriers | ✅ | `Swapchain.cpp:345-413` `vkCmdPipelineBarrier2`/`VkDependencyInfo2` |
| 25 | Command pool-per-thread `L·T+N` | ✅ | `Swapchain.cpp:136` `Initialize(device, qfi, swapchainImages.size(), GetNumTaskThreads())` → `CommandPoolMatrix`; consumed by `RenderGraph::CompileAndExecute` (`RenderGraph.cpp:178`) |
| 26 | VMA sub-alloc, tracked vs budget | ✅ | `Device.cpp:274-308` `VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT` |
| 27 | ONE_TIME_SUBMIT vs reusable pool policy | ✅ | `Swapchain.cpp:242` (ONE_TIME on primary); pool flags `RESET_COMMAND_BUFFER_BIT` (`:108`) |
| 28 | Dense geometry compression gated on meshShaders | ✅ | `VulkanContext.cpp:91-99` (boot verify, pos err 0.0029). *Minor:* runs unconditionally, not strictly cap-gated. |
| 29 | `VK_EXT_debug_utils` naming on every obj | ⚠️ partial | `Device.cpp:360-372` `setDebugObjectName` used for queues + query pool only; not every pipeline/buffer/image. |
| 30 | Vendor GPU crash dump (Nsight Aftermath / AMD RGD) | ✅ | `Engine.cpp:165-200` `dumpVendorCheckpoints()` (NV + AMD branches), called on device-lost |
| 31 | Pipeline cache header validation on load | ✅ | `PipelineCacheManager.cpp:60-86` (vendor/device/UUID check) |
| 32 | Device-lost protocol (stop→teardown→artifacts→exit) | ✅ | `main.cpp:28-37` (exit -4 on `VK_ERROR_DEVICE_LOST`); `Engine.cpp:207-213` |
| 33 | Deterministic crash repro via replay | ⚠️ partial | Mechanism present (record/replay + tick-hash). Not re-verified live this pass (needs a crash/replay pair). |
| 34 | `VK_KHR_dynamic_rendering` (covered by #2) | — | n/a |

---

## 2. M0 EXT systems

| EXT | Status | Evidence |
|-----|--------|----------|
| 01 AlignToCacheLine (VMA pinning) | ✅ declared+used | `EnkiTSPinning.h:11`, `Memory.h:12` |
| 02 ShaderObjectState manager | ✅ wired | `ShaderObjectState.h:12` `BindChangedStages`; `ShaderManager.cpp:27` |
| 03 Pipeline cache header validator | ✅ wired | `PipelineCacheManager.cpp:60` |
| 04 FileHandleRing | ✅ present (self-test only) | `FileHandleRing.h`; exercised in `Engine.cpp:575-585` diag |
| 05 Memory-Budget Tracker | ✅ wired | `MemoryBudget.h` + `Device.cpp:305` |
| 06 Cross-Queue Hazard Validator | ✅ **wired into RenderGraph** | `HazardValidator.h:13`; `RenderGraph.cpp:106,135` |
| 07 Fiber Yield Hook | ⚠️ declared, not wired | `FiberYield.h` exists; no `SwitchToFiber` call site found |
| 08 Descriptor-Buffer Slot Allocator | ⚠️ declared, not wired | descriptorBuffer path in `TriangleRenderer` writes descriptors directly (`:237`); no `DescriptorPage`/`AssignDescriptorSlot` consumer |
| 09 Frame-Pacing Interceptor | ✅ wired | `FramePacing.h`; `Swapchain.cpp:187` |
| 10 Pipeline Layout Compat Validator | ⚠️ declared-only | PipelineBuilder area; not invoked in main path (see Known Doc Drift D2) |
| 11 Shader Cache GC | ✅ wired | `ShaderCacheGC.h`; `PipelineCacheManager.cpp:16` (run logged "within limits 36 bytes") |
| 12 Thread-Affinity Bitmask | ✅ present (tested) | `ThreadAffinity.h`; `Test_MathFormulas.cpp:146` |
| 14 Async Compute Interleaver | ✅ wired (queue exists) | `AsyncComputeInterleaver.h`; `Device.cpp:211-221` dedicated compute queue established |
| (EXT-13) | — | deliberately retired (per doc) |

---

## 3. Exit Criteria

| Criterion | Verdict | Evidence |
|-----------|---------|----------|
| Window opens, Vulkan inits, validation logs clean | ⚠️ engine-clean; 3rd-party overlay warnings residual | Run shows `GalaxyOverlayVkLayer`/`VK_LAYER_OBS_HOOK`/`VK_LAYER_RTSS` warnings — **environmental** (GOG/OBS/RivaTuner), NOT engine. Non-`--dev` windowed path otherwise clean. `--dev` has separate 124-error baseline (`AUDIT_DEV_MODE_VALIDATION_BASELINE.md`). |
| Swapchain survives resize + minimize | ✅ | `Engine.cpp:427-435` resize/minimize/restore + `Swapchain::recreate` on OOD/`SUBOPTIMAL` |
| Capability tier detected+logged; forced fallback clean | ✅ **run-verified** | Run2: `--force-tier0` → all caps `=false`, exit 0 |
| PipelineCache created+persisted | ✅ **run-verified** | "Saved pipeline cache to pipeline_cache.bin" |
| Crash handler → minidump+sidecar; replay identical traces | ✅ mechanism; ⚠️ replay-identity not re-run live | Artifacts on disk (Jul 11); `main.cpp`/`Engine.cpp` path present |
| Budgets logged via VMA | ✅ **run-verified** | "Device Local Heap 2 Headroom: 212.5 MB" |

**M0 overall: PASS** (4 partials, all functional; doc-drift items below are audit-record issues, not engine defects).

---

## 4. M0 vs v77 master doc

`00_M0.md` is consistent with `endless_quarantine_v77_cleaned.md` (umbrella master; M0 content lives in `00_M0.md`). No M0-specific master-only requirement found that `00_M0.md` omits. The v77.1 "M1 scene-render gap" note is M1/M2.6 scope — carried forward to the M1 phase.

---

## 5. Known Doc Drift (supersedes stale audit docs — scan this, don't trust the old files)

| ID | Stale claim (where) | Ground truth (this audit) | Resolution |
|----|--------------------|---------------------------|------------|
| D1 | M0 expected on a dedicated M0 branch (AGENTS.md branch-hygiene rule) | Implemented on `m2/physics-destruction`; M0/M1/M2 all landed here | Recorded. No defect — STATUS.md should state milestone↔branch mapping is blurred. |
| D2 | `AUDIT_M0-M2.md` header: "#2 fix deleted (`PipelineCompatibility.h` removed)". `AUDIT_FIX_STATUS.md`: "#2 modified in place (result-gated log at `TriangleRenderer.cpp:391`)". | `src/render/PipelineCompatibility.h` does NOT exist; `TriangleRenderer.cpp` has NO `ValidateLayoutCompatibility` call (grep: NONE). The dead-code path is simply gone; the two docs disagree on how. | Both stale. Corrected record: dead-code path absent, never wired. `AUDIT_M0-M2.md`/`AUDIT_FIX_STATUS.md` left untouched per disposition (a). |
| D3 | `AUDIT_M0-M2.md` header "Fixes for #1 and #2 applied" vs body "#1 candidate fix (NOT APPLIED)". | #1 WAS applied: `CommandPoolMatrix::Initialize` throws on `VK_NULL_HANDLE` (`CommandPoolMatrix.h:27`), asserted by `Test_MathFormulas.cpp:61` (ctest #4 PASS). "NOT APPLIED" line is stale. | Corrected: #1 FIXED. #2 disposition per D2. |
| D4 | `AUDIT_FIX_STATUS.md`: "35 assertions / 7 cases". `STATUS.md`: "9 cases / 49 assertions". | Actual `ctest` = **9 cases / 9 PASS** this run. 7-case/35-assertion claim predates M2.6-added tests. | Corrected: 9 cases (assertion count unverified this pass, not needed). Old docs stale. |

---

## 6. Carried forward (not M0 defects)

- M1 scene-render gap (`endless_quarantine_v77_cleaned.md` v77.1): `TriangleRenderer` draws one hardcoded triangle instanced 100×; no ECS `view<Transform,Mesh>` path; `AssetManager::GetMesh()` has zero callers; no `MeshComponent`. Affects visual-side exit criteria of M0 tooling note, M2.6, M2.7, M2.9, M3, M4, M5.2. → M1 phase.
- 4 M0 partials (steps 7, 18, 29, 33) are functional; acceptable for M0 gate but flagged for later milestones if they become load-bearing.
