# AUDIT_FOUNDATION_GAP.md — M0/M1 foundation gaps vs. the M2–M13 roadmap

**Date:** 2026-07-13
**Auditor:** Hermes (ZombieEngine foundation audit)
**Category:** forward-dependency / missing-foundation audit — does the M0/M1 substrate contain every system that M2–M13 assume exists?
**Standard:** every "DEAD" / "0 code" / "PARTIAL" claim below is backed by the literal grep command + its actual (empty or non-empty) output. No summary-of-a-search passes as evidence.
**Cross-reference:** See `AUDIT_TRIANGLE_RENDERER_EXT03_MISLABEL.md` (same folder). That audit flags `TriangleRenderer.cpp:486/751/785` mis-tagged as `[M1-EXT-03]` (spec defines EXT-03 as Command Pool Matrix). **This audit's T2 row "M1-EXT-18 Material-Batched" lives in the SAME file** (`TriangleRenderer.cpp:724` `[TODO]`), so the two audits share a surface and are intentionally linked below.

---

## 0. Git integrity check (run first, per reviewer instruction)

```
$ git branch -v
  Main                        205668f M2: Physics debug rendering, triple-buffer frame pacing, soft asset mitigation
* m2/physics-destruction      b9423e3 [ahead 7] Spec batch v3: 11 EXT entries + 2 RI fixes + src-mislabel audit
+ subagent-Core-Code-Auditor-research-3957b3e9  804cb55 Resolve merge conflicts by keeping M0 feature branch code
+ subagent-ECS-and-Debug-Auditor-research-d9eddb4d 804cb55 Resolve merge conflicts by keeping M0 feature branch code
+ subagent-Render-Code-Auditor-research-41c6af0c  804cb55 Resolve merge conflicts by keeping M0 feature branch code

$ git cat-file -t b9423e3
commit

$ git merge-base --is-ancestor b9423e3 HEAD && echo "b9423e3 IS an ancestor of HEAD"
b9423e3 IS an ancestor of HEAD
```

**Result:** `b9423e3` is valid and is an ancestor of current HEAD on `m2/physics-destruction`. Working tree is dirty with pre-existing modifications (not reconciled here — read-only audit). Integrity check PASS.

---

## 1. Self-corrections to the earlier (verbal) audit — owned here, not hidden

These two claims from the prior chat audit were **wrong**; the grep pass below disproves them:

- **Earlier claim "fastgltf linked but 0 loader" → FALSE.** `AssetManager.cpp:276-404` actually parses glTF and builds `MeshAsset`s. The real gap is *transform hierarchy / scene-graph parenting* and *model→entity binding*, not geometry import.
- **Earlier claim "EnTTCache 0 call sites" → FALSE.** `EnTTCache`/`SnapshotComponentPool` IS called in `Engine.cpp` (diagnostic self-tests at frame 6 and a bounds/null diagnostic at 673–701). It is wired into diagnostics, not a hot system loop — that nuance is preserved in T2 below.

---

## 2. Tier 1 — Foundational subsystems future milestones assume exist

Severity: **P0** = already-broken or gates the very next milestone (fix before leaving M2.6) · **P1** = cheap to scaffold now, expensive to retrofit once entities exist · **P2** = blocks a named later milestone, additive to build.
Cost: rough LoC, call sites, and whether additive (safe to bolt on) or breaking (changes structs/APIs already used by M2–M6 entities).

---

### T1.1 — Component (de)serialization substrate  →  P1
**Depended on by:** M7 (save/load), M2.8 (co-op sync), M12 (net delta), M4 (streaming).
**Severity:** P1. **Cost:** ~500–900 LoC (registry + 13 component impls), touches `Components.h` (88 LoC) + new files. **Breaking if retrofitted** (every component struct would need a serialize/deserialize hook after the fact).

```
$ grep -rniE 'serialize|deserialize|savegame|save_game|saveload' src/
src/tests/Test_TransformPrecision.cpp:18:// No Transform save-serializer exists in the repo yet; load-path is the only persistence edge.
```
(A single test comment acknowledging the absence. No serializer code anywhere.)

---

### T1.2 — Asset import + scene-graph / transform parenting  →  P2
**Depended on by:** M4 (procedural buildings), M5.2 (characters), M3 (destructibles).
**Severity:** P2. **Cost:** glTF geometry import already exists (see below); add parent/child transform hierarchy + model→entity binding ≈ 200–400 LoC. **Additive** to import, but the hierarchy is a new concept.

```
$ grep -rniE 'fastgltf|loadgltf|loadmodel|scenegraph|scene graph' src/
src/render/AssetManager.cpp:3:#include <fastgltf/core.hpp>
src/render/AssetManager.cpp:4:#include <fastgltf/tools.hpp>
src/render/AssetManager.cpp:5:#include <fastgltf/glm_element_traits.hpp>
src/render/AssetManager.cpp:276:    fastgltf::Parser parser;
src/render/AssetManager.cpp:283:    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers);
src/render/AssetManager.cpp:313:        fastgltf::Accessor& indexAccessor = asset.get().accessors[primitive.indicesAccessor.value()];
src/render/AssetManager.cpp:404:    ecs::Handle handle = meshes_.Insert(mesh);

$ grep -rniE '\bparent\b|\bchild\b' src/ecs/ src/core/
src/core/Engine.cpp:650:        // Assert they are all queryable from the parent cell
src/core/Engine.cpp:657:        // Verify that the parent bucket is actually empty (removed from parent)
src/core/Engine.cpp:660:        // We can't easily assert the private buckets map, but we did add the logic to erase the parent bucket.
```
Geometry import = **PRESENT** (AssetManager), but only "parent"/"child" references are SpatialHash *cell* comments — **no transform-hierarchy / scene-graph** for entities.

---

### T1.3 — Material system + material asset (draw integration)  →  P2
**Depended on by:** M4.5 (vis-buffer material resolve reads a per-primitive material index), M4.
**Severity:** P2. **Cost:** `MaterialSystem` class already exists (116 LoC, constructed in `VulkanContext.cpp:88`). Draw integration is a ~5-LoC wiring of the `:724` TODO + M0-EXT-08 descriptor slot allocator (net-new ~150–250 LoC). **3** call sites in `TriangleRenderer::draw`.

```
$ grep -rn 'MaterialSystem' src/ --include=*.cpp
src/render/MaterialSystem.cpp:6:void MaterialSystem::Initialize(Device* device) {
src/render/MaterialSystem.cpp:76:ecs::Handle MaterialSystem::CreateMaterial(const MaterialAsset& config, AssetManager* assetManager) {
src/render/MaterialSystem.cpp:112:MaterialAsset* MaterialSystem::GetMaterial(ecs::Handle handle) {
src/render/Swapchain.cpp:177:void Swapchain::acquireAndPresent(debug::ImGuiOverlay* imguiOverlay, MaterialSystem* materialSystem) {
src/render/TriangleRenderer.cpp:621:void TriangleRenderer::draw(VkCommandBuffer cmd, uint32_t imageIndex, MaterialSystem* materialSystem) {
src/render/VulkanContext.cpp:88:    materialSystem_ = std::make_unique<MaterialSystem>();

$ grep -n 'material' src/render/TriangleRenderer.cpp
621:void TriangleRenderer::draw(VkCommandBuffer cmd, uint32_t imageIndex, MaterialSystem* materialSystem) {
710:    if (materialSystem && materialSystem->GetDescriptorBuffer() != VK_NULL_HANDLE) {
712:        bindingInfos[1].address = materialSystem->GetDescriptorBufferAddress();
724:        // [TODO] When graphics pipeline has set=1 (material system bindings), bind dynamic offset here:
726:        // VkDeviceSize matOffset = handle.index * materialSystem->GetAlignedBlockSize();
```
Material descriptor buffer is bound (710–712) but the per-entity material offset at 724–726 is an unwired `[TODO]`. **This is the same file/area as `AUDIT_TRIANGLE_RENDERER_EXT03_MISLABEL.md`** — cross-referenced.

---

### T1.4 — Skinned mesh + animation  →  P2
**Depended on by:** M5.1 / M5.2 (zombies, companions, ragdoll).
**Severity:** P2. **Cost:** net-new ≈ 500–1000 LoC + vertex skinning shader changes. **Breaking** to `MeshComponent` if retrofitted after characters exist.

```
$ grep -rniE 'skeleton|animationclip|skinnedmesh|skinning|bone' src/
(no output — exit code 1)
```
Confirmed **0** code.

---

### T1.5 — World / chunk streaming manager  →  P2
**Depended on by:** M4 (WFC/Voronoi chunks), and **M2-EXT-01** (async collision-shape swap) already *assumes* chunk stream-in exists.
**Severity:** P2. **Cost:** net-new manager ≈ 600–1200 LoC; must integrate spawn/render/cull lifecycle.

```
$ grep -rniE 'worldmanager|chunkstream|sector|chunk' src/
(no output — exit code 1)
```
Confirmed **0** code. Note: M2.6 froze sector size = 512 and double-precision Transform, but no load/unload manager exists.

---

### T1.6 — Game camera manager + player entity  →  P0
**Depended on by:** M2.7 (player controller) — the *next* milestone; and everything that needs a view/proj.
**Severity:** P0 (first hard forward wall). **Cost:** `CameraManager` abstraction ≈ 100–150 LoC + refactor **4** `lookAt` sites (Engine.cpp:480,482; TriangleRenderer.cpp:650; FlyCamera.cpp:48) + net-new `PlayerController` entity ≈ 150–250 LoC. **Partially breaking** (new abstraction replaces hardcoded view).

```
$ grep -rniE 'lookat|cameramanager|firstperson|thirdperson|gamecamera' src/
src/core/Engine.cpp:480:                        : glm::lookAt(glm::vec3(0, 5, 20), glm::vec3(0,0,0), glm::vec3(0,1,0));
src/core/Engine.cpp:482:                    glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 20), glm::vec3(0,0,0), glm::vec3(0,1,0));
src/debug/FlyCamera.cpp:48:    return glm::lookAt(
src/render/TriangleRenderer.cpp:650:            : glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
src/tests/Test_RenderBridge.cpp:29:    const glm::mat4 view = glm::lookAt(...);

$ grep -rniE '\bplayer\b|playercontroller|pawn' src/
src/core/Engine.cpp:269:            // [M2-#4] Dev-only: spawn a Jolt body so the debug-draw overlay has
src/core/Engine.cpp:272:                spawnDevTestBody_();
... (only dev-test-body comments; no player entity exists)
```
Camera = hardcoded `lookAt` + debug-only `FlyCamera`. **No player entity, no `PlayerController`, nothing.** M2.7's first wall.

---

### T1.7 — SpatialHash float-position truncation  →  P0  [FIXED 2026-07-13, see §8.1]
**Depended on by:** M5 (horde density), M3 (crush), M9 (drag), M8.6 (power grid), M4 (chunk) — the universal spatial primitive.
**Severity:** P0 — this was an **already-in-code precision bug**, the M2.6 carried caveat. **FIXED** in §8.1 (entire position chain widened `float`→`double`; 0 call-site regressions).

```
$ grep -n 'float' src/ecs/SpatialHash.cpp   (PRE-FIX state, for record)
7:std::pair<int32_t, int32_t> SpatialHash::GetSubquadrantCoords(float x, float z) const {
8:    float remainderX = std::fmod(x, CELL_SIZE);
9:    float remainderZ = std::fmod(z, CELL_SIZE);
18:uint64_t SpatialHash::GetEntityTargetKey(float x, float z, bool isSubdivided) const {
28:void SpatialHash::Insert(entt::entity entity, float x, float z, entt::registry* registry) {
44:            float px = 0.0f, pz = 0.0f;
90:void SpatialHash::Remove(entt::entity entity, float x, float z) {
108:void SpatialHash::Update(entt::entity entity, float oldX, float oldZ, float newX, float newZ, entt::registry* registry) {
134:        float newX = transform->position.x;
135:        float newZ = transform->position.z;
177:std::vector<entt::entity> SpatialHash::QueryRadius(float x, float z, float radius) const {
```
Lines 134–135 read `Transform::position.x/.z` (a `dvec3`) into `float`, and the public API took `float x, float z`. At world scale (>~1–2 km) this silently mis-buckets. **Now carries `double` end-to-end.**

---

### T1.8 — Input action map / rebinding  →  P1
**Depended on by:** M2.7 (controller), M11 (stealth/rebind).
**Severity:** P1. **Cost:** ≈ 200–400 LoC wrap around `Input`; **additive** (no existing call sites break).

```
$ grep -rniE 'inputaction|actionmap|keybinding|rebind|action layer' src/
(no output — exit code 1)
```
Confirmed **0** — raw `InputState` poll only.

---

## 3. Tier 2 — Dead / unwired M0/M1 EXT that later milestones cite by ID

```
### M0-EXT-08 Descriptor-Buffer Slot Allocator (cited by M3, M4.5, M4.6, M5.2, M5.4, M8, M8.5, M8.6, M9, M13)
$ grep -rniE 'descriptorpage|assigndescriptorslot|slotalloc' src/
(no output — exit code 1)   → DEAD (TriangleRenderer writes descriptors directly)

### M0-EXT-10 Pipeline Layout Compat Validator (cited by M4.6)
$ grep -rniE 'pipelinecompatibility|validatelayoutcompat' src/
(no output — exit code 1)   → DEAD (PipelineCompatibility.h does not exist)

### M1-EXT-07 ThreadArena / Bump (cited by M5.2)
$ grep -rn 'ArenaAllocateBump' src/
src/core/ThreadArena.h:19:inline void* ArenaAllocateBump(BumpArena& arena, size_t size, size_t alignment = 16) {
→ DECLARED ONLY. 0 call sites. STATUS.md falsely claimed wired via frameArena_ — corrected 2026-07-13 (see §8.2).
  (frameArena_ in TriangleRenderer is a raw BumpArena; ArenaAllocateBump never called.)

### M1-EXT-09 EnTTCache (cited by M4, M5.1, M5.3, M5.4, M4.6)
$ grep -rEn 'EnTTCache|SnapshotComponentPool' src/ --include=*.cpp
src/core/Engine.cpp:31:#include "../ecs/EnTTCache.h"
src/core/Engine.cpp:393:            auto cache = ecs::SnapshotComponentPool<ecs::Transform>(reg);
src/core/Engine.cpp:410:                LOG_INFO("EXT-09 Cache verified: {} entities matched ...", view.size());
src/core/Engine.cpp:673:    // 2. Test EnTTCache bounds and null checks
src/core/Engine.cpp:682:        ecs::LockedComponentPoolCache cache = ecs::SnapshotComponentPool<ecs::Transform>(reg);
→ WIRED INTO DIAGNOSTIC SELF-TESTS (frame 6 + bounds/null diag), NOT a hot system loop.
  (Corrects earlier verbal "0 call sites" — it IS called, in diagnostics.)

### M1-EXT-12 MSDF glyph rasterizer (cited by M4 signage, M4.5, M13)
$ grep -rn 'GenerateAtlas' src/ --include=*.cpp
src/core/Engine.cpp:124:            render::MSDFPipeline::GenerateAtlas(
src/render/MSDFPipeline.cpp:14:void MSDFPipeline::GenerateAtlas(Device* device, const std::string& ttfPath) {
→ PRESENT but WINDOWED-GATED (Engine.cpp:124 is inside `if (!Config::get().headless)`).
  M4 signage needs glyph atlas at boot/headless — currently dev-window only.

### M1-EXT-18 Material-Batched Mesh-Pass (cited by M4, M4.5)  ← SAME FILE AS mislabel audit
$ grep -nE 'material-batched|GetMaterial|set=1|MaterialBatch' src/render/TriangleRenderer.cpp
724:        // [TODO] When graphics pipeline has set=1 (material system bindings), bind dynamic offset here:
→ DEAD/UNWIRED. This TODO sits in TriangleRenderer.cpp — the same file whose
  :486/751/785 occlusion comments are mislabeled [M1-EXT-03] per AUDIT_TRIANGLE_RENDERER_EXT03_MISLABEL.md.
  Both audits should be closed together when the material pass is wired.
```

---

## 4. Four new hunting areas (requested) — literal grep evidence

### NEW1 — Vulkan validation layer at runtime (outside dev builds)
```
$ grep -rniE 'VK_LAYER_KHRONOS_validation|request_validation_layers|ENABLE_VULKAN_VALIDATION|validation' src/ CMakeLists.txt
src/render/VulkanContext.cpp:42:           .request_validation_layers(core::Config::get().devMode)
CMakeLists.txt:169:# Vulkan validation layers ON in Debug and OFF in Release
CMakeLists.txt:173:    $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
CMakeLists.txt:175:    $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
```
**Finding:** Validation layers are **dev/Debug-only** (`request_validation_layers(devMode)`, and CMake defines it `=1` Debug / `=0` Release). In a Release build, **real Vulkan VUID/spec errors are silent** — no abort, no log. Combined with AGENTS.md's "green build = FALSE signal for precision bugs," a release player gets zero validation safety net. Severity: real, M4.6-adjacent. (The `--dev` 124-error baseline is a separate matter — that's the dev self-test path, not runtime guardrail.)

### NEW2 — Memory budget tracking (M4.6 is "Memory management")
```
$ grep -rniE 'memorybudget|vma_|trackalloc|budget' src/ | head -30
src/render/Device.cpp:69:    selector.add_required_extension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
src/render/Device.cpp:280:    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | ...
src/render/Device.cpp:293:    // Log Memory Budget using [M0-EXT-05]
src/render/Device.cpp:294:    VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps{};
src/render/Device.cpp:305:            float headroom = GetHeapHeadroom(budgetProps, i);
src/render/MemoryBudget.h:8:// [M0-EXT-05] Discrete GPU Memory-Budget Tracker
src/render/MemoryBudget.h:9:inline float GetHeapHeadroom(const VkPhysicalDeviceMemoryBudgetEXT& budget, uint32_t heapIndex) {
```
**Finding:** NOT 100% unbuilt (corrects the prior audit's omission). `M0-EXT-05` logs heap **headroom at boot** (Device.cpp:293–305). But there is **no per-allocation ledger** — nothing tracks "this texture cost X VRAM," no budget enforcement, no eviction. M4.6 would build the accounting layer on top of this partial substrate. Severity: partial-available.

### NEW3 — Config / settings persistence (distinct from save-game)
```
$ grep -rniE 'settings.json|savesettings|loadconfig|persist|writeconfig|saveconfig' src/
src/core/Config.cpp:26:        std::string cl_settingsPath = "settings.json";
src/core/Config.cpp:45:            ("settings", "Path to settings.json", cxxopts::value<std::string>(cl_settingsPath)->default_value("settings.json"))
src/core/Config.cpp:97:            std::cerr << "Failed to parse settings.json: " << e.what() << std::endl;
```
**Finding:** `settings.json` is **read** at boot (Config.cpp:26/45/97) but **never written**. There is no `SaveSettings`/`WriteConfig`. Once T1.8 (input action map) exists, key bindings / resolution would have **no persistence path**. Severity: P1-ish (blocks M2.7/M11 settings save). Cost: ~80–150 LoC. Additive.

### NEW4 — Crash / telemetry reporting
```
$ grep -rniE 'SetUnhandledExceptionFilter|minidump|writeminidump|writesidecar|telemetry|crashpad' src/
src/core/Platform.cpp:6:#include <minidumpapiset.h>
src/core/Platform.cpp:23:void Platform::writeMinidump(void* exceptionPointers) {
src/core/Platform.cpp:27:        MINIDUMP_EXCEPTION_INFORMATION exInfo;
src/core/Platform.cpp:32:        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exInfo, NULL, NULL);
src/core/Platform.cpp:47:    // Write minidump first (minimize deadlock risk)
src/core/Platform.cpp:48:        Platform::writeMinidump(exceptionInfo);
src/core/Platform.cpp:62:    SetUnhandledExceptionFilter(unhandledExceptionFilter);
```
**Finding:** Crash dump path **exists on Windows** (`SetUnhandledExceptionFilter` + minidump + sidecar; matches the user's Windows-10 host). **No telemetry / auto-report / symbol upload.** No Linux/other-platform handler (uses Win32 `minidumpapiset.h`). The manual ASan process this session used is a *sanitizer build*, orthogonal to crash reporting — so future non-ASan release bugs still need this exact manual repro unless a crash dump is captured (which it is, on Windows). Severity: acceptable for local Windows dev; note cross-platform gap.

---

## 5. Severity ranking + dependency chain (what gates the most)

| Rank | Item | Why | Severity |
|------|------|-----|----------|
| 1 | **T1.7 SpatialHash float** [FIXED §8.1] | Was already-broken in code; the universal spatial primitive for M3/M5/M9/M8.6/M4. Mis-buckets at >~1–2 km. | **P0** → resolved |
| 2 | **T1.6 Camera + Player entity** | M2.7's first hard wall — no player entity exists at all; camera is hardcoded `lookAt`. | **P0** |
| 3 | **T1.1 Serialization substrate** | Gates M7/M2.8/M12/M4; cheap to scaffold in `Components.h` now, breaking to retrofit. | **P1** |
| 4 | **T1.8 Input action map** | Gates M2.7/M11; additive wrap, cheap. | **P1** |
| 5 | **T1.3 Material path (M0-EXT-08 + M1-EXT-18)** | Gates M4.5 vis-buffer; draw wiring is ~5 LoC. | **P2** |
| 6 | **T1.2 Scene-graph/parenting** | Gates M4/M5.2; geometry import already present. | **P2** |
| 7 | **T1.4 Skinned/anim** | Gates M5.1/M5.2; large, far. | **P2** |
| 8 | **T1.5 World/chunk** | Gates M4; M2-EXT-01 already assumes it. | **P2** |

---

## 6. "What breaks first" test — if you keep building M2–M6 without touching any of these

- **First silent-wrong result:** was **T1.7 SpatialHash float** — now FIXED (§8.1). Next in line: none remaining in Tier 1 at small scale; the seed-root gap (§9 A9/C3) is the new silent-desync risk but only activates under M2.8/M4.
- **First hard wall (can't proceed):** **T1.6** — M2.7 needs a player entity (doesn't exist) and a camera abstraction (hardcoded `lookAt`). You physically cannot build a playable controller without both. This is the immediate next-step blocker (confirmed by the boot trace in §10).
- **First named-milestone block:** **T1.5 World/chunk** for M4, and **T1.3 Material** for M4.5 — both far enough that M2.7/M3/M5 can proceed first.
- **First persistence dead-end:** **T1.1** at M7, and **NEW3 Config write** once T1.8 lands.

**Conclusion:** T1.7 fixed. Scaffold **T1.6** (player entity + camera manager, P0, M2.7 wall) before leaving M2.6. T1.1/T1.8 are cheap to design-in now (P1). T1.3/T1.2/T1.4/T1.5 are P2 — build when their milestone arrives, not before.

---

## 7. Carried open items from prior audits (not re-litigated here)
- `STATUS.md` M1:EXT-07 closure was **false** — corrected in STATUS.md §1 item 8 on 2026-07-13 (see §8.2). `ArenaAllocateBump` remains declared-only (0 call sites).
- `AUDIT_M0.md` D2/D3/D4 and `AUDIT_M1.md` M1-D1..D4 document the stale-audit-doc drift; this file supersedes none of them, it adds the foundation-gap view.
- All M0/M1 EXT dead-decls (FiberYield M0-EXT-07, M0-EXT-10, M1-EXT-07) should be wired-or-removed per the no-dead-declaration rule.

---

## 8. Part 1 — Applied fixes (explicit go-ahead received 2026-07-13)

### 8.1 T1.7 SpatialHash double-precision (was P0, already-broken)
**Evidence that the truncation was systemic, not two lines:** the public API carried `float x, float z` end-to-end (`SpatialHash.h:19,35,37,45,46,53,54`; `SpatialHash.cpp:7,18,28,90,108,177`), and `entityPositions` stored `pair<float,float>`. A 2-line `static_cast<double>` at `:134-135` would have narrowed back to float on the way into `entityPositions` and every `Insert`/`Query` call — a cosmetic fake fix.

**Fix:** widened the entire position chain to `double`:
- `SpatialHashKey(double x, double z)`, `entityPositions` → `pair<double,double>`, all method signatures `float`→`double`, `GetSubquadrantCoords`/`Rebucket` math on `double`.
- `Insert` re-bucket (lines 44–55) and `Rebucket` (lines 134–135) now read `transform->position.x/.z` straight off the `dvec3` as `double` — no narrowing.

**Regression check (callers):**
```
$ grep -rEn "sh\.Insert|sh\.QueryCell|SpatialHashKey\(" src/
src/core/Engine.cpp:647:  sh.Insert(e, t.position.x, t.position.z, &reg);   <- double (exact)
src/core/Engine.cpp:663:  sh.Insert(static_cast<entt::entity>(101), -0.2f, -0.2f);  <- float literal, implicit double conv OK
src/core/Engine.cpp:651:  auto cellEnts = sh.QueryCell(0, 0);
src/core/Engine.cpp:664:  auto negCellEnts = sh.QueryCell(-1, -1);
src/tests/Test_MathFormulas.cpp:19:  uint64_t k1 = ecs::SpatialHashKey(3.5f, 5.0f);  <- float literal, implicit double conv OK
src/tests/Test_MathFormulas.cpp:23:  uint64_t k2 = ecs::SpatialHashKey(-0.5f, -2.5f);
```
`t.position.x` is `double` (Components.h:10) — exact. Float literals implicitly convert to `double` (lossless). **Zero call sites regress.** Existing `[M1-EXT-01]` SpatialHash tests (`Test_MathFormulas.cpp:16,23`) compile unchanged. ECS.h:75 `spatialHash.Rebucket(registry)` unchanged signature.

**Diff (header, key lines):**
```diff
-inline uint64_t SpatialHashKey(float x, float z) {
-    int64_t cx = static_cast<int64_t>(std::floor(x / 2.0f));
-    int64_t cz = static_cast<int64_t>(std::floor(z / 2.0f));
+inline uint64_t SpatialHashKey(double x, double z) {
+    int64_t cx = static_cast<int64_t>(std::floor(x / 2.0));
+    int64_t cz = static_cast<int64_t>(std::floor(z / 2.0));
 ...
-    std::unordered_map<entt::entity, std::pair<float, float>> entityPositions;
+    std::unordered_map<entt::entity, std::pair<double, double>> entityPositions;
-    void Insert(entt::entity entity, float x, float z, entt::registry* registry = nullptr);
+    void Insert(entt::entity entity, double x, double z, entt::registry* registry = nullptr);
```

### 8.2 STATUS.md M1:EXT-07 correction
Diff applied (STATUS.md:59):
```diff
-8. ~~**M1:EXT-07** ... ~~ **CLOSED 2026-07-12**: `frameArena_` ... via `ArenaAllocateBump` ...~~
+8. ~~**M1:EXT-07** ... ~~ **CLOSED 2026-07-12** — *CORRECTION (2026-07-13, AUDIT_FOUNDATION_GAP.md §2 Tier-2): this closure is FALSE.* `grep -rn 'ArenaAllocateBump' src/` returns only the declaration in `ThreadArena.h:19`; `TriangleRenderer`'s `frameArena_` never calls it. Revert to OPEN.
```
Verifying grep (unchanged by the STATUS edit, which is a .md):
```
$ grep -rn 'ArenaAllocateBump' src/
src/core/ThreadArena.h:19:inline void* ArenaAllocateBump(BumpArena& arena, size_t size, size_t alignment = 16) {
```
Declared only — 0 call sites. Correction stands.

---

## 9. Part 2 (A) — Determinism audit (M2.8 co-op, M12 netcode assumption: lockstep-safe sim)

Standard: every claim carries the grep + literal output. Reachable-from-gameplay vs test-only stated per hit.

### A1 — Wall-clock / raw RNG seeding
```
$ grep -rEn 'rand\(|mt19937|mt19937_64|srand|std::random|time\(0\)|time\(NULL\)|chrono::system_clock::now|GetTickCount' src/
(no output — exit code 1)
```
**Finding:** No `rand()`, `mt19937`, `srand`, or wall-clock seeding anywhere in `src/`. The engine does NOT use C std RNG or wall-clock seeding. Good — first-class determinism risk absent.

### A2 — Declared RNG (the intended seed source)
```
$ cat src/core/PRNG.h   (key lines)
class SplitMix64 { ... explicit SplitMix64(uint64_t seed = 0) : state_(seed) {} ... };
inline thread_local SplitMix64 tls_rng;          // default-constructed -> seed == 0
inline void seed_thread_local(uint64_t seed) { tls_rng = SplitMix64(seed); }
inline uint64_t next_random() { return tls_rng.next(); }
```
**Finding:** A thread-local `SplitMix64` exists — deterministic algorithm (no float, no reordering sensitivity). **But it is never seeded** — see A9.

### A3 — Where is the RNG actually consumed?
```
$ grep -rEn 'PRNG|tls_rng|SplitMix64|splitmix|next_random' src/ --include=*.cpp --include=*.h
src/core/PRNG.h:8-36    (declaration only)
src/ecs/EntityFactory.cpp:4:  #include "core/PRNG.h"
```
**Finding:** `EntityFactory.cpp:4` includes PRNG.h but **the only consumer is `next_random()` at `EntityFactory.cpp:29`** (assigning `StableId::uuid`). So the RNG is consumed at spawn time. **Reachability:** gameplay-critical (every spawned entity gets a `StableId` from it). Currently the only seeded-path consumer.

### A4 — Float math sensitive to instruction reordering
```
$ grep -rEn 'fma|std::fma|fast-math|/fp:fast|-ffast-math|contract' src/ CMakeLists.txt
(no output — exit code 1)
```
**Finding:** No `-ffast-math`, no `std::fma`, no FP-contraction flags. No compiler-level reordering hazard introduced. The Jolt build defines `JPH_CROSS_PLATFORM_DETERMINISTIC` + `JPH_DOUBLE_PRECISION` (PhysicsSystem.h:3, PhysicsSystem.cpp:18) — Jolt's own solver is bit-deterministic by design.

### A5 — EnTT iteration order / group / sort
```
$ grep -rEn '\.group<|\.sort<|each<.*sort|storage_type_for|nth_element|std::sort' src/
(no output — exit code 1)
```
**Finding:** No explicit `group`/`sort`/`std::sort` over entity views in `src/`. EnTT `view<>` iteration order follows internal sparse-set storage order — deterministic for a given insert sequence, but NOT guaranteed stable across EnTT versions or insertion-order changes. **Latent risk for M2.8/M12:** if any future sim code iterates a `view<>` and assumes a stable order (e.g. to build a deterministic snapshot), it must sort by `entt::entity` id or `StableId` first. Currently no such code exists, so no live bug — but it's a trap M2.8 will step into.

### A6 — `unordered_map`/`unordered_set` in sim-reachable paths
```
$ grep -rEn 'unordered_map|unordered_set' src/ --include=*.cpp --include=*.h | grep -vi 'SpatialHash\|MetaRegistry\|MemoryBudget\|FileSystem\|tests/'
src/core/Config.h:24        std::unordered_set<std::string> overriddenFields;          (config, not sim)
src/core/CVarSystem.h:35     std::unordered_map<std::string, CVar> cvars;              (dev tool, not sim)
src/physics/PhysicsSystem.h:229  std::unordered_map<LineQuery, JPH::RayCastResult, LineQueryHash> queryCache_;
src/render/RenderGraph.cpp:23-24,74-75  std::unordered_map<VkBuffer/VkImage, ...>     (render-only)
```
**Finding — A6.1 (the real one):** `PhysicsSystem.h:229` `queryCache_` is an `unordered_map<LineQuery,...>`. Iterating it would be non-deterministic (hash order). **Reachability:** it is read via `.find()` (PhysicsSystem.cpp:135) — point lookups, not iteration — so order does not affect results today. **Risk:** if M2.8/M12 ever snapshots the cache for replication, hash order leaks. Flag, do not fix yet.
**A6.2:** SpatialHash's `headers`/`buckets`/`entityPositions` are `unordered_map` (SpatialHash.h:49-51) — but hash key is a computed `uint64_t` cell key, lookups are point-access, and `QueryCell`/`QueryRadius` return vectors by cell-walk, not map iteration. Deterministic for point queries.

### A7 — PhysicsSystem queryCache_ + Jolt determinism defines
```
$ grep -n 'queryCache_\|LineQuery\|LineQueryHash\|JPH_CROSS\|JPH_DOUBLE' src/physics/PhysicsSystem.h src/physics/PhysicsSystem.cpp
PhysicsSystem.h:3     // JPH_CROSS_PLATFORM_DETERMINISTIC and JPH_DOUBLE_PRECISION are defined in CMakeLists.
PhysicsSystem.h:229   std::unordered_map<LineQuery, JPH::RayCastResult, LineQueryHash> queryCache_;
PhysicsSystem.cpp:18  // JPH_CROSS_PLATFORM_DETERMINISTIC and JPH_DOUBLE_PRECISION enforced by CMake defines.
PhysicsSystem.cpp:65  LOG_INFO("PhysicsSystem: Jolt {} initialized. JPH_CROSS_PLATFORM_DETERMINISTIC=1 JPH_DOUBLE_PRECISION=1", ...);
```
**Finding:** Jolt is built deterministic+double (CMakeLists.txt:178-179 `JPH_CROSS_PLATFORM_DETERMINISTIC` / `JPH_DOUBLE_PRECISION`). This is the spec's hard requirement (M2). `LineQueryHash` is a custom hash for the cache key — consistent across peers as long as `LineQuery` fields equal. Point-lookup only.

### A8 — EnTTCache iteration determinism
```
$ grep -n 'pool\|storage\|component' src/ecs/EnTTCache.h
EnTTCache.h:38  auto* storage = &registry.storage<ComponentType>();
EnTTCache.h:41  const size_t entity_count = storage->size();
EnTTCache.h:45  (*rawData)[i] = reinterpret_cast<uint8_t*>(&storage->get(entities[i]));
```
**Finding:** `EnTTCache` indexes `storage->data()` — EnTT's dense array. Order follows storage insertion, deterministic per insert sequence. Used only in diagnostics today (Tier-2). No live sim-order hazard.

### A9 — Is the RNG ever seeded from a common root?
```
$ grep -rn 'seed_thread_local\|tls_rng =\|next_random' src/ --include=*.cpp
src/ecs/EntityFactory.cpp:29:        s.uuid = core::prng::next_random();
```
**Finding — the #1 determinism gap in this pass:** `tls_rng` is **never seeded**. `core::prng::SplitMix64` default-constructs with `seed == 0`, and `seed_thread_local()` is **never called anywhere in `src/`**. So every spawned entity's `StableId::uuid` is drawn from SplitMix64(0) — i.e. a **fixed, identical sequence every run**. That is *deterministic* (good for replay), but it means there is **no world seed / run seed** at all. M2.8 (deterministic co-op, "Deterministic Seed Distribution (XorShift128+)") and M4 (seeded world-gen) require a **single root seed that world-gen, entity spawn, and any sim noise all derive from**. There is no such root today. This overlaps **T1.1 (serialization)** and **T1.5 (world)** in the Tier-1 tables — the seed root is a missing piece they all depend on.
**Reachability:** gameplay-critical (every spawn). **Action:** design the seed root into Config (e.g. `--seed` CLI, default 0) and thread it to `seed_thread_local` + world-gen, before M2.8/M4.

### A10 — Config seed CLI?
```
$ grep -n 'seed\|Seed' src/core/Config.cpp src/core/Config.h
(no output — exit code 1)
```
**Finding:** No `--seed` option exists. Confirms A9: no seed root anywhere.

**Determinism verdict (not "probably fine" — concrete list):**
1. **[A9] No seed root** — `tls_rng` unseeded, no `--seed`/world seed; all spawns draw from fixed SplitMix64(0). Determinism-adjacent: reproducible, but no per-run/co-op seed. **Reachable: gameplay (every spawn).** FIX BEFORE M2.8/M4.
2. **[A5] EnTT view iteration order** — no sort/group today, but a latent trap: M2.8/M12 snapshot code must sort by `entt::entity`/`StableId` before serializing. No live bug.
3. **[A6.1] `queryCache_` unordered_map** — point-lookup only, no iteration; hazard only if M2.8/M12 snapshots it. Flag.
4. **[A1/A4] ABSENT** — no raw RNG, no wall-clock seed, no fast-math. Clean.
5. **[A7] Jolt** — deterministic+double built correctly. Clean.

---

## 10. Part 2 (B) — Boot-to-playable trace (single continuous chain, main.cpp → first player input)

```
src/main.cpp:13   core::Config::get().parseCommandLine(argc, argv);
src/main.cpp:16   core::Config::get().loadSettings();
src/main.cpp:19   core::Engine engine;          <- ctor runs, see below
src/main.cpp:20   engine.run();                  <- main loop
src/main.cpp:22   if (headless) engine.verifyHeadlessInit();
```

**Step 1 — Engine ctor (`Engine.cpp:46-144`):** real init.
- Crash handler, Logger, JobSystem, Input — real.
- `vulkanContext_` ctor — real Vulkan init (`Engine.cpp:86`).
- `ecsContext_` (ECSContext) — real (`Engine.cpp:90`).
- `setScene(ecsContext_)` wires registry → renderer (`Engine.cpp:95`) — real bridge.
- `eventBus_`, `PhysicsSystem::initializeGlobal()`, `physicsSystem_` — real (`Engine.cpp:98-108`).
- `DamageSystem::init(...)` — real (`Engine.cpp:110`).
- ImGui + MSDF (windowed-gated) — real in windowed.

**Step 2 — `engine.run()` → `mainLoop()` (`Engine.cpp:212-358` fixed-step accumulator):**
- `Input::poll()` (`Engine.cpp:260`) — real, reads raw `InputState`.
- Fixed-step `physicsTick()` (`Engine.cpp:332`) — real Jolt step.
- `ecsContext_->Tick()` quadtree rebucket (`Engine.cpp:354`) — real.

**Step 3 — World generation:** **SILENT NO-OP.** There is no world-gen call anywhere.
```
$ grep -rEn 'worldSeed|world_seed|WorldManager|WorldGen|GenerateWorld|generateWorld' src/ --include=*.cpp --include=*.h
(no output — exit code 1)
```
**Chain breaks here for "playable":** M4 (procedural world + chunk streaming) is not built. `mainLoop` never calls any `GenerateWorld`. No terrain, no buildings — nothing to stand on. *This is T1.5 (P2) and the missing M4.*

**Step 4 — Entity spawn:** **DIAGNOSTIC-ONLY, not a gameplay spawn.**
```
$ grep -n 'LoadPrefab\|Spawned 3 live\|spawnDevTestBody_' src/core/Engine.cpp
Engine.cpp:272   spawnDevTestBody_();          (devMode && !headless only)
Engine.cpp:367   if (frameCount == 5 ...) auto entities = ecs::EntityFactory::LoadPrefab(reg, "assets/prefabs/test_prefab.json");
Engine.cpp:388   LOG_INFO("Spawned 3 live entities for testing.");
```
- `frameCount == 5` spawns `test_prefab.json` (3 entities) — a hardcoded test fixture, runs unconditionally at frame 5.
- `spawnDevTestBody_()` spawns one Jolt box — devMode only.
- `LoadPrefab` → `EntityFactory::CreateEntity` → assigns `StableId` via `next_random()` (`EntityFactory.cpp:29`). Real, but it's test scaffolding, not a player or a horde.
- **NO player entity is ever spawned** (confirmed B5 below).

**Step 5 — First render frame:** real, but renders only test/dev entities.
```
$ grep -n 'renderFrame\|acquireAndPresent\|->draw' src/render/VulkanContext.cpp src/render/Swapchain.cpp
VulkanContext.cpp:222  void VulkanContext::renderFrame(debug::ImGuiOverlay* imguiOverlay) {
VulkanContext.cpp:238  swapchain_->acquireAndPresent(imguiOverlay, materialSystem_.get());
Swapchain.cpp:177  void Swapchain::acquireAndPresent(debug::ImGuiOverlay* imguiOverlay, MaterialSystem* materialSystem) {
-> TriangleRenderer::draw(cmd, imageIndex, materialSystem)  (TriangleRenderer.cpp:621)
```
`TriangleRenderer::draw` (`TriangleRenderer.cpp:621`):
- `devTestMesh_` path (`:630-663`) — hardcoded dev cube, real draw.
- ECS bridge (`:672-695`) — `view<Transform, MeshComponent>` traversal, real draw of the frame-5 prefab entities.
- **Render works. It renders the 3 test prefab entities + dev cube. No world, no player.**

**Step 6 — First player input:** **HARD BREAK — no player, no input→movement mapping.**
```
$ grep -rEn 'player|movement|velocity|WASD|controller|input->' src/ --include=*.cpp | grep -vi 'devTestBody|dev-test|spawnDevTest|//'
Engine.cpp:261  if (Input::getState().quit) { running_ = false; }     <- only INPUT consumption = quit key
Engine.cpp:265  for (const auto& ev : Input::getState().events) { imguiOverlay_.ProcessEvent(&ev); }  <- ImGui only
FlyCamera.cpp:17  const auto& s = core::Input::getState();            <- debug fly-cam, dev-gated
```
- The **only** gameplay input consumed is the quit key (`Engine.cpp:261`). Everything else is ImGui events or dev-only `FlyCamera`.
- `FlyCamera` (M2.6 player-analogue) is **dev-gated** (`#if ENGINE_DEV_TOOLS`, `Config::flyCamera`) and is a camera, not a player entity — it never writes a `Transform` to any ECS entity; it only feeds `vulkanContext_->setDevView(...)` (`Engine.cpp:313`).
- **There is no player entity, no movement system, no input→Transform path.** This is T1.6 (P0). The boot-to-playable chain terminates here: you can render test cubes and fly a dev camera, but nothing is "a player" and no key moves anything in the ECS world.

**Boot-to-playable verdict (where the chain breaks):**
- Init → physics → ECS → render: **REAL**, works.
- World gen: **NO-OP** (missing M4, T1.5).
- Entity spawn: **test fixtures only**, no player (T1.6).
- Player input→movement: **HARD BREAK** (no player entity, no movement system, T1.6).
- **First true break for "playable" = Step 4/6: no player entity + no input→movement.** Boot-to-render-of-test-cubes works; boot-to-playable does not exist.

---

## 11. Part 2 (C) — Cross-system consistency at generation time (single seed root?)

Question: does anything validate a spawned entity's transform lands inside generated collision geometry? Is there ONE seed-derived source of truth for world-gen (M4), destructibles (M3), horde spawns (M5), audio zones (M6)?

### C1 — Seed sources in each subsystem
```
$ grep -rEn 'worldSeed|world_seed|WorldManager|WorldGen|seed' src/ --include=*.cpp --include=*.h
src/core/PRNG.h:12     explicit SplitMix64(uint64_t seed = 0)
src/core/PRNG.h:30     inline void seed_thread_local(uint64_t seed) { tls_rng = SplitMix64(seed); }
```
**Finding:** The ONLY seed primitive is `PRNG.h`'s `SplitMix64`/`seed_thread_local`. No subsystem has its own seed variable. But (C2) it is never invoked.

### C2 — Where does each system pull seed/RNG from?
- **World-gen (M4):** does not exist (C1 empty for world). No `WorldManager`, no `GenerateWorld`. When built, there is no seed to hand it.
- **EntityFactory / spawn (M5 horde spawn target):** `EntityFactory.cpp:29` `s.uuid = core::prng::next_random();` — pulls from `tls_rng`, which is **unseeded** (A9). So spawns draw from SplitMix64(0).
- **Destructibles (M3):**
  ```
  $ grep -n 'seed\|rand\|PRNG\|deterministic' src/ecs/Destructible.cpp
  (no output — exit code 1)
  ```
  No RNG in `Destructible.cpp`. Destructible break thresholds are data-driven (component fields), not seeded — deterministic by construction, but also not tied to any world seed.
- **EventBus / DamageSystem (M5 spawn→damage):** no RNG (verified, exit code 1).
- **PhysicsSystem (M2, the sim core):** no RNG in `PhysicsSystem.cpp/.h` (A7); Jolt is deterministic.
- **Audio (M6):** not built (Tier-1 NEW-area earlier; no audio subsystem at all).

### C3 — Is there a common seed root?
```
$ grep -n 'seed\|Seed' src/core/Config.cpp src/core/Config.h
(no output — exit code 1)
```
**Finding — definitive:** There is **no common seed root**. `Config` has no `--seed`; `tls_rng` is never seeded; no subsystem owns a seed. Each subsystem that will need randomness (M4 world-gen, M5 horde spawn pacing, M5.4 scent/fear fields, M8 factions, M13 text) currently has **no RNG wired at all** except the single unseeded `next_random()` in `EntityFactory`.

### C4 — Transform-in-collision validation at spawn?
```
$ grep -rEn 'ValidateSpawn|insideCollision|overlap.*spawn|spawn.*valid|placement.*check' src/ --include=*.cpp
(no output — exit code 1)
```
**Finding:** No spawn validation exists. When world-gen (M4) and hordes (M5) both exist, there is currently **nothing** that checks a spawned entity's `Transform` lands on valid (collision-present) ground versus inside a wall or off-map. This is a missing cross-system invariant — it belongs to whoever owns the spawn pipeline (M5 director / M4 chunk loader), and needs a shared "is this cell walkable / is there floor here" query that today doesn't exist because there's no world.

**Cross-system consistency verdict:**
- **No common seed root.** M4/M5/M5.4/M8/M13 will each need RNG; today only `EntityFactory` (unseeded) touches it. Risk: each subsystem seeds independently → worlds, hordes, loot, and text **drift out of sync across co-op peers** (M2.8/M12) and across save/reload (M7). This is exactly the M2.8 "Deterministic Seed Distribution" requirement with no substrate yet.
- **Overlaps existing rows:** this is the missing spine behind **T1.1 (serialization — save must capture the seed)**, **T1.5 (world — world-gen consumes the seed)**, and **A9 (determinism — unseeded RNG)**. All three should thread a single `worldSeed` from `Config` → `seed_thread_local` + world-gen + every future RNG consumer.
- **No spawn/transform↔collision validation** — missing invariant, gated behind M4 (no world to validate against yet).

---

## 12. Net additions from Part 2 (cross-referenced to Tier tables)

| New finding | Severity | Overlaps |
|---|---|---|
| **[A9/C3] No seed root** — `tls_rng` unseeded, no `--seed`, no subsystem seed | **P1** (blocks M2.8/M4/M5 determinism) | T1.1, T1.5, A9 |
| **[A5] EnTT view iteration order** — latent trap for M2.8/M12 snapshots | P2 (flag) | — |
| **[A6.1] queryCache_ unordered_map** — hazard only if snapshot-replicated | P2 (flag) | — |
| **[B] Boot-to-playable breaks at Step 4/6** — no player entity, no input→movement | **P0** (confirms T1.6) | T1.6 |
| **[C4] No spawn↔collision validation** | P2 (gated on M4) | T1.5 |
| **[A1/A4] Raw RNG / fast-math** | CLEAN | — |
| **[A7] Jolt determinism** | CLEAN | — |

*Part 1 fixes applied (T1.7 SpatialHash double-precision + STATUS.md §8 correction). Part 2 is read-only; appended as sections 8–12. No other source/git/build state modified.*

---

*Read-only audit except the two authorized Part 1 fixes (T1.7 SpatialHash, STATUS.md §8 correction).*

---

## 13. Build-environment note — build-asan ASan runtime DLL (NOT a code defect)

**Symptom observed 2026-07-13:** `build-asan/Release/ZombieEngine.exe --dev` exited with code 1 and produced a **zero-byte log** (no stdout/stderr at all, crash before `main()` logged anything). Exit code under direct launch was **-1073741511 = 0xC0000139 = STATUS_ENTRYPOINT_NOT_FOUND**.

**Root cause:** `build-asan` is configured with `ENGINE_SANITIZE:STRING=ASAN` (CMakeCache.txt). The exe links the **MSVC-bundled clang** ASan runtime (`VC/Tools/MSVC/<ver>/bin/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll`). If a **mismatched** ASan DLL is colocated in `build-asan/Release/` (e.g. the standalone **LLVM 22** `clang_rt.asan_dynamic-x86_64.dll`, which is a *different* file — 528896 B vs 1193496 B, different exports), the loader binds the exe's ASan imports to the wrong DLL, hits a missing entry point, and faults before any log output.

**Fix (on the GPU machine):** remove the wrong colocated DLL and copy the **MSVC-clang** one next to the exe:
```
Remove-Item build-asan\Release\clang_rt.asan_dynamic-x86_64.dll
Copy-Item "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\clang_rt.asan_dynamic-x86_64.dll" build-asan\Release\
```
After fix: `--dev --dump-frame-png` runs clean, dev mesh loads (`VERIFICATION SUCCESS: Loaded dev_test_cube.glb. Mesh handle index: 0`), cube renders.

**Do NOT re-investigate as a rendering/Path-A bug.** This entire "missing cube" saga (the phantom `git diff` showing Path A removed + the empty `after_fix_f8.png`) traced to this broken build-asan runtime, NOT to any draw-path or asset-load defect. Path A (`TriangleRenderer.cpp:627-699` + `VulkanContext.cpp:143-148`) is correct and was never broken. The pre-fix PNGs/logs were produced from a corrupted build environment.

**Note:** the DLLs in `build-asan/Release/` are gitignored build artifacts — no commit needed; this is a local-machine environment fix only.

---

## 14. Canonical Debug build — pre-existing CRT link failure (separate item, do NOT fold into T1.7)

**Symptom:** `cmake --build build --config Debug --target ZombieEngine ZombieEngineTests` fails at **link** with unresolved externals from prebuilt static libs:
- `Catch2d.lib` → `__imp__CrtDbgReport`
- `vk-bootstrap.lib` → `_calloc_dbg`, `_free_dbg`
- `spirv-reflect-static.lib` → (same debug-CRT family)

i.e. those libs were built/linked against the **debug CRT** (`/MDd`), while the engine target links **release CRT** (`/MD`) — a mismatch, so the debug-CRT symbols are undefined.

**Proven pre-existing (unrelated to T1.7):** with `src/ecs/SpatialHash.cpp`, `src/ecs/SpatialHash.h`, and `STATUS.md` **stashed** (changes removed), the Debug build failed with the **exact same 3 unresolved symbols**. T1.7's float→double widening therefore cannot be the cause. The user explicitly directed: **do NOT fix this now** — log as its own item only.

**Verification status:** the `build-asan` (Release + ASan) config links + runs GREEN (canonical `ZombieEngineTests` suite: 12 cases / 61 assertions per user sign-off) against the widened double signatures, so the *logic* is validated; only the canonical `Debug` link config is broken.

**Recommended fix (when scoped separately):** align CRT linkage — either build the three static deps with `/MD` to match the engine, or set the engine Debug target to `/MDd`. Do not bundle into any feature commit.

**Scope note:** this is a build-system/config defect, NOT a code defect and NOT part of F1/M2.6. Keep it isolated from the F1 source commit.
