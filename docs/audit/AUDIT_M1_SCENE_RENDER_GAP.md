# AUDIT_M1_SCENE_RENDER_GAP.md — Major Finding

**Classification:** Major Finding (planning/scoping deliverable). No implementation in this pass. No commits.
**Status:** INVESTIGATION COMPLETE — deliverable for review. All downstream fixes deferred pending explicit go-ahead.
**Convention:** permanent paper trail, same class as `AUDIT_M0-M2.md` / `AUDIT_FIX_STATUS.md`. Update in place; never delete; reference in future handoffs.

> **RESOLVED — finding obsolete as of 2026-07-15.** At audit time the scene renderer was genuinely unbuilt. It has since been completed: commit `de97881` ("F1 mesh-swap: complete ECS bridge + render path and GPU-verify shattered geometry") added the ECS→render bridge. `git grep` confirms `TriangleRenderer::draw` now traverses `view<ecs::Transform, ecs::MeshComponent>()` (line 690) and issues `vkCmdDrawIndexed` per entity. The below "Header Finding" is historically accurate but no longer describes the current tree. The M1-EXT-03 occlusion mislabel described in `AUDIT_TRIANGLE_RENDERER_EXT03_MISLABEL.md` was a separate, genuine defect and has been fixed (retagged to `M1-EXT-28`).

---

## Header Finding (stated plainly)

**Major Finding: M1 exit criteria not actually met — the scene renderer was never built.**

The engine has a fully functioning Vulkan swapchain, a real compute-cull pass, an indirect-draw pipeline, a GLTF `AssetManager`, and a Jolt/ECS simulation — but **nothing drives geometry from the ECS**. `TriangleRenderer` draws one hardcoded triangle instanced 100× at hardcoded grid positions through an identity MVP and a hardcoded 800×600 viewport. No code path reads `Transform`/`Mesh` components to produce draws. Every "scene" the window shows is demo placeholder data (the "4 static gradient triangles" observed eyes-on). M2 was built and verified on top of this gap without the gap being caught.

---

## Explicit Non-Implication Statement

**M2's physics/ECS/EventBus work is independently real and verified and is NOT cast into doubt by this finding.**

Specifically, the following M2 systems were exercised on real hardware (RTX 2070 SUPER) and pass:
- Fixed-timestep physics loop (`Engine::physicsTick`, `PhysicsSystem::step`) — verified.
- EventBus damage routing (`DamageSystem::tick`, `DamageEvent` enqueue) — verified (150 dmg → destroy → mesh swap to handle 2 → collider removal; see `AUDIT_M0-M2.md` "Positive verifications").
- Destructible mechanism (`Destructible.cpp` mesh-swap + `PhysicsSystem.cpp` collider removal) — verified live.
- Soft-asset graceful degradation (`LoadTexture` fallback) — verified.

This finding identifies a **separate, earlier gap one milestone up (M1)**. M2 logic is correct; it has nothing to draw *into*. Do not re-open M2 verification on account of this.

---

## Root Cause (every claim is a direct file:line citation)

The renderer is a self-contained demo. None of it is ECS-driven.

1. **Hardcoded single triangle geometry.** `src/render/TriangleRenderer.cpp:57-61` — literal 3-vertex array, written once into `vertexBuffer` in `createBuffers`. Never updated post-init.
2. **Hardcoded 100-instance grid positions.** `src/render/TriangleRenderer.cpp:83-89` — `instances[i].pos_rad` set to a fixed `(i%10)*1.5-7, (i/10)*1.5-7` grid. Baked once; never updated.
3. **Identity MVP in both compute (cull) and graphics (draw).** `TriangleRenderer.cpp:502-503` (cull push-constant `pc.mvp` set to identity) and `Triangle.cpp:590-591` (draw push-constant `pc.mvp` set to identity). No camera ever feeds these.
4. **Hardcoded 800×600 viewport + scissor.** `TriangleRenderer.cpp:575-582` — `viewport.width/height = 800/600`, `scissor.extent = {800,600}`. Independent of the actual window/swapchain extent (`Swapchain.cpp` acquires real `vkbSwapchain_.extent` at line 282 but the renderer ignores it).
5. **Only draw call issues static indirect buffer.** `TriangleRenderer.cpp:603` — `vkCmdDrawIndexedIndirectCount(cmd, indirectBuffer[imageIndex], 0, countBuffer[imageIndex], 0, 100, ...)`. `indirectBuffer` is created once in `createBuffers` (`TriangleRenderer.cpp:97-113`) and **never written after init** — the compute cull pass writes to it, but its *input* instance data is the static grid from #2.
6. **No ECS scene query exists anywhere in `src/`.** Grepped `registry.view<Transform, Mesh>`, `view<Mesh>`, `GetRegistry().view` — **zero matches** for a render query. The only `registry.view<>` calls are:
   - `src/core/Engine.cpp:330` and `:352` — both inside **self-test scaffolding** (`frameCount==6` cache-verify, `frameCount==10` SPSC-queue test), not rendering.
   - `src/physics/PhysicsSystem.cpp:106` and `:222` — physics (`PhysicsBodyComponent`), not rendering.
7. **Loaded meshes are never fetched — and none are ever loaded.** `AssetManager::LoadMesh(ecs::Handle)` is declared (`src/render/AssetManager.h:16`) and defined (`src/render/AssetManager.cpp:275`), populating `MeshAsset.vertexBuffer` (`AssetTypes.h:87`) on success — **but it has zero callers anywhere in `src/`** (grep-confirmed: only `AssetManager.cpp`/`.h` reference it). No mesh load has ever executed, so `meshes_` is empty at runtime and **no GLTF geometry resides in GPU buffers**. `GetMesh` (`AssetManager.cpp:410`) therefore returns null for every handle. *(Correction v77.1/A1-precondition, 2026-07-12: the earlier draft's claim that "loaded GLTF geometry exists in GPU buffers" was factually wrong — see Slice-0a A1 remediation, which adds the first-ever `LoadMesh` invocation as a dev-test asset-manager self-test.)*
8. **No render/mesh component for general entities.** Grepped `struct Mesh`/`MeshComponent`/`class Mesh`/`using Mesh` → only `Destructible.h:13` (`using MeshHandle = uint32_t`) and `Destructible.h:17-18` (`intactMeshHandle`/`destroyedMeshHandle`). There is **no `MeshComponent`** (or any render-handle-bearing component) that a scene render path could iterate. A general entity to be drawn has no field declaring "draw mesh N."
9. **Render entry point receives no scene context.** `Engine.cpp:431` — `vulkanContext_->renderFrame(&imguiOverlay_)` passes only the ImGui overlay; no ECS registry, camera, or scene reference crosses into the renderer. `VulkanContext::renderFrame` (`VulkanContext.cpp:188-199`) → `swapchain_->acquireAndPresent(imguiOverlay, materialSystem_.get())` (`VulkanContext.cpp:197`) is a pure pass-through to the demo.
10. **Debug overlay uses its own hardcoded camera, not the scene's.** `Engine.cpp:409-411` — `glm::lookAt((0,5,20),(0,0,0))` + `glm::perspective(60°)` hardcoded. Even if hull lines are generated (`PhysicsSystem::drawBodies` → `getLines()`), they are projected through a fixed off-scene camera and drawn on the ImGui background list, floating over the demo triangle — never tracking actual entity positions/camera.

**Absence assertions (grep-confirmed, not inferred):** "No `registry.view<Transform,Mesh>` in src/" — confirmed via grep, 0 matches. "GetMesh has no callers" — confirmed via grep, 0 matches. "No MeshComponent" — confirmed via grep, only Destructible.h MeshHandle.

---

## Cross-Milestone Blast-Radius Audit

**Method:** for each milestone, isolate the *rendering-dependent* portion of its exit criteria (cited to GDD line) from the logic/sim portion, and grade only the rendering portion with the project vocabulary (VERIFIED / IMPLEMENTED-UNVERIFIED / NOT STARTED). Logic portions are noted as separately fine.

| Milestone | GDD exit-criterion line(s) | Rendering-dependent criterion | Status (render portion) | Evidence / note |
|---|---|---|---|---|
| **M0** | Tooling claim `endless_quarantine_v77_cleaned.md:195` ("RenderDoc-capturable, confirmed at M0"); exit criteria `:287-294` | Swapchain RenderDoc-capturability; "window opens / validation clean / swapchain survives resize" | **VERIFIED** (capturability) but **content = NOT STARTED** | M0 exit criteria (:287-294) are scene-agnostic (init/validation/swapchain/caps/pipeline-cache/crash-handler/VMA) and genuinely pass. The `:195` "RenderDoc-capturable" claim is *technically true* — the swapchain IS capturable — but the only thing ever captured is the hollow demo triangle (#1–#5 above). So RenderDoc-capturability was confirmed against demo geometry, not a scene. **Precision defect, not a criteria failure:** M0's own exit criteria do not require a scene; the contamination is the GDD tooling bullet at `:195`, which implies scene-level capturability that was never real. |
| **M2.7** | `:1233-1238` | "Seamless 1st/3rd-person camera" (visible); "Ballistic drop/penetration/ricochet VISIBLE"; "Sway/recoil/IK-limp VISIBLE" | **NOT STARTED** | All three "visible" criteria require a renderer drawing the player model + world. Controller *movement* (Jolt `CharacterVirtual`) is sim logic and is IMPLEMENTED-UNVERIFIED headless, but its *visible* confirmation is NOT STARTED. No camera entity and no player model render path exist. |
| **M2.9** | `:1952-1957` | Visual traversal (vault/swim against geometry); visible ballistic/hallucination feedback | **NOT STARTED** (visual) | Mechanics (vault physics, critical-power depletion, sleep-debt, BCS carry limits) are sim/logic and IMPLEMENTED-UNVERIFIED headless. But "traverse geometry" and "visible" feedback presume rendered geometry, which does not exist. Render-dependent visual confirmation = NOT STARTED. |
| **M3** | `:2247-2251` | "Dozens of structures destructible" (visually breaks); structural collapse visible | **NOT STARTED** (visual) | Sim portion is real and partly verified: `Destructible` mesh-swap + collider-removal path proven live in M2; BFS graph search and nav-grid are logic. The *visual* disappearance/fracture of structures on screen = NOT STARTED. |
| **M4** | `:2560-2563` | "Player traverses chunks with no stutter" (visual); "Urban layouts read as logically zoned" (visual read) | **NOT STARTED** (visual) | World-gen logic (sector gen, zoning tags, macro-graph) is IMPLEMENTED-UNVERIFIED headless. The generated world has no render path, so "traverse without stutter" and "layouts read as zoned" cannot be visually confirmed. |

**Other milestones with implicit render dependency (found via grep, not stopping at the list):**
- **M2.6** (`:1198-1203`): "Flying the debug fly-camera … produces no visible jitter/pop" and "origin rebase … produces no visible pop" are explicitly visual → NOT STARTED (visual). Logic (dvec3 Transform, StableId) is fine. See M2.6 gate below.
- **M5.2** (animation, skinned-character rendering, `:8811` transient GPU skinning cache): full NOT STARTED on render side — depends on a character mesh draw path that doesn't exist.
- **M4.5 / M4.6** (rendering-owned streaming/VRAM budgeting, per GDD): NOT STARTED — these *are* render-milestones and were presumably also assumed done on the same hollow demo.
- **M1 itself** (the render milestone): its "renders a scene" criterion is the subject of this finding — NOT STARTED at the scene level; only the demo pipeline exists.

**Systemic pattern (the important output):** This is **not a contained one-milestone gap**. Every milestone M0→M4+ that lists a "visible" / "renders" / "traverses" / "reads as zoned" exit criterion has had that criterion **silently unverifiable** because no renderer was ever wired to ECS. The logic/sim in those milestones is largely genuine, but their *visual* exit criteria were passed (or assumed passable) on a premise — "the renderer shows the world" — that was never true. The pattern is: **milestones marked done on logic while their visual criteria were never checkable.** This must be called out at each milestone's next gate, not retro-fixed silently.

---

## Remediation Plan (scoping only — not to be implemented without go-ahead)

### Concrete build items

**R1 — Real ECS render query + Mesh component.** Add a `MeshComponent { MeshHandle handle; }` (or reuse `DestructibleComponent`'s handle for destructibles and add a plain one for static props) and a render path that iterates `registry.view<Transform, MeshComponent>` (or `Transform` + any component carrying a `MeshHandle`). **Size: MEDIUM.** Note: a `Mesh` render component does not currently exist (root cause #8) — this is a net-new component, not a refactor.

**R2 — Per-frame transform/instance upload.** Replace the static `instanceBuffer`/`indirectBuffer` (baked once, `TriangleRenderer.cpp:82-113`) with a per-frame write of each visible entity's `Transform` (world→camera-relative per M2.6 `:1185`, `renderPos = entityPos - cameraPos`) into a GPU instance buffer the indirect draw consumes. **Size: MEDIUM.**

**R3 — Real camera (view/proj).** Add a `Camera` (free-fly debug camera per M2.6 `:1187`, or a fixed scene camera) producing `view`/`proj` matrices fed into the `pc.mvp` push-constant at `TriangleRenderer.cpp:590-591` and the viewport/scissor at `:575-582` sourced from the real swapchain extent (`Swapchain.cpp:282`). **Size: MEDIUM.** No camera entity currently exists.

**R4 — MeshHandle → vertex/index buffer binding.** `GetMesh()` (`AssetManager.cpp:410`) already returns `MeshAsset.vertexBuffer`/`indexBuffer` (`AssetTypes.h:87`) but has **zero callers** (root cause #7). Route the draw through the per-entity `MeshHandle` → `GetMesh(handle)` → bind that asset's buffers instead of the hardcoded demo `vertexBuffer`/`indexBuffer`. **Size: SMALL–MEDIUM** (binding plumbing is small; the asset system itself is already built — only unwired).

**R5 — Debug-draw overlay through the real camera.** Replace the hardcoded `lookAt((0,5,20),(0,0,0))` + `perspective(60°)` at `Engine.cpp:409-411` with the real camera's `view`/`proj` so hull lines track entity positions. **Size: SMALL** (projection swap only) — but **meaningful only after R3 lands.**

### Dependencies (linear critical path for solo dev)
```
R3 (camera) ──┬──> R1 (MeshComponent + query)
              └──> R2 (per-frame upload)   ──> R4 (MeshHandle bind)  ──> R5 (overlay reproject)
```
R3 must land before R1/R2/R5 are meaningful (no matrices → nothing to project). R1+R2+R4 together turn "4 demo triangles" into "actual ECS entities drawn." R5 is a leaf that rides on R3.

### Sequencing — smallest first slice that unblocks the most verification, fastest
**Slice 1 (recommended first commit):** R3 (minimal free-fly camera) + R1 (MeshComponent) + R2 (upload transforms for `view<Transform, MeshComponent>`) + R4 (bind `GetMesh(handle)` instead of demo buffer) as one cohesive change. Result: the window shows **actual ECS entities** (the dev-test body from the M2 #4 hook, plus any spawned entities) instead of demo triangles.

This single slice immediately unblocks:
- **M2 #3** (mesh-swap visible — destructible entity's geometry actually changes),
- **M2 #4** (hull overlay visible *over real geometry* — then R5 can follow),
- **M2.6 visual** (`:1200-1202` fly-camera jitter/rebase pop check becomes observable),
- **M2.7/M2.9/M3/M4 visual** confirmation on real content.

R5 (overlay reproject) is a fast follow-up Slice 2 once Slice 1 is eyes-on confirmed.

### Owning milestone
This work belongs to **M1** (the render milestone — referenced throughout GDD as the rendering owner, e.g. `AUDIT_M0-M2.md` "M1's 'No validation warnings' criterion", `M1-EXT-03/04/11` IDs). It should have been built *under M1*, not bolted on later.
**Flag:** the *asset/streaming* home for `MeshHandle` mapping is properly **M4 / M4.6** per M2.6 `:1188` (glTF Geometry Caching, LRU eviction against M4.6 VRAM budget). R4's binding is M1's job; the *cache/eviction* policy around it is M4's. Keep R4 as a thin M1 bind; do not pre-build M4 streaming here.

---

## M2.6 Gate — Explicit Answer

**Answer: PARTIAL.** M2.6 is **not fully hard-blocked**, but its visual exit criteria cannot be verified without a renderer.

- **Renderer-independent (can start / is fine now):**
  - `:1200` — dvec3 `Transform` standardization, zero bare-float world-position call sites (logic, headless-verifiable).
  - `:1203` — StableId survives save/reload + worldSeed re-derivation (logic; M7 persistence, headless-verifiable).
- **Renderer-dependent (must wait for Slice 1 above):**
  - `:1201` — "Flying the debug fly-camera … no visible jitter/pop; origin rebase … no visible pop." Requires a camera + rendered world to *see*. Hard-blocked on visual.
  - `:1202` — "Two adjacent sectors' terrain shows no visible seam; rolling barrel crossing boundary picked up by exactly one sector." The barrel's *simulation* handoff is logic (verifiable headless); the *visible* seam/continuity check requires rendered terrain. Partially blocked (sim verifiable, visual not).

**Conclusion:** M2.6 logic can proceed now; its two visual criteria stay gated until the M1 scene renderer (Slice 1) exists. Do not mark M2.6 "done" on the strength of logic alone.

---

## GDD Documentation Proposal (PROPOSED ONLY — not applied; diffs for review)

> These are review diffs. Nothing in `endless_quarantine_v77_cleaned.md` or `STATUS.md` is modified by this audit. Apply only after explicit approval.

**(a) Changelog entry** (insert after `## Changelog — v77` header at `:3`, matching existing `## Changelog — vNN` convention):
```markdown
## Changelog — v77.1 (M1 scene-render gap audit)
* **[Audit]** Major Finding: M1's scene renderer was never built. `TriangleRenderer` draws one
  hardcoded triangle instanced 100× through an identity MVP + fixed 800×600 viewport; no ECS
  `view<Transform,Mesh>` path, `GetMesh()` has zero callers, no `MeshComponent` exists. All
  "visible"/"renders" exit criteria across M0–M4+ were silently unverifiable. M2 physics/ECS/
  EventBus logic remains verified and unaffected. Corrective pass scoped under M1 (see
  AUDIT_M1_SCENE_RENDER_GAP.md). No code changed in this audit.
```

**(b) M1 GDD-section re-verification flag** (proposed insertion into M1's own section — cite the M1 milestone file when located; text form):
```markdown
> ⚠ RE-VERIFICATION REQUIRED — M1 exit criteria were marked met on a hollow demo renderer
> (`TriangleRenderer` placeholder, no ECS scene path). All "renders a scene / visible / draws
> entities" criteria are NOT STARTED until the M1 scene-render corrective pass lands. See
> AUDIT_M1_SCENE_RENDER_GAP.md.
```

**(c) M0 tooling-bullet correction** (at `endless_quarantine_v77_cleaned.md:195`, amend the "RenderDoc-capturable, confirmed at M0" bullet):
```markdown
- **RenderDoc-capturable** — swapchain IS capturable (verified against the M1 demo-triangle
  placeholder). NOTE (v77.1): this confirms only swapchain capturability, NOT scene content —
  the scene renderer was never built (AUDIT_M1_SCENE_RENDER_GAP.md). Re-confirm against real
  scene geometry after the M1 corrective pass.
```

---

## Guardrails (non-negotiable, per standing rules)

- **No implementation in this pass.** Investigation + documentation + planning only.
- **No commits.** All new/changed files stay local, uncommitted, for review. (`AUDIT_M1_SCENE_RENDER_GAP.md` is new/untracked; no other files touched.)
- **Did not touch** `.clang-tidy`, `IDEA.md`, the stray Temp file, or any file unrelated to this task.
- **Every factual claim is traceable** to a `file:line` citation or a specific GDD line number above — no unsourced assertions.
- **Plan/code-adjacent changes wait for explicit go-ahead.** Next action (e.g. "implement Slice 1", "apply GDD diffs") requires your yes.

---

## Definition-of-Done (for this audit task)

- [x] `AUDIT_M1_SCENE_RENDER_GAP.md` written — root cause fact-checked with `file:line`, non-implication statement present.
- [x] Cross-milestone contamination table complete (M0, M2.6, M2.7, M2.9, M3, M4 + M5.2/M4.5/M4.6 flagged).
- [x] Sequenced solo-dev remediation plan with sizes + dependencies + owning milestone.
- [x] Explicit M2.6 gate answer (PARTIAL, with renderer-independent vs dependent split).
- [x] Proposed (not applied) GDD diffs (changelog + M1 flag + M0 correction).
- [x] No commits; no unrelated files touched; guardrails honored.
