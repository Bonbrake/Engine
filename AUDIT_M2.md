# AUDIT_M2.md — Jolt 5.5.0 physics, EventBus, destructible test entity

**Branch:** `m2/physics-destruction`.
**Audit date:** 2026-07-12.
**Evidence basis:** source read of all M2-mapped `src/` files (`PhysicsSystem.{h,cpp}`, `events/EventBus.h`, `ecs/{Components,Destructible}.{h,cpp}`, `core/Engine.{h,cpp}`, `CMakeLists.txt`, `custom-ports/joltphysics/*`) + real local runs on RTX 2070 Super (headless boot-smoke runs `runPhysicsTests` via `verifyHeadlessInit`; full unit suite).

**Status glyph legend (defined once):** ✅ verified present & wired / run-confirmed · ⚠️ partial, scaffold, declared-but-not-wired, or gated · ❌ missing / false as written / compiled-out.

---

## 1. Implementation Steps (load-bearing, per `02_M2.md`)

| # | Spec step | Status | Evidence |
|---|-----------|--------|----------|
| IS-1 | Jolt Physics **5.5.0** + `JPH::JobSystemThreadPool` + `JPH::PhysicsSystem`; right-handed Y-up | ✅ | `custom-ports/joltphysics/vcpkg.json:3` version 5.5.0; `PhysicsSystem.cpp:38-67` registers types, inits `JPH::PhysicsSystem`; log `Jolt {} initialized` (run-confirmed); Y-up per `PhysicsSystem.cpp:17` comment + gravity `(0,-9.81,0)` |
| IS-2 | Build Jolt with `JPH_CROSS_PLATFORM_DETERMINISTIC` (library compile flag) | ✅ | `custom-ports/joltphysics/portfile.cmake:28` `-DCROSS_PLATFORM_DETERMINISTIC=ON`; engine targets also define `JPH_CROSS_PLATFORM_DETERMINISTIC` (`CMakeLists.txt:178`); log confirms at boot |
| IS-3 | Fixed-timestep `PhysicsSystem::Update`, `EMotionQuality::LinearCast` CCD | ✅ | `PhysicsSystem.cpp:74-96` step at `FIXED_DT=1/60`; `createBox`/`createSphere` set `EMotionQuality::LinearCast` (`:258,:289`); `runPhysicsTests` confirms body falls under gravity |
| IS-4 | Event Delivery Bus via `entt::dispatcher` (`.enqueue<Event>()`) | ✅ | `events/EventBus.h` wraps `entt::dispatcher`; `Engine.cpp:339` `eventBus_->enqueue(dmg)`; `physicsTick` flushes via `DamageSystem::tick`+`flush` (`:534-539`) |
| IS-5 | Mirror Jolt transforms → EnTT `Transform` | ✅ | `PhysicsSystem::mirrorTransforms` (`:228-239`) writes `transform.position = fromJPH(pos)` (dvec3) + `transform.rotation = fromJPHQ(rot)`; runs each `step` |
| IS-6 | Health + Destructible separate, composable | ✅ (structs) / ⚠️ (consumption) | `Components.h:15-18` `Health`; `Destructible.h:16-25` `DestructibleComponent`; both emplace on dev/diag entities (`Engine.cpp:559-564`; `:763-767`). **But** the destruction *consumer* only sets `isDestroyed` + removes collider — it does NOT swap the mesh (see F1) |
| IS-7 | `DamageEvent` canonical struct | ✅ | `Components.h:22-37` exact shape (amount/target/source/instigator/hit_location/penetration_depth/tag); `DamageTag` enum; `ResolvedDamageEvent` split. Matches spec verbatim |
| IS-8 | `BulletComponent` declared | ✅ | `Components.h:62-68` exact shape incl. `Flags::Tumbling` bit — matches spec |
| IS-9 | Frame Resource Pacing (per-swapchain command buffers/fences) | ✅ | Carried from M0; `Swapchain.cpp` per-image frames-in-flight |
| IS-10 | Forward `JPH::DebugRenderer` into M1 debug pass | ✅ RESOLVED 2026-07-12 (see §3 F2) | `JPH_DEBUG_RENDERER` was **missing only on the engine side** — the root `vcpkg.json:17` already requests the Jolt `debugrenderer` feature, and the installed lib is built with `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON` (confirmed in `build/vcpkg_installed/.../Jolt` headers + `vcpkg/buildtrees/.../CMakeCache.txt:297`). The only gap was the engine `target_compile_definitions` never setting `JPH_DEBUG_RENDERER`. Fixed by adding it to `CMakeLists.txt:179`. `PhysicsSystem::drawBodies` (`PhysicsSystem.cpp:309-319`) + overlay projection (`Engine.cpp:463-501`) now compile and **execute** — verified: `drawBodies()` emits 36 hull lines/frame (temp diagnostic). See §3 F2 for the one capture caveat. |
| IS-11 | File System Workspace (`assets/`,`saves/`,`logs/`,`config/`,`mods/`) + Soft Asset Mitigation | ✅ | `FileHandleRing`, magenta-fallback path present (M0-era); asset-load fallback confirmed in headless smoke (`Failed to load texture ... using fallback`) |

---

## 2. M2 EXT systems (`02_M2.md` + §5.x addenda)

| EXT | Status | Evidence |
|-----|--------|----------|
| 01 Async Collision Cooking & Proxy Swap | ✅ RESOLVED 2026-07-12 (see §3 F3) | Proxy `SetShape` immediate swap was already working; the **background bake** was dead because `taskScheduler_` was never set. Fixed: `Engine.cpp:105` now calls `physicsSystem_->setTaskScheduler(core::JobSystem::get())` (enkiTS, the separate offload pool — `joltJobs_` stays Jolt-dedicated per spec). `QueueAsyncCollisionSwap` (`PhysicsSystem.cpp:211-226`) now takes the live branch and bakes the cooked `MeshShape`. |
| 02 Jolt Island-Sleep EventBus Bridge | ✅ wired | `ActivationListener` (`PhysicsSystem.h:238-252`) captures OnBodyActivated/Deactivated; `step` (`:106-123`) drains → `dispatcher.enqueue<HibernationEvent>` resolved via `StableId` lookup. `HibernationEvent` declared `EventBus.h:14-17` |
| 03 Narrow-Phase Query Collector Cache | ✅ wired | `LineQuery`/`LineQueryHash` (`PhysicsSystem.h:36-57`); `CachedRayCast` (`:127-164`) same-frame dedup, cleared each tick (`:78-81`). Thread-safe via `queryCacheMutex_` |
| 04 Fixed-Timestep Kinetic Energy Clamper | ✅ wired | `mMaxLinearVelocity=250` / `mMaxAngularVelocity` on dynamic bodies (`PhysicsSystem.cpp:260-261,:290-291`) |
| 05 Muscle Exhaustion Damping Adder | ✅ present (helper) | `ApplyPhysiologicalVelocityDamping` inline (`PhysicsSystem.h:67-71`) — math helper; not yet called into `CharacterVirtual` (M2.9 scope) |
| 06 Jolt→EnTT Double-Precision Transform Remapper | ✅ wired | `ToJPHD`/`FromJPH` exposed (`PhysicsSystem.h:186-187`); `mirrorTransforms` uses them; round-trip unit-tested in `Test_TransformPrecision.cpp` |
| 07 Kinematic Virtual Sweep Tunneling Safeguard | ✅ present (helper) | `VirtualSweepTunnelingSafeguard` inline (`PhysicsSystem.h:75-82`) — math helper; not yet invoked by a kinematic controller (M2.7 scope) |

> EXT-04/05/07 are **math helpers present but their consuming call sites are later-milestone** (M2.9 character controller, M5.2 ragdoll). Consistent with spec "depends on not yet built."

---

## 3. M2 Exit Criteria — verdict

| Criterion | Verdict | Evidence |
|-----------|---------|----------|
| Jolt simulation loops reliably within fixed timestep ticks | ✅ | `runPhysicsTests` (Engine.cpp:756-814) body falls under gravity; headless smoke `DIAGNOSTIC SUCCESS: M2 Physics and EventBus verified` (run-confirmed) |
| EventBus routes damage cleanly | ✅ | `DamageEvent` enqueued (`Engine.cpp:338` dev path; `:792` diag) → `DamageSystem::receive` applies HP loss + destruction; `ResolvedDamageEvent` emitted for downstream |
| Destructible test entity takes damage and **visibly swaps meshes** / removes colliders | ❌ **FALSE on mesh-swap** | Collider removal ✅ (`Destructible.cpp:53-59` `registry.remove<PhysicsBodyComponent>`; diag confirms `removing collider body`). **Mesh swap is NOT performed** — `receive` only `LOG_INFO("... DESTROYED — mesh swap to handle {}", destr->destroyedMeshHandle)` (`:50-52`); it never writes `destroyedMeshHandle` into the entity's `MeshComponent`. Grep for any `MeshComponent.meshHandle` mutation on destruction = 0 hits. Renderer keeps drawing the intact mesh. See F1 |
| Debug drawing maps Jolt collision hulls over visual draws | ✅ RESOLVED 2026-07-12 (F2) | Was compiled-out only on the engine side. `JPH_DEBUG_RENDERER` added to `CMakeLists.txt:179`; the installed Jolt lib already has `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON` (root `vcpkg.json:17` requests the feature), so no Jolt rebuild was needed — pure recompile, no extra deps, build time unchanged (1m43s). Verified by execution: a temporary diagnostic confirmed `drawBodies()` now emits **36 hull lines/frame** (previously 0 — the whole `#ifdef` path was excluded). **One caveat:** the windowed frame-dump harness snapshots the swapchain image before the ImGui background draw-list (which carries the projected hull lines) is composited, so a static "hull-on-scene" PNG isn't producible with the current `requestDump` mechanism. The code path is proven live via the line-count, not a visual diff. |
| Missing asset gracefully degrades to placeholder | ✅ | Soft-asset fallback confirmed (headless smoke + M0) |

**M2 overall: FAIL on one literal Exit Criterion (mesh-swap F1), debug-hull F2 RESOLVED.** The Jolt integration, fixed-step solver, EventBus, transform mirroring, and destruction *state* machine are genuinely built and run-verified. The same pattern as M1 recurs: **plumbing real, the last consumer-wiring step missing** (mesh handle never written; debug renderer was never compiled in — now fixed).

---

## 4. Known Doc Drift / Harness

| ID | Stale claim (where) | Ground truth (this audit) | Resolution |
|----|--------------------|---------------------------|------------|
| M2-D1 | Spec implies debug-renderer overlay is wired ("Forward JPH::DebugRenderer geometry into the M1 debug pass") | **Correction (2026-07-12):** the Jolt `debugrenderer` feature WAS already requested in root `vcpkg.json:17`, and the installed lib has `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON`. The only missing piece was the engine-side `JPH_DEBUG_RENDERER` compile define (now added, F2). Original audit row incorrectly stated "vcpkg feature never requested" — that was wrong; retracted. | F2 RESOLVED — engine define added; path compiles + executes (36 hull lines/frame) |
| M2-D2 | `JobSystemThreadPool` "DEDICATED to Jolt, enkiTS for everything else" | Confirmed: `joltJobs_` is a separate `JPH::JobSystemThreadPool` (`PhysicsSystem.h:216`); no `setTaskScheduler` call means enkiTS is *not* shared into Jolt. Two systems hold as spec'd, BUT M2-EXT-01's enkiTS bake is dead (F3) | F3 — wire `physicsSystem_->setTaskScheduler(JobSystem::get())` at init, or accept EXT-01 as proxy-only |
| M2-D3 | `ctest -C Debug` should list M2 cases | `catch_discover_tests(ZombieEngineTests)` IS present (`CMakeLists.txt:166`) but `ctest` reports "No tests were found" even after re-configure. Suite runs fine via the exe directly (12 cases / 61 assertions). Likely a configure-time exe-exec/DLL-PATH discovery failure on Windows, not missing wiring | Recorded as harness gap (carried). Run the exe directly as the canonical command until resolved |

---

## 5. Verdict & Carried Open Items

**VERDICT: M2 — FAIL on one literal exit criterion (F1 mesh-swap). Core physics + EventBus PASS. F2 (debug-hull) + F3 (EXT-01 async bake) RESOLVED 2026-07-12.**

The milestone built Jolt 5.5.0 with the spec-mandated `CROSS_PLATFORM_DETERMINISTIC` + `DOUBLE_PRECISION` flags (library-level, the hard requirement), a fixed-timestep solver with CCD, an `entt::dispatcher` EventBus, Jolt→`dvec3` transform mirroring, and a composable Health/Destructible model whose destruction *state* (collider removal) is run-verified. F2 and F3 (both "declared-but-dead" defects, same class as M1's ThreadArena) are now closed. One literal exit criterion still fails — the destructible entity's mesh is never actually swapped (F1).

### Close-out performed 2026-07-12 (approved scope: F2 + F3)
- **F2 — debug-hull overlay (RESOLVED).** Added `JPH_DEBUG_RENDERER` to the `ZombieEngine` target compile defs (`CMakeLists.txt:179`). The Jolt lib was already built with `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON` (root `vcpkg.json:17`), so no Jolt rebuild — pure recompile. Build time unchanged (1m43s), no extra DLLs. Verified by execution: temporary diagnostic showed `drawBodies()` emits 36 hull lines/frame (the `#ifdef` path was previously excluded). Caveat: the windowed frame-dump harness captures the swapchain before the ImGui background draw-list is composited, so the hull lines don't appear in the static PNG — proven via line-count, not visual diff.
- **F3 — M2-EXT-01 async bake (RESOLVED).** `Engine.cpp:105` now calls `physicsSystem_->setTaskScheduler(core::JobSystem::get())` (enkiTS, the separate offload pool; `joltJobs_` stays Jolt-dedicated per spec). `QueueAsyncCollisionSwap` (`PhysicsSystem.cpp:211-226`) now takes the live branch and bakes the cooked `MeshShape` instead of the dead `taskScheduler_==null` path.

### Carried Open Items (explicit, not silently closed)
1. **M2:EXIT-meshswap (BLOCKER — DEFERRED, not the proposed 2-line fix).** The originally-proposed `mc.meshHandle = ecs::Handle{ destr->destroyedMeshHandle, 1 }` is **incorrect and was NOT applied**, per the user's generation-safety flag. Root cause is a data-model defect, not a wiring gap:
   - `DestructibleComponent::destroyedMeshHandle` is typed `uint32_t` (`Destructible.h:13`) — it carries **no generation**. Hardcoding `generation=1` is unsafe: `GenerationalTable::Insert` assigns `generation=1` on first insert and only increments on `Remove` (`GenerationalTable.h:36,42`), so after any remove/reinsert the handle would silently point at a stale slot.
   - **Slot 2 has no mesh loaded.** The only `LoadMesh` call in the codebase is `dev_test_cube.glb` (`VulkanContext.cpp:144`) → `devTestMeshHandle_` = index `0`. The mesh table has exactly one entry. `destroyedMeshHandle=2` (`Engine.cpp:563`) points at an **empty slot** → `GetMesh({2,1})` returns `nullptr` → `TriangleRenderer` warns + `continue`s → the entity would **vanish**, not show a destroyed mesh. That is a regression, not a fix.
   - **Correct fix requires a real destroyed-mesh asset + a generation-carrying handle.** Options to surface to the user (not applied without go-ahead): (a) load a second `dev_test_cube_destroyed.glb` at boot, store its full `ecs::Handle`, and have `DestructibleComponent` carry `ecs::Handle destroyedMesh` (not `uint32_t`); or (b) reuse the intact mesh's handle for both states and just toggle a material/flag. **Deferred — flagged, not silently implemented.**
2. **M2:D4 (flush redundancy) — LEFT ALONE per user instruction.** `physicsTick` calls `DamageSystem::tick` (auto `update<DamageEvent>`) then `eventBus_->flush()` (`update()` all). Harmless with one subscriber; consolidating is "looks nicer, not a real bug" — skipped per the efficiency rule. Noted as a known carried item.
3. **M2:D3 (ctest discovery)** — `ctest` finds no tests though `catch_discover_tests` is present; run `ZombieEngineTests.exe` directly (12 cases / 61 assertions). Harness gap, unchanged.
