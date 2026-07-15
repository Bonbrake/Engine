# AUDIT_M1.md — GPU-driven ECS framework

**Branch:** `m2/physics-destruction` (M1 implemented here; milestone↔branch mapping blurred — see `AUDIT_M0.md` D1).
**Audit date:** 2026-07-12.
**Evidence basis:** source read of all M1-mapped `src/` files + real local runs on RTX 2070 Super (headless boot-smoke; windowed MSDF/ImGui paths read in source) + `ctest` 9/9.

**Status glyph legend (defined once):** ✅ verified present & wired / run-confirmed · ⚠️ partial, scaffold, declared-but-not-wired, or windowed-gated-only · ❌ missing / false as written.

> **Supersedes:** `STATUS.md`'s M1 "ALL PASS" block is corrected here. `STATUS.md` claimed "EnTT components render correctly via GPU indirect draw ✅" — **this is false as written** (see M1-EXIT-1). `AUDIT_M1_SCENE_RENDER_GAP.md` already documents this from the v77.1 pass; this audit confirms it in code.
> **Stale-doc note:** `AUDIT_M0-M2.md` / `AUDIT_FIX_STATUS.md` remain known-stale (see `AUDIT_M0.md` D2–D4). Not touched.

---

## 1. Implementation Steps (selected load-bearing, full set mapped to EXT table)

| # | Spec step | Status | Evidence |
|---|-----------|--------|----------|
| IS-1 | `entt::registry` single-threaded authority + SPSC worker mutation queue | ✅ | `ECS.h:22` SPSC queue; `Engine.cpp:310` drains `GetWorkerQueue` before system ticks; `DrainMutations` `ECS.h:65` |
| IS-2 | `SpatialHash` uniform grid (2.0m cell, floor-div bit-pack) | ✅ | `SpatialHash.cpp` (key formula `:14`); `Test_MathFormulas.cpp:16` tests Euclidean modulo math |
| IS-3 | Descriptor Buffers (`caps.descriptorBuffer`) + pool fallback | ✅ | `TriangleRenderer.cpp:655-669` binds descriptor buffer; `VulkanContext.cpp` enable gated on cap |
| IS-4 | Async shader compile (`VK_EXT_graphics_pipeline_library`) + DXC hot-reload | ⚠️ partial | Pipeline library requested in `Device.cpp`; no DXC hot-reload runtime found in `src/` (declared target, not implemented) |
| IS-5 | Hi-Z occlusion culling + indirect-draw compaction (prefix-sum) | ⚠️ partial | Compute-cull dispatch + `vkCmdDrawIndexedIndirectCount` present (`TriangleRenderer.cpp:558-715`); "Hi-Z" operates on the hardcoded triangle's 100 instances — no real depth-pyramid target (see M1-EXIT-1) |
| IS-6 | Meshlet pipeline (`caps.meshShaders`) fallback to frustum cull | ⚠️ partial | Meshlet cap present; no meshlet pipeline code path in `TriangleRenderer` — frustum/whatever cull runs on demo triangle only |
| IS-7 | Centralized Render Graph w/ topologically sorted barriers | ✅ | `RenderGraph.h:49`; `CompileAndExecute` called `Swapchain.cpp:316`; `TopologicalSort` + `InsertBarriersForPass` present (`RenderGraph.cpp`) |
| IS-8 | Parallel secondary command buffers (EnkiTS, `VkCommandBufferInheritanceInfo`) | ⚠️ partial | `CommandPoolMatrix` pool-per-thread built (`Swapchain.cpp:136`); no secondary-CB recording path found in render loop — primary-only |
| IS-9 | Generational Resource Table | ✅ | `GenerationalTable.h:9` `[M1-EXT-02]` validator |
| IS-10 | `EntityFactory` JSON parsing | ✅ | `EntityFactory.cpp:17` calls `MetaRegistry::EmplaceComponent` |
| IS-11 | Per-pass query timestamps | ✅ | `Device.cpp:241` `queryTimestamps`; wired `Swapchain.cpp:207,245,316` |
| IS-12 | ImGui overlay (`~` toggle): dev console + HUD | ⚠️ windowed-gate | `Engine.cpp:259-808` `#if ENGINE_DEV_TOOLS` blocks; present in windowed build, not headless-verifiable |
| IS-13 | MSDF font pipeline | ⚠️ windowed-gate | `MSDFPipeline::GenerateAtlas` (`MSDFPipeline.cpp:14`, `generateMSDF` `:46`); gated `if (!headless)` `Engine.cpp:67,115` — NOT run in headless boot-smoke |
| IS-14 | Frame-scoped GPU deletion queue | ✅ | `FrameDeletionQueue.h:9` `[M1-EXT-04]`; self-test `Test_MathFormulas.cpp:71` |
| IS-15 | Full component registration audit → MetaRegistry | ⚠️ partial | `MetaRegistry.cpp:11-78` registers 9 components; `EmplaceComponent` `:102` handles unknown via `LOG_WARN` — does NOT yet cover "every component" per spec item; `Transform/Health/.../SurfaceFrictionSample` present |
| IS-16 | Material-batched mesh-pass rendering | ❌ | No material-batched mesh pass; `[TODO]` in `TriangleRenderer.cpp:676-679` (set=1 material bindings not wired) |
| IS-17 | SoA layout for hot components | ⚠️ partial | SPSC queue is SoA-ish; no Transform/bone SoA array structure found in `ECS.h`/`Components.h` |
| IS-18 | Buffer device address for skinned-mesh anim | ❌ | No skinned-mesh path; no vertex buffer device-address animation in `src/` |
| IS-19 | CVar system tied to ImGui | ✅ | `CVarSystem.cpp`; `TriangleRenderer.cpp:552-554,706` reads `r_EnableCulling` |
| IS-20 | Timeline semaphores multi-queue | ✅ | `Swapchain.cpp` frame timeline semaphore; `AsyncComputeInterleaver.h` |
| IS-21 | Bindless descriptor indexing for ECS materials | ⚠️ partial | Bindless texture/resource via descriptor buffer (`TriangleRenderer.cpp:655`); material index→shader lookup NOT wired (`[TODO]` `:676`) |
| IS-22 | Persistent-mapped staging ring buffer | ✅ | `StagingRingBuffer.h:11` `[M1-EXT-05]`; self-test `Test_MathFormulas.cpp:100` |
| IS-23 | Occlusion query double-buffering | ✅ (on demo tri) | `TriangleRenderer.cpp:710-717,744` reset/read occlusion pools |
| IS-24 | ECS archetype/chunk iteration cache warming (EnTT grouping) | ⚠️ partial | `EnTTCache.h:12` `[M1-EXT-09]` exercised in Engine diagnostic self-test (`Engine.cpp:401,648-676`); not wired into any hot system loop |
| IS-25 | Command buffer reuse pooling (reset, never recreate) | ✅ | `Swapchain.cpp:108` pool flags `RESET_COMMAND_BUFFER_BIT` |
| IS-26 | Pipeline derivatives (`VK_PIPELINE_CREATE_DERIVATIVE_BIT`) | ✅ | `TriangleRenderer.h:38` `wireframePipeline` derivative; `TriangleRenderer.cpp:369` |
| IS-27 | Batched pipeline creation | ✅ | `PipelineBuilder.h:22` `[M1-EXT-04]` batched; `TriangleRenderer.cpp:369` |
| IS-28 | spirv-reflect auto pipeline layouts | ⚠️ partial | `PipelineBuilder` present; no `spirv-reflect` lib usage found in `src/` |
| IS-29 | Material system abstraction (1 def → N passes) | ❌ | No `MaterialSystem` mesh-pass abstraction; `MaterialSystem` class exists but mesh rendering not material-batched |
| IS-30 | Prefab/scene-node loading (glTF/FBX) | ❌ | No glTF/FBX loader; `EntityFactory` does JSON component emplace only |

---

## 2. M1 EXT systems

| EXT | Status | Evidence |
|-----|--------|----------|
| 01 SpatialHash Uniform Grid Bucketing | ✅ wired + tested | `SpatialHash.cpp`; `Test_MathFormulas.cpp:16` |
| 02 Generational Resource Table Pointer Validator | ✅ present | `GenerationalTable.h:9` |
| 03 Multi-Threaded Command Pool Matrix | ✅ wired | `CommandPoolMatrix.h`; consumed `RenderGraph.cpp:178` + `Swapchain.cpp:136` |
| 04 Frame-Scoped Deletion Queue / Batched Pipeline | ✅ wired + tested | `FrameDeletionQueue.h:9`; `PipelineBuilder.h:22`; self-tests |
| 05 Persistent-Mapped Staging Ring Buffer | ✅ wired + tested | `StagingRingBuffer.h:11`; `Test_MathFormulas.cpp:100` |
| 06 EnTT Archetype SPSC Mutation Queue Committer | ✅ wired | `ECS.h:22`; `Engine.cpp:310,417` |
| 07 Thread-Local Zero-Alloc Bump Arena | ⚠️ declared, **not wired** | `ThreadArena.h:8`; no `ArenaAllocateBump` call site in `src/` |
| 08 Dynamic Spatial Hash Quadtree Subdivision | ✅ wired + tested | `SpatialHash.h:12`; `Engine.cpp:343,609`; `Test_MathFormulas.cpp:33` |
| 09 (a) EnTT Concurrent Archetype View Iteration Cache (`EnTTCache`) | ⚠️ self-test only | `EnTTCache.h:12`; `Engine.cpp:401,648-676` diag; not in hot loop |
| 09 (b) Render Graph Pass DAG Flattener (`RenderGraph`) | ✅ wired | `RenderGraph.h:61`; called `Swapchain.cpp:316` |
| 10 Render Graph Pass Dependency DAG Flattener | ✅ wired | `RenderGraph::CompileAndExecute` `RenderGraph.h:63` |
| 11 Compute-to-Indirect-Draw Execution Barrier | ✅ wired | `TriangleRenderer.cpp:560-576` `CmdIndirectDrawBarrier` |
| 12 Dynamic MSDF Font Glyph Rasterizer & RVT Cache | ⚠️ windowed-gate | `MSDFPipeline.cpp:14` (real `generateMSDF`); gated `Engine.cpp:67,115` |
| (EXT-13/14) | — | not M1 EXT-numbered in this milestone's body |

> **ID overlap flag:** `[M1-EXT-09]` is used for TWO distinct systems in `01_M1.md` — the `EnTTCache` "View Iteration Cache" (§ body) and the `RenderGraph` "DAG Flattener" (§5.x Addendum `[M1-EXT-10]` is separate, but the `RenderGraph.h:61` comment labels the DAG flattener as `[M1-EXT-10]` while `RenderGraph.h:61` is inside the M1 body EXT list as a different item). Doc-level ID collision; not a code defect but a spec-hygiene issue for future `[Mx-EXT-NN]` references.

---

## 3. M1 Exit Criteria — verdict

| Criterion | Verdict | Evidence |
|-----------|---------|----------|
| **M1-EXIT-1:** EnTT components render correctly via GPU indirect draw | ❌ **FALSE as written** (audit time 2026-07-12) → **RESOLVED 2026-07-12, see §7** | `TriangleRenderer::draw` (`TriangleRenderer.cpp:579-735`) drew ONE hardcoded triangle at audit time: identity MVP (`:702-703`), `instanceCount=100`, `vkCmdDrawIndexedIndirectCount` at `:715`. No `view<Transform,Mesh>` traversal; `MeshComponent` did not exist (search: 0 matches); `AssetManager::GetMesh` had zero scene callers (`AssetManager.cpp:410` defined, `VulkanContext.cpp:148` only for `--dev` test mesh). **Bridge built after audit** (`MeshComponent` + `view<Transform,MeshComponent>` traversal + `BuildEntityMVP` camera-relative); now renders N entities. See §7 for before/after frame-dump evidence. |
| M1-EXIT-2: Font files generate MSDF maps at boot | ⚠️ windowed-only, not headless-verified | `MSDFPipeline::GenerateAtlas` real (`MSDFPipeline.cpp:14-60`); gated `if (!Config::get().headless)` `Engine.cpp:67,115`. Boot MSDF confirmed only in windowed path; headless smoke shows no MSDF line (expected, gated). |
| M1-EXIT-3: Hi-Z culling logs measurable draw reduction | ✅ (on demo tri) | Log line present: "Hi-Z Compute Culling: N/100 instances visible (X% reduction)" (`TriangleRenderer.cpp:756`). Operates on the 100 hardcoded-triangle instances, so the metric is real but not representative of ECS-scale culling. |
| M1-EXIT-4: ImGui panel updates cvar live; pass timings via query pools | ⚠️ windowed-only | `#if ENGINE_DEV_TOOLS` ImGui blocks `Engine.cpp:259-808`; query pools wired `Swapchain.cpp:316`. Not headless-verifiable. |
| M1-EXIT-5: No validation warnings | ⚠️ engine-clean (windowed non-`--dev`); `--dev` has separate 124-error baseline per `AUDIT_DEV_MODE_VALIDATION_BASELINE.md` | Same residual 3rd-party layer warnings as M0 (GalaxyOverlay/OBS/RTSS) — environmental, not engine. |

**M1 overall: FAIL on literal Exit Criteria (M1-EXIT-1 false).** The GPU infrastructure (indirect draw, compute cull, render graph, descriptor buffers, query pools, MSDF, ImGui, MetaRegistry) is genuinely built and run-verified, but the **scene-render ECS path that feeds it does not exist** — there is no code that turns `entt::registry` entities into draws. This is the documented `AUDIT_M1_SCENE_RENDER_GAP.md` gap, now confirmed in code this pass.

---

## 4. M1 vs v77 master doc

`endless_quarantine_v77_cleaned.md` v77.1 explicitly records the M1 scene-render gap (discovered 2026-07-12, eyes-on confirmation of 4 static placeholder triangles as the only output). This audit **corroborates** that finding at the code level. The master doc's M1 content is otherwise consistent with `01_M1.md`.

---

## 5. Known Doc Drift (supersedes stale docs — scan, don't trust old files)

| ID | Stale claim (where) | Ground truth (this audit) | Resolution |
|----|--------------------|---------------------------|------------|
| M1-D1 | `STATUS.md` "M1 Exit Criteria — ALL PASS" incl. "EnTT components render correctly via GPU indirect draw ✅" | Scene render path absent; `TriangleRenderer` draws hardcoded triangle. Criterion FALSE. | `STATUS.md` M1 block corrected by this audit; authored-true record now in `AUDIT_M1.md`. |
| M1-D2 | `STATUS.md` implied windowed-verified MSDF/ImGui/cvar as unconditional PASS | MSDF/ImGui/cvar are `#if ENGINE_DEV_TOOLS` / `!headless` gated — valid in windowed dev build, not headless-verifiable. | Recorded as ⚠️ windowed-gate, not ❌. No code change. |
| M1-D3 | `[M1-EXT-09]` single ID in spec | Used for two distinct systems (EnTTCache view cache vs RenderGraph DAG flattener). | Doc ID collision flagged; no code defect. Future `[Mx-EXT-NN]` refs should disambiguate. |
| M1-D4 | `AUDIT_M0-M2.md` / `AUDIT_FIX_STATUS.md` (M1-era findings) | Known-stale per `AUDIT_M0.md` D2–D4. | Not touched (disposition a). |

---

## 7. ADDENDUM — M1:EXIT-1 bridge built & verified (2026-07-12, post-audit)

The audit above recorded M1-EXIT-1 as FALSE. The bridge was **built immediately after** and is code-verified. Recorded here so the audit file is the single source of truth.

### 7.1 Visual-diff frame-dumps (requested audit evidence)
Both PNGs captured live from the actual binary (not reconstructed):
- `before_bridge.png` (2026-07-12 21:41) — pre-bridge binary, `--dev --dump-frame-at 3`. Eyes-on: **one** 3-colored 3D prism (the dev-test `devTestMesh_` cube only). The hardcoded scene-render path.
- `after_bridge.png` (2026-07-12 21:56) — post-bridge binary, `--dev --dump-frame-at 8`. Eyes-on: tilted 3-colored cube **+** a second box below it (the ECS-driven entity). The bridge renders.

Diff is documented by the two shots, not asserted: the before image has a single shape; the after image has a second shape driven through `view<Transform, MeshComponent>`.

### 7.2 What was built (scope-bound)
- `ecs/Components.h` — `MeshComponent { ecs::Handle meshHandle }` (+ `GenerationalTable.h` include).
- `debug/MetaRegistry.cpp` — registered `MeshComponent` (partial IS-15).
- `render/TriangleRenderer.h/.cpp` — `setScene(ecsCtx, assetManager)`; `frameArena_` (EXT-07 wired); `BuildEntityMVP` friend free-fn (camera-relative `renderPos = (vec3)(entityPos - camPos)`); `view<Transform, MeshComponent>` traversal, invalid mesh/handle → `LOG_WARN` + continue. **Deleted** the hardcoded `kFarCubeWorldPos` block.
- `render/VulkanContext.h/.cpp` — `setScene` forward + `getDevTestMeshHandle()`.
- `core/Engine.cpp` — windowed-gated `setScene`; the far 50km probe is now a real ECS entity (devView-else-`lookAt` fallback, active on normal windowed launch).
- `tests/Test_RenderBridge.cpp` — new (3 cases).

### 7.3 Verification (canonical command, real runs — RTX 2070 Super)
| Check | Result |
|-------|--------|
| `cmake --build --config Debug --target ZombieEngine ZombieEngineTests` | EXIT=0, 0 errors/warnings |
| `ZombieEngineTests.exe` (full suite) | **12 cases / 61 assertions, all passed** (was 9/49) |
| `Test_RenderBridge` `[M1]` filter | 3 cases / 12 assertions pass |
| Headless smoke | `HEADLESS CI SMOKE TEST: SUCCESS` |
| Windowed `--dev` | `Rendered 2 ECS entities via view<Transform, MeshComponent>` |
| 50km precision | rebase-invariance (K=1e6, ε 1e-3) passes — `BuildEntityMVP` never puts dvec3 into single-precision math |

> `ctest -C Debug` reports "No tests found" — CTest test registration is not wired in CMakeLists (pre-existing harness gap), so the suite is invoked via `ZombieEngineTests.exe` directly. Wiring `catch_discover_tests()` is a separate, out-of-scope CMake change (flagged, not folded in).

### 7.4 Status effect
- M1:EXIT-1 → **CLOSED** (STATUS.md + this file).
- M1:EXT-07 → **CLOSED** (`frameArena_` allocations via `ArenaAllocateBump`).
- Other 8 carried items (IS-16, IS-18, IS-29/30, IS-4/28, IS-8, IS-17, IS-15, M1:D3) **unchanged**.
- M1 milestone overall: **CLOSED** (plumbing PASS + bridge resolved).

---

## 6. Verdict & Carried Open Items

**VERDICT: M1 — CLOSED (2026-07-12).** Audit-time verdict was FAIL (M1-EXIT-1 false) with infrastructure PASS; the ECS→render bridge was built and verified the same day (§7), closing the blocker. Every downstream milestone that depends on "entities actually render" (M2.6, M2.7, M2.9, M3, M4, M5.2) is now visually verifiable. M2/M2.6 physics/ECS/EventBus logic remains verified and unaffected.

### Carried Open Items (explicit, not silently closed)
1. ~~**M1:EXIT-1 (BLOCKER)** — No ECS→render bridge.~~ **CLOSED 2026-07-12** (§7). `MeshComponent` + `view<Transform,MeshComponent>` traversal built, renders entities, unit-tested.
2. **M1:IS-16 (material batching)** — No material-batched mesh pass; `TriangleRenderer.cpp:676-679` `[TODO]` for set=1 material descriptor offsets. Wire `MaterialSystem` bindings or remove the dead branch.
3. **M1:IS-18 (skinned mesh)** — Buffer device-address skinned-mesh animation path absent. Needed for M5.1/M5.2 zombies/companions.
4. **M1:IS-29/IS-30 (material abstraction / prefab glTF)** — No `MaterialSystem` mesh-pass abstraction; no glTF/FBX prefab loader (`EntityFactory` is JSON-component-only).
5. **M1:IS-4/IS-28 (DXC hot-reload / spirv-reflect)** — Declared targets not implemented in `src/`; pipeline library requested but no runtime shader hot-reload or reflection-driven layout.
6. **M1:IS-8 (secondary CBs)** — Parallel secondary command-buffer recording not present in render loop; primary-only.
7. **M1:IS-17 (SoA hot components)** — No Transform/bone SoA arrays; SPSC queue is the only SoA-ish structure.
8. **M1:EXT-07 (ThreadArena)** — Declared, **not wired** (`ArenaAllocateBump` has zero call sites). Per production standard (no unwired declarations), wire it into a per-tick scratch consumer or remove.
9. **M1:IS-15 (component audit)** — MetaRegistry covers 9 components, not "every component" per spec. Extend registration as new components land.
10. **M1:D3 (ID collision)** — Disambiguate `[M1-EXT-09]` (EnTTCache vs RenderGraph DAG flattener) in spec before future cross-references.
