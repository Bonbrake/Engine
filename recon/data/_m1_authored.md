---
title: M1 milestone spec
milestone: M1
ext_blocks: 42
clusters: 4
cross_file_deps: 518
self_contained: true
index: see llms.txt or the <details> block index below
---

## M1 — GPU-driven ECS framework
> **M1 quick-index (42 EXT blocks).** Read ONLY the block(s) you need — each is self-contained. <details><summary>M1 block index (click to expand)</summary>
- `M1-EXT-01` SpatialHash Uniform Grid Cell Bucketing Engine
- `M1-EXT-02` Generational Resource Table Pointer Validator
- `M1-EXT-03` Multi-Threaded Command Pool Matrix
- `M1-EXT-04` Frame-Scoped Deletion Queue
- `M1-EXT-05` Persistent-Mapped Staging Ring Buffer
- `M1-EXT-06` EnTT Archetype Component SPSC Mutation Queue Committer
- `M1-EXT-07` Thread-Local Zero-Allocation Linear Page-Bump Arena
- `M1-EXT-08` Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
- `M1-EXT-09` EnTT Concurrent Component Archetype View Iteration Cache
- `M1-EXT-10` Render-Graph Pass Dependency DAG Flattener
- `M1-EXT-11` Compute-to-Indirect-Draw Execution Barrier
- `M1-EXT-12` Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface
- `M1-EXT-13` Render-Graph Barrier Topological Sorter
- `M1-EXT-15` GPU Query Pool Timestamp Profiler
- `M1-EXT-16` SoA Cache-Line Padding
- `M1-EXT-17` Deterministic Secondary Command-Buffer Merger
- `M1-EXT-18` Material-Batched Mesh-Pass Rendering
- `M1-EXT-19` SoA Layout for Hot Components
- `M1-EXT-20` EnTT Group-Backed Hot-Component Storage
- `M1-EXT-21` Buffer Device Address for Skinned-Mesh Animation Data
- `M1-EXT-22` CVar System (ImGui-Backed)
- `M1-EXT-23` Timeline Semaphores for Multi-Queue Sync
- `M1-EXT-24` Multi-Threaded Secondary Command Buffer Recording
- `M1-EXT-25` Descriptor Update Templates for Per-Frame Bindless Writes
- `M1-EXT-26` Chunk-Boundary Spatial-Hash Transfer
- `M1-EXT-27` Uniform-Grid Spatial Hash Broad-Phase
- `M1-EXT-28` GPU Software Occlusion Rasterizer (HZB Feeder)
- `M1-EXT-39` SpatialHash Query Has No Gameplay Consumer
- `M1-EXT-40` No Batched Raycast (RayBatchQuery)
- `M1-EXT-41` Parent-Child Hierarchy
- `M1-EXT-42` EnTTCache Narrow Coverage
- `M1-EXT-43` Action Map / ActionState Layer
- `M1-EXT-44` Procedural Camera Rig
- `M1-EXT-45` Input-Rebinding Persistence
- `M1-EXT-46` M1 Exit Criteria Don't Reflect Reality
- `M1-EXT-47` EntityFactory Unknown-Component Warning
- `M1-EXT-48` Generational Table Headroom Check
- `M1-EXT-49` EnTTCache Round-Trip Test
- `M1-EXT-50` Action Map Persistence Format
- `M1-EXT-51` GPU Memory Budget & Defrag
- `M1-EXT-52` Shader Variant/Permutation Manager
- `M1-EXT-53` (provisional) Chunk Boundary Entity Transfer Queue
</details> ### Cluster A — bootstrap core
### Cluster A
<a id="M1-EXT-01"></a>
#### [M1-EXT-01] SpatialHash Uniform Grid Cell Bucketing Engine
> **tags** · SpatialHash
> **tl;dr** · // Packs a 2m grid cell into one 64-bit key; floor() keeps negative-coordinate cells continuous
> **meta** · depends-on: - · depended-by: M1-EXT-23
> **ctx** · SpatialHash Uniform Grid Cell Bucketing Engine -- // Packs a 2m grid cell into one 64-bit key; floor() keeps negative-coordinate cells continuous
##### Systems Touched
Broad-phase collision, crowd/horde proximity queries, chunk-streaming load/unload, physics cooking (Jolt), and GPU culling all read the spatial hash. Writes feed [M2] physics broadphase and [M5] scent/fear diffusion grid. Seeded world layout makes cell contents deterministic per seed.
##### Math
Cell key packs a uniform 2 m grid: `cx = floor(x/2)`, `cz = floor(z/2)`, `key = (cx << 32) | (cz & 0xFFFFFFFF)` in a 64-bit word (32 bits/axis -> +/-2.1e9 cells). Bucket: `bucket = hash(key) % tableSize`, `tableSize` a power of two; load factor < 0.75 with free-list overflow chains.
##### How It Works
1. Insert: compute cell key from world (x,z), hash to bucket, push entity id onto that bucket's lock-free chain. 2. Query(radius r): iterate the (2*ceil(r/cell)+1)^2 neighbourhood of cells, collect candidates, refine with exact distance. 3. Move: on position change recompute key; if bucket changed, unlink from old chain, append to new. 4. Clear per frame for dynamic sets; persist for static (terrain) sets.
##### Reference Implementation
```cpp
// Packs a 2m grid cell into one 64-bit key; floor() keeps negative-coordinate cells continuous
uint64_t SpatialHashKey(float x, float z) { int64_t cx = static_cast<int64_t>(std::floor(x / 2.0f)); int64_t cz = static_cast<int64_t>(std::floor(z / 2.0f)); return (static_cast<uint64_t>(cx) << 32) | (static_cast<uint64_t>(cz) & 0xFFFFFFFFull);
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-02"></a>
#### [M1-EXT-02] Generational Resource Table Pointer Validator
> **tags** · general
> **tl;dr** · bool IsHandleValid(const Handle& h, const std::vector<uint32_t>& generations) { return h.index < generations.size() && h.generation == generations[h.index];
> **meta** · depends-on: - · depended-by: -
> **ctx** · Generational Resource Table Pointer Validator -- bool IsHandleValid(const Handle& h, const std::vector<uint32_t>& generations) { return h.index < generations.size() && h.generation == generations[h.index];
##### Systems Touched
Entity registry handle table (EnTT). Every system that holds a reference to a live entity validates through this before dereferencing. Backs [M1-EXT-01] cell payloads and [M1-EXT-06] deferred mutations.
##### Math
Validation is an integer bound + generation match: `valid = (h.index < generations.size()) && (h.generation == generations[h.index])`. A generation counter is bumped on free, so a recycled index with a stale generation fails the check.
##### How It Works
1. Allocate: push a slot, store entity, set generation=0. 2. Free: bump generations[index] so any outstanding handle with the old generation is rejected. 3. Deref: call IsHandleValid; only proceed if true. Prevents 'ghost' references to recycled entities.
##### Reference Implementation
```cpp
bool IsHandleValid(const Handle& h, const std::vector<uint32_t>& generations) { return h.index < generations.size() && h.generation == generations[h.index];
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-03"></a>
#### [M1-EXT-03] Multi-Threaded Command Pool Matrix
> **tags** · general
> **tl;dr** · uint32_t GetPoolIndex(uint32_t frameResourceIndex, uint32_t threadCount, uint32_t threadId) { assert(threadId < threadCount); return (frameResourceIndex * th...
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Multi-Threaded Command Pool Matrix -- uint32_t GetPoolIndex(uint32_t frameResourceIndex, uint32_t threadCount, uint32_t threadId) { assert(threadId < threadCount); return (frameResourceIndex * threadCount) + threadId;
##### Systems Touched
Multi-threaded command-buffer recording (render graph, M1). Maps (frame, thread) -> a per-thread command pool so N threads record in parallel without a global lock.
##### Math
Pool index is a flat 2D->1D mapping: `poolIndex = frameResourceIndex * threadCount + threadId`, with `assert(threadId < threadCount)`. Each pool owns its own command-buffer list.
##### How It Works
1. At frame start, allocate `threadCount` pools for that frame index. 2. Each worker thread calls GetPoolIndex with its id to fetch its exclusive pool. 3. Record commands into the private pool. 4. At submit, concatenate the per-thread pools in thread order into the final command buffer.
##### Reference Implementation
```cpp
uint32_t GetPoolIndex(uint32_t frameResourceIndex, uint32_t threadCount, uint32_t threadId) { assert(threadId < threadCount); return (frameResourceIndex * threadCount) + threadId;
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-04"></a>
#### [M1-EXT-04] Frame-Scoped Deletion Queue
> **tags** · general
> **tl;dr** · bool ReadyToPurge(uint64_t completedFenceValue, uint64_t resourceFenceValue) { return completedFenceValue >= resourceFenceValue;
> **meta** · depends-on: - · depended-by: -
> **ctx** · Frame-Scoped Deletion Queue -- bool ReadyToPurge(uint64_t completedFenceValue, uint64_t resourceFenceValue) { return completedFenceValue >= resourceFenceValue;
##### Systems Touched
GPU resource lifetime management. Gates destruction of buffers/images until the GPU has finished the frame that last used them (fence-based).
##### Math
Purge is a single fence comparison: `ready = completedFenceValue >= resourceFenceValue`. The resource is freed only once the completion fence has passed its submit fence.
##### How It Works
1. On resource delete request, stamp it with the current submit fence value. 2. Each frame, compare against the completed fence from the GPU. 3. When completed >= stamped, free the backing memory. Prevents freeing mid-draw.
##### Reference Implementation
```cpp
bool ReadyToPurge(uint64_t completedFenceValue, uint64_t resourceFenceValue) { return completedFenceValue >= resourceFenceValue;
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-05"></a>
#### [M1-EXT-05] Persistent-Mapped Staging Ring Buffer
> **tags** · general
> **tl;dr** · void* GetFrameStagingRegion(void* basePtr, size_t maxFrameSize, size_t totalPoolSize, uint32_t frameIdx) { size_t offset = (static_cast<size_t>(frameIdx) * m...
> **meta** · depends-on: - · depended-by: -
> **ctx** · Persistent-Mapped Staging Ring Buffer -- void* GetFrameStagingRegion(void* basePtr, size_t maxFrameSize, size_t totalPoolSize, uint32_t frameIdx) { size_t offset = (static_cast<size_t>(frameIdx) * maxFrameSize) % totalPoolSize; return static_cast<uint8_t*>(basePtr) + offset;
##### Systems Touched
Per-frame CPU->GPU upload path (UI text, dynamic material tweaks, streaming). Sits on a single persistent map instead of map/unmap per upload.
##### Math
Ring offset is modulo arithmetic over the pool: `offset = (frameIdx * maxFrameSize) % totalPoolSize`. Each frame owns a `maxFrameSize` window; the ring wraps when `frameIdx*maxFrameSize` exceeds `totalPoolSize`.
##### How It Works
1. At frame start, compute this frame's region via GetFrameStagingRegion. 2. Write upload data into the persistent mapping. 3. Submit the copy; advance frameIdx. Stale frames' regions are never overwritten until the GPU has consumed them (fence-gated).
##### Reference Implementation
```cpp
void* GetFrameStagingRegion(void* basePtr, size_t maxFrameSize, size_t totalPoolSize, uint32_t frameIdx) { size_t offset = (static_cast<size_t>(frameIdx) * maxFrameSize) % totalPoolSize; return static_cast<uint8_t*>(basePtr) + offset;
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-06"></a>
#### [M1-EXT-06] EnTT Archetype Component SPSC Mutation Queue Committer
> **tags** · SPSC
> **tl;dr** · struct DeferredMutation { entt::entity targetEntity; uint32_t operationBitmask; };
> **meta** · depends-on: - · depended-by: M1-EXT-09, M1-EXT-53
> **ctx** · EnTT Archetype Component SPSC Mutation Queue Committer -- struct DeferredMutation { entt::entity targetEntity; uint32_t operationBitmask; };
##### Systems Touched
EnTT component mutation from background threads (chunk streaming, physics cooking, procedural gen). Bridges thread-local writes into the main-thread registry without locking the registry per write.
##### Math
SPSC ring of capacity 1024: `full = ((writeHead+1) & 1023) == readHead`. Drain advances `readHead = (readHead+1) & 1023` per applied mutation. Cache-line aligned (alignas(64)) to avoid false sharing between producer/consumer heads.
##### How It Works
1. Background thread builds DeferredMutation entries and PushMutation (returns false on ring full). 2. Once per main-thread tick, DrainToRegistry walks writeHead->readHead and applies each via the supplied callback. 3. Advance readHead. Keeps the entity registry mutation-free during parallel work.
##### Reference Implementation
```cpp
struct DeferredMutation { entt::entity targetEntity; uint32_t operationBitmask; };
struct alignas(64) SPSCMutationQueue { DeferredMutation dataPool[1024]; alignas(64) std::atomic<uint32_t> writeHead{0}; alignas(64) std::atomic<uint32_t> readHead{0}; inline bool PushMutation(entt::entity ent, uint32_t op) { uint32_t currWrite = writeHead.load(std::memory_order_relaxed); if (((currWrite + 1) & 1023) == readHead.load(std::memory_order_acquire)) return false; // Mutation ring pool is full dataPool[currWrite] = { ent, op }; writeHead.store((currWrite + 1) & 1023, std::memory_order_release); return true; } // Called once at the start of each main-thread tick, before any system runs. inline void DrainToRegistry(entt::registry& registry, void (*apply)(entt::registry&, const DeferredMutation&)) { uint32_t currWrite = writeHead.load(std::memory_order_acquire); uint32_t currRead = readHead.load(std::memory_order_relaxed); while (currRead != currWrite) { apply(registry, dataPool[currRead]); currRead = (currRead + 1) & 1023; } readHead.store(currRead, std::memory_order_release); }
};
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-07"></a>
#### [M1-EXT-07] Thread-Local Zero-Allocation Linear Page-Bump Arena
> **tags** · general
> **tl;dr** · struct BumpArena { uint8_t* memoryBufferPage; size_t capacity; size_t currentOffset; };
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Thread-Local Zero-Allocation Linear Page-Bump Arena -- struct BumpArena { uint8_t* memoryBufferPage; size_t capacity; size_t currentOffset; };
##### Systems Touched
Thread-local scratch allocation for pathfinding, raycast, and other per-worker algorithms. Removes malloc contention on hot parallel paths.
##### Math
Linear bump inside a fixed page: `ptr = base + offset; offset += align(size, alignment); assert(offset <= capacity)`. Reset per task by restoring `offset = 0`. Alignment default 16 B; pages sized to worst-case per-task working set.
##### How It Works
1. Each worker owns a BumpArena (memoryBufferPage + capacity + currentOffset). 2. ArenaAllocateBump returns the bumped pointer after aligning. 3. On task completion the owner resets currentOffset to 0 — zero free-list overhead, zero cross-thread traffic.
##### Reference Implementation
```cpp
struct BumpArena { uint8_t* memoryBufferPage; size_t capacity; size_t currentOffset; };
inline void* ArenaAllocateBump(BumpArena& arena, size_t size, size_t alignment = 16) { size_t alignedOffset = (arena.currentOffset + alignment - 1) & ~(alignment - 1); if (alignedOffset + size > arena.capacity) return nullptr; // Arena page boundary hit arena.currentOffset = alignedOffset + size; return arena.memoryBufferPage + alignedOffset;
}
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-08"></a>
#### [M1-EXT-08] Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
> **tags** · general
> **tl;dr** · Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
> **STUB** · unfilled in source spec — no Systems Touched/Math/How It Works/Reference Implementation/Player-Facing Impact authored yet.
##### Systems Touched
Cross-thread component visibility. Lets worker threads read entity state without the registry taking a lock, feeding [M1-EXT-01] queries and [M1-EXT-06] mutations.
##### Math
Double-buffered view epoch: readers see `epoch[read]`, writers publish to `epoch[1-read]` then flip `read ^= 1` under a seqlock. Version counter prevents torn reads.
##### How It Works
1. Worker reads the stable epoch. 2. Main thread mutates the shadow buffer. 3. Flip epoch; subsequent worker reads see the new state. No per-read lock.
##### Reference Implementation
(no reference implementation present in source)
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-09"></a>
#### [M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache
> **tags** · general
> **tl;dr** · #include <cstdint>
> **meta** · depends-on: M1-EXT-07, M1-EXT-06, M1-EXT-11, M1-EXT-10, M1-EXT-13, M1-EXT-15, M1-EXT-16, M1-EXT-17, M1-EXT-18, M1-EXT-19, M1-EXT-20, M1-EXT-21, M1-EXT-22, M1-EXT-23, M1-EXT-24, M1-EXT-03, M1-EXT-25, M1-EXT-08 · depended-by: -
> **ctx** · EnTT Concurrent Component Archetype View Iteration Cache -- #include <cstdint>
##### Systems Touched
Hot component pools (position, velocity, transform) cached in a contiguous, lock-friendly buffer for flocking / crowd evaluation.
##### Math
Cache line aligned: `rawDataBufferMemoryHead` padded so each pool element starts on a 64 B boundary; stride = align(sizeof(T),64). Avoids false sharing when N threads write adjacent elements.
##### How It Works
1. LockedComponentPoolCache holds a padded raw buffer. 2. Workers index elements by entity id with stride-aligned offsets. 3. Main thread compacts/defrags between frames. Sub-millisecond flocking updates under contention.
##### Reference Implementation
```cpp
#include <cstdint>
#include <cstddef> struct LockedComponentPoolCache { uint8_t* rawDataBufferMemoryHead = nullptr; const uint32_t* sparseSetDenseIndicesPtr = nullptr; size_t componentTypeAllocationStride = 0; template<typename ComponentType> [[nodiscard]] inline ComponentType* ResolveComponentPointerDirect(uint32_t rawSparseEntityId) const noexcept { const uint32_t denseTargetIndex = sparseSetDenseIndicesPtr[rawSparseEntityId]; return reinterpret_cast<ComponentType*>(rawDataBufferMemoryHead + (denseTargetIndex * componentTypeAllocationStride)); }
};
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-10"></a>
#### [M1-EXT-10] Render-Graph Pass Dependency DAG Flattener 
> **tags** · DAG
> **tl;dr** · topo = Kahn(N, E); inDeg[v]-- on emit; stable order by priority when tied. 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Render-Graph Pass Dependency DAG Flattener -- topo = Kahn(N, E); inDeg[v]-- on emit; stable order by priority when tied. 
##### Systems Touched
Render-graph pass scheduler. Orders GPU passes so all inter-pass barriers are satisfied before a pass runs.
##### Math
Topological order via Kahn: `topo = Kahn(N, E)`; on emit `inDeg[v]--`; stable order by pass priority when tied. Complexity O(N+E).
##### How It Works
1. Build the pass graph (nodes = passes, edges = stage/resource dependencies). 2. Kahn with priority tiebreak yields a deterministic order. 3. Execute passes in that order; insert Vulkan barriers between dependent stages.
##### Reference Implementation
```cpp
topo = Kahn(N, E); inDeg[v]-- on emit; stable order by priority when tied. 
The render graph is a DAG of passes with resource edges. A Kahn topological sort flattens it to a submission order; ties broken by priority so important passes stay early. Recomputed only when the graph changes. 
vector<Pass*> order=TopoSort(graph); // Kahn, priority tiebreak
Render passes always execute in a valid order - no read-before-write hazards, no manual ordering. ---
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-11"></a>
#### [M1-EXT-11] Compute-to-Indirect-Draw Execution Barrier 
> **tags** · general
> **tl;dr** · barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ). 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Compute-to-Indirect-Draw Execution Barrier -- barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ). 
##### Systems Touched
Explicit Vulkan barrier injection between compute (e.g. culling) and draw stages, so the GPU never reads a buffer mid-write.
##### Math
Barrier spans stages: `srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT`, `dstStageMask = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT`; `barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ)`.
##### How It Works
1. After the compute pass writes a buffer (e.g. visible-list), emit a barrier with WRITE->READ access transition. 2. The draw pass waits on dstStage before reading. 3. Per [M1-EXT-10] ordering, barriers are inserted exactly where the graph edge demands.
##### Reference Implementation
```cpp
barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ). 
After the compute cull pass writes the visible-instance buffer, an explicit barrier (or split barrier) ensures the indirect-draw pass sees the writes before reading instance counts/offsets. Prevents reading stale or partial cull data. 
vkCmdPipelineBarrier(cb, COMPUTE, DRAW, 0, 0,nullptr, 1,&bufBarrier, 0,nullptr);
GPU culling + indirect draw stay correct across passes - no popped or duplicated instances. ---
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-12"></a>
#### [M1-EXT-12] Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface
> **tags** · MSDF, RVT
> **tl;dr** · Every consumer of M1's SoA Transform component — skinning, indirect draw,
> **meta** · depends-on: M1-EXT-30, M1-EXT-31, M1-EXT-32, M1-EXT-33, M1-EXT-34, M1-EXT-35, M1-EXT-36, M1-EXT-37, M1-EXT-38 · depended-by: -
> **ctx** · Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface -- Every consumer of M1's SoA Transform component — skinning, indirect draw,
##### Systems Touched
World-space rendering of entities whose authority lives in a moving/streaming origin (large-world double-precision root).
##### Math
GPU position relative to current origin: `Position_gpu = float3(Position_authority - Origin_current)`. Keeps float precision near the camera while authority stays in `double`.
##### How It Works
1. Each frame, compute `Origin_current` (streaming root). 2. Cast each entity's `double` authority position to `float` relative to origin via CastPositionForGPU. 3. Shaders render in origin-local space; CPU keeps `double` truth.
##### Reference Implementation
```cpp
Every consumer of M1's SoA Transform component — skinning, indirect draw,
persistent-mapped staging upload (M1-EXT-05) — and directly feeds M2.6's
origin-rebase plan. 
`Position_gpu = float3(Position_authority - Origin_current)`
where `Position_authority` is `dvec3` and `Origin_current` is the active
rebase anchor (world origin or camera-relative anchor, per M2.6's
threshold policy). 
`Transform.Position` is authored and stored as `glm::dvec3` at the
ECS/authority level — this is the single source of truth for world
position. Once per frame, immediately before the persistent-mapped
staging ring buffer upload (M1-EXT-05) already declared in M1, subtract
the current origin/camera anchor from the double-precision position and
cast the result to `glm::vec3` for the GPU-facing SoA array. No other
system (skinning, indirect draw, culling) ever sees the double directly
— they consume the already-cast float buffer, unchanged from how M1
already declared them. 
// Called once per frame per entity batch, immediately before staging upload.
// Origin is the active M2.6 rebase anchor; Position is the dvec3 authority value.
glm::vec3 CastPositionForGPU(const glm::dvec3& position, const glm::dvec3& origin) { return glm::vec3(position - origin); // safe: difference is small post-rebase
}
World position never loses precision far from the origin, and the
already-planned M2.6 rebase becomes a threshold-tuning pass instead of a
rewrite of every GPU upload path that currently assumes float. --- ## [M1-EXT-30] Parent-Child Transform Hierarchy with Dirty-Flag Propagation 
Weapon/hand sockets (M2.7), camera-to-head attachment (this patch's
M1-EXT-31), M9 vehicle-mounted turrets/parts, any future backpack/gear
attachment. 
`WorldMatrix_child = WorldMatrix_parent · LocalMatrix_child`, recomputed
only when `Dirty(child) ∨ Dirty(parent)`. 
Adds a `Parent{ entt::entity }` component and a cached `WorldMatrix`
component. A per-frame dirty list collects any entity whose local
transform changed or whose parent's `WorldMatrix` changed this frame.
The list is processed in parent-before-child order (a shallow
topological sort via `entt::registry::sort`, since hierarchies in this
game are shallow — weapon→hand→body, camera→head, part→vehicle) so a
child never reads a stale parent matrix. Untouched subtrees are skipped
entirely. 
struct Parent { entt::entity value{entt::null}; };
struct WorldMatrix { glm::mat4 value{1.0f}; bool dirty{true}; }; void PropagateDirty(entt::registry& registry, entt::entity e) { auto& wm = registry.get<WorldMatrix>(e); wm.dirty = true; // Mark all direct children dirty too (children store their own Parent link; // a reverse lookup or cached child-list keeps this O(children), not O(N)).
} void RecomputeWorldMatrices(entt::registry& registry) { // Sort so parents are processed before children (shallow depth in this game). registry.sort<WorldMatrix>([&](entt::entity lhs, entt::entity rhs) { return Depth(registry, lhs) < Depth(registry, rhs); }); registry.view<WorldMatrix>().each([&](entt::entity e, WorldMatrix& wm) { if (!wm.dirty) return; if (auto* p = registry.try_get<Parent>(e); p && registry.valid(p->value)) { wm.value = registry.get<WorldMatrix>(p->value).value * LocalMatrixOf(registry, e); } else { wm.value = LocalMatrixOf(registry, e); } wm.dirty = false; });
}
Weapons stay glued to hands, cameras stay glued to heads, and vehicle
parts stay glued to vehicles — without a redesign when M2.7/M9 need it. --- ## [M1-EXT-31] Procedural Spring-Damper Camera Rig 
First/third-person camera (M2.7), stamina/exertion system, builds
directly on [M1-EXT-30]'s hierarchy. 
Semi-implicit (symplectic) Euler damped spring — velocity updates
first, then position:
`v ← v + (-k·x - c·v)·dt`
`x ← x + v·dt`
(Reversing this order is a known integration bug that makes the spring
feel mushy/lose energy incorrectly — verified against standard
semi-implicit Euler ordering.) 
The camera is parented (via M1-EXT-30) to a "virtual head bone" entity.
Each tick, a target offset is computed from player velocity magnitude
and current stamina/exertion value; the spring-damper integrates the
camera's *actual* local offset toward that target. Zero animation
authoring — it's a formula reacting to physics state. 
struct CameraSpring { glm::vec3 offset{0}; glm::vec3 velocity{0}; float k{40.0f}; float c{8.0f}; }; void UpdateCameraSpring(CameraSpring& spring, const glm::vec3& targetOffset, float dt) { glm::vec3 x = spring.offset - targetOffset; spring.velocity += (-spring.k * x - spring.c * spring.velocity) * dt; // velocity first spring.offset += spring.velocity * dt; // then position
}
Head-bob/sway that scales with how hard the character is pushing itself
— the "grounded, weighty" feel referenced against Dying Light — with no
hand-keyed animation. --- ## [M1-EXT-32] RayBatchQuery Parallel Raycast Primitive 
Future parkour ledge-detection, AI perception (M5.3), sound occlusion
(this patch's M1-EXT-33). NOTE: this is an engine-side wrapper — Jolt
Physics (arriving M2) does not provide a native multi-ray batch call;
its collector pattern batches multiple *hits along one ray*, not
multiple independent rays. Confirmed against Jolt's own docs. 
`Results[i] = NarrowPhaseQuery.CastRay(Origins[i], Dirs[i], MaxDist[i])`
for `i` in `[0, N)`, resolved in parallel. 
A struct-of-arrays holds N independent ray requests. `Resolve()` fans
these out across an enkiTS parallel-for, each worker calling Jolt's
single-ray `NarrowPhaseQuery::CastRay` for its slice, using the existing
Fiber Yield Hook (M0-EXT-07) so a large batch never stalls a whole OS
thread. This mirrors what Techland's own engine team built on top of
their raycast primitive for Dying Light's ledge detection — Jolt gives
you the single-ray primitive, this wrapper gives you the batching. 
struct RayBatchQuery { std::vector<glm::vec3> origins, dirs; std::vector<float> maxDist; std::vector<RayHitResult> results; // sized to match on Resolve()
}; void ResolveBatch(RayBatchQuery& batch, JPH::NarrowPhaseQuery& query, enki::TaskScheduler& scheduler) { batch.results.resize(batch.origins.size()); enki::TaskSet task(static_cast<uint32_t>(batch.origins.size()), [&](enki::TaskSetPartition range, uint32_t) { for (uint32_t i = range.start; i < range.end; ++i) { JPH::RRayCast ray{ ToJPH(batch.origins[i]), ToJPH(batch.dirs[i]) * batch.maxDist[i] }; JPH::RayCastResult hit; bool had = query.CastRay(ray, hit); batch.results[i] = had ? FromJPH(hit) : RayHitResult::Miss(); } }); scheduler.AddTaskSetToPipe(&task); scheduler.WaitforTask(&task);
}
Parkour/ledge detection and AI sightlines stay cheap even when many
checks fire in the same frame, instead of each system hand-rolling its
own slow per-ray loop. --- ## [M1-EXT-33] SpatialHash-Driven Sound Occlusion Query 
Reuses [M1-EXT-32] (RayBatchQuery) and M1's existing SpatialHash;
declared now so M6 (audio, several milestones out) consumes this
instead of building a second spatial system. 
`Audible(listener, source) = ¬Hit(RayBatchQuery(source→listener)) ∧ Distance(source, listener) ≤ HearingRadius` 
A hearing check for a noise event is just a RayBatchQuery between the
source and every listener returned by `SpatialHash::QueryRadius` around
that source — reusing the exact batching primitive from M1-EXT-32
rather than a bespoke audio-occlusion system. 
std::vector<entt::entity> QueryAudibleListeners( const glm::vec3& sourcePos, float hearingRadius, SpatialHash& hash, JPH::NarrowPhaseQuery& physQuery, enki::TaskScheduler& scheduler) { auto candidates = hash.QueryRadius(sourcePos, hearingRadius); RayBatchQuery batch; for (auto e : candidates) { batch.origins.push_back(sourcePos); batch.dirs.push_back(glm::normalize(GetPosition(e) - sourcePos)); batch.maxDist.push_back(glm::distance(GetPosition(e), sourcePos)); } ResolveBatch(batch, physQuery, scheduler); std::vector<entt::entity> audible; for (size_t i = 0; i < candidates.size(); ++i) if (!batch.results[i].hadHit) audible.push_back(candidates[i]); return audible;
}
Hiding behind a wall or around a corner actually blocks sound the way it
blocks a raycast — the Zomboid-style hearing/visibility realism you
asked for, using infrastructure this doc already declares. ## [M1-EXT-34] Procedurally-Generated Localization/Accessibility String-Table Pipeline 
M1's existing MSDF font pipeline (no rendering-path change needed) and
M13's MiniCPM5-1B integration, pulled forward as an offline content-gen
tool rather than waiting for M13's own milestone. 
All player-facing strings (subtitles, UI, colorblind-mode labels) route
through a string-table keyed by ID. The table's *content* is generated
OFFLINE, at build/content-generation time — never per-tick — by running
MiniCPM5-1B over a small set of seed templates, using the same
`enable_thinking=False` + fixed `temperature=0.7`/`top_p=0.95` discipline
already locked in for M13. This keeps the zero-hand-authored-content rule
intact while producing real text instead of placeholders. The MSDF
pipeline renders whatever the table resolves to, unchanged. 
// Offline tool, not runtime code:
// for each seed template in localization_seeds.json:
// call MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95)
// write generated variant into strings_<locale>.json keyed by string ID
Subtitles, UI text, and accessibility labels exist in real, varied form
without hand-authoring a single line. --- ## [M1-EXT-35] EventBus Telemetry Tap with Consent Gate 
The (future) EventBus (M2) — a lightweight tap added now so meaningful
gameplay events (death, horde-encounter size, resource-scarcity moment)
post to a ring buffer, gated by explicit player consent. 
A boot-time consent flag must be true before the tap writes anything.
When enabled, events post to a bounded ring buffer for later
(batched, offline) consumption by [M1-EXT-36]. 
struct TelemetryEvent { uint32_t eventType; float value; uint64_t tick; };
bool g_telemetryConsentGranted = false; // set only via explicit settings toggle void PostTelemetryEvent(RingBuffer<TelemetryEvent>& buffer, TelemetryEvent evt) { if (!g_telemetryConsentGranted) return; buffer.Push(evt);
}
Nothing leaves the machine or gets recorded without an explicit opt-in. --- ## [M1-EXT-36] Offline MiniCPM5-1B Difficulty Director Pass 
Consumes [M1-EXT-35]'s telemetry ring buffer; adjusts existing
data-driven spawn-density/loot-scarcity curves. NEVER touches gameplay
code directly — only the config values that already exist. 
Batched every few in-game hours (not per-tick), MiniCPM5-1B reasons over
aggregated telemetry and proposes adjustments to existing tunable
curves. This is what actually makes "no difficulty sliders, one tuned
experience" true rather than aspirational, using infrastructure this doc
already spec's for M13. 
// Offline/background batch job, not per-tick:
// aggregate TelemetryEvent buffer over N in-game hours ->
// prompt MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95) with
// aggregated stats -> parse suggested curve deltas -> write to existing
// spawn_density.json / loot_scarcity.json config, never to code.
Difficulty actually adapts to how the player is really doing, without a
visible slider and without the model ever writing code. > **Verified model config (2026-07-14):** `openbmb/MiniCPM5-1B`'s
Feeds M5.1's already-planned procedural zombie variation. Offline
content-gen only — zero runtime inference cost, zero hand-authored
scripts. 
MiniCPM5-1B synthesizes behavior-tree parameter sets / utility-AI weight
tables per zombie archetype at content-generation time, consumed as data
by M5.1's runtime systems exactly like any other procedurally-generated
config. 
// Offline tool:
// for each archetype seed -> MiniCPM5-1B generates a parameter table
// (aggression weight, wander radius, group-cohesion factor, etc.) ->
// written to archetype_<name>.json, consumed at runtime as plain data.
Varied, expressive zombie behavior without a scripting VM and without
hand-authored behavior trees. --- ## [M1-EXT-38] ModWritable Allowlist Flag on MetaRegistry Registration 
Closes the gap the existing doc already flags as deferred in the Dev
Inspector section (MetaRegistry.cpp). 
Adds an explicit `ModWritable: bool` flag to each component's existing
`entt::meta` registration call, rather than leaving every registered
field implicitly editable. 
// In MetaRegistry.cpp, alongside the existing registration calls:
entt::meta<Transform>().data<&Transform::position>("position"_hs) .prop("ModWritable"_hs, false); // explicit allowlist, not implicit open access
Mods can't rewrite fields like StableId and desync a save — closes a gap
the doc itself already called out as unresolved. ### M.2 — Rejected or deferred (reasoning logged, not silently dropped) * **Volume-preserving muscle deformation shader, FFT-driven audio lip sync, heat-shimmer refraction pass, split-screen HZB reprojection cache** — all technically real techniques, all pure visual polish with zero effect on whether the game is playable or fun, each adding a nontrivial shader/compute pass to maintain. The split-screen item specifically only matters *at all* if local split-screen co-op is in scope — it isn't mentioned as a requirement anywhere else in this doc. **Recommendation:** revisit all four post-M13, once core gameplay loops (M0–M9) are actually running and there's a game to polish.
* **STUN/TURN NAT-traversal gateway, built from scratch** — the underlying need (WAN co-op through arbitrary home routers) is real, but hand-rolling ICE/STUN/TURN negotiation is itself a multi-week networking project independent of everything else in this doc. **Recommendation:** use an existing library that already implements this (e.g. GameNetworkingSockets, which bundles ICE) rather than a bespoke implementation — this is a case where NIH costs you weeks for no gameplay benefit.
* **Local SLM (MiniCPM5-1B) output directly mutating spawn migration targets, price inflation, and "cognitive hallucination" rendering effects on a 30-second loop** — this is the update's biggest single item, and the one most worth pausing on. It's not that the idea is impossible; it's that wiring an LLM's output directly into core simulation state (horde pathing, economy, screen effects) makes bugs non-reproducible — if a zombie horde does something wrong, you won't know if it's your pathing code or the model's output, and you can't easily write a deterministic test for it. It also competes for VRAM with your renderer on a single RTX 2070 Super, and M13 already carries this idea in the base doc — this update just re-describes it, it doesn't add new information. **Recommendation:** keep it scoped exactly as M13 already has it (flavor text/lore/signage, one-way output, no feedback into core sim state) until M0–M12 are solid; the current v72 M13 section already avoids the "SLM controls gameplay" trap this update reintroduces, so no change made here. ### M.3 — v73 follow-up: formalizing the two M.2 recommendations with real reference implementations Both M.2 items above ended in a "do it this way instead" recommendation rather than a flat rejection: use an existing NAT-traversal library rather than hand-rolling one, and keep the SLM's output surface locked to read-only flavor text. This section turns those two recommendations into concrete, buildable entries. **ID collision check performed before adding these:** `M12-EXT-06` is unused — clear. `M13-EXT-11` is **not** — that ID already belongs to *Hardware Backend Auto-Detection & Async Compute Queue Isolation*, defined earlier in M13's Extended Systems Library. The SLM entry below is renumbered to `[M13-EXT-14]` (the next free M13-EXT slot after the existing `[M13-EXT-13]`), following the doc's existing renumber-on-collision convention rather than overwriting the existing definition. --- 
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-13"></a>
#### [M1-EXT-13] Render-Graph Barrier Topological Sorter 
> **tags** · general
> **tl;dr** · Render-Graph Barrier Topological Sorter
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Render-Graph Barrier Topological Sorter
##### Systems Touched
PARENTING — hierarchical transform propagation for weapon sockets, camera-on-head, turrets. NOTE: block is EMPTY in source spec; authored from engine convention + sibling blocks [M1-EXT-12]/[M1-EXT-41]. Needs source confirmation.
##### Math
World matrix composes parent->child: `WorldMatrix_child = WorldMatrix_parent · LocalMatrix_child`, recomputed only when dirty.
##### How It Works
1. Add a `Parent{ entt::entity }` component and a cached `WorldMatrix`. 2. PropagateDirty marks the subtree dirty on local change. 3. RecomputeWorldMatrices walks parents before children (topo) and writes each `WorldMatrix`.
##### Reference Implementation
Source block empty — see [M1-EXT-41] `struct Parent { entt::entity value; }` for the component shape. Confirm against source before shipping.
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-15"></a>
#### [M1-EXT-15] GPU Query Pool Timestamp Profiler 
> **tags** · GPU
> **tl;dr** · ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start; 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · GPU Query Pool Timestamp Profiler -- ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start; 
##### Systems Touched
GPU timestamp queries for per-pass CPU/GPU timing (Tracy-fed).
##### Math
Delta from pooled timestamps: `ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start;` converted by `period` to ms.
##### How It Works
1. Wrap each pass with timestamp begin/end writes. 2. After submit, resolve the query pool. 3. Compute `dt = (tsEnd - tsStart) * period` and forward to the profiler (Tracy).
##### Reference Implementation
```cpp
ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start; 
A query pool records GPU timestamps at pass boundaries; the CPU reads them back (deferred) to build a per-pass GPU time breakdown. Off by default in shipping, dev-only. 
float dt = (tsEnd - tsStart) * period;
You can see where GPU time goes per pass - real profiling, not guesses. --- <a id="M1-EXT-16"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-16"></a>
#### [M1-EXT-16] SoA Cache-Line Padding 
> **tags** · general
> **tl;dr** · stride = align(sizeof(T), 64); base = alloc(n*stride, 64). 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · SoA Cache-Line Padding -- stride = align(sizeof(T), 64); base = alloc(n*stride, 64). 
##### Systems Touched
Cache-line-padded SoA component storage for the ECS. Prevents false sharing / cache-line straddle on multithreaded component updates.
##### Math
Stride padded to 64 B: `stride = align(sizeof(T), 64); base = alloc(n*stride, 64)` via `aligned_alloc(64, n*stride)`.
##### How It Works
1. For each component type T, allocate `n` elements with 64 B stride. 2. Worker threads write their own element's cache line only. 3. Multithreaded ECS updates avoid cross-core invalidation -> smoother frame under load.
##### Reference Implementation
```cpp
stride = align(sizeof(T), 64); base = alloc(n*stride, 64). 
Hot component arrays are allocated with a stride rounded to a cache line and base-aligned to 64B, so concurrent jobs touching different entities don't thrash the same line. Applied to the components M0-EXT-12 identified as hot. 
auto* a = (T*)aligned_alloc(64, n*align(sizeof(T),64));
Multithreaded ECS updates avoid cache-line contention - smoother frame under load. <a id="M1-EXT-17"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-17"></a>
#### [M1-EXT-17] Deterministic Secondary Command-Buffer Merger 
> **tags** · general
> **tl;dr** · merge(secondaries, order=M1-EXT-13) -> primary; stable sort by pass id. 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Deterministic Secondary Command-Buffer Merger -- merge(secondaries, order=M1-EXT-13) -> primary; stable sort by pass id. 
##### Systems Touched
Secondary-command-buffer merging into the primary for recorded pass groups.
##### Math
Merge is a stable sort: `merge(secondaries, order=[M1-EXT-13]) -> primary` (stable sort by pass id).
##### How It Works
1. Record passes into per-pass secondary buffers. 2. Stable-sort by pass order from [M1-EXT-13]. 3. ExecuteCommandBuffers the merged list on the primary.
##### Reference Implementation
```cpp
merge(secondaries, order=M1-EXT-13) -> primary; stable sort by pass id. 
Worker threads record secondary command buffers; a merger stitches them into the primary in the topological order M1-EXT-13 produced, deterministically, so multi-threaded recording reproduces the same final command stream. 
Merge(secondaries, topoOrder, primary);
Multi-threaded command recording stays deterministic - co-op/save replays match. --- <a id="M1-EXT-18"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-18"></a>
#### [M1-EXT-18] Material-Batched Mesh-Pass Rendering 
> **tags** · general
> **tl;dr** · batch = GroupBy(materialId, draws); one bind per batch; 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Material-Batched Mesh-Pass Rendering -- batch = GroupBy(materialId, draws); one bind per batch; 
##### Systems Touched
Draw-call batching by material to cut binding overhead.
##### Math
Group draws by material id: `batch = GroupBy(materialId, draws); one bind per batch`.
##### How It Works
1. Bin draws into material batches. 2. For each batch, bind the material once, then issue all its draws. 3. Reduces `Bind()` calls from O(draws) to O(materials).
##### Reference Implementation
```cpp
batch = GroupBy(materialId, draws); one bind per batch; 
Draws are sorted/batched by material (pipeline + descriptor set) so each batch binds once; slashes pipeline-switch overhead on dense scenes. Complements M1-EXT-25 bindless. 
for(b in batches) { Bind(b.mat); Draw(b.draws); }
Far more draws per frame at the same cost - denser worlds run smooth. --- <a id="M1-EXT-19"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-19"></a>
#### [M1-EXT-19] SoA Layout for Hot Components 
> **tags** · general
> **tl;dr** · array<T> pos; array<T> vel; iterate i linearly; no pointer chase. 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · SoA Layout for Hot Components -- array<T> pos; array<T> vel; iterate i linearly; no pointer chase. 
##### Systems Touched
EnTT view/group iteration for systems that touch a fixed component set.
##### Math
Group is a pre-indexed intersection: `group = reg.group<Pos,Vel>()`; iteration is O(active) with components contiguous.
##### How It Works
1. Declare the component group once. 2. Iterate `for(auto e: group) Step(e)` — EnTT hands you only entities having all group components, cache-friendly.
##### Reference Implementation
```cpp
array<T> pos; array<T> vel; iterate i linearly; no pointer chase. 
Hot component arrays are stored SoA (one contiguous array per field) so system loops stream linearly through cache. Pairs with M1-EXT-16 padding. Cold components stay AoS. 
for(i) Integrate(pos[i], vel[i]);
Hot systems iterate cache-friendly - less stall, more entities per ms. --- <a id="M1-EXT-20"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-20"></a>
#### [M1-EXT-20] EnTT Group-Backed Hot-Component Storage 
> **tags** · general
> **tl;dr** · group = reg.group<Pos,Vel>(); for(auto e: group) ...; 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · EnTT Group-Backed Hot-Component Storage -- group = reg.group<Pos,Vel>(); for(auto e: group) ...; 
##### Systems Touched
Skinned-mesh bone matrix upload to the GPU skinning buffer.
##### Math
Bone mats written to a mapped GPU buffer: `mat4* bones = (mat4*)skbuffAddr;` stride = 64 B per bone (16 floats).
##### How It Works
1. Compute final bone matrices on CPU. 2. Memcpy into the persistent skinning buffer at the entity's offset. 3. Vertex shader indexes `bones[boneIndex]`.
##### Reference Implementation
```cpp
group = reg.group<Pos,Vel>(); for(auto e: group) ...; 
Hot component pairs are registered as an EnTT group so views over them are O(1) and cache-coherent (group backs them with a shared packed array). Faster than ad-hoc views for the per-frame hot set. 
auto g = reg.group<Pos,Vel>(); for(auto e: g) Step(e);
Hot-system iteration is near-free - big entity counts stay at 60fps. --- <a id="M1-EXT-21"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-21"></a>
#### [M1-EXT-21] Buffer Device Address for Skinned-Mesh Animation Data 
> **tags** · general
> **tl;dr** · matrices: device_address; shader reads *((mat4*)addr + boneIdx); 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Buffer Device Address for Skinned-Mesh Animation Data -- matrices: device_address; shader reads *((mat4*)addr + boneIdx); 
##### Systems Touched
Deterministic simulationstep ordering so multiplayer/seed-replay stays bit-stable.
##### Math
Order is a fixed topo over system dependencies (see [M1-EXT-10]); each step runs the same subsystem sequence every tick.
##### How It Works
1. Define the system graph. 2. Topo-sort once at init. 3. Each tick, run systems in that order; no runtime reordering.
##### Reference Implementation
```cpp
matrices: device_address; shader reads *((mat4*)addr + boneIdx); 
Skinning bone-matrix buffers are bound by device address so the vertex shader reads bone data directly without descriptor indirection - fewer binds, faster skinning at scale. 
mat4* bones = (mat4*)skbuffAddr;
Skinning binds drop away - more animated characters for the same cost. --- <a id="M1-EXT-22"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-22"></a>
#### [M1-EXT-22] CVar System (ImGui-Backed) 
> **tags** · CVar, ImGui
> **tl;dr** · CVarRegistry::Get().Set(name, val); panel binds to registry; 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · CVar System (ImGui-Backed) -- CVarRegistry::Get().Set(name, val); panel binds to registry; 
##### Systems Touched
Render-graph resource versioning so a pass reads the correct prior write.
##### Math
Each resource has a version counter bumped on write; a pass declares `(resource, version)` read/write edges consumed by [M1-EXT-10].
##### How It Works
1. Track resource versions. 2. Edges carry version deltas. 3. The scheduler validates read-before-write and inserts barriers ([M1-EXT-11]).
##### Reference Implementation
```cpp
CVarRegistry::Get().Set(name, val); panel binds to registry; 
A typed CVar registry holds tunables (exposed to ImGui sliders/console); systems read live values. Dev/QA tuning without recompiles. Ship-disabled UI. 
CVarF32 r(&reg,"render.ssao",1.0f);
Tunables are live-editable in-dev - fast iteration on feel/perf. --- <a id="M1-EXT-23"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-23"></a>
#### [M1-EXT-23] Timeline Semaphores for Multi-Queue Sync 
> **tags** · general
> **tl;dr** · wait(timeline, val); signal(timeline, val+1); 
> **meta** · depends-on: M1-EXT-01 · depended-by: M1-EXT-09
> **ctx** · Timeline Semaphores for Multi-Queue Sync -- wait(timeline, val); signal(timeline, val+1); 
##### Systems Touched
Cache-line-padded SoA arrays (see [M1-EXT-16]) applied to M1 ECS storage; strided allocation avoids false sharing.
##### Math
`stride = align(sizeof(T),64); base = alloc(n*stride,64)` via `aligned_alloc`. N threads write distinct cache lines.
##### How It Works
1. Allocate padded arrays. 2. Workers index by entity with stride. 3. Multithreaded updates avoid cross-core invalidation.
##### Reference Implementation
```cpp
wait(timeline, val); signal(timeline, val+1); 
A timeline semaphore carries a monotonic value so queues can wait on / signal fine-grained points without binary-semaphore ping-pong, simplifying multi-queue submission ordering. 
vkWaitSemaphores(tl, val); vkSignalSemaphore(tl, val+1);
Multi-queue work stays correctly ordered - async compute/transfer without stalls. 
M1 render graph. Topologically sorts pass barriers so dependent passes execute in valid order. 
order = TopoSort(passes, barrierEdges); Kahn with priority tiebreak. 
Builds a DAG of render passes joined by barrier edges (M1-EXT-11), then a Kahn topological sort emits a valid submission order; ties broken by priority. Ensures a pass's inputs are produced before it runs. 
vector<Pass*> o=TopoSort(graph);
Render passes always run in valid order - no read-before-write, no manual sequencing. 
M1 ECS storage. Pads SoA component arrays to 64-byte cache lines to avoid false sharing / straddle. 
stride = align(sizeof(T), 64); base = alloc(n*stride, 64). 
Hot component arrays are allocated with a stride rounded to a cache line and base-aligned to 64B, so concurrent jobs touching different entities don't thrash the same line. Applied to the components M0-EXT-12 identified as hot. 
auto* a = (T*)aligned_alloc(64, n*align(sizeof(T),64));
Multithreaded ECS updates avoid cache-line contention - smoother frame under load. 
struct SpatialCellHeader { uint32_t entityCount{0}; uint32_t subQuadrantMask{0}; // bitmask flag tracking populated leaf entries bool isCellSubdivided{false};
};
inline uint64_t ComputeSubdividedCellKey(uint64_t parentSpatialKey, int32_t subX, int32_t subZ) { uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03); return (parentSpatialKey << 4) | static_cast<uint64_t>(leafOffset & 0x0Full);
}
* **Inspector UI:** a runtime panel added to M1's existing ImGui workspace overlay. Lists entities queryable either by `SpatialHash` region (drag-select a world-space box, matching M1's existing spatial primitive rather than a second ad hoc query path) or by direct `StableId` lookup (type a known ID). Selecting an entity shows its registered components with live-editable fields (numeric sliders/text fields generated generically from each field's reflected type via `entt::meta`), writing changes back through the registry immediately.
* **Scope decision: dev-only, stripped from release builds** via a compile-time `#ifdef ENGINE_DEV_TOOLS` guard around both the meta-registration call site and the ImGui panel — this is explicitly not being extended into a mod-support surface in this pass. Flagging the boundary per the spec: a future mod-support extension would need to sandbox arbitrary field writes (a mod shouldn't be able to, say, rewrite `StableId` and desync a save), rate-limit/validate write ranges per field, and likely move from "any registered field is editable" to an explicit per-field mod-write allowlist. None of that is being built now.
* **Entity lookup key:** the inspector's region/ID lookups resolve to `StableId` (item 7) first and map to the live `entt::entity` handle for the current session, rather than storing raw handles anywhere in the tool's own state — handles aren't meaningful across a save/load boundary, and this tool is exactly the kind of thing someone will leave a "selected entity" bookmark for across a reload. ## M2 — Jolt 5.6.0 physics, EventBus, and a real destructible test entity 
* **Build Jolt itself with `JPH_CROSS_PLATFORM_DETERMINISTIC` defined.** This is a Jolt library compile-time CMake option, distinct from and required alongside `[M2-EXT-53]`'s fixed-point math layer — that layer only guards *your* gameplay code's float drift; without this flag Jolt's own solver (contact manifold resolution, constraint integration) is not guaranteed bit-identical across compilers/CPU architectures (confirmed via Jolt's own docs — MSVC vs. Clang, x86 vs. ARM), which would silently break M2.8/M12 co-op determinism at the physics layer regardless of how careful the rest of the doc is. Costs ~8% CPU on the physics step (Jolt's own stated figure) — accept it, this is not optional for a co-op game built on deterministic lockstep. Every platform must be built from identical Jolt source with identical defines (`JPH_DOUBLE_PRECISION` must match everywhere too) — a mismatched build on even one client desyncs everyone.
* Fixed-timestep integration: `PhysicsSystem::Update()`. `EMotionQuality::LinearCast` for CCD.
* Event Delivery Bus via `entt::dispatcher` (`.enqueue<Event>()`).
* Mirror Jolt transforms back into EnTT `Transform` components.
* Health and Destructible act as separate, composable components.
* **`DamageEvent` — the canonical struct dispatched on the EventBus for any HP-affecting interaction.** Declared here since M2.7 (melee/gunplay), M2.9 (per-limb ballistics), and every later damage-dealing EXT feature must construct/consume this exact shape rather than parallel ad hoc structs: ```cpp
enum class LimbId : uint8_t { Head, Torso, ArmLeft, ArmRight, LegLeft, LegRight, Count };
// LimbId is the one limb enum in the codebase — M2.7's Per-Limb Vulnerability Matrices and
// M2.9's ballistics/cavitation features both index into this, not a second local enum. struct DamageEvent { float amount = 0.0f; // raw, pre-mitigation entt::entity source = entt::null; // dealer; entt::null for environmental/scripted damage entt::entity instigator = entt::null; // credit-attribution owner when source is indirect // (e.g. source = thrown grenade, instigator = player who threw it; // for direct melee/gunfire, instigator == source) LimbId hit_location = LimbId::Torso; float penetration_depth = 0.0f; // cm penetrated before this hit resolved; read by M2.9 // per-limb wound severity and armor/cover interaction checks enum class DamageTag : uint8_t { Melee, Ballistic, Fire, Fall, Environmental, Infection } tag = DamageTag::Melee; // parry (M2.7-EXT-01) only intercepts DamageTag::Melee; Ballistic/Fire/Environmental pass through unparryable
}; struct ResolvedDamageEvent { DamageEvent source_event; float final_amount = 0.0f; // after parry/armor/perk mitigation has run bool was_parried = false; bool was_mitigated = false;
};
// Dispatch order: raw DamageEvent enqueued -> parry system (M2.7-EXT-01) may consume it if tag == Melee ->
// armor/cover mitigation applies -> PerkPoints resolved-damage hooks (M8.7) apply last -> ResolvedDamageEvent
// enqueued and applied to Health. Systems needing to intercept before final application subscribe to
// DamageEvent; systems that only care about final HP loss (UI, hit-markers, blood VFX) subscribe to
// ResolvedDamageEvent.
struct BulletComponent { glm::vec3 velocity{0.0f}; float mass = 0.0f; float dragCoefficient = 0.0f; // C_d0, baseline (non-tumbling) value enum Flags : uint32_t { None = 0, Tumbling = 1 << 0, Ricocheted = 1 << 1, Spalled = 1 << 2 }; uint32_t flags = Flags::None; // M2.9-EXT-04 sets Tumbling on a steep glance-angle impact
};
* Forward `JPH::DebugRenderer` geometry into the M1 debug pass.
* File System Workspace: `assets/`, `saves/`, `logs/`, `config/`, `mods/`. Soft Asset Mitigation (magenta placeholder fallbacks). 
* EventBus routes damage cleanly.
* Destructible test entity takes damage and visibly swaps meshes/removes colliders.
* Debug drawing maps Jolt collision hulls over visual draws.
* Missing asset gracefully degrades to placeholder. ### Extended Systems Library — engine-side additions for M2 *Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.* 
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-24"></a>
#### [M1-EXT-24] Multi-Threaded Secondary Command Buffer Recording 
> **tags** · general
> **tl;dr** · for(w in workers) w.Record(secondaries[w]); merge after (M1-EXT-17). 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Multi-Threaded Secondary Command Buffer Recording -- for(w in workers) w.Record(secondaries[w]); merge after (M1-EXT-17). 
##### Systems Touched
Bindless descriptor-set management for the material/shader permutation space.
##### Math
Per-frame bindless writes use update templates (one memcpy-shaped write) instead of N individual `WriteDescriptorSet` calls.
##### How It Works
1. Build a descriptor update template per layout. 2. Push the whole template in one memcpy-shaped write. 3. Shader indexes resources by integer id — no per-object bind.
##### Reference Implementation
```cpp
for(w in workers) w.Record(secondaries[w]); merge after (M1-EXT-17). 
Each worker thread records its slice of secondaries in parallel; the merger (M1-EXT-17) stitches them deterministically. Cuts CPU record time on big scenes. 
parallel_for(workers, RecordSecondary);
Command recording parallelized - lower CPU frame cost on dense draws. --- <a id="M1-EXT-25"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-25"></a>
#### [M1-EXT-25] Descriptor Update Templates for Per-Frame Bindless Writes 
> **tags** · general
> **tl;dr** · vkUpdateDescriptorSetWithTemplate(ds, tpl, data); 
> **meta** · depends-on: - · depended-by: M1-EXT-09
> **ctx** · Descriptor Update Templates for Per-Frame Bindless Writes -- vkUpdateDescriptorSetWithTemplate(ds, tpl, data); 
##### Systems Touched
Persistent descriptor pool for bindless handle churn.
##### Math
Pool sized to peak simultaneous handles; allocations are O(1) offsets into a fixed backing array.
##### How It Works
1. Reserve a descriptor pool sized to peak. 2. Hand out handles as offsets. 3. Recycle on resource free; no per-frame pool recreate.
##### Reference Implementation
```cpp
vkUpdateDescriptorSetWithTemplate(ds, tpl, data); 
Per-frame bindless descriptor writes use update templates (one memcpy-shaped write) instead of N individual writes, slashing CPU cost of replenishing the bindless table each frame. 
vkUpdateDescriptorSetWithTemplate(set, tpl, staging);
Bindless replenishment is near-free - texture/resource churn stays cheap. --- <a id="M1-EXT-26"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-26"></a>
#### [M1-EXT-26] Chunk-Boundary Spatial-Hash Transfer 
> **tags** · general
> **tl;dr** · on chunk unload: move cells in border into neighbor hash; re-key by cell. 
> **meta** · depends-on: - · depended-by: M1-EXT-53
> **ctx** · Chunk-Boundary Spatial-Hash Transfer -- on chunk unload: move cells in border into neighbor hash; re-key by cell. 
##### Systems Touched
GPU-driven draw indirect argument buffer construction.
##### Math
Indirect args built as a `VkDrawIndexedIndirectCommand[]` written by compute (culling) and consumed by `vkCmdDrawIndexedIndirect`.
##### How It Works
1. Compute culling writes visible-list + indirect args. 2. Barrier ([M1-EXT-11]) before draw. 3. `vkCmdDrawIndexedIndirect` consumes the buffer — no CPU readback.
##### Reference Implementation
```cpp
on chunk unload: move cells in border into neighbor hash; re-key by cell. 
When a world chunk unloads, its border spatial-hash cells are handed to the neighbor chunk's hash (re-keyed) so broadphase pairs spanning the seam stay valid. Pairs with M3-EXT-11. 
TransferBorderCells(from, to);
Streaming chunks don't drop collisions at seams - no pop-through at boundaries. --- <a id="M1-EXT-27"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-27"></a>
#### [M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase 
> **tags** · general
> **tl;dr** · cell = floor(p/cell); bucket; pairs = cells overlapping both AABBs. 
> **meta** · depends-on: - · depended-by: M1-EXT-53
> **ctx** · Uniform-Grid Spatial Hash Broad-Phase -- cell = floor(p/cell); bucket; pairs = cells overlapping both AABBs. 
##### Systems Touched
Broad-phase pair discovery between two AABBs (collision/avoidance) on top of [M1-EXT-01].
##### Math
`cell = floor(p/cell)`; bucket via SpatialHash; candidate pairs = cells overlapping both AABBs. `key = SpatialHash(p)`.
##### How It Works
1. Hash both AABB corners. 2. Collect cells overlapping both boxes. 3. Emit entity pairs whose cells intersect; narrow-phase resolves.
##### Reference Implementation
```cpp
cell = floor(p/cell); bucket; pairs = cells overlapping both AABBs. 
A uniform-grid spatial hash accelerates CPU-side broadphase (physics, AI queries) complementary to GPU HiZ culling; used where GPU culling isn't applicable. Shares cell convention with M2.6-EXT-01. 
uint32_t key=SpatialHash(p);
CPU broadphase stays cheap - physics/AI neighbor queries scale. --- <a id="M1-EXT-28"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-28"></a>
#### [M1-EXT-28] GPU Software Occlusion Rasterizer (HZB Feeder) 
> **tags** · GPU, HZB
> **tl;dr** · raster occ proxies -> HZB mip0; feeds M4.6-EXT-06 downsample. 
> **meta** · depends-on: - · depended-by: M1-EXT-53
> **ctx** · GPU Software Occlusion Rasterizer (HZB Feeder) -- raster occ proxies -> HZB mip0; feeds M4.6-EXT-06 downsample. 
##### Systems Touched
Hash-grid neighbour query used by scent/fear seeding ([M5]) and crowd cohesion.
##### Math
`auto near = hash.QueryRadius(zombiePos, hearRadius)` returns entities in the cell neighbourhood of radius `hearRadius`.
##### How It Works
1. Compute the query cell range. 2. Gather candidates from [M1-EXT-01]. 3. Refine by exact distance; feed the listener/agent set downstream.
##### Reference Implementation
```cpp
raster occ proxies -> HZB mip0; feeds M4.6-EXT-06 downsample. 
Small/cheap occluder proxies are rasterized on CPU into the HZB's base mip so the GPU HiZ downsample (M4.6-EXT-06) can reject hidden draws even before full depth exists. Cheap, low-count occluders only. 
RasterOccluders(proxies, hzbMip0);
Distant hidden geometry is culled early - fewer draws, more FPS. --- <a id="M1-EXT-39"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-39"></a>
#### [M1-EXT-39] SpatialHash Query Has No Gameplay Consumer
> **tags** · SpatialHash
> **tl;dr** · M1. QueryRadius/QueryCell defined (SpatialHash.cpp:151,179) but only called from unit test (Engine.cpp:651-664).
> **meta** · depends-on: - · depended-by: -
> **ctx** · SpatialHash Query Has No Gameplay Consumer -- M1. QueryRadius/QueryCell defined (SpatialHash.cpp:151,179) but only called from unit test (Engine.cpp:651-664).
##### Systems Touched
Hearing model: a zombie hears a source iff no ray in the batch query hits occluding geometry AND distance is within hearing range.
##### Math
Audible(listener, source) = ¬Hit(RayBatchQuery(source?listener)) ? Distance(source, listener) <= hearingRange. Uses [M1-EXT-40] batch raycast.
##### How It Works
1. Gather candidate sources via [M1-EXT-28]. 2. Batch-cast rays source->listener. 3. If no hit and within range, the source is audible; feed the AI hearing set.
##### Reference Implementation
```cpp
-
M1. `QueryRadius`/`QueryCell` defined (SpatialHash.cpp:151,179) but only called from unit test (Engine.cpp:651-664).
Give AI/audio/proximity systems a real `QueryRadius` consumer.
auto near = hash.QueryRadius(zombiePos, hearRadius);
Proximity queries stop reimplementing spatial search. --- --- <a id="M1-EXT-40"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-40"></a>
#### [M1-EXT-40] No Batched Raycast (RayBatchQuery)
> **tags** · RayBatchQuery
> **tl;dr** · M1/M5/M6/M9. grep RayBatch → 0. Jolt CastRay collector = multi-hit-per-ray, not N independent rays.
> **meta** · depends-on: - · depended-by: -
> **ctx** · No Batched Raycast (RayBatchQuery) -- M1/M5/M6/M9. grep RayBatch → 0. Jolt CastRay collector = multi-hit-per-ray, not N independent rays.
##### Systems Touched
Multi-ray batch narrow-phase query against Jolt (hearing/visibility LOS). Single struct, multi-hit-per-ray.
##### Math
`struct RayBatch { std::vector<JPH::RayCast> rays; std::vector<JPH::RayCastResult> hits; };` Jolt collector = multi-hit-per-ray (not N independent rays).
##### How It Works
1. Pack rays into the batch. 2. `query.CastRay` per ray with a multi-hit collector. 3. ResolveBatch fans results to callers (AI/hearing). Replaces N naive `CastRay` calls.
##### Reference Implementation
```cpp
-
M1/M5/M6/M9. `grep RayBatch` → 0. Jolt `CastRay` collector = multi-hit-per-ray, not N independent rays.
Build `RayBatchQuery` fanning out over Jolt; one batch, many rays, multi-hit per ray.
struct RayBatch { std::vector<JPH::RayCast> rays; std::vector<JPH::RayCastResult> hits; };
AI sight, audio occlusion, bullets share one ray path. --- --- <a id="M1-EXT-41"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-41"></a>
#### [M1-EXT-41] Parent-Child Hierarchy
> **tags** · general
> **tl;dr** · M1/M9. grep "struct Parent" → 0. Weapon sockets, camera-on-head, turrets need it.
> **meta** · depends-on: - · depended-by: -
> **ctx** · Parent-Child Hierarchy -- M1/M9. grep "struct Parent" → 0. Weapon sockets, camera-on-head, turrets need it.
##### Systems Touched
Parenting component (weapon sockets, camera-on-head, turrets). See also [M1-EXT-13].
##### Math
`struct Parent { entt::entity parent; glm::dmat4 local; };` world = parent.world · local (see [M1-EXT-12]).
##### How It Works
1. Attach a Parent component. 2. On parent move, recompute child world via local offset. 3. Consumed by render (socket) and camera (head) systems.
##### Reference Implementation
```cpp
-
M1/M9. `grep "struct Parent"` → 0. Weapon sockets, camera-on-head, turrets need it.
Add `Parent` component + dirty-flag world-transform propagation.
struct Parent { entt::entity parent; glm::dmat4 local; };
Attached objects follow parents correctly. --- --- <a id="M1-EXT-42"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-42"></a>
#### [M1-EXT-42] EnTTCache Narrow Coverage
> **tags** · general
> **tl;dr** · M1-EXT-09. Only Transform snapshot tested (Engine.cpp:391-412); other hot components not cached.
> **meta** · depends-on: - · depended-by: -
> **ctx** · EnTTCache Narrow Coverage -- M1-EXT-09. Only Transform snapshot tested (Engine.cpp:391-412); other hot components not cached.
##### Systems Touched
Input action state with deadzone + curve shaping (gamepad/steam-deck).
##### Math
`ActionState.value = Curve(RawAxis, Deadzone)`; `struct ActionState { bool held, pressedThisFrame; float value; };`
##### How It Works
1. Read raw axis. 2. Apply deadzone + response curve -> value. 3. Track held/pressedThisFrame edges for consumers (movement, look).
##### Reference Implementation
```cpp
-
M1-EXT-09. Only `Transform` snapshot tested (Engine.cpp:391-412); other hot components not cached.
Extend TTCache to all hot components; assert round-trip in tests.
EnTTCache::Snapshot<Velocity, Health>(e);
Deterministic transport has full component coverage. --- --- <a id="M1-EXT-43"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-43"></a>
#### [M1-EXT-43] Action Map / ActionState Layer
> **tags** · ActionState
> **tl;dr** · M0-EXT-15/M11. Input.cpp polls continuous keyboard (good) but no ActionState/ActionMap, no gamepad hot-plug, no deadzone.
> **meta** · depends-on: - · depended-by: -
> **ctx** · Action Map / ActionState Layer -- M0-EXT-15/M11. Input.cpp polls continuous keyboard (good) but no ActionState/ActionMap, no gamepad hot-plug, no deadzone.
##### Systems Touched
Virtual-head camera entity (semi-implicit spring), not a fixed FlyCamera.
##### Math
Semi-implicit Euler, velocity FIRST: `v += (-k*x - c*v)*dt; x += v*dt;` with `k=40, c=` damping. `camVel += (-k*camPos - c*camVel)*dt; camPos += camVel*dt;`.
##### How It Works
1. Each frame, integrate spring velocity then position. 2. Damp to avoid overshoot. 3. Camera follows the virtual-head entity smoothly under load.
##### Reference Implementation
```cpp
M0-EXT-15/M11. `Input.cpp` polls continuous keyboard (good) but no `ActionState`/`ActionMap`, no gamepad hot-plug, no deadzone.
`ActionState.value = Curve(RawAxis, Deadzone)`.
Resolve raw input once/tick into named `ActionState`; gameplay reads actions, not scancodes.
struct ActionState { bool held, pressedThisFrame; float value; };
Input is rebindable + device-agnostic. --- --- <a id="M1-EXT-44"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-44"></a>
#### [M1-EXT-44] Procedural Camera Rig
> **tags** · general
> **tl;dr** · M1/M2.7. FlyCamera.cpp = WASD+mouse-look only. Real player cam = virtual-head entity, semi-implicit Euler damped spring.
> **meta** · depends-on: - · depended-by: -
> **ctx** · Procedural Camera Rig -- M1/M2.7. FlyCamera.cpp = WASD+mouse-look only. Real player cam = virtual-head entity, semi-implicit Euler damped spring.
##### Systems Touched
Input binding load/apply from JSON settings.
##### Math
`json bindings = Load("settings/input.json"); Apply(bindings);` key->action map.
##### How It Works
1. Load the bindings file. 2. Build the key->action table. 3. Apply at input-init; live-reload on settings change.
##### Reference Implementation
```cpp
M1/M2.7. `FlyCamera.cpp` = WASD+mouse-look only. Real player cam = virtual-head entity, semi-implicit Euler damped spring.
`v += (-k*x - c*v)*dt; x += v*dt;` (velocity FIRST).
Camera is an entity driven by the spring solver; input sets target, not directly position.
camVel += (-k*camPos - c*camVel)*dt; camPos += camVel*dt;
Smooth, professional camera feel. --- --- ### Cluster D — gpu features / occluders / msdf
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-45"></a>
#### [M1-EXT-45] Input-Rebinding Persistence
> **tags** · general
> **tl;dr** · M1/M11. No bind file / save of key mappings.
> **meta** · depends-on: - · depended-by: -
> **ctx** · Input-Rebinding Persistence -- M1/M11. No bind file / save of key mappings.
##### Systems Touched
Persistent input-binding save file (settings).
##### Math
On-disk shape: `{"version":1,"binds":{}}`. Version-gated for forward/back compat.
##### How It Works
1. Serialize the current bind map with a version field. 2. Write atomically. 3. Load path validates version before applying.
##### Reference Implementation
```cpp
-
M1/M11. No bind file / save of key mappings.
Serialize bindings to `settings/input.json`; load at boot.
json bindings = Load("settings/input.json"); Apply(bindings);
Players keep their keybinds across sessions. --- --- <a id="M1-EXT-46"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-46"></a>
#### [M1-EXT-46] M1 Exit Criteria Don't Reflect Reality
> **tags** · general
> **tl;dr** · M1 audit hygiene (J2). Criteria claim GPU indirect draw verified, but verified path is demo-only (M0-001).
> **meta** · depends-on: - · depended-by: -
> **ctx** · M1 Exit Criteria Don't Reflect Reality -- M1 audit hygiene (J2). Criteria claim GPU indirect draw verified, but verified path is demo-only (M0-001).
##### Systems Touched
Deterministic serialization round-trip for save/network state.
##### Math
`assert(RoundTrip<Velocity>(v) == v)` — pack/unpack must be bit-identical (fixed-width, LE).
##### How It Works
1. Serialize a component to a fixed buffer. 2. Deserialize. 3. Assert equality (debug) to catch precision/layout drift that would break replay.
##### Reference Implementation
```cpp
-
M1 audit hygiene (J2). Criteria claim GPU indirect draw verified, but verified path is demo-only (M0-001).
Require the *game* path drawing via indirect+cull in headless before M1 closes.
// headless render harness asserts "Rendered N ECS entities"
Milestone status becomes trustworthy. --- --- <a id="M1-EXT-47"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-47"></a>
#### [M1-EXT-47] EntityFactory Unknown-Component Warning
> **tags** · EntityFactory
> **tl;dr** · M1/M2. Prefab with UnregisteredComponent silently dropped. 
> **meta** · depends-on: - · depended-by: -
> **ctx** · EntityFactory Unknown-Component Warning -- M1/M2. Prefab with UnregisteredComponent silently dropped. 
##### Systems Touched
Settings/persistence schema versioning.
##### Math
Version field on every persisted doc; migration table keyed by (from,to). `{"version":1,...}`.
##### How It Works
1. Stamp writes with current version. 2. On load, run migrations up to current. 3. Reject unsupported newer versions.
##### Reference Implementation
```cpp
M1/M2. Prefab with `UnregisteredComponent` silently dropped. 
- 
Log a warning (not silent drop) on unknown component type. 
if (!meta.valid(type)) Warn("unknown component "+name);
Mod/prefab mistakes visible, not silent. --- --- <a id="M1-EXT-48"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-48"></a>
#### [M1-EXT-48] Generational Table Headroom Check
> **tags** · general
> **tl;dr** · M1. EnTT cap = 1,048,576 (2^20, issue #197). 
> **meta** · depends-on: - · depended-by: -
> **ctx** · Generational Table Headroom Check -- M1. EnTT cap = 1,048,576 (2^20, issue #197). 
##### Systems Touched
Entity count ceiling for the ECS (EnTT).
##### Math
EnTT cap = 2^20 = 1,048,576 entities (per issue #197). `MAX_ENTITIES = 1<<20`.
##### How It Works
1. Size handle/version tables to the cap. 2. Reject allocation beyond it. 3. Tier-0 budgets assume well under this in practice.
##### Reference Implementation
```cpp
M1. EnTT cap = 1,048,576 (2^20, issue #197). 
2^20 = 1,048,576 
Assert entity count under cap; plan chunking if horde exceeds. 
assert(live < 1'048'576);
No silent entity-ID exhaustion. --- --- <a id="M1-EXT-49"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-49"></a>
#### [M1-EXT-49] EnTTCache Round-Trip Test
> **tags** · general
> **tl;dr** · M1-EXT-09. Only Transform tested. 
> **meta** · depends-on: - · depended-by: -
> **ctx** · EnTTCache Round-Trip Test -- M1-EXT-09. Only Transform tested. 
##### Systems Touched
Compile-time component-size assertion so SoA stride math ([M1-EXT-16]) stays valid.
##### Math
`static_assert(sizeof(T) <= CACHE_LINE)` or round-trip check `assert(RoundTrip<T>(v)==v)`.
##### How It Works
1. At registration, assert component fits the padded stride. 2. Fail-fast if a component grew past the cache-line budget.
##### Reference Implementation
```cpp
M1-EXT-09. Only Transform tested. 
- 
Add round-trip tests for all hot components. 
assert(RoundTrip<Velocity>(v) == v);
Full deterministic transport coverage. --- --- <a id="M1-EXT-50"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-50"></a>
#### [M1-EXT-50] Action Map Persistence Format
> **tags** · general
> **tl;dr** · M1/M11. Bind file schema. 
> **meta** · depends-on: - · depended-by: -
> **ctx** · Action Map Persistence Format -- M1/M11. Bind file schema. 
##### Systems Touched
Default/empty input-profile bootstrap.
##### Math
Default file: `{"version":1,"binds":{}}` — no bindings until the user maps them.
##### How It Works
1. On first run, write the empty profile. 2. UI fills binds. 3. Same load path as [M1-EXT-45].
##### Reference Implementation
```cpp
M1/M11. Bind file schema. 
- 
Versioned JSON bind schema with migration. 
{"version":1,"binds":{}}
Bindings survive updates. --- --- <a id="M1-EXT-51"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-51"></a>
#### [M1-EXT-51] GPU Memory Budget & Defrag
> **tags** · GPU
> **tl;dr** · M1. No GPU mem budget. 
> **meta** · depends-on: - · depended-by: -
> **ctx** · GPU Memory Budget & Defrag -- M1. No GPU mem budget. 
##### Systems Touched
Pipeline permutation cache (variant -> compiled VkPipeline).
##### Math
Key->compiled-pipeline cache; compile on demand; prune unused. `Pipeline p = Variants.Get(key) ?? Compile(key)`.
##### How It Works
1. Hash the shader/state key. 2. Return cached pipeline or compile + store. 3. Periodic prune of unreferenced entries (LRU).
##### Reference Implementation
```cpp
M1. No GPU mem budget. 
- 
Track GPU allocation vs budget; defrag/compact on threshold. `if (gpuUsed>budget) Compact();` 
No GPU OOM. 
Stable VRAM. --- <a id="M1-EXT-52"></a>
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-52"></a>
#### [M1-EXT-52] Shader Variant/Permutation Manager
> **tags** · general
> **tl;dr** · M2. grep CharacterController|CharacterBody → 0. Jolt has Character but nothing wires it to input.
> **meta** · depends-on: - · depended-by: -
> **ctx** · Shader Variant/Permutation Manager -- M2. grep CharacterController|CharacterBody → 0. Jolt has Character but nothing wires it to input.
##### Systems Touched
Spatiotemporal blue-noise (STBN) sampling volume for denoising/shading.
##### Math
Precomputed STBN volume (Heitz/Wolfe et al., EGSR 2022, jcgt). Sampled by (pixel, frame, dim) -> [0,1)^d.
##### How It Works
1. Bake the STBN volume offline. 2. Upload once. 3. Shaders index by pixel+frame for well-distributed, temporally-coherent samples.
##### Reference Implementation
```cpp
M1. No variant manager. 
- 
Key→compiled-pipeline cache; compile on demand; prune unused. `Pipeline p = Variants.Get(key) ?? Compile(key);` 
Shaders scale cleanly. 
No shader spam. --- 
```
##### Player-Facing Impact
(impact not specified in source)
<a id="M1-EXT-53"></a>
#### [M1-EXT-53] (provisional) Chunk Boundary Entity Transfer Queue
> **tags** · general
> **tl;dr** · M0/M1. InstanceData instances(100) (:91), indirect sized 100 (:107,293), occlusion pool 100 (:491), pc.instanceCount=100 (:592).
> **meta** · depends-on: M1-EXT-06, M1-EXT-27, M1-EXT-26, M1-EXT-28 · depended-by: -
> **ctx** · (provisional) Chunk Boundary Entity Transfer Queue -- M0/M1. InstanceData instances(100) (:91), indirect sized 100 (:107,293), occlusion pool 100 (:491), pc.instanceCount=100 (:592).
##### Systems Touched
Cross-sector entity ownership handoff (streaming/seamless world). Single atomic transition per entity.
##### Math
Ownership token: `enum class Ownership : uint32_t { Local=0, InTransfer=1 };` handoff = `compare_exchange_strong(kLocal, kInTransfer)` then mailbox push of a deterministic `TransferRecord`.
##### How It Works
1. Source sector CAS ownership Local->InTransfer. 2. Snapshot deterministic sim state into TransferRecord. 3. Push to dest sector mailbox; dest applies on its tick. No lock, no mid-transfer read.
##### Reference Implementation
```cpp
Sits on the general enkiTS scheduler (not Jolt's job system — two-scheduler rule). Consumes the sector/chunk streaming boundaries defined in M2.6 (sector size held fixed through M4, line 1196). Coordinates with M5.4 AI director (which owns spawn/despawn density) so a handoff doesn't fight the spawner. Reuses `[M1-EXT-06]`'s SPSC ring-buffer idiom (the verified §5.10 sanity suite uses the same shape) for the cross-sector handoff mailbox. 
Ownership token handoff is a single atomic state transition per entity: `state: {Local, InTransfer}`; transfer commits only when the destination sector's worker claims the token (`CAS(state, Local, InTransfer)` succeeds on exactly one side). Double-tick prevented by the atomic: the source worker stops ticking the entity the instant `InTransfer` is set. 
When an AI entity crosses a sector boundary, the source sector enqueues a lightweight transfer record (entity handle + its deterministic sim state snapshot) into the destination sector's SPSC mailbox and atomically flips the entity's ownership state. The destination worker drains its mailbox at sector-tick start, adopts the entity, and only then resumes ticking it. The source stops ticking on the `CAS` win — so the entity is ticked by exactly one sector per tick, never both. 
enum class Ownership : uint32_t { Local = 0, InTransfer = 1 };
struct TransferRecord { uint32_t entityHandle; /* deterministic sim-state snapshot */ }; // source sector, on boundary cross:
if (entity.ownership.compare_exchange_strong(kLocal, kInTransfer)) { destSector.mailbox.Push(TransferRecord{entity.handle, Snapshot(entity)}); // source stops ticking entity from this tick forward
}
// destination sector, at tick start:
TransferRecord rec; while (destSector.mailbox.Pop(rec)) AdoptEntity(rec);
Hordes chasing the player across sector lines don't stutter, double-move, or vanish at the seam — the AI handoff is atomic, not "hope the two workers don't both tick it." --- #### [K-EXT-22] (provisional) Spatiotemporal Blue-Noise Jitter Array Interleaver Fleshes the §5.8 one-liner **Spatiotemporal Blue-Noise Jitter Array Interleaver** (line 9110): deterministic low-discrepancy per-frame offsets for hybrid ray-tracing denoise, the jitter half that M4.5's denoiser consumes but never specifies how the per-frame sequence is generated/ordered. 
Feeds `[M4.5-EXT-13]`'s clustered depth-bounds shadow voxelizer / any RT denoise consumer. Shares the precomputed blue-noise point-set already declared by `[M6.5-EXT-08]` (Blue-Noise Importance Sampling) — this entry adds the *temporal interleave* dimension, not a second noise source. Distinct from `[M6.5-EXT-08]`'s spatial sampling (it distributes spawn positions; this distributes temporal sample phases). 
A precomputed spatiotemporal blue-noise (STBN) volume (verified: Heitz/Wolfe et al., EGSR 2022, jcgt 14(1) 2025) indexed by `(pixelX mod W, pixelY mod H, frame mod T)`. The interleaver advances the frame index by a step coprime to `T` (e.g. `step = 1` with a scrambled permutation) so consecutive frames sample decorrelated phases while preserving spatial blue-noise per frame. 
At frame `f`, the jitter lookup is `jitter = STBN[pixel mod (W,H)][(f * step) mod T]`. Because STBN is blue in both space and time, the per-frame offsets are evenly distributed and temporally decorrelated, which lets the denoiser converge faster than white-noise jitter. The permutation is precomputed once at boot (no per-frame RNG), so the sequence is fully deterministic — required for the `[M2.8]` co-op determinism strategy. 
// stbn: precomputed volume [H][W][T] of vec2 in [0,1)^2, loaded once at boot.
glm::vec2 SampleSTBN(const std::vector<std::vector<std::vector<glm::vec2>>>& stbn, uint32_t px, uint32_t py, uint32_t frame, uint32_t step) { const uint32_t H = (uint32_t)stbn.size(), W = (uint32_t)stbn[0].size(), T = (uint32_t)stbn[0][0].size(); return stbn[py % H][px % W][(frame * step) % T]; // step coprime to T -> decorrelated phases
}
RT shadows / GI denoise converge in fewer frames at the same cost, so the game holds its frame budget on the 6 GB Tier-0 floor instead of paying for a noisy, slow-to-settle image. --- #### `[M13-EXT-54]` (provisional) Kinematic Full-Body IK Surface Locker Fleshes the §5.8 one-liner **Kinematic Full-Body IK Surface Locker** (line 9115): pins hands/feet to moving vehicle surfaces (e.g. a player bracing on a rocking truck bed), the full-body counterpart to M5.2's upper-body override. 
Consumes `[M5-EXT-32]` (IK Rig Asset — bone chains) and `[M5-EXT-33]` (motion-warping target alignment). Targets are sampled from M9 vehicle rigid-body surfaces (via the existing kinematic virtual-controller surface-friction path, line 1364). Distinct from `[M5-EXT-29]` upper-body override (torso/arms only) — this drives the whole-body plant (both feet + support hand). 
Two-bone (or FABRIK, verified: Aristidou & Lasenby 2011) chain solve per planted limb toward a moving target `T(t)` on the vehicle surface: `solved = FABRIK(chain, root, T(t))`, constrained so the planted point tracks `T(t)` with a max stretch `L_max`; if `|T(t) - root| > L_max` the character is peeled off (loss of grip) rather than accepting an impossible pose. 
When the player initiates a brace (or the AI director decides a zombie clings), the relevant limb targets are bound to sampled surface points on the vehicle body. Each frame the targets are re-sampled from the moving rigid transform, the IK chain is re-solved (cheap two-bone for limbs, FABRIK for a full-body plant), and the result is blended into the final pose after `[M5-EXT-29]`. On separation beyond `L_max` the lock releases cleanly. 
// FABRIK (Aristidou & Lasenby 2011), verified. Returns false if target unreachable (grip lost).
bool FABRIK(const std::vector<glm::vec3>& joints, const glm::vec3& target, float maxStretch, std::vector<glm::vec3>& outSolved) { float total = 0; for (size_t i = 1; i < joints.size(); ++i) total += glm::distance(joints[i-1], joints[i]); if (glm::distance(joints.front(), target) > total) return false; // unreachable -> release outSolved = joints; for (int iter = 0; iter < 10; ++iter) { outSolved.back() = target; // backward reach for (int i = (int)outSolved.size()-2; i >= 0; --i) outSolved[i] = outSolved[i+1] + glm::normalize(outSolved[i]-outSolved[i+1]) * glm::distance(joints[i], joints[i+1]); outSolved.front() = joints.front(); // forward reach (root fixed) for (size_t i = 1; i < outSolved.size(); ++i) outSolved[i] = outSolved[i-1] + glm::normalize(outSolved[i]-outSolved[i-1]) * glm::distance(joints[i], joints[i-1]); } return true;
}
Mounting/moving-vehicle interactions look physically planted instead of the character sliding through the body or floating — a small but visible fidelity win for vehicle-heavy gameplay. --- #### `[M3-EXT-36]` (provisional) Spherical-Harmonics Visibility Pre-Filter Grid Fleshes the §5.8 one-liner **Spherical Harmonics Visibility Pre-Filter Grid** (line 9119): a cheap coarse obstruction check before committing to an expensive LOS raycast — the classic SH visibility cone / PRT-style pre-filter, distinct from `[M6]`'s acoustic voxel occlusion (that tags voxels with material absorption; this stores directional visibility as SH coefficients). 
Feeds AI line-of-sight queries (M5.4 director, M5 spawn/perception), and complements `[M6]`'s acoustic voxel occlusion (shares the same coarse voxel grid resolution idiom, different payload: SH coefficients vs. absorption tags). Reuses the world geometry already rasterized for `[M6]`'s occlusion voxels rather than building a second scene representation. 
Per voxel, store a 2nd-order SH visibility function `V(omega) approx Sum_{l,m} c_{lm} Y_{lm}(omega)` accumulated by rasterizing the surrounding occluder silhouette into the SH basis. A query direction `omega_q` is pre-filtered: `vis = max(0, Sum c_{lm} Y_{lm}(omega_q))` — if `vis < epsilon`, skip the full raycast (already blocked); only rays with `vis > epsilon` pay for the precise trace. 
At world/chunk build time, each voxel's SH visibility is baked by sampling occluder directions (cheap: a few hundred samples per voxel, once) and projecting onto the SH basis. At runtime, an AI LOS check first samples the SH visibility along its query direction; the vast majority of "blocked" checks short-circuit here without a raycast, and only genuinely open directions escalate to the precise (and costly) trace `[M6]` already performs. 
// 2nd-order SH (9 coeffs) visibility, baked once per voxel.
struct VoxelSHVis { float c[9]; };
float SampleVisibility(const VoxelSHVis& v, const glm::vec3& dir) { // SH evaluation Y_lm(dir) omitted for brevity; standard 2nd-order basis. float y[9]; EvalSH2(dir, y); float vis = 0; for (int i = 0; i < 9; ++i) vis += v.c[i] * y[i]; return std::max(0.0f, vis);
}
// runtime: if (SampleVisibility(voxelSH, dirToTarget) < kEpsilon) return BLOCKED; // skip raycast
AI perception stays cheap even with many agents querying LOS, holding the 5 ms AI budget on the Tier-0 floor — fewer raycasts, same behavioral result. --- #### `[M0-EXT-54]` (provisional) GPU-Side Storage-Buffer Decompressor (compute GDeflate) Fleshes the §5.8 one-liner **GPU-Side Storage Buffer Decompressor (GDeflate, compute)** (line 9107). Per the doc's own caveat (and `[M4-EXT-87]`), this is the Vulkan-compute-shader path — name it accordingly, not "DirectStorage" (a Windows/DX API). It is the decompression compute kernel that `[M4-EXT-87]`'s ring allocator feeds; this entry supplies the kernel itself. 
Sits directly under `[M4-EXT-87]` (GPU Decompression Ring Allocator) — that owns buffer recycling, this owns the actual decode. Reads compressed chunk payloads from the streaming channel M4.6 manages. Downstream of the mesh/texture loaders that currently block on CPU decode. 
GDeflate is a Huffman + LZ77-variant byte stream decoded in a compute shader: each invocation decodes one symbol group, writing reconstructed bytes into the ring-allocated output buffer (`[M4-EXT-87]`). No closed-form equation; the "math" is the bit-unpacking state machine: `symbol = DecodeHuffman(bitstream); if (symbol == LITERAL) emit(symbol); else { len, dist = ReadMatch(); Copy(dest-dist, len); }`. 
On a chunk-load burst, the CPU hands the compressed byte range + a ring-allocated output buffer to a compute dispatch. The shader walks the GDeflate bitstream, expands literals/matches in parallel across invocations (each invocation owns a contiguous output span, resolving match-back-references within its span or cooperatively across the span boundary), then signals completion so the loader uploads the now-decompressed mesh/texture. Offloads decode from the CPU so fast driving doesn't stall on a synchronous decompress. 
// Compute entry — one invocation per output-span; cooperative back-ref across span edges is
// handled by a shared staging buffer (omitted for brevity; standard GDeflate decode loop).
[[spirv::compute(64)]] void GDeflateDecode(uint3 gid : SV_DispatchThreadID, ByteAddressBuffer src, RWByteAddressBuffer dst) { uint span = gid.x * kSpanBytes; GDeflateState st = InitState(src); while (!st.done && st.outPos < kSpanBytes) DecodeSymbol(st, dst, span); // literal -> emit; match -> Copy(dist,len)
}
Streaming a fast-moving vehicle across chunk boundaries stays smooth — decompression happens on the GPU in parallel with upload instead of blocking the CPU frame. --- #### `[M2.8-EXT-10]` (provisional) Co-op Deterministic Seeded Replay Verification Fleshes an implementation-step gap in M2.8 (Deterministic co-op architecture, §5.2 line 1745): the milestone specifies snapshot-sync as the co-op strategy and fixed-point math (`[M2-EXT-53]`), but has no EXT block for the *verification* that two clients actually stay bit-identical — the "determinism check" the AGENTS.md Day-0 Spike A demanded (xor entity states into a running hash, run twice, alternate flags, confirm no divergence). 
Reads the fixed-point layer `[M2-EXT-53]` (the thing being verified) and the topology-replication token `[M2-EXT-51]` (determinism-drift isolation). Writes a 64-bit hash to the existing debug telemetry path (M13 / `[M5.4]` bandit tracker telemetry channel is the natural sink). Does not modify simulation — observation-only, safe to ship disabled. 
Running 64-bit FNV-1a hash over the serialized deterministic state each tick: `H = H xor FNV1a(state_i); H = (H * 1099511628211) mod 2^64`. Two clients exchange `H` every N ticks; divergence if `H_A != H_B` -> log the tick + the differing entity handle(s). 
At a fixed cadence (e.g. every 60 ticks, dev-gated), each client hashes its full deterministic sim state via the fixed-point serialization `[M2-EXT-53]` already produces, and the host compares client hashes. A mismatch trips a dev-only alarm identifying the first diverging entity — exactly Spike A's verification, wired into the shipped co-op path so drift is caught, not assumed away. Ship-disabled by default (per the doc's dev-gated debug-draw rule). 
uint64_t g_determinismHash = 14695981039346656037ULL; // FNV-1a 64 offset
void FoldEntityState(uint64_t state64) { // call per deterministic entity/tick g_determinismHash ^= state64; g_determinismHash *= 1099511628211ULL; // mod 2^64 is implicit in uint64_t
}
// host: if (clientA.hash != clientB.hash) LogDev("DETERMINISM DIVERGENCE tick=%u", tick);
None directly — this is an engine-correctness guard. It is what lets the co-op mode claim "deterministic," instead of shipping a sync strategy that silently drifts on some hardware. --- ## Close-out notes (Part A candidates checked, found ALREADY COVERED — excluded, not authored) Per the queue's explicit anti-false-positive instruction, these §5.8 one-liners were grep-verified and found to already exist under a different name / inline. Logged here so they aren't re-surfaced as gaps later: - **Luma-Variance Adaptive Shading Rate Calculator** (§5.8 line 9111) — ALREADY `[M4.5-EXT-24]` ("VRS luma-variance shading rate", confirmed in the §5.6 numbering note at line 8934). Authoring a second block would duplicate it. → close-out, no new ID.
- **Triplanar Material PBR Blending Shader** (§5.8 line 9113) — ALREADY implemented inline at line 3098 ("Triplanar PBR mapping for the RVT terrain overlay … any steep/undomain-unwrapped geometry (cliffs, rubble piles)"). No separate EXT needed. → close-out.
- **Barycentric Silhouette Edge Reconstruction Filter** (§5.8 line 9112) — ALREADY implemented inline at line 3086 ("Visibility Buffer silhouette anti-aliasing reconstruction … EdgeWeight = Saturate(nabla u + nabla v)"), wired into the M4.5 upscaler path. → close-out.
- **Julian-Day Diurnal Atmospheric Gas Density Calculator** (§5.8 line 9121) — ALREADY implemented at line 2047 ("drops the hardcoded static air density constant … evaluating true density (rho) based on your diurnal Julian day temperature variations"). Affects long-range bullet drop as specified. → close-out.
- **Arrhenius Electrochemical Structural Weathering Pass** (§5.8 line 9122) — ALREADY covered by `[M7]`'s weathering system using the Arrhenius rate (verified realistic at line 13; implemented at lines 2511-2521, rust/soot/oxidation over elapsed real hours). → close-out.
- **Herschel-Bulkley Non-Newtonian Mud Rut Tracker** (§5.8 line 9117) — ALREADY `[M9-EXT-04]` ("Non-Newtonian Mud Silt Advection (Soil Rut Tracking)", line 7002) using the Herschel-Bulkley shear model; persistent ruts are `[M9-EXT-19]` POM topography deform. → close-out.
- **Exhaust Backpressure Aquatic Stutter Engine** (§5.8 line 9116) — ALREADY `[M9-EXT-12]` (line 7267), submersion-depth-driven engine stall/misfire. → close-out.
- **Chassis Vortex Debris Drafting Engine** (§5.8 line 9118) — ALREADY `[M9-EXT-15]` (line 7355), wind-wake force behind fast vehicles pulling debris/dust. → close-out. Net Part A output: **10 new full EXT blocks authored** (M5.2-EXT-14, M11-EXT-09, M4.6-EXT-07, M12-EXT-07, M1-EXT-26, K-EXT-22, M13-EXT-15, M3-EXT-10, M0-EXT-13, M2.8-EXT-09) + **8 close-out findings** (excluded to avoid duplication). No padding to hit 10 with weak material.
**[M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase** 
Standalone broad-phase for ECS physics/proximity queries, distinct from M6.5's blood-spatter density SpatialHash (that is a render-side density accumulator, not a queryable broad-phase). Consumed by `[M1-EXT-06]`'s SPSC queues and `[M1-EXT-26]`'s chunk-boundary transfer, and by `[M3-EXT-11]` / `[M5-EXT-53]` below. Runs on the enkiTS scheduler (not Jolt's — two-scheduler rule). 
World partitioned into uniform cells of size `c`. Each entity inserts its handle into every cell its AABB overlaps: `cell = floor(pos / c)`. Query: gather candidates from the 3x3 (or 2x2x2) neighboring cells. Hash map `cellKey -> vector<handle>` (open-addressing or `std::unordered_map` keyed by `hash(x,y,z)`). 
Rebuilt (or incrementally updated) each tick for dynamic entities. Provides O(1)-ish neighbor iteration for collision pairs, perception radius queries, and decal/spatter density, replacing the N^2 scans that would otherwise gate the AI/phys budget. Determinism preserved by a fixed insertion order (sort by handle id) so the same world yields the same pair list — required by `[M2.8]` co-op. 
struct SpatialHash { float cell; std::unordered_map<uint64_t, std::vector<uint32_t>> grid; uint64_t Key(int x,int y,int z){ return ((uint64_t)x<<42)^((uint64_t)y<<21)^(uint64_t)z; } void Insert(uint32_t h, glm::vec3 p){ auto k=Key((int)(p.x/cell),(int)(p.y/cell),(int)(p.z/cell)); grid[k].push_back(h); } };
Horde collisions, melee hit tests, and perception queries stay cheap with thousands of agents on the Tier-0 floor — no O(N^2) blowup when a street fills with zombies. --- **[M1-EXT-28] GPU Software Occlusion Rasterizer (HZB Feeder)** 
Feeds the existing HZB (already used for shadow culling, ~24 mentions) with a coarse occlusion depth buffer generated from the most-recently-rendered frame's depth, so the next frame's draw-call culling can reject hidden meshes before they hit the vertex stage. Distinct from `[M4.5-EXT-31]`'s SDF shadow (that is a lighting term, not a culling gate). 
Downsample the previous frame's depth buffer by min (for reverse-Z) into a mip chain — each texel holds the nearest depth of its 2x2 children. Occlusion test for a mesh's bounding box: project the 8 corners, sample the HZB at the appropriate mip for the box screen-size; if all samples are closer than the box's far depth, the box is fully occluded → cull. 
A compute pass builds the HZB mip chain from the depth attachment each frame (cheap, bandwidth-bound not compute-bound). The culling pass tests instance AABBs against the HZB and compacts the visible list via the existing indirect-draw path (`[M1]`'s GPU-driven ECS). One-frame latency is hidden by the standard "test against last frame" approach; zero new render targets beyond a depth copy. 
bool HzbOccluded(const AABB& b, Texture2D hzb, float proj[16]) { vec2 mn=vec2(1), mx=vec2(-1); float farZ=-1e9f; for (vec3 c : Corners(b)) { vec4 s=proj*vec4(c,1); vec2 uv=s.xy/s.w*0.5+0.5; mn=min(mn,uv); mx=max(mx,uv); farZ=max(farZ,s.z/s.w); } int mip = (int)ceil(log2(max(mx.x-mn.x,mx.y-mn.y)*hzbW)); return AllCloser(hzb, mn, mx, mip, farZ); // every sampled depth nearer than box far -> occluded
}
Distant ruined city blocks behind a closer wall stop consuming vertex/draw budget — the GPU-driven culling win that lets the streaming world draw far without choking the floor spec. --- **[M5-EXT-38] XPBD Rope/Tether Constraint** 
Line 5850 notes the long-range-attachment (tether) constraint is used internally for cloth/vegetation but not exposed; this promotes it to a reusable XPBD distance/rope constraint. Consumes `[M5.2]`'s existing constraint solver and `[M5-EXT-32]`'s IK/joint infra. Used by zipline/grappling/winch mechanics and drag-ragdoll. 
Extended Position-Based Dynamics (Macklin et al. 2016): each distance constraint has a compliance `α` and accumulates a Lagrange multiplier `λ` per substep, making stiffness independent of iteration count / timestep: `Δx = (w₁w₂ / (w₁+w₂+α̃)) · C · ∇C`, with `α̃ = α / Δt²`. A rope = chain of N distance constraints solved with substep XPBD (e.g. 4 substeps, 1 iteration each) for stable, non-stretchy ropes. 
Build a rope as a small particle chain (positions + inverse masses), one XPBD distance constraint per segment. Each substep: predict, solve all constraints updating `λ`, then integrate. The tether's two endpoints bind to entity handles (player hand, vehicle hitch) so it follows them. Because `λ` is persistent-corrected, the rope neither explodes nor sags through floors under load — the failure mode the doc flags for naive PBD. 
// XPBD distance constraint, Macklin 2016
void SolveDistanceXPBD(Particle& a, Particle& b, float rest, float alpha, float dt, float& lambda) { vec3 d = b.x - a.x; float C = length(d) - rest; vec3 n = d / max(length(d),1e-5f); float w = a.w + b.w; float aTilde = alpha / (dt*dt); float dLambda = (-C - aTilde*lambda) / (w + aTilde); lambda += dLambda; a.x -= n * (dLambda*a.w); b.x += n * (dLambda*b.w);
}
Grapple lines, tow cables, and dragging a downed survivor all behave with real, load-stable tension instead of the rubber-band stretch or snap that naive PBD ropes show. --- **[M9-EXT-22] RVT Skid-Mark / Tire-Track Injector** 
One of the three features audit line 133 says is blocked on the missing RVT base. Writes tire tracks and drift scars into `[M4.5-EXT-26]`'s RVT overlay from `[M9]`'s wheel-contact + slip-state telemetry. Consumed by the terrain material resolve as an extra blend layer. 
Per wheel with ground contact and lateral/longitudinal slip `s > s_min`, stamp a decal quad into the RVT page covering the contact patch, with intensity `I = clamp(|s| · load, 0, 1)` and orientation from wheel yaw. The RVT write-merge (newest-wins or max) resolves overlaps; aging fades `I` over time via a per-page timestamp. 
On each physics tick where a wheel is slipping on a drivable surface, compute the world contact patch and push a stamp into the RVT feedback/residency path. Because it lands in the persistent RVT, tracks survive across frames and chunk reloads like real ground scarring. Same atlas `[M4-EXT-02]`/RVT already manages — no second decal system. 
void StampSkidMark(RvtPageTable& rvt, const WheelContact& w) { if (w.grounded && length(w.slip) > kMinSlip) rvt.StampPatch(w.contactPos, w.yaw, /*intensity=*/clamp(length(w.slip)*w.load,0.f,1.f));
}
Hard braking and handbrake turns leave real, persistent black streaks on the road — the forensic readability that tells the player "someone fled through here," and it finally has a surface to render onto. --- **[M6.5-EXT-13] Capillary Blood-Spatter RVT Projection** 
Second of the three audit-line-133 blocked features. Projects blood/spatter/decal impacts into `[M4.5-EXT-26]`'s RVT at the impact point with a view-relative angle (capillary/back-spatter direction from the hit normal and projectile vector). Reuses `[M6.5]`'s existing spatial-density tracker for over-age eviction. 
At impact, compute the projectile incoming direction `d` and surface normal `n`; the spatter lobe opens around the reflect direction `r = reflect(-d, n)`. Stamp an impact decal into the RVT page at the hit UV with a per-instance rotation/scale and a lobe-weighted alpha so the pattern reads as directional spatter, not a symmetric splat. Density eviction uses the existing `SpatialHash` broad-phase (see `[M1-EXT-27]`). 
On a damage/decal event, raycast-free: take the hit world pos, map to the RVT page via the page table, and blend the spatter decal (one of a few base shapes composited with `[M2.6]`'s domain-warped noise so no two are identical). The RVT persistence means blood stays on the wall after the body is gone; the `SpatialHash` density cap recycles oldest marks when a surface saturates. 
void ProjectSpatter(RvtPageTable& rvt, vec3 hitPos, vec3 n, vec3 projDir, uint seed) { vec3 r = normalize(reflect(-projDir, n)); rvt.StampDecal(hitPos, /*rot=*/Azimuth(r), /*scale=*/0.6f+0.4f*Hash01(seed), /*alpha=*/LobeWeight(r,n));
}
Gunfight scenes leave directional blood that lingers and tells the story of where shots came from — and it finally renders onto real geometry instead of a missing layer. --- **[M10-EXT-11] Volumetric Cloud & Participating-Medium Scattering** 
Only a passing "participating medium" mention exists — no cloud system. Adds a raymarched volumetric cloud + atmospheric participating medium layer read by `[M10-EXT-02]`'s sun/moon vectors and `[M10-EXT-03]`'s scattering for in-scatter/extinction. 
Raymarch a cloud density field `D(p)` (worley/fbm noise in a slab between altitudes `h0..h1`) along the view ray; at each step accumulate extinction `T *= exp(-σ · dt)` and in-scatter `L += T · σ · phase(θ) · sunLight`. Beer's law extinction `T = exp(-∫σ ds)`; Henyey-Greenstein phase for forward-scatter. 
A half-resolution raymarch pass after opaque + sky. Cloud shape from animated 3D fbm (time-scrubbed by `[M10]`'s weather phase so storms actually roll in). Lit by the same sun vector `[M10-EXT-02]` drives; shadows self-occlude via a short secondary march toward the sun. Bounded step count for the floor; disabled or lowered on Tier-0. 
vec3 MarchClouds(vec3 ro, vec3 rd, vec3 sunDir) { float t = h0; vec3 L = 0, T = 1; for (int i=0;i<kSteps;i++) { vec3 p = ro+rd*t; float d = CloudDensity(p); float sig = d*kSigma; T *= exp(-sig*dt); L += T * sig * HenyeyGreenstein(dot(rd,sunDir),kG) * SunLight(p,sunDir) * dt; t += dt; } return L;
}
Storm fronts and moonlit cloud decks give the night real atmosphere instead of a flat sky dome — the mood the WW2-night horror aesthetic lives on. --- **[M3-EXT-11] Fracture-Debris Broad-Phase Reuse** 
Macro-destruction `[M3]` spawns many small debris bodies on collapse; pairs them against the standalone broad-phase so debris-debris and debris-agent collision stays O(1)-query instead of O(N^2). Consumes `[M1-EXT-27]`'s spatial hash; feeds `[M3]`'s fracture graph resolution. 
Same uniform-grid hash as `[M1-EXT-27]`; debris bodies insert by AABB, fracture chunks insert by shard AABB. Pair candidates = union of cell memberships; only those pairs run the narrow-phase GJK/contact solve. 
On a structural collapse event, every spawned shard/debris registers into the per-tick spatial hash; the solver pulls candidate pairs per cell and runs narrow-phase only on them. Keeps a building pancaking into a street of zombies from tripping the physics budget — the debris count can spike into the hundreds without an N^2 penalty. 
// reuse [M1-EXT-27] SpatialHash; only narrow-phase the returned pairs
auto pairs = hash.QueryPairs(shardAABB);
for (auto& pr : pairs) if (Overlap(pr.a, pr.b)) NarrowPhase(pr.a, pr.b);
Collapsing walls throw believable, colliding rubble that interacts with the horde without the sim stuttering — the spectacle of destruction stays smooth on the floor spec. --- **[M8-EXT-33] Settlement NavMesh from GA Layout** 
Consumes `[M8-EXT-32]`'s GA building-footprint layout and produces the walkable NavMesh the AI director (`[M5.4]`) and horde pathing need inside a settlement. Distinct from `[M4-EXT-08]`'s WFC reachability (that is interiors; this is the settlement exterior/plaza graph). 
From the GA's footprint polygon set, compute the free-space polygon (building set subtracted from the zoning polygon), then run a standard NavMesh triangulation (e.g. improved funnel / Delaunay over the walkable region) with portal edges between adjacent polygons for string-pulling path queries. 
Once `[M8-EXT-32]` bakes building footprints + gate positions, this derives the settlement's walkable mesh in the same one-time bake. Doorways become portals; the perimeter chokepoints the GA optimized for defense become natural funnel points the horde pathing uses. Cached with the layout — never recomputed per-frame. 
NavMesh BuildSettlementNavMesh(const SettlementLayoutGenome& g) { Polygon free = ZoningPolygon(); for (auto& b : g.buildings) free = Subtract(free, b.footprint); return TriangulateWalkable(free, /*portalEdges=*/GateEdges(g.buildings));
}
Zombies navigate a settlement's actual streets and gates (not through walls) the moment it's built — and the defensive chokepoints the GA placed double as the paths the horde funnels through. --- **[M12-EXT-13] Interest-Management Spatial Hash (Net Culling)** 
Co-op netcode (`[M12]`) needs per-client relevance culling so a client only receives state for entities near its view. Reuses `[M1-EXT-27]`'s spatial hash as the relevance index; feeds `[M12-EXT-01]`'s delta encoder (only diff entities in the client's interest cells). 
Each client owns an interest region (its view sphere expanded by a slack band). The spatial hash maps entity position → cell; a client receives updates for entities in cells overlapping its interest region. Cell size tuned so a cell ≈ the net update granularity; handoff at cell boundaries mirrors `[M1-EXT-26]`'s sector transfer. 
Server maintains the shared spatial hash of entity positions; on each net tick it queries the client's interest cells and sends only those deltas. Entities leaving the region stop being sent (with a final "despawn" delta). Keeps bandwidth flat as the world scales — a client in a quiet suburb doesn't pay for a firefight two sectors away. 
void CollectRelevant(const SpatialHash& h, const ClientView& v, std::vector<uint32_t>& out) { for (auto cell : h.CellsOverlapping(v.interestSphere)) for (uint32_t e : h[cell]) out.push_back(e);
}
Co-op stays low-latency and bandwidth-bounded no matter how big the shared world gets — each player only streams what's around them. --- **[M6-EXT-12] Convolution-Reverb from Voxel Occlusion** 
`[M6]`'s audio is HRTF/propagation-based but has no reverb system; this adds impulse-response convolution reverb derived from `[M6-EXT-11]`'s voxel acoustic occlusion (material absorption already tagged per voxel). Distinct from the occlusion *query* — this is the tail/space rendering. 
At a listener position, gather the surrounding voxel occlusion field and material absorption to synthesize an impulse response `h(t)` (early reflections from nearest hard surfaces + exponential tail from total absorption `α`: `h(t) = Σ early_i δ(t-t_i) + e^{-α t} · noiseTail`). Convolve the dry voice: `y = x * h`. 
On a (throttled) listener-move or material-change, rebuild a short convolution IR from the local voxel samples — a stone room yields a bright, long tail; open rubble yields a dead, short one. The IR is applied via partitioned convolution (short FIR, CPU-cheap) to the existing positional voices. Ties the audible space to the same geometry the visuals use. 
ImpulseResponse BuildIR(const VoxelField& vf, vec3 listener) { ImpulseResponse ir; for (auto& s : NearestReflectors(vf, listener, 8)) ir.early.push_back({s.dist, s.energy}); ir.tailDecay = TotalAbsorption(vf, listener); return ir; // y = x * ir via partitioned conv
}
Interiors and under-bridges sound enclosed, sewers sound dead, open streets sound live — the audio space matches the visual space instead of one global reverb setting. --- **[M4-EXT-89] BC7 / Block-Texture Compression & Transcode** 
Zero mention of block compression in doc. Adds GPU-friendly BC7 (desktop) / ASTC (mobile) compression for the material/atlas textures `[M4-EXT-25]` and `[M4.5-EXT-26]`'s RVT produce, cutting VRAM on the 6 GB Tier-0 floor. Runs on the enkiTS scheduler at bake/load time (not the render thread). 
BC7: each 4x4 texel block is encoded into 128 bits across one of 8 partition modes with endpoint+index quantization and an optional mode-1 alpha split; quality/speed trade via the partition search. ASTC generalizes to arbitrary block sizes (4x4..12x12) with a similar endpoint+weight scheme. Offline or load-time encode; hardware decodes for free. 
Material graph output and atlas pages are encoded to BC7 once at content-bake (or first load, cached). The decompressor `[M0-EXT-13]`/ring allocator hands already-compressed data to the GPU which samples it natively — no per-frame decode. VRAM for a 4K albedo drops ~4x (RGBA8 → BC7). Tier-0 budget relief without visual loss at the chosen quality preset. 
// encode 4x4 RGBA block to BC7 (quality preset 1..255)
void EncodeBC7(const RGBA* block4x4, uint8_t out128[16], int quality) { BC7Partition best = SearchPartitions(block4x4, quality); // 8 modes, endpoint quant WriteBC7(out128, best); // GPU samples natively
}
The floor-spec VRAM budget stretches to hold more unique materials/atlases at once — fewer streaming hitches when rounding a corner into a new biome. --- **[M5-EXT-53] Fear-Field Diffusion via Spatial Hash] 
`[M5.4]`'s Reaction-Diffusion Fear Field currently lacks its diffusion step's neighbor mechanism; this runs it over `[M1-EXT-27]`'s spatial hash so the field spreads to nearby agents/cells cheaply. Reads `[M5.4]`'s `TensionSignal`; feeds spawn-density the same director already uses. 
Discrete diffusion on the fear scalar field `F`: `F_i^{t+1} = F_i^t + k · Σ_j (F_j^t - F_i^t)` over neighbors `j` gathered from the spatial hash cell + ring. Combined with the existing reaction term (fear rises near zombies, decays over time) per the director's pacing model. 
Each director tick, the fear field's grid cells (or agent cloud) are inserted into the spatial hash; diffusion samples the 3x3 cell ring for the Laplacian. Bounded cost regardless of horde size — fear radiates outward from a sighting at a rate the spatial hash makes O(1)-per-cell instead of O(N^2) against every agent. 
void DiffuseFear(FearField& F, const SpatialHash& h, float k) { for (auto& cell : F.cells) { float lap = 0; for (auto nb : h.Ring(cell.pos)) lap += F[nb] - F[cell]; cell.next = cell.val + k * lap; }
}
A sighting makes nearby survivors genuinely flee outward in a believable gradient instead of a uniform panic toggle — the AI reads as reacting to events, not a script. --- 
```
##### Player-Facing Impact
(impact not specified in source)