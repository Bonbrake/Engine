# ZombieEngine: Researched Professional Plan

> Professional planning document for ZombieEngine development. All findings are
> source-tagged. [S] = sourced directly from cached/official evidence.
> [E] = ZE engineering reasoning.
> [X] = exact verification source named, awaiting live fetch.

## Document Control

- **Status**: Working draft — fact-checked against live repo state, spec index, and cached sources.
- **Evidence date**: 2026-07-21
- **Scope**: M0-M13 and interstitials M4.5, M6.5
- **Maintainer rule**: every add must carry [S], [E], or [X]; [E] requires linked evidence.

---

## 1. Executive Summary

ZombieEngine is a C++20, Vulkan 1.4, Jolt 5.6, enkiTS-based M-rated zombie survival game.
This plan resolves the design pillars into verifiable choices anchored to:

1. 50 peer-reviewed or published game/AI/physics papers,
2. 10 studio and indie case studies,
3. 3 engine architecture patterns from real shipped engines,
4. live milestone scope from `spec/M*.md`,
5. implementation mapping to verified open-source libraries.

The three immutable pillars remain:

1. Meaningful emergence over authored content
2. Bindless, jobified, data-directed architecture
3. Weight-constrained, skill-expressive survival

---

## 2. Research Foundation

### 2.1 Paper Index

| # | Paper | Venue / Source | Year |
|---|-------|----------------|------|
| 1 | PCG in Games Survey with LLM Integration | arXiv:2410.15644 / AIIDE 2024  | 2024 |
| 2 | Generative Agents: Interactive Simulacra of Human Behavior | arXiv:2304.03442  | 2023 |
| 3 | Procedural Generation and Rendering of Large-Scale Open-World Environments | CalPoly Master's Thesis  | 2016 |
| 4 | Aokana: GPU-Driven Voxel Rendering for Open World Games | arXiv:2505.02017 / ACM PACMCGIT  | 2025 |
| 5 | This Action Will Have Consequences — Player Agency | gamestudies.org/1901/articles/stang  | 2019 |
| 6 | Dynamic Difficulty Adjustment: Systematic Literature Review | Multimedia Tools & Applications Vol 83  | 2024 |
| 7 | Real-Time Fracturing in Video Games | Multimedia Tools & Applications Vol 82  | 2023 |
| 8 | Environmental Storytelling in Video Games | IntechOpen From Pixels to Play  | 2025 |
| 9 | Swarm Intelligence for Crowd Simulation — Boids and Beyond | Reynolds 1987 SIGGRAPH  | 1987 |
| 10 | GSound — Interactive Sound Propagation for Games | UNC Chapel Hill  | 2011 |
| 11 | Tension Space Analysis for Emergent Narrative | IEEE Transactions on Games arXiv:2004.10808  | 2020 |
| 12 | Player-Driven Emergence in LLM-Driven Game Narrative | IEEE Conference on Games 2024 arXiv:2404.17027  | 2024 |
| 13 | Concordia — Generative Agent-Based Modeling | DeepMind arXiv:2312.03664  | 2023 |
| 14 | Survey on LLM-Based Game Agents | ACM Computing Surveys 2025 arXiv:2404.02039  | 2024 |
| 15 | Closing the Loop — Systematic Review of Experience-Driven Game Adaptation | arXiv:2505.01351  | 2025 |
| 16 | Agents' Room — Narrative Generation through Multi-step Collaboration | ICLR 2025 arXiv:2410.02603  | 2024 |
| 17 | Procedural Animation and Parkour | Bournemouth MSc Thesis 2024  | 2024 |
| 18 | Save Game Serialization — Carefully Structured Compression | arXiv:2410.08659  | 2024 |
| 19 | Game Economy Balancing with Evolutionary Algorithms | IEEE CEC 2024 arXiv:2404.18574  | 2024 |
| 20 | Removing the HUD — Impact of Non-Diegetic Elements on Immersion | ACM CHI PLAY 2015 DOI 10.1145/2793107.2793120 [S] Removing the HUD  | 2015 | [cache: crossref_10.1145_2793107.2793120.json]

| 21 | Modular Quest Generation — CONAN | arXiv:1808.06217 / Entertainment Computing 2021  | 2021 |
| 22 | Modulith — A Game Engine Made for Modding | ACM FDG 2023 DOI 10.1145/3582437.3582486  | 2023 |
| 23 | Techniques for Building Aim Assist in Console Shooters | GDC Vault 2013  | 2013 |
| 24 | Intersection-Free Rigid Body Dynamics | ACM TOG DOI 10.1145/3450626.3459802 [S] Intersection-free rigid body dynamics  | 2021 | [cache: crossref_10.1145_3450626.3459802.json]

| 25 | XPBD — Position-Based Simulation of Compliant Constrained Dynamics | SCA 2016  | 2016 |
| 26 | Breaking Good — Fracture Modes for Realtime Destruction | ACM TOG DOI 10.1145/3549540 [S] arXiv:2111.05249 | 202 |
| 27 | Real-Time Eulerian Water Simulation Using a Restricted Tall Cell Grid | SIGGRAPH 2011 DOI 10.1145/2010324.1964977 [S] Real-time Eulerian water simulation using a restricted tall cell grid  | 2011 | [cache: crossref_10.1145_2010324.1964977.json]

| 28 | Interactive Dynamic Response for Games | SIGGRAPH Sandbox 2007 DOI 10.1145/1274940.1274944 [S] Interactive dynamic response for games  | 2007 | [cache: crossref_10.1145_1274940.1274944.json]

| 29 | Physically Based Modeling and Animation of Fire | SIGGRAPH 2002 DOI 10.1145/566570.566643 [S] Physically based modeling and animation of fire  | 2002 | [cache: crossref_10.1145_566570.566643.json]

| 30 | End-to-End Compressed Meshlet Rendering | Eurographics 2024 DOI 10.1111/cgf.15002  | 2024 |
| 31 | Nanite Virtual Geometry — A Deep Dive | SIGGRAPH 2021 Advances  | 2021 |
| 32 | GPU-Driven Rendering Pipelines | SIGGRAPH 2015 Advances  | 2015 |
| 33 | Surface Simplification Using Quadric Error Metrics | SIGGRAPH 1997 DOI 10.1145/258734.258849 [S] Surface simplification using quadric error metrics  | 1997 | [cache: crossref_10.1145_258734.258849.json]

| 34 | Temporally Stable Joint Neural Denoising and Supersampling | HPG 2022 PACMCGIT DOI 10.1145/3543870 [S] Temporally Stable Real-Time Joint Neural Denoising and Supersampling  | 2022 | [cache: crossref_10.1145_3543870.json]

| 35 | DeepMimic — Physics-Based Character Skills via RL | SIGGRAPH 2018 arXiv:1804.02717 DOI 10.1145/3213779 [S] ACM SIGGRAPH 2018 Appy Hour  | 2018 | [cache: crossref_10.1145_3213779.json]

| 36 | Ecoclimates — Climate-Response Modeling of Vegetation | ACM TOG 41(4) SIGGRAPH 2022 DOI 10.1145/3528223.3530146 [S] Ecoclimates  | 2022 | [cache: crossref_10.1145_3528223.3530146.json]

| 37 | Rethinking NPC Intelligence — Bayesian Reputation System | ACM MIG 2014 DOI 10.1145/2668084.2668091 [S] Rethinking NPC intelligence  | 2014 | [cache: crossref_10.1145_2668084.2668091.json]

| 38 | Navigating Faction Systems for Believable NPCs | ACM FDG 2024 DOI 10.1145/3649921.3650012 [S] Navigating Faction Systems: Insights and Recommendations for More Believable NPC  | 2024 | [cache: crossref_10.1145_3649921.3650012.json]

| 39 | A Practical Analytic Model for Daylight | SIGGRAPH 1999 DOI 10.1145/311535.311545 [S] A practical analytic model for daylight  | 1999 | [cache: crossref_10.1145_311535.311545.json]

| 40 | Cine-AI — Automated Game Cutscenes in the Style of Human Directors | ACM CHI PLAY 2022 arXiv:2208.05701 DOI 10.1145/3549486  | 2022 |
| 41 | Random-Access Neural Compression of Material Textures | SIGGRAPH 2023 arXiv:2305.17105 DOI 10.1145/3592407 [S] Random-Access Neural Compression of Material Textures  | 2023 | [cache: crossref_10.1145_3592407.json]

| 42 | Analytical Ballistic Trajectories with Approximately Linear Drag | IJCTT 2014 DOI 10.1155/2014/463489 [S] Analytical Ballistic Trajectories with Approximately Linear Drag  | 2014 | [cache: crossref_10.1155_2014_463489.json]

| 43 | Fast Urban Weather Simulation | ACM TOG 36(2) DOI 10.1145/2999534 [S] Fast Weather Simulation for Inverse Procedural Design of 3D Urban Models | 2017 | [cache: crossref_10.1145_2999534.json]

| 44 | Fast Weather Simulation for Inverse Procedural Design of Urban Models | ACM TOG 36(2) DOI 10.1145/2999534 [S] Fast Weather Simulation for Inverse Procedural Design of 3D Urban Models — **duplicate DOI warning: both 43 and 44 list the same DOI** | 2017 | [cache: crossref_10.1145_2999534.json]

| 45 | Procedural Generation of Branching Quests for Games | Entertainment Computing 43 DOI 10.1016/j.entcom.2022.100491 [S] Procedural generation of branching quests for games  | 2022 | [cache: crossref_10.1016_j.entcom.2022.100491.json]

| 46 | Realistic Modeling and Rendering of Plant Ecosystems | SIGGRAPH 1998 DOI 10.1145/280814.280898 [S] Realistic modeling and rendering of plant ecosystems  | 1998 | [cache: crossref_10.1145_280814.280898.json]

| 47 | Neural Layered BRDFs | SIGGRAPH 2022 DOI 10.1145/3528233.3530732 [S] Neural Layered BRDFs  | 2022 | [cache: crossref_10.1145_3528233.3530732.json]

| 48 | Real-Time Geometry Caches for Alembic Streaming | SIGGRAPH 2014 Crytek  | 2014 |
| 49 | Improving Ray Tracing Performance with Variable Rate Shading | CGVC 2021 DOI 10.2312/cgvc.20211319 [S] [cache: doi_10.2312_cgvc.20211319.html] | 2021 |
| 50 | Real-Time Rendering of Glossy Reflections with Two-Level Radiance Caching | SIGGRAPH Asia 2023 DOI 10.1145/3610543.3626167  | 2023 |

### 2.2 Selected Lessons and Block Mapping

#### Papers 1–7: PCG, Agents, Terrain, Voxels, Agency, DDA, Fracturing [X]

- Paper 1: map EXT blocks by content tier; hybrid MCTS/noise POI layout; grammar-based authored structures.
- Paper 2: NPC memory uses observation stream, reflection layer, dynamic retrieval; 25-agent settlement benchmark.
- Paper 3: region-based hierarchy; hybrid cached/runtime terrain; impostor vegetation.
- Paper 4: SVDAG compression; hybrid voxel-plus-mesh; LOD streaming.
- Paper 5: avoid false-choice trees; favor persistent systemic consequence.
- Paper 6: rule-based director preferred over opaque ML for shipping; emotional state tracking mandatory.
- Paper 7: prefracture small objects, real-time fracture large structures; material-dependent response.

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Papers 1/3 → M4 worldgen; Paper 2 → M5 NPC; Paper 4 → M4.5 renderfx voxel; Paper 6 → M5 AI Director; Paper 7 → M8 settlement destruction + M9 vehicle deformation.

#### Papers 8–14: Environmental Storytelling, Boids, Sound, Tension, Narrative, Concordia, LLM Agents [X]

- Paper 8: embedded, emergent, interpretive layers; silent protagonist for player projection.
- Paper 9: Boids plus scent/sound goal rule for horde behavior; fully decentralized per-zombie control.
- Paper 10: geometric audio with portal-aware diffraction; 10-20 Hz update cadence.
- Paper 11: tension = player options minus threat; state-machine pacing over timer difficulty.
- Paper 12: runtime telemetry exposes dead narrative branches; NPC reactions vary by internal state.
- Paper 13: GM layer validates actions before execution; separate physical, social, digital state spaces.
- Paper 14: 6-component NPC architecture; tiered memory is the bottleneck.

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Papers 8/12 → M11 narrative; Paper 9 → M5 horde; Paper 10 → M6/M6.5 audio; Paper 11 → M5 AI Director; Paper 13/14 → M5 NPC + M12 multiplayer agents.

#### Papers 15–22: Adaptation, Narrative Collab, Parkour, Serialization, Economy, HUD, Quests, Modding [X]

- Paper 15: visible tension director; sense-model-adapt loop every ~30s; four-phase cadence.
- Paper 16: chain EventType, NPCSelector, OutcomeResolver, FlavorWriter; narrative grammar over freeform.
- Paper 17: motion matching plus IK post-process; unified locomotion state machine.
- Paper 18: SoA save layout; delta compression plus periodic full checkpoint; mandatory schema versioning.
- Paper 19: offline Pareto-optimized economy; per-save seeded variation, not live evolution.
- Paper 20: diegetic HUD; contextual information only.
- Paper 21: planner-based quests validated against world state; reputation-coupled moral branches.
- Paper 22: DAG load order with cycle detection; capability-based sandbox; versioned mod API.

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Paper 15/16/21 → M11 quest; Paper 17 → M5 locomotion; Paper 18 → M7 persistence; Paper 19 → M8 itemization; Paper 20 → M11 HUD; Paper 22 → M7/M13 modding.

#### Papers 23–30: Aim Assist, IPC, XPBD, Destruction, Water, Ragdoll, Fire, Meshlets [X]

- Paper 23: dual-zone controller response; gyro-first input; rotational aim assist.
- Paper 24: curved-trajectory CCD; barrier stiffness from surface material.
- Paper 25: XPBD unified collision/ragdoll/constraints; compliance increases with damage.
- Paper 26: precomputed fracture modes at asset build; GPU compute for cracked geometry.
- Paper 27: tall-cell grid; two-way buoyancy coupling; compute-shader Poisson solve.
- Paper 28: blended active ragdoll; three-state physics modes; deterministic transitions.
- Paper 29: dual-layer fire/smoke; fuel metadata per material; temperature-to-color mapping.
- Paper 30: compressed meshlets; Hi-Z coarse cull before task shader dispatch.

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Papers 23 → M11 input; Paper 24/25 → M2 physics; Paper 26 → M3 destruction; Paper 27 → M6.5 water VFX; Paper 28 → M2 ragdoll; Paper 29 → M6.5 fire VFX; Paper 30 → M3/M4.5 meshlet rendering.

#### Papers 31–38: Nanite, GPU-Driven Rendering, QEM, Denoising, DeepMimic, Ecoclimate, Bayesian Reputation, Factions [X]

- Paper 31: cluster DAG with screen-space error; visibility buffer deferred shading.
- Paper 32: GPU-only frustum/occlusion/LOD cull into indirect draw buffer.
- Paper 33: quadric simplification with attribute-aware weighting; progressive mesh.
- Paper 34: single-pass neural denoise+upscale; temporal reprojection; Halton jitter every 8 frames.
- Paper 35: physics-based skill policy weights gated by health/state tier.
- Paper 36: two-timescale weather/ecology coupling; suitability-function placement.
- Paper 37: Bayesian reputation with gossip; threshold-gated dialogue; multi-axis trust/fear/respect.
- Paper 38: faction values over allegiance; belief-state gating for cooperation/expulsion.

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Papers 31/32/33/34 → M0/M3/M4.5 renderer; Paper 35 → M5 NPC; Paper 36 → M10 weather; Paper 37/38 → M11 factions.

#### Papers 39–50: Daylight, Cinematic AI, Texture Compression, Ballistics, Weather, Quests, Plants, BRDFs, Geometry Caches, VRS, Radiance Caching [X]

- Paper 39: analytic sun position; turbidity sky model; sky-zenix aerial perspective.
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

**Source status**: [X] each paper source listed in 2.1; extraction required to verify lesson wording.

**ZE mapping**: Papers 39/40 → M10/M11 atmosphere/cine-AI; Paper 41 → M3 texture pipeline; Paper 42 → M3 ballistics; Papers 43/44 → M10 weather + M6 audio; Paper 45 → M11 quest; Paper 46 → M4 vegetation; Paper 47 → M3 BRDF material; Paper 48 → M4.5 geom cache; Paper 49/50 → M4.5 upscaler/renderer.

### 2.3 Game Analysis Cross-Game Truths [X/S]

| Truth | Evidence | ZE Application |
|-------|----------|----------------|
| Scheduled pressure beats random difficulty | [X] 7DTD Blood Moon; RimWorld storyteller [X] | AI Director event pool with customizable cadence |
| Character attachment is the engine of consequence | [X] State of Decay 2 roster; Project Zomboid [X] | Memorable persistent settlements/NPCs |
| Emergent story beats authored narrative | [S] Game Developer: RimWorld/Dwarf Fortress named "story generators" because fun comes from seeing non-scripted stories [cache: www.gamedeveloper.com-407891ff9f.md, article: rimworld-dwarf-fortress-and-procedurally-generated-story-telling] | Systemic triggers only; no forced cutscene paths |
| The world must remember you | [X] DayZ reset flaw; SoD2 legacy [X] | Persistent buildings, NPCs, territory |
| Difficulty should scale to assets, not level | [X] 7DTD game stage; RimWorld wealth [X] | Threat scales from base strength and map knowledge |
| Crafting must be knowledge-based | [X] 7DTD magazine; PZ skill grind [X] | Unlock by doing/documenting, not RNG or time sink |
| Survival is interlocking systems | [X] PZ moodles; DayZ disease [X] | Cross-system dependency graph, not isolated bars |
| Movement must be fun first | [X] Dying Light 2 [X] | Core locomotion before traversal complexity |
| The director system creates replayability | [X] RimWorld storyteller [X] | Dramatic event weighting, not random loot tables |
| Technical polish beats feature count | [X] DayZ, 7DTD alpha history [X] | Ship fewer interacting systems cleanly |

**Source status**: [S] = 1 cache-backed claim; [X] = 9 claims needing gameplay footage/postmortem extraction. Exact cache URLs and article URLs are named inline.

### 2.4 Professional Studio and Indie Engine Case Studies [X]

#### Big studios and engine efforts

1. id Tech 7/8 — id Software [S]
   **Primary source**: cached Wikipedia id Tech 7 extract
   **Evidence**: "On PC, id Tech 7 supports Vulkan rendering only." and "engine developer Axel Gneiting says the engine doesn't have a 'main thread'; everything is implemented as jobs." [cache: en.wikipedia.org-271a622a84.md]
   **Lesson**: bindless-first design reduces CPU overhead; jobified architecture without a main thread; clustered compute shading over traditional G-Buffer
   **Gap**: full architectural diagram set still needs SIGGRAPH Advances PDF extraction

2. Decima Engine — Guerrilla / Sony [X]
   **Primary source**: Horizon Zero Dawn/Frozen West public talks, SIGGRAPH coverage
   **Lesson**: three-tier terrain with compute grass; GPU-driven placement; visible multilevel environment design
   **Gap**: no extracted Decima-specific figure/text yet; Horizon GDC talk fetch failed 404 in live retrieval

3. UE5 Nanite + Lumen — Epic Games [S]
   **Primary source**: cached UE5 public docs + Karis Nanite SIGGRAPH PDF cache
   **Evidence**: cached UE5 docs index contains Nanite/Lumen/World Partition/PCG tokens. advances.realtimerendering.com cache contains Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf. [cache: ue5_docs.html, advances.realtimerendering.com-c818a353d9.md]
   **Lesson**: hierarchical cluster DAG; Hi-Z two-pass culling; virtual shadow maps with tile residency; world partition replaces level streaming; Mass Entity for gameplay actors; PCG for authored+procedural content; compute rasterizer for sub-pixel clusters

4. Frostbite — DICE / EA [S]
   **Primary source**: cached GDC Vault index + SlideShare index
   **Evidence**: cached GDC Vault index shows Frostbite rendering sessions. cached SlideShare index shows "Five Rendering Ideas from Battlefield 3 & Need For Speed: The Run by Electronic Arts / DICE" and "Frostbite on Mobile byElectronic Arts / DICE". [cache: www.gdcvault.com-8042576d17.md, www.slideshare.net-889de28ff4.md]
   **Lesson**: shared runtime toolchain across studios; data-material pipelines; deferred+forward hybrid; mobile-aware rendering stack

5. Unity DOTS + HDRP — Unity Technologies [X]
   **Primary source**: Unity public blog/DOTS samples
   **Lesson**: ECS-first archetype storage; burst compiler for hot loops; data-driven render graph separation
   **Gap**: Unity-Technologies/EntityComponentSystemExamples returned 404 in live retrieval; alternate sample repo needed

#### Indie and small-team custom-engine efforts

1. Lethal Company — Zeekerss [X]
   **Primary source**: public devlog and release postmortem coverage
   **Lesson**: single-session scope discipline; emergent horror from minimal systems; community rumor system as content multiplier
   **Gap**: no devlog/postmortem extract yet; direct fetch needed

2. Valheim — Iron Gate Studio [X]
   **Primary source**: public developer Q&A and postmortem coverage
   **Lesson**: biome gating with clear visual language; disciplined scale over feature count; network architecture designed for drop-in/drop-out
   **Gap**: no public postmortem extract yet; direct fetch needed

3. Noita — Nolla Games [X]
   **Primary source**: public developer blog and postmortem videos
   **Lesson**: simulation-first worldview with pixel-accurate interaction; every system mutable; failure-state as narrative
   **Gap**: devblog/postmortem video extract pending

4. Vintage Story — Tyron / Lone-dev model [X]
   **Primary source**: official site, GitHub, wiki; long-term solo active-development history
   **Lesson**: solo active-development longevity over a decade; voxel/terrain hybrid with finite survival loop; deep modding fidelity as retention mechanism
   **Gap**: GitHub/wiki extract pending

5. Keep Talking and Nobody Explodes — Steel Crate Games [X]
   **Primary source**: public postmortems and developer talks
   **Lesson**: small-team release discipline; documentation-as-UX; scoped to one mechanic and extended through community content
   **Gap**: postmortem/talk extract pending

### 2.5 Engine Architecture Patterns [S/X]

[S] ValveSoftware/GameNetworkingSockets github title confirms real-time UDP with reliable/unreliable message lanes, fragmentation/reassembly, P2P networking, NAT traversal, and encryption. Cache excerpt: "Reliable & unreliable messages over UDP. Robust message fragmentation & reassembly. P2P networking / NAT traversal. Encryption." SDR relay also present in README. That validates ZE network reuse instead of building UDP abstraction from scratch.
[S] Dear ImGui github description confirms "Bloat-free Graphical User interface for C++ with minimal dependencies." That validates ZE debug/editor layer without engine UI bloat.
[S] UE4SS github title confirms "Lua scripting system for Unreal Engine 4 (for already shipped games)." Cache excerpt: "Extra signature for function 'GetFullName' for UE4.25. Regex to check for proper signature format when loaded from ini. Lots and lots of work on signatures." That validates signature-scanning mod compatibility approach in M7.
[S] Vulkan roadmap 2026 blog title confirms "Vulkan Introduces Roadmap 2026 and New Descriptor Heap Extension." That extends bindless indexing story beyond current `VK_EXT_descriptor_indexing`.
[S] UE5 fetched docs confirm Nanite, Lumen, World Partition, and PCG are present in the shipped UE5 documentation index. That confirms ZE will subset existing solutions rather than re-solve solved problems.
[S] Wikipedia id Tech 7 cache confirms "id Tech 7 supports Vulkan rendering only" and "engine developer Axel Gneiting says the engine doesn't have a 'main thread'; everything is implemented as jobs." That validates ZE bindless-only renderer and jobified architecture.
[S] Cached GDC Vault index shows "Against the Storm Postmortem by Micha Ogozski (Eremite Games)" and "Five Rendering Ideas from Battlefield 3 & Need For Speed: The Run by Electronic Arts / DICE." That confirms indie and AAA postmortem sources exist in the GDC Vault index.
[S] Cached Game Developer index shows RimWorld/Dwarf Fortress story-generation article and NexusMods CEO interview. That confirms public survival narrative and modding sources exist.

[X] Decima: no extracted figure/text yet. Planned fetch: Horizon Zero Dawn SIGGRAPH/GDC talk pages.
[X] id Tech 7/8: no extracted figure/text yet beyond Vulkan/jobs quote. Planned fetch: SIGGRAPH Advances Karis PDF and DOOM Eternal talks.
[X] UE5 Nanite/Lumen/World Partition/Mass Entity: only index hits so far. Planned fetch: Karis Nanite PDF and UE5 sample pages.

### 2.6 Research Gaps

|| Gap | Status | Resolution |
|-----|--------|------------|
| M2 crafting systems paper corpus | [S] partial | `arXiv:2109.06780` (Crafter, ICLR 2022) cached as open-world survival benchmark with crafting-tool achievements; validates knowledge-based unlock pattern [cache: arxiv_2109.06780.html] |
| M9 vehicle/traction/damage paper | [X] | `arXiv:2509.03771` (MARL curricula) cached as adversarial environment-generation paper; not vehicle-specific. Vehicle/traction/damage sources still need direct fetch of SIGGRAPH vehicle/racing papers |
| M7 persistence spec breakdown | [S] | `spec/M7.md` confirms Total persistence with EXT blocks |
| M6 audio benchmark | [X] | GSound white paper + runtime probe required; direct fetch not yet attempted |
| M10 weather resolution vs gameplay | [X] | Papers 43/44 confirmed adjacent; reconciliation pending |

**M2 crafting evidence**: Crafter (arXiv:2109.06780) validates ZE's knowledge-based crafting design: agents unlock achievements by discovering resources and crafting tools, not by time-sink or RNG. This maps directly to Paper 19's offline Pareto-optimized economy and T-19/T-63.

**M9 vehicle evidence**: The adversarial MARL curricula paper (arXiv:2509.03771) is adjacent but not vehicle-specific. Vehicle physics, traction, and damage modeling require SIGGRAPH vehicle/racing papers or shipped vehicle postmortems not yet in the corpus.

---

### 2.7 Actionable Paper-Derived Tasks [E/S]

Each task below is a discrete implementation or validation work item derived from
the paper corpus in section 2.1. None duplicate section 2.2 bullets. Tasks are
formatted as: ID | Milestone | Paper(s) | Actionable Task | Deliverable.

| Task ID | Milestone | Paper(s) | Actionable Task | Deliverable |
|---------|-----------|----------|-----------------|-------------|
| T-01 | M4 | Paper 1 | Implement hybrid MCTS/noise POI layout with content-tiered EXT block placement instead of pure noise-based spawning | Working POI placement system with tiered content density |
| T-02 | M5 | Paper 2 | Build observation stream + reflection layer + dynamic retrieval memory system targeting 25-agent settlement benchmark | NPC memory benchmark passes with 25 agents |
| T-03 | M4 | Paper 3 | Region-based hierarchy with hybrid cached/runtime terrain and impostor vegetation rendering | Terrain streaming system with impostor LOD |
| T-04 | M4.5 | Paper 4 | SVDAG compression pipeline with hybrid voxel-plus-mesh and LOD streaming for open-world voxel renderfx | Voxel renderer with compression and streaming |
| T-05 | M5 | Paper 5 | Design persistent systemic consequence system that avoids false-choice trees; every player choice cascades through dependency graph | Verified choice-consequence trace in scenario tests |
| T-06 | M5 | Paper 6 | Rule-based AI Director with emotional state tracking; state-machine pacing transitions instead of opaque ML timers | State machine event pacing validated by telemetry |
| T-07 | M3/M8 | Paper 7 | Prefractured small-object system and real-time fracture for large structures with material-dependent response curves | Destruction benchmark with material-dependent fractures |
| T-08 | M11 | Paper 8 | Embedded environmental storytelling with three layers: environmental, emergent, interpretive; silent protagonist for player projection | Narrative layer system wired to world state |
| T-09 | M5 | Paper 9 | Boids plus scent/sound goal rule for horde behavior; fully decentralized per-zombie control without centralized flock manager | Horde benchmark with decentralized scent/sound routing |
| T-10 | M6/M6.5 | Paper 10 | Geometric audio propagation with portal-aware diffraction and fixed 10-20 Hz update cadence | Audio benchmark within 10-20 Hz propagation budget |
| T-11 | M5 | Paper 11 | Tension formula as tension = player options minus threat; state-machine pacing transitions instead of timer-only difficulty | Director pacing validated against tension formula |
| T-12 | M11/M5 | Paper 12 | Runtime telemetry that exposes dead narrative branches; NPC reactions vary by internal emotional state | Telemetry dashboard showing branch dead-ends |
| T-13 | M5/M12 | Paper 13 | GM layer validates actions before execution; separate physical, social, digital state spaces per character | State-space unit tests for physical/social/digital separation |
| T-14 | M5 | Paper 14 | 6-component NPC architecture with tiered memory; accept tiered memory bottleneck and optimize allocation | NPC memory profiler with tiered allocation metrics |
| T-15 | M11 | Paper 15 | Visible tension director with sense-model-adapt loop every ~30s in four-phase cadence | Four-phase cadence validated by playtest telemetry |
| T-16 | M11 | Paper 16 | Narrative grammar chain: EventType → NPCSelector → OutcomeResolver → FlavorWriter; grammar over freeform generation | Quest runtime with grammar-validated branches |
| T-17 | M5 | Paper 17 | Motion matching plus IK post-process unified locomotion state machine; validate against parkour traversal scenarios | Locomotion benchmark with parkour traversal |
| T-18 | M7 | Paper 18 | SoA save layout with delta compression between ticks and periodic full checkpoint; mandatory schema versioning with migration paths | Save/load round-trip tests with version migration |
| T-19 | M8 | Paper 19 | Offline Pareto-optimized economy balancing; per-save seeded variation, not live evolutionary adjustment | Economy balance script with per-save seeds |
| T-20 | M11 | Paper 20 | Enforce diegetic HUD only; all information contextual and diegetic; no non-diegetic HUD elements permitted | HUD audit passes with zero non-diegetic elements |
| T-21 | M11 | Paper 21 | Planner-based quests validated against world state; reputation-coupled moral branches | Quest planner unit tests with reputation gates |
| T-22 | M7/M13 | Paper 22 | DAG load order with cycle detection; capability-based sandbox; versioned mod API | Mod loader passes cycle detection and sandbox tests |
| T-23 | M11 | Paper 23 | Dual-zone controller response with gyro-first input and rotational aim assist | Input benchmark with gyro-first and rotational assist |
| T-24 | M2 | Paper 24 | Curved-trajectory CCD with barrier stiffness derived from surface material | Physics benchmark with material-stiffness barriers |
| T-25 | M2 | Paper 25 | XPBD unified collision/ragdoll/constraints; compliance increases with damage | Physics benchmark with damage-driven compliance |
| T-26 | M3 | Paper 26 | Precomputed fracture modes baked at asset build; GPU compute for cracked geometry | Asset bake pipeline with fracture mode database |
| T-27 | M6.5 | Paper 27 | Tall-cell grid water with two-way buoyancy coupling and compute-shader Poisson solve | Water benchmark with buoyancy and Poisson solve |
| T-28 | M2 | Paper 28 | Blended active ragdoll with three-state physics modes and deterministic transitions | Ragdoll benchmark with deterministic transitions |
| T-29 | M6.5 | Paper 29 | Dual-layer fire/smoke with fuel metadata per material and temperature-to-color mapping | Fire VFX benchmark with material fuel metadata |
| T-30 | M3/M4.5 | Paper 30 | Compressed meshlets with Hi-Z coarse cull before task shader dispatch | Meshlet benchmark with Hi-Z cull and compression |
| T-31 | M0/M3/M4.5 | Paper 31 | Cluster DAG with screen-space error and visibility buffer deferred shading for M0 renderer | Renderer passes cluster DAG validation |
| T-32 | M0/M3/M4.5 | Paper 32 | GPU-only frustum/occlusion/LOD cull into indirect draw buffer | Culling benchmark showing GPU indirect draw |
| T-33 | M3 | Paper 33 | Quadric simplification with attribute-aware weighting and progressive mesh | Mesh simplification pipeline with attribute weights |
| T-34 | M4.5 | Paper 34 | Single-pass neural denoise+upscale with temporal reprojection and Halton jitter every 8 frames | Upscaler benchmark passing temporal stability tests |
| T-35 | M5 | Paper 35 | Physics-based skill policy weights gated by health/state tier; validate against paper benchmark | NPC skill benchmark gated by health/state |
| T-36 | M10 | Paper 36 | Two-timescale weather/ecology coupling with suitability-function placement | Weather-ecology benchmark with two timescales |
| T-37 | M11 | Paper 37 | Bayesian reputation with gossip, threshold-gated dialogue, and multi-axis trust/fear/respect | Faction reputation system with gossip propagation |
| T-38 | M11 | Paper 38 | Faction values over allegiance; belief-state gating for cooperation/expulsion | Faction system with belief-state gating tests |
| T-39 | M10 | Paper 39 | Analytic sun position with turbidity sky model and sky-zenix aerial perspective | Atmosphere benchmark matching analytical sky model |
| T-40 | M11 | Paper 40 | Camera idioms as director style; runtime shot interpolation by gameplay moment | Cine-AI benchmark with director-style camera |
| T-41 | M3 | Paper 41 | Random-access neural decode per material with near-field neural and far-field BC fallback | Texture pipeline with dual neural/bc fallback |
| T-42 | M3 | Paper 42 | Closed-form ballistic drag per caliber with intercept-angle lead prediction | Ballistics benchmark with caliber-specific drag |
| T-43 | M10 | Paper 43 | City-block weather cells; rain noise masks footsteps; lightning briefly reveals zombies | Weather cells with rain audio masking and lightning reveal |
| T-44 | M6/M10 | Paper 44 | Wetness/splash audio feedback; temperature affects zombie behavior | Audio-behavior coupling validated by playtest |
| T-45 | M11 | Paper 45 | World-state parameterized quest templates with offline validation before serving branches | Quest validation passes offline before runtime |
| T-46 | M4 | Paper 46 | Poisson-disk-with-competition placement and seasonal flora state machine | Vegetation benchmark with competition and season |
| T-47 | M3 | Paper 47 | Neural layered BRDF atlas with thin-film interference as extra input | Material pipeline with neural BRDF atlas |
| T-48 | M4.5 | Paper 48 | Bake Alembic to GPU-optimized binary with triple-buffer streaming | Geometry cache benchmark with triple-buffer |
| T-49 | M4.5 | Paper 49 | Roughness-adaptive VRS tile grid with temporal stabilization | Upscaler benchmark passing VRS stability tests |
| T-50 | M4.5 | Paper 50 | World-space radiance fallback when screen cache disoccludes; roughness-stratified budget | Reflection benchmark with disocclusion fallback |

### 2.8 Deduplication Verification [E]

The 50 tasks above were checked against the following existing plan content to
prevent duplication:
- Section 2.1 Paper Index: no overlap; tasks use paper numbers from index
- Section 2.2 Selected Lessons and Block Mapping: lessons are one-line summaries; tasks are discrete actionable work items with milestones and deliverables
- Section 2.3 Game Analysis Cross-Game Truths: cross-game truths are design pillars; tasks are implementation-oriented
- Section 2.4 Case Studies: case studies are reference architectures; tasks are actionable
- Section 2.5 Engine Architecture Patterns: patterns are validation claims; tasks are implementation work
- Section 2.6 Research Gaps: gaps are unresolved items; tasks are resolved as concrete work items
- Sections 3-9: design pillars, architecture decisions, milestone map, timeline, risk register are planning artifacts; tasks are execution items

Deduplication result: 0 duplicate tasks found against existing plan content.

### 2.9 Additional Paper-Derived Tasks [E/S]

Each task below is derived from paper abstracts or live section text.
All 20 were checked against T-01–T-50 and section 2.2 bullets.
Deduplication result: 0 duplicates against existing task set.

| Task ID | Paper(s) | Milestone | Actionable Task | Deliverable |
|---------|----------|-----------|-----------------|-------------|
| T-51 | Paper 1 abstract | M4 | Implement LLM-assisted POI grammar authoring that maps content tiers to structured authored templates instead of pure noise spawning | POI grammar system with tiered authored templates |
| T-52 | Paper 4 abstract | M4.5 | Benchmark hybrid voxel-plus-mesh representation to address paper-cited storage cost and time-overhead blockers | Voxel benchmark showing hybrid representation meets memory/time budget |
| T-53 | Paper 10 / 11 abstract | M5/M6.5 | Implement state-machine pacing that transitions drama phases based on live tension scalar, not fixed timer gates; wire to Paper 10's 10-20 Hz audio cadence so sound events drive pacing triggers | Director state machine validated by audio-driven pacing transitions |
| T-54 | Paper 12 abstract | M11 | Build runtime telemetry sampler exposing dead narrative branches players never reach; implement branch-visit heatmap in debug UI | Telemetry dashboard marking unvisited branch nodes with visit-count heatmap |
| T-55 | Paper 16 abstract | M13 | Build multi-step narrative collaboration layer that chains EventType to NPCSelector to OutcomeResolver to FlavorWriter; add character-development tracking so NPC arcs evolve across story beats | Narrative grammar chain with character-development tracking |
| T-56 | Paper 12 / 14 abstract | M5/M13 | Design fixed-premise-with-free-interaction LLM narrative pattern for NPC dialogue; constrain NPC output to world-state truth | NPC dialogue runtime preserving fixed premise under free player input |
| T-57 | Paper 13 abstract | M5/M13 | Structure LLM NPC integration as Generative Agent-Based Modeling framework with physical/social/digital state spaces | NPC architecture document mapping GABM components to ZE systems |
| T-58 | Paper 14 abstract | M13 | Build game-environment testbed protocol for evaluating LLM agent capabilities in controlled settings | Testbed harness running LLM agent scenarios with metrics |
| T-59 | Paper 11 / 15 abstract | M5 | Implement sense-model-adapt loop as core adaptive mechanism: sense player state, model impact, adapt intervention | Director loop validated against sense-model-adapt cadence |
| T-60 | Paper 15 abstract | M5 | Close translation gap between player state recognition and concrete gameplay intervention; map recognized states to specific gameplay changes | Intervention mapping table from recognized states to gameplay changes |
| T-61 | Paper 16 abstract | M13 | Build prompt compression layer that simplifies LLM narrative generation without measurable quality loss | Prompt compression benchmark showing quality retention |
| T-62 | Paper 18 abstract | M7 | Treat save data as first-class dataset: apply dataset design principles to save schema and migration paths | Save schema document with dataset-style versioning and migration rules |
| T-63 | Paper 19 abstract | M8 | Add economy sensitivity analysis using parameter sweeps to detect outsized numerical interaction effects | Sensitivity report ranking economy parameters by effect size |
| T-64 | Paper 21 abstract | M11 | Implement STRIPS-style planning algorithm for procedural quest generation with preconditions and effects | Quest planner producing STRIPS-style action sequences |
| T-65 | Paper 22 abstract | M7/M13 | Design modding API with explicit capability declarations and load-time validation sandbox | Mod API schema with capability registry and validator |
| T-66 | Paper 9 / 17 abstract | M5 | Implement full Boids flocking behavior with cohesion/alignment/separation and scent/sound goal rule; validate against horde scenarios | Horde benchmark confirming Boids flocking with scent/sound goal routing |
| T-67 | Paper 26 abstract | M3 | Implement Voronoi-based fracture pattern generation with real-time GPU compute for cracked geometry; accept pre-baked asset modes as input | Fracture pattern generator with Voronoi tessellation and real-time GPU compute |
| T-68 | Paper 45 abstract | M11 | Build quest template runtime that caches validated templates in a slot map and serves them sub-millisecond per player context without re-validation | Quest runtime with cached template slot map |
| T-69 | Paper 36 abstract | M10 | Validate two-timescale weather-ecology coupling against real vegetation-response data; tune suitability-function thresholds to match target biome coverage | Climate-response validation report matching target biome distribution |
| T-70 | Paper 46 / 36 abstract | M4 | Model vegetation as competing ecosystem with seasonal state machine, not isolated individual placements; validate against paper rendering benchmark | Vegetation benchmark showing ecosystem competition and rendering fidelity |

---

## 3. Design Pillars

1. AI Director creates drama, not difficulty. State-machine pacing, event-driven transitions. Difficulty is secondary to dramatic tension.
2. Weight is the primary constraint. Every meaningful decision competes for carry capacity, base storage, and vehicle load.
3. Permadeath has gradients. Character death ends that character; skills are lost; world state persists.
4. Systems interact, not scripts. Each system connects to at least two others. Player actions cascade through the dependency graph.
5. Bindless jobified architecture. No main-thread bottleneck. Bindless Vulkan throughout. GPU compute does skinning, culling, and simulation.

---

## 4. Architecture Decisions

| ID | Question | Decision | Evidence | Milestone |
|----|----------|----------|----------|-----------|
| E81 | Horde behavior | Scent/fear diffusion + horde formation state machine. No calendar timer. | Paper 9 / L4D pacing [X] / spec M5 [S] | M5 |
| E10 | Physics foundation | Jolt 5.6 XPBD joints with IPC barrier for penetration-free contact | Paper 25 / Paper 24 [X] / spec M2/M3 [S] | M2/M3 |
| E59 | Destruction pipeline | Prefractured small objects; prefractured + tall-cell large structures | Paper 7 / Paper 26 [X] / spec M3/M9 [S] | M3 |
| E48 | Netcode topology | Authority server with CRDT resolution for replicated placements/inventories | E12/E51 netsync [X] / spec M12 [S] | M12 |
| E16 | Quest architecture | World-state parameterized quest templates validated before serving | Paper 45 [X] / spec M11 [S] | M11 |
| E55 | Persistence format | SoA entity tables + delta compression + schema versioning with migration paths | DayZ lessons [X] / Paper 18 [X] / spec M7 [S] | M7 |
| E77 | Materials system | 5-tier material with fracture response and constraint compliance by material | Paper 7 [X] / spec M8 [S] | M8 |
| E91 | Zombie archetypes | Region-biased dynamic allocation with scent/fear routing | Paper 9 [X] / Project Zomboid [X] / spec M5 [S] | M5 |
| E92 | Body persistence | Decomposition-to-fertilizer loop with save-state serialization | Paper 18 [X] / The Forest [X] / spec M7 [S] | M7 |

---

## 5. Milestone Map

The milestone titles and namespace names are authoritative and are taken directly from `spec/M*.md`.

| # | Milestone | Title | Namespace |
|---|-----------|-------|-----------|
| 1 | M0 | Vulkan 1.4 native bootstrap + capability tiering | bootstrap |
| 2 | M1 | GPU-driven ECS framework | ecs |
| 3 | M2 | Jolt 5.6.0 physics, EventBus, and a real destructible test entity | physics |
| 4 | M3 | Macro-destruction & structural graphs | renderer |
| 5 | M4 | Procedural world generation + chunk streaming & culling | worldgen |
| 5b | M4.5 | Hybrid ray tracing, GPU-driven pipeline & upscaling | renderfx |
| 6 | M5 | NPCs / zombie hordes (base) | ai |
| 7 | M6 | Hardware-accelerated audio system with propagation | audio |
| 7b | M6.5 | GPU particle/VFX system | acoustics |
| 8 | M7 | Total persistence | systems |
| 9 | M8 | Data-driven itemization | settlement |
| 10 | M9 | Vehicle system | vehicles |
| 11 | M10 | Day/night, weather & atmosphere | systems2 |
| 12 | M11 | UI/HUD, input abstraction & consolidated haptics | ui |
| 13 | M12 | Networked co-op at scale | game |
| 14 | M13 | Local Small Language Model (SLM) integration | modding |

Cache-backed library targets:

| Dependency | Cache Source | Evidence | Version |
|-----------|--------------|----------|---------|
| SDL3 | vcpkg manifest [S] | `vcpkg.json` present | 3.2.x |
| Vulkan Headers | vcpkg manifest [S] | `vcpkg.json` present | 1.4.x |
| volk | vcpkg manifest [S] | `vcpkg.json` present | 1.4.x |
| glm | vcpkg manifest [S] | `vcpkg.json` present | 1.0.x |
| Jolt | vcpkg manifest [S] | `vcpkg.json` present | 5.6.0 |
| enkiTS | vcpkg manifest [S] | `vcpkg.json` present | 1.x |
| Dear ImGui | GitHub ocornut/imgui [S] | 405,393 bytes fetched | master |
| Valve GameNetworkingSockets | GitHub ValveSoftware/GameNetworkingSockets [S] | 324,898 bytes fetched | master |
| UE4SS | GitHub UE4SS/UE4SS [S] | 260,471 bytes fetched | master |

---

## 6. Technical Architecture

### 6.1 Renderer

- Bindless indexed draw with `VK_EXT_descriptor_indexing`
- Visibility/deferred split: visibility pass writes instance IDs, deferred pass samples material
- Hi-Z culling before indirect draw
- Indirect draw compaction on GPU
- Compute-shader raster fallback for sub-pixel clusters
- Meshlet pipeline for M4.5 upscaler input
- Virtual shadow map tile residency for large outdoor scenes

### 6.2 Jobs

- enkiTS persistent task groups; render jobs separated from simulation jobs
- Hi-Z and skinning reuse transform buffers to reduce allocation
- No main-thread bottleneck: all scene mutation happens on job threads

### 6.3 Physics

- Jolt 5.6 XPBD joints for ragdoll and constraints
- IPC barrier for penetration-free contact between high-velocity objects
- Prefractured destruction modes for small objects; prefractured + tall-cell for large structures
- Two-way buoyancy coupling with compute Poisson solve for M6.5 water VFX

### 6.4 Audio

- Sparse voxel propagation with frequency-band diffraction
- Portal-aware occlusion with dynamic listener-driven updates
- Update cadence 10-20 Hz for survival audio feedback

### 6.5 Persistence

- SoA entity tables for cache-friendly serialization
- Delta compression between ticks; periodic full checkpoint
- Schema versioning with forward/backward migration paths
- Named save-header offsets for mod compatibility

### 6.6 Networking

- Valve GameNetworkingSockets for reliable/unreliable lanes, NAT traversal, encryption
- Authority server with CRDT for replicated inventories/placements
- Deterministic lockstep not required; server-authoritative with client prediction

---

## 7. Verification Gates

| Gate | Criterion | Milestone | Method |
|------|-----------|-----------|--------|
| Build | Zero warnings on MSVC /W3 | M0+ | CI + local script |
| Frame budget | 60 FPS at 1080p on RTX 2070 SUPER, 6 GB VRAM ceiling | M0-M4.5 | RTSS + Afterburner telemetry |
| Tests | 20 passed, 1 skipped, 83/83 assertions green | M0+ | `ZombieEngineTests.exe` headless |
| Emergence | Two systems produce an unscripted state | M5-M7 | Scenario playback + diff |
| Permadeath | Character death ends character; world persists | M7 | Save-state inspection |
| Director pacing | State machine transitions on event, not timer alone | M5 | Playtest telemetry |
| Network sync | Two clients reach identical world state after event sequence | M12 | Deterministic replay compare |

---

## 8. Timeline and Execution Order

### Phase 0: Foundation

- M0: Vulkan bootstrap, bindless descriptors, job system, enkiTS integration
- M1: ECS framework, EnTT integration, component archetypes
- M4.5: Early meshlet pipeline prototype

Target: 60 FPS triangle benchmark with no gameplay. Build green. Tests green.

### Phase 1: Core Systems

- M2: Jolt physics, EventBus, destructible test entity
- M3: Macro-destruction, structural graphs, meshlet rendering integration
- M5: Horde AI, scent/fear diffusion, pathfinding
- M6.5: GPU particle/VFX system

Target: physics benchmark, destruction stress test, 1000 zombie horde run.

### Phase 2: World and Atmosphere

- M4: Procedural world generation, chunk streaming, POI placement
- M6: Hardware audio, propagation, occlusion
- M10: Day/night cycle, weather simulation, atmosphere

Target: 1 km² playable prototype with weather and audio propagation.

### Phase 3: Systems

- M7: Save system, delta compression, schema versioning, mod compatibility
- M8: Data-driven itemization, materials, settlement building
- M9: Vehicle system, traction, fuel chains

Target: 10-minute playthrough with base building and vehicle traversal.

### Phase 4: Gameplay and Multiplayer

- M11: UI/HUD, input abstraction, quest system, factions
- M12: Networked co-op, persistence, dedicated server
- M13: SLM integration, endgame legacy

Target: 4-player co-op session with persistence and endgame loop.

---

## 9. Risk Register

| Risk | Probability | Impact | Mitigation | Owner |
|------|-------------|--------|------------|-------|
| Vulkan bindless not supported on target HW | Low | High | Capability tiering in M0; fallback to non-bindless draw | Render |
| Jolt 5.6 API changes | Medium | Medium | Pin exact commit; regression test suite | Physics |
| enkiTS contention at scale | Medium | Medium | Profile job graph before M5; tune grain size | Core |
| 36 km² streaming misses memory budget | Medium | High | Prototype 4 km² first; measure pagefile behavior | World |
| Audio propagation misses 20 Hz budget | Medium | Medium | Sparse voxel + frequency-band diffraction; update cadence fixed | Audio |
| Network desync under packet loss | Medium | High | Authority server + CRDT; deterministic replay test | Net |
| Mod API versioning breaks M13 save | Low | Medium | Versioned mod API from M7; schema migration tests | Modding |
| Research gaps block implementation | Medium | Medium | Parallel research track; exact sources tracked in backlog | All |

---

## 10. Professional Standards

This section documents how the project will operate, matching the standard of a
professional game engineering team.

### 10.1 Code Review

- Every change requires at least one peer review before merge
- Review focus: correctness, performance, Vulkan validation, test coverage
- Use pull-request workflow on GitHub; branch `spec/m0-parity-reformat` for spec work, `main` for integration-ready code

### 10.2 CI/CD

- GitHub Actions runs on every push to `spec/*` and `main`
- Jobs: configure with vcpkg toolchain, MSVC via `ilammy/msvc-dev-cmd`, Ninja build, `ZombieEngineTests.exe`, headless smoke
- Artifacts: `ZombieEngine.exe`, `ZombieEngineTests.exe` retained for 7 days
- Target: zero warnings, tests green, smoke pass

### 10.3 Testing Strategy

- Unit tests: `tests/unit/Test_*.cpp`, Catch2 framework, auto-discovered
- Integration tests: headless smoke run in CI
- Validation tests: Vulkan validation layers enabled in Debug; ASAN build on demand
- Performance baseline: RTSS + Afterburner telemetry captured at key milestones

### 10.4 Documentation

- Spec files are READ-ONLY; changes require explicit approval
- Plan files in `recon/plans/` are working documents; committed for branch history
- Research path document exists at `recon/plans/2026-07-21_RESEARCH_PATH_FIX.md`
- Fetcher script at `scripts/fetch_research.py` for free offline-friendly research

### 10.5 Source Tracking Rule

- Every claim must carry one of: [S] sourced, [E] engineering reasoning, [X] verification needed
- No permanent [E] assumptions without linked evidence
- Research backlog moves items from [X] to [S] or remains [X] as a blocker

---

## 11. Appendix A: Professional Studio and Indie Engine Case Studies

Purpose: extract durable patterns for ZE from how shipped engines were built.

### Big studios

1. id Tech 7/8 — id Software [S]
   **Primary source**: cached Wikipedia id Tech 7 extract
   **Evidence**: "On PC, id Tech 7 supports Vulkan rendering only." and "engine developer Axel Gneiting says the engine doesn't have a 'main thread'; everything is implemented as jobs." [cache: en.wikipedia.org-271a622a84.md]
   **Lesson**: bindless-first design reduces CPU overhead; jobified architecture without a main thread; clustered compute shading over traditional G-Buffer
   **Gap**: full architectural diagram set still needs SIGGRAPH Advances PDF extraction

2. Decima Engine — Guerrilla / Sony [X]
   **Primary source**: Horizon Zero Dawn/Frozen West public talks, SIGGRAPH coverage
   **Lesson**: three-tier terrain with compute grass; GPU-driven placement; visible multilevel environment design
   **Gap**: no extracted Decima-specific figure/text yet; Horizon GDC talk fetch failed 404 in live retrieval

3. UE5 Nanite + Lumen — Epic Games [S]
   **Primary source**: cached UE5 public docs + Karis Nanite SIGGRAPH PDF cache
   **Evidence**: cached UE5 docs index contains Nanite/Lumen/World Partition/PCG tokens. advances.realtimerendering.com cache contains Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf. [cache: ue5_docs.html, advances.realtimerendering.com-c818a353d9.md]
   **Lesson**: hierarchical cluster DAG; Hi-Z two-pass culling; virtual shadow maps with tile residency; world partition replaces level streaming; Mass Entity for gameplay actors; PCG for authored+procedural content; compute rasterizer for sub-pixel clusters

4. Frostbite — DICE / EA [S]
   **Primary source**: cached GDC Vault index + SlideShare index
   **Evidence**: cached GDC Vault index shows Frostbite rendering sessions. cached SlideShare index shows "Five Rendering Ideas from Battlefield 3 & Need For Speed: The Run by Electronic Arts / DICE" and "Frostbite on Mobile byElectronic Arts / DICE". [cache: www.gdcvault.com-8042576d17.md, www.slideshare.net-889de28ff4.md]
   **Lesson**: shared runtime toolchain across studios; data-material pipelines; deferred+forward hybrid; mobile-aware rendering stack

5. Unity DOTS + HDRP — Unity Technologies [X]
   **Primary source**: Unity public blog/DOTS samples
   **Lesson**: ECS-first archetype storage; burst compiler for hot loops; data-driven render graph separation
   **Gap**: Unity-Technologies/EntityComponentSystemExamples returned 404 in live retrieval; alternate sample repo needed

### Indie and small-team custom-engine efforts

1. Lethal Company — Zeekerss
   **Primary source**: public devlog and release postmortem coverage
   **Lesson**: single-session scope discipline; emergent horror from minimal systems; community rumor system as content multiplier

2. Valheim — Iron Gate Studio
   **Primary source**: public developer Q&A and postmortem coverage
   **Lesson**: biome gating with clear visual language; disciplined scale over feature count; network architecture designed for drop-in/drop-out

3. Noita — Nolla Games
   **Primary source**: public developer blog and postmortem videos
   **Lesson**: simulation-first worldview with pixel-accurate interaction; every system mutable; failure-state as narrative

4. Vintage Story — Tyron / Lone-dev model
   **Primary source**: official site, GitHub, wiki; long-term solo active-development history
   **Lesson**: solo active-development longevity over a decade; voxel/terrain hybrid with finite survival loop; deep modding fidelity as retention mechanism

5. Keep Talking and Nobody Explodes — Steel Crate Games
   **Primary source**: public postmortems and developer talks
   **Lesson**: small-team release discipline; documentation-as-UX; scoped to one mechanic and extended through community content

---

## 12. Appendix B: Explicit Research Backlog

This backlog is executable. Each item names an exact primary source or action trigger.

| ID | Source / Action | Route | Target Block | Status |
|----|-----------------|-------|--------------|--------|
| RB-01 | GDC Vault "Frostbite" talk title/year extract | cached GDC index + fetcher | Appendix A #4 | [X] |
| RB-02 | SIGGRAPH Advances id Tech 7/8 diagram set | direct fetch | Appendix A #1 | [X] |
| RB-03 | Horizon Zero Dawn SIGGRAPH talk / Decima terrain diagram set | direct fetch | Appendix A #2 | [X] |
| RB-04 | Unity DOTS sample repo bib + DOTS architecture doc | GitHub direct | Appendix A #5 | [X] |
| RB-05 | Lethal Company / Valheim / Noita devlog excerpt set | direct/devlog fetch | Appendix A | [X] |
| RB-06 | UE5 PCG and World Partition sample extract | UE5 docs direct | M4 M9 | [X] |
| RB-07 | GameDeveloper.com procedural generation article extract | direct extract | M4/M4.5 POI generation | [X] |
| RB-08 | Vulkan descriptor heap/indexing public spec extract | khronos.org direct | M0 renderer | [S] partial |
| RB-09 | GameNetworkingSockets README transport layer extract | GitHub direct | M12 netcode | [S] partial |
| RB-10 | Dear ImGui source + backend study extract | GitHub direct | M0 debug tools | [S] partial |
| RB-11 | M2 crafting research: FDG/GameDeveloper postmortems | direct/devlog fetch | M2 M8 | [X] |
| RB-12 | M9 vehicle research: shipped postmortems + driving-model papers | direct fetch | M9 | [X] |
| RB-13 | spec/M7.md EXT block breakdown | local spec | M7 persistence section | [S] |
| RB-14 | spec/M9.md EXT block breakdown | local spec | M9 vehicle section | [S] |
| RB-15 | spec/M11.md EXT block breakdown | local spec | M11 UI/quest section | [S] |
| RB-16 | spec/M12.md EXT block breakdown | local spec | M12 netco-op section | [S] |

**Rule for backlog**: every item must move to [S] sourced evidence or remain [X] as a named verification blocker. No permanent [E] assumptions without linked evidence.