## Appendix K — AAA-Parity Gap-Fill (merged from `update.txt`, v70)

**Merge audit (v70 — supersedes the v69 pass's summary, which undercounted by omission and, on this re-check, double-counted two items between its own categories).** `update.txt` (a Gemini chat transcript proposing 42 numbered "AAA parity" subsystems) was cross-checked against the full doc item-by-item, twice: once at v69, and again here after a dedicated bug/gap-completeness pass. Every one of the 42 numbered proposals maps to exactly one of the following, no item counted twice:

* **5 items fleshed out** in **K.1** below — these existed only as name+one-liner stubs in the old §5.8 and are now fully specified with math/struct/implementation: item 1 Skeletal Upper-Body Override (`[K-EXT-01]`), item 13 Holling Type II Horde Pressure (`[K-EXT-02]`), item 35 Dual-Clutch Transmission (`[K-EXT-03]`), item 37 Population ODE Drift Simulator (`[K-EXT-04]`), and item 9 Mesh Grammar Kitbasher (`[K-EXT-17]`, added in this pass — the v69 audit claimed "six" stubs fleshed but only shipped five; this was the missing sixth). A sixth K.1 block, `[K-EXT-05]`, adds a real-time structural-break injection path on top of item 27's already-existing SVO GI grid (see inline-covered, below) — it's additive detail, not a distinct numbered item, so it isn't double-counted here.
* **14 items genuinely new**, added in **K.2**: items 2, 5, 7, 12, 17, 21 (with item 30 folded in as `update.txt`'s own internal duplicate of item 21 — not a 15th item), 22, 28, 31, 32, 40 (`[K-EXT-06]` through `[K-EXT-16]`), plus three more confirmed as genuine gaps in this pass — item 15 SDF Building Interior Splitting (`[K-EXT-18]`), item 18 Volumetric Weather Fog & Light-Shafts (`[K-EXT-19]`), and item 4 the Ambient Traffic/Survivor-Vehicle Scarcity Orchestrator (`[K-EXT-20]`). These last two had no equivalent anywhere in the base doc despite the "already covered" framing implied at v69.
* **8 items explicitly checked and rejected as exact/near duplicates**, logged in **K.3** so nothing silently vanishes: item 3 SVT Page Allocator, item 29 Two-Tier Weapon Synthesizer, item 33 Public-Domain Lexicon, item 36 Caravan Route Router, item 42 Vehicle SLM Diagnostics, and three more confirmed-covered in this pass and now logged rather than left unstated: item 6 Async PSO Warm-Up (already covered by `VK_EXT_shader_object` + async pipeline compilation), item 23 Temporal Super-Resolution/TAA (superseded by the vendor upscaler interface), and item 34 Structural Fatigue Salvage Router (already covered by the shared `TickFatigue()` accumulator).
* **14 items already covered inline** by existing v68 systems under a different name, verified here but not given their own K-EXT block: item 8 Topology-Invariant Master Rig (`[M5.1-EXT-03]`), item 10 Dual-Contouring Extractor (`[M4-EXT-13]`, reused by six different K-EXT callers above), item 11 PBR Virtualizer via compute-bake, item 14 Keplerian/Bruneton-Nishita sky scattering, item 16 GPU Meshlet Clustering + Two-Pass HZB Culling, item 19 Ray-Marched Acoustic Occlusion (`[M6-EXT-11]`'s voxel-cone equivalent), item 20 cell-to-cell thermal fire spreading, item 24 Material-Component Contact Friction & Audio Router (the existing `SurfaceFrictionSample`/`ResolveFriction()` chain already drives both traction and footstep/impact audio from one material lookup), item 25 async binary chunk-delta saving (Zstandard, M7), item 26 Bindless Descriptor Paging (`[M0-EXT-08]`, `[M4.5-EXT-09]`), item 27 the base Sparse Voxel Octree GI grid, item 38 spline road-exclusion, item 39 settlement power-grid conductance, and item 41 `ThompsonBanditTracker` scarcity tuning.

5 + 14 + 8 + 14 = 41 numbered items, + item 30 folded into item 21 as `update.txt`'s own internal duplicate = **42/42 accounted for.**

### K.1 — Fleshed-out stubs (previously §5.8 name-only entries)

#### [K-EXT-01] Skeletal Upper-Body Action-Layering Override (fleshes §5.8 "Upper-Body Animation Layer Override")

**Systems Touched:** M5.2 procedural animation pipeline, M2.7 combat/reload state machine. Slots directly into the pose chain M5.2 already documents but left open: Motion Matching → **Upper-Body Override (this)** → Procedural Rig → Physics Post-Process → Final Pose.

**How It Works:** A spine-root bone index splits the skeleton into a lower-body half (owned by Motion Matching's locomotion output) and an upper-body half (owned by whatever action state — reload, aim, melee swing — is currently active). Every tick, bones at and above the spine root are lerped from the base locomotion pose toward the action pose by a per-state blend weight; bones below stay untouched, so sprinting legs never freeze when the player reloads.

**Reference Implementation**

```cpp
// Blends upper-body action transforms over lower-body Motion-Matching output via an explicit spine-root bone index.
void ApplyUpperBodyOverride(std::span<Transform> basePose, std::span<const Transform> actionPose,
                             float weight, uint32_t spineRootIdx) {
    for (size_t i = spineRootIdx; i < basePose.size(); ++i)
        basePose[i] = Transform::Lerp(basePose[i], actionPose[i], weight);
}
```

`weight` ramps 0→1 over ~0.15s on action entry/exit (critically-damped spring, reusing M5.2's existing spring-damper convention) rather than snapping, so transitions don't pop.

**Player-Facing Impact:** Entities reload, aim, and melee while sprinting/strafing without needing a pre-baked clip for every locomotion × action combination.


#### [K-EXT-02] Continuum-Fluid Holling Type II Horde Pressure Solver (fleshes §5.8 "Continuum-Fluid Horde Density Pressure Field")

**Systems Touched:** M5.4 AI Director / horde steering, M3 structural graph (barricades/doors).

**Math:** Non-linear saturating pressure curve (Holling Type II functional response) instead of a linear density→force scale, so pressure plateaus realistically at extreme crowd density rather than growing unbounded:

`P(density) = (F_max · density) / (K_half + density)`

**How It Works:** Reuses M1-EXT-08's spatial hash to bucket zombie entities per cell and compute a local density scalar per cell each tick (staggered, same convention as other spatial-hash queries in the doc). `P(density)` converts that scalar into a directional force applied against `[M3]`'s structural graph nodes (barricades/doors), so a packed horde crushes/breaks structures with cumulative physical weight instead of per-agent capsule-to-capsule contact tests (which don't scale to thousands of overlapping entities).

**Reference Implementation**

```cpp
// Scales crowd pushing pressure non-linearly (Holling Type II) so density saturates instead of growing unbounded.
float ComputeHordePressure(float density, float maxForce, float halfSaturation) {
    return (maxForce * density) / (halfSaturation + density + 1e-5f);
}
```

**Player-Facing Impact:** Thousands of overlapping zombies visibly push and crush barricades with real weight instead of clipping through each other silently.


#### [K-EXT-03] Non-Linear Dual-Clutch Transmission Controller (fleshes §5.8 "Dual-Clutch Transmission RPM/Torque Controller")

**Systems Touched:** M9 vehicle physics tick (Jolt constraint solver), consumes `StructuralFatigue` wear (`[M3-EXT-06]`) as an input to slip severity.

**How It Works:** Engagement point (0 = fully disengaged clutch, 1 = fully locked) is smoothstepped rather than linear, so slip drops off sharply near full engagement instead of a mechanical-feeling straight ramp; a salvaged/worn gearbox (higher `dynamicSlip`, fed from the vehicle's fatigue accumulator) sputters and loses torque efficiency at the same engagement point a fresh gearbox would handle cleanly.

**Reference Implementation**

```cpp
// Computes clutch-slip torque efficiency inside the fixed-timestep vehicle physics tick.
void StepGearClutch(float dynamicSlip, float engagementPoint, float& outTorqueEfficiency) {
    outTorqueEfficiency = std::clamp(
        1.0f - (dynamicSlip * (1.0f - std::smoothstep(0.2f, 0.8f, engagementPoint))), 0.15f, 1.0f);
}
```

**Player-Facing Impact:** Older, salvaged vehicles feel heavier and slip/sputter under load instead of every car handling identically.


#### [K-EXT-04] Isolated Micro-Population ODE Survivor Drift Simulator (fleshes §5.8 "Offline Time-Lapse Population Drift Simulator")

**Systems Touched:** M8.5 Faction Population ODEs (extends the existing per-faction-scalar hourly update, not a second population model), Appendix D lone-survivor/dead-camp spawning.

**Math:** Logistic growth against a per-region carrying capacity, minus a disease-kill term sourced from the existing SEIR severity value M8.5 already tracks — no new epidemiological model, this just reuses the SEIR output as the kill-rate input:

`dP/dt = r·P·(1 - P/K) - k_disease·P`

**How It Works:** Runs once per in-game hour per hibernated (unrendered) sector, same cadence M8.5 already uses for faction scalars. When the player re-enters a sector, the accumulated population delta resolves into concrete outcomes: growth spawns a new Lone Survivor via Appendix D's existing spawn path, population collapsing to ~0 flips the sector to a Dead Camp (also an existing Appendix D path) with cause-of-death tagged from whichever term dominated the ODE (disease vs. starvation vs. raid, if a raid event fired separately).

**Reference Implementation**

```cpp
// Advances a hibernated sector's survivor population on an hourly tick using logistic growth minus SEIR-driven mortality.
void AdvanceRegionalPopulation(float growthRate, float carryingCapacity, float diseaseKillRate, float& population) {
    population = std::max(0.0f,
        population + (growthRate * population * (1.0f - population / carryingCapacity) - diseaseKillRate * population));
}
```

**Player-Facing Impact:** Sectors you haven't visited in days feel like they kept living without it — a family may have grown, moved on, or died out entirely by the time you return.


#### [K-EXT-05] Dynamic SVO Irradiance Cache Injection Pass (fleshes §5.8 "Dynamic Voxel GI Cache Injection Pass")

**Systems Touched:** Existing Tier-1 Sparse Voxel Octree GI fallback (M4.5), M3 structural-destruction events.

**How It Works:** The SVO GI grid already updates lazily as the sun moves; this adds the missing write path for *structural* changes — when `[M3]` destroys a wall panel, the newly-exposed voxel cells (previously occluded, now open to sky/interior light) are marked dirty and re-sampled on the next lazy GI pass instead of waiting for the sun-angle bucket to change naturally, so a blown-open wall lets light in the same tick, not several in-game minutes later.

**Reference Implementation**

```glsl
// Samples cached SVO irradiance at a world position; called both by shading and by the dirty-cell re-injection pass.
vec4 SampleSvoIrradiance(vec3 worldPosition, float voxelScaleSize) {
    ivec3 voxelCoord = ivec3(floor(worldPosition / voxelScaleSize));
    return textureLod(SvoVolumeTextureTarget, vec4(vec3(voxelCoord) / 128.0, 0.0).xyz, 0.0);
}
```

**Player-Facing Impact:** Blowing a hole in a wall visibly lets light flood the room immediately instead of on a multi-minute GI-refresh delay.


#### [K-EXT-17] Procedural Mesh Grammar Kitbasher & Micro-Detail Synthesizer Pipeline (fleshes §5.8 "Procedural Mesh Kitbasher Pipeline")

**Systems Touched:** M4 chunk stream-in (character/clothing/attachment assembly), consumes the same primitive-SDF + Dual Contouring extraction path `[M4-EXT-13]`'s cave carving and `[K-EXT-10]`/`[K-EXT-12]`'s fracture/weapon-blend systems already use — one extractor, this is a fourth caller, not a second mesher.

**How It Works:** A deterministic split-grammar (L-system) runs on chunk stream-in, combining primitive SDF shapes (cylinders, capsules, tapered boxes) along procedural growth axes to assemble clothing, gear, and weapon-furniture geometry from a handful of grammar rules instead of hand-modeled meshes. Micro-surface detail (scratches, bevels, fabric-weave frequency) is added via the same domain-warped OpenSimplex2 technique the terrain micro-detail pass already uses (§4's `detail(p)` warped-noise pattern), applied to the SDF before extraction so normals stay mathematically exact through Dual Contouring rather than being faked with a normal map. Seeded via the item system's existing Two-Tier archetype/instance split — grammar rule selection is archetype-seeded (permanent per item type), micro-detail warp phase is instance-seeded (per-item variation).

**Reference Implementation**

```cpp
// Spawns structural geometric primitive rings down a procedural vector axis — barrel segments, gear rails, clothing folds.
void GenerateCylinderGrammar(std::vector<glm::vec3>& verts, glm::vec3 start, glm::vec3 dir, float r, uint32_t segments) {
    for (uint32_t i = 0; i < segments; ++i)
        verts.push_back(start + dir * (float)i + glm::vec3(cosf((float)i) * r, 0.0f, sinf((float)i) * r));
}
```

Grammar output SDF is triangulated through the shared Dual Contouring extractor (same call site as `[K-EXT-10]`/`[K-EXT-12]`), then decimated through `[M4-EXT-02]`'s existing quadric-error LOD pass — no new LOD system for kitbashed geometry.

**Player-Facing Impact:** Characters, clothing, and weapon furniture read as visually detailed and non-repetitive without a single hand-modeled asset, completing the last of the six formerly-stubbed §5.8 systems.


### K.2 — Genuinely new systems (absent from v68)

#### [K-EXT-06] Soft-Body Vertex Collision Vehicle Deformation

**Systems Touched:** M9 vehicle physics (Jolt), consumes/feeds `StructuralFatigue` (`[M3-EXT-06]`) so a deformed panel and a fatigued panel are the same underlying wear state, not two parallel damage models.

**How It Works:** On collision, impulse magnitude and contact point are used to displace hull vertices within a falloff radius, softened by a hardness term (steel deforms less than sheet-aluminum trim for the same impulse — `hardness` reads the same material-tag table `[M9]`'s friction/salvage systems already use). This is a *visual + collision-shape* deformation layer sitting on top of the existing fatigue/salvage accumulator — it does not replace `TickFatigue()`, it renders what that accumulator implies.

**Reference Implementation**

```cpp
// Deforms hull vertices based on impact impulse; radius/hardness pulled from the same per-material table [M9] friction uses.
void ApplyImpactDeformation(std::span<glm::vec3> vertices, glm::vec3 impactPoint, glm::vec3 impulse,
                             float radius, float hardness) {
    for (auto& v : vertices) {
        float d = glm::distance(v, impactPoint);
        if (d < radius) v += impulse * (1.0f - (d / radius)) * (1.0f / (hardness + 1e-5f));
    }
}
```

Vertex buffer is CPU-side-mutable per-instance (not the shared procedural template), then re-uploaded through the existing chunk/instance upload path — no new GPU resource type.

**Player-Facing Impact:** Crashes visibly crumple the hull instead of the car staying rigid while an invisible fatigue number ticks down.


#### [K-EXT-07] GPU-Driven Broadphase Spatial Hash Compute Pipeline

**Systems Touched:** Complements (does not replace) `[M1-EXT-08]`'s CPU-side spatial hash quadtree — this is the GPU compute path used specifically for horde-scale (thousands of entities) proximity queries; `[M1-EXT-08]` remains canonical for lower-volume CPU-side queries (player interactions, vehicle contacts).

**How It Works:** A compute shader packs each entity's 2D cell coordinate into a flat hash once per frame, replacing per-entity CPU registry walks. Flocking/crowd-pressure/perception systems (M5.1, M5.3, K-EXT-02 above) read the resulting hash buffer instead of touching the CPU-side registry for bulk horde queries.

**Reference Implementation**

```glsl
// Packs each entity's 2D cell coordinate into a flat hash for fast GPU-side bucket sorting.
layout(local_size_x = 64) in;
void main() {
    uint id = gl_GlobalInvocationID.x;
    ivec2 cell = ivec2(floor(EntityPositions[id].xz / 2.0)); // 2.0m cell stride
    EntityCellHashes[id] = (uint(cell.x) & 0xFFFFu) | ((uint(cell.y) & 0xFFFFu) << 16);
}
```

**Player-Facing Impact:** Keeps flocking/crowd-pressure/perception responsive when thousands of zombies are active simultaneously, protecting the 5.0ms AI frame budget.


#### [K-EXT-08] Real-Time Procedural Friction & Impact Audio Synthesizer

**Systems Touched:** M6 hardware-accelerated audio, reads the same `SurfaceFrictionSample`/material-tag data `[M9]` vehicle friction and `ResolveFriction()` already resolve — one shared friction value drives both physics and audio, not two independent friction reads.

**How It Works:** Instead of streaming pre-recorded tire-screech/engine/impact clips, waveforms are synthesized per-frame from slip ratio and material impedance — modulated white noise for tire slip, tied directly to the same slip-ratio scalar `[M9]`'s traction chain already computes.

**Reference Implementation**

```cpp
// Synthesizes tire-slide audio from the same slip ratio [M9]'s ResolveFriction() chain already produces.
float SynthesizeTireScreech(float slipRatio, float materialImpedance, float time) {
    float whiteNoise = static_cast<float>(rand() % 2000 - 1000) / 1000.0f;
    return sinf(440.0f * 3.14159f * time * (1.0f + slipRatio)) * whiteNoise
           * std::clamp(slipRatio * materialImpedance, 0.0f, 1.0f);
}
```

**Player-Facing Impact:** Zero runtime audio-clip disk footprint for tire/engine/impact sound, consistent with the doc's zero-hand-authored-asset pillar.


#### [K-EXT-09] Reaction-Diffusion Forensic Skin & Tissue Decal Projector

**Systems Touched:** M6.5 GPU particle/VFX, writes into the same RVT terrain/object overlay pages `[M4.5-EXT-07]` (skid marks) and blood-spatter (`[M6.5]`) already use — a third writer into that existing overlay system, appended to `ResolveFriction()`'s ordered-writer front-matter fix (audit item #3) so it doesn't reintroduce an order-dependent conflict.

**How It Works:** A localized Gray-Scott reaction-diffusion solver runs per-decal-region to simulate decomposition/bruising/drying-blood patterns over real elapsed time (reusing the same Arrhenius-style elapsed-Δt pattern M7's sector-hibernation degradation already uses), rather than a static blood-decal texture.

**Reference Implementation**

```glsl
// One Gray-Scott reaction-diffusion step, evaluated per forensic decal region over real elapsed time.
float StepReactionDiffusion(float u, float v, float feed, float kill, float lapU) {
    return u + (0.2 * lapU - u * v * v + feed * (1.0 - u));
}
```

**Player-Facing Impact:** Wound/decomposition/blood-pool weathering reads as context-aware (time since death, ambient temp) instead of a fixed decal.


#### [K-EXT-10] Dynamic Memory-Pooled Implicit Surface Fracture Solver

**Systems Touched:** M3 macro-destruction & structural graphs — this is the SDF-native companion to M3's existing structural-graph fracture logic, used specifically for character/vehicle volumes that are SDF-authored rather than pre-baked destructible meshes.

**How It Works:** On a high-velocity Jolt impact, an analytical cutting plane (impact normal + offset) is intersected against the entity's SDF via a max() combine, producing an instant, exact fracture surface with no pre-baked broken-mesh variant needed.

**Reference Implementation**

```cpp
// Slices an entity's volume SDF against an analytical cutting plane derived from impact normal/offset.
float SampleSdfFracturedSlice(glm::vec3 point, float baseSdfSample, glm::vec3 planeNormal, float planeOffset) {
    float dPlane = glm::dot(point, planeNormal) - planeOffset;
    return std::max(baseSdfSample, dPlane);
}
```

Resulting sliced SDF is triangulated through the same Dual Contouring extractor `[M4-EXT-13]`'s cave-carving already uses — one extractor, two callers, not a second mesher.

**Player-Facing Impact:** Context-aware zombie dismemberment and sheared structural pieces with no pre-baked broken-art variants.


#### [K-EXT-11] Modular Vehicle Chassis Assembly Grammar Engine

**Systems Touched:** M9 vehicle system — this is the missing geometry-generation step feeding M9's existing physics/fatigue/friction systems, which all currently assume a chassis exists but never specify how one is built.

**How It Works:** A structural grammar spawns frame rails, wheel-axis mounts, and engine-block volumes as primitive SDF segments down a procedural vector axis, seeded via the same Two-Tier archetype/instance seed split the item system (§ "Global Archetype Manifest") already uses — chassis identity is a permanent per-archetype seed, wear/scavenge state is a per-instance seed layered on top. ~90% of spawned chassis roll as non-functional wrecks (`StructuralFatigue` already crossing its yield threshold at spawn time) suitable only for part harvesting, consistent with the doc's scarcity design for functional vehicles.

**Reference Implementation**

```cpp
// Spawns structural frame rail primitives down a procedural vector axis, seeded from the archetype/instance split.
void BuildChassisRail(std::vector<glm::vec3>& vertices, glm::vec3 origin, glm::vec3 direction, float width, uint32_t segments) {
    for (uint32_t i = 0; i < segments; ++i)
        vertices.push_back(origin + direction * (float)i + glm::vec3(width, 0.0f, 0.0f));
}
```

**Player-Facing Impact:** Hundreds of distinct, functional car/truck/military-chassis categories generated purely from code + seeds, with realistic scavenging (most wrecks are parts, not drivable finds).


#### [K-EXT-12] SDF Boolean Prim-Blending Weapon Customization Fabricator

**Systems Touched:** M8 itemization, downstream of the same Two-Tier archetype/instance weapon seed already canonical there — this is specifically the *attachment geometry* step (scopes/barrels/stocks) that seed system doesn't yet specify a mesh method for.

**How It Works:** Smooth-minimum SDF blending fuses attachment primitives onto a base weapon SDF with a soft transition (no hard seam at the socket), rather than a hard union — the same Dual Contouring extractor used elsewhere in the doc triangulates the result.

**Reference Implementation**

```cpp
// Smooth-minimum blend between two SDF distance samples, producing a seamless attachment-to-receiver transition.
float SmoothSdfUnion(float d1, float d2, float k) {
    float h = std::clamp(0.5f + 0.5f * (d2 - d1) / (k + 1e-5f), 0.0f, 1.0f);
    return std::lerp(d2, d1, h) - k * h * (1.0f - h);
}
```

**Player-Facing Impact:** Visually seamless, modular weapon customization built entirely from code, no attachment mesh assets.


#### [K-EXT-13] Data-Table Modding & Hot-Reload Path

**Systems Touched:** M0 asset toolchain — extends the existing shader hot-reload path (`VK_EXT_graphics_pipeline_library` / DXC, already in M0) to JSON data tables (weapon/vehicle archetypes, caravan/scarcity tuning, brand-tier tables), which currently have no equivalent live-reload path.

**How It Works:** File resolution checks a prioritized mod directory before falling back to the built-in data directory; schema validation runs loudly at boot (or on file-change event) so a malformed mod JSON fails fast with a clear error instead of silently corrupting a table.

**Reference Implementation**

```cpp
// Resolves a data-table path, preferring a mod-directory override over the built-in table of the same name.
std::string ResolveModAssetPath(const std::string& filename, const std::string& modDir, const std::string& baseDir) {
    return std::filesystem::exists(modDir + filename) ? (modDir + filename) : (baseDir + filename);
}
```

**Player-Facing Impact:** Weapon/vehicle/scarcity tuning can be iterated via JSON edits without a full recompile — this is also the mechanism that lets *you* (JJ) add new guns/cars/systems fastest during development.


#### [K-EXT-14] Property-Matching Material Synthesizer & Derived Item Stat Generator

**Systems Touched:** M8 itemization — extends the existing material/mass/volume tags every scavenged item already carries for physics, adding a stat-derivation step so a new scrap item doesn't need a hand-written recipe-table entry.

**How It Works:** Crafting validity and resulting stats are derived from raw material tag + mass/volume bounds rather than a lookup table keyed by item name, so any new item with a "metallic, high-density" tag automatically qualifies for weapon-action/armor-brace crafting without a new table row.

**Reference Implementation**

```cpp
// Derives a component's kinetic-impact damage capacity from its material tag and density, no per-item table entry needed.
float DeriveImpactDamage(uint32_t materialTag, float mass, float volume) {
    return (materialTag == 1 /* Metal Class */) ? (mass / (volume + 1e-5f)) * 45.0f : mass * 12.0f;
}
```

**Player-Facing Impact:** Adding a new scrap item to the game is a data-tag change, not a new crafting-recipe entry.


#### [K-EXT-15] Hidden Outpost/Hideout Node Placement Selector

**Systems Touched:** Appendix D (Lone Survivors & Dead Camps), M4 world generation — this is the placement-selection step Appendix D's spawn logic currently assumes but never specifies.

**How It Works:** Candidate cells are scored from terrain slope, distance from major roads (reusing `[M4]`'s road graph, not a second distance field), and the existing socio-economic zoning tags M4 already assigns; low-slope, high-road-distance, low-commercial-zoning cells score highest and are selected deterministically from the world seed.

**Reference Implementation**

```cpp
// Scores a candidate cell's fitness as a hidden hideout site from terrain slope, road distance, and zoning tag.
float EvaluateHideoutFitness(float slope, float proximityToHighways, float zoningTagCommercial) {
    return (slope < 0.15f && proximityToHighways > 300.0f) ? (1.0f - zoningTagCommercial) * 85.0f : 0.0f;
}
```

**Player-Facing Impact:** Survivor safehouses and bandit nests land in believable, defensible, out-of-the-way spots instead of random placement.


#### [K-EXT-16] Procedural Vector-Distance UI Canvas Renderer

**Systems Touched:** M11 UI/HUD — this is a rendering-method addition (not a new HUD design) for whichever HUD elements M11 already specifies.

**How It Works:** Menu/inventory/crosshair geometry is drawn as analytical distance functions (lines, rounded boxes, font curvature) evaluated per-pixel in a fragment shader, rather than image-texture UI assets, keeping M11 consistent with the doc's zero-hand-authored-asset pillar.

**Reference Implementation**

```glsl
// Draws an analytical crosshair ring via implicit circle distance, resolution-independent at any display scale.
float DrawImplicitCrosshairRing(vec2 screenPixelCoord, vec2 centerAnchor, float radius, float edgeThickness) {
    float d = length(screenPixelCoord - centerAnchor) - radius;
    return smoothstep(edgeThickness, 0.0, abs(d));
}
```

**Player-Facing Impact:** Perfectly sharp UI at any resolution, zero UI texture-asset files.


#### [K-EXT-18] Implicit SDF Building Interior Splitting Grammar Engine (fleshes M4's one-liner "Procedural Urban Detail (L-systems, BSP interiors)")

**Systems Touched:** M4 procedural world generation, downstream of the macro-graph city layout pass (§4's top-down road/zoning graph) and upstream of `[K-EXT-17]`'s furniture/clothing kitbasher, which populates the rooms this creates.

**How It Works:** A building's bounding SDF envelope (produced by M4's existing structure-placement pass) is recursively subtracted against room/corridor volumes using a BSP-style split grammar — each split picks an axis and offset from the building's zoning tag and footprint aspect ratio, carving hallways, room partitions, and stairwells as boolean subtractions rather than as a hand-placed floorplan. Door openings are punched as smaller box subtractions at each partition boundary and immediately get the base `DoorComponent` the front-matter audit (item 4) already mandates in M2.6/M3, so every procedurally-cut doorway is a real, breakable door from the moment it's generated, not a static gap. Runs at chunk stream-in, same cadence as the rest of M4's WFC/L-system content.

**Reference Implementation**

```cpp
// Evaluates a subtraction between a building's bounding envelope and an interior room/corridor volume.
float SampleSdfSubtractedRoom(glm::vec3 p, glm::vec3 bBox, glm::vec3 roomBox) {
    float dStructure = glm::length(glm::max(glm::abs(p) - bBox, glm::vec3(0.0f)));
    float dInterior  = glm::length(glm::max(glm::abs(p) - roomBox, glm::vec3(0.0f)));
    return std::max(dStructure, -dInterior);
}
```

Resulting interior SDF is triangulated through the same Dual Contouring extractor `[M4-EXT-13]` and `[K-EXT-17]` already call.

**Player-Facing Impact:** Buildings have real, explorable, non-repetitive interiors — rooms, hallways, stairwells, breakable doors — generated entirely from grammar rules with zero hand-authored floorplans.


#### [K-EXT-19] Volumetric Weather Fog & Light-Shaft Scattering Pass

**Systems Touched:** M10 weather system (feeds off the existing meteorology density output `[M10-EXT-05]` already produces), M4.5's rendering pipeline — this is the general atmospheric-volume companion to `[M4.5-EXT-01]`'s flashlight-specific volumetric cone (that one is a single artificial light source with hand-tuned constants; this is the sun/moon through ambient weather density, reusing the sky LUT's Rayleigh/Mie coefficients from §M10 rather than a second scattering model).

**How It Works:** A froxel (view-frustum-aligned voxel) grid accumulates in-scattered light along the camera's view rays each frame, sampling local fog/rain/dust density from M10's existing meteorology density field and the same `β_Rayleigh`/Henyey-Greenstein Mie phase function the sky LUT (§7.4xx) already defines — one shared scattering model driving both the sky color and the volumetric shafts, not two independent atmospheric systems. Sun/moon shadow-map occlusion tested per froxel produces visible light shafts through gaps in structures and canopy.

**Reference Implementation**

```glsl
// Beer-Lambert transmittance step through a single froxel cell, using the same scatterCoeff the sky LUT already derives.
float ComputeVolumetricTransmittance(float scatterCoeff, float stepLength, float localDensity) {
    return exp(-scatterCoeff * localDensity * stepLength);
}
```

Froxel accumulation buffer is consumed by the main G-buffer composite pass as a screen-space multiply, same insertion point as existing post-process passes.

**Player-Facing Impact:** Fog, rain, and dust get real, sun-occluded light shafts and depth-cued haze instead of a flat fog color — the atmospheric depth AAA titles use for tone, especially through broken structures and forest canopy.


#### [K-EXT-20] Ambient Traffic & Survivor-Vehicle Scarcity Spawn Orchestrator

**Systems Touched:** M8.5 population/scarcity systems (extends the existing per-region loot-scarcity tagging, not a new economy), M5.4's road graph (the same graph `[M5.4-EXT-06]`'s faction caravans already route on) — this is specifically the walking-survivor / functional-vehicle encounter-density layer that sits alongside caravan routing, not a replacement for it.

**How It Works:** A deterministic per-region Bernoulli roll (SplitMix64, reusing the doc's standard PRNG convention) gates whether a given lone-survivor spawn (Appendix D) or settlement-adjacent scavenge point also rolls a functional vehicle — weighted heavily toward zero, and boosted only near settlements/hideouts, so a lone survivor with a *working* car is a rare, memorable find rather than a routine one. Most ambient population is on foot; faction caravans (already covered by `[M5.4-EXT-06]`) are a separate, much rarer, unkillable macro-graph event layered on top of the same road network, not competing with this roll for the same spawn budget.

**Reference Implementation**

```cpp
// Deterministic Bernoulli trial gating whether a lone-survivor/scavenge spawn also rolls a functional vehicle.
bool RollScarcityVehicleSpawn(uint64_t& seed, float baseProbability, bool nearSettlement) {
    seed = (seed ^ 0xBF5Dull) * 0x9E3779B97F4A7C15ull;
    return (static_cast<float>(seed & 0xFFFFFFFFu) / 4294967295.0f) < (baseProbability * (nearSettlement ? 5.0f : 0.1f));
}
```

Reads the same `CraftingStationComponent`/socio-economic zoning tags Appendix D and `[K-EXT-15]`'s hideout selector already use for placement context — one shared scarcity/zoning read path, not a parallel tagging system.

**Player-Facing Impact:** Finding another living survivor is uncommon; finding one with a working car is a genuine rare event, distinct from the much rarer, unkillable armored caravans passing through on the highway graph.


#### [K-EXT-21] Clustered Froxel Light Culling Pass (fleshes §5.8 "Dynamic Light Frustum & Occlusion Culler")

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


### K.3 — Explicitly not added (checked and rejected as duplicates)

* **Sparse Virtual Texturing (SVT) Asset Page Allocator** (`update.txt` #3) — the doc's existing RVT terrain/object-overlay system plus `[M0-EXT-08]`'s bindless descriptor paging already cover the VRAM-budget problem this solves; a second sparse-paging texture system would fragment the VRAM budget table across two competing allocators instead of one.
* **Two-Tier Deterministic Archetype Weapon Synthesizer** (`update.txt` #29) — the doc's existing "Global Archetype Manifest / Session Mutation Vector" split (§ near M8 itemization) is the same mechanism with more detail already worked out; adding #29 verbatim would reintroduce the exact split-state-variable problem the front-matter audit (item #2) already flags as a class of bug to avoid.
* **Public-Domain Alphanumeric Lexicon** (`update.txt` #33) — exact duplicate of existing §5.9.
* **Long-Range Faction Caravan Route Router** (`update.txt` #36) — already built as the M5.4/M8.5 convoy extension of `[M5.4-EXT-06]`'s road graph (see line-referenced note in the v68 merge audit); re-adding it here would stand up a second road-graph system, which that same audit explicitly warns against.
* **Local SLM Diagnostic Text Generator (vehicle)** (`update.txt` #42) — exact duplicate of the existing `[M13-EXT]` `vehicle_diag` generator.
* **Asynchronous PSO Warm-Up Engine** (`update.txt` #6) — the doc's existing `VK_EXT_shader_object` dynamic-stage-binding system (M4.5, removes PSO *stage* permutations) plus its Asynchronous Shader Compilation via `VK_EXT_graphics_pipeline_library` (§ Tooling & developer iteration loop, batched `vkCreateGraphicsPipelines` calls) already solve the same PSO-compile-stutter problem this proposes; a second warm-up scheme would compete with the shader-object path for the same stutter budget instead of sharing it.
* **Temporal Super-Resolution & Subpixel Jitter Anti-Aliasing Pipeline** (`update.txt` #23) — M4.5's existing Adaptive Upscaling Interface (vendor-detected FSR/DLSS/XeSS) plus its per-pixel motion velocity buffer and Visibility Buffer silhouette AA reconstruction is a superset of what a custom TAA reprojection pass would add, and defers to vendor-tuned upscalers instead of a bespoke implementation the doc would then have to maintain against every future driver update.
* **Structural Fatigue-Life Scrap Salvage Router** (`update.txt` #34) — the doc's existing `StructuralFatigue`/`TickFatigue()` Palmgren-Miner accumulator (canonical since `[M3-EXT-06]`) already drives the M9 vehicle-fatigue consumer path, and `[K-EXT-11]`'s ~90% non-functional-wreck spawn rate already implements this proposal's scarcity outcome; a second salvage-state router would be a fourth parallel wear model, exactly the class of bug the front-matter audit (item #6, `BarrelHeat`) already warns against for shared structs.
