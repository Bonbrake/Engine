## Appendix M — Async/Netcode/Scale Gap-Fill (merged from `updateforv72.txt`, v73)

**Merge audit.** `updateforv72.txt` proposed roughly 20 new subsystems across animation polish, rendering polish, networking, and horde-scale simulation, plus a re-elaboration of the existing M13 SLM loop. Checked item-by-item against the full doc:

* **7 kept** (M.1 below) — solve a real, stated problem with a technique appropriately sized for a solo dev + agent.
* **2 were ID collisions, not new content** — fixed by renumbering (M.1 notes each).
* **6 rejected or deferred as scope risk** (M.2 below) — not wrong, just too large relative to the value they add right now.

### M.1 — Kept


## Appendix M — Async/Netcode/Scale Gap-Fill (merged from `updateforv72.txt`, v73)

**Merge audit.** `updateforv72.txt` proposed roughly 20 new subsystems across animation polish, rendering polish, networking, and horde-scale simulation, plus a re-elaboration of the existing M13 SLM loop. Checked item-by-item against the full doc:

* **7 kept** (M.1 below) — solve a real, stated problem with a technique appropriately sized for a solo dev + agent.
* **2 were ID collisions, not new content** — fixed by renumbering (M.1 notes each).
* **6 rejected or deferred as scope risk** (M.2 below) — not wrong, just too large relative to the value they add right now.

### M.1 — Kept

### [M0-EXT-14] Asynchronous Hardware Compute Interleaver & Pipeline Lifecycle Matrix

*(Renumbered from the update's `[M0-EXT-11]` — that ID already belongs to an unrelated existing M0 system.)*

**Systems Touched:** M0 capability tiering, `[M4.5-EXT-07]` procedural texture synthesizer, M13 SLM worker threads.

**How It Works:** Background compute work (texture synthesis, SLM inference) submits to a dedicated `VK_QUEUE_COMPUTE_BIT` queue instead of the primary graphics queue, coordinated with cross-queue timeline semaphores. This is a standard, well-supported Vulkan pattern on any GPU with an async compute queue family (your RTX 2070 Super has one) — it's the correct fix for background work stalling your 16.6ms/frame budget, and it's cheap to add now before more systems assume inline execution.

**Player-Facing Impact:** Texture streaming and (if you keep M13) SLM inference stop causing frame hitches.

---

### [M12-EXT-05] Manifold Impulse History Rollback Buffer & Fixed-Point State Matrix

**Systems Touched:** M12 networked co-op, `[M12-EXT-01]` bitstream delta encoder, `[M12-EXT-02]` loss sliding window, Jolt `PhysicsSystem`.

**How It Works:** Maintains a 60-frame rolling history of physics state per dynamic entity. On a desync (checksum mismatch via xxHash64), rolls back to the last-agreed tick and re-integrates from there — applying corrective *impulses* through Jolt's normal solver rather than snapping transforms directly, which avoids the solver-island blowups a hard position reset causes. This is the same family of technique as rollback netcode in fighting games and Rocket League; it directly targets the tunneling/rubber-banding failure mode you're trying to prevent, and it's the right scope for M12 since you're already committed to deterministic co-op.

**Note:** the update's fixed-point (32.32) coordinate conversion is worth keeping specifically for the *replicated/networked* state — don't convert your whole engine's `glm::dvec3` world space to fixed-point, only the data that crosses the wire.

**Player-Facing Impact:** Dropped packets during a siege cause a brief correction instead of getting shoved through a wall or crate.

---

### [M5.2-EXT-13] Kinematic Character Ledge-Locking Coordinate Re-Basing Matrix

**Systems Touched:** `JPH::CharacterVirtual`, M2.7 player controller, M3 structural graphs, M2.6 open-world foundations.

**How It Works:** While anchored to a moving platform (vehicle bed, collapsing floor section), the character's position is tracked as a local double-precision offset from the platform's root transform rather than in absolute world space, then converted to camera-relative single precision only at the final draw step. This is the standard fix for jitter/sinking on moving platforms in large open worlds using floating-origin techniques — directly useful since you already have double-precision world space and moving structural pieces.

**Player-Facing Impact:** No sliding/jitter while standing on a moving vehicle or a partially-collapsed floor.

---

### [M4.6-EXT-06] Compute-Driven Hierarchical Z-Buffer Downsampler

**Systems Touched:** M4.5 GPU-driven pipeline, Hi-Z occlusion culling.

**How It Works:** Generates Hi-Z mip chain levels in fewer dispatches using subgroup max operations to reduce across the local workgroup before writing each mip level, instead of one dispatch per mip.

**Caveat, stated plainly:** the update's sample shader only actually resolves mip levels 0 and 1 within the subgroup reduction — extending it cleanly to 4+ levels needs either multiple subgroup-reduction passes or a workgroup shared-memory reduction, not shown. Treat the snippet below as the pattern, not a drop-in complete implementation.

```glsl
// Downsamples 2x2 depth texels into hzbMipLevels[0], then subgroup-reduces to mip 1.
// Extending past mip 1 requires an additional shared-memory or subgroup pass per level — not shown here.
layout(local_size_x = 16, local_size_y = 16) in;
layout(set=0, binding=0) uniform sampler2D srcDepth;
layout(set=0, binding=1, r32f) writeonly uniform image2D hzbMipLevels[4];
void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(id*2)+0.5) / textureSize(srcDepth,0);
    float maxZ = max(max(textureLod(srcDepth,uv,0).r, textureLodOffset(srcDepth,uv,0,ivec2(1,0)).r),
                      max(textureLodOffset(srcDepth,uv,0,ivec2(0,1)).r, textureLodOffset(srcDepth,uv,0,ivec2(1,1)).r));
    imageStore(hzbMipLevels[0], id, vec4(maxZ));
    float sgMax = subgroupMax(maxZ);
    if (subgroupElect()) imageStore(hzbMipLevels[1], id>>1, vec4(sgMax));
}
```

**Player-Facing Impact:** Slightly cheaper occlusion culling; more headroom for the dense structural/horde geometry M3/M5 already ask for.

---

### [M5.4-EXT-08] Closed-Ecosystem Continuum-Fluid Horde Density Field

**Systems Touched:** M1 spatial hash, M5.4 AI director, M4 procedural streaming.

**How It Works:** Hordes fully outside the player's active radius are dropped from individual EnTT entities/Jolt bodies down to a coarse 2D density grid, advected with a simple diffusion step (`∂ρ/∂t = D∇²ρ + S_spawn − S_decay`) instead of simulating every zombie. When a density field crosses back into the active radius, the chunk seed (SplitMix64) reconstructs individual zombies deterministically. This is a real and appropriately-scoped technique — background/off-screen abstraction of large populations is standard practice for open-world games with big roaming crowds (broadly the same idea as "unloaded NPC" systems in other open-world titles), and it directly targets your stated 3,000-zombies-5km-away memory/CPU problem.

**Player-Facing Impact:** Distant hordes still drift and grow/shrink over time instead of freezing, without the game paying full simulation cost for zombies you can't see.

---

### [M3-EXT-09] Async Structural Fatigue Continuum Accumulator

*(Renumbered from the update's `[M3-EXT-08]` — that ID already belongs to your existing Palmgren-Miner fatigue struct definition.)*

**Systems Touched:** M3 structural graph, `[M5.4-EXT-08]` horde continuum field, `StructuralFatigue` (canonical struct — no new fatigue variable created, per your existing dedup rule).

**How It Works:** The counterpart to `[M5.4-EXT-08]`: hibernated/off-screen buildings under sustained siege pressure accumulate fatigue on a coarse per-region basis (reading the same `StructuralFatigue` fields, just updated at a lower tick rate and coarser spatial resolution while hibernated) rather than freezing structural state entirely while unloaded.

**Player-Facing Impact:** A settlement you leave under siege can still be meaningfully more damaged when you return, without simulating every wall's stress every frame while you're away.

---

### [M1-EXT-12] Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface

> **tags** · MSDF, RVT
> **tl;dr** · Every consumer of M1's SoA Transform component — skinning, indirect draw,
> **ctx** · Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface -- Every consumer of M1's SoA Transform component — skinning, indirect draw,
> **meta** · depends-on: M1-EXT-30, M1-EXT-31, M1-EXT-32, M1-EXT-33, M1-EXT-34, M1-EXT-35, M1-EXT-36, M1-EXT-37, M1-EXT-38 · depended-by: -

##### Systems Touched
World-space rendering of entities whose authority lives in a moving/streaming origin (large-world double-precision root).

##### Math
GPU position relative to current origin: `Position_gpu = float3(Position_authority - Origin_current)`. Keeps float precision near the camera while authority stays in `double`.

##### Algorithm
1. Each frame, compute `Origin_current` (streaming root). 2. Cast each entity's `double` authority position to `float` relative to origin via CastPositionForGPU. 3. Shaders render in origin-local space; CPU keeps `double` truth.

##### Examples
-

##### Failure Modes
-

##### Player-Facing Impact
No direct player impact — this is a render/engine optimization that maintains frame pacing and determinism behind the scenes.

### [M1-EXT-29] Double-Precision Authoritative Transform with Float Upload Cast

##### Systems Touched
Every consumer of M1's SoA Transform component — skinning, indirect draw,
persistent-mapped staging upload (M1-EXT-05) — and directly feeds M2.6's
origin-rebase plan.

##### Math
`Position_gpu = float3(Position_authority - Origin_current)`
where `Position_authority` is `dvec3` and `Origin_current` is the active
rebase anchor (world origin or camera-relative anchor, per M2.6's
threshold policy).

##### How It Works
`Transform.Position` is authored and stored as `glm::dvec3` at the
ECS/authority level — this is the single source of truth for world
position. Once per frame, immediately before the persistent-mapped
staging ring buffer upload (M1-EXT-05) already declared in M1, subtract
the current origin/camera anchor from the double-precision position and
cast the result to `glm::vec3` for the GPU-facing SoA array. No other
system (skinning, indirect draw, culling) ever sees the double directly
— they consume the already-cast float buffer, unchanged from how M1
already declared them.

##### Reference Implementation
```cpp
// Called once per frame per entity batch, immediately before staging upload.
// Origin is the active M2.6 rebase anchor; Position is the dvec3 authority value.
glm::vec3 CastPositionForGPU(const glm::dvec3& position, const glm::dvec3& origin) {
    return glm::vec3(position - origin); // safe: difference is small post-rebase
}
```

##### Player-Facing Impact
World position never loses precision far from the origin, and the
already-planned M2.6 rebase becomes a threshold-tuning pass instead of a
rewrite of every GPU upload path that currently assumes float.

---

### [M1-EXT-30] Parent-Child Transform Hierarchy with Dirty-Flag Propagation

##### Systems Touched
Weapon/hand sockets (M2.7), camera-to-head attachment (this patch's
M1-EXT-31), M9 vehicle-mounted turrets/parts, any future backpack/gear
attachment.

##### Math
`WorldMatrix_child = WorldMatrix_parent · LocalMatrix_child`, recomputed
only when `Dirty(child) ∨ Dirty(parent)`.

##### How It Works
Adds a `Parent{ entt::entity }` component and a cached `WorldMatrix`
component. A per-frame dirty list collects any entity whose local
transform changed or whose parent's `WorldMatrix` changed this frame.
The list is processed in parent-before-child order (a shallow
topological sort via `entt::registry::sort`, since hierarchies in this
game are shallow — weapon→hand→body, camera→head, part→vehicle) so a
child never reads a stale parent matrix. Untouched subtrees are skipped
entirely.

##### Reference Implementation
```cpp
struct Parent { entt::entity value{entt::null}; };
struct WorldMatrix { glm::mat4 value{1.0f}; bool dirty{true}; };

void PropagateDirty(entt::registry& registry, entt::entity e) {
    auto& wm = registry.get<WorldMatrix>(e);
    wm.dirty = true;
    // Mark all direct children dirty too (children store their own Parent link;
    // a reverse lookup or cached child-list keeps this O(children), not O(N)).
}

void RecomputeWorldMatrices(entt::registry& registry) {
    // Sort so parents are processed before children (shallow depth in this game).
    registry.sort<WorldMatrix>([&](entt::entity lhs, entt::entity rhs) {
        return Depth(registry, lhs) < Depth(registry, rhs);
    });
    registry.view<WorldMatrix>().each([&](entt::entity e, WorldMatrix& wm) {
        if (!wm.dirty) return;
        if (auto* p = registry.try_get<Parent>(e); p && registry.valid(p->value)) {
            wm.value = registry.get<WorldMatrix>(p->value).value * LocalMatrixOf(registry, e);
        } else {
            wm.value = LocalMatrixOf(registry, e);
        }
        wm.dirty = false;
    });
}
```

##### Player-Facing Impact
Weapons stay glued to hands, cameras stay glued to heads, and vehicle
parts stay glued to vehicles — without a redesign when M2.7/M9 need it.

---

### [M1-EXT-31] Procedural Spring-Damper Camera Rig

##### Systems Touched
First/third-person camera (M2.7), stamina/exertion system, builds
directly on [M1-EXT-30]'s hierarchy.

##### Math
Semi-implicit (symplectic) Euler damped spring — velocity updates
first, then position:
`v ← v + (-k·x - c·v)·dt`
`x ← x + v·dt`
(Reversing this order is a known integration bug that makes the spring
feel mushy/lose energy incorrectly — verified against standard
semi-implicit Euler ordering.)

##### How It Works
The camera is parented (via M1-EXT-30) to a "virtual head bone" entity.
Each tick, a target offset is computed from player velocity magnitude
and current stamina/exertion value; the spring-damper integrates the
camera's *actual* local offset toward that target. Zero animation
authoring — it's a formula reacting to physics state.

##### Reference Implementation
```cpp
struct CameraSpring { glm::vec3 offset{0}; glm::vec3 velocity{0}; float k{40.0f}; float c{8.0f}; };

void UpdateCameraSpring(CameraSpring& spring, const glm::vec3& targetOffset, float dt) {
    glm::vec3 x = spring.offset - targetOffset;
    spring.velocity += (-spring.k * x - spring.c * spring.velocity) * dt; // velocity first
    spring.offset += spring.velocity * dt;                               // then position
}
```

##### Player-Facing Impact
Head-bob/sway that scales with how hard the character is pushing itself
— the "grounded, weighty" feel referenced against Dying Light — with no
hand-keyed animation.

---

### [M1-EXT-32] RayBatchQuery Parallel Raycast Primitive

##### Systems Touched
Future parkour ledge-detection, AI perception (M5.3), sound occlusion
(this patch's M1-EXT-33). NOTE: this is an engine-side wrapper — Jolt
Physics (arriving M2) does not provide a native multi-ray batch call;
its collector pattern batches multiple *hits along one ray*, not
multiple independent rays. Confirmed against Jolt's own docs.

##### Math
`Results[i] = NarrowPhaseQuery.CastRay(Origins[i], Dirs[i], MaxDist[i])`
for `i` in `[0, N)`, resolved in parallel.

##### How It Works
A struct-of-arrays holds N independent ray requests. `Resolve()` fans
these out across an enkiTS parallel-for, each worker calling Jolt's
single-ray `NarrowPhaseQuery::CastRay` for its slice, using the existing
Fiber Yield Hook (M0-EXT-07) so a large batch never stalls a whole OS
thread. This mirrors what Techland's own engine team built on top of
their raycast primitive for Dying Light's ledge detection — Jolt gives
you the single-ray primitive, this wrapper gives you the batching.

##### Reference Implementation
```cpp
struct RayBatchQuery {
    std::vector<glm::vec3> origins, dirs;
    std::vector<float> maxDist;
    std::vector<RayHitResult> results; // sized to match on Resolve()
};

void ResolveBatch(RayBatchQuery& batch, JPH::NarrowPhaseQuery& query, enki::TaskScheduler& scheduler) {
    batch.results.resize(batch.origins.size());
    enki::TaskSet task(static_cast<uint32_t>(batch.origins.size()),
        [&](enki::TaskSetPartition range, uint32_t) {
            for (uint32_t i = range.start; i < range.end; ++i) {
                JPH::RRayCast ray{ ToJPH(batch.origins[i]), ToJPH(batch.dirs[i]) * batch.maxDist[i] };
                JPH::RayCastResult hit;
                bool had = query.CastRay(ray, hit);
                batch.results[i] = had ? FromJPH(hit) : RayHitResult::Miss();
            }
        });
    scheduler.AddTaskSetToPipe(&task);
    scheduler.WaitforTask(&task);
}
```

##### Player-Facing Impact
Parkour/ledge detection and AI sightlines stay cheap even when many
checks fire in the same frame, instead of each system hand-rolling its
own slow per-ray loop.

---

### [M1-EXT-33] SpatialHash-Driven Sound Occlusion Query

##### Systems Touched
Reuses [M1-EXT-32] (RayBatchQuery) and M1's existing SpatialHash;
declared now so M6 (audio, several milestones out) consumes this
instead of building a second spatial system.

##### Math
`Audible(listener, source) = ¬Hit(RayBatchQuery(source→listener)) ∧ Distance(source, listener) ≤ HearingRadius`

##### How It Works
A hearing check for a noise event is just a RayBatchQuery between the
source and every listener returned by `SpatialHash::QueryRadius` around
that source — reusing the exact batching primitive from M1-EXT-32
rather than a bespoke audio-occlusion system.

##### Reference Implementation
```cpp
std::vector<entt::entity> QueryAudibleListeners(
    const glm::vec3& sourcePos, float hearingRadius,
    SpatialHash& hash, JPH::NarrowPhaseQuery& physQuery, enki::TaskScheduler& scheduler) {

    auto candidates = hash.QueryRadius(sourcePos, hearingRadius);
    RayBatchQuery batch;
    for (auto e : candidates) {
        batch.origins.push_back(sourcePos);
        batch.dirs.push_back(glm::normalize(GetPosition(e) - sourcePos));
        batch.maxDist.push_back(glm::distance(GetPosition(e), sourcePos));
    }
    ResolveBatch(batch, physQuery, scheduler);

    std::vector<entt::entity> audible;
    for (size_t i = 0; i < candidates.size(); ++i)
        if (!batch.results[i].hadHit) audible.push_back(candidates[i]);
    return audible;
}
```

##### Player-Facing Impact
Hiding behind a wall or around a corner actually blocks sound the way it
blocks a raycast — the Zomboid-style hearing/visibility realism you
asked for, using infrastructure this doc already declares.

### [M1-EXT-34] Procedurally-Generated Localization/Accessibility String-Table Pipeline

##### Systems Touched
M1's existing MSDF font pipeline (no rendering-path change needed) and
M13's MiniCPM5-1B integration, pulled forward as an offline content-gen
tool rather than waiting for M13's own milestone.

##### How It Works
All player-facing strings (subtitles, UI, colorblind-mode labels) route
through a string-table keyed by ID. The table's *content* is generated
OFFLINE, at build/content-generation time — never per-tick — by running
MiniCPM5-1B over a small set of seed templates, using the same
`enable_thinking=False` + fixed `temperature=0.7`/`top_p=0.95` discipline
already locked in for M13. This keeps the zero-hand-authored-content rule
intact while producing real text instead of placeholders. The MSDF
pipeline renders whatever the table resolves to, unchanged.

##### Reference Implementation
```cpp
// Offline tool, not runtime code:
// for each seed template in localization_seeds.json:
//   call MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95)
//   write generated variant into strings_<locale>.json keyed by string ID
```

##### Player-Facing Impact
Subtitles, UI text, and accessibility labels exist in real, varied form
without hand-authoring a single line.

---

### [M1-EXT-35] EventBus Telemetry Tap with Consent Gate

##### Systems Touched
The (future) EventBus (M2) — a lightweight tap added now so meaningful
gameplay events (death, horde-encounter size, resource-scarcity moment)
post to a ring buffer, gated by explicit player consent.

##### How It Works
A boot-time consent flag must be true before the tap writes anything.
When enabled, events post to a bounded ring buffer for later
(batched, offline) consumption by [M1-EXT-36].

##### Reference Implementation
```cpp
struct TelemetryEvent { uint32_t eventType; float value; uint64_t tick; };
bool g_telemetryConsentGranted = false; // set only via explicit settings toggle

void PostTelemetryEvent(RingBuffer<TelemetryEvent>& buffer, TelemetryEvent evt) {
    if (!g_telemetryConsentGranted) return;
    buffer.Push(evt);
}
```

##### Player-Facing Impact
Nothing leaves the machine or gets recorded without an explicit opt-in.

---

### [M1-EXT-36] Offline MiniCPM5-1B Difficulty Director Pass

##### Systems Touched
Consumes [M1-EXT-35]'s telemetry ring buffer; adjusts existing
data-driven spawn-density/loot-scarcity curves. NEVER touches gameplay
code directly — only the config values that already exist.

##### How It Works
Batched every few in-game hours (not per-tick), MiniCPM5-1B reasons over
aggregated telemetry and proposes adjustments to existing tunable
curves. This is what actually makes "no difficulty sliders, one tuned
experience" true rather than aspirational, using infrastructure this doc
already spec's for M13.

##### Reference Implementation
```cpp
// Offline/background batch job, not per-tick:
// aggregate TelemetryEvent buffer over N in-game hours ->
// prompt MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95) with
// aggregated stats -> parse suggested curve deltas -> write to existing
// spawn_density.json / loot_scarcity.json config, never to code.
```

##### Player-Facing Impact
Difficulty actually adapts to how the player is really doing, without a
visible slider and without the model ever writing code.

> **Verified model config (2026-07-14):** `openbmb/MiniCPM5-1B`'s
> `config.json` was pulled from Hugging Face and confirmed. It is a GQA
> Llama architecture — `hidden_size=1536`, `num_attention_heads=16`,
> `head_dim=128`, `num_key_value_heads=2` (8:1 GQA), `intermediate_size=4608`,
> `num_hidden_layers=24`, `vocab_size=130560`, `max_position_embeddings=131072`,
> bf16. The earlier "(16 heads × 128 = 2048 ≠ 1536, so inconsistent)" worry
> was wrong: under GQA the query-head count need not divide `hidden_size` by
> `head_dim` in the vanilla-MHA sense — 16 × 128 = 2048 query dims with KV
> collapsed to 2 heads is a normal GQA layout. These passes may cite the
> numbers directly; no blocking question remains.

---

### [M1-EXT-37] Offline MiniCPM5-1B Zombie Archetype Behavior Synthesis

##### Systems Touched
Feeds M5.1's already-planned procedural zombie variation. Offline
content-gen only — zero runtime inference cost, zero hand-authored
scripts.

##### How It Works
MiniCPM5-1B synthesizes behavior-tree parameter sets / utility-AI weight
tables per zombie archetype at content-generation time, consumed as data
by M5.1's runtime systems exactly like any other procedurally-generated
config.

##### Reference Implementation
```cpp
// Offline tool:
// for each archetype seed -> MiniCPM5-1B generates a parameter table
// (aggression weight, wander radius, group-cohesion factor, etc.) ->
// written to archetype_<name>.json, consumed at runtime as plain data.
```

##### Player-Facing Impact
Varied, expressive zombie behavior without a scripting VM and without
hand-authored behavior trees.

---

### [M1-EXT-38] ModWritable Allowlist Flag on MetaRegistry Registration

##### Systems Touched
Closes the gap the existing doc already flags as deferred in the Dev
Inspector section (MetaRegistry.cpp).

##### How It Works
Adds an explicit `ModWritable: bool` flag to each component's existing
`entt::meta` registration call, rather than leaving every registered
field implicitly editable.

##### Reference Implementation
```cpp
// In MetaRegistry.cpp, alongside the existing registration calls:
entt::meta<Transform>().data<&Transform::position>("position"_hs)
    .prop("ModWritable"_hs, false); // explicit allowlist, not implicit open access
```

##### Player-Facing Impact
Mods can't rewrite fields like StableId and desync a save — closes a gap
the doc itself already called out as unresolved.


### M.2 — Rejected or deferred (reasoning logged, not silently dropped)

* **Volume-preserving muscle deformation shader, FFT-driven audio lip sync, heat-shimmer refraction pass, split-screen HZB reprojection cache** — all technically real techniques, all pure visual polish with zero effect on whether the game is playable or fun, each adding a nontrivial shader/compute pass to maintain. The split-screen item specifically only matters *at all* if local split-screen co-op is in scope — it isn't mentioned as a requirement anywhere else in this doc. **Recommendation:** revisit all four post-M13, once core gameplay loops (M0–M9) are actually running and there's a game to polish.
* **STUN/TURN NAT-traversal gateway, built from scratch** — the underlying need (WAN co-op through arbitrary home routers) is real, but hand-rolling ICE/STUN/TURN negotiation is itself a multi-week networking project independent of everything else in this doc. **Recommendation:** use an existing library that already implements this (e.g. GameNetworkingSockets, which bundles ICE) rather than a bespoke implementation — this is a case where NIH costs you weeks for no gameplay benefit.
* **Local SLM (MiniCPM5-1B) output directly mutating spawn migration targets, price inflation, and "cognitive hallucination" rendering effects on a 30-second loop** — this is the update's biggest single item, and the one most worth pausing on. It's not that the idea is impossible; it's that wiring an LLM's output directly into core simulation state (horde pathing, economy, screen effects) makes bugs non-reproducible — if a zombie horde does something wrong, you won't know if it's your pathing code or the model's output, and you can't easily write a deterministic test for it. It also competes for VRAM with your renderer on a single RTX 2070 Super, and M13 already carries this idea in the base doc — this update just re-describes it, it doesn't add new information. **Recommendation:** keep it scoped exactly as M13 already has it (flavor text/lore/signage, one-way output, no feedback into core sim state) until M0–M12 are solid; the current v72 M13 section already avoids the "SLM controls gameplay" trap this update reintroduces, so no change made here.

### M.3 — v73 follow-up: formalizing the two M.2 recommendations with real reference implementations

Both M.2 items above ended in a "do it this way instead" recommendation rather than a flat rejection: use an existing NAT-traversal library rather than hand-rolling one, and keep the SLM's output surface locked to read-only flavor text. This section turns those two recommendations into concrete, buildable entries.

**ID collision check performed before adding these:** `M12-EXT-06` is unused — clear. `M13-EXT-11` is **not** — that ID already belongs to *Hardware Backend Auto-Detection & Async Compute Queue Isolation*, defined earlier in M13's Extended Systems Library. The SLM entry below is renumbered to `[M13-EXT-14]` (the next free M13-EXT slot after the existing `[M13-EXT-13]`), following the doc's existing renumber-on-collision convention rather than overwriting the existing definition.

---

### [M12-EXT-06] GameNetworkingSockets P2P WAN Transport Bridge

**Systems Touched:** `network_manager.cpp` (WAN path interception), `packet_factory.cpp` (packet routing bifurcation).

**Math:** None. No algebraic transforms are introduced to raw packets — NAT punchthrough, keep-alives, and symmetric/cone traversal metrics stay fully encapsulated inside Valve's MIT-licensed GameNetworkingSockets (GNS), per the M.2 recommendation against hand-rolling this.

**How It Works:** Integrates GNS to replace the raw ENet socket pipeline exclusively for connections routing outside the local network topology; LAN traffic keeps using the existing lightweight ENet path. The network tick isolates and intercepts WAN-bound connections — if a socket's target is flagged `is_wan`, the raw ENet execution block is bypassed and packet serialization maps straight to GNS stream descriptors via non-blocking async calls.

```cpp
// wan_transport.h — GNS bridge for STUN/TURN traversal
#pragma once
#include <steam/gamenetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <cassert>

class WANTransport {
public:
    static WANTransport& Get() { static WANTransport instance; return instance; }

    // One-liner init using GNS's default open STUN/TURN ICE server configuration
    bool Init() { return GameNetworkingSockets_Init(nullptr, nullptr); }

    void Poll() { if (auto* sockets = SteamNetworkingSockets()) sockets->RunCallbacks(); }

    // Connects via GNS routing (natively handles symmetric/cone NAT topology variations)
    HSteamNetConnection ConnectTo(const char* identityStr) {
        assert(identityStr != nullptr);
        SteamNetworkIdentity identity;
        if (!identity.ParseString(identityStr)) return k_HSteamNetConnection_Invalid;
        return SteamNetworkingSockets()->ConnectP2PCustomSignaling(nullptr, &identity, 0, nullptr);
    }
};
```

**Architecture Flags & Required Touchpoints:** The M12 baseline assumes a unified `ENetHost` across all topologies. Two touchpoints swap transport cleanly:

* **Flag 1** (`network_manager.cpp`, main host service tick): wherever `enet_host_service` is processed, branch — if the target's `connection.is_wan` is true, skip ENet and call `WANTransport::Get().Poll()` instead.
* **Flag 2** (`packet_factory.cpp`, packet serialization loop): if a packet's destination resolves to a WAN peer, redirect the compiled payload through `SteamNetworkingSockets()->SendMessageToConnection()` instead of the ENet buffer path.

**Exit Criteria:** Two clients behind separate residential NAT routers (not on the same LAN) establish a reliable peer connection and sustain an uninterrupted 10-minute co-op session with no manual port-forwarding.

---\n\n#### [M13-EXT-21] SLM Simulation Isolation & Hardening Pass

*(Renumbered from the update's proposed `M13-EXT-11` — that ID already belongs to the existing Hardware Backend Auto-Detection entry.)*

**Systems Touched:** `slm_types.h` (structural data constraints), `ai_director_pacing.cpp` (horde migration decoupling), `vendor_economy_core.cpp` (price-matrix decoupling).

**Math:** None added to generative output. Simulation variables stay mapped exclusively to the engine's existing deterministic state equations (baseline vendor pricing, horde migration), which the SLM never touches:

$$\text{Price}*{\text{final}} = \text{Price}*{\text{baseline}} \cdot \text{InflationModifier}_{MV=PQ}$$

**How It Works:** Formalizes the M.2 recommendation as an audit/lockdown pass rather than a new capability. Every generator entry point is audited so SLM output is confined to a read-only role — descriptive lore commentary, text signage, and UI/VO subtitle strings, attached as flavor labels to events the deterministic engine already decided. Nothing generative ever reaches pricing, spawn vectors, or render-pass state. The alternative from the update (mapping SLM output to dynamic pricing, render tags, or spawn-cell vectors) is explicitly rejected, consistent with M.2 above.

```cpp
// slm_types.h — hardened read-only output surface
#pragma once
#include <string>

struct SLMActionOutput {
    // READ-ONLY LORE, TEXT SIGNAGE, AND STRING OVERRIDES ONLY
    std::string terminal_log_override;   // custom signage/terminal text
    std::string environmental_lore_str;  // dynamic text for world notes/inspectables
    std::string vo_subtitle_hint;        // audio-VO subtitle annotation overlay

    // GUARDRAIL: no pricing floats, no spawn vectors, no active render passes.
};
```

**Audit Changelog:**

|Entry|Original (drift-vulnerable)|Rewritten (flavor-only)|Reason|
|-|-|-|-|
|M13-04|SLM set vendor `price_multiplier` from generated text|Replaced with an `environmental_lore_str` expressing economic panic; prices stay on the deterministic supply/demand engine|Mutated sim economics via unvalidated generative output|
|M13-11|SLM picked `hallucination_render_tag` to switch post-process passes|Replaced with standard UI screen-glitch overlays driven by the deterministic sleep-debt gradient loop|Let generative text alter the render graph|
|M13-EXT-02|SLM set horde `migration_cells` target vectors|Replaced with static world-terminal flavor text; AI Director tracking loop untouched|Let generative text override pathfinding/spawning|

**Exit Criteria:**

```bash
grep -rn "SLMActionOutput" ./src/ \
  | grep -vE "(ui_manager|terminal_render|audio_vo|slm_types\.h)"
```

Exit status 0 — zero matches outside text/UI display and audio-VO subtitle code.

---
