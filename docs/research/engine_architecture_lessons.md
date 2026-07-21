# Engine Architecture Lessons for ZombieEngine
## id Tech 7/8 · Decima · UE5 Nanite/Lumen
### Extracted for C++/Vulkan Open-World Zombie Survival Game

> **Purpose:** 15 concrete, actionable rendering techniques (5 per engine) that directly apply to
> ZombieEngine's rendering pipeline. Focus areas: open-world streaming, terrain rendering,
> visibility systems, LOD, GPU-driven rendering, and material systems.

---

# 1. id Tech 7/8 (DOOM Eternal / The Dark Ages)

## 1.1 Bindless Descriptor Architecture + Dynamic Draw Call Merging

**What id did:**
id Tech 7 moved to a **fully bindless Vulkan pipeline**. Every texture, buffer, and descriptor is
accessible via dynamic indexing in shaders — no per-draw descriptor binding. On top of this,
a compute shader scans all potential draw calls, groups those sharing the same shader/material
state, and **merges them into a single multi-draw indirect command** executed via
`vkCmdDrawIndexedIndirect`. DOOM Eternal renders 80-90M triangle scenes with extremely low
CPU draw-call overhead.

**ZombieEngine lesson:**
ZombieEngine already uses Vulkan + indirect drawing. Implement a **post-culling compute merge pass**:
after GPU frustum/occlusion culling writes visible instance lists, a compaction compute shader
groups surviving instances by mesh ID + material ID, then writes a compacted indirect command
buffer. This reduces CPU-side draw iteration from O(visible_instances) to O(unique_mesh_material_pairs).
Use `VK_EXT_descriptor_indexing` for bindless — eliminates descriptor set binding overhead entirely.

**Key Vulkan primitives:** `VK_EXT_descriptor_indexing`, `vkCmdDrawIndexedIndirect`,
`VK_KHR_draw_indirect_count`, storage buffers for indirect commands.

---

## 1.2 GPU Compute Skinning Pre-Pass

**What id did:**
Instead of skinning inside each vertex shader invocation (which requires the vertex shader to
know about bone matrices and weights), DOOM Eternal runs a **single compute shader pass**
before any geometry rendering. This pass reads the input skinned mesh, transforms all vertices
via their bone influences, and writes the result to a persistent GPU buffer. Downstream vertex
shaders read this buffer as if the mesh were static — they need no skinning logic.

Benefits: (a) skinning is done once per frame regardless of how many passes use the mesh
(shadow, depth, forward); (b) vertex shaders become simpler with fewer permutations;
(c) the skinned buffer can be reused for velocity-buffer computation.

**ZombieEngine lesson:**
For zombie hordes (potentially 50+ skinned characters on screen), implement a **compute-based
skinning dispatch** that runs once per frame before shadow/depth/forward passes. Use the
enkiTS task graph to schedule it as a dependency producer. Only re-skin meshes whose
animation state or bone transforms changed since last frame. Store the output as a GPU
vertex buffer in device-local memory.

---

## 1.3 Cached Shadow Maps with Static-Geometry Persistence

**What id did:**
DOOM Eternal uses a **single large 4096×8192 shadow atlas**. Static geometry's shadow map
contribution is **cached across frames** — as long as the light hasn't moved and no static object
has changed, the static portion of the shadow map persists without re-rendering. Dynamic
objects are rendered on top of the cached static buffer. Per-light atlas allocations are
dynamically sized based on screen-importance heuristics. Cascaded shadow maps for the
sun with 3×3 PCF filtering.

**ZombieEngine lesson:**
For a large open world with day/night cycle, the sun light moves slowly frame-to-frame.
Implement a **two-tier shadow cache**: (a) a persistent static shadow depth buffer that only
re-renders when the sun direction changes beyond a threshold angle; (b) dynamic-object-only
shadow rendering layered on top each frame. Combine with a **shadow atlas** that allocates
resolution proportional to light's screen-space size. For spot lights attached to the player's
flashlight/vehicle headlights, use the cached approach aggressively since these lights move
smoothly.

---

## 1.4 Visibility Buffer + Compute-Based Deferred Texturing (id Tech 8)

**What id did:**
id Tech 8 evolved from pure Forward+ (id Tech 7) to a **hybrid visibility-buffer pipeline**.
First pass renders only a tiny G-buffer — triangle ID + instance ID (4-8 bytes per pixel).
A compute shader then performs **tile classification** (analyzes which materials/shading models
are present in each 16×16 tile), followed by **compute-based material dispatch** that only
evaluates materials actually visible. Deferred texturing reads materials from the visibility buffer
and shades. The pipeline uses **software Variable-Rate Compute Shaders (VRCS)** to control
shading rate per tile — areas with low detail or motion get fewer shading samples.

**ZombieEngine lesson:**
ZombieEngine currently uses forward rendering. For a zombie survival game with complex
materials (blood, mud, decay, wetness overlays, dirt decals), consider adding a **visibility-buffer
path as an optional codepath**. Key benefits: (a) material evaluation cost is proportional to
visible pixels, not drawn triangles; (b) decals and material overlays (blood spatter, mud,
weather effects) are trivially applied in compute; (c) variable-rate shading lets you shade
distant terrain at lower rate without quality loss. Start with a thin visibility buffer
(32-bit instance ID + 32-bit triangle ID) in a compute-friendly format.

---

## 1.5 Sector Streaming + Auto Vista LOD Generation (id Tech 8)

**What id did:**
id Tech 8 introduced **sector streaming**: the world is divided into sectors/cells. The engine
predicts player movement and proactively streams content — geometry, textures, animations —
via a **world geometry manager**. Combined with DirectStorage (Xbox) and intelligent
compression, this achieves sub-second level loads. **Auto Vista LOD generation** automatically
creates distant LOD representations of fully-built play spaces (castles, structures, terrain
features) without artists hand-crafting skybox geometry. LOD transitions are imperceptible
due to blending.

**ZombieEngine lesson:**
For ZombieEngine's open world, implement a **spatial sector system** where each sector stores
its own dedicated geometry, texture, and instance data as independent GPU upload batches.
Use the enkiTS task system to **prefetch sectors within a prediction radius** asynchronously
while the player moves. For vista/terrain LOD, generate **automated impostor geometry**:
render distant sectors to a cubemap/texture array from multiple angles at build time, then
display as view-aligned billboards beyond a distance threshold. Combine with distance-based
material simplification (remove normal maps, reduce specular evaluations far away).

---

# 2. Decima Engine (Horizon Zero Dawn / Death Stranding)

## 2.1 GPU Compute-Based Runtime Procedural Placement

**What Decima did:**
Horizon Zero Dawn's world is built by a **GPU compute shader placement system** that runs at
runtime. Artists define procedural rules in a **graph editor** — inputs like height, slope angle,
biome mask, distance-to-road, and exclusion zones feed into evaluation graphs that output
what to place (trees, rocks, grass, sound sources, even gameplay elements). The compute
shader evaluates these graphs for each tile near the player, populating the world on-the-fly.
Placement is **deterministic per seed**, so it's stable across visits. The system handles millions
of instances and covers everything from vegetation to wildlife spawn points.

**ZombieEngine lesson:**
For a zombie survival open world with forested areas, ruined cities, and varied biomes,
implement a **GPU compute placement system** that runs on a background queue. Artists define
placement rules: "place destroyed cars near road splines at 40-60% density," "place zombie
horde spawners in urban tiles at night with 30% probability," "place debris piles near building
walls." Compute evaluates tile-bound AABBs against the graph. Output is a GPU instance buffer
consumed directly by the indirect draw pipeline. This removes CPU-side culling bottlenecks for
the millions of small objects (rocks, grass, debris) that define a post-apocalyptic world.

---

## 2.2 GPU Compute Visibility Queries with Wavefront Batching

**What Decima did:**
Decima's visibility system runs **entirely on compute shaders** on PS4's async compute.
It handles **500K–1.5M static instances** per query. The key innovation is **wavefront batching**:
within a GPU wavefront (64 threads on GCN), instances that share the same DrawableSetup
(mesh + material) are automatically batched — each batch writes one entry instead of 64,
yielding up to **64× reduction in output draw commands**. The system uses a spatial grid
tile structure to accelerate queries, culling against both camera frustum and shadow frusta.
CPU-side SIMD fallback exists for older codepaths.

**ZombieEngine lesson:**
Implement a **compute-based hierarchical visibility query** for static geometry (buildings,
terrain, debris, vegetation). Use a fixed-grid spatial index stored in GPU buffers. Each frame:
1. Dispatch compute shader that traverses grid cells against combined frustum.
2. Within each cell, test instance bounding boxes against frustum + HiZ occlusion.
3. **Batch surviving instances** per wavefront that share mesh+material — write compacted
   batch entries instead of per-instance entries.
4. Feed directly to indirect draw pipeline.
Since ZombieEngine uses enkiTS on CPU, run this as a GPU compute graph node before the
main rendering dispatch. The batching is particularly valuable for zombie horde rendering
where hundreds of instances share the same zombie mesh with different animation frames.

---

## 2.3 Three-Tier Terrain Rendering with Dynamic Clod Switching

**What Decima did:**
Decima renders terrain with **three distinct data representations**, each with optimized LOD
stages: (1) **Heightmap-based tessellated patches** for rolling terrain and plains; (2) **Voxel/cliff
representations** for vertical rock formations and canyon walls; (3) **Procedural instancing**
for surface detail (grass, small rocks). These switch dynamically based on viewing distance.
Death Stranding 2 handles scenes with **25M terrain polygons** at stable framerate. The terrain
system integrates with the procedural placement system so that vegetation and debris
naturally conform to the terrain surface.

**ZombieEngine lesson:**
For ZombieEngine's post-apocalyptic terrain (ruined urban areas, forested hills, underground
tunnels), implement a **multi-representation terrain renderer**:
- **Primary: GPU-tessellated heightmap patches** for base terrain (large open areas).
- **Secondary: Chunked mesh-cluster LOD** for complex man-made structures overlaying
  terrain (ruined highways, building foundations, crater rims).
- **Tertiary: GPU-instanced detail** for grass, rubble, small rocks integrated with the
  procedural placement system.
Use a **geometry clipmap** or **CDLOD** quadtree for the heightmap patches, but transition
to mesh clusters near structures where heightmaps are insufficient. Share the same visibility
query system across all three representations.

---

## 2.4 GPU-Based Dynamic Grass/Foliage Rendering

**What Decima did:**
Decima renders vegetation as **GPU-computed geometry**. Grass blades are generated by
compute shaders that evaluate blade position, bend, and wind deformation per-instance.
The system handles millions of grass blades and small plants with **wind animation driven
by a global wind force field** that artists can paint (GDC 2018 talk: "Between Tech and Art:
The Vegetation of Horizon Zero Dawn"). Translucency and sub-surface scattering are
approximated via specialized foliage shading. LOD for vegetation uses billboard transition
at distance.

**ZombieEngine lesson:**
ZombieEngine should implement **GPU-generated grass and foliage** for the post-apocalyptic
overgrowth aesthetic. Use a compute shader that takes a density map and generates
individual blade quads with per-blade wind phase, bend, and color variation. **Wind simulation**
should be a global compute pass affecting all vegetation (grass, tree leaves, wheat fields)
unified under a single wind field system. For the zombie game's atmosphere, add a
"disturbance" channel: zombies or the player moving through grass pushes blades aside
via a per-frame disturbance texture rendered from a top-down depth pass.

---

## 2.5 Nubis Volumetric Cloud System + Atmospheric Scattering

**What Decima did:**
Decima's **Nubis cloud system** (SIGGRAPH 2017) renders regional-scale volumetric clouds
as a gameplay element — not just a skybox. Clouds are authored as 3D density fields with
artist-defined shapes, rendered via ray-marching through a 3D texture. The system handles
**weather as a gameplay mechanic**: Death Stranding's timefall (rain) interacts with the cloud
system. Combined with **practical atmospheric scattering** with height-based fog that provides
depth cues across Horizon's vast draw distances. The system runs on compute shaders and
is temporally stable.

**ZombieEngine lesson:**
For an open-world zombie game, volumetric weather is a powerful atmospheric tool. Implement
a **simplified Nubis-style cloud system** using compute shaders and a 3D weather texture that
evolves over time. Clouds cast volumetric shadows that affect the world below (darker
overcast = zombies behave differently). Combine with **height-based atmospheric scattering**
to handle the long view distances of open-world terrain. The scattering can reuse the same
depth buffer as the main render. Use temporal reprojection to reduce ray-march samples
(4-8 samples/pixel with temporal reuse → 32-64 sample quality).

---

# 3. UE5 Nanite + Lumen

## 3.1 Hierarchical Cluster-Based Virtualized Geometry (Group-Decimate-Split)

**What Nanite does:**
Nanite decimates each mesh into a **hierarchical DAG of clusters**, each containing ~128 triangles.
Each cluster has two versions: high-res (128 tris) and decimated low-res (64 tris). The key
innovation is **Group-Decimate-Split**: the system merges 2N clusters, decimates them together
(including shared borders), then splits back. This avoids the "dense border" problem of naive
recursive cluster decimation. The result is a tree where every node's internal vertices are
decimated but boundaries remain watertight — **no cracks ever**. The GPU selects the "cut"
through this tree per-frame based on screen-space projected error. A software rasterizer
handles clusters whose triangles are sub-pixel size.

**ZombieEngine lesson:**
For ZombieEngine's open-world buildings, ruins, and terrain features, implement a
**simplified cluster-based LOD system** for hero static meshes. Pre-process meshes offline
to generate a cluster hierarchy:
1. Cluster input mesh into ~128-triangle groups.
2. Decimate each cluster's interior to ~64 tris (preserve borders).
3. Merge adjacent clusters into parent groups and repeat.
At runtime, the GPU evaluates each cluster's screen-space error and selects the appropriate
LOD level per cluster, not per-object. This eliminates LOD pop because the transition is
per-cluster (30-50 triangles changing at a time = imperceptible). Start with this for large
structures (buildings, bridges, terrain overlays) where LOD popping is most visible.

**Key difference from Nanite:** You don't need the full software rasterizer immediately — start
with hardware rasterization and cluster-level LOD selection. Add software fallback later if
sub-pixel triangles become a bottleneck.

---

## 3.2 GPU-Driven Two-Pass Hi-Z Occlusion Culling with LOD Integration

**What Nanite does:**
Nanite occlusion culling is a **two-pass GPU process** integrated with LOD selection:
- **Pass 1 (Occlusion testing):** Test instance bounding boxes against the **previous frame's
  Hierarchical Z-Buffer** (Hi-Z). This is conservative — some occluded instances pass
  (temporally stable). Use the Hi-Z mip chain so each test is a constant-time 4-texel lookup
  for small objects.
- **Pass 2 (Render):** The surviving instances are rendered with their appropriate LOD clusters.
  During rendering, the **Simplify stage** re-evaluates occlusions at cluster granularity —
  individual clusters behind others are culled even within a visible instance.
- LOD selection and occlusion culling share the same error metric: a cluster is dropped if
  its projected error is below a threshold OR if it's occluded.

**ZombieEngine lesson:**
Implement a **GPU Hi-Z occlusion culling pipeline** that runs before the main draw:
1. Render a **depth-only prepass** (existing in ZombieEngine) → generate Hi-Z mip chain
   via compute shader (single-pass mip generation).
2. Compute shader: frustum-cull instance bounding boxes → test against Hi-Z of previous
   frame → write surviving instance IDs to indirect buffer.
3. Main render consumes indirect buffer. For particularly dense areas (zombie horde inside
   a building), add a second Hi-Z test after the prepass of the current frame for a tighter
   cull on the forward pass.
Integrate with the cluster LOD from lesson 3.1: the culling shader also selects cluster level
based on distance and writes cluster LOD index into the indirect command.

---

## 3.3 Virtual Streaming via Page-Based Cluster Storage

**What Nanite does:**
All Nanite cluster data is stored in **128KB pages** that are **virtually streamed** from disk
to GPU memory. Pages are allocated based on **spatial locality + LOD level** — clusters near
each other in space and at similar LOD levels share a page. The **first page always resident**
contains the top LOD levels (coarsest representation), guaranteeing something is always
renderable. A **feedback system** from the simplify stage tracks which pages are needed next
frame and issues streaming requests. On-disk compression uses standard LZ; in-memory
uses bitstream compression (variable-bit encoding per-cluster for positions, normals, UVs).

**ZombieEngine lesson:**
For ZombieEngine's open world, implement a **page-based geometry streaming system**:
- Partition world geometry into spatial pages (128KB-256KB each).
- Assign each page a LOD range and always keep the coarsest representation loaded.
- Track per-frame which pages are touched by the visibility query → issue async streaming
  requests via a dedicated I/O thread (using enkiTS).
- Use **sparse residency** (`VK_EXT_memory_budget` + `VK_EXT_pageable_device_local_memory`)
  to allow GPU memory oversubscription.
- For in-memory compression, use a lightweight bitstream codec (like meshoptimizer or
  custom cluster quantization) to reduce memory footprint by 2-3×.
The key metric: the game should never stall for geometry streaming. Always have the coarse
LOD resident, and stream in higher detail asynchronously.

---

## 3.4 Virtual Shadow Maps (Sparse Page-Atlas Shadows)

**What Nanite does:**
Nanite uses a **single 16384×16384 virtual shadow texture per light**, implemented via
**sparse paging**. Each frame, the system marks which mip levels of which tiles are needed
(one shadow texel per screen pixel). If a tile hasn't changed (light/view still) and is already
populated, it's kept. Otherwise, a render view for just that mip+tile is requested as one of
Nanite's multi-views — rendered with full LOD and occlusion culling. Since the texture is
virtual, the full map would be 1GB at 4-byte depth — but sparse paging means only visible
tiles are physically allocated. A screen-space trace step reconciles LOD differences between
the viewer's perspective and the light's perspective.

**ZombieEngine lesson:**
For the large open-world zombie game with dynamic day/night cycle, implement a
**virtual shadow map** using Vulkan sparse bindings:
- Allocate a large logical shadow texture (16384² or 8192² per relevant light) as a sparse
  resource.
- Each frame, compute a **visibility bitmask** of which tiles are needed (project shadow-map
  pixels to screen space, mark required mip).
- Only render into tiles that are (a) newly needed or (b) invalidated by light/mesh movement.
- Keep a persistent cache of previously rendered tiles.
This dramatically reduces shadow rendering cost for the sun and large light sources compared
to rendering full cascades every frame. For a game with dynamic lighting (player's flashlight,
explosions, burning buildings), VSMs scale naturally — each light only pays for visible shadow
texels.

---

## 3.5 Nanite Software Rasterizer + Doubly-Deferred Shading

**What Nanite does:**
When clusters become small enough (< ~64 pixels on screen), Nanite switches from hardware
rasterization to a **software rasterizer**. This is a compute shader that iterates over each
cluster's triangles in a wavefront, performs 2D bounding-box rasterization, and writes to the
visibility buffer. The software path avoids the GPU's fixed-function triangle setup overhead
for tiny triangles (which is the bottleneck at millions of sub-pixel triangles). **Doubly-deferred
shading** then groups visible pixels by material ID per tile — each tile evaluates each
visible material exactly once, amortizing material evaluation over many pixels sharing the
same material.

**ZombieEngine lesson:**
For ZombieEngine's dense ruined-city environments with high triangle counts, implement a
**software rasterization path for far-distance geometry**:
- When a cluster's projected screen area is < 64 pixels, dispatch a compute shader
  that software-rasterizes its triangles directly into a visibility buffer.
- Follow with a **deferred material evaluation** compute shader that groups pixels per
  16×16 tile by material ID, evaluating each material once per tile and writing the result
  to all pixels of that material.
This saves the per-triangle hardware setup cost that dominates at large triangle counts.
For the zombie game, this is particularly valuable for rendering the dense urban environment
(broken buildings, debris fields) where far-distant geometry would otherwise create millions
of sub-pixel triangles.

---

# Summary: Priority for ZombieEngine

| Priority | Technique | Engine | Impact Area |
|----------|-----------|--------|-------------|
| P0 | Compute-based visibility + HiZ occlusion | Decima + Nanite | Core rendering |
| P0 | Bindless + indirect draw merging | id Tech 7 | CPU draw-call elimination |
| P1 | GPU compute skinning | id Tech 7 | Zombie horde rendering |
| P1 | Cluster-based LOD (simplified Nanite) | Nanite | Pop-free LOD |
| P1 | GPU procedural placement | Decima | World detail automation |
| P1 | Cached shadow maps | id Tech 7 | Shadow cost reduction |
| P2 | Sector streaming + prediction | id Tech 8 | Open-world streaming |
| P2 | Visibility buffer + deferred texturing | id Tech 8 | Complex material handling |
| P2 | Wind + foliage GPU compute | Decima | Atmosphere / overgrowth |
| P2 | Virtual shadow maps | Nanite | Scalable dynamic shadows |
| P3 | Software rasterizer for far geometry | Nanite | Ultra-dense geometry |
| P3 | Volumetric clouds (Nubis-style) | Decima | Atmosphere / weather |
| P3 | Doubly-deferred shading | Nanite | Material evaluation cost |
| P3 | Three-tier terrain renderer | Decima | Varied terrain types |
| P3 | Auto vista LOD | id Tech 8 | Long-distance vistas |

---

# References

1. Simon Coenen, "DOOM Eternal Graphics Study" (2020)
2. Jean Geffroy et al., "Rendering the Hellscape of Doom Eternal" — SIGGRAPH 2020 Advances in Real-Time Rendering
3. Philip Hammer & Dominik Lazarek, "Rip & Tear: Breaking Down the Rendering of DOOM: The Dark Ages" — GDC 2026
4. Digital Foundry, "Creating Doom: The Dark Ages — ray tracing, load times and optimisation in id Tech 8" (2025)
5. NVIDIA, "How id Software Used Neural Rendering and Path Tracing in DOOM: The Dark Ages" (2025)
6. Guerrilla Games, "Decima Engine: Visibility in Horizon Zero Dawn" — GDC 2017
7. Guerrilla Games, "GPU-Based Run-Time Procedural Placement in Horizon: Zero Dawn" — GDC 2017
8. Guerrilla Games, "The Vegetation of Horizon Zero Dawn: Between Tech and Art" — GDC 2018
9. Giliam de Carpentier & Kohei Ishiyama, "Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine" — SIGGRAPH 2017
10. Brian Karis et al., "A Deep Dive into Nanite Virtualized Geometry" — SIGGRAPH 2021 Advances in Real-Time Rendering
11. Epic Games, "Understanding Nanite — Unreal Engine 5's Virtualized Geometry System" (2021)
12. Luther Tychonievich, "Streaming, Level of Detail, and Occlusion" — UIUC CS 418
