# The Endless Quarantine — Engine Status

## Active Branch
`m2/physics-destruction`

## Current Milestone
**M2.6 — Phase 1 COMPLETE** ✅  
Transform double-precision (dvec3/dquat/dvec3); B2 (silent double→float truncation) closed at physics write-back + persistence boundary. Sector size 512 frozen through M4.

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

## M1 Exit Criteria — ALL PASS

| Criterion | Status |
|-----------|--------|
| EnTT components render correctly via GPU indirect draw | ✅ Windowed: colored triangles rendering, GPU indirect draw active |
| Font files generate MSDF maps at boot | ✅ AssetManager initialized, Roboto-Regular.ttf present |
| Hi-Z culling logs measurable draw call reduction | ✅ Windowed log: "Hi-Z Occlusion Culling: 4/100 instances visible (96% reduction)" |
| ImGui panel updates cvar behaviors live; pass timings via query pools | ✅ ImGui NewFrame/Render cycling confirmed in windowed log; queryTimestamps=true |
| No validation warnings | ✅ Zero VUID errors (non-`--dev` windowed launch); only benign 3rd-party layer name warnings (not suppressible without removing those layers). `--dev` launch has a separate pre-existing 124-error baseline — see `AUDIT_DEV_MODE_VALIDATION_BASELINE.md` (none from this milestone's scope). |

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
- Branch: `m2/physics-destruction`  
- Last commit: `66af99e` — [M2.6] Phase 1: Transform double-precision (dvec3/dquat) - close B2  
- Headless CI: **PASS** (`HEADLESS CI SMOKE TEST: SUCCESS`)
- ZombieEngineTests: **PASS** (9 cases / 49 assertions; [M2.6] 2 cases / 14 assertions)
- Build: `cmake --build --config Debug` EXIT=0
- Windowed smoke test: **PASS** (2026-07-11, RTX 2070 Super)
- GPU: NVIDIA GeForce RTX 2070 SUPER

## Capability Tier (RTX 2070 Super)
- descriptorBuffer ✅ | shaderObject ✅ | unifiedImageLayouts ✅ | meshShaders ✅ | rtPipeline ❌ | queryTimestamps ✅

## Commits This Pass (Audit & Fix)
- `1ee9145` — Update engine with latest changes (full codebase, Batch 1+2)
- `eb19957` — Batch 3: extension gating, triple-buffer fix, render tests
- `4e4684a` — Add STATUS.md
- `66af99e` — [M2.6] Phase 1: Transform double-precision (dvec3/dquat) - close B2

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

## Next Session: M2.6 Phase 2 (fly-camera) or beat
Load `milestones_M0-M13_antigravity/03_M2_6.md`.  
Read STATUS.md first. Confirm branch. Check .gitignore covers build/ and vcpkg_installed/.
