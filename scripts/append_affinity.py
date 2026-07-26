text = """

### [M1-EXT-101] Topology-Aware Thread Affinity & Hybrid CPU Scheduling

> **tags** · ecs, threading, performance, aaa
> **tl;dr** · M1. Pins high-priority enkiTS jobs (Physics, Rendering, ECS) to CPU P-cores / 3D V-Cache, offloading background streaming to E-cores to eliminate micro-stuttering.
> **ctx** · AAA Engine Standard. Un-pinned multi-threading on modern hybrid CPUs causes L3 cache thrashing and up to 40% frame latency spikes.
> **meta** · depends-on: M1-EXT-01

##### Implementation
1. On engine startup, queries logical processor topology via `GetLogicalProcessorInformationEx(RelationProcessorCore)`.
2. Identifies Performance Cores (P-cores) vs Efficiency Cores (E-cores) and NUMA L3 cache nodes.
3. Configures enkiTS custom thread pool pinning:
   - **Pool A (Main/Render/Physics):** Bound strictly to P-core logical IDs via `SetThreadSelectedCpuSets`.
   - **Pool B (Background Streaming/IO/Defrag):** Bound strictly to E-core logical IDs.
4. Prevents cross-CCD L3 cache thrashing on AMD Ryzen 3D V-Cache processors by pinning render threads to the V-Cache CCD.
"""

with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'a', encoding='utf-8') as f:
    f.write(text)
print('Appended Topology-Aware Thread Affinity block via script!')
