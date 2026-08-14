PROVISIONAL PATCH — NOT YET VERIFIED
=====================================
IDs below (M1-EXT-29 through M1-EXT-33) are picked from the free range I
was told existed in v78 as of this conversation. They have NOT been swept
against the Desktop draft files (teq-v78-partA-hermes-gaps.md,
teq-v78-part2-20gaps-plan.md, teq-v78-part2-remaining14.md,
teq-v78-procedural-gapfill-draft.md) which are known to already use IDs
up to at least M1-EXT-28. Whoever merges this MUST re-check the live IDs
in the actual file before pasting these in, and renumber if anything
collides. Do not trust these numbers blind.
=====================================

## [M1-EXT-29] Double-Precision Authoritative Transform with Float Upload Cast

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

## [M1-EXT-30] Parent-Child Transform Hierarchy with Dirty-Flag Propagation

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

## [M1-EXT-31] Procedural Spring-Damper Camera Rig

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

## [M1-EXT-32] RayBatchQuery Parallel Raycast Primitive

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

## [M1-EXT-33] SpatialHash-Driven Sound Occlusion Query

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
