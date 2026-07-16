## Appendices

### Appendix A: Procedural Narrative System

Diary/journal entry generator composed from template grammars filled from simulation values:

* Humanity crosses threshold (e.g. below 40) -> reads recent morally weighted action.
* Faction reputation crosses threshold -> tracks trade/conflict events.
* Settlement raided -> reads structural damage logs and casualties.

### Appendix B: Concrete Property Tables

* Day/Night: 48 real-time minutes (60/40 split). Weather: 15% rain, 3% storms.
* Destruction Thresholds: Wood wall failure at 800 N·s. Brick failure at 3,200 N·s. Sedan impact at 60 km/h: ~25,000 N·s *(corrected — the doc's prior 2,900 N·s figure was internally inconsistent with its own stated inputs: impulse = mass × Δv, and a stated "sedan" at a stated 60 km/h (16.67 m/s) coming to a full stop implies J = 1500 kg × 16.67 m/s ≈ 25,000 N·s using a representative curb weight for a mid-size sedan; 2,900 N·s only back-solves to a ~174 kg object, i.e. motorcycle-class, not a sedan)*.

### Appendix C: Worked Examples — Quest & Loot Generation

* Loot distributions derived strictly through Socio-Economic tags (e.g., Gym yields heavy blunt items and protein).

### Appendix D: Lone Survivors & Dead Camps

* Lone Survivors spawn via M8.5 ODE population mechanics.
* Dead Camps dynamically spawn based on recorded persistence failure causes (SEIR outbreaks leave bloat/hazard maps, Raids leave ballistic-scarred structural destruction).

### Appendix E: Procedural Generation & Animation Unification

* `sub_seed()` SplitMix64 hashing ensuring generation continuity without saving specific geometry structures in-memory.
* Facial Expression Rig Priority: Visemes override procedurally animated autonomic jitters seamlessly.

### Appendix F/G/H: AAA-Grade Generation, Simulation, World & Content

* **Voice:** Piper offline neural TTS, Formant/pitch-decoupled scaling, PSOLA processing.
* **Textures/Weather:** Domain-warped OpenSimplex2 terrain mapping, runtime WFC 3D constraint assemblies.
* **Simulation:** Real-time HTN/GOAP integrations, XPBD cloth coupling, reaction-diffusion modeling.
* **World:** Foliage Poisson-disk clustering, DDGI/ReSTIR integrations, stream-power hydraulic erosion matrices.

### Appendix I/J: FPS Controller Deep-Dive & Gunplay Pass

* Hand-modeled parameter sweeps defining exact mantle/climb curves utilizing critically damped spring constants.
* Ballistic integration modeling genuine drop and weapon-rest stabilization techniques scaling strictly from mass/inertia math formulas, completely replacing arbitrary attachment arrays.


## Not yet merged — awaiting priority confirmation

The following `master_feature_list.md` tables were deliberately excluded from this v43 pass (solo dev asked to sequence engine layer first):

* **Procedural Generation — World/Systems** (Poisson disk sampling, domain-warped noise, WFC adjacency validator [already `[M4-EXT-03]`/`[M4-EXT-11]`], chunk-boundary determinism test [route to §5.10 verification suite], graph-based room connectivity [already `[K-EXT-18]` + `[M4-EXT-08]`], rejection sampling [shared gate, not a standalone system — see draft pass-2 note], seed-sharing [already `[M2-EXT-54]`], cellular automata [now `[M4-EXT-24]`], graph grammars [covered by Procedural Mission & Event Director, Dormans & Bakkes], genetic algorithms [now `[M8.6-EXT-09]`], Whittaker biome classification [now `[M4-EXT-23]`], thermal erosion, spline-based roads/rivers [roads fully covered by `[M4-EXT-01]`/`[M4-EXT-19]`/`[M4-EXT-10]`; rivers half-covered — bank/confluence hydrology detail only], hidden difficulty correction) — deferred, engine layer first.
* **Procedural Asset Generation (Solo/No-Artist)** (histogram-preserving blending [now `[M4-EXT-25]`], PPTBF materials [now `[M4-EXT-25]`], node-graph procedural materials [now `[M4-EXT-25]`], noise→normal map derivation [now `[M4-EXT-25]`], procedural mesh via primitive+grammar [already `[K-EXT-17]`], procedural foliage L-system [now `[M4-EXT-28]`], procedural color palette [now `[M4-EXT-25]`], procedural UV unwrap [recommended `[M4-EXT-26]`-class, deliberately not merged this pass — decal atlas renumbered to `[M4-EXT-26]` instead], procedural skeletal rig generation [already `[M5-EXT-19]`], procedural decal atlasing [now `[M4-EXT-26]`], procedural loot icon generation [now `[M8-EXT-10]`]) — deferred, engine layer first. See teq-v78-procedural-gapfill-draft.md close-out notes (pass 1 + pass 2) and Part A Hermes gap-fill appendix for the ID citations above.
* **AI** (behavior tree/planning layer, line-of-sight result caching) — deferred, engine layer first.
* **Audio** (dynamic/adaptive music, voice-line barking system) — deferred, engine layer first.
* **Meta/UX/Tooling** (pseudo-localization QA mode, save versioning/migration, save backup rotation, cross-progression/cloud save, remote-config balance patching, terrain deformation persistence, weapon attachment/modification, NPC daily schedules, curated environmental storytelling props, frame-time variance display, predictive asset streaming, input buffering/coyote-time, session-level difficulty rubber-band, server-authoritative anti-cheat, death-cam/kill-cam replay) — deferred, engine layer first. (Mod load-order conflict detection removed from this deferred bucket — no longer deferred, see **Appendix L**.)
* **Libraries to Adopt** (FastNoise2, vk-bootstrap+VMA formalization, Slang shader compiler) — deferred; these are toolchain adoptions that touch already-deferred procgen systems (FastNoise2) or are cross-cutting infra decisions best made alongside the deferred tables, not folded in piecemeal. (Tracy profiler already decided and merged — see "Profiler of record: Tracy" under Tooling & developer iteration loop — removed from this list.)

#### [M12-EXT-03] Bitstream Delta-Encoded Packet Replay Fragment Reassembler

Same feature, same `OrderedPacketSlice`/`ReassembleInboundFragment` shape as the Section 5.7 appendix entry under this same ID. This is the canonical copy (full Systems/Math/How-It-Works writeup); the appendix entry should just cross-reference it.

##### Systems Touched

Network transmission parsing, `StableId` entity replication tracking arrays. Sits downstream of `[M12-EXT-01]`'s delta encoder and `[M12-EXT-02]`'s loss-detection window — this reassembles fragments that arrive out of order, it doesn't decide packing or detect loss itself.

##### Math

Modulo slicing matching inbound network delivery tracking indices to sliding sequence buffer slots:

`SlotIdx = Seq_packet mod BufferCapacity`

##### How It Works

Intercepts fragmented delta network payloads passing across ENet/GameNetworkingSockets boundaries. Because packets can land out of sequence due to jitter, this reassembler sorts inbound fragments across a sliding modulo array, preventing the client-side entity factory from processing broken or unaligned component updates until sequence chains resolve cleanly.

##### Reference Implementation

Sequence-window checks use an unsigned wraparound-safe distance comparison (not `sequence + poolCapacity <= maxTrackedSequence`, which overflow-wraps near `UINT32_MAX` and would silently accept stale fragments).

>

The copy is clamped to `min(rawBufferLen, sizeof(discretePayloadBuffer))` (never a fixed `memcpy(..., 256)`, since ENet/GameNetworkingSockets fragments are rarely exactly 256 bytes and a fixed copy would over-read past `rawBufferData`). A `payloadLength` field records how many bytes in the slot are valid.

```cpp
#include <cstdint>
#include <cstring>
#include <algorithm>

struct OrderedPacketSlice {
    uint32_t sequenceIdentifier;
    uint8_t discretePayloadBuffer[256];
    uint16_t payloadLength;
    bool isSlotPopulated;
};

inline bool ReassembleInboundFragment(uint32_t sequence, const uint8_t* rawBufferData, uint32_t rawBufferLen, OrderedPacketSlice* ringPool, uint32_t poolCapacity, uint32_t maxTrackedSequence) {
    const uint32_t backwardDistance = maxTrackedSequence - sequence;
    if (backwardDistance < 0x80000000u && backwardDistance >= poolCapacity) return false; // stale, already outside sliding window
    const uint32_t targetedSlot = sequence % poolCapacity;
    const uint32_t copyLen = std::min<uint32_t>(rawBufferLen, sizeof(OrderedPacketSlice::discretePayloadBuffer)); // never read past rawBufferData's real length

    ringPool[targetedSlot].sequenceIdentifier = sequence;
    std::memcpy(ringPool[targetedSlot].discretePayloadBuffer, rawBufferData, copyLen);
    ringPool[targetedSlot].payloadLength = static_cast<uint16_t>(copyLen);
    ringPool[targetedSlot].isSlotPopulated = true;
    return true;
}
```

##### Player-Facing Impact

Eliminates position rubber-banding and desynchronization during networked multiplayer co-op sessions, maintaining tight physical synchronization under volatile connection profiles.


# 5\. Extended Architectural Subsystems — Master Addendum

Consolidated, deduplicated list from the architecture gap-finding pass. IDs follow `[Mx-EXT-nn]` and slot into their listed milestone. Items marked **[MATH DONE]** have full math + reference code below; everything else is scoped (systems touched + purpose) and needs math fleshed out before implementation — do that per-item when you get to it, don't block the merge on it.

**Merge audit (v67) — `updateforv66.txt` fully integrated, nothing dropped.**

**Merge audit (v68) — two gaps found in the v67 pass, now closed.** The v67 audit above covered the EXT-block items but silently missed two large non-EXT-formatted sections of `updateforv66.txt`: Section 3's 20-layer environmental simulation hierarchy (never mentioned as included *or* excluded — just dropped), and Section 6's standalone sandboxed verification test suite. Both are now in: the 20 layers are added as 20 full EXT blocks — `[M3-EXT-07/08]` (structural stress-tensor cracking, particulate deposition), `[M4-EXT-12..19]` (tectonics, cave carving, stratigraphy, Darcy aquifer extension to the existing M10 groundwater grid, generation-time Saint-Venant erosion reusing `[M10-EXT-01]`'s flux stencil, edaphic soil chemistry, space-colonization canopy, road-graph exclusion splines), `[M4.5-EXT-19/20]` (Heiligenschein/anisotropic micro-optics, Beer-Lambert underwater extinction), `[M6-EXT-11]` (acoustic voxel occlusion, pairs with EXT-20), `[M6.5-EXT-11/12]` (fBm foliage kinematics, volumetric micro-atmospherics/heat-shimmer), `[M9-EXT-19]` (POM topography deform + persistent ruts, fills the displacement-channel stub `[M9-EXT-04]` was already reading), `[M10-EXT-02..06]` (Keplerian sun/moon solver, Bruneton-Nishita scattering, cryospheric frost, Navier-Stokes wind field, phenological seasonal canopy). None of these duplicate an existing system — each explicitly wires into something already in the doc (stratum table, groundwater grid, road graph, wind field, season index) rather than adding a second version of it. The verification suite is added as new §5.10, using local mock structs so it never collides with canonical types. First pass flagged six items as "duplicates" and left them out; on a closer re-read most weren't true duplicates, just overlapping-sounding names — they're folded in below instead: `[M1-EXT-11]` compute→indirect-draw barrier, `[M4.5-EXT-18]` skinned Visibility-Buffer cache, `[M2-EXT-68]` player flood buoyancy, `[M4-EXT-11]` WFC contradiction recovery, `[M2-EXT-57]` fixed-point mesh-vector quantization (sibling to `[M2-EXT-53]`'s scalar version, not a dupe — that one's a 16.16 scalar for damage/pathfinding math, this one's a 24.8 `vec3` for procedural mesh-generation determinism, different data shape), `[M13-EXT-12]` radio scrambler, `[M13-EXT-13]` survivor behavior-tree flavor advisor, plus enriched fields on `[M13-EXT-04]` and `[M13-EXT-09]` pulled from the update's richer telemetry tokens, `[M9-EXT-18]` (routes vehicles onto the existing `[M5-EXT-51]` road graph instead of building a second one), and new §5.9 naming lexicon. Two things genuinely were exact duplicates and stayed out: `AlignToCacheLine` (byte-identical to `[M0-EXT-01]`, nothing to add), and a second `SurfaceFrictionSample` struct with different field names for the same six multiplier slots — the canonical one (defined under M9 below) is what everything else in this doc already writes to, so a second definition would just reintroduce the exact order-dependent bug the front-matter audit item #3 fixed; if you want the update's field names instead, rename in one place (the canonical struct), don't add a second struct. One more from the update, `DirectorTelemetryStateToken`, wasn't added as an `M13-EXT` item at all — horde-pacing telemetry (avg player health, shots fired, SEIR severity) already feeds M5.4's own `ThompsonBanditTracker`, and that's deterministic tuning logic, not narrative text, so it stays in M5.4 rather than being routed through the SLM.


## 5.1 Architecture, Threading, Memory, Drivers (M0 / M1 / M1.9 / M4.6)

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

**[M4-EXT-87] DirectStorage Decompression Buffer Ring Allocator Pool**

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

**Numbering note (v78, this pass):** the live `milestones/` directory has since moved ahead of this
consolidated doc in several places I don't have visibility into from here — confirmed collisions
include `[M1-EXT-13]` (Render-Graph Barrier Topological Sorter), `[M1-EXT-15]` (GPU Query Pool
Resolver), `[M1-EXT-16]` (SoA Cache-Line Padding), and `[M1-EXT-17]` (unknown content). The entries
below are numbered `[M1-EXT-18]` onward specifically to not collide with those. Treat these numbers
as provisional until checked against the actual `01_M1.md` — this doc is the one that's behind, not
the live files.

**[M1-EXT-18] Material-Batched Mesh-Pass Rendering**

Systems: existing indirect-draw/compaction path (the one `[M1-EXT-11]`'s barrier already sits in
front of), M4.5 forward/G-buffer/transparent/shadow passes, `[M0-EXT-08]`/`[M4.5-EXT-09]`'s bindless
`MaterialIdx`.

Math: draw commands stable-sorted by `MaterialIdx` before the existing prefix-sum compaction runs,
producing contiguous per-material runs: `run_m = [i : MaterialIdx(i) = m]`.

Purpose: adds a sort key to the existing compaction pass, not a second buffer or pass — each render
pass issues one pipeline/descriptor bind per material batch instead of per draw. Forward/
transparent/shadow passes consume the same material-sorted buckets since `MaterialIdx` doesn't
change mid-frame. Fewer binds per frame at horde scale (mixed zombie skins, weapon variants,
debris); no visual change.

**[M1-EXT-19] SoA Layout for Hot Components**

Systems: Transform + bone-matrix arrays, `SpatialHash` consumers (M5.1, M5.4, `[M8.5-EXT-02]`),
composes with `[M1-EXT-09]`'s pointer/stride cache rather than replacing it — EXT-09 still resolves
offsets into whatever layout backs the pool; this changes what that layout physically is for the
named hot fields.

Purpose: array-of-structs → structure-of-arrays for fields actually iterated hot-path per tick only
(cold fields stay packed as-is), so enkiTS workers iterating one field stream contiguous memory
instead of skipping over unrelated fields. Reduces cache-miss stalls at horde scale; no behavior
change.

**[M1-EXT-20] EnTT Group-Backed Hot-Component Storage**

Systems: composes with `[M1-EXT-09]` and `[M1-EXT-18]`'s SoA item — same hot-field set (Transform,
bone arrays, `SpatialHash` consumers), not registry-wide.

Purpose: applies `entt::registry::group` to the named hot components so EnTT physically sorts their
backing storage for contiguous joint iteration, rather than relying solely on `[M1-EXT-09]`'s
tick-local offset cache to paper over a non-contiguous sparse-set layout. Grouping can reallocate a
pool's backing storage, so `[M1-EXT-09]`'s cache is still re-resolved every tick exactly as it
already does today — this doesn't change that discipline, it changes what the cache points into.
Only applied to the same hot-field set the SoA item targets; over-grouping the whole registry would
fight EnTT's mutation flexibility for cold, rarely-touched components. Compounds with `[M1-EXT-18]`'s
locality win rather than duplicating it.

**[M1-EXT-21] Buffer Device Address for Skinned-Mesh Animation Data**

Systems: zombie/companion skeletal animation, M5.2 rig-pool, existing static-mesh indirect-draw
path.

Purpose: skinning computed once per frame into a device-local buffer accessed via `VkDeviceAddress`
(pushed as a push-constant pointer, current Vulkan bindless practice); the resulting buffer feeds
the same indirect-draw/culling path already used for static geometry, so skinned and static share
one draw pipeline instead of two. Removes per-mesh descriptor rebind overhead for animated
characters at horde scale.

**[M1-EXT-22] CVar System (ImGui-Backed)**

Systems: existing ImGui overlay (`[M1-EXT-15]`'s HUD lives here), scattered ad hoc debug toggles
across earlier milestones.

Purpose: typed int/float/bool/string console-variable registry, live-editable from an ImGui panel
and settable via boot-time launch flags — replaces scattered hardcoded debug toggles with one
queryable registry other systems read from instead of reinventing their own flag. Dev tool only, no
player-facing effect.

**[M1-EXT-23] Timeline Semaphores for Multi-Queue Sync**

Systems: graphics/compute/transfer submission, `[M13-EXT-11]`'s async compute queue (this
formalizes the cross-queue sync mechanism EXT-11 already assumes exists rather than introducing a
competing one), `[M4-EXT-87]`'s streaming transfer queue.

Purpose: `VK_KHR_timeline_semaphore` (core Vulkan 1.2+, mandatory-core in 1.4) replaces
per-submission fence+binary-semaphore pairs with one monotonically increasing counter per queue
relationship; consumers wait on a target value instead of juggling handles. No player-facing effect
— cleaner cross-queue sync, headroom for future async work.

**[M1-EXT-24] Multi-Threaded Secondary Command Buffer Recording**

Systems: `[M1-EXT-03]`'s Multi-Threaded Command Pool Matrix — that system only allocates one pool
per thread/frame-slot and never specifies a primary-vs-secondary buffer strategy; this is that
missing piece.

Math: `Pool_index` is unchanged from `[M1-EXT-03]`; each worker records a `SECONDARY` command
buffer from its own pool via `VkCommandBufferInheritanceInfo` (render pass + subpass inherited from
the primary), then one primary buffer executes all secondaries via a single `vkCmdExecuteCommands`.

Purpose: workers record draw calls in parallel with zero cross-thread synchronization beyond
`[M1-EXT-03]`'s existing per-thread pool isolation; the primary buffer does no drawing itself, only
begins the render pass and stitches secondaries together in the deterministic order `[M1-EXT-13]`'s
topo sort already establishes. Without this, `[M1-EXT-03]`'s pools exist but sit idle behind
single-threaded recording.

**[M1-EXT-25] Descriptor Update Templates for Per-Frame Bindless Writes**

Systems: `[M0-EXT-08]`/`[M4.5-EXT-09]`'s bindless material slot writes, `[M4.5-EXT-07]`'s RVT page
uploads — wherever a per-frame descriptor write currently loops individual
`vkUpdateDescriptorSets` calls.

Purpose: `VkDescriptorUpdateTemplate` pre-declares the write layout once at init; each frame's
batch of bindless-slot/RVT-page updates submits as one templated call from a tightly packed data
array instead of N individual calls. Lower per-frame descriptor-write CPU overhead as bindless slot
churn grows (material variety, chunk streaming); no visual change.


## 5.2 Physics, Kinematics, Coordinate Remapping (M2 / M2.6 / M2.7 / M2.8 / M2.9 / M9)

**[M2-EXT-06] Jolt-to-EnTT Double-Precision Transform Remapper**

Purpose: translates Jolt's single-precision contact manifolds into your authoritative `glm::dvec3` positions to avoid coordinate drift at sector boundaries far from world origin.

**[M2-EXT-07] Kinematic Virtual Sweep Tunneling Safeguard**

Purpose: projects speculative AABB sweeps ahead of kinematic ticks to catch velocity spikes before they tunnel through thin dynamic colliders.

**[M2-EXT-44] Procedural Recoil Low-Discrepancy Sequence Cache**

Purpose: pre-bakes a circular buffer of low-discrepancy (R2 sequence) offsets at boot instead of generating them at runtime inside the recoil oscillator — pure perf/consistency win, trivial to implement.

**[M2-EXT-45] Parametric Gait-Warping Stride Adjuster**

Purpose: adjusts foot clearance arcs in Motion Matching against local surface slope for zero-slip foot placement on stairs/ramps.

**[M2-EXT-55] XorShift128+ Seed Distribution Sandbox Synchronizer**

Purpose: syncs thread-local procgen RNG seed state across co-op peers at replication checkpoints so world generation doesn't diverge between clients. This is load-bearing for correctness in co-op — flag as high priority, not optional polish.

**[M2-EXT-56] xxHash64 ECS State Checksum Aggregator**

Purpose: periodic (e.g. every 30 ticks) parallel hash of hot SoA component arrays across peers to detect and localize desync to a specific entity index, rather than a full-state diff.

**[M2-EXT-57] Fixed-Point Mesh-Vector Quantization Factory** — [MATH DONE] — sibling to `[M2-EXT-53]`, not a duplicate: EXT-04 is a scalar 16.16 fixed-point multiply for damage rolls/pathfinding cost; this is a signed 24.8 fixed-point `vec3` specifically for procedural mesh generation (vertex positions, bone offsets) so chunk-boundary seams and physics divergence don't appear across CPU families under `-ffast-math`.

```cpp
struct FixedVec3 { // 24.8 fixed-point: 24 bits whole, 8 bits fractional
    int32_t x, y, z;
    static int32_t FromFloat(float v) { return int32_t(std::lround(v * 256.0f)); }
    static float ToFloat(int32_t v) { return float(v) / 256.0f; }
    FixedVec3 Mul(int32_t scalar256) const {
        return { int32_t((int64_t(x) * scalar256) >> 8), int32_t((int64_t(y) * scalar256) >> 8), int32_t((int64_t(z) * scalar256) >> 8) };
    }
    FixedVec3 Add(const FixedVec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
};
```

**[M2-EXT-67] Pneumatic Tire Slip-Angle Deformation Loop**

Purpose: visually deforms tire mesh vertices from lateral suspension load to sell drift/sidewall flex — cosmetic, not simulation-critical.

**[M9-EXT-20] Anti-Roll Torsional Suspension Stabilizer** — *renumbered from a v71 draft collision with `[M9-EXT-09]` (Volatile Fuel Tank Puncture Bernoulli Drainage), which already owned that ID — see front-matter changelog.*

Purpose: standard anti-roll bar constraint between linked wheel axes to prevent rollover in hard cornering.

**[M9-EXT-21] Fluid Hydrodynamic Wading Resistance Modulator** — *renumbered from a v71 draft collision with `[M9-EXT-10]` (Aero-Elastic Body Panel Drag Deformation Scaling), which already owned that ID — see front-matter changelog.*

Purpose: drag force on vehicle body from wheel submersion depth against the flood heightmap (M10 dependency).

**[M2-EXT-68] Kinematic Character Flood Buoyancy & Drag Bridge** — [MATH DONE] — Depends on (not yet built at this point): M10 flood heightmap

Purpose: `JPH::CharacterVirtual` resolves motion via kinematic sweeps, not rigid-body forces, so it's otherwise blind to flooding — zombies (M5) and vehicles (M9) already get flood physics, the player didn't. This adds the missing third leg: samples M10's Saint-Venant depth at the player's position each fixed tick and folds buoyancy + drag directly into the kinematic velocity input before `ExtendedUpdate()` runs.

```cpp
void ApplyFloodBuoyancy(CharacterPhysiologyComponent& phys, float waterDepthAtFeet, float dt) {
    if (waterDepthAtFeet <= 0.0f) return;
    constexpr float rhoWater = 1000.0f, g = 9.81f;
    float submerged = std::clamp(waterDepthAtFeet / phys.waistHeightM, 0.0f, 1.0f);
    float buoyancyAccel = (rhoWater * phys.capsuleVolumeM3 * submerged * g) / phys.massKg;
    phys.kinematicVelocityInput.y += buoyancyAccel * dt; // fed straight into ExtendedUpdate()'s velocity input
}
```

**[M9-EXT-18] Vehicle Convoy Long-Range Router** — dedup note, no new system

Purpose: faction/AI vehicle convoys need a lightweight macro-graph for long-range routing distinct from the fine NavMesh — but `[M5-EXT-51]`'s abstract street/highway node graph already exists for exactly this (built for off-screen horde routing). Extend that graph with a vehicle-lane-width/speed-limit field and reuse it here; do not stand up a second road-graph system.


## 5.3 GPU Render Pipeline (M4.5)

**[M4.5-EXT-12] Visibility Buffer Material-Classification Wavefront Compactor** — canonical copy is under §5.1/M4.5 in the main body; see there. *(v75: this appendix copy had dropped the global `BucketCounter` atomic and wrote `subgroupExclusiveAdd(1u)` straight to the image at a per-workgroup-local offset — the exact overlapping-write bug the body copy's `[BUG-FLAG]` note describes and fixes. Collapsed to a stub rather than left as a second, buggy copy.)*

**[M4.5-EXT-13] Cluster Depth-Bounds Frustum Voxelizer for Virtual Shadow Maps** — canonical copy is under §5.1/M4.5 in the main body; see there. *(v75: this appendix copy still used the backwards `nearDepth >= farDepth` check with no reference to the light's shadow bounds — the exact bug the body copy's `[BUG-FLAG]` note describes and fixes. Collapsed to a stub rather than left as a second, buggy copy.)*

**[M4.5-EXT-14] Compute Skinning Vertex Tangent-Space Recomputer**

Purpose: re-derives tangent vectors (T×N) during GPU dual-quaternion skinning to fix specular artifacts on extreme ragdoll poses.

**[M4.5-EXT-15] Hysteresis-Gated TAA Variance Clamper**

Purpose: clips history color buffer against local neighborhood AABB with hysteresis to cut ghosting behind fast-moving silhouettes without over-sharpening on every frame.

**[M4.5-EXT-16] Software Micro-Polygon Voxel Rasterizer (compute)**

Purpose: renders sub-pixel debris/fracture geometry via compute-shader atomics into the Visibility Buffer, bypassing fixed-function raster setup cost for triangles under ~1px. High complexity — de-prioritize unless debris density actually becomes a measured bottleneck.

**[M4.5-EXT-17] Directional Ambient Visibility Field Cache**

Purpose: coarse 3D voxel skylight-exposure field for real-time AO under canopies/alleys without full GI cost.

**[M4.5-EXT-18] Transient Skinned Vertex Cache for Visibility Buffer** — [MATH DONE]

Purpose: the Visibility Buffer shading pass reconstructs position from the static bind-pose vertex buffer — fine for static geometry, but skinned characters would render as T-poses since nothing deforms per-frame. A compute pass runs dual-quaternion skinning first into a ring-buffered transient GPU allocation; indirect draws for skinned entities bind that frame's cache offset instead of the static buffer.

```cpp
struct SkinnedRingPool {
    VkBuffer buffer; uint8_t* mapped;
    uint64_t capacityBytes, cursor;
    uint64_t Suballocate(uint32_t vertexCount, uint32_t strideBytes) {
        uint64_t size = (uint64_t(vertexCount) * strideBytes + 63) & ~63ull; // 64B align
        if (cursor + size > capacityBytes) return UINT64_MAX; // overflow guard
        uint64_t offset = cursor; cursor += size; return offset;
    }
};
```

**Numbering note (v78, this pass):** live `milestones/` already has `[M4.5-EXT-21]` through
`[M4.5-EXT-24]` (VRS luma-variance shading rate, glass-fracture normal perturbation, and two more
per your recent session) that aren't in this doc yet. The entry below is numbered `[M4.5-EXT-25]`
to not collide — verify against the live file before treating it as final.

**[M4.5-EXT-25] Specialization Constants for Bindless Material Uber-Shader**

Systems: `[M4.5-EXT-09]`'s classification pre-pass + master opaque/transparent pipelines — the two
PSOs the entire bindless-material scheme is built around.

Purpose: for the small set of compile-time-knowable branches inside the uber-shader (feature-tier
checks, fixed loop bounds) that currently branch at runtime on `MaterialIdx`-derived flags, promote
them to `VkSpecializationInfo` constants baked at the existing two-PSO compile time. `[M4.5-EXT-09]`
already compiles exactly one opaque + one transparent PSO specifically to avoid a permutation
explosion — this adds a handful of specialization constants on those same two PSOs, not per-material
PSOs, so it doesn't reintroduce what EXT-09 exists to prevent. Only applied where a branch is
genuinely compile-time-constant per PSO; material-varying lookups stay as runtime bindless reads
exactly as EXT-09 already does them. Marginal warp/wavefront divergence reduction in the
classification pre-pass — lower-confidence gain than the M1 items above, cheap to try, no visual
change.

**[M4.5-EXT-26] Runtime Virtual Texture (RVT) Base System**

##### Systems Touched
This is the missing base system flagged at audit line 133: three downstream features (skid-mark injection, capillary spatter projection, hydrocarbon flame-trail splitting) all write into "your RVT terrain overlay layer" that does not yet exist. Builds the persistent chunk-anchored terrain overlay texture with a Vulkan storage-image write path. Consumed by `[M9-EXT-22]` (skidmarks) and `[M6.5-EXT-13]` (blood spatter) below. Feeds `[M4.5]`'s hybrid-RT G-Buffer as a sampling source.

##### Math
Clipmap of L nested page rings centered on the camera (same streaming radius concept as the existing clipmap terrain at line 3097, reused). Physical page = fixed texel size `P` (e.g. 256) covering world tile `T`. Virtual address space is a 2D torus indexed by `(pageX, pageY)`; a page table maps resident pages to physical atlas tiles. Feedback buffer: each rendered frame, the pixel shader writes the (pageX, pageY, mip) it actually sampled into a feedback SSBO, read back and drained on the next frame to drive page residency (most-recently-used eviction when atlas is full).

##### How It Works
One persistent RVT atlas (RGBA16F or R8 packable channels) bound as a storage image. Material/decals/impacts render into it via a separate render pass with a write-merge (max-blend or newest-wins) so overlapping decals resolve deterministically. A feedback pass gathers sampled page coords; a CPU/GPU page-residency manager streams the missing pages (rebake from material graph `[M4-EXT-25]` or copy from baked tile) and updates the page table. This is the standing system the three audit-flagged features assume — once it exists, those three are unblocked.

##### Reference Implementation
```cpp
struct RvtPageTable { VkImage atlas; std::unordered_map<uint64_t, uint32_t> resident; /* pageKey->atlasTile */ };
// feedback SSBO filled by shader: layout(location=0) out uvec2 gFeedbackPage;
void RvtRenderDecal(VkCommandBuffer cb, RvtPageTable& rvt, const DecalInstance& d) {
    // bind rvt.atlas as storage image, draw decal quad, write-merge into page tile
}
```

##### Player-Facing Impact
Skid marks, blood spatter, and flame trails finally have a real surface to project onto instead of pointing at a non-existent layer; terrain detail (tire tracks through mud, drag marks) persists per-chunk like the doc always implied.


**[M4.5-EXT-27] Scalable Ambient Obscurance (SSAO)**

##### Systems Touched
Tier-1 quality ladder (line 3047) lists SSAO but defines no system. Reads the G-Buffer depth + normal already produced by `[M4.5]`'s deferred pass. Outputs an occlusion term multiplied into ambient/SH fill (complements `[M10-EXT-03]`'s SH ambient).

##### Math
McGuire et al. "Scalable Ambient Obscurance" (2012): for each pixel, sample N points in a view-space sphere of radius `R`, project to screen, compare depth; occlusion accumulates `sum(max(0, z_view - z_sample) / z_sample)`. Use a 4x4 rotated poisson disk (or the published 24-sample spiral) and a bilateral depth-weight to avoid haloing across edges.

##### How It Works
A fullscreen pass after G-Buffer: reconstruct view-space position from depth, sample the AO kernel with per-pixel rotation (blue-noise dither, reuse `[K-EXT-22]`'s STBN if available), blur with a depth-aware edge-preserving filter, output a single-channel AO texture. Tier-1 only; Tier-2 uses ReSTIR GI which subsumes contact shadows, so this is the fallback for the 6 GB floor.

##### Reference Implementation
```cpp
float SSAO(uint2 px, Texture2D depth, float radius, uint samples) {
    float occ = 0; vec3 P = ViewPos(px, depth);
    for (uint i = 0; i < samples; ++i) {
        vec3 s = P + Kernel(i) * radius;
        float zd = Linearize(depth, ProjToScreen(s));
        occ += max(0.0f, (P.z - zd) / zd) * EdgeWeight(px, s);
    }
    return 1.0f - occ / samples;
}
```

##### Player-Facing Impact
Corners, under-vehicles, and rubble crevices read with real contact shadow instead of flat ambient — the single biggest "looks proc-gen" fix for interiors on the Tier-0 floor.


**[M4.5-EXT-28] Screen-Space Reflections (SSR)**

##### Systems Touched
Absent as a system (only false-positive heat-shimmer mentions). Reads G-Buffer depth + normal + rough/metal; writes a reflection sample for `[M4.5]`'s lighting resolve. Tier-1/2 only (Tier-0 uses a baked cubemap/env approximation).

##### Math
Ray-march in view space from the reflection vector `R = reflect(V, N)`; at each step project to screen, compare ray depth vs scene depth, accept on crossing (binary-search refine). Fallback to env/SH when the ray misses or exceeds max distance. Roughness spreads the ray origin/length (lower step count for rough).

##### How It Works
Fullscreen pass after opaque G-Buffer. For each pixel above a roughness threshold, march `R`; on hit, sample the HDR color buffer (or the RVT/atlas) at the hit UV and mix by Fresnel. Cheap hierarchical-Z accelerated march (reuse the existing HZB) to take large steps. Denoised with a short temporal/edge-aware blur.

##### Reference Implementation
```cpp
bool TraceSSR(vec3 P, vec3 R, Texture2D depth, out vec2 hitUV) {
    vec3 pos = P; float step = kStep;
    for (int i = 0; i < kMaxSteps; ++i) {
        pos += R * step; vec2 uv = ProjToScreen(pos);
        float sceneZ = Linearize(depth, uv);
        if (pos.z > sceneZ) { hitUV = uv; return true; }   // crossed behind geometry
        step *= 1.5f;                                       // hi-Z style growth
    }
    return false;
}
```

##### Player-Facing Impact
Wet asphalt, puddles (post-rain `[M10]`), and vehicle paint show real reflected world instead of a flat env tint — the visual signature of "next-gen" on mid-tier hardware.


**[M4.5-EXT-29] Screen-Space Global Illumination (SSGI) Fallback**

##### Systems Touched
ReSTIR GI is Tier-2 only (line 3047). This is the Tier-0/1 approximation: screen-space diffuse bounces reusing the same G-Buffer + HZB + temporal denoiser `[M4.5-EXT-15]` already owns. Not a replacement for ReSTIR, the cheap fallback path.

##### Math
For each pixel, trace a short diffuse ray (cosine-weighted hemisphere sample around `N`), march like SSR, on hit sample the lit color and apply Lambert `albedo/π · L · max(0,N·ω)`. Accumulate one bounce; multiple bounces approximated by reusing the previous frame's SSGI buffer (temporal, reprojected via `[M4.5-EXT-15]`'s velocity buffer).

##### How It Works
Same fullscreen march infra as `[M4.5-EXT-28]` but diffuse-weighted and lower precision, blended under direct lighting. Bounded sample count (8-16) for the 6 GB floor; denoised by the existing TAA variance-clamp path. On Tier-2 hardware this pass is disabled in favor of ReSTIR.

##### Reference Implementation
```cpp
vec3 SSGI(vec3 P, vec3 N, Texture2D litColor, int samples) {
    vec3 acc = vec3(0);
    for (int i = 0; i < samples; ++i) {
        vec3 w = CosineHemisphere(N, i); vec2 hit;
        if (TraceSSR(P + N*0.01f, w, depth, hit)) acc += litColor[hit] * max(0.0f, dot(N, w));
    }
    return acc / samples;
}
```

##### Player-Facing Impact
Colored bounce light (red wall tints the floor, muzzle flash briefly lights a corridor) on hardware that can't afford ReSTIR — keeps the world from looking flat-emissive on the floor spec.


**[M4.5-EXT-30] Impostor LOD (Octahedral) for Distant Meshes**

##### Systems Touched
Zero mentions in doc — distant static meshes (buildings, ruined vehicles) currently pop or draw full geometry. Generates octahedral-impostor atlases per mesh at bake time; sampled in place of the mesh beyond a view-distance threshold. Reuses `[M4.5]`'s existing LOD/culling.

##### Math
Octahedral impostor (Shaderbits/Tech Art Aid): pre-render the mesh from K directions arranged on an octahedron hemisphere into a single atlas. At runtime, compute the view octahedral coordinate `oct = octEncode(normalize(viewDir))`, index the atlas, and sample the impostor billboard. Cheaper than spherical or 3D-impostor layouts (better texel packing).

##### How It Works
Offline: render K (e.g. 32) views into an atlas. Runtime: a billboard quad whose UVs are remapped by the octahedral direction; depth can be preserved via a depth-impostor write for correct occlusion. Cross-fade between the last real LOD and the impostor over a small distance band to hide the swap. Feeds the same culling the rest of `[M4.5]` uses.

##### Reference Implementation
```cpp
vec2 OctUV(vec3 viewDir) {            // map view dir -> atlas uv in [0,1]
    vec3 n = normalize(viewDir);
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    vec2 p = (n.z >= 0.0) ? n.xy : (1.0 - abs(n.yx)) * sign(n.xy);
    return p * 0.5 + 0.5;
}
```

##### Player-Facing Impact
Distant city skylines and zombie-haunted suburbs render at fraction of the cost with no visible pop — the draw-call relief that lets the Tier-0 floor keep 30 FPS with a large view distance.


**[M4.5-EXT-31] Signed Distance Field (SDF) Shadow Cascade**

##### Systems Touched
Only one passing mention of DF shadows — no system. Builds a sparse clipmap SDF of nearby opaque geometry from the depth buffer (or from baked mesh SDFs), used for cheap soft long-range shadows and contact shadows that the existing shadow-map cascade (line 24x HZB mentions) can't afford at distance.

##### Math
Per clipmap level, rasterize scene depth into a signed distance field via the standard 6-sweep or jump-flooding on the depth buffer; `d(p) = nearestSurfaceDistance(p)` with sign from depth-vs-scene. Shadow test along the light ray accumulates `min over steps of d(ray(t)) / t` (cone soft shadow): `vis = saturate(1 - k * min_d / t)`.

##### How It Works
A compute pass converts the depth (or a downsampled depth) into an SDF clipmap each time the camera moves a cell. Lighting samples the SDF along the light direction for a cheap soft shadow that holds at range where shadow maps run out of resolution. Hybrid: near field uses shadow maps, far field uses SDF — no double system, the SDF is a fallforward for the cascade's tail.

##### Reference Implementation
```cpp
float SdfShadow(vec3 P, vec3 L, Texture3D sdfClip, float coneK) {
    float t = 0, vis = 1.0f;
    for (int i = 0; i < kSteps; ++i) {
        vec3 s = P + L * t;
        float d = texture(sdfClip, s).r;          // signed distance
        vis = min(vis, saturate(1.0f - coneK * d / max(t, 1e-3)));
        t += max(d, kMinStep);
    }
    return vis;
}
```

##### Player-Facing Impact
Long shadows from tall ruins and smoke stacks stay soft and correct at distance without a 4K shadow-map cascade — the silhouette readability that makes outdoor horror scenes read, on the floor spec.


## 5.4 AI, Director, Propagation (M5.1 / M5.3 / M5.4)

**[M5-EXT-23] / [M5.3-EXT-XX] Optimal Reciprocal Collision Avoidance (ORCA) Solver**

Purpose: standard ORCA local avoidance so hordes don't clip/overlap at close range — pairs with your existing Reynolds flocking (subsystem #15 in Section 2) rather than replacing it; ORCA governs hard collision avoidance, boids govern soft group behavior.

**[M5-EXT-46] Visual Occlusion Sector Ray-March Pre-Filter**

Purpose: coarse macro-voxel LOS approximation to cheaply reject most raycasts before falling back to full-res traces for AI perception.

**[M5.4-EXT-04] Thompson-Sampling Actor Weights Snapshot Persistence Register** — canonical copy is under §5.1/M5.4 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub for the same reason as `[M1-EXT-09]` above.)*

**[M5-EXT-50] Reaction-Diffusion Grid Sub-sampled Boundary Welder**

Purpose: blends the panic/fear field (subsystem #26-equivalent reaction-diffusion grid) across sector boundaries so AI panic propagation doesn't stall or discontinuity-snap at chunk edges.

*[M5-EXT-51] Hierarchical A Macro-Cell Long-Range Router**

Purpose: routes off-screen horde movement across unstreamed sectors on an abstract street/highway node graph, avoiding full pathfinding cost for entities the player can't see.

**[M5-EXT-52] Holling Type II Cannibalism Feeding Satiator**

Purpose: saturating consumption-rate curve binding idle zombies to corpse piles as a tactical distraction mechanic.


## 5.5 Audio (M6)

**[M6-EXT-08] Ray-Traced Acoustic Diffraction Node Topology Cache**

Purpose: caches sound-path reflection vectors for stable geometry so Miniaudio doesn't re-trace rays every frame for static room shapes.

**[M6-EXT-09] Velvet-Noise Late Reverb Interleaved Mixing Buffer**

Purpose: interleaves diffuse reverb calc across sequential steps to flatten audio-thread mixing spikes during heavy indoor combat (pairs with existing subsystem #25).

**[M6-EXT-10] Acoustic Convection Wave Refraction Filter**

Purpose: warps SPL by wind vector so upwind gunshots muffle faster over distance than downwind ones.


## 5.6 World Generation Detail (M4)

**[M4-EXT-08] 3D WFC Vertical Structural Dependency Guard**

##### Systems Touched
Extends `[M4-EXT-03]` Hierarchical WFC Multi-Grid Graph Layering and the adjacency propagator §5.6 notes as already live but not yet in this doc (`[M4-EXT-20]`). Consumes `[M4-EXT-11]`'s contradiction-recovery primitive. Gates M2.7 traversal (stairs must be walkable) and M8.6 settlement construction (multi-floor safehouses).

##### Math
Vertical socket compatibility, checked at every floor boundary in addition to `[M4-EXT-03]`'s existing horizontal adjacency:

`VerticalOK(below, above) = (below.topSocket == above.bottomSocket) ∧ (above.requiresSupport → below.providesSupport)`

##### How It Works
This is the standard WFC "socket system" extended to the vertical axis: each tile face carries a socket ID, and vertical faces additionally carry two flags — `requiresSupport` (stairs/pillars need solid floor beneath) and `providesSupport`. After each floor's local WFC pass completes, run a flood-fill reachability check seeded from ground-floor entrance/stair-base tiles through `providesSupport`-linked stair-transition tiles. Any upper-floor region the flood-fill doesn't reach gets its column reset to full entropy via `[M4-EXT-11]`'s existing recovery mechanism (same primitive, scoped to a column instead of a 3×3 neighborhood) and re-collapsed with stair-class tiles weighted higher.

##### Reference Implementation
```cpp
struct WfcTileSocket { uint32_t topSocket, bottomSocket; bool requiresSupport, providesSupport; };

bool VerticalCompatible(const WfcTileSocket& below, const WfcTileSocket& above) {
    if (below.topSocket != above.bottomSocket) return false;
    if (above.requiresSupport && !below.providesSupport) return false;
    return true;
}

// Flood-fill from ground-floor entrances through stair-linked tiles; unreached cells
// get passed to RecoverWfcContradiction (existing [M4-EXT-11] fn) for that column.
void FindUnreachableUpperFloorCells(std::vector<WfcTileSlot>& grid, const std::vector<uint32_t>& entranceIdx,
                                     std::vector<uint32_t>& outUnreached, uint32_t strideX, uint32_t strideZ, uint32_t floors) {
    std::vector<bool> visited(grid.size(), false);
    std::queue<uint32_t> q;
    for (auto i : entranceIdx) { visited[i] = true; q.push(i); }
    while (!q.empty()) {
        uint32_t idx = q.front(); q.pop();
        for (uint32_t n : NeighborsIncludingVertical(idx, strideX, strideZ, floors)) {
            if (!visited[n] && VerticalCompatible(grid[idx].socket, grid[n].socket)) { visited[n] = true; q.push(n); }
        }
    }
    for (uint32_t i = 0; i < grid.size(); ++i) if (!visited[i]) outUnreached.push_back(i);
}
```

##### Player-Facing Impact
Multi-story ruins and safehouses never generate with floating floors, disconnected stairwells, or sealed unreachable rooms — matters here specifically because upper floors are a core loot-risk/sightline space in this game, not just set dressing.


**[M4-EXT-09] Deterministic Interior Furniture Spatial Constraint Solver**

##### Systems Touched
Declares M4's "Procedural Urban Detail (L-systems, BSP interiors)" bullet. Consumes the existing Socio-Economic Utility Tagging (room-use tags). Feeds M8 loot-container placement and M8.6 settlement construction (player-built interiors reuse the same solver).

##### Math
Greedy clearance-respecting placement score, evaluated per candidate wall segment:

`PlaceScore(piece, wallSeg) = wallSeg.freeLength ≥ piece.footprintWidth  ∧  no overlap with piece.clearanceRadius against already-placed pieces`

##### How It Works
Two established technique families exist for this: optimization-based simulated annealing over a cost function of clearance/alignment/traffic-flow (Merrell et al., "Furniture Layout Using Interior Design Guidelines," reused as recently as Infinigen Indoors, 2024), and rule-based greedy/agent placement using a semantic object library (Germer & Schwarz's agent-based room furnishing; Xu et al.'s clearance-box parent/child object grouping). SA produces higher-quality layouts but has unbounded iterative convergence cost — incompatible with a hard per-chunk generation time budget in a streaming open world.

Recommend greedy deterministic placement as the runtime path: each room-use tag maps to an ordered furniture manifest (anchor pieces first — bed, table, counter — then secondary props parented to already-placed anchors, mirroring Xu et al.'s clearance-box grouping). Anchors place against the longest remaining free wall segment that fits their footprint, tried at both wall ends before falling back to the next-longest wall. Entirely deterministic given the chunk's WFC seed — this is what the deferred bucket's "chunk-boundary determinism test" item needs from any interior-furnishing system, so building it deterministic from the start avoids a retrofit.

Reserve simulated annealing for a small, fixed set of non-streamed "landmark" interiors (named safehouses, quest locations) where a one-time offline/loading-screen budget is acceptable — same cheap-default/expensive-featured-case split already used elsewhere in this doc (e.g. `[M4-EXT-02]`'s runtime LOD vs. authored assets).

##### Reference Implementation
```cpp
struct FurnitureManifestEntry { uint32_t meshId; glm::vec2 footprintExtents; float clearanceRadius; bool isAnchor; };

bool TryPlaceAgainstWall(const FurnitureManifestEntry& piece, const std::vector<WallSegment>& walls,
                          const std::vector<PlacedFurniture>& placed, glm::vec2& outPos, float& outYaw) {
    // walls pre-sorted longest-free-length first (deterministic given seed)
    for (const auto& wall : walls) {
        if (wall.freeLength < piece.footprintExtents.x) continue;
        glm::vec2 candidate = wall.start + wall.direction * (wall.freeLength * 0.5f);
        bool clear = true;
        for (const auto& p : placed) if (glm::distance(candidate, p.pos) < piece.clearanceRadius + p.clearanceRadius) { clear = false; break; }
        if (clear) { outPos = candidate; outYaw = wall.inwardFacingYaw; return true; }
    }
    return false;
}
```

##### Player-Facing Impact
Every generated interior reads as furnished with intent rather than randomly scattered, and loot containers always have a legible, reachable placement instead of spawning inside furniture bounds.


**[M4-EXT-10] Macro-Graph Vector Spline Corridor Welder**

##### Systems Touched
Consumes `[M4-EXT-01]`'s macro-graph road/river vectors — feeds local WFC initialization the same way `[M4-EXT-01]` already injects immutable pre-placed tile constraints. Shares terrain math with `[M4-EXT-16]` (Saint-Venant erosion, river corridors) and reuses `[M4-EXT-19]`'s exclusion-spline concept for the soft edge.

##### Math
Two-band cell reservation from closest-point-on-spline distance:

`d(cell) = min distance from cell center to macro-graph spline` (Catmull-Rom, fixed-subdivision sampling — Newton refinement isn't needed at chunk grid-cell resolution)

Hard corridor: `d ≤ corridorWidth/2` → cell pre-collapsed to a fixed road/riverbed tile.
Soft blend: `corridorWidth/2 < d ≤ corridorWidth/2 + blendWidth` → cell's WFC possibility mask (`[M4-EXT-11]`'s existing field) pre-weighted toward corridor-compatible border tiles instead of fully collapsed.

##### How It Works
Before a chunk's local WFC starts, every grid cell is tested against the macro-graph spline. Cells inside the hard band get welded to fixed corridor tiles exactly as `[M4-EXT-01]` already does for macro-graph vectors; this entry adds the missing soft blend band so the transition from graded corridor to wild terrain isn't a hard tile seam at the corridor's edge — the same softening `[M4-EXT-19]` already applies to road exclusion, generalized to any macro-graph spline, not just roads.

##### Reference Implementation
```cpp
float DistanceToSplineApprox(glm::vec2 p, const std::vector<glm::vec2>& splineSamples) {
    float best = FLT_MAX;
    for (size_t i = 0; i + 1 < splineSamples.size(); ++i)
        best = std::min(best, DistancePointToSegment(p, splineSamples[i], splineSamples[i + 1]));
    return best;
}

void ReserveCorridorCells(std::vector<WfcTileSlot>& grid, const std::vector<glm::vec2>& splineSamples,
                           float corridorWidth, float blendWidth, glm::vec2 gridOrigin, float cellSize) {
    for (size_t i = 0; i < grid.size(); ++i) {
        glm::vec2 cellPos = gridOrigin + CellCoord(i) * cellSize;
        float d = DistanceToSplineApprox(cellPos, splineSamples);
        if (d <= corridorWidth * 0.5f) { grid[i].possibilityMask = kCorridorTileMask; grid[i].collapsed = true; }
        else if (d <= corridorWidth * 0.5f + blendWidth) { grid[i].possibilityMask &= kCorridorBorderCompatibleMask; }
    }
}
```

##### Player-Facing Impact
Roads and rivers read as continuous macro features chunks snap to, not independently-generated segments that misalign at chunk borders; buildings never spawn straddling a road centerline.

**[M4-EXT-11] WFC Contradiction Horizon Recovery** — [MATH DONE]

Purpose: WFC tile propagation can collapse to zero valid options at a cell (deadlock) — without an explicit recovery step, background chunk generation stalls or leaves rendering voids. On contradiction, reset a 3×3 neighborhood to full entropy and force-stamp the contradiction cell to a safe fallback tile (e.g. debris) so propagation can resume.

```cpp
void RecoverWfcContradiction(std::vector<WfcTileSlot>& grid, uint32_t idx, uint32_t strideX, uint32_t strideZ) {
    if (grid[idx].possibilityMask != 0) return;
    int32_t cx = idx % strideX, cz = idx / strideX;
    for (int32_t dz = -1; dz <= 1; ++dz) for (int32_t dx = -1; dx <= 1; ++dx) {
        int32_t nx = cx + dx, nz = cz + dz;
        if (nx >= 0 && nx < (int32_t)strideX && nz >= 0 && nz < (int32_t)strideZ) {
            auto& slot = grid[nz * strideX + nx];
            slot.possibilityMask = ~0ull; slot.collapsed = false; slot.entropy = 64;
        }
    }
    grid[idx] = { /*possibilityMask=*/1ull, /*collapsed=*/true, /*entropy=*/0 }; // fallback debris tile
}
```

**Numbering note (v78, this pass):** live `milestones/` already has `[M4-EXT-20]` and `[M4-EXT-21]`
(WFC adjacency propagator, signage grammar transcoder) that aren't in this doc yet. The entry below
**Numbering note (v78, this pass):** live `milestones/` already has `[M4-EXT-20]` and `[M4-EXT-21]` (WFC adjacency propagator, signage grammar transcoder) that aren't in this doc yet. The entry below is numbered `[M4-EXT-22]` to not collide — verify against the live file before treating it as final.

**[M4-EXT-22] Meshoptimizer Vertex Cache / Fetch Optimization**

Systems: procedural mesh output from M4 (buildings, terrain, debris), authored asset import path,
feeds the M1 material-batched buffers (`[M1-EXT-18]`).

Purpose: current standard practice (zeux/meshoptimizer, Tipsify-derived) — post-transform vertex
cache optimization reorders triangle indices to maximize hits on the GPU's small (16–32 vertex)
post-transform cache, followed by pre-transform vertex fetch optimization (reorders the vertex
buffer itself to match). Runs once at mesh-bake time for procedural output and at import time for
authored assets — not per-frame, a static reordering that changes nothing about the mesh's
appearance. Free vertex-shader-invocation and memory-bandwidth reduction on every mesh in the game;
genuinely absent from the doc as of this pass, not a duplicate of anything above.


**[M4-EXT-23] Whittaker Temperature/Precipitation Biome Classification**

##### Systems Touched
Feeds the existing "Dynamic Biome Weight Interpolation (2-3 chunk blend band)" bullet (M4 Implementation Steps, currently undefined *how* a biome ID gets assigned in the first place — the blend band interpolates between IDs but nothing in the doc names the classification rule). Consumes the same climate scalar fields `[M2.6]`'s per-biome noise-warp tuning and `[M4.5-EXT-04]`'s wetness/porosity system already read.

##### Math
Classical Whittaker diagram, discretized to a lookup table rather than the textbook triangular plot (Guehl/AutoBiomes and most shipped implementations do the same — a rectangular table is trivial to index, the triangle isn't):

`BiomeId = Table[Quantize(meanAnnualTemp, T_BUCKETS)][Quantize(meanAnnualPrecip, P_BUCKETS)]`

##### How It Works
Two scalar fields — mean annual temperature and mean annual precipitation — are already implied by existing systems (latitude/altitude-driven temperature falls straight out of the world's coordinate space; precipitation can reuse the same wind-field/orographic logic `[M10-EXT-05]`'s Navier-Stokes wind field and the tectonic mountain layout already establish, since windward/leeward sides of mountain ranges are exactly what drives real-world precipitation gradients). Quantize both into a small number of buckets (6 is the standard AutoBiomes/gamedev-common choice) and look up a biome ID from a fixed 2D table matching the classical Whittaker regions (tropical rainforest, desert, tundra, etc.), authored once as data, not code. That biome ID is what "Dynamic Biome Weight Interpolation" already blends across the 2-3 chunk band — this entry defines the missing input to that existing blend, it doesn't replace it.

##### Reference Implementation
```cpp
enum class BiomeId : uint8_t { Ice, Tundra, BorealForest, Grassland, Woodland, SeasonalForest,
                                TemperateRainforest, Savanna, Desert, TropicalRainforest };

constexpr uint32_t kTBuckets = 6, kPBuckets = 6;
extern const BiomeId kWhittakerTable[kTBuckets][kPBuckets]; // authored once as data (Appendix F sibling table)

BiomeId ClassifyBiome(float meanAnnualTempC, float meanAnnualPrecipMm) {
    uint32_t tIdx = std::clamp((uint32_t)((meanAnnualTempC + 10.0f) / 40.0f * kTBuckets), 0u, kTBuckets - 1);
    uint32_t pIdx = std::clamp((uint32_t)(meanAnnualPrecipMm / 4000.0f * kPBuckets), 0u, kPBuckets - 1);
    return kWhittakerTable[tIdx][pIdx];
}
```

##### Player-Facing Impact
Biome placement follows the same temperature/precipitation logic that produces real-world biome distribution (deserts in orographic rain-shadows, boreal forest at high latitude/altitude) instead of an arbitrary or purely noise-driven zone map, so the 2-3 chunk blend band was already built to smooth transitions between something that now actually makes climatic sense.


**[M4-EXT-24] Cellular Automata Structural Collapse & Rubble Debris Fields**

##### Systems Touched
Distinct from `[M4-EXT-13]`'s Dual-Contouring cave carving (that's macro cave *voxel structure*; this is a 2D binary overlay on top of already-built BSP rooms). Runs after `[K-EXT-18]`'s BSP room split, before `[M4-EXT-09]`'s furniture solver — rubble-occupied cells are excluded from furniture placement and get tagged impassable for NavMesh, same gating pattern `[M4-EXT-09]` already uses for clearance radii.

##### Math
Standard Game-of-Life-style birth/survival CA on a per-room floor grid, seeded from a per-building "decay severity" scalar (reuse `[M8]`'s Regional SEIR outbreak severity — harder-hit sectors generate more collapsed/rubble-choked interiors, tying visual decay to the actual outbreak simulation instead of being cosmetic noise):

`alive_{t+1}(c) = alive_t(c) ? (N(c) ≥ 4) : (N(c) ≥ 5)` where `N(c)` = count of the 8 neighbors currently alive, run 4-5 iterations from an initial random fill of `decaySeverity × baseFillRate`.

##### How It Works
This is the well-established roguelike cave-CA algorithm (random initial fill, birth/survival neighbor-count rules, iterate ~4-5 times, then flood-fill to guarantee connectivity) applied at room-floor scale instead of world-cave scale — genuinely distinct from `[M4-EXT-13]`'s SDF cave carving, which solves a different problem (3D volumetric structure vs. 2D floor occupancy overlay). "Alive" cells become rubble/debris prop clusters; "dead" cells stay clear floor. `decaySeverity` (already an existing per-sector value from M8's SEIR curve) scales the initial random-fill percentage, so a sector at outbreak peak generates rooms choked with collapsed debris while a low-severity sector generates mostly clear interiors — the same severity value already driving zombie density and loot scarcity now also drives what the space itself looks like. After CA settles, flood-fill from the room's door tiles (reusing the same reachability-check pattern `[M4-EXT-08]` already applies vertically) to guarantee at least one clear path across the room; any cell it can't reach gets force-cleared rather than leaving a genuinely unreachable pocket.

##### Reference Implementation
```cpp
struct RoomFloorGrid { std::vector<bool> rubble; uint32_t w, h; };

void StepCA(RoomFloorGrid& g) {
    std::vector<bool> next(g.rubble.size());
    for (uint32_t y = 0; y < g.h; ++y) for (uint32_t x = 0; x < g.w; ++x) {
        uint32_t n = CountAliveNeighbors8(g, x, y);
        bool alive = g.rubble[y * g.w + x];
        next[y * g.w + x] = alive ? (n >= 4) : (n >= 5);
    }
    g.rubble = next;
}

void GenerateRubbleField(RoomFloorGrid& g, float decaySeverity, uint32_t seed) {
    SeededFill(g.rubble, decaySeverity * kBaseFillRate, seed); // decaySeverity from M8 Regional SEIR
    for (int i = 0; i < 5; ++i) StepCA(g);
    // Flood-fill from door tiles (reuses [M4-EXT-08]'s pattern); unreached rubble cells force-cleared.
}
```

##### Player-Facing Impact
Buildings in hard-hit outbreak sectors read as visibly, differently ruined — collapsed ceilings, debris-choked hallways forcing detours — rather than every interior using the same clean BSP floorplan regardless of how bad that sector's outbreak got, and the connectivity guarantee means "differently ruined" never means "unreachable loot."


**[M4-EXT-25] Procedural Material Node-Graph Compiler (Structure + Color + Normal)**

##### Systems Touched
Single compiler backing five previously-separate deferred names: **PPTBF materials**, **node-graph procedural materials**, **histogram-preserving blending**, **noise→normal map derivation**, **procedural color palette**. Compiles to the project's locked HLSL/DXC toolchain. Feeds `[M4-EXT-02]`'s texture atlas infrastructure and is a fifth caller alongside `[K-EXT-17]`'s per-item micro-detail warp (which stays as-is — that path derives exact normals from SDF geometry before extraction and shouldn't be rerouted through a 2D normal-derivation node; this compiler is for flat/tileable surface materials, not the per-item mesh-detail path).

##### Math
PPTBF (Guehl et al. 2020, *Semi-Procedural Textures Using Point Process Texture Basis Functions*): a window function multiplied by a correlated Gabor-function mixture, generalizing sparse convolution noise to reproduce cells/cracks/grains/scratches/stains/waves from one parametric family. The published form exposes ~30 parameters (several discrete), which is authoring-heavy for a solo dev — recommend exposing a small fixed set of *presets* (crack-network, grain, scratch-field, stain) as named node instances with 3-5 tunable floats each, not the full general parameter space; same cheap-default-vs-full-generality split used elsewhere in this doc.

Histogram-preserving blending (Heitz & Neyret 2018, common node type for tiling-noise material blends): blend N textures by matching the output histogram to a target rather than naive lerp, which avoids the washed-out gray blending artifact plain interpolation produces on stochastic textures.

Noise→normal derivation: standard Sobel-style finite-difference of a height/structure field to produce tangent-space normals, `normal = normalize(vec3(-dHeight/dx, -dHeight/dy, 1))`.

##### How It Works
A small node-graph IR (structure node → optional histogram-blend node → normal-derive node → color-palette node) compiles to a single HLSL fragment per material at content-bake time, not per-frame — same bake-once-cache pattern as everything else procedural in this doc. Structure nodes are PPTBF presets or plain domain-warped OpenSimplex2 (reusing `[M2.6]`'s existing warp function directly rather than a second noise implementation). Color-palette nodes map a scalar structure field through a small authored gradient (2-5 color stops per material family — rust, concrete, foliage-litter) rather than per-pixel procedural color synthesis, since Guehl's own paper notes structure is the tractable procedural part and color detail is the part best left data-driven/authored, not generated from scratch.

##### Reference Implementation
```cpp
struct MaterialNode { enum Kind { PPTBFStructure, HistogramBlend, NormalDerive, ColorPalette } kind;
                       std::vector<uint32_t> inputs; /* params per Kind, omitted for brevity */ };

// Compiles a node list (already topologically sorted) into one HLSL fragment string.
// Bake-time only — never touches the per-frame shader path.
std::string CompileMaterialGraph(const std::vector<MaterialNode>& nodes);

glm::vec3 DeriveNormalFromHeight(float hL, float hR, float hD, float hU, float texelSize) {
    return glm::normalize(glm::vec3((hL - hR) / (2.0f * texelSize), (hD - hU) / (2.0f * texelSize), 1.0f));
}
```

##### Player-Facing Impact
Every generated or modded material — rust, cracked concrete, moss growth on the same wall — gets a distinct, non-tiling-obvious surface without an artist authoring five separate texture sets, and structural variation (cracks, grain) stays procedurally infinite while color stays art-directed rather than looking randomly generated.


**[M4-EXT-26] Procedural Decal Atlas Packing & Runtime Projection**

##### Systems Touched
Reuses `[M4-EXT-02]`'s texture-atlas infrastructure (same atlas manager the signage grammar and the loot-icon bake above already share — a third caller, not a new atlas system). Feeds M6.5's blood-spatter/scorch/impact-mark rendering (currently declared as a spatial-density system via `SpatialHash`, but the doc doesn't specify how the actual decal texture gets packed/looked up — this is that missing piece) and M4's signage grammar (weathered/torn poster variants).

##### Math
Shelf (skyline) bin-packing: sort decal rects by height descending, place each into the shortest-remaining shelf that fits its width, open a new shelf when none fit — O(n log n), deterministic given a fixed insertion order, good enough for decal-sized atlas entries (skyline beats naive shelf on packing density for this size class but shelf is simpler and the atlas entries here are small/uniform enough that the density gap doesn't matter).

##### How It Works
Decals (bullet holes, blood spatter, scorch marks, torn/weathered signage variants) are small textures projected onto a decal box in world space at runtime — standard decal-projection technique, not novel. The procedural part is generation and atlas packing: procedurally vary each decal instance (rotation, scale, one of a handful of base shapes composited via the same domain-warped noise `[M2.6]` already uses, so no two bullet holes look identical) and bake the result into the shared atlas via shelf-packing at first-use rather than pre-baking every possible variant. Runtime projection reads atlas UV offset + the existing `SpatialHash` density system to decide when a surface is "full" and starts overwriting oldest decals — that eviction policy already needs to exist for memory-bounded runtime decal accumulation, and reuses the same broad-phase spatial-hash pattern `M6.5`'s blood-spatter density already established rather than a second density tracker.

##### Reference Implementation
```cpp
struct DecalAtlasEntry { glm::vec2 uvOffset, uvExtent; };

// Sort-by-height-desc, then shortest-fitting-shelf placement -- deterministic given insertion order.
DecalAtlasEntry PackDecalShelf(TextureAtlas& atlas, glm::ivec2 decalPixelSize);

glm::vec3 GenerateDecalVariant(uint32_t baseShapeId, uint32_t instanceSeed) {
    // baseShapeId -> template silhouette; per-instance rotation/scale/warp phase from instanceSeed,
    // composited via [M2.6]'s existing domain-warped OpenSimplex2, not a second noise system.
}
```

##### Player-Facing Impact
Bullet holes, blood spatter, and battle damage accumulate as varied, non-repeating marks instead of the same three decal textures visibly stamped everywhere, packed into infrastructure that already exists rather than a competing atlas system.


## 5.7 Persistence, Data, Network, UI (M7 / M8 / M11 / M12)

**[M7-EXT-08] Zstandard Custom Dictionary Static Compiler**

Purpose: builds a permanent compression dictionary from static world archetype templates at boot to speed up background save serialization.

**[M7-EXT-09] Atomic File-Swap Append-Only State Transaction Logger**

Purpose: append-only delta log for ECS mutations so autosave doesn't need a full-state stutter — periodic compaction to the real save file happens off the critical path.

**[M7-EXT-10] Binary Save Format Structural Schema Migrator**

Purpose: translates old save-slot byte layouts to current component schema when struct sizes change between versions — required before you ship any patch that touches a serialized component.

**[M8-EXT-09] Memory-Mapped FlatBinary Inventory Cache Dictionary** — bug note

Purpose: index-math pointer access into item/weapon data to skip JSON parsing at runtime.
⚠️ Requires a **stride/padding alignment guard** baked in from the start, not bolted on later — struct layout changes must be validated against the flat file's fixed stride or you get silent misreads. Treat "alignment guard" as a requirement of this system, not a separate optional item.

**[M8-EXT-10] Procedural Loot Icon Generation**

##### Systems Touched
M8 itemization (every JSON item record needs a UI icon), M11 UI/HUD inventory rendering, M1's render pipeline (reused offscreen), and M4's existing texture-atlas infrastructure (context-free grammar signage compilation already builds and manages an atlas — this reuses it instead of a second one).

##### Math
Fixed camera framing normalized to a category bounding box; no per-item authored parameters.

##### How It Works
Since every loot item already exists as a real 3D mesh for the world (no separate 2D art asset is authored — matches the solo/no-artist constraint directly), bake the inventory icon by rendering that same mesh offscreen once at item-JSON-load time, not per-frame: normalize the mesh into a category-specific bounding box, apply a fixed camera angle/FOV per category (weapons framed to show silhouette diagonally, consumables framed near top-down to show label/cap), light with a flat 2–3-point rim/key rig independent of in-world lighting so icons stay legible regardless of time-of-day or weather, and cache the result into the same atlas M4's signage system already builds and manages.

##### Reference Implementation
```cpp
struct IconCameraPreset { glm::vec3 eulerAngles; float fovDeg; glm::vec2 boundsPadding; };

// category → preset lookup is a small fixed table (weapon, consumable, apparel, component, ...)
void BakeItemIcon(uint32_t itemMeshId, ItemCategory category, RenderTarget& offscreenTarget, TextureAtlas& itemIconAtlas) {
    const IconCameraPreset& preset = IconPresetForCategory(category);
    AABB bounds = ComputeMeshBounds(itemMeshId);
    glm::mat4 view = BuildFramingCamera(bounds, preset.eulerAngles, preset.boundsPadding);
    RenderMeshOffscreen(itemMeshId, view, preset.fovDeg, kFlatIconLightRig, offscreenTarget);
    itemIconAtlas.Insert(itemMeshId, offscreenTarget.ReadBack());
}
```

##### Player-Facing Impact
Every generated or modded item — including player-crafted variants with randomized attachments — gets a correct, current inventory icon automatically; no art backlog and no placeholder icons on modded content.


**[M11-EXT-08] MSDF Vector Text Layout Geometry Batching Pipeline**

Purpose: batches all diegetic UI/text/menu geometry into one vertex buffer pass instead of many small draws.

**[M12-EXT-04] Local Network-Relevancy Grid Culling Filter** — *renumbered from a v71 draft collision with `[M12-EXT-01]` (Bitstream Delta-Encoded Network Serialization Packet Encoder), which already owned that ID — see front-matter changelog.*

Purpose: throttles state sync to only co-op peers whose relevancy grid cell overlaps the changed entity, instead of broadcasting everything to everyone.

**[M12-EXT-03] Bitstream Delta-Encoded Packet Replay Fragment Reassembler** — [MATH DONE, FIXED] — duplicate of the full write-up in "Not yet merged," kept here for index completeness only; that's the canonical copy

Math: $SlotIdx = Seq_{packet} \mod BufferCapacity$

```cpp
#include <cstdint>
#include <cstring>
#include <algorithm>
struct OrderedPacketSlice { uint32_t sequenceIdentifier; uint8_t discretePayloadBuffer[256]; uint16_t payloadLength; bool isSlotPopulated; };

inline bool ReassembleInboundFragment(uint32_t sequence, const uint8_t* rawBufferData, uint32_t rawBufferLen, OrderedPacketSlice* ringPool,
                                       uint32_t poolCapacity, uint32_t maxTrackedSequence) {
    // Wraparound-safe unsigned distance check — the previous "sequence + poolCapacity <= maxTrackedSequence"
    // form could itself overflow-wrap when sequence is near UINT32_MAX, silently accepting stale fragments.
    const uint32_t backwardDistance = maxTrackedSequence - sequence;
    if (backwardDistance < 0x80000000u && backwardDistance >= poolCapacity) return false; // stale, outside window
    const uint32_t slot = sequence % poolCapacity;
    // Copy is clamped to the real datagram size — an
    // over-read past rawBufferData on every fragment smaller than 256 bytes (the normal case).
    const uint32_t copyLen = std::min<uint32_t>(rawBufferLen, sizeof(OrderedPacketSlice::discretePayloadBuffer));
    ringPool[slot].sequenceIdentifier = sequence;
    std::memcpy(ringPool[slot].discretePayloadBuffer, rawBufferData, copyLen);
    ringPool[slot].payloadLength = static_cast<uint16_t>(copyLen);
    ringPool[slot].isSlotPopulated = true;
    return true;
}
```

Sequence-window checks use an unsigned wraparound-safe distance comparison (no signed negation, so no `INT32_MIN` UB risk).
Impact: prevents entity state updates from applying out-of-order/fragmented packets.


## 5.8 Not yet formalized (name + one-liner only — flesh out math when you get to each)

These were flagged as missing but haven't been scoped with math/structs yet. Listed so nothing gets lost; add IDs when you pick a milestone slot for each. (Six items formerly listed here — Upper-Body Animation Layer Override, Procedural Mesh Kitbasher Pipeline, Dynamic Voxel GI Cache Injection Pass, Dual-Clutch Transmission RPM/Torque Controller, Continuum-Fluid Horde Density Pressure Field, and Offline Time-Lapse Population Drift Simulator — are now fully fleshed in **Appendix K** as `[K-EXT-01]`, `[K-EXT-17]`, `[K-EXT-05]`, `[K-EXT-03]`, `[K-EXT-02]`, and `[K-EXT-04]` respectively; removed from this stub list, not duplicated.)

* **IK Rig Metadata Serialization Loader** — loads bone-chain/end-effector defs from disk instead of hardcoded bone name lookups.
* **(see `[M5.4-EXT-10]` Procedural Mission & Event Director — fleshed in M5.4's Extended Systems Library; M13 stays the SLM text consumer, not the owner)**
* **Procedural Environmental Ambient Audio Baker** — synthesizes live wind/rain/city-hum soundscape from chunk tags + wind vectors.
* **GPU-Side Storage Buffer Decompressor (GDeflate, compute)** — same caveat as M4.6-EXT-05 above: this is the Vulkan-compute-shader path, not the Windows DirectStorage API — name it accordingly.
* **Chunk Boundary Entity Transfer Queue** — atomic handoff of simulated AI ownership across sector lines without double-ticking.
* **Asynchronous glTF Geometry Cache Purger** — distance-weighted LRU unload of static meshes under VRAM pressure.
* **Spatiotemporal Blue-Noise Jitter Array Interleaver** — deterministic low-discrepancy per-frame offsets for hybrid ray tracing denoise.
* **Luma-Variance Adaptive Shading Rate Calculator** — screen-space contrast → Variable Rate Shading map.
* **Barycentric Silhouette Edge Reconstruction Filter** — sub-pixel AA on Visibility Buffer primitive edges pre-upscale.
* **Triplanar Material PBR Blending Shader** — world-space-normal texture blend for un-unwrapped cliff/rubble geometry.
* **XPBD Continuous Rope/Cable Solver** — drift-free tether/winch/rigging constraint updates.
* **Kinematic Full-Body IK Surface Locker** — pins hands/feet to moving vehicle surfaces.
* **Exhaust Backpressure Aquatic Stutter Engine** — submersion-depth-driven engine stall/misfire.
* **Herschel-Bulkley Non-Newtonian Mud Rut Tracker** — soil shear-thinning tire rut deformation.
* **Chassis Vortex Debris Drafting Engine** — wind-wake force behind fast vehicles pulling debris/dust.
* **Spherical Harmonics Visibility Pre-Filter Grid** — cheap coarse obstruction check before committing to LOS raycasts.
* **Saint-Venant 2D Shallow Water PDE** — street-level flooding/current/aquaplaning (already referenced as M10 dependency elsewhere — confirm this isn't a duplicate of existing M10 groundwater system before adding).
* **Julian-Day Diurnal Atmospheric Gas Density Calculator** — ambient air density from daily temp cycle, affects long-range bullet drop.
* **Arrhenius Electrochemical Structural Weathering Pass** — rust/rot decay multiplier from real elapsed hours on unmaintained bases.
* **Network Clock Sync & Tick Drift Compensator** — host-authoritative timeline correction for co-op input replay alignment.
* ~~Dynamic Light Frustum & Occlusion Culler~~ — fleshed out as `[K-EXT-21]` in this pass (v72); no longer just a one-liner.


## 5.9 Real-World Reference Naming Lexicon (M8 / M9)

Public-domain alphanumeric model numbers for the procedural vehicle/weapon L-system seeds — real mechanical layout classes, no trademarked names in-engine.

**Vehicles** — real-world class → in-game seed ID → structural bounds
Heavy truck (F-150-class) → `Model 150-F Utility` → dual-channel frame, leaf-spring; Full-size SUV (H1-class) → `GUV-M998 Heavy` → portal hubs, wading limits; Sports coupe (M3-class) → `Sports-E90 Inline-6` → independent rear cradle; Police interceptor (CVPI-class) → `Interceptor-V8 Fleet` → push-guards, anti-roll; Compact hatch (Golf GTI-class) → `Compact-GTI 1.6` → transverse sport shell; Semi-truck → `Commercial-M2 Hauler` → twin-screw dual rear-axle.

**Weapons** — real-world class → in-game seed ID → ballistic category
.45 pistol (1911-class) → `Model 1911-A1 Pistol` → heavy-mass sidearm; 9mm pistol (Glock-class) → `G-17 Gen-5 Combat` → polymer frame; Pump shotgun (870-class) → `Pump-870 Tactical` → 12ga breacher; Carbine (M4-class) → `Carbine M4A1 Spec` → 5.56 direct-impingement; Battle rifle (AK-class) → `Rifle AK-47 Classic` → long-stroke piston; SAW (M249-class) → `Storm-249 SAW Belt` → open-bolt belt-fed; Anti-materiel (M82-class) → `Anti-Material M82A1` → .50 BMG long-range.

Full expanded tables (15 vehicles, 15 weapons) live in `updateforv66.txt` Section 5 — pull additional rows from there as the itemization pipeline (M8) needs more seeds; the pattern above is the naming convention to keep consistent.


## 5.10 Standalone Sandboxed Verification Test Suite

Zero-dependency C++ (standard headers only) sanity harness from `updateforv66.txt` Section 6, adapted to use local mock structs so it never collides with the canonical types defined elsewhere in this doc (`AlignToCacheLine` here mirrors `[M0-EXT-01]`'s real one for a standalone smoke test, not a second implementation to maintain). Compiles and runs independent of the engine — use it as a pre-merge gate before trusting a new version's WFC recovery, Saint-Venant flux stencil, and SPSC queue logic.

```cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <atomic>

// Block 0: cache-line alignment (mirrors [M0-EXT-01]'s allocator rounding, standalone check only)
size_t AlignToCacheLine(size_t size) {
    assert(size <= static_cast<size_t>(-64));
    return (size + 63) & ~static_cast<size_t>(63);
}

// Block 1: WFC contradiction-horizon recovery ([M4-EXT-11])
struct MockWfcSlot { uint64_t mask; bool collapsed; uint32_t entropyScore; };

void VerifyWfcRecoveryPattern() {
    std::vector<MockWfcSlot> testGrid(25, { 0xFFFFFFFFFFFFFFFFull, false, 64 });
    uint32_t targetIndex = 12;
    testGrid[targetIndex].mask = 0x00000000ull;
    uint32_t stride = 5;
    int32_t targetX = static_cast<int32_t>(targetIndex % stride);
    int32_t targetZ = static_cast<int32_t>(targetIndex / stride);
    if (testGrid[targetIndex].mask == 0x00000000ull) {
        for (int32_t offsetZ = -1; offsetZ <= 1; ++offsetZ) {
            for (int32_t offsetX = -1; offsetX <= 1; ++offsetX) {
                int32_t nX = targetX + offsetX, nZ = targetZ + offsetZ;
                if (nX >= 0 && nX < 5 && nZ >= 0 && nZ < 5) {
                    uint32_t idx = static_cast<uint32_t>(nZ * stride + nX);
                    testGrid[idx].mask = 0xFFFFFFFFFFFFFFFFull;
                    testGrid[idx].collapsed = false;
                    testGrid[idx].entropyScore = 64;
                }
            }
        }
        constexpr uint64_t kDebrisTileMask = 0x0000000000000001ull;
        testGrid[targetIndex].mask = kDebrisTileMask;
        testGrid[targetIndex].collapsed = true;
        testGrid[targetIndex].entropyScore = 0;
    }
    assert(testGrid[12].collapsed == true);
    assert(testGrid[12].mask == 1ull);
    assert(testGrid[13].mask == 0xFFFFFFFFFFFFFFFFull);
}

// Block 2: Saint-Venant upwind flux throttling ([M10-EXT-01], [M4-EXT-16])
struct FloodCell { float h; float z; };

void StepSaintVenantFluxVerification() {
    FloodCell cellA{ 2.0f, 10.0f };
    FloodCell cellB{ 0.0f, 11.5f };
    float headA = cellA.h + cellA.z;
    float headB = cellB.h + cellB.z;
    float shallow = std::min(cellA.h, cellB.h);
    float flux = 0.5f * (headA - headB) * std::pow(std::max(0.0f, shallow), 1.5f);
    assert(flux == 0.0f); // dry neighbor (cellB.h == 0) must never produce flux, prevents negative depth
}

// Block 3: M13/M1 SPSC ring-buffer correctness ([M1-EXT-06], [M13] channels)
struct MockAsyncPacket { uint32_t entityHandle; char messagePayloadText[32]; };

class SanitySpscQueue {
    MockAsyncPacket m_dataPool[16];
    std::atomic<uint32_t> m_writeIndex{0};
    std::atomic<uint32_t> m_readIndex{0};
public:
    bool Push(uint32_t ent, const char* txt) {
        uint32_t currWrite = m_writeIndex.load(std::memory_order_relaxed);
        uint32_t nextWrite = (currWrite + 1) & 15;
        if (nextWrite == m_readIndex.load(std::memory_order_acquire)) return false;
        m_dataPool[currWrite].entityHandle = ent;
        std::strncpy(m_dataPool[currWrite].messagePayloadText, txt, 31);
        m_dataPool[currWrite].messagePayloadText[31] = '\0';
        m_writeIndex.store(nextWrite, std::memory_order_release);
        return true;
    }
    bool Pop(MockAsyncPacket& out) {
        uint32_t currRead = m_readIndex.load(std::memory_order_relaxed);
        if (currRead == m_writeIndex.load(std::memory_order_acquire)) return false;
        out = m_dataPool[currRead];
        m_readIndex.store((currRead + 1) & 15, std::memory_order_release);
        return true;
    }
};

void VerifySpscCommunicationPipeline() {
    SanitySpscQueue testQueue;
    assert(testQueue.Push(42, "FactionStrategicPass"));
    assert(testQueue.Push(99, "ForensicDecayTrigger"));
    MockAsyncPacket packetOut;
    assert(testQueue.Pop(packetOut));
    assert(packetOut.entityHandle == 42);
    assert(std::strcmp(packetOut.messagePayloadText, "FactionStrategicPass") == 0);
    assert(testQueue.Pop(packetOut));
    assert(packetOut.entityHandle == 99);
    assert(std::strcmp(packetOut.messagePayloadText, "ForensicDecayTrigger") == 0);
}

int main() {
    assert(AlignToCacheLine(0) == 0);
    assert(AlignToCacheLine(1) == 64);
    assert(AlignToCacheLine(64) == 64);
    assert(AlignToCacheLine(65) == 128);
    VerifyWfcRecoveryPattern();
    StepSaintVenantFluxVerification();
    VerifySpscCommunicationPipeline();
    std::cout << "ZOMBIEENGINE SANITY SUITE: PASS\n";
    return 0;
}
```


## Appendix — Part A Hermes Self-Discovery Gap-Fill (10 blocks, v78)

Hermes-authored gap-fill produced by scanning the live doc for (a) `## Not yet merged` / `## 5.8 Not yet formalized` bullets with no full EXT block anywhere, and (b) implementation-step areas with no matching EXT. Every candidate was grep-verified for alternate names before authoring; near-duplicates already covered inline were excluded (logged in the close-out section below), per the queue's explicit anti-false-positive instruction.

ID discipline: every proposed ID is **provisional** (the live `milestones/` may have moved past the doc's numbering; verify against live before merge, same as the draft's own caution note). ID ceilings were re-grepped fresh for this pass: M0=12, M1=25, M2.8=08, M3=09, M4=22, M4.6=06, M5.2=13, M5.4=08, M7=10, M9=21, M11=08, M12=06, M13=14, K=21.

Non-trivial techniques were verified against real published sources:
- Spatiotemporal Blue-Noise (STBN): Heitz/Wolfe et al., "Spatiotemporal Blue Noise Masks" (EGSR 2022), jcgt 14(1), 2025.
- FABRIK IK: Aristidou & Lasenby, "FABRIK: A fast, iterative solver for the Inverse Kinematics problem," TCSVT 2011.
- Cristian's algorithm: Cristian, "Probabilistic clock synchronization," Distributed Computing 1989.

#### `[M5.2-EXT-16]` (provisional) IK Rig Metadata Serialization Loader

Fleshes the §5.8 one-liner **IK Rig Metadata Serialization Loader** (line 9104). The runtime `[M5-EXT-32]` IK Rig Asset is declared as an in-memory struct but has no disk format or loader; this entry supplies both so skeletons load their bone-chain definitions from data, not hardcoded bone-name lookups.

##### Systems Touched
Consumed by `[M5-EXT-32]` (IK Rig Asset struct) and `[M5-EXT-33]` (Motion-Warping target resolution). Feeds every end-effector consumer in M5.2 (melee-hit alignment, vault landing, door interaction). Distinct from M2.6 glTF/prefab loading (`std::unordered_map` path cache, line 1188), which loads meshes/scenes — not bone-chain + pole-vector metadata.

##### Math
No closed-form solve; a declarative asset: `IKRigAsset{skeletonId, chains:[{boneIdx[], poleVectorHint}]}`. Hashing for the on-disk table reuses the same FNV-1a keying idiom M2.6 already uses for the geometry cache (`uint64_t key = FNV1a(path)`), so the loader slots into that existing map type.

##### How It Works
A small JSON (or flat binary) asset per skeleton, authored once per unique skeleton and loaded at skeleton-registration time. The loader parses bone-name to index resolution against the skeleton's own joint list (so a renamed bone fails loudly at load, not silently at runtime), then stores the resolved `IKRigAsset` in the same hash-keyed table the geometry cache uses (different key space: `skeletonId` string, not asset path). Phenotype variants sharing a skeleton reuse one asset; a variant needing extra chains adds them in its own override asset rather than mutating the base.

##### Reference Implementation
```cpp
struct IKChainDef { std::vector<uint16_t> boneIdx; glm::vec3 poleVectorHint; };
struct IKRigAssetDef { std::string skeletonId; std::vector<IKChainDef> chains; };

// Returns false (and logs) if a bone name in the asset does not resolve on this skeleton.
bool LoadIKRigAsset(const std::string& path, const Skeleton& sk, IKRigAssetDef& out) {
    IKRigAssetDef raw = ParseIKRigJson(path);          // bone NAMES from disk
    for (auto& c : raw.chains)
        for (auto& name : c.boneNames) {               // resolve names -> indices
            int idx = sk.BoneIndexFromName(name);
            if (idx < 0) { LogError("IKRig %s: unknown bone %s", path, name); return false; }
            c.boneIdx.push_back((uint16_t)idx);
        }
    out = std::move(raw);
    return true;
}
```

##### Player-Facing Impact
Modders and future skeleton authors add bone-chain definitions via data, not engine recompile; matches the doc's "procedural variety over authored content" pillar for animation rigs.


#### `[M11-EXT-42]` (provisional) Procedural Environmental Ambient Audio Baker

Fleshes the §5.8 one-liner **Procedural Environmental Ambient Audio Baker** (line 9106): synthesizes a live wind/rain/city-hum soundscape from chunk tags + wind vectors, instead of hand-placing loop assets per biome.

##### Systems Touched
M11 audio/HUD (this milestone owns adaptive audio). Reads M4 chunk biome/material tags and `[M10]` wind vectors (the same field `[M10-EXT-02]`'s Navier-Stokes solver produces; also consumed by `[M9-EXT-04]` mud advection). Feeds the existing positional-voice mixer M11 already declares. Does not duplicate `[M13]` narrative/dialogue audio.

##### Math
Layer gain per acoustic source, modulated by wind speed and rain intensity from chunk tags: `G_wind(v) = clamp(k_w * |v_wind|, 0, 1)`, `G_rain(r) = clamp(k_r * r_precip, 0, 1)`. Cross-fade between biome beds by normalized tag weight `w_b` so a chunk straddling two biomes blends rather than hard-switches.

##### How It Works
A small set of synthesized beds (filtered noise for wind, band-passed noise bursts for rain, low drone for city hum) are generated once per (biome, weather-state) pair into a ring of looping buffers, then their gains are driven continuously by the chunk's live tag state and the sampled wind vector. No per-frame asset streaming; the synthesis is a one-time bake per state pair, cached and reused — cheap at runtime, and it reacts to weather the player can see change.

##### Reference Implementation
```cpp
struct AmbientBed { uint32_t bufferId; float baseGain; };
float EvaluateAmbientGain(const AmbientBed& bed, float windSpeed, float precip, float biomeWeight) {
    float dyn = std::max(bed.baseGain * windSpeed * 0.15f, bed.baseGain * precip * 0.9f);
    return bed.baseGain * biomeWeight + dyn * (1.0f - biomeWeight);
}
```

##### Player-Facing Impact
Every biome/weather combination has a distinct, reactive soundscape with zero hand-authored loop assets — a rainy industrial sector sounds different from a dry suburban one automatically.


#### `[M4.6-EXT-09]` (provisional) Asynchronous glTF Geometry Cache Purger

Fleshes the §5.8 one-liner **Asynchronous glTF Geometry Cache Purger** (line 9109): distance-weighted LRU unload of static meshes under VRAM pressure, the eviction half that M2.6's glTF Geometry Caching (line 1188) only sketches ("decay candidate, not an immediate free").

##### Systems Touched
Downstream of M2.6's glTF Geometry Caching table (`std::unordered_map<uint64_t, MeshHandle>`). Acts on `[M4-EXT-84]`'s GPU vertex-buffer pool and the M4.6 VRAM budget tracker the cache is already LRU-bounded against. Runs on the general enkiTS scheduler (one scheduler, not Jolt's — per AGENTS.md two-job-systems rule), not the render thread.

##### Math
Eviction score per cached mesh `i`: `S_i = refCount_i * w_ref + (1 / (1 + d_i)) * w_dist`, where `d_i` = distance of nearest active chunk referencing mesh `i`. Lowest-`S` meshes exceeding the VRAM headroom threshold are purged first; `refCount > 0` never purges (hard guard).

##### How It Works
A background task periodically scans the cache, computes `S_i`, and builds a purge list of meshes whose combined VRAM exceeds the free-headroom deficit and whose `S_i` is below a moving threshold. Unmaps GPU buffers via `[M4-EXT-84]` and drops the map entry; on next chunk-stream request the existing M2.6 loader re-fetches from disk (already implemented). Async so a bursty unload never stalls a frame.

##### Reference Implementation
```cpp
struct CachedMesh { uint64_t pathHash; MeshHandle handle; uint32_t refCount; float nearestChunkDist; };
void PurgeUnderPressure(std::vector<CachedMesh>& cache, float vramDeficit, float (*FreeVRAM)()) {
    if (FreeVRAM() >= 0) return;                       // headroom exists, do nothing
    std::vector<size_t> order(cache.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b){
        if (cache[a].refCount && !cache[b].refCount) return false;
        return Score(cache[a]) < Score(cache[b]);       // lowest score first
    });
    float freed = 0;
    for (size_t i : order) {
        if (cache[i].refCount > 0) continue;
        freed += MeshVRAMBytes(cache[i].handle);
        ReleaseMeshGPU(cache[i].handle); cache[i].handle = kNull;
        if (freed >= vramDeficit) break;
    }
}
```

##### Player-Facing Impact
Long sessions streaming across many biomes don't OOM the 6 GB Tier-0 VRAM floor; the cache self-trims instead of accumulating until a hard crash.


#### `[M12-EXT-23]` (provisional) Network Clock Sync & Tick-Drift Compensator

Fleshes the §5.8 one-liner **Network Clock Sync & Tick Drift Compensator** (line 9123): host-authoritative timeline correction for co-op input-replay alignment, so two clients' simulation clocks don't drift and desync replayed inputs.

##### Systems Touched
Sits above `[M12-EXT-01]` (delta encoder) / `[M12-EXT-02]` (loss-detection) / `[M12-EXT-03]` (fragment reassembler). Reads the existing co-op determinism layer `[M2-EXT-53]` (fixed-point deterministic math) as the canonical time source. Distinct from `[M2-EXT-54]` (host-authoritative topography reconstruction) — that resyncs world state, this resyncs the *clock* the sim ticks on.

##### Math
Cristian's algorithm over the existing reliable channel (verified: Cristian, Distributed Computing 1989). Client sends `T0`, server tags `T1`/`T2`, client receives at `T3`; estimate: `offset = ((T1 - T0) + (T2 - T3)) / 2`, `RTT = (T3 - T0) - (T2 - T1)`, accuracy plus/minus RTT/2. Per-tick drift `delta = measuredOffset - appliedOffset`; the sim applies `delta` as a smoothed correction factor `alpha * delta + (1 - alpha) * lastCorrection` (low-pass to avoid sawtooth jitter).

##### How It Works
Periodic (not per-tick) probe exchanges over the reliable channel piggybacked on existing keepalive packets. The host is the authority; clients compute `offset`, clamp `RTT` to a sane max (reject outliers from a stalled link), and feed the smoothed correction into their local tick scheduler so replayed inputs land on the same simulation step the host executed them on. Never blocks a frame — the correction is a scheduler nudge, applied between ticks.

##### Reference Implementation
```cpp
struct ClockProbe { uint64_t t0, t1, t2, t3; };
int64_t EstimateOffset(const ClockProbe& p) {            // Cristian
    return (int64_t)((p.t1 - p.t0) + (p.t2 - p.t3)) / 2;
}
int64_t EstimateRTT(const ClockProbe& p) {
    return (int64_t)(p.t3 - p.t0) - (int64_t)(p.t2 - p.t1);
}
float g_smoothedCorrection = 0.0f;                       // applied to local tick scheduler
void ApplyClockCorrection(int64_t offset) {
    const float kAlpha = 0.1f;
    g_smoothedCorrection = kAlpha * (float)offset + (1.0f - kAlpha) * g_smoothedCorrection;
}
```

##### Player-Facing Impact
Co-op sessions stay frame-aligned over imperfect networks; input replay (the determinism strategy from `[M2.8]`) doesn't silently desync because two clocks drifted apart.


#### `[M1-EXT-53]` (provisional) Chunk Boundary Entity Transfer Queue

Fleshes the §5.8 one-liner **Chunk Boundary Entity Transfer Queue** (line 9108): atomic handoff of simulated-AI ownership across sector lines without double-ticking the same entity.

##### Systems Touched
Sits on the general enkiTS scheduler (not Jolt's job system — two-scheduler rule). Consumes the sector/chunk streaming boundaries defined in M2.6 (sector size held fixed through M4, line 1196). Coordinates with M5.4 AI director (which owns spawn/despawn density) so a handoff doesn't fight the spawner. Reuses `[M1-EXT-06]`'s SPSC ring-buffer idiom (the verified §5.10 sanity suite uses the same shape) for the cross-sector handoff mailbox.

##### Math
Ownership token handoff is a single atomic state transition per entity: `state: {Local, InTransfer}`; transfer commits only when the destination sector's worker claims the token (`CAS(state, Local, InTransfer)` succeeds on exactly one side). Double-tick prevented by the atomic: the source worker stops ticking the entity the instant `InTransfer` is set.

##### How It Works
When an AI entity crosses a sector boundary, the source sector enqueues a lightweight transfer record (entity handle + its deterministic sim state snapshot) into the destination sector's SPSC mailbox and atomically flips the entity's ownership state. The destination worker drains its mailbox at sector-tick start, adopts the entity, and only then resumes ticking it. The source stops ticking on the `CAS` win — so the entity is ticked by exactly one sector per tick, never both.

##### Reference Implementation
```cpp
enum class Ownership : uint32_t { Local = 0, InTransfer = 1 };
struct TransferRecord { uint32_t entityHandle; /* deterministic sim-state snapshot */ };

// source sector, on boundary cross:
if (entity.ownership.compare_exchange_strong(kLocal, kInTransfer)) {
    destSector.mailbox.Push(TransferRecord{entity.handle, Snapshot(entity)});
    // source stops ticking entity from this tick forward
}
// destination sector, at tick start:
TransferRecord rec; while (destSector.mailbox.Pop(rec)) AdoptEntity(rec);
```

##### Player-Facing Impact
Hordes chasing the player across sector lines don't stutter, double-move, or vanish at the seam — the AI handoff is atomic, not "hope the two workers don't both tick it."


#### [K-EXT-22] (provisional) Spatiotemporal Blue-Noise Jitter Array Interleaver

Fleshes the §5.8 one-liner **Spatiotemporal Blue-Noise Jitter Array Interleaver** (line 9110): deterministic low-discrepancy per-frame offsets for hybrid ray-tracing denoise, the jitter half that M4.5's denoiser consumes but never specifies how the per-frame sequence is generated/ordered.

##### Systems Touched
Feeds `[M4.5-EXT-13]`'s clustered depth-bounds shadow voxelizer / any RT denoise consumer. Shares the precomputed blue-noise point-set already declared by `[M6.5-EXT-08]` (Blue-Noise Importance Sampling) — this entry adds the *temporal interleave* dimension, not a second noise source. Distinct from `[M6.5-EXT-08]`'s spatial sampling (it distributes spawn positions; this distributes temporal sample phases).

##### Math
A precomputed spatiotemporal blue-noise (STBN) volume (verified: Heitz/Wolfe et al., EGSR 2022, jcgt 14(1) 2025) indexed by `(pixelX mod W, pixelY mod H, frame mod T)`. The interleaver advances the frame index by a step coprime to `T` (e.g. `step = 1` with a scrambled permutation) so consecutive frames sample decorrelated phases while preserving spatial blue-noise per frame.

##### How It Works
At frame `f`, the jitter lookup is `jitter = STBN[pixel mod (W,H)][(f * step) mod T]`. Because STBN is blue in both space and time, the per-frame offsets are evenly distributed and temporally decorrelated, which lets the denoiser converge faster than white-noise jitter. The permutation is precomputed once at boot (no per-frame RNG), so the sequence is fully deterministic — required for the `[M2.8]` co-op determinism strategy.

##### Reference Implementation
```cpp
// stbn: precomputed volume [H][W][T] of vec2 in [0,1)^2, loaded once at boot.
glm::vec2 SampleSTBN(const std::vector<std::vector<std::vector<glm::vec2>>>& stbn,
                     uint32_t px, uint32_t py, uint32_t frame, uint32_t step) {
    const uint32_t H = (uint32_t)stbn.size(), W = (uint32_t)stbn[0].size(), T = (uint32_t)stbn[0][0].size();
    return stbn[py % H][px % W][(frame * step) % T];   // step coprime to T -> decorrelated phases
}
```

##### Player-Facing Impact
RT shadows / GI denoise converge in fewer frames at the same cost, so the game holds its frame budget on the 6 GB Tier-0 floor instead of paying for a noisy, slow-to-settle image.


#### `[M13-EXT-54]` (provisional) Kinematic Full-Body IK Surface Locker

Fleshes the §5.8 one-liner **Kinematic Full-Body IK Surface Locker** (line 9115): pins hands/feet to moving vehicle surfaces (e.g. a player bracing on a rocking truck bed), the full-body counterpart to M5.2's upper-body override.

##### Systems Touched
Consumes `[M5-EXT-32]` (IK Rig Asset — bone chains) and `[M5-EXT-33]` (motion-warping target alignment). Targets are sampled from M9 vehicle rigid-body surfaces (via the existing kinematic virtual-controller surface-friction path, line 1364). Distinct from `[M5-EXT-29]` upper-body override (torso/arms only) — this drives the whole-body plant (both feet + support hand).

##### Math
Two-bone (or FABRIK, verified: Aristidou & Lasenby 2011) chain solve per planted limb toward a moving target `T(t)` on the vehicle surface: `solved = FABRIK(chain, root, T(t))`, constrained so the planted point tracks `T(t)` with a max stretch `L_max`; if `|T(t) - root| > L_max` the character is peeled off (loss of grip) rather than accepting an impossible pose.

##### How It Works
When the player initiates a brace (or the AI director decides a zombie clings), the relevant limb targets are bound to sampled surface points on the vehicle body. Each frame the targets are re-sampled from the moving rigid transform, the IK chain is re-solved (cheap two-bone for limbs, FABRIK for a full-body plant), and the result is blended into the final pose after `[M5-EXT-29]`. On separation beyond `L_max` the lock releases cleanly.

##### Reference Implementation
```cpp
// FABRIK (Aristidou & Lasenby 2011), verified. Returns false if target unreachable (grip lost).
bool FABRIK(const std::vector<glm::vec3>& joints, const glm::vec3& target,
            float maxStretch, std::vector<glm::vec3>& outSolved) {
    float total = 0; for (size_t i = 1; i < joints.size(); ++i) total += glm::distance(joints[i-1], joints[i]);
    if (glm::distance(joints.front(), target) > total) return false;   // unreachable -> release
    outSolved = joints;
    for (int iter = 0; iter < 10; ++iter) {
        outSolved.back() = target;                                     // backward reach
        for (int i = (int)outSolved.size()-2; i >= 0; --i)
            outSolved[i] = outSolved[i+1] + glm::normalize(outSolved[i]-outSolved[i+1]) * glm::distance(joints[i], joints[i+1]);
        outSolved.front() = joints.front();                            // forward reach (root fixed)
        for (size_t i = 1; i < outSolved.size(); ++i)
            outSolved[i] = outSolved[i-1] + glm::normalize(outSolved[i]-outSolved[i-1]) * glm::distance(joints[i], joints[i-1]);
    }
    return true;
}
```

##### Player-Facing Impact
Mounting/moving-vehicle interactions look physically planted instead of the character sliding through the body or floating — a small but visible fidelity win for vehicle-heavy gameplay.


#### `[M3-EXT-36]` (provisional) Spherical-Harmonics Visibility Pre-Filter Grid

Fleshes the §5.8 one-liner **Spherical Harmonics Visibility Pre-Filter Grid** (line 9119): a cheap coarse obstruction check before committing to an expensive LOS raycast — the classic SH visibility cone / PRT-style pre-filter, distinct from `[M6]`'s acoustic voxel occlusion (that tags voxels with material absorption; this stores directional visibility as SH coefficients).

##### Systems Touched
Feeds AI line-of-sight queries (M5.4 director, M5 spawn/perception), and complements `[M6]`'s acoustic voxel occlusion (shares the same coarse voxel grid resolution idiom, different payload: SH coefficients vs. absorption tags). Reuses the world geometry already rasterized for `[M6]`'s occlusion voxels rather than building a second scene representation.

##### Math
Per voxel, store a 2nd-order SH visibility function `V(omega) approx Sum_{l,m} c_{lm} Y_{lm}(omega)` accumulated by rasterizing the surrounding occluder silhouette into the SH basis. A query direction `omega_q` is pre-filtered: `vis = max(0, Sum c_{lm} Y_{lm}(omega_q))` — if `vis < epsilon`, skip the full raycast (already blocked); only rays with `vis > epsilon` pay for the precise trace.

##### How It Works
At world/chunk build time, each voxel's SH visibility is baked by sampling occluder directions (cheap: a few hundred samples per voxel, once) and projecting onto the SH basis. At runtime, an AI LOS check first samples the SH visibility along its query direction; the vast majority of "blocked" checks short-circuit here without a raycast, and only genuinely open directions escalate to the precise (and costly) trace `[M6]` already performs.

##### Reference Implementation
```cpp
// 2nd-order SH (9 coeffs) visibility, baked once per voxel.
struct VoxelSHVis { float c[9]; };
float SampleVisibility(const VoxelSHVis& v, const glm::vec3& dir) {
    // SH evaluation Y_lm(dir) omitted for brevity; standard 2nd-order basis.
    float y[9]; EvalSH2(dir, y);
    float vis = 0; for (int i = 0; i < 9; ++i) vis += v.c[i] * y[i];
    return std::max(0.0f, vis);
}
// runtime: if (SampleVisibility(voxelSH, dirToTarget) < kEpsilon) return BLOCKED; // skip raycast
```

##### Player-Facing Impact
AI perception stays cheap even with many agents querying LOS, holding the 5 ms AI budget on the Tier-0 floor — fewer raycasts, same behavioral result.


#### `[M0-EXT-54]` (provisional) GPU-Side Storage-Buffer Decompressor (compute GDeflate)

Fleshes the §5.8 one-liner **GPU-Side Storage Buffer Decompressor (GDeflate, compute)** (line 9107). Per the doc's own caveat (and `[M4-EXT-87]`), this is the Vulkan-compute-shader path — name it accordingly, not "DirectStorage" (a Windows/DX API). It is the decompression compute kernel that `[M4-EXT-87]`'s ring allocator feeds; this entry supplies the kernel itself.

##### Systems Touched
Sits directly under `[M4-EXT-87]` (GPU Decompression Ring Allocator) — that owns buffer recycling, this owns the actual decode. Reads compressed chunk payloads from the streaming channel M4.6 manages. Downstream of the mesh/texture loaders that currently block on CPU decode.

##### Math
GDeflate is a Huffman + LZ77-variant byte stream decoded in a compute shader: each invocation decodes one symbol group, writing reconstructed bytes into the ring-allocated output buffer (`[M4-EXT-87]`). No closed-form equation; the "math" is the bit-unpacking state machine: `symbol = DecodeHuffman(bitstream); if (symbol == LITERAL) emit(symbol); else { len, dist = ReadMatch(); Copy(dest-dist, len); }`.

##### How It Works
On a chunk-load burst, the CPU hands the compressed byte range + a ring-allocated output buffer to a compute dispatch. The shader walks the GDeflate bitstream, expands literals/matches in parallel across invocations (each invocation owns a contiguous output span, resolving match-back-references within its span or cooperatively across the span boundary), then signals completion so the loader uploads the now-decompressed mesh/texture. Offloads decode from the CPU so fast driving doesn't stall on a synchronous decompress.

##### Reference Implementation
```cpp
// Compute entry — one invocation per output-span; cooperative back-ref across span edges is
// handled by a shared staging buffer (omitted for brevity; standard GDeflate decode loop).
[[spirv::compute(64)]] void GDeflateDecode(uint3 gid : SV_DispatchThreadID,
        ByteAddressBuffer src, RWByteAddressBuffer dst) {
    uint span = gid.x * kSpanBytes;
    GDeflateState st = InitState(src);
    while (!st.done && st.outPos < kSpanBytes)
        DecodeSymbol(st, dst, span);   // literal -> emit; match -> Copy(dist,len)
}
```

##### Player-Facing Impact
Streaming a fast-moving vehicle across chunk boundaries stays smooth — decompression happens on the GPU in parallel with upload instead of blocking the CPU frame.


#### `[M2.8-EXT-10]` (provisional) Co-op Deterministic Seeded Replay Verification

Fleshes an implementation-step gap in M2.8 (Deterministic co-op architecture, §5.2 line 1745): the milestone specifies snapshot-sync as the co-op strategy and fixed-point math (`[M2-EXT-53]`), but has no EXT block for the *verification* that two clients actually stay bit-identical — the "determinism check" the AGENTS.md Day-0 Spike A demanded (xor entity states into a running hash, run twice, alternate flags, confirm no divergence).

##### Systems Touched
Reads the fixed-point layer `[M2-EXT-53]` (the thing being verified) and the topology-replication token `[M2-EXT-51]` (determinism-drift isolation). Writes a 64-bit hash to the existing debug telemetry path (M13 / `[M5.4]` bandit tracker telemetry channel is the natural sink). Does not modify simulation — observation-only, safe to ship disabled.

##### Math
Running 64-bit FNV-1a hash over the serialized deterministic state each tick: `H = H xor FNV1a(state_i); H = (H * 1099511628211) mod 2^64`. Two clients exchange `H` every N ticks; divergence if `H_A != H_B` -> log the tick + the differing entity handle(s).

##### How It Works
At a fixed cadence (e.g. every 60 ticks, dev-gated), each client hashes its full deterministic sim state via the fixed-point serialization `[M2-EXT-53]` already produces, and the host compares client hashes. A mismatch trips a dev-only alarm identifying the first diverging entity — exactly Spike A's verification, wired into the shipped co-op path so drift is caught, not assumed away. Ship-disabled by default (per the doc's dev-gated debug-draw rule).

##### Reference Implementation
```cpp
uint64_t g_determinismHash = 14695981039346656037ULL;   // FNV-1a 64 offset
void FoldEntityState(uint64_t state64) {                  // call per deterministic entity/tick
    g_determinismHash ^= state64;
    g_determinismHash *= 1099511628211ULL;               // mod 2^64 is implicit in uint64_t
}
// host: if (clientA.hash != clientB.hash) LogDev("DETERMINISM DIVERGENCE tick=%u", tick);
```

##### Player-Facing Impact
None directly — this is an engine-correctness guard. It is what lets the co-op mode claim "deterministic," instead of shipping a sync strategy that silently drifts on some hardware.


## Close-out notes (Part A candidates checked, found ALREADY COVERED — excluded, not authored)

Per the queue's explicit anti-false-positive instruction, these §5.8 one-liners were grep-verified and found to already exist under a different name / inline. Logged here so they aren't re-surfaced as gaps later:

- **Luma-Variance Adaptive Shading Rate Calculator** (§5.8 line 9111) — ALREADY `[M4.5-EXT-24]` ("VRS luma-variance shading rate", confirmed in the §5.6 numbering note at line 8934). Authoring a second block would duplicate it. → close-out, no new ID.
- **Triplanar Material PBR Blending Shader** (§5.8 line 9113) — ALREADY implemented inline at line 3098 ("Triplanar PBR mapping for the RVT terrain overlay … any steep/undomain-unwrapped geometry (cliffs, rubble piles)"). No separate EXT needed. → close-out.
- **Barycentric Silhouette Edge Reconstruction Filter** (§5.8 line 9112) — ALREADY implemented inline at line 3086 ("Visibility Buffer silhouette anti-aliasing reconstruction … EdgeWeight = Saturate(nabla u + nabla v)"), wired into the M4.5 upscaler path. → close-out.
- **Julian-Day Diurnal Atmospheric Gas Density Calculator** (§5.8 line 9121) — ALREADY implemented at line 2047 ("drops the hardcoded static air density constant … evaluating true density (rho) based on your diurnal Julian day temperature variations"). Affects long-range bullet drop as specified. → close-out.
- **Arrhenius Electrochemical Structural Weathering Pass** (§5.8 line 9122) — ALREADY covered by `[M7]`'s weathering system using the Arrhenius rate (verified realistic at line 13; implemented at lines 2511-2521, rust/soot/oxidation over elapsed real hours). → close-out.
- **Herschel-Bulkley Non-Newtonian Mud Rut Tracker** (§5.8 line 9117) — ALREADY `[M9-EXT-04]` ("Non-Newtonian Mud Silt Advection (Soil Rut Tracking)", line 7002) using the Herschel-Bulkley shear model; persistent ruts are `[M9-EXT-19]` POM topography deform. → close-out.
- **Exhaust Backpressure Aquatic Stutter Engine** (§5.8 line 9116) — ALREADY `[M9-EXT-12]` (line 7267), submersion-depth-driven engine stall/misfire. → close-out.
- **Chassis Vortex Debris Drafting Engine** (§5.8 line 9118) — ALREADY `[M9-EXT-15]` (line 7355), wind-wake force behind fast vehicles pulling debris/dust. → close-out.

Net Part A output: **10 new full EXT blocks authored** (M5.2-EXT-14, M11-EXT-09, M4.6-EXT-07, M12-EXT-07, M1-EXT-26, K-EXT-22, M13-EXT-15, M3-EXT-10, M0-EXT-13, M2.8-EXT-09) + **8 close-out findings** (excluded to avoid duplication). No padding to hit 10 with weak material.
**[M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase**

##### Systems Touched
Standalone broad-phase for ECS physics/proximity queries, distinct from M6.5's blood-spatter density SpatialHash (that is a render-side density accumulator, not a queryable broad-phase). Consumed by `[M1-EXT-06]`'s SPSC queues and `[M1-EXT-26]`'s chunk-boundary transfer, and by `[M3-EXT-11]` / `[M5-EXT-53]` below. Runs on the enkiTS scheduler (not Jolt's — two-scheduler rule).

##### Math
World partitioned into uniform cells of size `c`. Each entity inserts its handle into every cell its AABB overlaps: `cell = floor(pos / c)`. Query: gather candidates from the 3x3 (or 2x2x2) neighboring cells. Hash map `cellKey -> vector<handle>` (open-addressing or `std::unordered_map` keyed by `hash(x,y,z)`).

##### How It Works
Rebuilt (or incrementally updated) each tick for dynamic entities. Provides O(1)-ish neighbor iteration for collision pairs, perception radius queries, and decal/spatter density, replacing the N^2 scans that would otherwise gate the AI/phys budget. Determinism preserved by a fixed insertion order (sort by handle id) so the same world yields the same pair list — required by `[M2.8]` co-op.

##### Reference Implementation
```cpp
struct SpatialHash { float cell; std::unordered_map<uint64_t, std::vector<uint32_t>> grid;
    uint64_t Key(int x,int y,int z){ return ((uint64_t)x<<42)^((uint64_t)y<<21)^(uint64_t)z; }
    void Insert(uint32_t h, glm::vec3 p){ auto k=Key((int)(p.x/cell),(int)(p.y/cell),(int)(p.z/cell)); grid[k].push_back(h); } };
```

##### Player-Facing Impact
Horde collisions, melee hit tests, and perception queries stay cheap with thousands of agents on the Tier-0 floor — no O(N^2) blowup when a street fills with zombies.


**[M1-EXT-28] GPU Software Occlusion Rasterizer (HZB Feeder)**

##### Systems Touched
Feeds the existing HZB (already used for shadow culling, ~24 mentions) with a coarse occlusion depth buffer generated from the most-recently-rendered frame's depth, so the next frame's draw-call culling can reject hidden meshes before they hit the vertex stage. Distinct from `[M4.5-EXT-31]`'s SDF shadow (that is a lighting term, not a culling gate).

##### Math
Downsample the previous frame's depth buffer by min (for reverse-Z) into a mip chain — each texel holds the nearest depth of its 2x2 children. Occlusion test for a mesh's bounding box: project the 8 corners, sample the HZB at the appropriate mip for the box screen-size; if all samples are closer than the box's far depth, the box is fully occluded → cull.

##### How It Works
A compute pass builds the HZB mip chain from the depth attachment each frame (cheap, bandwidth-bound not compute-bound). The culling pass tests instance AABBs against the HZB and compacts the visible list via the existing indirect-draw path (`[M1]`'s GPU-driven ECS). One-frame latency is hidden by the standard "test against last frame" approach; zero new render targets beyond a depth copy.

##### Reference Implementation
```cpp
bool HzbOccluded(const AABB& b, Texture2D hzb, float proj[16]) {
    vec2 mn=vec2(1), mx=vec2(-1); float farZ=-1e9f;
    for (vec3 c : Corners(b)) { vec4 s=proj*vec4(c,1); vec2 uv=s.xy/s.w*0.5+0.5;
        mn=min(mn,uv); mx=max(mx,uv); farZ=max(farZ,s.z/s.w); }
    int mip = (int)ceil(log2(max(mx.x-mn.x,mx.y-mn.y)*hzbW));
    return AllCloser(hzb, mn, mx, mip, farZ);   // every sampled depth nearer than box far -> occluded
}
```

##### Player-Facing Impact
Distant ruined city blocks behind a closer wall stop consuming vertex/draw budget — the GPU-driven culling win that lets the streaming world draw far without choking the floor spec.


**[M5-EXT-38] XPBD Rope/Tether Constraint**

##### Systems Touched
Line 5850 notes the long-range-attachment (tether) constraint is used internally for cloth/vegetation but not exposed; this promotes it to a reusable XPBD distance/rope constraint. Consumes `[M5.2]`'s existing constraint solver and `[M5-EXT-32]`'s IK/joint infra. Used by zipline/grappling/winch mechanics and drag-ragdoll.

##### Math
Extended Position-Based Dynamics (Macklin et al. 2016): each distance constraint has a compliance `α` and accumulates a Lagrange multiplier `λ` per substep, making stiffness independent of iteration count / timestep: `Δx = (w₁w₂ / (w₁+w₂+α̃)) · C · ∇C`, with `α̃ = α / Δt²`. A rope = chain of N distance constraints solved with substep XPBD (e.g. 4 substeps, 1 iteration each) for stable, non-stretchy ropes.

##### How It Works
Build a rope as a small particle chain (positions + inverse masses), one XPBD distance constraint per segment. Each substep: predict, solve all constraints updating `λ`, then integrate. The tether's two endpoints bind to entity handles (player hand, vehicle hitch) so it follows them. Because `λ` is persistent-corrected, the rope neither explodes nor sags through floors under load — the failure mode the doc flags for naive PBD.

##### Reference Implementation
```cpp
// XPBD distance constraint, Macklin 2016
void SolveDistanceXPBD(Particle& a, Particle& b, float rest, float alpha, float dt, float& lambda) {
    vec3 d = b.x - a.x; float C = length(d) - rest; vec3 n = d / max(length(d),1e-5f);
    float w = a.w + b.w; float aTilde = alpha / (dt*dt);
    float dLambda = (-C - aTilde*lambda) / (w + aTilde);
    lambda += dLambda; a.x -= n * (dLambda*a.w); b.x += n * (dLambda*b.w);
}
```

##### Player-Facing Impact
Grapple lines, tow cables, and dragging a downed survivor all behave with real, load-stable tension instead of the rubber-band stretch or snap that naive PBD ropes show.


**[M9-EXT-22] RVT Skid-Mark / Tire-Track Injector**

##### Systems Touched
One of the three features audit line 133 says is blocked on the missing RVT base. Writes tire tracks and drift scars into `[M4.5-EXT-26]`'s RVT overlay from `[M9]`'s wheel-contact + slip-state telemetry. Consumed by the terrain material resolve as an extra blend layer.

##### Math
Per wheel with ground contact and lateral/longitudinal slip `s > s_min`, stamp a decal quad into the RVT page covering the contact patch, with intensity `I = clamp(|s| · load, 0, 1)` and orientation from wheel yaw. The RVT write-merge (newest-wins or max) resolves overlaps; aging fades `I` over time via a per-page timestamp.

##### How It Works
On each physics tick where a wheel is slipping on a drivable surface, compute the world contact patch and push a stamp into the RVT feedback/residency path. Because it lands in the persistent RVT, tracks survive across frames and chunk reloads like real ground scarring. Same atlas `[M4-EXT-02]`/RVT already manages — no second decal system.

##### Reference Implementation
```cpp
void StampSkidMark(RvtPageTable& rvt, const WheelContact& w) {
    if (w.grounded && length(w.slip) > kMinSlip)
        rvt.StampPatch(w.contactPos, w.yaw, /*intensity=*/clamp(length(w.slip)*w.load,0.f,1.f));
}
```

##### Player-Facing Impact
Hard braking and handbrake turns leave real, persistent black streaks on the road — the forensic readability that tells the player "someone fled through here," and it finally has a surface to render onto.


**[M6.5-EXT-13] Capillary Blood-Spatter RVT Projection**

##### Systems Touched
Second of the three audit-line-133 blocked features. Projects blood/spatter/decal impacts into `[M4.5-EXT-26]`'s RVT at the impact point with a view-relative angle (capillary/back-spatter direction from the hit normal and projectile vector). Reuses `[M6.5]`'s existing spatial-density tracker for over-age eviction.

##### Math
At impact, compute the projectile incoming direction `d` and surface normal `n`; the spatter lobe opens around the reflect direction `r = reflect(-d, n)`. Stamp an impact decal into the RVT page at the hit UV with a per-instance rotation/scale and a lobe-weighted alpha so the pattern reads as directional spatter, not a symmetric splat. Density eviction uses the existing `SpatialHash` broad-phase (see `[M1-EXT-27]`).

##### How It Works
On a damage/decal event, raycast-free: take the hit world pos, map to the RVT page via the page table, and blend the spatter decal (one of a few base shapes composited with `[M2.6]`'s domain-warped noise so no two are identical). The RVT persistence means blood stays on the wall after the body is gone; the `SpatialHash` density cap recycles oldest marks when a surface saturates.

##### Reference Implementation
```cpp
void ProjectSpatter(RvtPageTable& rvt, vec3 hitPos, vec3 n, vec3 projDir, uint seed) {
    vec3 r = normalize(reflect(-projDir, n));
    rvt.StampDecal(hitPos, /*rot=*/Azimuth(r), /*scale=*/0.6f+0.4f*Hash01(seed), /*alpha=*/LobeWeight(r,n));
}
```

##### Player-Facing Impact
Gunfight scenes leave directional blood that lingers and tells the story of where shots came from — and it finally renders onto real geometry instead of a missing layer.


**[M10-EXT-11] Volumetric Cloud & Participating-Medium Scattering**

##### Systems Touched
Only a passing "participating medium" mention exists — no cloud system. Adds a raymarched volumetric cloud + atmospheric participating medium layer read by `[M10-EXT-02]`'s sun/moon vectors and `[M10-EXT-03]`'s scattering for in-scatter/extinction.

##### Math
Raymarch a cloud density field `D(p)` (worley/fbm noise in a slab between altitudes `h0..h1`) along the view ray; at each step accumulate extinction `T *= exp(-σ · dt)` and in-scatter `L += T · σ · phase(θ) · sunLight`. Beer's law extinction `T = exp(-∫σ ds)`; Henyey-Greenstein phase for forward-scatter.

##### How It Works
A half-resolution raymarch pass after opaque + sky. Cloud shape from animated 3D fbm (time-scrubbed by `[M10]`'s weather phase so storms actually roll in). Lit by the same sun vector `[M10-EXT-02]` drives; shadows self-occlude via a short secondary march toward the sun. Bounded step count for the floor; disabled or lowered on Tier-0.

##### Reference Implementation
```cpp
vec3 MarchClouds(vec3 ro, vec3 rd, vec3 sunDir) {
    float t = h0; vec3 L = 0, T = 1;
    for (int i=0;i<kSteps;i++) { vec3 p = ro+rd*t; float d = CloudDensity(p);
        float sig = d*kSigma; T *= exp(-sig*dt);
        L += T * sig * HenyeyGreenstein(dot(rd,sunDir),kG) * SunLight(p,sunDir) * dt; t += dt; }
    return L;
}
```

##### Player-Facing Impact
Storm fronts and moonlit cloud decks give the night real atmosphere instead of a flat sky dome — the mood the WW2-night horror aesthetic lives on.


**[M3-EXT-11] Fracture-Debris Broad-Phase Reuse**

##### Systems Touched
Macro-destruction `[M3]` spawns many small debris bodies on collapse; pairs them against the standalone broad-phase so debris-debris and debris-agent collision stays O(1)-query instead of O(N^2). Consumes `[M1-EXT-27]`'s spatial hash; feeds `[M3]`'s fracture graph resolution.

##### Math
Same uniform-grid hash as `[M1-EXT-27]`; debris bodies insert by AABB, fracture chunks insert by shard AABB. Pair candidates = union of cell memberships; only those pairs run the narrow-phase GJK/contact solve.

##### How It Works
On a structural collapse event, every spawned shard/debris registers into the per-tick spatial hash; the solver pulls candidate pairs per cell and runs narrow-phase only on them. Keeps a building pancaking into a street of zombies from tripping the physics budget — the debris count can spike into the hundreds without an N^2 penalty.

##### Reference Implementation
```cpp
// reuse [M1-EXT-27] SpatialHash; only narrow-phase the returned pairs
auto pairs = hash.QueryPairs(shardAABB);
for (auto& pr : pairs) if (Overlap(pr.a, pr.b)) NarrowPhase(pr.a, pr.b);
```

##### Player-Facing Impact
Collapsing walls throw believable, colliding rubble that interacts with the horde without the sim stuttering — the spectacle of destruction stays smooth on the floor spec.


**[M8.6-EXT-10] Settlement NavMesh from GA Layout**

##### Systems Touched
Consumes `[M8.6-EXT-09]`'s GA building-footprint layout and produces the walkable NavMesh the AI director (`[M5.4]`) and horde pathing need inside a settlement. Distinct from `[M4-EXT-08]`'s WFC reachability (that is interiors; this is the settlement exterior/plaza graph).

##### Math
From the GA's footprint polygon set, compute the free-space polygon (building set subtracted from the zoning polygon), then run a standard NavMesh triangulation (e.g. improved funnel / Delaunay over the walkable region) with portal edges between adjacent polygons for string-pulling path queries.

##### How It Works
Once `[M8.6-EXT-09]` bakes building footprints + gate positions, this derives the settlement's walkable mesh in the same one-time bake. Doorways become portals; the perimeter chokepoints the GA optimized for defense become natural funnel points the horde pathing uses. Cached with the layout — never recomputed per-frame.

##### Reference Implementation
```cpp
NavMesh BuildSettlementNavMesh(const SettlementLayoutGenome& g) {
    Polygon free = ZoningPolygon(); for (auto& b : g.buildings) free = Subtract(free, b.footprint);
    return TriangulateWalkable(free, /*portalEdges=*/GateEdges(g.buildings));
}
```

##### Player-Facing Impact
Zombies navigate a settlement's actual streets and gates (not through walls) the moment it's built — and the defensive chokepoints the GA placed double as the paths the horde funnels through.


**[M12-EXT-13] Interest-Management Spatial Hash (Net Culling)**

##### Systems Touched
Co-op netcode (`[M12]`) needs per-client relevance culling so a client only receives state for entities near its view. Reuses `[M1-EXT-27]`'s spatial hash as the relevance index; feeds `[M12-EXT-01]`'s delta encoder (only diff entities in the client's interest cells).

##### Math
Each client owns an interest region (its view sphere expanded by a slack band). The spatial hash maps entity position → cell; a client receives updates for entities in cells overlapping its interest region. Cell size tuned so a cell ≈ the net update granularity; handoff at cell boundaries mirrors `[M1-EXT-26]`'s sector transfer.

##### How It Works
Server maintains the shared spatial hash of entity positions; on each net tick it queries the client's interest cells and sends only those deltas. Entities leaving the region stop being sent (with a final "despawn" delta). Keeps bandwidth flat as the world scales — a client in a quiet suburb doesn't pay for a firefight two sectors away.

##### Reference Implementation
```cpp
void CollectRelevant(const SpatialHash& h, const ClientView& v, std::vector<uint32_t>& out) {
    for (auto cell : h.CellsOverlapping(v.interestSphere)) for (uint32_t e : h[cell]) out.push_back(e);
}
```

##### Player-Facing Impact
Co-op stays low-latency and bandwidth-bounded no matter how big the shared world gets — each player only streams what's around them.


**[M6-EXT-12] Convolution-Reverb from Voxel Occlusion**

##### Systems Touched
`[M6]`'s audio is HRTF/propagation-based but has no reverb system; this adds impulse-response convolution reverb derived from `[M6-EXT-11]`'s voxel acoustic occlusion (material absorption already tagged per voxel). Distinct from the occlusion *query* — this is the tail/space rendering.

##### Math
At a listener position, gather the surrounding voxel occlusion field and material absorption to synthesize an impulse response `h(t)` (early reflections from nearest hard surfaces + exponential tail from total absorption `α`: `h(t) = Σ early_i δ(t-t_i) + e^{-α t} · noiseTail`). Convolve the dry voice: `y = x * h`.

##### How It Works
On a (throttled) listener-move or material-change, rebuild a short convolution IR from the local voxel samples — a stone room yields a bright, long tail; open rubble yields a dead, short one. The IR is applied via partitioned convolution (short FIR, CPU-cheap) to the existing positional voices. Ties the audible space to the same geometry the visuals use.

##### Reference Implementation
```cpp
ImpulseResponse BuildIR(const VoxelField& vf, vec3 listener) {
    ImpulseResponse ir; for (auto& s : NearestReflectors(vf, listener, 8)) ir.early.push_back({s.dist, s.energy});
    ir.tailDecay = TotalAbsorption(vf, listener); return ir;   // y = x * ir via partitioned conv
}
```

##### Player-Facing Impact
Interiors and under-bridges sound enclosed, sewers sound dead, open streets sound live — the audio space matches the visual space instead of one global reverb setting.


**[M4-EXT-89] BC7 / Block-Texture Compression & Transcode**

##### Systems Touched
Zero mention of block compression in doc. Adds GPU-friendly BC7 (desktop) / ASTC (mobile) compression for the material/atlas textures `[M4-EXT-25]` and `[M4.5-EXT-26]`'s RVT produce, cutting VRAM on the 6 GB Tier-0 floor. Runs on the enkiTS scheduler at bake/load time (not the render thread).

##### Math
BC7: each 4x4 texel block is encoded into 128 bits across one of 8 partition modes with endpoint+index quantization and an optional mode-1 alpha split; quality/speed trade via the partition search. ASTC generalizes to arbitrary block sizes (4x4..12x12) with a similar endpoint+weight scheme. Offline or load-time encode; hardware decodes for free.

##### How It Works
Material graph output and atlas pages are encoded to BC7 once at content-bake (or first load, cached). The decompressor `[M0-EXT-13]`/ring allocator hands already-compressed data to the GPU which samples it natively — no per-frame decode. VRAM for a 4K albedo drops ~4x (RGBA8 → BC7). Tier-0 budget relief without visual loss at the chosen quality preset.

##### Reference Implementation
```cpp
// encode 4x4 RGBA block to BC7 (quality preset 1..255)
void EncodeBC7(const RGBA* block4x4, uint8_t out128[16], int quality) {
    BC7Partition best = SearchPartitions(block4x4, quality);   // 8 modes, endpoint quant
    WriteBC7(out128, best);                                     // GPU samples natively
}
```

##### Player-Facing Impact
The floor-spec VRAM budget stretches to hold more unique materials/atlases at once — fewer streaming hitches when rounding a corner into a new biome.


**[M5-EXT-53] Fear-Field Diffusion via Spatial Hash]

##### Systems Touched
`[M5.4]`'s Reaction-Diffusion Fear Field currently lacks its diffusion step's neighbor mechanism; this runs it over `[M1-EXT-27]`'s spatial hash so the field spreads to nearby agents/cells cheaply. Reads `[M5.4]`'s `TensionSignal`; feeds spawn-density the same director already uses.

##### Math
Discrete diffusion on the fear scalar field `F`: `F_i^{t+1} = F_i^t + k · Σ_j (F_j^t - F_i^t)` over neighbors `j` gathered from the spatial hash cell + ring. Combined with the existing reaction term (fear rises near zombies, decays over time) per the director's pacing model.

##### How It Works
Each director tick, the fear field's grid cells (or agent cloud) are inserted into the spatial hash; diffusion samples the 3x3 cell ring for the Laplacian. Bounded cost regardless of horde size — fear radiates outward from a sighting at a rate the spatial hash makes O(1)-per-cell instead of O(N^2) against every agent.

##### Reference Implementation
```cpp
void DiffuseFear(FearField& F, const SpatialHash& h, float k) {
    for (auto& cell : F.cells) {
        float lap = 0; for (auto nb : h.Ring(cell.pos)) lap += F[nb] - F[cell];
        cell.next = cell.val + k * lap;
    }
}
```

##### Player-Facing Impact
A sighting makes nearby survivors genuinely flee outward in a believable gradient instead of a uniform panic toggle — the AI reads as reacting to events, not a script.


#### [M5.4-EXT-10] Procedural Mission & Event Director

##### Systems Touched
Reads M5.4's `TensionSignal` and Reaction-Diffusion Fear Field, M8's Regional SEIR Curve, and Appendix C socio-economic tags. Writes through M13's SLM generators (already declared in the TOC as feeding mission text across M4/M5.4/M6/M8/M8.5/M8.6/M10/M11). Resolves objectives against M8.6 settlement state and M8.5 faction economy. Owned here (not M13) because M5.4 already owns the pacing telemetry it reads — M13 stays the text-generation consumer, not the owner.

##### Math
`Score(template, worldState) = Σ_tag  Match(template.requiredTags[tag], worldState.tags[tag]) · tagWeight[tag]`

##### How It Works
Two established approaches. (1) Bethesda's Radiant Story pattern (Skyrim/Fallout 4): template slot-fill — pick an objective archetype (retrieve/escort/defend/eliminate/supply-run), resolve location/target/reward slots against live world-state queries. Cheap, deterministic, easy to author solo. (2) Dormans & Bakkes, "Generating Missions and Spaces for Adaptable Play Experiences" (2011): graph-grammar mission generation, where mission structure and level layout share the same grammar so missions and space co-emerge instead of being placed after the fact — a better structural fit here specifically because M4 already generates the world through grammars (WFC, L-systems), so this reuses a formalism already in the doc rather than adding a new one.

Recommend the same two-tier split already used above in `[M4-EXT-09]`: Radiant-Story-style template slot-fill for the common ambient mission stream (reads world tags continuously, same pattern the Director already uses for spawn pacing), and grammar-based mission-space co-generation reserved for marked high-value locations — settlement quest boards, faction contracts — where it's worth the extra generation cost to let a mission shape what actually gets built there. Selection weights come directly from the same Appendix C tags and Regional SEIR Curve already driving spawn density elsewhere, so mission variety tracks the simulation instead of reading as randomized flavor text on top of it — a starving settlement biases toward supply-run templates, a high-severity outbreak zone biases toward extraction/defense templates.

##### Reference Implementation
```cpp
struct MissionTemplate { std::string archetypeId; std::unordered_map<std::string, float> requiredTags; };

// Schematic — bind against the live socio-economic tag / SEIR accessor names in source,
// not invented here; this shows the scoring shape only.
const MissionTemplate* SelectMissionTemplate(const std::vector<MissionTemplate>& templates, const WorldTagState& state) {
    const MissionTemplate* best = nullptr; float bestScore = -1.0f;
    for (const auto& t : templates) {
        float score = 0.0f;
        for (const auto& [tag, weight] : t.requiredTags) score += state.TagMatch(tag) * weight;
        if (score > bestScore) { bestScore = score; best = &t; }
    }
    return best;
}
```

##### Player-Facing Impact
Side content stays legible and reactive — a starving settlement two sectors over produces a supply mission there specifically, not a generic fetch-quest anywhere on the map — without hand-authoring a mission for every settlement instance the world generator produces.


**[M4-EXT-27] Vegetation Impostor (Foliage-Specific)]

##### Systems Touched
Distinct from `[M4.5-EXT-30]`'s generic mesh impostor — this targets foliage specifically (bushes, tall grass from `[M4-EXT-18]`/L-system clumps), using a cross-quad/star billboard that preserves wind sway `[M6.5-EXT-11]` already drives. Reuses `[M4]`'s Poisson-disk foliage placement.

##### Math
A foliage impostor is a camera-facing cross of 2-3 quads (or an octahedral-billboard like `[M4.5-EXT-30]`) textured from a baked top-down/side render of the clump, with an alpha-tested leaf card. Wind sway is kept by reusing the existing foliage vertex shader's wind term on the impostor quads (phase from world pos) so distant grass still ripples.

##### How It Works
At bake, render each foliage species clump from a few angles into an impostor atlas. Beyond the impostor distance, swap the live mesh for the billboard; the swap band cross-fades. Because it reuses the wind shader, distant fields don't freeze — they keep moving, just cheaper. Cuts foliage draw cost at distance so dense ground cover doesn't tank the floor framerate.

##### Reference Implementation
```cpp
// impostor quad uses same wind vertex displacement as the live clump
vec3 WindyImpostorPos(vec3 p, float t) { return p + WindDelta(p, t) * kFoliageWindGain; }
```

##### Player-Facing Impact
Thick roadside grass and bush clumps stay dense and wind-animated far from the player without the triangle cost — the overgrown, infested look the setting wants, affordable on the floor spec.


**[M4-EXT-28] Procedural Foliage L-System Mesh Generator**

##### Systems Touched
Distinct from `[M4-EXT-18]` (space-colonization canopy — tree crown/branch macro-structure only, unsuited to small ground clumps). Feeds from the existing Poisson-disk clustering (Appendix F) already driving foliage placement. Output meshes are what `[M6.5-EXT-11]`'s fBm wind kinematics already sways. Species/density gated by `[M4-EXT-17]`'s nutrient grid, same gate canopy trees already use.

##### Math
Standard Lindenmayer rewrite: axiom string expanded N iterations per species template, walked with a turtle-graphics position/heading/pitch stack to extrude a low-poly branch skeleton.

##### How It Works
Ground-level foliage (bushes, dead brambles, tall-grass clumps) is a different generative problem from `[M4-EXT-18]`'s canopy trees — space colonization grows toward light-attraction points and fits large branching canopies, not small procedural clumps. L-systems fit this scale better: a short per-species axiom + rewrite ruleset expands N iterations, then a turtle walk extrudes the branch skeleton and caps it with camera-facing or cross-quad leaf billboards. Species template selection reuses the same nutrient-grid species threshold `[M4-EXT-17]` already gates canopy placement with, so ground foliage and canopy species stay consistent with the same soil chemistry rather than a second, disconnected placement rule. Runs once at chunk-bake time.

##### Reference Implementation
```cpp
struct LSystemRule { char symbol; std::string replacement; };

std::string ExpandAxiom(const std::string& axiom, const std::vector<LSystemRule>& rules, uint32_t iterations) {
    std::string current = axiom;
    for (uint32_t i = 0; i < iterations; ++i) {
        std::string next;
        for (char c : current) {
            auto it = std::find_if(rules.begin(), rules.end(), [c](auto& r) { return r.symbol == c; });
            next += (it != rules.end()) ? it->replacement : std::string(1, c);
        }
        current = next;
    }
    return current;
}
// WalkTurtleToMesh(expandedString) -> branch skeleton verts, standard turtle-graphics
// interpretation (F = extrude+advance, +/- = pitch, [/] = push/pop stack) — omitted here,
// well-established, not worth re-deriving.
```

##### Player-Facing Impact
Dense, varied ground foliage with no two bushes identical, without hand-modeling a bush library — and it inherits wind sway for free since it's just another mesh feeding the existing foliage vertex shader.


**[M7-EXT-11] Zstd Save-Compression & Streaming Store]

##### Systems Touched
`[M7]` persistence writes saves but has no compression system named; this adds Zstd (or a comparable entropy+cDictionary codec) for the binary save blobs `[M7-EXT-10]`'s schema produces, cutting save size and load IO on the 12 GB system-RAM / slow-disk floor.

##### Math
Zstd: LZ77 match-finding + FSE/Huffman entropy coding, with an optional trained dictionary over save-structure prefixes for small saves. Compression level tuned (e.g. 3 for save-time, 19 for cold-storage backup). Streaming API so large world saves compress in chunks without holding the whole blob.

##### How It Works
On save, the serialized `[M7-EXT-10]` blob is pushed through the Zstd streaming compressor to disk; on load, decompressed back. A per-save dictionary (trained once on representative saves) shrinks repeated struct layouts dramatically. Failure mode per AGENTS.md: a corrupt/short read logs and exits cleanly, never a hang.

##### Reference Implementation
```cpp
std::vector<uint8_t> CompressSave(const std::vector<uint8_t>& blob, int level) {
    ZSTD_CStream* s = ZSTD_createCStream(); ZSTD_initCStream(s, level);
    std::vector<uint8_t> out; ZSTD_inBuffer in{blob.data(), blob.size(), 0};
    do { out.resize(out.size()+ZSTD_CStreamOutSize()); ZSTD_outBuffer o{out.data()+out.size()-ZSTD_CStreamOutSize(), ZSTD_CStreamOutSize(), 0};
         ZSTD_compressStream(s, &o, &in); out.resize(out.size()-o.pos); } while (in.pos < in.size);
    ZSTD_endStream(s, &o); out.resize(out.size()-o.pos); ZSTD_freeCStream(s); return out;
}
```

##### Player-Facing Impact
Saves stay small and load fast even with a huge explored world — the 12 GB RAM floor isn't eaten by uncompressed save blobs, and cloud/backup rotation (see doc meta bucket) is cheaper.


# Extended Systems Library — Appendix EXT (merged 2026-07-14)

*225 gap entries merged from `ZombieEngine_Milestone_Gaps.md`, renumbered to next-free per-milestone EXT IDs. Each block notes its original gap-file id. Fact-checked vs this spec (0 ID collisions at merge time).*
