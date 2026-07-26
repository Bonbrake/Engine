text = """

### [M4.5-EXT-118] `gl_GlobalInvocationID` Compute Shader Culling

> **tags** · rendering, culling, optimization, aaa
> **tl;dr** · M4.5. Moves 10,000+ entity bounding-box frustum culling entirely to the GPU via Compute Shaders, filling `VkDrawIndexedIndirectCommand` buffers.
> **ctx** · Production Engine Audit. CPU culling bottlenecks instantly during massive horde sieges.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Dispatch 1 compute thread per zombie instance (`gl_GlobalInvocationID.x`).
2. Test AABB against the 6 camera frustum planes.
3. Pass results append directly into an indirect draw buffer via `atomicAdd`.


### [M3-EXT-103] Recast/Detour Real-Time NavMesh Carving

> **tags** · ai, navmesh, destruction, pathfinding, aaa
> **tl;dr** · M3. Dynamically regenerates localized 16x16 NavMesh tiles when FEM structural failure destroys barricades or walls.
> **ctx** · Production Engine Audit. Zombies try to pathfind around destroyed walls because the NavMesh is statically baked.
> **meta** · depends-on: M3-EXT-01, M3-EXT-102

##### Implementation
1. When a CSG voxel structure completely collapses, it triggers a NavMesh invalidation callback.
2. Background thread regenerates the 16x16 Recast NavMesh tile based on the new voxel density.
3. Atomic tile hot-swap pushes updated polygons to Detour, opening the breach for zombie routing instantly.


### [M0-EXT-111] Pre-Flight PSO Background Warmup

> **tags** · rendering, shaders, caching, vulkan, aaa
> **tl;dr** · M0. Pre-warms the Vulkan `VkPipelineCache` on background threads during loading screens to ensure 99% of PSOs are ready on frame 1.
> **ctx** · Production Engine Audit. Async mesh shader compilation prevents stutter, but still drops visual quality momentarily.
> **meta** · depends-on: M0-EXT-01

##### Implementation
1. Serialize the active `VkPipelineCache` to a `.vkcache` binary file on exit.
2. On boot, `enkiTS` background tasks read the `.vkcache` and aggressively instantiate `VkPipeline` objects before the loading screen drops.


### [M6-EXT-108] Hybrid A* / Wave Packet Acoustic Propagation

> **tags** · audio, acoustics, optimization, dsp, aaa
> **tl;dr** · M6. Switches to cheap A* distance sound attenuation for distant zombies, reserving true modal wave packet DSP raytracing for entities within 50 meters.
> **ctx** · Production Engine Audit. Calculating wave packet diffraction for 10,000 entities exceeds the 200MB Audio VRAM budget.
> **meta** · depends-on: M6-EXT-01

##### Implementation
1. Entities >50m away use a simple NavMesh A* distance heuristic ($dB \propto \frac{1}{r^2}$).
2. Entities <50m invoke the GPU wave packet acoustic raytracer for true material muffling (`M6-EXT-107`) and diffraction.
"""

with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'a', encoding='utf-8') as f:
    f.write(text)
print('Appended the 4 new gaps successfully!')
