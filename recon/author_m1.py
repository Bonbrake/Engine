#!/usr/bin/env python3
"""author_m1.py — Content pass for spec/M1.md (companion to fix_m1.py).

Takes the mechanically-repaired draft (recon/_m1_draft.md) and:
  1. Normalizes EVERY block: balances code fences, splits welded '##### Header <prose>'
     into '##### Header' + prose paragraph.
  2. Fills the missing 5-subsection skeleton (Systems Touched / Math / How It Works /
     Reference Implementation / Player-Facing Impact) with REAL prose grounded in each
     block's own code + deps (CONTENT dict below — hand-authored, no filler).

Does NOT touch spec/M1.md unless --apply AND --out points at it.

Run:
  python recon/author_m1.py --in recon/_m1_draft.md --out recon/_m1_authored.md
  python recon/author_m1.py --apply   # write spec/M1.md (needs GO)
"""
import sys, re, os, argparse, json

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")
SRC = os.path.join(SPEC, "M1.md")
OUT_DEF = os.path.join(BASE, "_m1_authored.md")
SIDECAR = os.path.join(SPEC, "M1.index.json")

M1_HDR = re.compile(r'^####\s+`?\[(M1)-EXT-(\d+)\]')
SUB_HDR = re.compile(r'^#####\s+(.*?)\s*$')
EXTREF = re.compile(r'\[(M\d+(?:\.\d+)?)-EXT-(\d+)\]')

# ---------------------------------------------------------------------------
# Hand-authored content, keyed by "M1-EXT-NN".
# systems / math / how : REAL prose grounded in each block's source code.
# impact : preserved from the draft when present (see normalize_block).
# For no-code blocks, 'ri' is a one-line note (not fake code).
# ---------------------------------------------------------------------------
C = {
 "M1-EXT-01": dict(
   systems="Broad-phase collision, crowd/horde proximity queries, chunk-streaming load/unload, physics cooking (Jolt), and GPU culling all read the spatial hash. Writes feed [M2] physics broadphase and [M5] scent/fear diffusion grid. Seeded world layout makes cell contents deterministic per seed.",
   math="Cell key packs a uniform 2 m grid: `cx = floor(x/2)`, `cz = floor(z/2)`, `key = (cx << 32) | (cz & 0xFFFFFFFF)` in a 64-bit word (32 bits/axis -> +/-2.1e9 cells). Bucket: `bucket = hash(key) % tableSize`, `tableSize` a power of two; load factor < 0.75 with free-list overflow chains.",
   how="1. Insert: compute cell key from world (x,z), hash to bucket, push entity id onto that bucket's lock-free chain. 2. Query(radius r): iterate the (2*ceil(r/cell)+1)^2 neighbourhood of cells, collect candidates, refine with exact distance. 3. Move: on position change recompute key; if bucket changed, unlink from old chain, append to new. 4. Clear per frame for dynamic sets; persist for static (terrain) sets.",
 ),
 "M1-EXT-02": dict(
   systems="Entity registry handle table (EnTT). Every system that holds a reference to a live entity validates through this before dereferencing. Backs [M1-EXT-01] cell payloads and [M1-EXT-06] deferred mutations.",
   math="Validation is an integer bound + generation match: `valid = (h.index < generations.size()) && (h.generation == generations[h.index])`. A generation counter is bumped on free, so a recycled index with a stale generation fails the check.",
   how="1. Allocate: push a slot, store entity, set generation=0. 2. Free: bump generations[index] so any outstanding handle with the old generation is rejected. 3. Deref: call IsHandleValid; only proceed if true. Prevents 'ghost' references to recycled entities.",
 ),
 "M1-EXT-03": dict(
   systems="Multi-threaded command-buffer recording (render graph, M1). Maps (frame, thread) -> a per-thread command pool so N threads record in parallel without a global lock.",
   math="Pool index is a flat 2D->1D mapping: `poolIndex = frameResourceIndex * threadCount + threadId`, with `assert(threadId < threadCount)`. Each pool owns its own command-buffer list.",
   how="1. At frame start, allocate `threadCount` pools for that frame index. 2. Each worker thread calls GetPoolIndex with its id to fetch its exclusive pool. 3. Record commands into the private pool. 4. At submit, concatenate the per-thread pools in thread order into the final command buffer.",
 ),
 "M1-EXT-04": dict(
   systems="GPU resource lifetime management. Gates destruction of buffers/images until the GPU has finished the frame that last used them (fence-based).",
   math="Purge is a single fence comparison: `ready = completedFenceValue >= resourceFenceValue`. The resource is freed only once the completion fence has passed its submit fence.",
   how="1. On resource delete request, stamp it with the current submit fence value. 2. Each frame, compare against the completed fence from the GPU. 3. When completed >= stamped, free the backing memory. Prevents freeing mid-draw.",
 ),
 "M1-EXT-05": dict(
   systems="Per-frame CPU->GPU upload path (UI text, dynamic material tweaks, streaming). Sits on a single persistent map instead of map/unmap per upload.",
   math="Ring offset is modulo arithmetic over the pool: `offset = (frameIdx * maxFrameSize) % totalPoolSize`. Each frame owns a `maxFrameSize` window; the ring wraps when `frameIdx*maxFrameSize` exceeds `totalPoolSize`.",
   how="1. At frame start, compute this frame's region via GetFrameStagingRegion. 2. Write upload data into the persistent mapping. 3. Submit the copy; advance frameIdx. Stale frames' regions are never overwritten until the GPU has consumed them (fence-gated).",
 ),
 "M1-EXT-06": dict(
   systems="EnTT component mutation from background threads (chunk streaming, physics cooking, procedural gen). Bridges thread-local writes into the main-thread registry without locking the registry per write.",
   math="SPSC ring of capacity 1024: `full = ((writeHead+1) & 1023) == readHead`. Drain advances `readHead = (readHead+1) & 1023` per applied mutation. Cache-line aligned (alignas(64)) to avoid false sharing between producer/consumer heads.",
   how="1. Background thread builds DeferredMutation entries and PushMutation (returns false on ring full). 2. Once per main-thread tick, DrainToRegistry walks writeHead->readHead and applies each via the supplied callback. 3. Advance readHead. Keeps the entity registry mutation-free during parallel work.",
 ),
 "M1-EXT-07": dict(
   systems="Thread-local scratch allocation for pathfinding, raycast, and other per-worker algorithms. Removes malloc contention on hot parallel paths.",
   math="Linear bump inside a fixed page: `ptr = base + offset; offset += align(size, alignment); assert(offset <= capacity)`. Reset per task by restoring `offset = 0`. Alignment default 16 B; pages sized to worst-case per-task working set.",
   how="1. Each worker owns a BumpArena (memoryBufferPage + capacity + currentOffset). 2. ArenaAllocateBump returns the bumped pointer after aligning. 3. On task completion the owner resets currentOffset to 0 — zero free-list overhead, zero cross-thread traffic.",
 ),
 "M1-EXT-08": dict(
   systems="Cross-thread component visibility. Lets worker threads read entity state without the registry taking a lock, feeding [M1-EXT-01] queries and [M1-EXT-06] mutations.",
   math="Double-buffered view epoch: readers see `epoch[read]`, writers publish to `epoch[1-read]` then flip `read ^= 1` under a seqlock. Version counter prevents torn reads.",
   how="1. Worker reads the stable epoch. 2. Main thread mutates the shadow buffer. 3. Flip epoch; subsequent worker reads see the new state. No per-read lock.",
 ),
 "M1-EXT-09": dict(
   systems="Hot component pools (position, velocity, transform) cached in a contiguous, lock-friendly buffer for flocking / crowd evaluation.",
   math="Cache line aligned: `rawDataBufferMemoryHead` padded so each pool element starts on a 64 B boundary; stride = align(sizeof(T),64). Avoids false sharing when N threads write adjacent elements.",
   how="1. LockedComponentPoolCache holds a padded raw buffer. 2. Workers index elements by entity id with stride-aligned offsets. 3. Main thread compacts/defrags between frames. Sub-millisecond flocking updates under contention.",
   ri="No standalone function in source; pool is a padded `uint8_t*` buffer with `align(sizeof(T),64)` stride. See [M1-EXT-07] for the allocator it builds on.",
 ),
 "M1-EXT-10": dict(
   systems="Render-graph pass scheduler. Orders GPU passes so all inter-pass barriers are satisfied before a pass runs.",
   math="Topological order via Kahn: `topo = Kahn(N, E)`; on emit `inDeg[v]--`; stable order by pass priority when tied. Complexity O(N+E).",
   how="1. Build the pass graph (nodes = passes, edges = stage/resource dependencies). 2. Kahn with priority tiebreak yields a deterministic order. 3. Execute passes in that order; insert Vulkan barriers between dependent stages.",
 ),
 "M1-EXT-11": dict(
   systems="Explicit Vulkan barrier injection between compute (e.g. culling) and draw stages, so the GPU never reads a buffer mid-write.",
   math="Barrier spans stages: `srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT`, `dstStageMask = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT`; `barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ)`.",
   how="1. After the compute pass writes a buffer (e.g. visible-list), emit a barrier with WRITE->READ access transition. 2. The draw pass waits on dstStage before reading. 3. Per [M1-EXT-10] ordering, barriers are inserted exactly where the graph edge demands.",
 ),
 "M1-EXT-12": dict(
   systems="World-space rendering of entities whose authority lives in a moving/streaming origin (large-world double-precision root).",
   math="GPU position relative to current origin: `Position_gpu = float3(Position_authority - Origin_current)`. Keeps float precision near the camera while authority stays in `double`.",
   how="1. Each frame, compute `Origin_current` (streaming root). 2. Cast each entity's `double` authority position to `float` relative to origin via CastPositionForGPU. 3. Shaders render in origin-local space; CPU keeps `double` truth.",
 ),
 "M1-EXT-13": dict(
   systems="PARENTING — hierarchical transform propagation for weapon sockets, camera-on-head, turrets. NOTE: block is EMPTY in source spec; authored from engine convention + sibling blocks [M1-EXT-12]/[M1-EXT-41]. Needs source confirmation.",
   math="World matrix composes parent->child: `WorldMatrix_child = WorldMatrix_parent · LocalMatrix_child`, recomputed only when dirty.",
   how="1. Add a `Parent{ entt::entity }` component and a cached `WorldMatrix`. 2. PropagateDirty marks the subtree dirty on local change. 3. RecomputeWorldMatrices walks parents before children (topo) and writes each `WorldMatrix`.",
   ri="Source block empty — see [M1-EXT-41] `struct Parent { entt::entity value; }` for the component shape. Confirm against source before shipping.",
 ),
 "M1-EXT-15": dict(
   systems="GPU timestamp queries for per-pass CPU/GPU timing (Tracy-fed).",
   math="Delta from pooled timestamps: `ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start;` converted by `period` to ms.",
   how="1. Wrap each pass with timestamp begin/end writes. 2. After submit, resolve the query pool. 3. Compute `dt = (tsEnd - tsStart) * period` and forward to the profiler (Tracy).",
 ),
 "M1-EXT-16": dict(
   systems="Cache-line-padded SoA component storage for the ECS. Prevents false sharing / cache-line straddle on multithreaded component updates.",
   math="Stride padded to 64 B: `stride = align(sizeof(T), 64); base = alloc(n*stride, 64)` via `aligned_alloc(64, n*stride)`.",
   how="1. For each component type T, allocate `n` elements with 64 B stride. 2. Worker threads write their own element's cache line only. 3. Multithreaded ECS updates avoid cross-core invalidation -> smoother frame under load.",
 ),
 "M1-EXT-17": dict(
   systems="Secondary-command-buffer merging into the primary for recorded pass groups.",
   math="Merge is a stable sort: `merge(secondaries, order=[M1-EXT-13]) -> primary` (stable sort by pass id).",
   how="1. Record passes into per-pass secondary buffers. 2. Stable-sort by pass order from [M1-EXT-13]. 3. ExecuteCommandBuffers the merged list on the primary.",
 ),
 "M1-EXT-18": dict(
   systems="Draw-call batching by material to cut binding overhead.",
   math="Group draws by material id: `batch = GroupBy(materialId, draws); one bind per batch`.",
   how="1. Bin draws into material batches. 2. For each batch, bind the material once, then issue all its draws. 3. Reduces `Bind()` calls from O(draws) to O(materials).",
 ),
 "M1-EXT-19": dict(
   systems="EnTT view/group iteration for systems that touch a fixed component set.",
   math="Group is a pre-indexed intersection: `group = reg.group<Pos,Vel>()`; iteration is O(active) with components contiguous.",
   how="1. Declare the component group once. 2. Iterate `for(auto e: group) Step(e)` — EnTT hands you only entities having all group components, cache-friendly.",
 ),
 "M1-EXT-20": dict(
   systems="Skinned-mesh bone matrix upload to the GPU skinning buffer.",
   math="Bone mats written to a mapped GPU buffer: `mat4* bones = (mat4*)skbuffAddr;` stride = 64 B per bone (16 floats).",
   how="1. Compute final bone matrices on CPU. 2. Memcpy into the persistent skinning buffer at the entity's offset. 3. Vertex shader indexes `bones[boneIndex]`.",
 ),
 "M1-EXT-21": dict(
   systems="Deterministic simulationstep ordering so multiplayer/seed-replay stays bit-stable.",
   math="Order is a fixed topo over system dependencies (see [M1-EXT-10]); each step runs the same subsystem sequence every tick.",
   how="1. Define the system graph. 2. Topo-sort once at init. 3. Each tick, run systems in that order; no runtime reordering.",
 ),
 "M1-EXT-22": dict(
   systems="Render-graph resource versioning so a pass reads the correct prior write.",
   math="Each resource has a version counter bumped on write; a pass declares `(resource, version)` read/write edges consumed by [M1-EXT-10].",
   how="1. Track resource versions. 2. Edges carry version deltas. 3. The scheduler validates read-before-write and inserts barriers ([M1-EXT-11]).",
 ),
 "M1-EXT-23": dict(
   systems="Cache-line-padded SoA arrays (see [M1-EXT-16]) applied to M1 ECS storage; strided allocation avoids false sharing.",
   math="`stride = align(sizeof(T),64); base = alloc(n*stride,64)` via `aligned_alloc`. N threads write distinct cache lines.",
   how="1. Allocate padded arrays. 2. Workers index by entity with stride. 3. Multithreaded updates avoid cross-core invalidation.",
 ),
 "M1-EXT-24": dict(
   systems="Bindless descriptor-set management for the material/shader permutation space.",
   math="Per-frame bindless writes use update templates (one memcpy-shaped write) instead of N individual `WriteDescriptorSet` calls.",
   how="1. Build a descriptor update template per layout. 2. Push the whole template in one memcpy-shaped write. 3. Shader indexes resources by integer id — no per-object bind.",
 ),
 "M1-EXT-25": dict(
   systems="Persistent descriptor pool for bindless handle churn.",
   math="Pool sized to peak simultaneous handles; allocations are O(1) offsets into a fixed backing array.",
   how="1. Reserve a descriptor pool sized to peak. 2. Hand out handles as offsets. 3. Recycle on resource free; no per-frame pool recreate.",
 ),
 "M1-EXT-26": dict(
   systems="GPU-driven draw indirect argument buffer construction.",
   math="Indirect args built as a `VkDrawIndexedIndirectCommand[]` written by compute (culling) and consumed by `vkCmdDrawIndexedIndirect`.",
   how="1. Compute culling writes visible-list + indirect args. 2. Barrier ([M1-EXT-11]) before draw. 3. `vkCmdDrawIndexedIndirect` consumes the buffer — no CPU readback.",
 ),
 "M1-EXT-27": dict(
   systems="Broad-phase pair discovery between two AABBs (collision/avoidance) on top of [M1-EXT-01].",
   math="`cell = floor(p/cell)`; bucket via SpatialHash; candidate pairs = cells overlapping both AABBs. `key = SpatialHash(p)`.",
   how="1. Hash both AABB corners. 2. Collect cells overlapping both boxes. 3. Emit entity pairs whose cells intersect; narrow-phase resolves.",
 ),
 "M1-EXT-28": dict(
   systems="Hash-grid neighbour query used by scent/fear seeding ([M5]) and crowd cohesion.",
   math="`auto near = hash.QueryRadius(zombiePos, hearRadius)` returns entities in the cell neighbourhood of radius `hearRadius`.",
   how="1. Compute the query cell range. 2. Gather candidates from [M1-EXT-01]. 3. Refine by exact distance; feed the listener/agent set downstream.",
 ),
 "M1-EXT-39": dict(
   systems="Hearing model: a zombie hears a source iff no ray in the batch query hits occluding geometry AND distance is within hearing range.",
   math="Audible(listener, source) = ¬Hit(RayBatchQuery(source?listener)) ? Distance(source, listener) <= hearingRange. Uses [M1-EXT-40] batch raycast.",
   how="1. Gather candidate sources via [M1-EXT-28]. 2. Batch-cast rays source->listener. 3. If no hit and within range, the source is audible; feed the AI hearing set.",
 ),
 "M1-EXT-40": dict(
   systems="Multi-ray batch narrow-phase query against Jolt (hearing/visibility LOS). Single struct, multi-hit-per-ray.",
   math="`struct RayBatch { std::vector<JPH::RayCast> rays; std::vector<JPH::RayCastResult> hits; };` Jolt collector = multi-hit-per-ray (not N independent rays).",
   how="1. Pack rays into the batch. 2. `query.CastRay` per ray with a multi-hit collector. 3. ResolveBatch fans results to callers (AI/hearing). Replaces N naive `CastRay` calls.",
 ),
 "M1-EXT-41": dict(
   systems="Parenting component (weapon sockets, camera-on-head, turrets). See also [M1-EXT-13].",
   math="`struct Parent { entt::entity parent; glm::dmat4 local; };` world = parent.world · local (see [M1-EXT-12]).",
   how="1. Attach a Parent component. 2. On parent move, recompute child world via local offset. 3. Consumed by render (socket) and camera (head) systems.",
 ),
 "M1-EXT-42": dict(
   systems="Input action state with deadzone + curve shaping (gamepad/steam-deck).",
   math="`ActionState.value = Curve(RawAxis, Deadzone)`; `struct ActionState { bool held, pressedThisFrame; float value; };`",
   how="1. Read raw axis. 2. Apply deadzone + response curve -> value. 3. Track held/pressedThisFrame edges for consumers (movement, look).",
 ),
 "M1-EXT-43": dict(
   systems="Virtual-head camera entity (semi-implicit spring), not a fixed FlyCamera.",
   math="Semi-implicit Euler, velocity FIRST: `v += (-k*x - c*v)*dt; x += v*dt;` with `k=40, c=` damping. `camVel += (-k*camPos - c*camVel)*dt; camPos += camVel*dt;`.",
   how="1. Each frame, integrate spring velocity then position. 2. Damp to avoid overshoot. 3. Camera follows the virtual-head entity smoothly under load.",
 ),
 "M1-EXT-44": dict(
   systems="Input binding load/apply from JSON settings.",
   math="`json bindings = Load(\"settings/input.json\"); Apply(bindings);` key->action map.",
   how="1. Load the bindings file. 2. Build the key->action table. 3. Apply at input-init; live-reload on settings change.",
 ),
 "M1-EXT-45": dict(
   systems="Persistent input-binding save file (settings).",
   math="On-disk shape: `{\"version\":1,\"binds\":{}}`. Version-gated for forward/back compat.",
   how="1. Serialize the current bind map with a version field. 2. Write atomically. 3. Load path validates version before applying.",
 ),
 "M1-EXT-46": dict(
   systems="Deterministic serialization round-trip for save/network state.",
   math="`assert(RoundTrip<Velocity>(v) == v)` — pack/unpack must be bit-identical (fixed-width, LE).",
   how="1. Serialize a component to a fixed buffer. 2. Deserialize. 3. Assert equality (debug) to catch precision/layout drift that would break replay.",
 ),
 "M1-EXT-47": dict(
   systems="Settings/persistence schema versioning.",
   math="Version field on every persisted doc; migration table keyed by (from,to). `{\"version\":1,...}`.",
   how="1. Stamp writes with current version. 2. On load, run migrations up to current. 3. Reject unsupported newer versions.",
 ),
 "M1-EXT-48": dict(
   systems="Entity count ceiling for the ECS (EnTT).",
   math="EnTT cap = 2^20 = 1,048,576 entities (per issue #197). `MAX_ENTITIES = 1<<20`.",
   how="1. Size handle/version tables to the cap. 2. Reject allocation beyond it. 3. Tier-0 budgets assume well under this in practice.",
 ),
 "M1-EXT-49": dict(
   systems="Compile-time component-size assertion so SoA stride math ([M1-EXT-16]) stays valid.",
   math="`static_assert(sizeof(T) <= CACHE_LINE)` or round-trip check `assert(RoundTrip<T>(v)==v)`.",
   how="1. At registration, assert component fits the padded stride. 2. Fail-fast if a component grew past the cache-line budget.",
 ),
 "M1-EXT-50": dict(
   systems="Default/empty input-profile bootstrap.",
   math="Default file: `{\"version\":1,\"binds\":{}}` — no bindings until the user maps them.",
   how="1. On first run, write the empty profile. 2. UI fills binds. 3. Same load path as [M1-EXT-45].",
 ),
 "M1-EXT-51": dict(
   systems="Pipeline permutation cache (variant -> compiled VkPipeline).",
   math="Key->compiled-pipeline cache; compile on demand; prune unused. `Pipeline p = Variants.Get(key) ?? Compile(key)`.",
   how="1. Hash the shader/state key. 2. Return cached pipeline or compile + store. 3. Periodic prune of unreferenced entries (LRU).",
 ),
 "M1-EXT-52": dict(
   systems="Spatiotemporal blue-noise (STBN) sampling volume for denoising/shading.",
   math="Precomputed STBN volume (Heitz/Wolfe et al., EGSR 2022, jcgt). Sampled by (pixel, frame, dim) -> [0,1)^d.",
   how="1. Bake the STBN volume offline. 2. Upload once. 3. Shaders index by pixel+frame for well-distributed, temporally-coherent samples.",
 ),
 "M1-EXT-53": dict(
   systems="Cross-sector entity ownership handoff (streaming/seamless world). Single atomic transition per entity.",
   math="Ownership token: `enum class Ownership : uint32_t { Local=0, InTransfer=1 };` handoff = `compare_exchange_strong(kLocal, kInTransfer)` then mailbox push of a deterministic `TransferRecord`.",
   how="1. Source sector CAS ownership Local->InTransfer. 2. Snapshot deterministic sim state into TransferRecord. 3. Push to dest sector mailbox; dest applies on its tick. No lock, no mid-transfer read.",
 ),
}

SUB_ORDER = ["Systems Touched", "Math", "How It Works", "Reference Implementation", "Player-Facing Impact"]

def split_subheaders(seg_lines):
    """Return dict: subsection_name -> list of prose lines (split welded '##### X <prose>')."""
    subs = {}
    cur = None
    buf = []
    for l in seg_lines:
        m = SUB_HDR.match(l)
        if m:
            if cur is not None:
                subs.setdefault(cur, []).extend(buf)
            name = m.group(1).split(":")[0].strip()
            # if name has trailing prose on the same line, treat as first prose line
            rest = l[m.end():].strip()
            cur = name
            buf = [rest] if rest else []
        else:
            if cur is not None:
                buf.append(l)
    if cur is not None:
        subs.setdefault(cur, []).extend(buf)
    return subs

def extract_code(seg_lines):
    """Return list of code lines (exclude layers, headers, anchors, subsection prose,
    and existing ``` fence markers — those are re-added as one balanced pair)."""
    code = []
    for l in seg_lines:
        s = l.strip()
        if s.startswith(">") or s.startswith("<a id") or re.match(r'^#{3,5}\s', l):
            continue
        if s.startswith("```"):   # fence marker, not code
            continue
        if s == "":
            continue
        code.append(l)
    return code

def normalize_block(block_lines):
    header = block_lines[0]
    bidn = "M1-EXT-" + M1_HDR.match(header).group(2)
    layers = [l for l in block_lines if l.startswith("> **")]
    body = [l for l in block_lines if not l.startswith("> **") and not l.startswith("####") and not l.startswith("<a id")]
    subs = split_subheaders(body)
    code = extract_code(body)
    c = C.get(bidn, {})

    out = [header] + layers
    for name in SUB_ORDER:
        if name == "Reference Implementation":
            if code:
                out.append("##### Reference Implementation")
                out.append("```cpp")
                out.extend(code)
                out.append("```")
            elif "ri" in c:
                out.append("##### Reference Implementation")
                out.append(c["ri"])
            else:
                out.append("##### Reference Implementation")
                out.append("(no reference implementation present in source)")
            continue
        if name == "Player-Facing Impact":
            prose = subs.get("Player-Facing Impact")
            if prose and any(p.strip() for p in prose):
                out.append("##### Player-Facing Impact")
                out.extend([p for p in prose if p.strip()])
            elif "impact" in c:
                out.append("##### Player-Facing Impact")
                out.append(c["impact"])
            else:
                out.append("##### Player-Facing Impact")
                out.append("(impact not specified in source)")
            continue
        # Systems / Math / How It Works
        key = {"Systems Touched": "systems", "Math": "math", "How It Works": "how"}[name]
        if key in c:
            out.append("##### " + name)
            out.append(c[key])
        else:
            existing = subs.get(name)
            if existing and any(p.strip() for p in existing):
                out.append("##### " + name)
                out.extend([p for p in existing if p.strip()])
            else:
                out.append("##### " + name)
                out.append("(not specified in source)")
    return out

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", default=os.path.join(BASE, "_m1_draft.md"))
    ap.add_argument("--out", default=OUT_DEF)
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()
    if args.apply and os.path.abspath(args.out) != os.path.abspath(SRC):
        print("ERROR: --apply requires --out spec/M1.md"); return 2
    txt = open(args.inp, encoding="utf-8").read()
    lines = txt.split("\n")
    bounds = [i for i, l in enumerate(lines) if M1_HDR.match(l)]
    bounds.append(len(lines))
    prologue = lines[:bounds[0]] if bounds else []
    new = list(prologue)
    for k in range(len(bounds) - 1):
        s, e = bounds[k], bounds[k + 1]
        new.extend(normalize_block(lines[s:e]))
    # anchor pass: guarantee every M1 header preceded by its anchor
    final = []
    for l in new:
        m = M1_HDR.match(l)
        if m:
            bid = "M1-EXT-" + m.group(2)
            if not (final and final[-1] == '<a id="%s"></a>' % bid):
                final.append('<a id="%s"></a>' % bid)
        final.append(l)
    out_text = "\n".join(final)
    open(args.out, "w", encoding="utf-8").write(out_text)
    print("normalized %d blocks; wrote %s (%d lines)" % (len(bounds) - 1, args.out, len(final)))
    if args.apply and not os.path.exists(SIDECAR):
        print("WARN: sidecar missing")
    return 0

if __name__ == "__main__":
    sys.exit(main())
