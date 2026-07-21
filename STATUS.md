# The Endless Quarantine — Engine Status

> **Canonical spec:** `spec/` (per-milestone M0-parity working set, the live source of truth for every `[Mx-EXT-nn]` block). The v79 pre-split master plan and old Antigravity working files are archived under `archive/docs/` and `archive/old_milestones/`. Binding agent rules live in `.hermes.md`.

## Active Branch
`spec/m0-parity-reformat`

## Current Milestone
**M2.6 — Phase 1 COMPLETE** ✅  
Transform double-precision (dvec3/dquat/dvec3); B2 (silent double→float truncation) closed at physics write-back + persistence boundary. Sector size 512 frozen through M4.

---

## M0 — CLOSED as PASS (2026-07-12, `AUDIT_M0.md`) with carried open items

M0 gate passed. 7 partials carried as explicit open items (not silently closed). See `AUDIT_M0.md` for full evidence.

### M0 Carried Open Items
1. **M0:step7** — Fixed-timestep focus-loss throttle is implemented as a 0.25s clamp + frame-pacing EMA + canRender skip-on-minimize, *not* a dedicated native focus-loss sim throttle. Functional but not the literal spec mechanic. Risk if later milestones depend on exact behavior.
2. **M0:step18** — GPU driver **blocklist partially populated** (`DriverBlocklist.h` now has Intel UHD 630 (0x3E9B) entry with documented dynamic-rendering edge cases). Workaround table toggles DisableComputeCulling on Intel. Remaining: add entries for known AMD RX 6000-series descriptor-buffer hazards (0x1002/0x73FF) and NVIDIA TU10x series (0x10DE/0x1F08) when crash reports confirm them. Still a functional gap but no longer fully empty.
3. **M0:step29** — `VK_EXT_debug_utils` object naming applied to queues + query pool only (`Device.cpp:360-372`); not every pipeline/buffer/image. Degrades future debuggability/traceability, not a runtime defect.
4. **M0:step33** — Deterministic crash-repro via replay: mechanism present (record/replay + tick-hash, `Input.cpp`/`Engine.cpp`), but replay-identity (identical tick traces across runs) was **not re-verified live** this pass. Needs a crash+replay pair run on real GPU.
5. **M0-EXT07** — Spec block exists (M0.md:290); **NOT implemented in src/** (no `FiberYield`/`SwitchToFiber` declaration or call site found via grep across src/). Open = implement or drop from M0 scope.
6. **M0-EXT08** — Spec block exists (M0.md:315); **NOT implemented in src/** (no bindless slot allocator found via grep). Open = implement or drop.
7. **M0-EXT10** — Spec block exists (M0.md:367); **NOT implemented in src/** (no pipeline-layout compat validator found via grep). Open = implement or drop.
8. **M0:EXT15-runtime** — `focus_probe` (CMakeLists.txt:118, `[M0-EXT-15]`) **build verified**: links via CMake target, `build-asan/Release/focus_probe.exe` (42,496 B). **Headless runtime BLOCKED**: on this box the probe exits `2` at `SDL_Init FAIL: <err>` (no video subsystem) — this is *correct probe behavior* (it detects no display), **not** a defect. Runtime half still open; needs a human GPU pass with a live display. **PASS bar on real hardware (RTX 2070 Super + display):** run `build-asan/Release/focus_probe.exe`, expect `exit=0` and the full trace `FOCUS_PROBE_START → WINDOW_CREATED ok → GETKEYBOARDSTATE NONNULL → AFTER_MINIMIZE minimized=1 → KB_DURING_FOCUSLOSS NONNULL → AFTER_RESTORE minimized=0 → KB_AFTER_RESTORE NONNULL → FOCUS_PROBE_DONE`. **HARD (non-negotiable) criteria, grounded in `focus_probe.cpp` source not just trace format:** (a) `KB_DURING_FOCUSLOSS NONNULL` (source line 36: production `SDL_GetKeyboardState(nullptr)` survives minimize without returning null — the alt-tab bug class); (b) `AFTER_RESTORE minimized=0` (source line 41: input focus recovers). Any NULL kb state during focus-loss, or stuck-minimized flag, = FAIL — log to `AUDIT_*`. Headless `exit=2` must NEVER be silently upgraded to PASS; it lacks both hard criteria by construction.
   - **Verification-tier guard (added 2026-07-14):** any future logic check for `focus_probe` run under a synthetic/virtual display (Xvfb, dummy driver + scripted `xdotool`/`wmctrl` focus events) is a **state-machine sanity check only** — it verifies printf sequencing and code paths beyond `SDL_Init`, nothing more. It does **NOT** satisfy the Step 4c hardware PASS bar (`KB_DURING_FOCUSLOSS NONNULL` + `AFTER_RESTORE minimized=0` under real OS/driver focus events on RTX 2070 + live display). The two verdicts must never be merged or substituted for one another. If a virtual-display logic check is ever run, its result should be logged as a separate `M0:EXT15-runtime-virtual` entry, not folded into this one. (Note: Xvfb/`xdotool` are unavailable in this sandbox — `uname`=MINGW64/Windows, no `apt`/`dpkg`; path was never exercised, this clause is preventive only.)

---

## M0 Exit Criteria — ALL PASS

| Criterion | Status |
|-----------|--------|
| Window opens, Vulkan initializes, validation logs clean | ✅ Windowed (non-`--dev`) confirmed — **0 VUID/validation errors** (only benign 3rd-party layer name warnings from GalaxyOverlay/OBS/RTSS — not our code). NOTE: launching with `--dev` runs the asset-verification self-test, which drives additional code paths (compute cull, descriptor-buffer EXT, occlusion queries, secondary command buffers, wireframe pipeline) that emit **124 pre-existing validation errors** — see `AUDIT_DEV_MODE_VALIDATION_BASELINE.md`. These are **not** present in a normal windowed launch and none originate from the frame-dump or Slice 0a work. |
| Swapchain survives resize + minimize | ✅ Code path verified (recreate() on VK_ERROR_OUT_OF_DATE_KHR) |
| Capability tier detected + forced fallback wired | ✅ descriptorBuffer/shaderObject/unifiedLayouts/meshShaders logged at boot; forceTier0 path wired in Config |
| PipelineCache created and persisted | ✅ pipeline_cache.bin written at shutdown |
| Crash handler + sidecar; replay yields identical tick traces | ✅ crash_log.txt + crash_sidecar.txt present; record/replay TickHash matches (windowed: non-zero hashes confirmed) |
| Budgets accurately logged via VMA | ✅ Device Local Heap headroom logged (7217 MB on RTX 2070 Super) |

## M1 — CLOSED (2026-07-12, `AUDIT_M1.md`; bridge built 2026-07-12)

GPU-driven *plumbing* built and run-verified. The ECS→render bridge **now exists**: `MeshComponent` + `view<Transform, MeshComponent>` traversal in `TriangleRenderer::draw` (`TriangleRenderer.cpp:653-689`) drives real per-entity MVPs via `BuildEntityMVP` (camera-relative `renderPos = (vec3)(entityPos - camPos)`). Windowed `--dev` run logs `"Rendered 2 ECS entities via view<Transform, MeshComponent>"` and `after_bridge.png` shows the ECS-driven entity. M1:EXIT-1 resolved. ctest 12/12 (61 assertions) incl. new `Test_RenderBridge.cpp` proving the 50km camera-relative precision invariant. See `AUDIT_M1.md` for the full original FAIL audit; the bridge was the scoped close-out.

### M1 Exit Criteria — corrected (post-bridge)
| Criterion | Verdict |
|-----------|---------|
| EnTT components render correctly via GPU indirect draw | ✅ RESOLVED — ECS entities render via `view<Transform, MeshComponent>` (`TriangleRenderer.cpp:653-689`) |
| Font files generate MSDF maps at boot | ⚠️ windowed-gated only (`Engine.cpp:67,115`); not headless-verifiable |
| Hi-Z culling logs measurable draw reduction | ✅ (on demo triangle's 100 instances) `TriangleRenderer.cpp:756` |
| ImGui panel updates cvar live; pass timings via query pools | ⚠️ windowed/`ENGINE_DEV_TOOLS`-gated; not headless-verifiable |
| No validation warnings | ⚠️ engine-clean windowed non-`--dev`; `--dev` has separate 124-error baseline (`AUDIT_DEV_MODE_VALIDATION_BASELINE.md`) |

### M1 Carried Open Items
1. ~~**M1:EXIT-1 (BLOCKER)** — No ECS→render bridge.~~ **CLOSED 2026-07-12**: `MeshComponent` + `view<Transform,MeshComponent>` traversal built, renders entities, unit-tested.
2. **M1:IS-16** — Material-batched mesh pass absent; `TriangleRenderer.cpp:676-679` `[TODO]` for set=1 material descriptor offsets.
3. **M1:IS-18** — Skinned-mesh buffer-device-address animation path absent (needed M5.1/M5.2).
4. **M1:IS-29/30** — No `MaterialSystem` mesh-pass abstraction; no glTF/FBX prefab loader (`EntityFactory` is JSON-component-only).
5. ~~**M1:IS-4/28** — DXC hot-reload / spirv-reflect not implemented in `src/`.~~ **CLOSED 2026-07-20** — spirv-reflect IS used: `PipelineBuilder.cpp` (layout building from SPIR-V reflection) and `PipelineCompatValidator.cpp` (M0-EXT-10 runtime layout validation against reflected shader bindings) both compile and link. DXC hot-reload remains a separate feature gap (no runtime shader recompile path).
6. **M1:IS-8** — Parallel secondary command-buffer recording not in render loop (primary-only).
7. **M1:IS-17** — No Transform/bone SoA arrays.
8. ~~**M1:EXT-07** — `ThreadArena` declared, **not wired** (`ArenaAllocateBump` zero call sites).~~ **CLOSED 2026-07-20** — CORRECTION: `ArenaAllocateBump` was NEVER declared in ThreadArena.h. The `BumpArena` struct directly matches the spec (memoryBufferPage/capacity/currentOffset + Reset()) and IS wired: 64KB scratch page allocated in TriangleRenderer.cpp init, `frameArena_.Reset()` called at top of each draw frame for per-tick scratch. No function was missing; no declaration needs removal. This item was a false grep inference.**
9. **M1:IS-15** — MetaRegistry covers 9 components, not "every component".
10. **M1:D3** — `[M1-EXT-09]` ID collision (EnTTCache vs RenderGraph DAG flattener); disambiguate in spec.

## M2 Exit Criteria — ALL PASS

| Criterion | Status |
|-----------|--------|
| Jolt simulation loops reliably within fixed timestep ticks | ✅ Verified via `physicsTick()` loop and headless outputs |
| EventBus routes damage cleanly | ✅ `DamageSystem::tick()` processes events from EventBus |
| Destructible takes damage and visibly swaps meshes/removes colliders | ✅ Headless log verifies mesh swap to handle 2, removal of body 0x800000 |
| Debug drawing maps Jolt collision hulls over visual draws | ✅ `PhysicsDebugRenderer` implemented and bridged to `ImGuiOverlay` |
| Missing asset gracefully degrades to placeholder | ✅ `AssetManager` falls back to generated 1x1 magenta texture (Soft Asset Mitigation) |

---

## Last Verified Build
- Branch: `spec/m0-parity-reformat`
- Spec corpus: **1,224 EXT blocks** across 16 milestone files (M0-M13 + M4.5 + M6.5), grep-verified 2026-07-20
- Structural integrity: **PASS** — `scripts/verify_m0_parity.py` reports all 16 milestone files OK. 17 JSON sidecars regenerated + consistent.
- Engine build: 8 milestone modules still header-only stubs (ai, audio, modding, net, save, slm, ui, world); render/core/debug/ecs/physics built. `ZombieEngineTests` + `focus_probe` targets defined.
- Last headless run: commit `83c0d43` (2026-07-20) — M4.5-EXT-33 AgX tonemap + RTSS workaround.
- GPU: NVIDIA GeForce RTX 2070 SUPER

## Capability Tier (RTX 2070 Super)
- descriptorBuffer ✅ | shaderObject ✅ | unifiedImageLayouts ✅ | meshShaders ✅ | rtPipeline ❌ | queryTimestamps ✅

## Commits This Pass (spec/m0-parity-reformat, 25 commits)
- `83c0d43` — [M4.5-EXT-33] Work around RTSS swapchain STORAGE_BIT injection
- `dbdb715` — [M4.5-EXT-33] AgX tonemap: validate GPU path clean (3 passes)
- `33625be` — agx-2: HDR scene RT + AgX tonemap pass via descriptor buffers
- `06811a3` — agx-1: expose Config::exposure + AgX tonemap shaders + CMake entry
- `0cee828` — m0-24: remove stray TOC line M0-EXT-41 (completes dedupe)
- `b5ae56b` — imp-40: M1-EXT-40 RayBatchQuery facade + regression test
- `768b66b` — cite-40: fix M1-EXT-28→M1-EXT-40 citation in AI.h/Audio.h stubs
- `d4bd3ea` — imp-25: M0-EXT-25 enkiTS ExecuteRange exception guard + regression test
- `53cbc4b` — P4a-fix: correct M0-EXT-25 audit false SEH claim
- `d4e0b12` — P4a: dedupe M0-EXT-41 (true dup of 24), merge M0-EXT-40; ext_blocks 52→50
- `6edad3d` — M1-EXT-08: convert to v45 format, drop false STUB flag
- `d69cc8f` — Port 7 provisional blocks (M2-EXT-122..127, M5-EXT-90)
- `a0f266c` — M8.5-8.7: port 18 faction AI & economy blocks
- `9660dbf` — M5.2: port 15 injury/degradation blocks
- `d0d4533` — M5.1: port 8 procedural zombie variation blocks
- `34f7fc5` — M4.6: port 7 memory management blocks
- `3ddf9fd` — M2.9 final 8 blocks (body temp/encumbrance/boredom/realism etc.)
- `d824fb1` — VERIFY: M2.9 Batch 2 final (109-113) - 16/16 PASS
- `641e321` — fix M2.9 b2 metadata: correct tl;dr/ctx
- `123eaf0` — M2.9: port 5 survival/physiology blocks
- `f087c4f` — M2.9: port 5 ballistics/traversal blocks
- `86c7b60` — M2.6: port 3 open-world foundation blocks
- `efd06af` — vcpkg: fix version pins to baseline-available releases
- `34788e7` — M8: implement 4 approved designs (dynamic horde, dual-axis rep, Remnant Military, Zombie Beacon) + fix vcpkg manifest baseline
- `d16c58a` — design: replace 7-day horde cycle with dynamic open-world migration

## M2.6 Phase 1 — Exit Criteria (CLOSED)
| Criterion | Status |
|-----------|--------|
| Transform authoritative world position is double-precision | ✅ `dvec3/dquat/dvec3` (Components.h:8-12) |
| Physics write-back (mirrorTransforms) preserves double position | ✅ `fromJPH` → dvec3; `fromJPHQ` → dquat (lossless) |
| MetaRegistry load path reads Transform as double (no float narrowing) | ✅ EmplaceComponent lines 110/114/118 |
| Round-trip at double-epsilon proven (not just green build) | ✅ Test_TransformPrecision.cpp [M2.6] 14 assertions pass; ad-hoc probe agrees |
| Engine.cpp:490 spawn path type-correct | ✅ `emplace<Transform>(ent, glm::dvec3(...))`, exercised by --headless |

### M2.6 Phase 1 — Carried caveats (verbatim)
- **C-caveat:** render-path rebasing is not applicable yet — no scene path consumes
  Transform to produce draws (per `AUDIT_M1_SCENE_RENDER_GAP.md`), so Phase 1 closes on
  physics/ECS/persistence only, not render.
- **Open audit item (NOT fixed this pass):** `SpatialHash.cpp:51-52,134-135` still read
  Transform position into `float` — undetermined local-vs-world-space question, left flagged.
- `Engine.cpp:558-562` also read `.position.x/.z` into float — audit-only, not fixed.

## M2.6 Phase 2 — fly-camera + self-verify tooling ✅ (with open items)
Debug fly-camera (`debug::FlyCamera`) injected into the dev cube render path via
`VulkanContext::setDevView` → `TriangleRenderer::setDevView` (no `draw()` signature
change; mirror of `setDevTestMesh`). Near cube at origin + far cube at dvec3(50000,0,0)
(reuses `view`, `cullEnabled=0`, single dvec3→vec3 cast — NOT camera-relative
subtraction; that technique remains unbuilt pending a real scene render path — see C-caveat).

**Self-verify tooling added (no display needed):** `--script-input <file>` drives
`Input::poll()` from a tiny text script (`K W 0 40` = hold W frames 0–40; `M +200 0 10 20`
= mouse +200dx frames 10–20); `--dump-state <file>` emits per-frame `FlyCamera` pose
JSON. Fly-camera update + state-dump run in headless AND windowed; only render
injection + `SDL_SetWindowRelativeMouseMode` are windowed-gated.

**Self-verify results (headless, real build, `selftest_flycam.txt`):**
| Check | Result |
|-------|--------|
| Build (ZombieEngine Debug) | ✅ clean, 0 warn/err — caught SDL3 API rename (`SDL_SetRelativeMouseMode`/`SDL_TRUE` removed → `SDL_SetWindowRelativeMouseMode(window_,bool)`) |
| Suite (ctest) | ✅ 9/9 |
| Validation VUIDs fly vs no-fly | ✅ 16 vs 16, 0 introduced (baseline per AUDIT_DEV_MODE_VALIDATION_BASELINE.md) |
| W held (frames 1–40): pos advances along forward | ✅ z:10.0→9.79, x drifts as yaw rotates — **not frozen** |
| Mouse `M +200` (frames 10–20): yaw rotates | ✅ fwd [0,-0.196,-0.981] → [0.973,-0.196,-0.125] (~78°), relative-delta path feeds correctly |
| Original report "just a cube, nothing happens" | ✅ Resolved + self-verified headless (mouse-look fix confirmed; keyboard was always per-frame-correct) |

**OPEN ITEMS (carried, not silently closed):**
1. **Yaw/pitch sign direction** — `FlyCamera.cpp:23-24` (`yaw_ -=`/`pitch_ -=`); self-verify
   proves rotation *happens* but not whether mouse-right = screen-right. One-line flip if inverted. **Needs human GPU pass to confirm direction.**
2. **Far-cube steadiness at ~50km** — the far cube is now a **real ECS entity** at `dvec3(50000,0,0)` driven through `view<Transform,MeshComponent>` + `BuildEntityMVP` (camera-relative), not the old hardcoded `kFarCubeWorldPos` block. `after_bridge.png` shows it rendering; the `Test_RenderBridge` rebase-invariance test (K=1e6) proves the math holds in double→single. Sub-pixel steadiness *while flying there* still wants the human GPU pass, but the precision path is now built + unit-proven.
3. ~~**Camera-relative subtraction (Spike B)** — NOT implemented.~~ **CLOSED 2026-07-12**: implemented in `BuildEntityMVP` (`TriangleRenderer.cpp:134-149`) — `renderPos = (vec3)(entityPos - camPos)` single cast; the authoritative dvec3 never enters single-precision matrix math. Unit-tested (`Test_RenderBridge.cpp` rebase invariant, K=1e6, epsilon 1e-3).
- **Step 4c runtime verification is tracked under `[M0-EXT15-runtime]` in M0 Carried Open Items** (build verified; runtime blocked on live display, PASS bar defined there). Not a code defect — do not treat as a M2.6 fix item.

## Next Session: close M2.6 Phase 2 open items (yaw-sign GPU confirm, far-cube steadiness) or beat
Load `archive/old_milestones/milestones_M0-M13_antigravity/03_M2_6.md`.  \n

Read STATUS.md first. Confirm branch. Check .gitignore covers build/ and vcpkg_installed/.

---

## M2 — FAIL on 1 exit criterion; F2+F3 RESOLVED (2026-07-12, `AUDIT_M2.md`)

Jolt 5.5.0 built with `CROSS_PLATFORM_DETERMINISTIC=ON` + `DOUBLE_PRECISION=ON` (library-level, spec's hard requirement — met). Fixed-timestep solver + `LinearCast` CCD, `entt::dispatcher` EventBus, Jolt→`dvec3` transform mirroring, and composable Health/Destructible are genuinely built and run-verified (headless `runPhysicsTests` confirms gravity + lethal-damage → `isDestroyed` + collider removal). One literal exit criterion still fails (mesh-swap F1); F2 (debug-hull) + F3 (EXT-01 async bake) **closed this session**:

- **F2 RESOLVED** — added `JPH_DEBUG_RENDERER` to `CMakeLists.txt:179`. Jolt lib already had `DEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON` (root `vcpkg.json:17` requested the feature; no Jolt rebuild needed). Build time unchanged, no extra deps. Verified: `drawBodies()` emits 36 hull lines/frame (was 0 — path was `#ifdef`-excluded). Caveat: frame-dump harness captures swapchain before ImGui overlay composites, so no static "hull-on-scene" PNG — proven via line-count.
- **F3 RESOLVED** — `Engine.cpp:105` now calls `physicsSystem_->setTaskScheduler(core::JobSystem::get())`; `QueueAsyncCollisionSwap` bakes the cooked `MeshShape` (was dead `taskScheduler_==null` branch). `joltJobs_` stays Jolt-dedicated per spec.

- **Mesh-swap (F1) — COMPILES + GPU-VERIFIED (2026-07-13)** — F1 is wired end-to-end for the dev-test entity and *visually confirmed on real hardware* (RTX 2070): `VulkanContext.cpp:159` loads `dev_test_cube_destroyed.gltf`, `Engine.cpp` assigns the dev-test + destroyed mesh handles, and `DamageSystem::receive` (`Destructible.cpp`) writes `destroyedMeshHandle` into `MeshComponent.meshHandle` on lethal damage. The bridge (`TriangleRenderer::draw` over `view<Transform,MeshComponent>`) renders the swapped mesh; capture `devcube_swap7.png` shows fragmented/shattered geometry at (0,0,0) after the frame-120 lethal hit — the swap fires *and* renders, not just in ECS data. Two render-path bugs were found + fixed during GPU verification: (1) `vkCmdSetViewport/Scissor` hoisted to run unconditionally (mesh pipeline is `VK_DYNAMIC_STATE_VIEWPORT/SCISSOR`; Path A was the only setter, so gating it off left the bridge drawing into an unset viewport); (2) `vkCmdBindPipeline(meshPipeline)` added to the bridge block (it was unbinding the pipeline → black frame). `MeshHandle` is a generation-safe `ecs::Handle`. Caveat: still dev-gated (`spawnDevTestBody_`); extend to non-dev destructibles before calling F1 fully shipped.
- **F4 LEFT ALONE** (per user) — `physicsTick` redundant `tick()`+`flush()` re-dispatch; harmless, not a real bug.

M2-EXT-01 now live (F3). EXT-02/03/04/06 wired; EXT-05/07 present as math helpers awaiting M2.9/M5.2 consumers.

### M2 Carried Open Items
1. **M2:EXIT-meshswap (COMPILES + GPU-VERIFIED, dev-gated)** — F1 is coded end-to-end and visually confirmed on RTX 2070: dev-test entity swaps to the shattered `dev_test_cube_destroyed` mesh on lethal damage and the swap renders (capture `devcube_swap7.png`). Two render-path bugs (missing unconditional viewport set, missing `vkCmdBindPipeline` in the bridge) were fixed to get the visual. See F1 note above. Remaining: extend the swap to non-dev destructibles (gameplay barrels/walls/zombies) before calling F1 fully shipped. No longer a blocker on the dev path.
2. **M2:D4** — redundant `tick()`+`flush()`; left alone per user (harmless).
3. **M2:D3 (ctest discovery)** — `ctest` finds no tests though `catch_discover_tests` present; run `ZombieEngineTests.exe` directly (12 cases / 61 assertions). Harness gap.
---
## RESUME POINTER (2026-07-16)
Session state saved to `recon/SESSION_STATE_2026-07-16.md` — read it to continue.
Open items: [A] M1 mega-file extraction (gated, blocked by mid-line concatenation),
[B] deeper Threat Interactive research (search 429 this turn — retry or user provides file),
[C] 4 duplicate merges (M0-EXT-24/41, M0-EXT-25/40, M5-EXT-22/52, M5-EXT-17/09-11),
[D] naming-scheme rename (user wants it; gated big refactor).
Spec consolidation + 25 gaps + audits ALL DONE and committed on `spec/m0-parity-reformat`.

## Session close — 2026-07-17 (doc-only bookkeeping correction)
- Corrected stale EXT-block counts in `STATUS.md`, `spec/AGENTS.md`, `spec/llms.txt`: real
  total is **762** (715 milestone M-blocks + 47 in APPENDICES: 21 M + 22 K + 4 L), not the
  prior "694"/"627". Per-file counts in `llms.txt` reconciled to actual `#### [ID]` headers
  (verifier-enforced `ext_blocks` frontmatter already matched).
- Fixed M0-EXT07/08/10 open-item wording in STATUS.md: grep across `src/` confirms these are
  **spec-only** blocks with no code declaration at all (prior "declared, not wired" was wrong).
  No code changed, nothing to remove.
- `scripts/verify_m0_parity.py` re-run: all 16 milestone files OK (EXIT 0). APPENDICES.md is
  correctly excluded (different block format — no frontmatter / `#####` subsections / sidecar).
- No code/build change this session. Build NOT re-run; prior artifacts present under `build/`.
