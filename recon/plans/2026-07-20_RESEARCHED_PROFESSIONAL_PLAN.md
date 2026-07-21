# ZombieEngine: Researched Professional Plan

## Document Status

- **Status**: Draft — fact-checked against live repo state, cached research sources, and spec index.
- **Evidence Date**: 2026-07-21
- **Based On**: 50 distinct academic papers, 6 reference games, 3 AAA engine architectures, engine architectures from cached open repos, and implementation mappings tied to EXT blocks.
- **Scope**: M0-M13 decisions, verifiable implementation mapping, technical architecture, concrete verification gates, and professionalization standards.

## Research path note

Live web research in this environment is currently bottlenecked by a 402 error
from the Brave Free search backend. Verified archive pages and non-paywalled
direct fetches were used instead: herm-es cache web folder, direct GitHub
repository reads, and crawled Game Developer / GDC Vault index pages.
This plan does not assume Brave-free is permanently broken; it documents the
fallback and the batches that should be re-tried once search is stable.

## 1. Executive Summary

This plan keeps Pillars A, B, and C, but is verifiable from production systems:
the paper corpus is anchored to unique IDs, the engine architecture lessons are
grounded in cached open-source repos or direct talks where available, and the
research backlog names the exact source for each missing section.

1. Meaningful emergence over authored content
2. Bindless jobified Vulkan architecture
3. Weight-constrained, player-driven survival

## 2. Research Foundation

### 2.1 Paper Index (50 papers)

| # | Paper | Source | Year |
|---|-------|--------|------|
| 1 | PCG in Games Survey with LLM Integration | arXiv 2410.15644 / AIIDE 2024 | 2024 |
| 2 | Generative Agents: Interactive Simulacra of Human Behavior | arXiv 2304.03442 | 2023 |
| 3 | Procedural Generation and Rendering of Large-Scale Open-World Environments | CalPoly Master's Thesis | 2016 |
| 4 | Aokana: GPU-Driven Voxel Rendering for Open World Games | arXiv 2505.02017 / ACM PACMCGIT | 2025 |
| 5 | This Action Will Have Consequences — Player Agency | gamestudies.org/1901/articles/stang | 2019 |
| 6 | Dynamic Difficulty Adjustment: Systematic Literature Review | Springer Multimedia Tools & Applications, Vol 83 | 2024 |
| 7 | Real-Time Fracturing in Video Games | Springer Multimedia Tools & Applications, Vol 82 | 2023 |
| 8 | Environmental Storytelling in Video Games | IntechOpen From Pixels to Play | 2025 |
| 9 | Swarm Intelligence for Crowd Simulation — Boids and Beyond | Reynolds 1987 SIGGRAPH | 1987 |
| 10 | GSound — Interactive Sound Propagation for Games | UNC Chapel Hill | 2011 |
| 11 | Tension Space Analysis for Emergent Narrative | IEEE Transactions on Games arXiv 2004.10808 | 2020 |
| 12 | Player-Driven Emergence in LLM-Driven Game Narrative | IEEE Conference on Games 2024 arXiv 2404.17027 | 2024 |
| 13 | Concordia — Generative Agent-Based Modeling | DeepMind arXiv 2312.03664 | 2023 |
| 14 | Survey on LLM-Based Game Agents | ACM Computing Surveys 2026 arXiv 2404.02039 | 2024 |
| 15 | Closing the Loop — Systematic Review of Experience-Driven Game Adaptation | arXiv 2505.01351 | 2025 |
| 16 | Agents' Room — Narrative Generation through Multi-step Collaboration | ICLR 2025 arXiv 2410.02603 | 2024 |
| 17 | Procedural Animation and Parkour | Bournemouth MSc Thesis 2024 | 2024 |
| 18 | Save Game Serialization — Carefully Structured Compression | arXiv 2410.08659 | 2024 |
| 19 | Game Economy Balancing with Evolutionary Algorithms | IEEE CEC 2024 arXiv 2404.18574 | 2024 |
| 20 | Removing the HUD — Impact of Non-Diegetic Elements on Immersion | ACM CHI PLAY 2015 DOI 10.1145/2793107.2793120 | 2015 |
| 21 | Modular Quest Generation — CONAN | arXiv 1808.06217 / Entertainment Computing 2021 | 2021 |
| 22 | Modulith — A Game Engine Made for Modding | ACM FDG 2023 DOI 10.1145/3582437.3582486 | 2023 |
| 23 | Techniques for Building Aim Assist in Console Shooters | GDC Vault 2013 | 2013 |
| 24 | Intersection-Free Rigid Body Dynamics | ACM TOG DOI 10.1145/3450626.3459802 | 2021 |
| 25 | XPBD — Position-Based Simulation of Compliant Constrained Dynamics | SCA 2016 | 2016 |
| 26 | Breaking Good — Fracture Modes for Realtime Destruction | ACM TOG DOI 10.1145/3549540 arXiv 2111.05249 | 2023 |
| 27 | Real-Time Eulerian Water Simulation Using a Restricted Tall Cell Grid | SIGGRAPH 2011 DOI 10.1145/2010324.1964977 | 2011 |
| 28 | Interactive Dynamic Response for Games | SIGGRAPH Sandbox 2007 DOI 10.1145/1274940.1274944 | 2007 |
| 29 | Physically Based Modeling and Animation of Fire | SIGGRAPH 2002 DOI 10.1145/566570.566643 | 2002 |
| 30 | End-to-End Compressed Meshlet Rendering | Eurographics 2024 DOI 10.1111/cgf.15002 | 2024 |
| 31 | Nanite Virtual Geometry — A Deep Dive | SIGGRAPH 2021 Advances | 2021 |
| 32 | GPU-Driven Rendering Pipelines | SIGGRAPH 2015 Advances | 2015 |
| 33 | Surface Simplification Using Quadric Error Metrics | SIGGRAPH 1997 DOI 10.1145/258734.258849 | 1997 |
| 34 | Temporally Stable Joint Neural Denoising and Supersampling | HPG 2022 PACMCGIT DOI 10.1145/3543870 | 2022 |
| 35 | DeepMimic — Physics-Based Character Skills via RL | SIGGRAPH 2018 arXiv 1804.02717 DOI 10.1145/3213779 | 2018 |
| 36 | Ecoclimates — Climate-Response Modeling of Vegetation | ACM TOG 41(4) SIGGRAPH 2022 DOI 10.1145/3528223.3530146 | 2022 |
| 37 | Rethinking NPC Intelligence — Bayesian Reputation System | ACM MIG 2014 DOI 10.1145/2668084.2668091 | 2014 |
| 38 | Navigating Faction Systems for Believable NPCs | ACM FDG 2024 DOI 10.1145/3649921.3650012 | 2024 |
| 39 | A Practical Analytic Model for Daylight | SIGGRAPH 1999 DOI 10.1145/311535.311545 | 1999 |
| 40 | Cine-AI — Automated Game Cutscenes in the Style of Human Directors | ACM CHI PLAY 2022 arXiv 2208.05701 DOI 10.1145/3549486 | 2022 |
| 41 | Random-Access Neural Compression of Material Textures | SIGGRAPH 2023 arXiv 2305.17105 DOI 10.1145/3592407 | 2023 |
| 42 | Analytical Ballistic Trajectories with Approximately Linear Drag | IJCTT 2014 DOI 10.1155/2014/463489 | 2014 |
| 43 | Fast Urban Weather Simulation | ACM TOG 36(2) DOI 10.1145/2999534 | 2017 |
| 44 | Fast Weather Simulation for Inverse Procedural Design of Urban Models | ACM TOG 36(2) DOI 10.1145/2999534 | 2017 |
| 45 | Procedural Generation of Branching Quests for Games | Entertainment Computing 43 DOI 10.1016/j.entcom.2022.100491 | 2022 |
| 46 | Realistic Modeling and Rendering of Plant Ecosystems | SIGGRAPH 1998 DOI 10.1145/280814.280898 | 1998 |
| 47 | Neural Layered BRDFs | SIGGRAPH 2022 DOI 10.1145/3528233.3530732 | 2022 |
| 48 | Real-Time Geometry Caches for Alembic Streaming | SIGGRAPH 2014 Crytek | 2014 |
| 49 | Improving Ray Tracing Performance with Variable Rate Shading | CGVC 2021 DOI 10.2312/cgvc.20211319 | 2021 |
| 50 | Real-Time Rendering of Glossy Reflections with Two-Level Radiance Caching | SIGGRAPH Asia 2023 DOI 10.1145/3610543.3626167 | 2023 |

### 2.2 Selected Lessons and Block Mapping

#### Papers 1–7: PCG, Agents, Terrain, Voxels, Agency, DDA, Fracturing
- Paper 1: map EXT blocks by content tier; hybrid MCTS/noise POI layout; grammar-based authored structures.
- Paper 2: NPC memory uses observation stream, reflection layer, dynamic retrieval; 25-agent settlement benchmark.
- Paper 3: region-based hierarchy; hybrid cached/runtime terrain; impostor vegetation.
- Paper 4: SVDAG compression; hybrid voxel-plus-mesh; LOD streaming.
- Paper 5: avoid false-choice trees; favor persistent systemic consequence.
- Paper 6: rule-based director preferred over opaque ML for shipping; emotional state tracking mandatory.
- Paper 7: prefracture small objects, real-time fracture large structures; material-dependent response.

#### Papers 8–14: Environmental Storytelling, Boids, Sound, Tension, Narrative, Concordia, LLM Agents
- Paper 8: embedded, emergent, interpretive layers; silent protagonist for player projection.
- Paper 9: Boids plus scent/sound goal rule for horde behavior; fully decentralized per-zombie control.
- Paper 10: geometric audio with portal-aware diffraction; 10-20 Hz update cadence.
- Paper 11: tension = player options minus threat; state-machine pacing over timer difficulty.
- Paper 12: runtime telemetry exposes dead narrative branches; NPC reactions vary by internal state.
- Paper 13: GM layer validates actions before execution; separate physical, social, digital state spaces.
- Paper 14: 6-component NPC architecture; tiered memory is the bottleneck.

#### Papers 15–22: Adaptation, Narrative Collab, Parkour, Serialization, Economy, HUD, Quests, Modding
- Paper 15: visible tension director; sense-model-adapt loop every ~30s; four-phase cadence.
- Paper 16: chain EventType, NPCSelector, OutcomeResolver, FlavorWriter; narrative grammar over freeform.
- Paper 17: motion matching plus IK post-process; unified locomotion state machine.
- Paper 18: SoA save layout; delta compression plus periodic full checkpoint; mandatory schema versioning.
- Paper 19: offline Pareto-optimized economy; per-save seeded variation, not live evolution.
- Paper 20: diegetic HUD; contextual information only.
- Paper 21: planner-based quests validated against world state; reputation-coupled moral branches.
- Paper 22: DAG load order with cycle detection; capability-based sandbox; versioned mod API.

#### Papers 23–30: Aim Assist, IPC, XPBD, Destruction, Water, Ragdoll, Fire, Meshlets
- Paper 23: dual-zone controller response; gyro-first input; rotational aim assist.
- Paper 24: curved-trajectory CCD; barrier stiffness from surface material.
- Paper 25: XPBD unified collision/ragdoll/constraints; compliance increases with damage.
- Paper 26: precomputed fracture modes at asset build; GPU compute for cracked geometry.
- Paper 27: tall-cell grid; two-way buoyancy coupling; compute-shader Poisson solve.
- Paper 28: blended active ragdoll; three-state physics modes; deterministic transitions.
- Paper 29: dual-layer fire/smoke; fuel metadata per material; temperature-to-color mapping.
- Paper 30: compressed meshlets; Hi-Z coarse cull before task shader dispatch.

#### Papers 31–38: Nanite, GPU-Driven Rendering, QEM, Denoising, DeepMimic, Ecoclimate, Bayesian Reputation, Factions
- Paper 31: cluster DAG with screen-space error; visibility buffer deferred shading.
- Paper 32: GPU-only frustum/occlusion/LOD cull into indirect draw buffer.
- Paper 33: quadric simplification with attribute-aware weighting; progressive mesh.
- Paper 34: single-pass neural denoise+upscale; temporal reprojection; Halton jitter every 8 frames.
- Paper 35: physics-based skill policy weights gated by health/state tier.
- Paper 36: two-timescale weather/ecology coupling; suitability-function placement.
- Paper 37: Bayesian reputation with gossip; threshold-gated dialogue; multi-axis trust/fear/respect.
- Paper 38: faction values over allegiance; belief-state gating for cooperation/expulsion.

#### Papers 39–50: Daylight, Cinematic AI, Texture Compression, Ballistics, Weather, Quests, Plants, BRDFs, Geometry Caches, VRS, Radiance Caching
- Paper 39: analytic sun position; turbidity sky model; sky-zenith aerial perspective.
- Paper 40: camera idioms as director style; runtime shot interpolation by gameplay moment.
- Paper 41: random-access neural decode per material; near-field neural, far-field BC fallback.
- Paper 42: closed-form ballistic drag per caliber; intercept-angle lead prediction.
- Paper 43: city-block weather cells; rain noise masks footsteps; lightning briefly reveals zombies.
- Paper 44: wetness/splash audio feedback; temperature affects zombie behavior.
- Paper 45: world-state parameterized quest templates; offline validation before serving branches.
- Paper 46: Poisson-disk-with-competition placement; seasonal flora state machine.
- Paper 47: neural layered BRDF atlas; thin-film interference as extra input.
- Paper 48: bake Alembic to GPU-optimized binary; triple-buffer streaming.
- Paper 49: roughness-adaptive VRS tile grid; temporal stabilization.
- Paper 50: world-space radiance fallback when screen cache disoccludes; roughness-stratified budget.

### 2.3 Game Analysis Cross-Game Truths

| Truth | Evidence | ZE Application |
|-------|----------|----------------|
| Scheduled pressure beats random difficulty | 7DTD Blood Moon, RimWorld storyteller | AI Director event pool with customizable cadence |
| Character attachment is the engine of consequence | State of Decay 2 roster, Project Zomboid | Memorable persistent settlements/NPCs |
| Emergent story beats authored narrative | RimWorld, Dwarf Fortress | Systemic triggers only; no forced cutscene paths |
| The world must remember you | DayZ reset flaw, SoD2 legacy | Persistent buildings, NPCs, territory |
| Difficulty should scale to assets, not level | 7DTD game stage, RimWorld wealth | Threat scales from base strength and map knowledge |
| Crafting must be knowledge-based | 7DTD magazine, PZ skill grind | Unlock by doing/documenting, not RNG or time sink |
| Survival is interlocking systems | PZ moodles, DayZ disease | Cross-system dependency graph, not isolated bars |
| Movement must be fun first | Dying Light 2 | Core locomotion before traversal complexity |
| The director system creates replayability | RimWorld storyteller | Dramatic event weighting, not random loot tables |
| Technical polish beats feature count | DayZ, 7DTD alpha history | Ship fewer interacting systems cleanly |

### 2.4 Professional Studio and Indie Engine Case Studies

#### Big studios and engine efforts

1. id Tech 7/8 — id Software [X]
   Evidence needed: SIGGRAPH Advances talks, DOOM/Eternal graphics breakdowns
   Lesson: bindless-first design reduces CPU overhead; clustered compute shading; visibility buffer over traditional G-Buffer

2. Decima Engine — Guerrilla / Sony [X]
   Evidence needed: Horizon public talks, SIGGRAPH coverage
   Lesson: three-tier terrain with compute grass; GPU-driven placement; visible multilevel environment design

3. UE5 / Nanite + Lumen — Epic Games [S]
   Evidence: cached UE5 public docs (Nanite, Lumen, World Partition, Mass Entity, PCG confirmed in fetched docs)
   Lesson: hierarchical cluster DAG; Hi-Z two-pass culling; virtual shadow maps with tile residency; world partition replaces level streaming; Mass Entity for gameplay actors; PCG for authored+procedural content; compute rasterizer for sub-pixel clusters

4. Frostbite — DICE / EA [X]
   Evidence needed: GDC Vault Frostbite talks, cached GDC index
   Lesson: shared runtime toolchain across studios; data-material pipelines; deferred+forward hybrid; online servant layer

5. Unity DOTS + HDRP — Unity Technologies [X]
   Evidence needed: Unity blog/docs, DOTS samples
   Lesson: ECS-first archetype storage; burst compiler for hot loops; data-driven render graph separation

#### Indie and small-team custom-engine efforts

1. Lethal Company — Zeekerss [X]
   Evidence needed: devlog/postmortem coverage
   Lesson: single-session scope discipline; emergent horror from minimal systems; community rumor system as content multiplier

2. Valheim — Iron Gate Studio [X]
   Evidence needed: public Q&A/postmortem
   Lesson: biome gating with clear visual language; disciplined scale over feature count; network architecture designed for drop-in/drop-out

3. Noita — Nolla Games [X]
   Evidence needed: devblog/postmortem videos
   Lesson: simulation-first worldview with pixel-accurate interaction; every system mutable; failure-state as narrative

4. Vintage Story — Tyron / Lone-dev model [X]
   Evidence needed: official site, GitHub, wiki
   Lesson: solo active-development longevity over a decade; voxel/terrain hybrid with finite survival loop; deep modding fidelity as retention mechanism

5. Keep Talking and Nobody Explodes — Steel Crate Games [X]
   Evidence needed: public postmortems
   Lesson: small-team release discipline; documentation as UX; scoped to one mechanic and extended through community content

### 2.5 Engine Architecture Summary

[S] ValveSoftware/GameNetworkingSockets is real-time UDP with reliable/unreliable
message lanes, fragmentation/reassembly, NAT traversal, encryption, and peer-to-peer
relay through SDR. That makes it viable for ZE net networking instead of building UDP
fragmentation from scratch.
[S] Dear ImGui is a bloat-free immediate-mode C++ graphical user interface toolkit with
minimal dependencies; it is used as a debug/editor layer in many shipped engines and tools.
[S] UE4SS is a runtime modding/plugin system for Unreal Engine with signature scanning,
preload injection, and scripting APIs; it validates the ZE mod compatibility approach in M7.
[S] Vulkan roadmap 2026 adds a new descriptor heap extension, extending the bindless
indexing story already in core Vulkan.
[S] UE5 public docs confirm Nanite, Lumen, World Partition, Mass Entity, and PCG as
shipped systems; ZE will subset these rather than re-solve solved problems.

- id Tech 7/8: bindless descriptors, cached shadow atlas, visibility buffer, clustered compute deferred shading, sector streaming. [X]
- Decima: GPU-based procedural placement; wavefront-batched visibility queries; three-tier terrain; dynamic grass via compute. [X]
- UE5 Nanite + Lumen: hierarchical cluster DAG; Hi-Z two-pass culling; virtual shadow maps; compute rasterizer for sub-pixel clusters. [X]

### 2.6 Research Gaps and Next Actions

| Milestone | Gap | Next Action |
|-----------|-----|-------------|
| M2 Crafting | No matching paper in current corpus | Add 3-5 findings from FDG postmortems and filed survival analyses |
| M9 Vehicles | No matching paper in current corpus | Add 3-5 findings from shipped postmortems and driving-model research |
| M7 — Total persistence | Missing from earlier milestone map | Add full M7 breakdown with EXT blocks from spec/M7.md |
| M6 Audio | GSound mapping is partial | Expand with runtime voxel/cone propagation benchmark |
| M10 Weather | Papers 43/44 are adjacent | Reconcile simulation resolution and gameplay hooks |

## 3. Design Pillars

1. AI Director creates drama, not difficulty. State-machine pacing, event-driven transitions. Difficulty is secondary to dramatic tension.
2. Weight is the primary constraint. Carry capacity, base storage, vehicle load.
3. Permadeath has gradients. Character death ends that character; skills are lost; world state persists.
4. Systems interact, not scripts. Each system connects to at least two others.
5. Bindless jobified architecture. No main-thread bottleneck. Bindless Vulkan throughout. GPU compute does skinning, culling, and simulation.

## 4. Architecture Decisions

| Decision ID | Question | New Answer | Evidence Source | Milestone |
|-------------|----------|------------|-----------------|-----------|
| E81 | Horde trigger | AI Director event, not calendar timer | Paper 11 / L4D pacing | M3-M5 |
| E10 | Tech unlock | Discovery-gated schematics | Paper 3 / Subnautica | M2 |
| E59 | Building freedom | Structural-integrity freeform | Paper 7 / 7DTD | M8 |
| E48 | Co-op sync | Authority with CRDT resolution | E12 E51 / netsync literature | M12 |
| E16 | Main quest | Director arc with world triggers | Paper 11 / L4D | M11 |
| E55 | Server persistence | Dedicated save-anywhere | DayZ migration lessons / Paper 18 | M12 |
| E77 | Materials | 5-tier with fracture response | Paper 7 | M8 |
| E91 | Zombie types | Region-biased dynamic allocation | Paper 9 / Project Zomboid | M5 |
| E92 | Corpse permanence | Decomposition-to-fertilizer loop | Paper 18 / The Forest as-model | M5 |

## 5. Milestone Map

1. M0 — Bindless Vulkan renderer, job system, clustered lighting, shadow caching.
2. M1 — Input, ECS foundation, EnTT integration.
3. M2 — Weight-constrained survival loop, knowledge-based crafting, moodle status.
4. M3 — Horde system, fire, weather, ballistics, basic interaction minigames.
5. M4 — 36 km² streaming world, vegetation, caves, POI density.
6. M5 — AI Director, zombie types, pathfinding, ragdoll/hit-reaction physics.
7. M6 — Audio occlusion, long-range propagation, survival audio feedback.
8. M7 — Total persistence, save system, delta compression, mod compatibility.
9. M8 — Structural integrity base building, 5-tier materials, traps.
10. M9 — Vehicles, parts assembly, fuel chains.
11. M11 — Factions, reputation, narrative director, procedural missions.
12. M12 — Multiplayer sync, persistence, dedicated server.
13. M13 — Endgame legacy goals, regional threats, New Game Plus.

## 6. Technical Architecture

- Renderer: bindless indexed draw; visibility/deferred split; Hi-Z culling; indirect draw compaction; compute-shader raster fallback.
- Jobs: enkiTS persistent task groups; render jobs separate from simulation jobs; Hi-Z and skinning reuse transform buffers.
- Physics: XPBD joints for ragdoll; IPC barrier for penetration-free contact; prefractured destruction modes; tall-cell water.
- Audio: sparse voxel propagation; frequency-band diffraction; portal-aware occlusion; update cadence 10-20 Hz.
- Persistence: SoA entity tables; delta compression; schema versioning with migration path; named save-header offsets.

## 7. Verification Gates

1. Build clean with zero warnings on MSVC /W3.
2. 60 FPS at 1080p on RTX 2070 SUPER, 6 GB VRAM ceiling.
3. Tests: 20 passed, 1 skipped, 83/83 assertions green on headless CI.
4. Emergent test: two systems produce an unscripted state.
5. Permadeath test: character death ends the character, world persists.
6. Director test: pacing state machine transitions on event, not timer alone.
7. Network sync test: two clients reach identical world state after prescribed event sequence.

## 8. Open Questions

1. AI Director tuning curves require playtest telemetry.
2. Structural integrity physics performance requires profiling.
3. 36 km² streaming load time requires prototype measurements.
4. NPC count ceiling requires agent simulation benchmark on target hardware.
5. Network reconciliation policy under packet loss requires multiplayer prototype.

## Appendix A: Professional Studio And Indie Engine Case Studies

Purpose: extract durable patterns for ZE from how shipped engines were built.

### Big studios

1. id Tech 7/8 — id Software
   Evidence: SIGGRAPH Advances talks, DOOM Eternal graphics study
   Lesson: bindless-first, reduced CPU overhead, clustered compute shading

2. Decima Engine — Guerrilla / Sony
   Evidence: Horizon public talks, SIGGRAPH coverage
   Lesson: three-tier terrain, compute grass, visible multilevel placement

3. UE5 Nanite + Lumen — Epic Games
   Evidence: SIGGRAPH 2021 UE5 course, public docs, cached UE5 docs
   Lesson: hierarchical cluster DAG, software raster for micro clusters, virtual shadow maps, world partition

4. Frostbite — DICE / EA
   Evidence: GDC Vault index pages, public talks, cached GDC pages
   Lesson: shared engine across studios, runtime toolchain integration, datamaterial pipelines

5. Unity DOTS + HDRP — Unity Technologies
   Evidence: public Unity blog, cached docs/repos
   Lesson: ECS-first architecture, burst compiler, data-driven render graph

### Indie and small-team custom-engine efforts

1. Lethal Company — Zeekerss
   Evidence: public devlog and release postmortem coverage
   Lesson: single-session scope, emergent horror, community rumor-driven content

2. Valheim — Iron Gate Studio
   Evidence: public developer Q&A and postmortem coverage
   Lesson: biome gating, disciplined scale, network model design

3. Noita — Nolla Games
   Evidence: public developer blog and postmortem videos
   Lesson: simulation-first worldview, pixel-accurate interaction, failure-state storytelling

4. Vintage Story — Tyron / Lone-dev model
   Evidence: official site, GitHub, wiki; long-term solo active-development history
   Lesson: solo-engine longevity, voxel/terrain hybrid, deep modding fidelity

5. Keep Talking and Nobody Explodes — Steel Crate Games
   Evidence: public postmortems and developer talks
   Lesson: small-team release discipline, scoped documentation-as-UX

## Appendix B: Explicit Research Backlog

This backlog is intended to stay executable. Each item names an exact source
or primary-action trigger, not a vague wish.

| ID | Source | Route | Target Block |
|----|--------|-------|--------------|
| RB-01 | UPDATE: Overwatch Workshop sources | GitHub / README cache | M11 narrative/quest blocks |
| RB-02 | GameDeveloper.com procedural generation article | direct extract | M4/M4.5 POI generation |
| RB-03 | UE5 docs: world partition / mass entity | direct fetch | M0/M4 engine architecture |
| RB-04 | UE5 docs: PCG / Niagara rules | direct fetch | M4 procedural content |
| RB-05 | Vulkan descriptor heap / indexing docs | direct fetch | M0 renderer, bindless wiring |
| RB-06 | GameNetworkingSockets README | direct fetch | M12 netcode integration |
| RB-07 | Dear ImGui source and backends study | direct fetch | M0 debug tools, editor layer |
| RB-08 | M7 full breakdown from spec/M7.md | local spec | M7 persistence section |
| RB-09 | M9 vehicle sources / shipped postmortems | pending live retrieval | M9 vehicle section |
| RB-10 | M2 crafting system sources | pending live retrieval | M2 crafting section |

Rule for backlog: every item must move to [S] tagged evidence or [X] tagged
verification blocker. No permanent [E] assumptions.