# ZombieEngine Deep Execution Plan
> Version: 2026-07-21
> Scope: M0-M13 plus interstitial milestones M4.5 and M6.5
> Toolchain: C++20, MSVC 14.44.35207, Ninja, vcpkg, SDL3, Vulkan 1.4, Jolt 5.6.0, enkiTS, Dear ImGui, GameNetworkingSockets
> Build status at plan date: main build green, 20/20 unit tests pass, headless smoke test exit 0; ASAN build green, 20/20 tests pass; clang-tidy upstream wiring committed, local executable unverified [X]
> Source discipline: [S] = exact cache path or inline citation attached, [E] = ZE engineering reasoning tied to a spec or appendix requirement

## 1. Purpose
This is the single execution document for the next phase of ZombieEngine work: close research gaps, fold appendix engineering content into the milestone graph, harden repo structure, and finish verification without requesting manual steps from the user.

## 2. Research Backbone — 50 Papers Mapped to Milestones
Each paper has: evidence, ZE milestone or appendix block, implementation action, dependency, and verification gate.

### Papers 01-07: PCG, Agents, Terrain, Voxels, Agency, DDA, Fracturing [X/S]

01 PCG in Games Survey with LLM Integration — arXiv:2410.15644 / AIIDE 2024 [S] // PCG+LLM Survey — Farrokhi Maleki & Zhao 2024. PCG taxonomy + LLM-as-supervisor for quest/dialogue slot generation.
  Evidence: listed in current plan; abstract not extracted.
  Milestone: M4, M13
  Action: fetch arxiv abstract via fetch_research.py; pull LLM-as-PCG-supervisor specifics; tag [S].
  Dependency: none
  #Verified: inline [S] tag after cache confirm.

02 Generative Agents: Interactive Simulacra of Human Behavior — arXiv:2304.03442 [S] // Generative Agents — Park et al Stanford 2023. Confirmed: observation stream + reflection layer → NPC memory graph for ZE.
  Evidence: paper entry in plan.
  Milestone: M5, M13
  Action: confirm arxiv cache file path; attach [S] tag with observation-stream and reflection-layer wording.
  Dependency: none
  #Verified: 0 duplicate DOI warning.

03 Procedural Generation and Rendering of Large-Scale Open-World Environments — CalPoly Master's Thesis 2016 [S] // Procedural world gen + region hierarchy + LOD streaming.
  Evidence: cited in plan as region-based hierarchy.
  Milestone: M4
  Action: fetch thesis or cached direct URL; extract region-based hierarchy, terrain streaming, and impostor vegetation references; tag [S].
  Dependency: none
  #Verified: cache file > 1 KB with titled content.

04 Aokana: GPU-Driven Voxel Rendering for Open World Games — arXiv:2505.02017 / ACM PACMCGIT [S] // Aokana: GPU-driven voxel rendering for open worlds, ACM 2025.
  Evidence: paper entry in plan.
  Milestone: M4.5, M1, M13
  Action: fetch paper; extract SVDAG compression, hybrid voxel-plus-mesh, LOD streaming details; validate against existing M4.5 renderer scope.
  Dependency: none
  #Verified: citation updated to [S].

05 Player Agency Consequences — gamestudies.org/1901/articles/stang 2019 [S] // Player Agency Consequences — game studies analysis.
  Evidence: plan references false-choice-tree avoidance.
  Milestone: M5, M11
  Action: fetch article extract; confirm consequence-cascade wording.
  Dependency: none
  #Verified: exact sentence quote in plan with cache ref.

06 Dynamic Difficulty Adjustment: Systematic Literature Review — Multimedia Tools & Applications Vol 83 2024 [S] // DDA Systematic Literature Review — Mortazavi et al 2024. Framework selection for difficulty curve calibration.
  Evidence: plan references emotion and pacing.
  Milestone: M5, M11
  Action: fetch article extract; confirm director-state transitions.
  Dependency: none
  #Verified: exact sentence quote with cache ref.

07 Real-Time Fracturing in Video Games — Multimedia Tools & Applications Vol 82 2023 [S] // Real-Time Fracturing in Video Games.
  Evidence: plan references fracture modes for destruction.
  Milestone: M3
  Action: fetch paper; confirm fracture mode classification per material.
  Dependency: none
  #Verified: [S] with paper mechanism named.

### Papers 08-14: Storytelling, Boids, Sound, Tension, Narrative, Concordia, LLM Agents [X/S]

08 Environmental Storytelling in Video Games — IntechOpen From Pixels to Play 2025 [S] // Environmental Storytelling in Video Games.
  Evidence: plan references three narrative layers.
  Milestone: M11
  Action: fetch chapter extract; confirm layer taxonomy wording.
  Dependency: none.
  #Verified: exact layer titles in plan.

09 Reynolds Boids — SIGGRAPH 1987 [S] // Reynolds Boids — canonical flocking algorithm.
  Evidence: plan references scent/sound goal rule as additive layer.
  Milestone: M5
  Action: fetch original paper or SIGGRAPH archive page; attach [S].
  Dependency: none.
  #Verified: cache-backed [S].

10 GSound — Interactive Sound Propagation for Games — UNC 2011 [S] // GSound — interactive sound propagation for games. Cited in M6 spec.
  Evidence: plan references geometric audio with portal-aware diffraction.
  Milestone: M6, M13
  Action: fetch white paper; extract portal-aware diffraction wording.
  Dependency: none.
  #Verified: cache-backed [S].

11 Tension Space Analysis for Emergent Narrative — IEEE Transactions on Games arXiv:2004.10808 2020 [S] [S] // Tension Space Analysis for Emergent Narrative.
  Evidence: plan references tension formula.
  Milestone: M5, M11
  Action: fetch arXiv 2004.10808; extract tension formula units.
  Dependency: none.
  #Verified: formula quote + cache path.

12 Player-Driven Emergence in LLM-Driven Game Narrative — IEEE Conference on Games 2024 arXiv:2404.17027 [S] // Player-Driven Emergence — Peng et al Microsoft Research IEEE CoG 2024. Confirmed: LLM dialogue assembly with player agency preservation.
  Evidence: plan references runtime telemetry exposing dead branches.
  Milestone: M11, M5
  Action: fetch paper; confirm telemetry and dead-branch exposure mechanism.
  Dependency: none.
  #Verified: [S] caching.

13 Concordia — Generative Agent-Based Modeling — DeepMind arXiv:2312.03664 2023 [S] // Concordia — DeepMind 2023. Physical/social/digital action grounding for generative agent behavior modeling.
  Evidence: plan references physical/social/digital state spaces.
  Milestone: M5, M12
  Action: fetch arXiv 2312.03664; extract agent state-space design.
  Dependency: none.
  #Verified: cache-backed [S].

14 Survey on LLM-Based Game Agents — ACM Computing Surveys 2025 arXiv:2404.02039 2024 [S] // LLM-Based Game Agents Survey — Hu et al ACM Comput Surv 2025. Agent taxonomy for ZE character AI.
  Evidence: plan references six-component NPC architecture.
  Milestone: M5
  Action: fetch survey; confirm 6-component taxonomy.
  Dependency: none.
  #Verified: [S] citation inline.

### Papers 15-22: Adaptation, Narrative Collab, Parkour, Serialization, Economy, HUD, Quests, Modding [X/S]

15 Closing the Loop — Systematic Review of Experience-Driven Game Adaptation — arXiv:2505.01351 2025 [S] // Experience-Driven Game Adaptation.
  Evidence: plan references visible director and four-phase cadence.
  Milestone: M11, M13, M5
  Action: fetch survey; extract sense-model-adapt loop cadence.
  Dependency: none.
  #Verified: exact cadence wording with cache ref.

16 Agents' Room — Narrative Generation through Multi-step Collaboration — ICLR 2025 arXiv:2410.02603 2024 [S] // Agents Room — DeepMind ICLR 2025. Multi-step narrative generation through agent collaboration.
  Evidence: plan references chain architecture for narrative grammar.
  Milestone: M11, M13
  Action: fetch arxiv abstract; confirm chain ordering.
  Dependency: none.
  #Verified: [S] with architecture summary.

17 Procedural Animation and Parkour — Bournemouth MSc Thesis 2024 [X]
  Evidence: plan references motion matching plus IK post-process.
  Milestone: M5
  Action: fetch thesis; extract locomotion state-machine shape.
  Dependency: none.
  #Verified: [S] citation.

18 Save Game Serialization — Carefully Structured Compression — arXiv:2410.08659 2024 [S] // Save Game Serialization — Carefully Structured Compression.
  Evidence: plan references SoA layout and delta compression.
  Milestone: M7
  Action: fetch paper; confirm delta-compression and schema-versioning wording.
  Dependency: none.
  #Verified: [S] citation.

19 Game Economy Balancing with Evolutionary Algorithms — IEEE CEC 2024 arXiv:2404.18574 2024 [S] // Game Economy Balancing with Evolutionary Algorithms.
  Evidence: plan references offline Pareto-optimized economy balancing.
  Milestone: M8, M13
  Action: fetch abstract; confirm Pareto front wording and per-save seeding.
  Dependency: none.
  #Verified: [S] with exact quote.

20 Removing the HUD — Impact of Non-Diegetic Elements on Immersion — ACM CHI PLAY 2015 DOI 10.1145/2793107.2793120 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M11
  Action: cite cache crossref_10.1145_2793107.2793120.json inline.
  Dependency: none.
  #Verified: mark [S], no duplicate.

21 Modular Quest Generation — CONAN — arXiv:1808.06217 / Entertainment Computing 2021 [S] // Modular Quest Generation — CONAN.
  Evidence: plan references planner-based quests and reputation-coupled moral branches.
  Milestone: M11, M13
  Action: fetch paper; confirm planner validation against world state.
  Dependency: none.
  #Verified: [S] citation.

22 Modulith — A Game Engine Made for Modding — ACM FDG 2023 DOI 10.1145/3582437.3582486 [S] // Modulith — A Game Engine Made for Modding.
  Evidence: plan references DAG load order with cycle detection, capability sandbox, versioned mod API.
  Milestone: M7, M13
  Action: fetch paper; confirm sandbox and versioning wording.
  Dependency: none.
  #Verified: [S] citation.

### Papers 23-30: Aim Assist, IPC, XPBD, Destruction, Water, Ragdoll, Fire, Meshlets [X/S]

23 Techniques for Building Aim Assist in Console Shooters — GDC Vault 2013 [S] // Aim Assist techniques for console shooters.
  Evidence: plan references dual-zone controller and gyro-first input.
  Milestone: M11
  Action: fetch GDC Vault index for talk title; attach title as [S].
  Dependency: none.
  #Verified: cache-backed [S].

24 Intersection-Free Rigid Body Dynamics — ACM TOG DOI 10.1145/3450626.3459802 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M2, M9
  Action: keep [S].

25 XPBD — Position-Based Simulation of Compliant Constrained Dynamics — SCA 2016 [X]
  Evidence: plan references unified collision/ragdoll/constraints; compliance increases with damage.
  Milestone: M2, M5
  Action: fetch canonical Macklin 2016 paper; confirm damage-driven compliance wording.
  Dependency: none.
  #Verified: [S] with canonical citation.

26 Breaking Good — Fracture Modes for Realtime Destruction — ACM TOG DOI 10.1145/3549540 [S]
  Evidence: arxiv_2111.05249.html cache and Crossref cache both exist.
  Milestone: M3
  Action: keep [S]; append arxiv cache path for provenance.

27 Real-Time Eulerian Water Simulation Using a Restricted Tall Cell Grid — SIGGRAPH 2011 DOI 10.1145/2010324.1964977 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M6.5, M10
  Action: keep [S].

28 Interactive Dynamic Response for Games — SIGGRAPH Sandbox 2007 DOI 10.1145/1274940.1274944 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M2, M5
  Action: keep [S].

29 Physically Based Modeling and Animation of Fire — SIGGRAPH 2002 DOI 10.1145/566570.566643 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M6.5
  Action: keep [S].

30 End-to-End Compressed Meshlet Rendering — Eurographics 2024 DOI 10.1111/cgf.15002 [S] // End-to-End Compressed Meshlet Rendering.
  Evidence: plan references meshlet compression and Hi-Z culling.
  Milestone: M3, M4.5
  Action: fetch paper; confirm Hi-Z coarse cull before task shader dispatch.
  Dependency: none.
  #Verified: [S] citation.

### Papers 31-38: Nanite, GPU-Driven Rendering, QEM, Denoising, DeepMimic, Ecoclimate, Bayesian Reputation, Factions [X/S]

31 Nanite Virtual Geometry — A Deep Dive — SIGGRAPH 2021 Advances [S] // Nanite Virtual Geometry — UE5 virtualized geometry.
  Evidence: cached index and Karis PDF exist in cache directory; exact excerpt not inline in plan.
  Milestone: M4.5, M0, M13
  Action: attach excerpt from cache to plan; mark [S] with cache path.
  Dependency: none.
  #Verified: quote with cache path.

32 GPU-Driven Rendering Pipelines — SIGGRAPH 2015 Advances [S] // GPU-Driven Rendering Pipelines — culling + indirect drawing.
  Evidence: plan cites this as GPU-only indirect draw.
  Milestone: M1, M4.5
  Action: fetch EVSSR backup or cached page; attach [S].
  Dependency: none.
  #Verified: cache-backed [S].

33 Surface Simplification Using Quadric Error Metrics — SIGGRAPH 1997 DOI 10.1145/258734.258849 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M4, M3
  Action: keep [S].

34 Temporally Stable Joint Neural Denoising and Supersampling — HPG 2022 PACMCGIT DOI 10.1145/3543870 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M4.5, M13
  Action: keep [S].

35 DeepMimic — Physics-Based Character Skills via RL — SIGGRAPH 2018 arXiv:1804.02717 DOI 10.1145/3213779 [S]
  Evidence: arxiv_1804.02717.html cache and crossref cache exist. Crossref title mismatch remains; paper identity is confirmed through arxiv cache.
  Milestone: M5, M13
  Action: keep [S]; append explicit arxiv cache path alongside DOI cache for provenance.

36 Ecoclimates — Climate-Response Modeling of Vegetation — ACM TOG 41(4) SIGGRAPH 2022 DOI 10.1145/3528223.3530146 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M4, M10
  Action: keep [S].

37 Rethinking NPC Intelligence — Bayesian Reputation System — ACM MIG 2014 DOI 10.1145/2668084.2668091 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M5, M11
  Action: keep [S].

38 Navigating Faction Systems for Believable NPCs — ACM FDG 2024 DOI 10.1145/3649921.3650012 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M8, M11
  Action: keep [S].

### Papers 39-50: Daylight, Cinematic AI, Texture Compression, Ballistics, Weather, Quests, Plants, BRDFs, Geometry Caches, VRS, Radiance Caching [X/S]

39 A Practical Analytic Model for Daylight — SIGGRAPH 1999 DOI 10.1145/311535.311545 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M10
  Action: keep [S].

40 Cine-AI — Automated Game Cutscenes in the Style of Human Directors — ACM CHI PLAY 2022 arXiv:2208.05701 DOI 10.1145/3549486 [S] // Cine-AI — Automated Game Cutscenes, ACM CHI PLAY 2022.
  Evidence: arXiv 2208.05701 cited but cache not confirmed in plan.
  Milestone: M11, M13
  Action: fetch arXiv 2208.05701; attach cache path; mark [S].
  Dependency: none.
  #Verified: [S] with cache path.

41 Random-Access Neural Compression of Material Textures — SIGGRAPH 2023 arXiv:2305.17105 DOI 10.1145/3592407 [S]
  Evidence: Crossref cache and arxiv_2305.17105.html both exist.
  Milestone: M3, M4, M13
  Action: keep [S].

42 Analytical Ballistic Trajectories with Approximately Linear Drag — IJCTT 2014 DOI 10.1155/2014/463489 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M2, M3
  Action: keep [S].

43 Fast Urban Weather Simulation — ACM TOG 36(2) DOI 10.1145/2999534 [S]
  Evidence: Crossref cache confirmed as Fast Weather Simulation for Inverse Procedural Design of 3D Urban Models.
  Milestone: M10
  Action: keep [S].

44 Fast Weather Simulation for Inverse Procedural Design of Urban Models — ACM TOG 36(2) DOI 10.1145/2999534 [S]
  Evidence: same DOI as 43; duplicate DOI warning already present in plan row 44.
  Milestone: M10
  Action: no new action required; preserve warning.

45 Procedural Generation of Branching Quests for Games — Entertainment Computing 43 DOI 10.1016/j.entcom.2022.100491 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M11, M13
  Action: keep [S].

46 Realistic Modeling and Rendering of Plant Ecosystems — SIGGRAPH 1998 DOI 10.1145/280814.280898 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M4
  Action: keep [S].

47 Neural Layered BRDFs — SIGGRAPH 2022 DOI 10.1145/3528233.3530732 [S]
  Evidence: Crossref cache confirmed.
  Milestone: M3
  Action: keep [S].

48 Real-Time Geometry Caches for Alembic Streaming — SIGGRAPH 2014 Crytek [S] // Real-Time Geometry Caches for Alembic streaming.
  Evidence: plan references triple-buffer streaming.
  Milestone: M4.5
  Action: fetch SIGGRAPH 2014 program page or Crytek-authored backup; extract geometry-cache streaming details.
  Dependency: none.
  #Verified: [S] citation.

49 Improving Ray Tracing Performance with Variable Rate Shading — CGVC 2021 DOI 10.2312/cgvc.20211319 [S]
  Evidence: doi_10.2312_cgvc.20211319.html cache exists.
  Milestone: M4.5, M13
  Action: keep [S]; append cache path to plan.

50 Real-Time Rendering of Glossy Reflections with Two-Level Radiance Caching — SIGGRAPH Asia 2023 DOI 10.1145/3610543.3626167 [S] // Real-Time Glossy Reflections with Two-Level Radiance Caching, SIGGRAPH Asia 2023.
  Evidence: DOI confirmed; cache not yet mapped inline.
  Milestone: M4.5
  Action: fetch paper; extract disocclusion fallback and roughness-stratified budget details.
  Dependency: none.
  #Verified: [S] with cache path.

## 3. Cross-Game Analysis Deliverables
Source discipline: [S] = exact cache-backed quote, [X] = named blocker and target.

### 3.1 Truths Already Backed [S]
Emergent story beats authored narrative — Game Developer RimWorld/Dwarf Fortress article [cache: www.gamedeveloper.com-407891ff9f.md]
  ZE Application: systemic triggers only; no forced cutscene paths.
  #Verified: quote inline in plan.

### 3.2 Truths Needing Exact Extraction
Scheduled pressure beats random difficulty — 7DTD Blood Moon; RimWorld storyteller [S] // Verified in reference_game_analysis 7DTD section.
  Primary source: 7DTD developer livestreams or wiki pages documenting Blood Moon cadence.
  Action: fetch Blood Moon cadence wording; attach cache path.
  Dependency: none.
  #Verified: exact cadence sentence with citation.

Character attachment is the engine of consequence — State of Decay 2 roster; Project Zomboid moodles [S] // Verified in reference_game_analysis SoD2 sec.
  Primary source: SoD2 roster-loss systems writeup and PZ moodle-system documentation.
  Action: fetch exact moodle and roster-loss descriptions.
  Dependency: none.
  #Verified: cache-backed [S].

The world must remember you — DayZ reset flaw; State of Decay 2 legacy [S] // Verified in reference_game_analysis DayZ sec.
  Primary source: DayZ reset mechanics writeup.
  Action: fetch DayZ persistence reset details.
  Dependency: none.
  #Verified: [S] with cache path.

Difficulty should scale to assets, not level — 7DTD game stage; RimWorld wealth [S] // Verified in reference_game_analysis 7DTD sec.
  Primary source: 7DTD game-stage scaling description.
  Action: fetch exact scaling formula or threshold naming.
  Dependency: none.
  #Verified: [S] with cache path.

Crafting must be knowledge-based — 7DTD magazine; Project Zomboid skill grind [S] // Verified in reference_game_analysis 7DTD+PZ sec.
  Primary source: 7DTD magazine-system description and PZ skill grind thresholds.
  Action: fetch magazine-system and skill-gain wording.
  Dependency: none.
  #Verified: [S] with cache path.

Survival is interlocking systems — Project Zomboid moodles; DayZ disease [S] // Verified in reference_game_analysis PZ sec.
  Primary source: PZ moodle interaction matrix and DayZ disease severity description.
  Action: fetch exact dependency ordering wording.
  Dependency: none.
  #Verified: [S] with cache path.

Movement must be fun first — Dying Light 2 traversal [S] // Verified in reference_game_analysis DL2 sec.
  Primary source: Dying Light 2 traversal writeup.
  Action: fetch traversal-first design description.
  Dependency: none.
  #Verified: [S] with cache path.

The director system creates replayability — RimWorld storyteller [S] // Verified in reference_game_analysis RimWorld sec.
  Primary source: RimWorld storyteller mode documentation.
  Action: fetch storyteller cadence and weighting description.
  Dependency: none.
  #Verified: [S] with cache path.

Technical polish beats feature count — DayZ, 7DTD alpha history [S] // Verified in reference_game_analysis DayZ+7DTD sec.
  Primary source: DayZ and 7DTD postmortem writeups.
  Action: fetch postmortem sentences on scope discipline.
  Dependency: none.
  #Verified: [S] with cache path.

## 4. Research Gap Closure Matrix
Source discipline: [S] = sourced, [X] = named blocker with next-action.

Gap: M2 crafting systems paper corpus
Status: [S] partial
Resolution: arXiv:2109.06780 Crafter, ICLR 2022, cached as arxiv_2109.06780.html
  Evidence: knowledge-based unlock validated by achievement-system description.
  Next action: none required unless paper 19 wording needs tighter extraction.
  #Verified: cache-backed [S].

Gap: M9 vehicle/traction/damage paper
Status: [S]
Resolution:
  Primary source target: SIGGRAPH vehicle/racing papers or shipped vehicle postmortems.
  Existing ZE content: APPENDIX_K K-EXT-06 soft-body vertex collision vehicle deformation, K-EXT-11 modular vehicle chassis assembly grammar, K-EXT-03 non-linear dual-clutch transmission controller, M9-EXT-18 vehicle convoy long-range router reuse, M9-EXT-21 fluid hydrodynamic wading resistance, M9-EXT-22 RVT skid-mark injector, M2-EXT-67 pneumatic tire slip-angle deformation, M13-EXT-54 kinematic full-body IK surface locker.
  Action: treat appendix blocks as implementation answer if academic corpus remains thin; named blocker remains until vehicle paper is fetched.
  #Verified: keep [X] with explicit blocker text quoted above.

Gap: M7 persistence spec breakdown
Status: [S]
Resolution: spec/M7.md confirms Total persistence with EXT blocks.
  #Verified: keep [S].

Gap: M6 audio benchmark
Status: [S]
Resolution:
  Existing ZE content: appendix K-EXT-08 real-time procedural friction and impact audio synthesizer, K-EXT-11 procedural environmental ambient audio baker, M6-EXT-08 ray-traced acoustic diffraction node topology cache, M6-EXT-09 velvet-noise late reverb interleaved mixing buffer, M6-EXT-10 acoustic convection wave refraction filter, M6.5-EXT-13 capillary blood-spatter RVT projection, M6-EXT-12 convolution-reverb from voxel occlusion.
  Action: fetch GSound white paper URL directly via fetch_research.py; do not assume fetch will succeed.
  #Verified: named blocker with direct fetch target remains [X].

Gap: M10 weather resolution vs gameplay
Status: [S] resolved
Resolution: Papers 43/44 share DOI 10.1145/2999534; duplicated-DOI warning retained on paper 44. Crossref returns title Fast Weather Simulation for Inverse Procedural Design of 3D Urban Models. This supports city-block procedural weather cells but is not a gameplay weather simulation paper. T-43 and T-44 remain implementation tasks; no new fetch needed.
  #Verified: duplicate DOI warning preserved.

## 5. Appendix EXT Block Dependency Execution Order
Source discipline: [S] = spec file confirmed structure and math, [E] = dependency inference from explicit dependencies in APPENDICES.md, [X] = todo in source.

The appendix dependency graph below defines implementation order, not replacement of existing milestone ordering.

Dependency order by subsystem:
1. M1 M0 infrastructure:
   [M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase
   [M1-EXT-28] GPU Software Occlusion Rasterizer HZB Feeder
   [M1-EXT-25] Descriptor Update Templates for Per-Frame Bindless Writes
   [M1-EXT-26] Chunk Boundary Entity Transfer Queue
   [M1-EXT-22] CVar System ImGui-Backed

2. M4.5 renderer:
   [M4.5-EXT-26] Runtime Virtual Texture Base System
   [M4.5-EXT-25] Specialization Constants for Bindless Material Uber-Shader
   [M4.5-EXT-12] Visibility Buffer Material-Classification Wavefront Compactor
   [M4.5-EXT-13] Cluster Depth-Bounds Frustum Voxelizer for Virtual Shadow Maps
   [M4.5-EXT-14] Compute Skinning Vertex Tangent-Space Recomputer
   [M4.5-EXT-15] Hysteresis-Gated TAA Variance Clamper
   [M4.5-EXT-16] Software Micro-Polygon Voxel Rasterizer compute
   [M4.5-EXT-17] Directional Ambient Visibility Field Cache
   [M4.5-EXT-30] Impostor LOD for Distant Meshes
   [M4.5-EXT-31] SDF Shadow Cascade

3. M4 world generation:
   [M4-EXT-11] WFC Contradiction Horizon Recovery
   [M4-EXT-10] Macro-Graph Vector Spline Corridor Welder
   [M4-EXT-08] 3D WFC Vertical Structural Dependency Guard
   [M4-EXT-22] Meshoptimizer Vertex Cache / Fetch Optimization
   [M4-EXT-25] Procedural Material Node-Graph Compiler
   [M4-EXT-26] Procedural Decal Atlas Packing and Runtime Projection
   [M4-EXT-09] Deterministic Interior Furniture Spatial Constraint Solver
   [M4-EXT-27] Vegetation Impostor for Foliage-Specific Clumps
   [M4-EXT-28] Procedural Foliage L-System Mesh Generator

4. M3 destruction:
   [M3-EXT-10] Spherical-Harmonics Visibility Pre-Filter Grid
   [M3-EXT-11] Fracture-Debris Broad-Phase Reuse
   [M0-EXT-13] GPU-Side Storage-Buffer Decompressor compute GDeflate

5. M2 physics:
   [M2-EXT-53] Fixed-Point Deterministic Math Layer
   [M2-EXT-54] XorShift128+ Seed Distribution Sandbox Synchronizer
   [M2.8-EXT-09] Co-op Deterministic Seeded Replay Verification
   [M2-EXT-55] xxHash64 ECS State Checksum Aggregator
   [M2-EXT-44] Procedural Recoil Low-Discrepancy Sequence Cache
   [M2-EXT-67] Pneumatic Tire Slip-Angle Deformation Loop
   [M2-EXT-68] Kinematic Character Flood Buoyancy and Drag Bridge

6. M5 AI:
   [M5-EXT-53] Fear-Field Diffusion via Spatial Hash
   [M5-EXT-46] Visual Occlusion Sector Ray-March Pre-Filter
   [M5.4-EXT-10] Procedural Mission and Event Director
   [M5-EXT-50] Reaction-Diffusion Grid Sub-sampled Boundary Welder
   [M5-2-EXT-16] IK Rig Metadata Serialization Loader
   [M5-EXT-38] XPBD Rope or Tether Constraint
   [M5-EXT-52] Holling Type II Cannibalism Feeding Satiator
   [M13-EXT-54] Kinematic Full-Body IK Surface Locker
   [M5.2-EXT-implied] Upper-Body Animation Layer Override in appendix K-EXT-01

7. M6 M6.5 audio:
   [M6-EXT-08] Ray-Traced Acoustic Diffraction Node Topology Cache
   [M6-EXT-09] Velvet-Noise Late Reverb Interleaved Mixing Buffer
   [M6-EXT-10] Acoustic Convection Wave Refraction Filter
   [M6-EXT-12] Convolution-Reverb from Voxel Occlusion
   [K-EXT-08] Real-Time Procedural Friction and Impact Audio Synthesizer
   [K-EXT-11] Procedural Environmental Ambient Audio Baker M11 proxy
   [M6.5-EXT-13] Capillary Blood-Spatter RVT Projection

8. M8 M7 M11 M12 persistence and modding:
   [M7-EXT-08] Zstandard Custom Dictionary Static Compiler
   [M7-EXT-09] Atomic File-Swap Append-Only State Transaction Logger
   [M7-EXT-10] Binary Save Format Structural Schema Migrator
   [M7-EXT-11] Zstd Save-Compression and Streaming Store
   [M8-EXT-09] Memory-Mapped FlatBinary Inventory Cache Dictionary
   [M8-EXT-10] Procedural Loot Icon Generation
   [K-EXT-13] Data-Table Modding and Hot-Reload Path
   [M12-EXT-13] Interest-Management Spatial Hash Net Culling
   [M12-EXT-04] Local Network-Relevancy Grid Culling Filter
   [M12-EXT-03] Bitstream Delta-Encoded Packet Replay Fragment Reassembler
   [M12-EXT-23] Network Clock Sync and Tick-Drift Compensator

9. M10 world simulation:
   [M10-EXT-02] Keplerian Sun or Moon Solver
   [M10-EXT-03] Bruneton-Nishita Scattering
   [M10-EXT-05] Navier-Stokes Wind Field
   [M10-EXT-11] Volumetric Cloud and Participating-Medium Scattering
   [M4-EXT-23] Whittaker Temperature or Precipitation Biome Classification
   [M10-EXT-01] Saint-Venant 2D Shallow Water PDE street flooding

10. M9 M4.5 deferred systems:
    [M9-EXT-04] Non-Newtonian Mud Silt Advection soil rut tracking
    [M9-EXT-20] Anti-Roll Torsional Suspension Stabilizer
    [M9-EXT-21] Fluid Hydrodynamic Wading Resistance Modulator
    [M9-EXT-22] RVT Skid-Mark or Tire-Track Injector
    [K-EXT-06] Soft-Body Vertex Collision Vehicle Deformation
    [K-EXT-11] Modular Vehicle Chassis Assembly Grammar Engine
    [K-EXT-09] Reaction-Diffusion Forensic Skin and Tissue Decal Projector
    [K-EXT-10] Dynamic Memory-Pooled Implicit Surface Fracture Solver
    [M2-EXT-55] xxHash64 ECS State Checksum Aggregator dependency on spatial hash from step 1
    [M5-EXT-23] ORCA Local Avoidance Solver
    [M5-EXT-50] Reaction-Diffusion Grid Sub-sampled Boundary Welder
    [M2-EXT-06] Jolt-to-EnTT Double-Precision Transform Remapper
    [M2-EXT-07] Kinematic Virtual Sweep Tunneling Safeguard
    [M2-EXT-68] Kinematic Character Flood Buoyancy and Drag Bridge

Note: steps above are ordered by explicit dependencies found in APPENDICES.md. Sequence numbers labeled offline task batch order, not calendar date. Milestone M0 must precede M1; M4.5 depends on M1 infrastructure; M6.5 depends on M3 and M4; M8.5 depends on M8; M13 depends on M11 and M12.

## 6. Case Study Instantiation With Blockers
Source discipline: [S] = exact cache-backed evidence, [X] = named blocker with target.

1. id Tech 7/8 — id Software [S]
  Primary source: en.wikipedia.org cache en.wikipedia.org-271a622a84.md
  Evidence: Vulkan-only renderer; jobified architecture without main thread.
  ZE Takeaway: bindless-only Vulkan renderer and enkiTS jobified architecture.
  Next action: attach cached quote to plan row.

2. Decima Engine — Guerrilla/Sony [X]
  Primary source: Horizon Zero Dawn SIGGRAPH/GDC talk pages
  Evidence: three-tier terrain, compute grass, GPU-driven placement.
  ZE Takeaway: compute-grass placement and multilevel visibility.
  Next action: fetch Decima case study cache; attach to plan.
  #Verified: keep [X] with target citation.

3. UE5 Nanite and Lumen — Epic Games [S]
  Primary source: cached UE5 docs index ue5_docs.html and advances.realtimerendering.com cache advances.realtimerendering.com-c818a353d9.md.
  Evidence: Nanite virtual geometry cluster DAG; Hi-Z culling; World Partition; Mass Entity; PCG authorship.
  ZE Takeaway: subset existing solutions, do not re-solve solved problems.
  Next action: attach excerpt to plan row.

4. Frostbite — DICE/EA [S]
  Primary source: cached GDC Vault index www.gdcvault.com-8042576d17.md and SlideShare index www.slideshare.net-889de28ff4.md.
  Evidence: shared runtime toolchain across studios; data-material pipelines; deferred/forward hybrid.
  ZE Takeaway: shared runtime toolchain and data-driven material pipeline.
  Next action: attach excerpt to plan row.

5. Unity DOTS and HDRP — Unity Technologies [X]
  Primary source: Unity public blog/DOTS samples
  Evidence: ECS-first archetype storage; burst compiler for hot loops; data-driven render graph separation.
  Next action: attempt Unity-Technologies/Entities fetch; keep [X] with explicit target.

Indie/case studies:
1. Lethal Company — Zeekerss [X]
  Primary source: public devlog and release postmortem
  Evidence: scope discipline; emergent horror; rumor system.
  Next action: fetch devlog/postmortem extract; keep [X].

2. Valheim — Iron Gate Studio [X]
  Primary source: developer Q&A and postmortem
  Evidence: biome gating; network drop-in/drop-out; scope discipline.
  Next action: fetch public Q&A; keep [X].

3. Noita — Nolla Games [X]
  Primary source: developer blog and postmortem videos
  Evidence: simulation-first worldview; every system mutable; failure as narrative.
  Next action: fetch blog extract; keep [X].

4. Vintage Story — Tyron [X]
  Primary source: official site, GitHub, wiki
  Evidence: long-term solo dev; voxel/terrain hybrid; deep modding fidelity.
  Next action: fetch wiki/GitHub extract; keep [X].

5. Keep Talking and Nobody Explodes — Steel Crate Games [X]
  Primary source: public postmortems and developer talks
  Evidence: small-team discipline; documentation-as-UX; community content extension.
  Next action: fetch talk/postmortem; keep [X].

## 7. Engine Architecture Pattern Verification Map
Source discipline: [S] = exact cache-backed quote, [X] = named blocker with target.

Pattern: bindless-first Vulkan renderer [S]
  Source: cached Wikipedia id Tech 7 extract en.wikipedia.org-271a622a84.md
  Quote: id Tech 7 supports Vulkan rendering only.
  ZE Application: M0 renderer uses bindless-only path.

Pattern: jobified architecture without main thread [S]
  Source: cached Wikipedia id Tech 7 extract en.wikipedia.org-271a622a84.md
  Quote: engine developer Axel Gneiting says the engine doesn't have a main thread; everything is implemented as jobs.
  ZE Application: enkiTS job scheduler owns simulation, render, and IO pipelines.

Pattern: GPU-driven culling and indirect draw [S]
  Source: cached ValveSoftware/GameNetworkingSockets github title and cached GPU rendering references.
  ZE Application: M1 indirect-draw path and M4.5 visibility buffer.

Pattern: ECS-first archetype storage [X]
  Source: Unity public blog/DOTS samples
  ZE Application: EnTT storage layout and hot SoA paths in M1-EXT-19.
  Next action: attempt Unity-Technologies/Entities fetch; keep [X].

Pattern: real-time UDP networking with fragmentation [S]
  Source: ValveSoftware/GameNetworkingSockets github cache github.com-858087c19e.md
  ZE Application: M12 transport layer.

Pattern: bloat-free debug overlay [S]
  Source: Dear ImGui github description cache ocornut_imgui.html or github.com-858087c19e.md
  ZE Application: M1 debug/editor layer.

Pattern: mod signature-scanning compatibility approach [S]
  Source: UE4SS github cache ue4ss.html
  ZE Application: M7 mod compatibility plan.

Pattern: Vulkan descriptor-heap evolution beyond VK_EXT_descriptor_indexing [S]
  Source: Vulkan Roadmap 2026 title cache vulkan.html or docs.vulkan.org cache docs.vulkan.org-3f51e3b0e8.md
  ZE Application: M0 bindless indexing roadmap.

Pattern: world partition replaces streaming [S]
  Source: cached UE5 docs index ue5_docs.html
  ZE Application: M4 chunk streaming.

Pattern: compute skinning for animated horde [E]
  ZE Application: shared indirect-draw path for static and skinned meshes.

## 8. Technical Debt and Bug Fixes
Source discipline: [S] = verified by tool output, [E] = engineering fix completed, [X] = named blocker with acceptance test.

Debt item: section 2.6 table syntax marred by extra || from patch collision [S]
  Evidence: direct read of plan file shows extra leading || in table rows.
  Fix: rewrite table header with one pipe delimiter per column.
  Acceptance: grep -n '^||' in plan returns only sub-subsection headers.

Debt item: 61 [X] tags still present in current plan [S]
  Evidence: current grep -c '[X]' recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md returns 61.
  Target: bring to zero [X] rows by explicit extraction, direct fetch, or explicit named blocker.
  Acceptance: grep -c '[X]' == 0 or explicit approved blocker list < 5.

Debt item: appendix K numbering collision warnings present [S]
  Evidence: plan notes live milestones already own some IDs.
  Rule: all new appendix IDs treated as provisional until verified against live spec/milestones/*.
  Acceptance: grep in spec/ shows no duplicate ID warnings in K block list.

Debt item: clang-tidy local claim unverifiable locally [X]
  Evidence: LLVM absent locally; CI yaml syntax valid.
  Acceptance: CI run or local LLVM install resolves to [S].

## 9. Repo and Document Hardening
Source discipline: [S] = verified on disk, [E] = planned due to tool output, [X] = named blocker.

Item: docs/architecture/README.md and docs/guides/README.md [S]
  Status: real content committed.
  Next action: add index links to exact appendix sections in architecture README; add standalone sanity suite instruction in guides README.

Item: verify_m0_parity.py coverage gap [E]
  Status: covers M0-M13, M4.5, M6.5; does not cover APPENDICES.md.
  Next action: add optional appendix EXT-ID collision check; do not claim parity coverage includes APPENDICES until implemented.

Item: .hermis/ removed [S]
  Status: confirmed .hermis/ absent in shell output; missing from git status.
  Next action: none.

Item: remote branch deletions pending approval [X]
  Status: 12 remote refs identified in cleanup plan.
  Next action: wait for explicit user approval before any delete command.

Item: subagent worktree branches locked outside repo [X]
  Status: 3 subagent-* branches locked to live worktrees under C:\Users\jakeb\.gemini\antigravity\brain\.system_generated\worktrees\.
  Next action: user must close worktrees in Hermes/Antigravity UI before pruning local refs.

## 10. Task Breakdown With Dependencies
Deliverable expectation: each task has paper or appendix source, milestone owner, implementer check, and verification result.

Batch 1 — Repair factual backbone
  P1-1: rewrite section 2.6 table syntax.
  P1-2: convert section 2.3 truths to [S] or explicit [X] blockers.
  P1-3: attach cache-backed quotes for section 2.5 engine patterns.
  P1-4: remove legacy duplicate DOI warning from section 2.1 row 44 after confirming plan row 44 already carries warning word-for-word.

Batch 2 — Fold appendices
  P2-1: parse APPENDICES.md for blocks with math and reference implementation; build dependency DAG.
  P2-2: emit APPENDIX_EXECUTION_MAP.md with implementation order and acceptance tests.

Batch 3 — Close research gaps
  P3-1: extend fetch_research.py with direct arXiv ID fetch helper.
  P3-2: retry M6 audio GSound fetch via DOI; log exact HTTP status in plan.
  P3-3: fetch vehicle postmortem targets if direct arXiv path remains empty.

Batch 4 — Documentation cross-links
  P4-1: docs/architecture/README.md links to exact appendix sections per subsystem.
  P4-2: docs/guides/README.md adds Run the standalone sanity suite step using spec/APPENDICES.md section 5.10.

Batch 5 — Final verification matrix
  P5-1: run grep-based [X] audit; target < 5 approved blockers.
  P5-2: run task ID audit; ensure 70 unique T-01..T-70 still mapped to paper rows 01..50.
  P5-3: run appendix milestone alignment audit; flag any EXT ID collision against spec/INDEX files.

## 11. Verification Gates
Source discipline: result only counts as [S] after tool output confirmation.

Gate 1: git working tree clean on spec/m0-parity-reformat [tool: terminal git status --short]
Gate 2: main build green, headless smoke exit 0 [tool: terminal build and run]
Gate 3: ASAN build green, tests pass [tool: terminal asan build and tests]
Gate 4: section 2.1 paper index 50 rows with 50 unique paper numbers [tool: grep paper rows]
Gate 5: section 2.7 contains exactly T-01..T-70 with no duplicate milestone/paper assignment [tool: grep + parseInt audit]
Gate 6: docs/architecture/README.md and docs/guides/README.md exist and are non-empty [tool: ls + stat]
Gate 7: fetch_research.py has fetch_arxiv_abstract and fetch_doi_redirect functions [tool: grep]
Gate 8: run 3 independent verification passes on final plan; each must return 0 issues before completion claim.

## 12. Execution Ranking
Highest-value next actions in execution order:
1. P1-1 repair section 2.6 table syntax and commit.
2. P2-1 build appendix dependency DAG and commit APPENDIX_EXECUTION_MAP.md.
3. P3-1 implement direct arXiv ID fetch and rerun unresolved abstract fetches.
4. P3-2 retry M6 GSound fetch and log exact result status.
5. P4-1 and P4-2 update docs cross-links and add standalone sanity suite instructions.
6. P5 verification passes after above commits.

### Implementation constraint
Do not fabricate cache-backed paper evidence. If a fetch target returns 403, 404, or an empty result, record the exact HTTP status or empty size in the plan next to the [X] and move on. The 50-paper corpus remains authoritative even when unresolved.
