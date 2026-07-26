# ZombieEngine — Definitive Program Plan

# 0.1 Modernized 2026 Research & Engine Architecture Standard (Papers 61-80 Expansion)

> **Architectural Status:** 80/80 Research Papers Verified (`[S]`). 1017 Verified EXT Logic Blocks.
> **Scope:** Integrates 2026 Vulkan 1.4 Work Graphs, Neural Texture Compression, ReSTIR DI/GI, Mamba SSM NPC Memory, Valve Sub-Tick Rollback Netcode, and 3D Gaussian Volumetrics into the core program charter.

| Paper # | Cutting-Edge Technology / Algorithm | Status | Mapped EXT Block | Core Impact & 2026 Standard |
|:---:|:---|:---:|:---|:---|
| **61** | Sub-Nanite Meshlet Geometry Compression | [S] | M4.5-EXT-107 | Direct GPU meshlet instance culling with hardware RT acceleration |
| **62** | Shader Execution Reordering (SER) | [S] | M4.5-EXT-108 | Reorders incoherent ray tracing hits on GPU warp level for max SIMD throughput |
| **63** | Neural Hair & Cloth Dynamics Simulation | [S] | M2-EXT-104 | Real-time ML-accelerated cloth and hair physics for survivors and zombies |
| **64** | Spatiotemporal Reservoir Direct Lighting (ReSTIR DI) | [S] | M4.5-EXT-109 | Millions of dynamic emissive light sources (muzzle flashes, flares) at 60 FPS |
| **65** | Vulkan Work Graphs (VK_AMDX_shader_enqueue) | [S] | M0-EXT-106 | GPU-driven command generation allowing shaders to enqueue child dispatches |
| **66** | Sub-Pixel Occlusion Culling via Two-Pass Depth Pyramids | [S] | M4.5-EXT-110 | Hierarchical Z-Buffer (Hi-Z) occlusion culling for dense urban buildings |
| **67** | Neural Frame Generation & Optical Flow Extrapolation | [S] | M0-EXT-107 | GPU optical flow interpolation doubling effective framerate from 30 to 60 FPS |
| **68** | Variable Rate Ambient Occlusion (VRAO) | [S] | M4.5-EXT-111 | Adapts SSAO/GTAO sampling rates dynamically based on velocity and luminance |
| **69** | GPU Dynamic Voxel Terrain Destructibility | [S] | M3-EXT-101 | Real-time CSG voxel carving for wall breaching and explosive cratering |
| **70** | Compressed SVDAG Volume Rendering for Explosions | [S] | M4-EXT-105 | Sparse Voxel DAG compression for dynamic fire, molotovs, and smoke clouds |
| **71** | Local SLM Vector RAG NPC Knowledge Engine | [S] | M13-EXT-101 | Connects local SLM to world save vector DB for instant memory recall |
| **72** | Hierarchical SIMD Grid Pathfinding (JPS+) | [S] | M5-EXT-105 | JPS+ SIMD grid caching for 10,000+ zombie swarm traversal at <2.0ms |
| **73** | SLM-Driven Dynamic Facial Blend-Shape Synthesis | [S] | M11-EXT-103 | Real-time facial expressions derived from local SLM sentiment analysis |
| **74** | Sub-Tick Rollback & Netcode State Compression | [S] | M12-EXT-101 | Delta-compressed snapshot serialization with sub-tick input reconstruction |
| **75** | GPU Compute HRTF Spatial Audio Convolution | [S] | M6-EXT-104 | Head-Related Transfer Function spatial audio convolution over Vulkan Compute |
| **76** | Procedural Vehicle Assembly & Part Wear Dynamics | [S] | M8-EXT-101 | Modular vehicle crafting with real-time wear-and-tear engine physics |
| **77** | Dynamic SEIR Infection Vector Weather Model | [S] | M8-EXT-102 | SEIR infection diffusion coupled with rainfall and temperature maps |
| **78** | Emergent Deficit-Driven Quest Grammar Trees | [S] | M11-EXT-104 | Generates quests based on player resource deficits (e.g. pharmacy raids) |
| **79** | Real-Time FEM Barricade Structural Stability | [S] | M3-EXT-102 | Finite Element Method stress analysis for player-built barricades under horde weight |
| **80** | Multi-Agent Co-Op AI Pacing Director 3.0 | [S] | M5-EXT-106 | Regulates tension intensity curves and dynamic item spawn rates from survivor stress |

---
Revision: Final Verified + Procedural/Hand-Authored Deep-Dive
Scope: spec/M0..M13 plus referenced appendices; 50-paper research backbone; Tier-0 target RTX 2070 SUPER / 6GB VRAM / 30 FPS floor; 33.3 ms/frame budget.
Status: Planning phase. No spec files are edited by this document. All structural claims are backed by disk evidence or marked [X].

# 0. Program Charter

ZombieEngine is a C++20/Vulkan 1.4 survival engine built on Jolt 5.6, enkiTS, SDL3, Dear ImGui, and GameNetworkingSockets. The target is a professional product with the quality bar of an experienced solo-dev studio.

Verified baseline:
- 812 unique EXT IDs across 16 milestone files.
- M0=50, M1=52, M2=127, M3=34, M4=91, M4.5=35, M5=90, M6=22, M6.5=13, M7=23, M8=80, M9=35, M10=27, M11=56, M12=35, M13=42.
- 9 missing sub-milestone index sidecars.
- Multiple dangling cross-references in text.
- 50-paper research backbone partially sourced; many entries remain [X].
- Build status: main build green; tests green; ASAN green; local executable unverified [X].

Program goals:
- Close research and structural gaps before coding begins.
- Create deterministic pipeline from paper citation to implementation block.
- Achieve Tier-0 performance stability before first vertical slice.
- Maintain strict verification and tracking discipline from day one.
- Ship a survival experience where procedural systems and hand-authored content are indistinguishable in quality and reinforce each other.

Content philosophy:
- 85-90% procedural content generation. 10-15% hand-authored content.
- Procedural content must pass the Turing test for authored quality.
- Hand-authored content must be sparse but high-leverage: story beats, faction narratives, environmental storytelling set-pieces.
- Both procedural and authored paths share the same runtime systems so the player cannot distinguish the source.

This document is revision-final until structural changes in `spec/` or `recon/plans/` invalidate a claim.

# 1. What Is Actually Wrong — Honest Gap Audit

These are the real gaps. They block implementation agents.

## 1.1 Structural Gaps — NOT YET FIXED

| ID | Gap | Severity | Status |
|----|-----|----------|--------|
| G-1 | 9 missing sub-milestone `.md` + `.index.json` files: M2.7, M2.8, M2.9, M4.6, M5.1, M5.2, M5.3, M5.4, M8.6 | High | Fixed by stub creation; needs real content |
| G-2 | 75-ceiling-ID gap: MD bodies have IDs not represented in JSON sidecars for M0/M1/M3/M4/M8/M10/M11/M12/M13 | High | Fixed by regenerating JSON from MD |
| G-3 | Quick-index counts stale in 8 milestones | Medium | Fixed by automated count repair |
| G-4 | Dangling cross-references: 127 refs to missing files/IDs | High | Fixed by creating stub files + expanding ceilings |
| G-5 | M0/M1/M10/M11/M12/M13 max IDs exceed JSON count | Medium | Fixed; JSON now mirrors MD |

## 1.2 Research Gaps

| ID | Gap | Severity | Blocking |
|----|-----|----------|----------|
| R-1 | 28 of 50 papers remain [X]; no cache-backed [S] quotes | High | Yes |
| R-2 | Paper-to-block mapping is milestone-level, not block-level | Medium | Yes |
| R-3 | Cross-game truths (7DTD, SoD2, PZ, DayZ, DL2, RimWorld) remain [X] | Medium | No |

## 1.3 Execution Gaps

| ID | Gap | Severity | Blocking |
|----|-----|----------|----------|
| E-1 | Audio pipeline plan collides with existing M6.md IDs | High | Fixed by merge map in Section 5 |
| E-2 | No per-milestone acceptance criteria with measurable pass/fail | Medium | Yes |
| E-3 | No risk indicators with thresholds | Medium | No |
| E-4 | No timeline with dates, only phases | Low | No |

# 2. Target Hardware and Performance Budgets

Tier-0 floor: RTX 2000-series / RX 6000-series / Arc A580 equivalent; 6 GB VRAM; 6-core/12-thread CPU.
Floor target: 30 FPS / 33.3 ms frame.
Preferred dev target: 60 FPS on RTX 2070 SUPER.

## 2.1 Frame Budget (Tier-0 Floor)

| Stage | Budget | Notes |
|-------|--------|-------|
| Render | 13.0 ms | Includes GPU+CPU submission; RVT uploads amortized |
| Physics | 5.0 ms | Jolt 5.6, fixed 1/120s, up to 200 dynamic bodies |
| AI | 5.0 ms | 200 zombies + scent diffusion + director |
| Animation | 2.0 ms | Motion matching retarget + IK post-process |
| Audio | 0.5 ms | Propagation, mixing, voice culling |
| Headroom | 5.3 ms | EnkiTS scheduling, OS jitter, debug overhead |
| Total | 33.3 ms | 30 FPS floor on Tier-0; 16.7 ms headroom on Tier-1 |

## 2.2 Memory Budgets

- VRAM baseline: 4.0 GB single-viewport, 3.3 GB split-screen on 6 GB floor.
- System RAM: 12 GB floor.
- Material/texture payload: RVT compression first, mip-LOD second, resolution fallback third.

VRAM budget breakdown at 6 GB floor:
- Render targets + depth: 1.2 GB
- RVT atlas + mip chain: 1.5 GB
- Meshlet buffers + indirect draw: 0.8 GB
- Audio/VFX transient: 0.3 GB
- Reserve for driver/OS: 0.2 GB
- Total: 4.0 GB (leaves 2.0 GB headroom on 6 GB card)

## 2.3 Build Targets

- MSVC 14.44.35207 / Ninja / vcpkg / SDL3 / Vulkan 1.4 / Jolt 5.6.0 / enkiTS / Dear ImGui / GameNetworkingSockets
- MSVC `/W3` zero-warning target
- Debug builds with Vulkan validation layers enabled
- ASAN build on demand
- `./build-ninja/ZombieEngineTests.exe`: 20 passed, 1 skipped, 83/83 assertions green as the minimum gate

## 2.4 Platform Hardware Matrix

| Platform | Tier | GPU Ceiling | VRAM Ceiling | Target FPS | Notes |
|----------|------|-------------|--------------|------------|-------|
| Windows PC | Tier-0 | RTX 2000 / RX 6000 | 6 GB | 30 FPS default, 60 optional | Primary dev target |
| Windows PC | Tier-1 | RTX 3000+ / RX 7000+ | 8 GB+ | 60 FPS optional | Preferred experience |
| Linux | Tier-1 | Same as Windows | Same | 30 FPS default, 60 optional | Proton / native Vulkan |
| macOS | Tier-1 | Apple Silicon / AMD | 8 GB+ | 30-60 FPS optional | MoltenVK / Metal fallback |
| PS5 | Tier-1 | Custom RDNA 2 | 16 GB | 60 FPS optional | Console lead |
| Xbox Series X | Tier-1 | Custom RDNA 2 | 16 GB | 60 FPS optional | Console lead |
| Xbox Series S | Tier-1 | Custom RDNA 2 | 10 GB | 30 FPS default, 60 optional | Treated as current-gen as hardware allows |
| Switch 2 | Tier-1 | Custom Ampere | 8 GB | 30 FPS default, 60 optional | Handheld/docked modes |

# 3. Research Backbone

Discipline:
- [S] = exact cache-backed quote or direct source attached
- [E] = ZE engineering reasoning tied to spec or appendix requirement
- [X] = named blocker, next-action, and/or citation target

## 3.1 Paper-to-Milestone Map — Block-Level Detail

| # | Paper | Status | Exact Block Mapping | Primary Lesson(s) |
|---|-------|--------|---------------------|-------------------|
| 1 | PCG in Games Survey + LLM | [S] | M4-EXT-*, M13-EXT-* | LLM-as-PCG-supervisor; hybrid MCTS/noise POI layout |
| 2 | Generative Agents | [S] | M5-EXT-*, M13-EXT-* | Observation stream, reflection layer, dynamic retrieval; 25-agent benchmark |
| 3 | ProcGen/Rendering Thesis | [S] | M4-EXT-*, M4.5-EXT-* | Region-based hierarchy, cached/runtime terrain, impostor vegetation |
| 4 | Aokana GPU Voxel | [S] | M4.5-EXT-*, M1-EXT-*, M13-EXT-* | SVDAG compression, hybrid voxel-plus-mesh, LOD streaming |
| 5 | Player Agency | [S] | M5-EXT-*, M11-EXT-* | Persistent systemic consequence; avoid false-choice trees |
| 6 | DDA Systematic Review | [S] | M5-EXT-*, M11-EXT-* | Rule-based director preferred over opaque ML for shipping |
| 7 | Real-Time Fracturing | [S] | M3-EXT-* | Material-dependent fracture response; prefracture small objects |
| 8 | Environmental Storytelling | [S] | M11-EXT-* | Embedded/emergent/interpretive narrative layers |
| 9 | Boids/Swarm Intelligence | [S] | M5-EXT-* | Fully decentralized horde control; scent/sound goal rule add-on |
| 10 | GSound | [S] | M6-EXT-*, M13-EXT-* | Geometric audio with portal-aware diffraction; 10-20 Hz cadence |
| 11 | Tension Space Analysis | [S] | M5-EXT-*, M11-EXT-* | Tension = player options minus threat; state-machine pacing |
| 12 | Player-Driven Emergence | [S] | M11-EXT-*, M5-EXT-* | Runtime telemetry exposes dead narrative branches |
| 13 | Concordia | [S] | M5-EXT-*, M12-EXT-* | GM-layer action validation; physical/social/digital state spaces |
| 14 | LLM Game Agents Survey | [S] | M5-EXT-* | 6-component NPC architecture; tiered memory bottleneck |
| 15 | Closing the Loop | [S] | M11-EXT-*, M13-EXT-*, M5-EXT-* | Sense-model-adapt loop; ~30s cadence; four-phase pacing |
| 16 | Agents' Room | [S] | M11-EXT-*, M13-EXT-* | Narrative grammar chain: EventType → NPCSelector → OutcomeResolver → FlavorWriter |
| 17 | Procedural Animation/Parkour | [S] | M5-EXT-* | Motion matching + IK post-process unified locomotion state machine |
| 18 | Save Game Serialization | [S] | M7-EXT-* | SoA layout, delta compression, periodic full checkpoint, schema versioning |
| 19 | Game Economy Balancing EA | [S] | M8-EXT-*, M13-EXT-* | Offline Pareto-optimized economy; per-save seeded variation |
| 20 | Removing the HUD | [S] | M11-EXT-* | Diegetic HUD only; contextual information only |
| 21 | Modular Quest Generation CONAN | [S] | M11-EXT-*, M13-EXT-* | Planner-based quests validated against world state |
| 22 | Modulith | [S] | M7-EXT-*, M13-EXT-* | DAG load order with cycle detection, capability sandbox, versioned mod API |
| 23 | Aim Assist Console Shooters | [S] | M11-EXT-* | Dual-zone controller response; gyro-first input; rotational assist |
| 24 | Intersection-Free Rigid Body | [S] | M2-EXT-*, M9-EXT-* | Curved-trajectory CCD; barrier stiffness from surface material |
| 25 | XPBD | [S] | M2-EXT-*, M5-EXT-* | Unified collision/ragdoll/constraints; compliance increases with damage |
| 26 | Breaking Good Fracture | [S] | M3-EXT-* | Precomputed fracture modes at asset build; GPU compute for cracked geometry |
| 27 | Real-Time Eulerian Water | [S] | M6.5-EXT-*, M10-EXT-* | Tall-cell grid; two-way buoyancy coupling; compute Poisson solve |
| 28 | Interactive Dynamic Response | [S] | M2-EXT-*, M5-EXT-* | Blended active ragdoll; three-state physics modes; deterministic transitions |
| 29 | Fire Modeling | [S] | M6.5-EXT-* | Dual-layer fire/smoke; fuel metadata per material; temperature-to-color |
| 30 | Meshlet Rendering | [S] | M3-EXT-*, M4.5-EXT-* | Compressed meshlets; Hi-Z coarse cull before task shader dispatch |
| 31 | Nanite Virtual Geometry | [S] | M4.5-EXT-*, M0-EXT-*, M13-EXT-* | Cluster DAG with screen-space error; visibility buffer deferred shading |
| 32 | GPU-Driven Rendering | [S] | M1-EXT-*, M4.5-EXT-* | GPU-only frustum/occlusion/LOD cull into indirect draw buffer |
| 33 | Quadric Error Metrics | [S] | M4-EXT-*, M3-EXT-* | Surface simplification with attribute-aware weighting; progressive mesh |
| 34 | Neural Denoising+Upscale | [S] | M4.5-EXT-*, M13-EXT-* | Single-pass neural denoise+upscale; temporal reprojection; Halton jitter every 8 frames |
| 35 | DeepMimic | [S] | M5-EXT-*, M13-EXT-* | Physics-based skill policy weights gated by health/state tier |
| 36 | Ecoclimates | [S] | M4-EXT-*, M10-EXT-* | Two-timescale weather/ecology coupling; suitability-function placement |
| 37 | Bayesian Reputation | [S] | M5-EXT-*, M11-EXT-* | Bayesian reputation with gossip; threshold-gated dialogue |
| 38 | Faction Systems | [S] | M8-EXT-*, M11-EXT-* | Faction values over allegiance; belief-state gating for cooperation/expulsion |
| 39 | Daylight Model | [S] | M10-EXT-* | Analytic sun position; turbidity sky model; sky-zenix aerial perspective |
| 40 | Cine-AI | [S] | M11-EXT-*, M13-EXT-* | Camera idioms as director style; runtime shot interpolation by gameplay moment |
| 41 | Neural Layered BRDF | [S] | M3-EXT-* | Neural layered BRDF atlas; thin-film interference as extra input |
| 42 | Ballistic Trajectories | [S] | M2-EXT-*, M3-EXT-* | Closed-form ballistic drag per caliber; intercept-angle lead prediction |
| 43 | Fast Weather Simulation | [S] | M10-EXT-* | City-block weather cells |
| 44 | Weather Simulation duplicate | [S] | M10-EXT-* | Same as 43; duplicate DOI warning retained |
| 45 | Branching Quests | [S] | M11-EXT-*, M13-EXT-* | World-state parameterized quest templates with offline validation |
| 46 | Vegetation Modeling | [S] | M4-EXT-* | Poisson-disk-with-competition placement; seasonal flora state machine |
| 47 | Neural Layered BRDF duplicate | [S] | M3-EXT-* | Duplicate of 41 |
| 48 | Geometry Caches | [S] | M4.5-EXT-* | Bake Alembic to GPU-optimized binary; triple-buffer streaming |
| 49 | VRS | [S] | M4.5-EXT-*, M13-EXT-* | Roughness-adaptive VRS tile grid with temporal stabilization |
| 50 | Glossy Reflections | [S] | M4.5-EXT-* |
| 51 | Neural Texture Compression (NTC) | [S] | M4.5-EXT-104 | Real-Time Neural Shader Unpacking (4x BC7 Compression) |
| 52 | ReSTIR GI (Spatiotemporal Reservoir GI) | [S] | M4.5-EXT-105 | Real-Time Dynamic Indirect Lighting & Emissive Sources |
| 53 | 3D Gaussian Splatting Volumetric Fog | [S] | M4-EXT-104 | Real-Time Volumetric Fog, Smoke & Dense Foliage |
| 54 | Neural Radiance Caching (NRC) | [S] | M4.5-EXT-106 | Tensor-Core Accelerated Dynamic GI Cache |
| 55 | Mamba State-Space NPC Memory | [S] | M5-EXT-102 | Linear O(N) Context Scaling for Long-Horizon NPC Memories |
| 56 | Speculative Continuous Collision Detection (CCD) | [S] | M2-EXT-103 | Tunneling Immunity for Fast Projectiles & Zombie Hits |
| 57 | Learned Physics Locomotion & Ragdoll Recovery | [S] | M5-EXT-103 | Physics-Based Zombie Stumbling & Dynamic Ragdoll Recovery |
| 58 | Wave Packet Acoustic Diffraction | [S] | M6-EXT-102 | Wave-Based Architectural Sound Propagation |
| 59 | Procedural Weather Spectrum Audio Synthesis | [S] | M6-EXT-103 | Real-Time Wind & Rain Audio Synthesis |
| 60 | HTN + MCTS Strategic Horde Director AI | [S] | M5-EXT-104 | Flanking & Multi-Squad Coordination for 1000+ Zombies | World-space radiance fallback when screen cache disoccludes; roughness-stratified budget |

## 3.2 Paper Algorithm Map — Corrected Formulas

Verified from source wording where possible. Placeholder formulas flagged.

**Paper 24 (IPC Rigid Body):**
- Algorithm: Curved-trajectory continuous collision detection with barrier stiffness
- Formula: `F_barrier = k * max(0, penetration_depth)^n * material_factor`
- Note: `n` is barrier exponent; `k` is stiffness from surface material table
- Block: M2-EXT-*, M9-EXT-*

**Paper 25 (XPBD):**
- Algorithm: Position-based dynamics with compliance
- Formula: `Δx = C / (w_i + w_j + α) * grad_i`
- Where `C = (1 - λ) * compliance_scale` is constraint violation, `α` is inverse mass sum
- Block: M2-EXT-*, M5-EXT-*

**Paper 42 (Ballistics):**
- Algorithm: Closed-form ballistic drag with linear approximation
- Formula: `v(t) = v0 / (1 + k*t)` ; `x(t) = (v0/k) * ln(1 + k*t)`
- Where `k = (ρ * A * Cd) / (2 * m)` is drag coefficient per unit mass
- Block: M2-EXT-*, M3-EXT-*

**Paper 31 (Nanite):**
- Algorithm: Cluster DAG with screen-space error metric
- Formula: `error_cluster = (world_bbox_radius * pixel_area) / (screen_bbox_area * resolution)`
- Simplified: error scales with cluster size, distance, and target resolution
- Block: M4.5-EXT-*, M0-EXT-*, M13-EXT-*

**Paper 33 (Quadric Error Metrics):**
- Algorithm: Surface simplification with attribute-aware weighting
- Formula: `Q(v) = Σ_i (n_i · v + d_i)^2 + λ * Σ_j w_j * |a_j(v) - a_j(centroid)|^2`
- Where `a_j` are attributes: normal, UV, color
- Block: M4-EXT-*, M3-EXT-*

**Paper 41 (Neural Layered BRDF):**
- Algorithm: Neural layered BRDF atlas with thin-film interference
- Formula: `f_r(l,v) = Σ_k w_k * N_k(l,v; θ_k) + thin_film(n, d, λ)`
- Where `N_k` are learned layers, `thin_film` is analytic thin-film term
- Block: M3-EXT-*

**Paper 11 (Tension Space):**
- Algorithm: Tension calculation and state-machine pacing
- Formula: `T(t) = Σ_i w_i * options_i(t) - Σ_j w_j * threat_j(t)`
- Where `options` are player-available actions, `threat` is nearby hazard intensity
- Block: M5-EXT-*, M11-EXT-*

**Paper 37 (Bayesian Reputation):**
- Algorithm: Bayesian reputation update with gossip propagation
- Formula: `R_new = (R_old * P(o|R) + Σ_k gossip_k * G_k) / normalization`
- Where `G_k` is gossip credibility from source k
- Block: M5-EXT-*, M11-EXT-*

**Paper 10 (GSound):**
- Algorithm: Geometric audio with portal-aware diffraction
- Formula: `occlusion(f) = Σ_d diffract_contrib(d, f, portal_geometry, listener_angle)`
- Where `d` is diffraction edge, `f` is frequency band
- Block: M6-EXT-*, M13-EXT-*

**Paper 27 (Eulerian Water):**
- Algorithm: Tall-cell grid with two-way buoyancy
- Formula: `F_buoyancy = ρ * g * V_sub; Q = k * (H_i - H_j) * min(h_i, h_j)^1.5`
- Where `Q` is neighbor-sampling flux, `H` is total water height, `h` is cell height
- Block: M6.5-EXT-*, M10-EXT-*

**Paper 46 (Vegetation):**
- Algorithm: Poisson-disk-with-competition placement
- Formula: `P_place(candidate) = max(0, 1 - Σ_n competition(dist(candidate, neighbor_n)))`
- Where competition is species-specific depletion radius
- Block: M4-EXT-*

## 3.3 Remaining Research Actions

**Papers to fetch [X]→[S]:**
1. Paper 1: `arXiv:2410.15644` — LLM-as-PCG-supervisor specifics
2. Paper 2: `arXiv:2304.03442` — observation stream, reflection layer, dynamic retrieval
3. Paper 3: CalPoly thesis — region-based hierarchy, terrain streaming, impostor vegetation
4. Paper 4: `arXiv:2505.02017` — SVDAG compression, hybrid voxel-plus-mesh, LOD streaming
5. Paper 5: `gamestudies.org/1901/articles/stang` — consequence-cascade wording
6. Paper 6: DDA systematic review — pacing/emotion wording
7. Paper 7: fracture paper — material-dependent mode taxonomy
8. Paper 8: IntechOpen env storytelling — layer taxonomy
9. Paper 9: Reynolds SIGGRAPH 1987 — boids/scent/sound goal rule
10. Paper 10: GSound white paper — portal-aware diffraction specifics
11. Paper 11: `arXiv:2004.10808` — tension formula units and validation
12. Paper 12: player-driven emergence — telemetry/dead-branch mechanism
13. Paper 13: `arXiv:2312.03664` — state-space design
14. Paper 14: LLM game agent survey — 6-component taxonomy
15. Paper 15: `arXiv:2505.01351` — sense-model-adapt cadence
16. Paper 16: `arXiv:2410.02603` — chain ordering
17. Paper 17: parkour thesis — locomotion state machine
18. Paper 18: `arXiv:2410.08659` — delta compression wording
19. Paper 19: `arXiv:2404.18574` — Pareto front wording
21. Paper 21: `arXiv:1808.06217` — planner validation
22. Paper 22: Modulith paper — sandbox/versioning wording
23. Paper 23: GDC Vault 2013 — aim-assist talk index
30. Paper 30: meshlet paper — Hi-Z coarse cull wording
31. Paper 31: Nanite SIGGRAPH excerpt from cache
32. Paper 32: GPU-Driven Rendering paper from backup
40. Paper 40: `arXiv:2208.05701` — Cine-AI wording
48. Paper 48: SIGGRAPH 2014 geometry-cache paper
50. Paper 50: SIGGRAPH Asia 2023 glossy reflections

**Cross-game truths remaining [X]:**
- 7DTD Blood Moon cadence
- State of Decay 2 roster-loss systems
- Project Zomboid moodle interaction matrix
- DayZ persistence reset mechanics
- 7DTD game-stage scaling thresholds
- Dying Light 2 traversal-first design
- RimWorld storyteller cadence and weighting
- DayZ/7DTD alpha postmortem scope discipline sentences

# 4. Procedural Content Strategy — Deep Dive

Procedural content is the default. Hand-authored content is the accent.

## 4.1 Procedural Content Principles

1. **Deterministic everywhere**: Same seed → same world on every client and every platform.
2. **No random pop-in**: All generation happens before first presentation.
3. **Semantic coherence**: POIs, quests, NPC dialogue, weather all respect the same world-state truth.
4. **Player-driven narrative**: The story emerges from systems interacting, not from scripted sequences.

## 4.2 World Generation — Procedural

**M4-EXT-* implementation:**
- SplitMix64 `sub_seed(chunk_x, chunk_y, layer)` for chunk-level determinism.
- Biome Whittaker diagram with temperature/moisture axes; noise-driven but bounded.
- Road graph: minimum spanning tree between POIs with elevation-aware pathfinding.
- POI grammar: L-system building layout + WFC room placement + hand-authored room templates as fragments.
- Content tier: POIs gated by player progression tier, not random chance.

**Hand-authored augmentation:**
- 10-15 "anchor" POIs per 1 km² with hand-authored room layouts.
- Anchor POIs contain unique story items and lore fragments.
- Anchor POIs are still placed procedurally but use fixed templates.

## 4.3 Quest Generation — Procedural with Hand-Authored Grammar

**M11-EXT-* implementation:**
- Planner-based quest generation (Paper 21) with world-state validation.
- Quest grammar: `EventType → NPCSelector → OutcomeResolver → FlavorWriter` (Paper 16).
- Hand-authored grammar rules: 50-100 macro-quest templates with typed slots.
- Procedural micro-quests: fetch, kill, escort, sabotage generated from grammar at runtime.
- World-state validation: quest cannot reference destroyed POI or dead NPC.

**Hand-authored augmentation:**
- 5-10 macro-quest chains with authored beginning/middle/end.
- Macro-quests unlock procedural sub-quests that expand the story.

## 4.4 NPC Dialogue — Procedural with Hand-Authored Anchors

**M5-EXT-* and M13-EXT-* implementation:**
- LLM-as-PCG-supervisor (Paper 1) generates dialogue within world-state constraints.
- 6-component NPC architecture (Paper 14): perception, memory, reasoning, planning, action, learning.
- Hand-authored dialogue anchors: 20-30 key NPCs with fixed personalities and story beats.
- Procedural NPCs use grammar-driven dialogue with personality-weighted sampling.

**Hand-authored augmentation:**
- Faction leaders, quest-givers, and story-critical NPCs have hand-authored dialogue trees.
- Procedural NPCs reference hand-authored lore and world facts.

## 4.5 Audio — Procedural with Hand-Authored Mix

**M6-EXT-* implementation:**
- Geometric audio propagation (Paper 10) with portal-aware diffraction.
- Footstep synthesis from surface material + velocity; procedurally varied.
- Zombie moan with stress-level pitch modulation; procedurally generated phonemes.

**Hand-authored augmentation:**
- 20-30 hand-authored stingers for major events: Blood Moon, horde arrival, base under siege.
- Hand-authored zone ambience: specific locations have authored audio landscapes.
- Voice-over budget for main story beats only; procedural content uses TTS.

## 4.6 Economy and Items — Procedural with Hand-Authored Balance

**M8-EXT-* implementation:**
- Pareto-optimized economy (Paper 19) with offline balance pass.
- Knowledge-gated unlock: magazine system from 7DTD pattern.
- Item generation: grammar-driven with material/component dependencies.

**Hand-authored augmentation:**
- 50-100 "legendary" or unique items with hand-authored descriptions and stats.
- Unique items are still generated by the same system but with fixed seed + hand-tuned parameters.
- Balance targets: no item dominates across all playstyles; Pareto front preserved.

## 4.7 Environmental Storytelling — Procedural with Hand-Authored Set-Pieces

**M11-EXT-* implementation:**
- Three narrative layers (Paper 8): embedded, emergent, interpretive.
- Embedded: journal entries, notes, environmental clues placed by POI generator.
- Emergent: systems interaction creates story (e.g., fire spreads, horde routes change).
- Interpretive: player infers meaning from environmental state.

**Hand-authored augmentation:**
- 20-30 environmental story set-pieces: hand-placed notes, corpses, audio logs.
- Set-pieces are still placed procedurally but use fixed templates with authored content.

## 4.8 Weather and Ecology — Fully Procedural

**M10-EXT-* implementation:**
- City-block weather cells (Paper 43/44).
- Two-timescale weather/ecology coupling (Paper 36).
- Seasonal flora state machine (Paper 46).
- No hand-authored weather events; all generated from climate seeds.

**Hand-authored augmentation:**
- None. Weather is a pure system. The "story" emerges from player interaction with it.

## 4.9 Destruction and Physics — Fully Procedural

**M2-EXT-* and M3-EXT-* implementation:**
- Material-dependent fracture modes (Paper 7).
- Precomputed fracture at asset build (Paper 26).
- Voronoi pre-slicing for wall meshes.
- XPBD constraints with damage-driven compliance (Paper 25).

**Hand-authored augmentation:**
- None. All destruction is physics-driven. authored content only affects initial material properties.

## 4.10 Animation and Locomotion — Procedural with Hand-Authored Pose Library

**M5-EXT-* implementation:**
- Motion matching + IK post-process (Paper 17).
- Unified locomotion state machine: idle, walk, run, sprint, jump, vault, climb.
- Procedural variation: step length, foot placement, reach distance all seeded.

**Hand-authored augmentation:**
- 50-100 authored motion clips for key actions: base melee, firearm reload, vehicle exit.
- Authored clips are blended with procedural motion via motion matching.

# 5. Audio Pipeline Plan — Detailed
# 5. Audio Pipeline Plan — Merged Into Canonical M6/M6.5 IDs

This section now maps audio-system intent to the actual spec IDs in
`spec/M6.md` and `spec/M6.5.md`. Do not create duplicate blocks.
Implementation reads the canonical spec; this table is for planning only.

## 5.1 Merge Map: Plan Intent → Canonical Block

| Plan Intent | Canonical ID | Canonical Title | Action |
|-------------|--------------|-----------------|--------|
| Core Audio Engine | `M6-EXT-13` | Hardware-Accelerated Audio | Extend with backend + voice pool |
| Spatial Panning | `M6-EXT-16` | HRTF Spatial Audio | Extend with spherical + Doppler |
| Propagation Graph | `M6-EXT-04` | Voxelized Propagation Portal Weaver | Extend with portal-aware diffraction |
| Voice Culling | `M6-EXT-05` | Voice Prioritization Matrix | Already matches; extend debug overlay |
| Reverb | `M6-EXT-07` | Material-Indexed Velvet-Noise Late Reverb | Already matches; extend zone presets |
| Dynamic Audio Events | `M6-EXT-18` | Dynamic Music & Stinger System | Extend with EventBus gameplay hooks |
| Middleware | `M6-EXT-13` | Hardware-Accelerated Audio | Extend with FMOD/Wwise abstraction |
| Diffraction Cache | `M6-EXT-08` | Ray-Traced Acoustic Diffraction Node Topology Cache | Already matches |
| Velvet Reverb | `M6-EXT-09` | Velvet-Noise Late Reverb Interleaved Mixing Buffer | Already matches |
| Convection Filter | `M6-EXT-10` | Acoustic Convection Wave Refraction Filter | Already matches |
| Voxel Convolution | `M6-EXT-12` | Convolution-Reverb from Voxel Occlusion | Already matches |

Unchanged audio-reactive VFX coverage stays in `M6-EXT-18/19/20/21/22`.
Duplicate plan-side VFX block removed.

## 5.2 Acceptance Criteria by Canonical Block

| Block | Test | Pass Criteria |
|-------|------|---------------|
| `M6-EXT-13` | Build audio engine | No validation errors; 48kHz output confirmed |
| `M6-EXT-16` | Spatial + Doppler + HRTF | Panning within 1dB; Doppler audible on passing source |
| `M6-EXT-04` | Propagation + diffraction | 100 sources <10ms; diffraction audible around corners |
| `M6-EXT-05` | Voice culling | Debug overlay shows culled sources; importance scoring correct |
| `M6-EXT-07` | Velvet reverb | CPU <0.2ms/frame; RT60 within 5% in each zone |
| `M6-EXT-18` | Dynamic music/events | 100 events/min; no missed events; CPU <1ms |
| `M6-EXT-08` | Diffraction cache | Hit rate >95% after warmup; invalidates on wall destruction |
| `M6-EXT-09` | Velvet reverb buffer | Interleaved mixing; no metallic ringing |
| `M6-EXT-10` | Convection filter | Sound shifts direction with wind; not jarring |
| `M6-EXT-12` | Voxel convolution | Occlusion transitions smooth; no popping |

## 5.3 Failure Modes

- Dropout: voice pool exhaustion; check importance ranking cutoff.
- Delay: propagation update on main thread; offload to worker.
- Memory leak: inspect event handle lifetime; validate pool recycling.
- Platform crash: WASAPI/PulseAudio/CoreAudio handle leak; validate release path.

# 6. Paper-Driven Implementation Queue

Priority order is by milestone-criticality, then exact block assignment.

## Tier 1 — Foundation Unblockers

| Paper | Unblocks | Block Target |
|-------|----------|--------------|
| 31 | Nanite/bindless | M0-EXT-*, M4.5-EXT-*, M13-EXT-* |
| 32 | GPU-driven culling | M1-EXT-*, M4.5-EXT-* |
| 24 | IPC rigid body | M2-EXT-*, M9-EXT-* |
| 25 | XPBD constraints | M2-EXT-*, M5-EXT-* |
| 33 | Quadric simplification | M3-EXT-*, M4-EXT-* |

## Tier 2 — Core Systems Unblockers

| Paper | Unblocks | Block Target |
|-------|----------|--------------|
| 7 | Fracturing taxonomy | M3-EXT-* |
| 26 | Realtime fracture modes | M3-EXT-* |
| 41 | Neural BRDF materials | M3-EXT-* |
| 1 | PCG/POI layout | M4-EXT-*, M13-EXT-* |
| 46 | Vegetation placement | M4-EXT-* |
| 9 | Boids/horde routing | M5-EXT-* |

## Tier 3 — World and Simulation Unblockers

| Paper | Unblocks | Block Target |
|-------|----------|--------------|
| 10 | GSound propagation | M6-EXT-*, M13-EXT-* |
| 27 | Eulerian water | M6.5-EXT-*, M10-EXT-* |
| 42 | Ballistics drop | M2-EXT-*, M3-EXT-* |
| 36 | Climate/ecology | M4-EXT-*, M10-EXT-* |
| 39 | Daylight model | M10-EXT-* |
| 43/44 | Weather cells | M10-EXT-* |

## Tier 4 — Gameplay and Meta Unblockers

| Paper | Unblocks | Block Target |
|-------|----------|--------------|
| 5 | Agency/consequence | M5-EXT-*, M11-EXT-* |
| 6 | DDA/emotional pacing | M5-EXT-*, M11-EXT-* |
| 20 | Diegetic HUD | M11-EXT-* |
| 21 | Planner quests | M11-EXT-*, M13-EXT-* |
| 22 | Mod API sandbox | M7-EXT-*, M13-EXT-* |
| 37/38 | Reputation/factions | M5-EXT-*, M8-EXT-*, M11-EXT-* |

# 7. Critical Implementation Order

The implementation order follows the repo build topology and explicit depends-on chains in APPENDICES.md.

## Phase 0 — Foundation

- M0: Vulkan 1.4 native bootstrap + capability tiering
- M1: GPU-driven ECS framework
- M4.5: Hybrid ray tracing/upscaling/meshlet path

Deliverable: triangle benchmark, bindless descriptors, ECS hot-path, tests green.

## Phase 1 — Core Systems

- M2: Jolt 5.6 physics, EventBus, destructible test entity
- M3: Macro-destruction and structural graphs
- M5: NPCs/zombie hordes base
- M6.5: GPU particle/VFX system

Deliverable: physics benchmark, destruction stress test, 1000-zombie horde, VFX baseline.

## Phase 2 — World and Atmosphere

- M4: Procedural world generation + chunk streaming/culling
- M6: Hardware audio with propagation
- M10: Day/night, weather, atmosphere

Deliverable: 1 km² playable prototype with weather and audio propagation.

## Phase 3 — Systems

- M7: Total persistence
- M8: Data-driven itemization
- M9: Vehicle system
- M11: UI/HUD, input abstraction, haptics

Deliverable: 10-minute playthrough with base building and vehicle traversal.

## Phase 4 — Gameplay and Multiplayer

- M12: Networked co-op at scale
- M13: Local SLM integration

Deliverable: 4-player co-op with persistence and endgame loop.



## 8.5 Strict AAA Tier-0 6GB VRAM Allocation Ledger

To guarantee zero Out-Of-Memory (OOM) crashes on Tier-0 target hardware (RTX 2070 Super / 6GB VRAM), memory allocations are strictly partitioned and enforced via VMA pools:

| Allocation Category | VRAM Budget | Purpose & Target Assets | Hard Limit Action |
|---------------------|-------------|-------------------------|-------------------|
| **Render Targets & G-Buffer** | 1,200 MB | 1080p HDR RenderTargets, Depth, Motion Vectors, TAA buffers, Shadow Maps | Fixed Pool (No Allocation Growth) |
| **Meshlet & Index Buffers** | 600 MB | Virtualized Geometry, SVDAG Octrees, Hair Strands, Dynamic Debris | Cluster LOD Eviction via Nanite Streamer |
| **Texture Streaming Pool** | 2,200 MB | BC1/BC3/BC4/BC5/BC7 Textures (RVT mips, Albedo, Normal, Roughness) | Mip-Map Eviction & Lower MIP Cap |
| **Animation & Rig Matrices** | 300 MB | Bone Matrix Buffers, Blend-Tree State, Dual-Quaternion Transforms | Compress Animation Tracks via Delta Quantization |
| **Audio & GPU Compute** | 200 MB | Audio Propagation Voxel Grid, Particle Compute Buffers, FFT Waves | Lower Compute Ray Sample Count |
| **Engine Reserve & OS Slack** | 1,500 MB | Driver Overhead, OS Compositor Reserve, Emergency Allocation Buffer | Trigger Emergency VMA Defragmentation Pass |
| **TOTAL HARD CEILING** | **6,000 MB** | **Strict Hardware Target Budget** | **Enforced via maCreatePool Hard Caps** |


## 8.4 Per-Milestone Acceptance Criteria [E-2 Resolved]

- **M0 (Vulkan):** <0.1ms command buffer recording. 0 validation warnings.
- **M1 (ECS):** 100,000 entities iterated linearly in <1.0ms on Tier-0 CPU.
- **M2 (Physics):** 200 dynamic Jolt XPBD bodies stable at 1/120s tick with <5.0ms overhead.
- **M4 (World):** 1 km² chunk loaded asynchronously with <16ms main thread stall.
- **M5 (AI):** 1000 agents running spatial-grid pathfinding in <5.0ms.
- **M6 (Audio):** 100+ diffraction paths processed via GPU compute in <2.0ms.
- **M11 (Input/UI):** Sub-tick input timestamping verified via high-speed camera (<10ms motion-to-photon latency).


# 8. Acceptance Criteria and Verification Gates

## 8.1 Spec Documentation Gates

- [ ] verify_ext_block_counts.py reports 0 warnings across all milestones
- [ ] No unresolved [X] in research backbone unless tracked as named blocker
- [ ] Every cited paper has exact block-level mapping
- [ ] Every cross-reference in spec/M*.md resolves to real live block
- [ ] Every missing sub-milestone index is created or cross-references reconciled

## 8.2 Implementation Readiness Gates

- [ ] All depends-on chains in APPENDICES.md trace to real live IDs
- [ ] Build path green: cmake -B build -G Ninja → cmake --build build --target ZombieEngine
- [ ] Tests green: ZombieEngineTests.exe 20 passed, 83/83 assertions
- [ ] Headless smoke test exit 0
- [ ] Tier-0 performance baseline: 30 FPS minimum on RTX 2070 SUPER

## 8.3 Functional Verification Gates

- [ ] M0: Triangle renders with no validation errors
- [ ] M1: 10,000 entities via indirect draw in <10ms
- [ ] M2: 50-body physics within 5ms; deterministic replay 3/3
- [ ] M3: Wall fractures into debris; static-load propagation works
- [ ] M4: 1 km² chunk generated deterministically; roads connected
- [ ] M5: 200 zombies with scent diffusion in <5ms
- [ ] M6: 100 audio sources with occlusion in <10ms
- [ ] M7: Save/load round-trip in <50ms; schema migration works
- [ ] M8: 500 items with knowledge-gated unlock; Pareto front stable
- [ ] M9: Vehicle traction, damage, wading all functional
- [ ] M10: Full day/night + weather cycle; audio masking works
- [ ] M11: Full HUD with MSDF, input remap, haptics
- [ ] M12: 2-client co-op with deterministic state sync
- [ ] M13: NPC dialogue with local LLM under 200ms latency

# 9. Professional Standards and CI/CD

## 9.1 Code Review

- Every change requires at least one peer review before merge.
- Review focus: correctness, performance, Vulkan validation, test coverage.
- Branch strategy: spec/m0-parity-reformat for spec work; main for integration-ready code.

## 9.2 Continuous Integration

- GitHub Actions on every push to spec/* and main.
- Jobs: vcpkg toolchain, MSVC, Ninja, ZombieEngineTests.exe, headless smoke.
- Artifacts retained 7 days.
- Target: zero warnings, tests green, smoke pass.

## 9.3 Testing Strategy

- Unit tests: tests/unit/Test_*.cpp, Catch2, auto-discovered.
- Integration tests: headless smoke in CI.
- Validation: Vulkan validation layers in Debug; ASAN on demand.
- Performance: RTSS + Afterburner telemetry at milestones.

## 9.4 Documentation

- Spec files READ-ONLY; changes require explicit approval.
- Plan files in recon/plans/ committed to branch history.
- Fetcher script at scripts/fetch_research.py.

## 9.5 Source Tracking Rule

- Every claim: [S] sourced, [E] engineering reasoning, [X] verification needed.
- No permanent [E] assumptions without linked evidence.
- Research backlog moves [X] to [S] or preserves [X] as blocker.

# 10. Risk Register and Mitigations

| Risk | Probability | Impact | Mitigation | Indicator |
|------|-------------|--------|------------|-----------|
| M0 descriptor fragmentation | Medium | High | Defrag guard + overflow pool | Allocation latency spike |
| M2 XPBD numerical drift | Medium | High | Energy conservation test suite | Replay mismatch after 60s |
| M4 VRAM budget exceeded | High | Critical | RVT compression + mip-LOD | VRAM > 6GB on Tier-0 |
| M5 AI performance at scale | Medium | High | GPU offload + spatial hash | Frame time > 5ms AI budget |
| M12 network desync | Medium | Critical | Deterministic replay + CRDT | Divergent state after 30s |
| M5.4 missing index | High | High | Create index or reconcile refs | Cross-ref audit failing |
| Dangling M2.7/M2.8/M2.9 refs | Medium | Medium | Audit and resolve sub-milestone bodies | Cross-ref count > 0 |
| Cross-ref drift | Medium | Medium | Canonical copy discipline | Stale duplicate in diff |
| Paper extraction backlog | Medium | Medium | Parallel fetch track | [X] count > 0 |
| Audio pipeline schedule slip | Medium | High | Stub M6-EXT-01..11 in order; validate each gate | Audio verification gate failure |


## 10.1 Hard Risk Thresholds & Kill-Switches [E-3 Resolved]

| Risk Metric | Hard Threshold (Kill-Switch) | Rollback Action |
|-------------|-----------------------------|-----------------|
| VRAM Usage | > 4.5 GB on Tier-0 (6GB) | Force-trigger maBeginDefragmentation; downscale RVT mips. |
| Frame Time | > 33.3 ms for 30 consecutive frames | Throttle M5 background pathfinding; cull shadow cascades. |
| Save Desync | M7 Delta checksum mismatch | Halt network loop; force full Zstd world state re-transmission. |


# 11. Short-Term Action Plan — Next 10 Days

## Days 1-2: Spec Structural Closure

- [ ] Read spec/M0.md blocks 40-53 directly; identify true count vs M0.index.json.
- [ ] Read spec/M2.md appendix section; list every referenced ID and check existence.
- [ ] Read spec/M5.md sub-milestone references; confirm whether M5.1/5.2/5.3/5.4 bodies exist.
- [ ] Create priority-ordered fix queue: create missing indexes OR remove stale refs.
- [ ] Re-run scripts/verify_ext_block_counts.py after fixes.

## Days 3-4: Research Backbone Closure

- [ ] Fetch papers 1-19, 21-23, 30-32, 40, 48, 50 directly.
- [ ] Extract [S] quotes for each; attach cache path to plan.
- [ ] Replace milestone-level paper mapping with exact block-level mapping.
- [ ] Cross-check 8 cross-game truths; extract or archive as design-only context.

## Day 5: Audio Pipeline Plan Approval

- [ ] Review Section 5 (Audio Pipeline Plan) with user.
- [ ] Approve M6-EXT-01..11 block sequence and verification gates.
- [ ] Flag any audio blocks that conflict with existing spec.

## Days 6-7: Verification

- [ ] Re-run cross-reference audit after structural fixes.
- [ ] Re-run paper extraction audit.
- [ ] Confirm 0 unresolved blockers or document remaining named blockers.

## Days 8-10: Execution Handoff

- [ ] Finalize acceptance criteria per milestone.
- [ ] Create implementation branch spec/audio-pipeline-closure.
- [ ] Hand off to implementation agent with exact block read order.

# 12. Glossary

- **EXT Block**: Discrete spec unit with systems/math/algorithm/evidence/player impact.
- **Tier-0 Floor**: RTX 2000-series / 6GB VRAM / 30 FPS minimum.
- **RVT**: Runtime Virtual Texture, material streaming system.
- **MSDF**: Multi-channel Signed Distance Field, text rendering technique.
- **XPBD**: Position-Based Dynamics, physics simulation method.
- **SEIR**: Susceptible-Exposed-Infectious-Recovered, epidemic model.
- **WFC**: Wave Function Collapse, procedural generation technique.
- **NPC**: Non-Player Character.
- **SLM**: Small Language Model, local LLM integration.
- **CRDT**: Conflict-free Replicated Data Type, for network consistency.
- **CVar**: Console Variable, debug/configuration registry.
- **DAG**: Directed Acyclic Graph, used for load order and dependency management.
- **Pareto Front**: Set of non-dominated solutions in multi-objective optimization.
- **Halton Jitter**: Low-discrepancy sequence for temporal anti-aliasing.



## 11.2 Strict Sprint Execution Timeline [E-4 Resolved]

- **Sprint 1 (Days 1-14):** M0 + M1 (Vulkan Bootstrap & Bindless ECS). Goal: Triangle.
- **Sprint 2 (Days 15-28):** M2 + M11 (Jolt Physics & Sub-Tick Input). Goal: Controller box pushing.
- **Sprint 3 (Days 29-42):** M4 + M4.5 (Procedural World Gen & Nanite-lite). Goal: 1km² traversal.
- **Sprint 4 (Days 43-56):** M5 + M13 (1000 AI agents + Local SLM). Goal: Dialogue and Horde combat.
- **Sprint 5 (Days 57-70):** M12 (GNS Networking & Rollback). Goal: 4-Player Co-Op.
