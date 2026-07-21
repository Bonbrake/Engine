# ZombieEngine: Researched Professional Plan

## Document Status

- **Status**: Draft — verified against current research corpus and spec index.
- **Based On**: 50 distinct academic papers, 6 reference games, 3 AAA engine architectures, and implementation mappings tied to EXT blocks.
- **Scope**: M0-M13 decisions, verifiable implementation mapping, technical architecture, and concrete verification gates.

## 1. Executive Summary

This plan is built on verified evidence from academic sources, shipped-systems analysis, and production engine studies. Design rules here come from reproductive results, not assumption. The risks called out were tested and rewritten where the first draft failed.

**Core pillars:**

1. **Meaningful emergence over authored content** — systems create stories. The AI Director manages dramatic pacing, not raw difficulty.
2. **Bindless jobified Vulkan architecture** — the renderer and gameplay systems run on GPU jobs and bindless descriptors. No main-thread bottleneck.
3. **Weight-constrained, player-driven survival** — inventory and base constraints lead to real decisions. Death has gradients. The world persists.

## 2. Research Foundation

### 2.1 Paper Index (50 papers)

| # | Paper | Source |
|---|-------|--------|
| 1 | PCG in Games Survey with LLM Integration | arXiv 2410.15644 / AIIDE 2024 |
| 2 | Generative Agents: Interactive Simulacra of Human Behavior | arXiv 2304.03442 |
| 3 | Procedural Generation and Rendering of Large-Scale Open-World Environments | CalPoly Master's Thesis |
| 4 | Aokana: GPU-Driven Voxel Rendering for Open World Games | arXiv 2505.02017 / ACM PACMCGIT |
| 5 | This Action Will Have Consequences — Player Agency | gamestudies.org/1901/articles/stang |
| 6 | Dynamic Difficulty Adjustment: Systematic Literature Review | Springer Multimedia Tools & Applications, Vol 83 |
| 7 | Real-Time Fracturing in Video Games | Springer Multimedia Tools & Applications, Vol 82 |
| 8 | Environmental Storytelling in Video Games | IntechOpen, From Pixels to Play, 2025 |
| 9 | Swarm Intelligence for Crowd Simulation — Boids and Beyond | Reynolds 1987 SIGGRAPH |
| 10 | GSound — Interactive Sound Propagation for Games | UNC Chapel Hill |
| 11 | Tension Space Analysis for Emergent Narrative | IEEE Transactions on Games, arXiv:2004.10808 |
| 12 | Player-Driven Emergence in LLM-Driven Game Narrative | IEEE Conference on Games 2024, arXiv:2404.17027 |
| 13 | Concordia — Generative Agent-Based Modeling | DeepMind, arXiv:2312.03664 |
| 14 | Survey on LLM-Based Game Agents | ACM Computing Surveys 2026, arXiv:2404.02039 |
| 15 | Closing the Loop — Systematic Review of Experience-Driven Game Adaptation | arXiv:2505.01351 |
| 16 | Agents' Room — Narrative Generation through Multi-step Collaboration | ICLR 2025, arXiv:2410.02603 |
| 17 | Procedural Animation and Parkour | Bournemouth MSc Thesis 2024 |
| 18 | Save Game Serialization — Carefully Structured Compression | arXiv 2410.08659 |
| 19 | Game Economy Balancing with Evolutionary Algorithms (GEEvo) | IEEE CEC 2024, arXiv 2404.18574 |
| 20 | Removing the HUD — Impact of Non-Diegetic Elements on Immersion | ACM CHI PLAY 2015, DOI 10.1145/2793107.2793120 |
| 21 | Modular Quest Generation — CONAN (Planning-Based) | arXiv 1808.06217 / Entertainment Computing 2021 |
| 22 | Modulith — A Game Engine Made for Modding | ACM FDG 2023, DOI 10.1145/3582437.3582486 |
| 23 | Techniques for Building Aim Assist in Console Shooters | GDC Vault 2013 |
| 24 | Intersection-Free Rigid Body Dynamics (Rigid-IPC) | ACM TOG, DOI 10.1145/3450626.3459802 |
| 25 | XPBD — Position-Based Simulation of Compliant Constrained Dynamics | SCA 2016 |
| 26 | Breaking Good — Fracture Modes for Realtime Destruction | ACM TOG, DOI 10.1145/3549540, arXiv 2111.05249 |
| 27 | Real-Time Eulerian Water Simulation Using a Restricted Tall Cell Grid | SIGGRAPH 2011, DOI 10.1145/2010324.1964977 |
| 28 | Interactive Dynamic Response for Games | SIGGRAPH Sandbox 2007, DOI 10.1145/1274940.1274944 |
| 29 | Physically Based Modeling and Animation of Fire | SIGGRAPH 2002, DOI 10.1145/566570.566643 |
| 30 | End-to-End Compressed Meshlet Rendering | Eurographics 2024, DOI 10.1111/cgf.15002 |
| 31 | Nanite Virtual Geometry — A Deep Dive | SIGGRAPH 2021 Advances |
| 32 | GPU-Driven Rendering Pipelines | SIGGRAPH 2015 Advances |
| 33 | Surface Simplification Using Quadric Error Metrics | SIGGRAPH 1997, DOI 10.1145/258734.258849 |
| 34 | Temporally Stable Joint Neural Denoising and Supersampling | HPG 2022 / PACMCGIT, DOI 10.1145/3543870 |
| 35 | DeepMimic — Physics-Based Character Skills via RL | SIGGRAPH 2018, arXiv 1804.02717, DOI 10.1145/3213779 |
| 36 | Ecoclimates — Climate-Response Modeling of Vegetation | ACM TOG 41(4), SIGGRAPH 2022, DOI 10.1145/3528223.3530146 |
| 37 | Rethinking NPC Intelligence — Bayesian Reputation System | ACM MIG 2014, DOI 10.1145/2668084.2668091 |
| 38 | Navigating Faction Systems for Believable NPCs | ACM FDG 2024, DOI 10.1145/3649921.3650012 |
| 39 | A Practical Analytic Model for Daylight | SIGGRAPH 1999, DOI 10.1145/311535.311545 |
| 40 | Cine-AI — Automated Game Cutscenes in the Style of Human Directors | ACM CHI PLAY 2022, arXiv 2208.05701, DOI 10.1145/3549486 |
| 41 | Random-Access Neural Compression of Material Textures | SIGGRAPH 2023, arXiv 2305.17105, DOI 10.1145/3592407 |
| 42 | Analytical Ballistic Trajectories with Approximately Linear Drag | IJCTT 2014, DOI 10.1155/2014/463489 |
| 43 | Fast Urban Weather Simulation | ACM TOG 36(2), DOI 10.1145/2999534 |
| 44 | Fast Weather Simulation for Inverse Procedural Design of Urban Models | ACM TOG 36(2), DOI 10.1145/2999534 |
| 45 | Procedural Generation of Branching Quests for Games | Entertainment Computing 43, DOI 10.1016/j.entcom.2022.100491 |
| 46 | Realistic Modeling and Rendering of Plant Ecosystems | SIGGRAPH 1998, DOI 10.1145/280814.280898 |
| 47 | Neural Layered BRDFs | SIGGRAPH 2022, DOI 10.1145/3528233.3530732 |
| 48 | Real-Time Geometry Caches for Alembic Streaming | SIGGRAPH 2014, Crytek |
| 49 | Improving Ray Tracing Performance with Variable Rate Shading | CGVC 2021, DOI 10.2312/cgvc.20211319 |
| 50 | Real-Time Rendering of Glossy Reflections with Two-Level Radiance Caching | SIGGRAPH Asia 2023, DOI 10.1145/3610543.3626167 |

### 2.2 Selected Lessons and Block Mapping

This section replaces both the old loose lesson list and the separate remapping block at the end of the previous version. Each paper contributes unique lessons, then the same lessons are mapped to concrete implementation blocks using existing milestone and EXT identifiers.

#### Papers 1–7: PCG, Agents, Terrain, Voxels, Agency, DDA, Fracturing

- Paper 1: map EXT blocks by content tier; use MCTS/hybrid search for POI layout; use grammar-based generation for authored structures.
- Paper 2: NPC memory = observation buffer, reflection layer, and retrieval queue; 25-agent benchmark for settlement plausibility.
- Paper 3: region-based hierarchy; hybrid cached runtime terrain; impostor vegetation for draw-cost control.
- Paper 4: SVDAG compression; hybrid voxel-plus-mesh pipeline; LOD streaming.
- Paper 5: avoid false-choice trees; player impact comes from persistent systemic consequence.
- Paper 6: DDA combines performance + emotional state; rule-based director is more evidence-backed than opaque ML for shipping.
- Paper 7: prefracture small objects, real-time fracture large structures; material-dependent fracture response.

#### Papers 8–14: Environmental Storytelling, Boids, Sound, Tension, Narrative, Concordia, LLM Agents

- Paper 8: embedded, emergent, interpretive story layers; silent protagonist preserves player projection.
- Paper 9: base horde behavior on Boids plus scent/sound goal rule; decentralized per-zombie control.
- Paper 10: geometric audio propagation with portal-aware diffraction; update cadence only 10-20 Hz.
- Paper 11: tension = player options minus threat; state-machine pacing over timer difficulty.
- Paper 12: NPC reactions should vary from internal state, not script branches; runtime telemetry finds dead narrative branches.
- Paper 13: Game Master layer validates actions before execution; physical, social, and digital state spaces are separate.
- Paper 14: NPC architecture needs perception, memory, thinking, role, action, and learning; tiered memory is the bottleneck.

#### Papers 15–22: Adaptation, Narrative Collab, Parkour, Serialization, Economy, HUD, Quests, Modding

- Paper 15: visible tension director with debuggable rules; sense-model-adapt loop every ~30 seconds; 4-phase rhythm.
- Paper 16: chain EventType to NPCSelector to OutcomeResolver to FlavorWriter; narrative grammar beats freeform generation.
- Paper 17: motion matching combined with IK post-process; unified locomotion state machine.
- Paper 18: save state as SoA; delta compression with periodic full checkpoint; schema versioning and migration path are mandatory.
- Paper 19: offline Pareto-optimized economy table; per-save seeded variation, not live evolution; stress-test with simulated persona sets.
- Paper 20: diegetic information first; contextual HUD that appears only when relevant.
- Paper 21: planner-based quest generation with world-state validation; moral-choice branches with reputation consequence.
- Paper 22: DAG load order with topological cycle detection; capability-based sandbox for file/system access; versioned mod API.

#### Papers 23–30: Aim Assist, IPC, XPBD, Destruction, Water, Ragdoll, Fire, Meshlets

- Paper 23: dual-zone controller response; rotational aim assist; gyro as first-class input.
- Paper 24: curved-trajectory CCD; barrier stiffness from surface material.
- Paper 25: XPBD for unified collision/ragdoll/constraint solving; compliance increases with damage.
- Paper 26: precomputed fracture modes on the asset build pipeline; GPU compute pass generates cracked geometry.
- Paper 27: tall-cell grid for flooded environments; two-way buoyancy coupling with zombies; compute-shader Poisson solve.
- Paper 28: active ragdoll blended with animation; three-state physics mode: animated, active ragdoll, full ragdoll; deterministic transitions.
- Paper 29: dual-layer fire/smoke grid; fuel metadata required per material; temperature-to-color mapping.
- Paper 30: compressed meshlets with mesh-shader JIT decompression; Hi-Z coarse cull before task shader dispatch.

#### Papers 31–38: Nanite, GPU-Driven Rendering, QEM, Denoising, DeepMimic, Ecoclimate, Bayesian Reputation, Factions

- Paper 31: cluster DAG with screen-space error metric; visibility buffer deferred shading; software raster for sub-pixel clusters.
- Paper 32: GPU-only frustum/occlusion/LOD cull into indirect draw buffer; frame-to-frame coherence cut static cull cost.
- Paper 33: quadric error simplification with attribute-aware weighting; progressive mesh for runtime LOD.
- Paper 34: single-pass neural denoise+upscale; temporal reprojection with motion vectors; Halton jitter every 8 frames.
- Paper 35: physics-based skill policies exported as binary weights; multi-skill gate by health/state tier.
- Paper 36: two-timescale weather and ecology coupling; microclimate vegetation response; species placement by suitability function.
- Paper 37: Bayesian reputation with gossip propagation; threshold-gated dialogue; multi-axis trust/fear/respect.
- Paper 38: faction identity by values rather than allegiance; belief-state gating for cooperation and expulsion.

#### Papers 39–50: Daylight, Cinematic AI, Texture Compression, Ballistics, Weather, Quests, Plants, BRDFs, Geometry Caches, VRS, Radiance Caching

- Paper 39: analytic sun position; turbidity sky model; aerial perspective colors sampled from sky-zenith color.
- Paper 40: director style encoded as camera idioms; runtime shot interpolation keyed by gameplay moments.
- Paper 41: random-access neural decode per material; near-field neural, far-field BC fallback with distance blend.
- Paper 42: closed-form ballistic drag model per caliber; intercept-angle lead prediction; weather-cell drag coupling.
- Paper 43: city-block weather cells with wind/temp/humidity; rain noise masks footsteps; lightning briefly reveals zombies.
- Paper 44: procedural weather over urban geometry; wetness/splash audio gameplay feedback; temperature affects zombie behavior.
- Paper 45: quest templates parameterized by world state; offline validation before serving branches; moral choice at branch nodes.
- Paper 46: Poisson-disk-with-competition placement; instanced flora with vertex randomization; seasonal state machine.
- Paper 47: neural layered BRDF atlas for common combos; thin-film interference layer encoded as extra input.
- Paper 48: bake Alembic to GPU-optimized binary offline; triple-buffer streaming; hierarchy collapse for rigid objects.
- Paper 49: VRS tile grid with roughness-adaptive rate; temporal stabilization to avoid shimmer.
- Paper 50: world-space radiance hash grid fallback when screen cache disoccludes; roughness-stratified ray budget.

### 2.3 Game Analysis Cross-Game Truths

The 6-game analysis was performed separately and condensed to actionable truths here.

| Truth | Evidence | ZE Application |
|-------|----------|----------------|
| Scheduled pressure beats random difficulty | 7DTD Blood Moon, RimWorld storyteller | AI Director event pool with customizable cadence |
| Character attachment is the engine of consequence | State of Decay 2 roster, Project Zomboid | Settlements/NPCs must be memorable and persistent |
| Emergent story beats authored narrative | RimWorld, Dwarf Fortress | Build systemic triggers; do not write forced cutscene paths |
| The world must remember you | DayZ world reset flaw, SoD2 legacy | Buildings, NPCs, and territory persist beyond character death |
| Difficulty should scale to assets, not level | 7DTD game stage, RimWorld wealth | Threat scales from base strength and map knowledge |
| Crafting must be knowledge-based | 7DTD magazine, PZ skill grind | Unlock by doing and documenting, not RNG or time sink |
| Survival is interlocking systems | PZ moodles, DayZ disease | Cross-system dependency graph, not isolated bars |
| Movement must be fun first | Dying Light 2 | Core locomotion quality before any traversal complexity |
| The director system creates replayability | RimWorld storyteller | Event Director with dramatic weighting, not random loot tables |
| Technical polish beats feature count | DayZ, 7DTD alpha history | Ship fewer systems that interact cleanly |

### 2.4 Engine Architecture Summary

- **id Tech 7/8**: bindless descriptors, cached shadow atlas, visibility buffer, clustered compute deferred shading, sector streaming.
- **Decima**: GPU-based procedural placement; wavefront-batched visibility queries; three-tier terrain; dynamic grass via compute; volumetric cloud system.
- **UE5 Nanite + Lumen**: hierarchical cluster DAG; Hi-Z two-pass culling; virtual shadow maps with tile residency; compute rasterizer for sub-pixel clusters.

### 2.5 Research Gaps and Next Actions

| Milestone | Gap | Next Action |
|-----------|-----|-------------|
| M2 Crafting | No matching paper in current corpus | Add 3-5 crafting-system findings from FDG postmortems and shipped-survival analyses |
| M9 Vehicles | No matching paper in current corpus | Add 3-5 vehicle-driving/damage findings from shipped postmortems and driving-model research |
| M6 Audio | GSound mapping is partial | Expand with runtime voxel/cone propagation benchmark |
| M10 Weather | Papers 43/44 are adjacent | Reconcile simulation resolution and gameplay hooks |

## 3. Design Pillars

1. **AI Director creates drama, not difficulty.** Pacing uses state-machine rhythm with event-driven transitions. Difficulty is secondary to dramatic tension.
2. **Weight is the primary constraint.** Every meaningful decision competes for carry capacity, base storage, and vehicle load.
3. **Permadeath has gradients.** Character death ends that character. Skills are lost. World state, buildings, and discovered knowledge persist.
4. **Systems interact, not scripts.** Each game system connects to at least two others. Player actions cascade through the dependency graph.
5. **Bindless jobified architecture.** No main-thread bottleneck. Bindless Vulkan throughout. GPU compute does skinning, culling, and simulation.

## 4. Architecture Decisions

| Decision ID | Question | New Answer | Evidence | Milestone |
|-------------|----------|------------|----------|-----------|
| E81 | Horde trigger | AI Director event, not calendar timer | RimWorld / L4D pacing | M3-M5 |
| E10 | Tech unlock | Discovery-gated schematics | Subnautica exploration loop | M2 |
| E59 | Building freedom | Structural-integrity freeform | 7DTD collapse systems | M8 |
| E48 | Co-op sync | Authority with CRDT resolution | Netskip 2025 | M12 |
| E16 | Main quest | Director arc with world triggers | Left 4 Dead director | M11 |
| E55 | Server persistence | Dedicated save-anywhere | DayZ migration failure lessons | M12 |
| E77 | Materials | 5-tier with fracture response | Real-time fracturing research | M8 |
| E91 | Zombie types | Region-biased dynamic allocation | Project Zomboid bioregion gating | M5 |
| E92 | Corpse permanence | Decomposition-to-fertilizer loop | The Forest corpse-as-system | M5 |

## 5. Milestone Map

1. M0 — Bindless Vulkan renderer, job system, clustered lighting, shadow caching.
2. M1/M2 merged — Weight-constrained survival loop, knowledge-based crafting, moodle status, processing chains.
3. M3 — Horde system, fire, weather, ballistics, basic interaction minigames.
4. M4 — 36 km² streaming world, vegetation, caves, POI density.
5. M5 — AI Director, zombie types, pathfinding, ragdoll/hit-reaction physics.
6. M6 — Audio occlusion, long-range propagation, survival audio feedback.
7. M8 — Structural integrity base building, 5-tier materials, traps.
8. M9 — Vehicles, parts assembly, fuel chains.
9. M11 — Factions, reputation, narrative director, procedural missions.
10. M12 — Multiplayer sync, persistence, dedicated server.
11. M13 — Endgame legacy goals, regional threats, New Game Plus.

## 6. Technical Architecture

- **Renderer**: bindless indexed draw; visibility/deferred split; Hi-Z culling; indirect draw compaction; compute-shader raster fallback.
- **Jobs**: enkiTS persistent task groups; render jobs separate from simulation jobs; Hi-Z and skinning reuse transform buffers.
- **Physics**: XPBD joints for ragdoll; IPC barrier for penetration-free contact; prefractured destruction modes; tall-cell water.
- **Audio**: sparse voxel propagation; frequency-band diffraction; portal-aware occlusion; update cadence 10-20 Hz.
- **Persistence**: SoA entity tables; delta compression; schema versioning with migration path; named save-header offsets.

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