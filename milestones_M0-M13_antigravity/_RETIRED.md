# Retired / Superseded Systems (audit records — NOT built)

> These were design notes from earlier audit passes that are already satisfied or renumbered. Retained for traceability only; they are excluded from the 707 buildable systems and from the build-loop gate.


---

#### [M0-EXT-14]

> **STATUS: record-only — do NOT build.** Audit/superseded record; retained for traceability, not a system to implement. Asynchronous Hardware Compute Interleaver & Pipeline Lifecycle Matrix

*(ID note: EXT-13 does not exist in this milestone — EXT-14 was renumbered up from a colliding EXT-11 during a later merge pass and the doc's convention is to never reuse or backfill a retired number. This is expected, not a gap.)*

##### Systems Touched

M0 capability tiering, `[M4.5-EXT-07]` procedural texture synthesizer, M13 SLM worker threads.

##### How It Works

Background compute work (texture synthesis, SLM inference) submits to a dedicated `VK_QUEUE_COMPUTE_BIT` queue instead of the primary graphics queue, coordinated with cross-queue timeline semaphores. This is a standard, well-supported Vulkan pattern on any GPU with an async compute queue family (the RTX 2070 Super dev-reference card has one) — it's the correct fix for background work stalling the frame budget, and it's cheap to add now before more systems assume inline execution.

##### Math
phase = (submit - complete) / frameBudget; interleave = computeDuring(phase==idle)

##### Reference Implementation
```cpp
if(phase==idle) vkCmdExecuteCommands(computeQ, interleaveCmd);
```
##### Player-Facing Impact

Texture streaming and (if M13's SLM system is kept) SLM inference stop causing frame hitches.


---

## Milestone Sequencing

**Current milestone:** M0

Do not proceed past this file's Exit Criteria until every item passes verification by actually running the build — this is not a read-the-code-and-assume-it-works check. In particular: boot on real hardware (not just headless CI), resize/minimize the window during a session, and force `--force-tier0` at least once to confirm the fallback path actually initializes cleanly rather than merely being detected.

**M0 completeness audit (v77 cross-check):** this file's Extended Systems Library contains all thirteen M0-EXT items that exist anywhere in the v77 source doc — `[M0-EXT-01]` through `[M0-EXT-12]`, plus `[M0-EXT-14]` (there is no `[M0-EXT-14]`; that number was retired during a renumber and the doc's convention is to never reuse a retired ID). If any implementation already in the codebase references an `M0-EXT-*` ID not listed here, treat that as a signal the codebase drifted from a stale/pre-cleanup copy of this doc, not as a real system to reverse-engineer a spec for — flag it for review instead of guessing at intent.

**Once Exit Criteria pass:** load the next milestone file in sequence: `01_M1.md` (M1).


---

### Extended Systems — Gap-Fill Additions (merged 2026-07-14)

---

#### [M1-EXT-26] GPU-Written Draw Count for Mesh Compaction [SUPERSEDED — ALREADY SATISFIED]

> **STATUS: record-only — do NOT build.** The capability already exists in `TriangleRenderer.cpp` (`vkCmdDrawIndexedIndirectCount`). Retained as an audit record, not a system.

##### Systems Touched

Would have touched M1's cluster-culling / indirect-draw compaction pass (the entity-mesh one the M6.5 particle system extends).

##### How It Works

**Not implemented — the gap does not exist.** Per the batch instruction, the actual M1 EXT source was grepped before implementing: `TriangleRenderer.cpp` already consumes a GPU-written count via `vkCmdDrawIndexedIndirectCount(cmd, indirectBuffer, 0, countBuffer, 0, 100, ...)` (line 769), where `countBuffer` is zeroed on the GPU by `vkCmdFillBuffer` (line 554) and written by the `cull.comp` compute dispatch (line 600) using the same atomic-counter pattern the HZB pass (`[M4.5-EXT-08]`) uses for its visible-list — the exact mechanism this item would have added. The compaction pass therefore already issues exactly as many draw commands as survive culling, with no fixed-size dispatch carrying dead entries. Re-adding it would duplicate an existing system. (Side note for review: the legacy `Occlusion Query Double-Buffering` still present in `TriangleRenderer.cpp` at lines 764/798 is the path the companion M1 base-step bullet now marks superseded by `[M4.5-EXT-08]`; that source cleanup is out of scope for this spec batch.)

##### Math
N/A - SUPERSEDED (already satisfied by M1 cluster-cull compaction; retained as a record, not a build target)

##### Reference Implementation
```cpp
// SUPERSEDED: M1 cluster-cull compaction already returns draw count via GPU; no work needed.
```
##### Player-Facing Impact

None directly — and no change needed: the GPU-written draw count already exists in the compaction pass.


### Dev-Tool: EnTT Meta Entity Inspector (EnTT 3.16.0 reflection unlock)

*Not a gameplay milestone — scoped as a dev-tool/debug-overlay feature riding on M1's existing ImGui overlay.*

* **Registration:** EnTT 3.16.0's `entt::meta` reflection is registered for a fixed set of components in a single central file (`src/debug/MetaRegistry.cpp`), not scattered per-component registration calls — keeps the "what's inspectable" list auditable in one place. Initial registration set: `Transform`, `Health`, `DamageEvent`/`ResolvedDamageEvent` (item 5), `StableId` (item 7), `PerkPoints`-related components (item 6/M8.7), `BarrelHeat`, `BulletComponent` (item 4), and `SurfaceFrictionSample`.
* **Inspector UI:** a runtime panel added to M1's existing ImGui workspace overlay. Lists entities queryable either by `SpatialHash` region (drag-select a world-space box, matching M1's existing spatial primitive rather than a second ad hoc query path) or by direct `StableId` lookup (type a known ID). Selecting an entity shows its registered components with live-editable fields (numeric sliders/text fields generated generically from each field's reflected type via `entt::meta`), writing changes back through the registry immediately.
* **Scope decision: dev-only, stripped from release builds** via a compile-time `#ifdef ENGINE_DEV_TOOLS` guard around both the meta-registration call site and the ImGui panel — this is explicitly not being extended into a mod-support surface in this pass. Flagging the boundary per the spec: a future mod-support extension would need to sandbox arbitrary field writes (a mod shouldn't be able to, say, rewrite `StableId` and desync a save), rate-limit/validate write ranges per field, and likely move from "any registered field is editable" to an explicit per-field mod-write allowlist. None of that is being built now.
* **Entity lookup key:** the inspector's region/ID lookups resolve to `StableId` (item 7) first and map to the live `entt::entity` handle for the current session, rather than storing raw handles anywhere in the tool's own state — handles aren't meaningful across a save/load boundary, and this tool is exactly the kind of thing someone will leave a "selected entity" bookmark for across a reload.



---


---



### [Appendix K] [K-EXT-21] Clustered Froxel Light Culling Pass (fleshes §5.8 "Dynamic Light Frustum & Occlusion Culler")


**Gap this closes:** every other rendering system in this doc (Nanite-equivalent meshlet clustering, Lumen-equivalent SVO GI, Virtual Shadow Maps) already matches current-gen practice, but the doc repeatedly assumes "thousands of procedural lights" (muzzle flashes, headlights, fires, floodlights, `[M8.6-EXT-04]`'s electrical traps) get to a "tight GPU list" without ever specifying how — that phrase was §5.8-listed as a one-liner and never implemented. A per-object or per-tile linear light loop does not scale to that count; clustered/froxel light culling is the current standard fix (Forward+/clustered forward, the same family of technique DOOM Eternal and most UE5/Unity HDRP titles use for exactly this problem) and is the natural counterpart to this doc's existing clustered depth-bounds shadow voxelizer (`[M4.5-EXT-13]`).

**Systems Touched:** M4.5 GPU render pipeline — sits between the M1 GPU-driven G-buffer pass and shading; consumed by every forward-lit surface (translucent particles, foliage) and by the deferred lighting pass. Shares its light list with `[K-EXT-08]`'s friction/impact audio synthesizer only in the sense that both read the same light-source entity set `[M0-EXT-01]`'s spatial hash already indexes — no new registry.

**How It Works:** View frustum is subdivided into a 3D grid of froxels (screen-space tiles × exponential depth slices, matching `[M4.5-EXT-13]`'s existing depth-bounds slicing so the two systems share one depth-bucketing scheme instead of each computing its own). A compute pass assigns each active light to every froxel its bounding sphere overlaps, writing a per-froxel `(offset, count)` pair into an indirection buffer plus a flat light-index list. The shading pass then only evaluates lights in its fragment's froxel instead of the full scene light array.

**Reference Implementation**

```glsl
// Pass 1: compute froxel index for a light's bounding sphere and mark it active in every overlapped froxel.
layout(local_size_x = 64) in;
void main() {
    uint lightId = gl_GlobalInvocationID.x;
    if (lightId >= activeLightCount) return;
    vec3 viewPos = (View * vec4(Lights[lightId].worldPos, 1.0)).xyz;
    float radius = Lights[lightId].radius;
    ivec3 minCell = FroxelCellFromView(viewPos - vec3(radius), zSliceExpBase);
    ivec3 maxCell = FroxelCellFromView(viewPos + vec3(radius), zSliceExpBase);
    for (int z = minCell.z; z <= maxCell.z; ++z)
        for (int y = minCell.y; y <= maxCell.y; ++y)
            for (int x = minCell.x; x <= maxCell.x; ++x)
                AppendLightToFroxel(ivec3(x, y, z), lightId); // atomic append into that froxel's index list
}
```

**Player-Facing Impact:** Frame rate holds steady when a horde fight lights up with muzzle flashes, a burning building, and multiple vehicle headlights simultaneously, instead of the lighting pass becoming the frame-time bottleneck at exactly the moment the most is happening on screen.

---

### K.3 — Explicitly not added (checked and rejected as duplicates)

* **Sparse Virtual Texturing (SVT) Asset Page Allocator** (`update.txt` #3) — the doc's existing RVT terrain/object-overlay system plus `[M0-EXT-08]`'s bindless descriptor paging already cover the VRAM-budget problem this solves; a second sparse-paging texture system would fragment the VRAM budget table across two competing allocators instead of one.
* **Two-Tier Deterministic Archetype Weapon Synthesizer** (`update.txt` #29) — the doc's existing "Global Archetype Manifest / Session Mutation Vector" split (§ near M8 itemization) is the same mechanism with more detail already worked out; adding #29 verbatim would reintroduce the exact split-state-variable problem the front-matter audit (item #2) already flags as a class of bug to avoid.
* **Public-Domain Alphanumeric Lexicon** (`update.txt` #33) — exact duplicate of existing §5.9.
* **Long-Range Faction Caravan Route Router** (`update.txt` #36) — already built as the M5.4/M8.5 convoy extension of `[M5.4-EXT-19]`'s road graph (see line-referenced note in the v68 merge audit); re-adding it here would stand up a second road-graph system, which that same audit explicitly warns against.
* **Local SLM Diagnostic Text Generator (vehicle)** (`update.txt` #42) — exact duplicate of the existing `[M13-EXT]` `vehicle_diag` generator.
* **Asynchronous PSO Warm-Up Engine** (`update.txt` #6) — the doc's existing `VK_EXT_shader_object` dynamic-stage-binding system (M4.5, removes PSO *stage* permutations) plus its Asynchronous Shader Compilation via `VK_EXT_graphics_pipeline_library` (§ Tooling & developer iteration loop, batched `vkCreateGraphicsPipelines` calls) already solve the same PSO-compile-stutter problem this proposes; a second warm-up scheme would compete with the shader-object path for the same stutter budget instead of sharing it.
* **Temporal Super-Resolution & Subpixel Jitter Anti-Aliasing Pipeline** (`update.txt` #23) — M4.5's existing Adaptive Upscaling Interface (vendor-detected FSR/DLSS/XeSS) plus its per-pixel motion velocity buffer and Visibility Buffer silhouette AA reconstruction is a superset of what a custom TAA reprojection pass would add, and defers to vendor-tuned upscalers instead of a bespoke implementation the doc would then have to maintain against every future driver update.
* **Structural Fatigue-Life Scrap Salvage Router** (`update.txt` #34) — the doc's existing `StructuralFatigue`/`TickFatigue()` Palmgren-Miner accumulator (canonical since `[M3-EXT-06]`) already drives the M9 vehicle-fatigue consumer path, and `[K-EXT-11]`'s ~90% non-functional-wreck spawn rate already implements this proposal's scarcity outcome; a second salvage-state router would be a fourth parallel wear model, exactly the class of bug the front-matter audit (item #6, `BarrelHeat`) already warns against for shared structs.

---



### [Appendix M] [M5.4-EXT-08] Closed-Ecosystem Continuum-Fluid Horde Density Field


**Systems Touched:** M1 spatial hash, M5.4 AI director, M4 procedural streaming.

**How It Works:** Hordes fully outside the player's active radius are dropped from individual EnTT entities/Jolt bodies down to a coarse 2D density grid, advected with a simple diffusion step (`∂ρ/∂t = D∇²ρ + S_spawn − S_decay`) instead of simulating every zombie. When a density field crosses back into the active radius, the chunk seed (SplitMix64) reconstructs individual zombies deterministically. This is a real and appropriately-scoped technique — background/off-screen abstraction of large populations is standard practice for open-world games with big roaming crowds (broadly the same idea as "unloaded NPC" systems in other open-world titles), and it directly targets your stated 3,000-zombies-5km-away memory/CPU problem.

**Player-Facing Impact:** Distant hordes still drift and grow/shrink over time instead of freezing, without the game paying full simulation cost for zombies you can't see.

---



### [Appendix M] [M1-EXT-12] Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface


**Systems Touched:** M1 MSDF font pipeline, `[M4.5-EXT-07]` texture synthesizer, Runtime Virtual Texture pages.

**How It Works:** Rasterizes novel text (procedurally generated signage, or M13 SLM-generated text if you keep that system) into MSDF glyph pages on the fly via `VK_EXT_host_image_copy`, rather than requiring every possible string pre-baked into a font atlas.

**Conditional note:** this only earns its place if you keep some form of runtime-generated text (dynamic signage, mission text). If you cut or shelve M13's text-generation features, this has no consumer and should wait.

**Player-Facing Impact:** Signage/UI text can be generated at runtime instead of only from a fixed pre-authored string table.

---

### M.2 — Rejected or deferred (reasoning logged, not silently dropped)

* **Volume-preserving muscle deformation shader, FFT-driven audio lip sync, heat-shimmer refraction pass, split-screen HZB reprojection cache** — all technically real techniques, all pure visual polish with zero effect on whether the game is playable or fun, each adding a nontrivial shader/compute pass to maintain. The split-screen item specifically only matters *at all* if local split-screen co-op is in scope — it isn't mentioned as a requirement anywhere else in this doc. **Recommendation:** revisit all four post-M13, once core gameplay loops (M0–M9) are actually running and there's a game to polish.
* **STUN/TURN NAT-traversal gateway, built from scratch** — the underlying need (WAN co-op through arbitrary home routers) is real, but hand-rolling ICE/STUN/TURN negotiation is itself a multi-week networking project independent of everything else in this doc. **Recommendation:** use an existing library that already implements this (e.g. GameNetworkingSockets, which bundles ICE) rather than a bespoke implementation — this is a case where NIH costs you weeks for no gameplay benefit.
* **Local SLM (MiniCPM5-1B) output directly mutating spawn migration targets, price inflation, and "cognitive hallucination" rendering effects on a 30-second loop** — this is the update's biggest single item, and the one most worth pausing on. It's not that the idea is impossible; it's that wiring an LLM's output directly into core simulation state (horde pathing, economy, screen effects) makes bugs non-reproducible — if a zombie horde does something wrong, you won't know if it's your pathing code or the model's output, and you can't easily write a deterministic test for it. It also competes for VRAM with your renderer on a single RTX 2070 Super, and M13 already carries this idea in the base doc — this update just re-describes it, it doesn't add new information. **Recommendation:** keep it scoped exactly as M13 already has it (flavor text/lore/signage, one-way output, no feedback into core sim state) until M0–M12 are solid; the current v72 M13 section already avoids the "SLM controls gameplay" trap this update reintroduces, so no change made here.

### M.3 — v73 follow-up: formalizing the two M.2 recommendations with real reference implementations

Both M.2 items above ended in a "do it this way instead" recommendation rather than a flat rejection: use an existing NAT-traversal library rather than hand-rolling one, and keep the SLM's output surface locked to read-only flavor text. This section turns those two recommendations into concrete, buildable entries.

**ID collision check performed before adding these:** `M12-EXT-06` is unused — clear. `M13-EXT-11` is **not** — that ID already belongs to *Hardware Backend Auto-Detection & Async Compute Queue Isolation*, defined earlier in M13's Extended Systems Library. The SLM entry below is renumbered to `[M13-EXT-14]` (the next free M13-EXT slot after the existing `[M13-EXT-13]`), following the doc's existing renumber-on-collision convention rather than overwriting the existing definition.

---



### [§5.x Addendum] 5.1 Architecture, Threading, Memory, Drivers (M0 / M1 / M1.9 / M4.6)


**[M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator** — canonical copy is under §5.1/M0 in the main body; see there. *(v75: this appendix copy had regressed to the pre-fix `== activeStages` condition the body copy's own comment documents and replaces with `!= 0` — collapsed to this stub rather than left as a second, stale copy that could drift again.)*

**[M0-EXT-11] Asynchronous SPIR-V Shader Cache Garbage Collector**

Systems: shader toolchain, disk cache.
Purpose: sweeps orphaned SPIR-V variant bytecode off disk on an isolated thread after engine updates so the shader cache doesn't grow unbounded across versions.

**[M0-EXT-12] Compute-Worker Thread-Affinity Bitmask Allocator**

Systems: enkiTS scheduler.
Purpose: pins background worker threads to explicit physical cores during heavy workloads to avoid L1/L2 cache-line migration thrashing. (Verify actual win on target hardware before committing — thread pinning is a double-edged sword on hybrid P/E-core CPUs.)

**[M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache** — canonical copy is under §5.1/M1 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub so a future edit to the canonical copy can't leave a stale duplicate behind, as happened with three other Appendix K entries this pass.)*

**[M1-EXT-10] Render Graph Pass Dependency DAG Flattener**

Systems: frame graph compiler.
Purpose: unrolls nested render-pass dependencies into one linear Vulkan execution order, cutting driver-side barrier overhead per frame.

**[M4.6-EXT-04] VRAM Sparse-Resident Memory Page Physical Allocation Tracker** — canonical copy is under §5.1/M4.6 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub for the same reason as `[M1-EXT-09]` above.)*

**[M4.6-EXT-05] DirectStorage Decompression Buffer Ring Allocator Pool**

Systems: GPU decompression streaming.
Purpose: recycles raw byte allocations on the VRAM streaming channel so high-speed chunk load bursts (e.g. driving fast) don't spike memory footprint. Note: requires GDeflate-capable path — confirm this is actually available on your target Vulkan drivers (DirectStorage proper is Windows/DX; on Vulkan you're relying on VK_EXT/vendor GDeflate compute decompression, not the DirectStorage API itself — name this something like "GPU Decompression Ring Allocator" to avoid implying a Windows-only API).

**[M1-EXT-11] Compute-to-Indirect-Draw Execution Barrier** — [MATH DONE]

Purpose: explicit `VkDependencyInfo`/`VkBufferMemoryBarrier2` between the GPU-driven culling compute dispatch and `vkCmdDrawIndexedIndirectCount`, so L2 writes from the compute pass are guaranteed visible to the Command Processor's indirect reader — without it, expect intermittent mesh flicker / missing chunks / hangs on tighter GPU schedulers.

```cpp
void CmdIndirectDrawBarrier(VkCommandBuffer cmd, VkBuffer indirectBuf, VkBuffer countBuf) {
    VkBufferMemoryBarrier2 b[2]{};
    for (auto* buf : { &indirectBuf, &countBuf }) {
        VkBufferMemoryBarrier2& x = b[buf == &indirectBuf ? 0 : 1];
        x.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        x.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        x.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        x.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        x.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        x.buffer = *buf; x.offset = 0; x.size = VK_WHOLE_SIZE;
    }
    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.bufferMemoryBarrierCount = 2; dep.pBufferMemoryBarriers = b;
    vkCmdPipelineBarrier2(cmd, &dep);
}
```





---

## Milestone Sequencing

**Current milestone:** M1

Do not proceed past this file's Exit Criteria until every item passes verification by actually running the build — per the doc's Engineering Audit, this is not a read-the-code-and-assume-it-works check.

**Once Exit Criteria pass:** load the next milestone file in sequence: `02_M2.md` (M2).


---

### Extended Systems — Gap-Fill Additions (merged 2026-07-14)

---

#### [M4.5-EXT-04]

> **STATUS: record-only — do NOT build.** Audit/superseded record; retained for traceability, not a system to implement. Thin-Film Specular Wetness Shading

##### Systems Touched

Inlined within your Vulkan M1/M4.5 G-buffer terrain fragment shader passes.

##### Math

Accumulated rainfall maps create a microscopic water layer thickness (d) over world surfaces. The surface specular reflection intensity (R) updates dynamically using an adaptation of Fresnel's thin-film interference formulas based on the water saturation variables (Wetness):

`R(θ) = R_base·(1.0−Wetness) + Wetness·(R_water + (1.0−R_water)·(1.0−cosθ)^5)`

##### How It Works

To run safely within your 7.0ms rendering target, this feature completely avoids extra rendering passes. Your base terrain fragment shader reads the 2D wetness byte field from your active weather stream data. The layer calculations adjust the roughness and specular parameters inside the lighting calculations inline, executing on the GPU.

##### Reference Implementation

```glsl
// [SUPERSEDED — see [M4.5-EXT-06]] Equivalent to calling that version's ApplyWetShading with porosity = 0.0
vec3 ApplyWetShading(vec3 diffuse, vec3 spec, float wet, float cos_theta) {
    float f = 0.02 + 0.98 * pow(1.0 - cos_theta, 5.0);
    return mix(diffuse, diffuse * 0.3, wet) + mix(spec, vec3(f), wet);
}
```

##### Player-Facing Impact

As storm conditions pass over a sector, the dry, desaturated concrete roads visually shift. Pavements take on a dark, saturated look while reflecting sharp silhouettes of safehouse searchlights and muzzle flashes directly across the asphalt surface, enhancing visual realism without dropping your rendering frame rates.
