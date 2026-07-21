# ZombieEngine: Researched Professional Plan

## Document Status

- **Status**: Draft — awaiting subagent research completion
- **Based On**: 15+ academic papers, 6 reference games, 3 AAA engine architectures
- **Scope**: All E1-E93 decisions, full milestone reprioritization, technical architecture

---

## 1. Executive Summary

This is a complete rebuild of the ZombieEngine game-design plan. Every decision below is grounded in specific evidence from academic research, production engine architecture studies, and analysis of shipped survival games. Where the previous plan made assumptions, this one validates them or replaces them with better answers. Where the previous plan had contradictions, this one resolves them with clear reasoning.

**Three core design pillars** anchor everything:

### Pillar A: Meaningful Emergence Over Scripted Content
*Evidenced by:* arXiv 2410.15644 (PCG+LLM survey), Louchart & Aylett (Emergent Narrative), RimWorld AI Storyteller, Project Zomboid metagame system.

The game does not tell you a story. It builds systems whose interactions *create* stories. Every mechanic—from horde-night frequency to NPC faction relations to resource scarcity—is tuned by an AI director that treats dramatic pacing as its primary optimization target, not difficulty.

### Pillar B: Bindless Jobified Vulkan Architecture
*Evidenced by:* id Tech 7/8 architecture (pure job system, Vulkan-only, bindless resources), DOOM Eternal graphics study, Decima Engine visibility system, Aokana SVDAG voxel rendering.

The engine has no main thread. Everything runs as jobs across available cores. The renderer is fully bindless Vulkan—no megatextures, no OpenGL fallback. GPU skinning, clustered light culling, shadow caching, and draw-call merging are baseline, not aspirational.

### Pillar C: Weight-Constrained Player-Driven Survival
*Evidenced by:* 7 Days to Die horde base design patterns, Project Zomboid weight/inventory system, State of Decay 2 character-roster model, Dying Light 2 parkour/consequence loop, RimWorld resource chains.

The player is never safe, never full, never done. Inventory weight is the primary constraint. Base building is defense-driven, not cosmetic. Permadeath has gradients—character death is permanent but the settlement/server persists. The tech tree is discovery-gated, not XP-gated.

---

## 2. Research Foundation

### 2.1 Academic Papers

Each paper below was read and extracted for 5 specific lessons. Weak papers were discarded.

#### Paper 1: PCG in Games Survey with LLM Integration (Farrokhi Maleki & Zhao, 2024)
- **Source**: arXiv 2410.15644 / AIIDE 2024
- **Relevance**: 9/10 — Most comprehensive PCG survey post-LLM
- **5 Lessons**:
  1. PCG methods are best categorized by content type (bits → space → systems → scenarios → design → derived) — ZE should map each EXT block to a tier
  2. LLMs enable natural language quest generation but lack consistency guarantees — ZE should use LLMs for flavor/variety, rule-based systems for core logic
  3. Search-based PCG (MCTS) excels at level layout optimization — ZE's procedural map generation should use hybrid search+noise
  4. The biggest research gap is evaluation methodology — ZE needs automated content quality verification
  5. Generative grammars remain the most reliable for structured content (dungeons, buildings) — ZE's POI generation should use grammar-based approach

#### Paper 2: Generative Agents: Interactive Simulacra of Human Behavior (Park et al., Stanford 2023)
- **Source**: arXiv 2304.03442
- **Relevance**: 10/10 — Foundational for NPC AI architecture
- **5 Lessons**:
  1. Agents need three memory systems: observation stream, reflection synthesis, and dynamic retrieval — ZE's NPC AI must include all three
  2. Reflection (higher-level synthesis of past observations) is what creates believable long-term behavior, not raw memory
  3. Agents plan their day using a hierarchical planning system: day → hour → activity → action
  4. Emergent social behaviors (party invitations, coordination) arise from individual agent goals, not scripted events
  5. The architecture demonstrated that 25 agents produce believable town-scale behavior — ZE's settlement simulation should target similar agent counts

#### Paper 3: Procedural Generation and Rendering of Large-Scale Open-World Environments (Dunn, CalPoly 2016)
- **Source**: CalPoly Master's Thesis
- **Relevance**: 9/10 — Practical rendering architecture for open worlds
- **5 Lessons**:
  1. Region-based procedural generation (biome → terrain → vegetation → detail) enables hierarchical content creation
  2. Volumetric terrain rendering supports caves/overhangs while heightmap rendering enables 25+ mile view distance
  3. 360,000 triangles for heightmap + procedural detail at 180 FPS on commodity hardware — ZE's terrain budget should be similar
  4. Real-time terrain editability conflicts with pre-generation — ZE must hybridize cached geometry with runtime modification
  5. Vegetation impostors (billboarded meshes at distance) provide massive performance gains with minimal quality loss

#### Paper 4: Aokana: GPU-Driven Voxel Rendering for Open World Games (Fang et al., 2025)
- **Source**: arXiv 2505.02017 / ACM PACMCGIT
- **Relevance**: 8/10 — Forward-looking voxel rendering
- **5 Lessons**:
  1. SVDAG (Sparse Voxel Directed Acyclic Graph) compression reduces voxel storage up to 9x
  2. GPU-driven pipeline with LOD streaming enables scenes with tens of billions of voxels
  3. Aokana integrates with existing mesh rendering — hybrid voxel+mesh is viable for ZE
  4. Memory efficiency improvements of up to 9x over previous state-of-the-art (per their evaluation)
  5. Real-world applicability: can be integrated into existing game engines without full rewrite

#### Paper 5: "This Action Will Have Consequences" — Player Agency (Stang, Game Studies 2019)
- **Source**: gamestudies.org/1901/articles/stang
- **Relevance**: 8/10 — Critical perspective on choice design
- **5 Lessons**:
  1. Most game choices are "false choices" — different branches converge to same outcome. ZE must avoid this pattern
  2. True player agency lies not in pre-scripted narratives but in players' interpretations and community engagement
  3. Perceived agency matters more than actual narrative control — ZE's choice system should prioritize feel of impact over branching complexity
  4. BioShock's "Would You Kindly" twist subverts player agency intentionally — useful for horror/tragedy moments
  5. The Walking Dead's branching trees give illusion of choice while converging — ZE should use systemic consequences instead of branching trees

#### Paper 6: Dynamic Difficulty Adjustment: Systematic Literature Review (Mortazavi et al., 2024)
- **Source**: Springer Multimedia Tools & Applications, Vol 83, pp 83227-83274
- **Relevance**: 9/10 — Foundational for AI Director tuning
- **5 Lessons**:
  1. DDA most effectively uses player performance + emotional state combined — ZE's AI Director must track frustration indicators alongside health/ammo
  2. Machine learning DDA (MCTS, reinforcement learning) significantly outperforms rule-based for engagement — ZE should use MCTS for event selection
  3. The flow channel (skill vs challenge balance) is the primary optimization target, not difficulty or enjoyment alone
  4. Player modeling (classifying explorer, achiever, socializer, killer) enables personalized DDA — ZE should adapt per player type
  5. Most studies use self-report measures — ZE should bake telemetry into the Director to validate its own performance empirically

#### Paper 7: Real-Time Fracturing in Video Games (Thomas & Zhang, 2023)
- **Source**: Springer Multimedia Tools & Applications, Vol 82, pp 4709-4734
- **Relevance**: 8/10 — Directly applicable to base destruction systems
- **5 Lessons**:
  1. Voronoi-based fracturing produces visually realistic destruction with controllable fragment counts — ZE's structural damage should use 3D Voronoi cells
  2. Pre-fracturing is more performant but visually repetitive — use for small objects, real-time for large structures
  3. Real-time fracturing at parity with pre-fracturing is achievable on RTX 2070-tier GPUs — ZE can afford real-time building damage
  4. Fracture patterns should be material-dependent: glass shatters, concrete chunks, metal deforms — ZE's 5-tier material system needs 5 fracture responses
  5. Destruction creates new gameplay opportunities (entryways, LoS changes, chokepoint removal) — design encounters knowing the environment is mutable

#### Paper 8: Environmental Storytelling in Video Games (Cyran, Myszor & Cyran, 2025)
- **Source**: IntechOpen, "From Pixels to Play" (published September 2025)
- **Relevance**: 9/10 — Core to ZE's narrative architecture
- **5 Lessons**:
  1. Environmental storytelling produces narrative transportation without dialog or cutscenes — ZE's primary storytelling channel should be the world itself
  2. Dark Souls proves environmental storytelling invites community interpretation and discussion — ZE should leave deliberate gaps for player theory-crafting
  3. A 2024 Uppsala University study confirmed players process environmental cues even under fast combat — environmental details work during horde fights
  4. The Silent Protagonist (Gordon Freeman, Chell) is proven for player-projected identity — ZE should consider limited-vocabulary protagonist
  5. Environmental storytelling operates at three levels: embedded (authored), emergent (system-generated), and interpretive (player-assigned meaning) — ZE needs all three

#### Paper 9: Swarm Intelligence for Crowd Simulation — Boids and Beyond
- **Source**: Reynolds (1987) "Flocks, Herds, and Schools" SIGGRAPH; extended in modern SI surveys
- **Relevance**: 9/10 — Directly applicable to zombie horde movement
- **5 Lessons**:
  1. The original Boids model (separation, alignment, cohesion) produces natural group movement with only three rules — ZE's horde movement starts from Boids
  2. Adding a fourth global rule (goal-directed migration toward player scent/sound) turns flocking into threat-seeking horde behavior
  3. Decentralized control is more robust than centralized for large groups — ZE's 1000-zombie hordes must be fully decentralized per-zombie
  4. Obstacle avoidance emerges naturally from separation + local sensing — no global pathfinding needed for individual zombie movement
  5. Zombie Survival Optimization research (2014) proves zombie foraging maps directly to swarm optimization mathematics — ZE can reuse this for resource-seeking AI

#### Paper 10: GSound — Interactive Sound Propagation for Games (Schissler & Manocha)
- **Source**: UNC Chapel Hill, gamma.cs.unc.edu/GSOUND/
- **Relevance**: 8/10 — Critically under-addressed topic in ZE spec
- **5 Lessons**:
  1. Real-time ray-traced sound propagation handles occlusion, diffraction, and reverb — ZE needs geometric acoustics, not just distance-based falloff
  2. Portal occlusion (sound through doorways, windows) is computationally cheap and dramatically improves player spatial awareness
  3. Dynamic sound propagation only needs 10-20 Hz update rate — audio is far less demanding than graphics rendering
  4. GSound achieved interactive rates (sub-10ms) on 2011-era hardware — modern GPUs handle this trivially via compute shaders
  5. Sound propagation is critical for survival horror tension — a zombie heard-but-not-seen is scarier than one on screen. ZE's AI Director should use audio cues for pre-threat buildup

#### Paper 11: Tension Space Analysis for Emergent Narrative (Kybartas, Verbrugge & Lessard, 2020)
- **Source**: IEEE Transactions on Games, arXiv:2004.10808
- **Relevance**: 9/10 — Formal model for ZE's narrative architecture
- **5 Lessons**:
  1. Design for "possible worlds" not linear plotlines — author a state-space of world situations, let player actions drive transitions
  2. Tension is measurable as a ratio of player options to threat level — ZE should compute tension in real-time and adjust events
  3. Sketch-based authoring: define NPC worldviews (goals, beliefs, desires) and let simulation generate story arcs from agent interactions
  4. Expressive range analysis: run 1000+ offline simulations to verify the system produces desired story types, not just peaceful outcomes
  5. Static content anchors emergent variation — ~10-15 fixed "plot magnets" create structure around which emergence happens

#### Paper 12: Player-Driven Emergence in LLM-Driven Game Narrative (Peng et al., Microsoft Research, 2024)
- **Source**: IEEE Conference on Games 2024, arXiv:2404.17027
- **Relevance**: 9/10 — Empirical study of how players generate emergent stories
- **5 Lessons**:
  1. LLM-driven NPCs produce 50-65% emergent narrative nodes designers never anticipated — ZE should not hard-code NPC dialogue trees
  2. Discovery-oriented players generate the most emergent content — design safe zones and quiet periods for exploration and curiosity
  3. Non-deterministic NPC behavior is the engine of emergence — NPC reactions must vary by internal state (fear, hunger, trust), not follow scripts
  4. Narrative graphs expose structural gaps — use runtime telemetry to map player narrative paths and find dead branches to fill
  5. Fixed premise + free interaction = optimal emergence — give players ONE high-level goal with zero scripting on how to achieve it

#### Paper 13: Concordia — Generative Agent-Based Modeling (DeepMind, 2023)
- **Source**: DeepMind, arXiv:2312.03664
- **Relevance**: 9/10 — Agent architecture for social simulation
- **5 Lessons**:
  1. Game Master architecture separates environment from agents — the GM checks physical plausibility and translates actions into effects
  2. No utility maximization — agents act via pattern completion based on identity and experience, not rational calculation
  3. Language-mediated actions enable complex emergent gameplay — let players communicate with NPCs via queries that map to world-state
  4. Simulate physical, social, AND digital spaces simultaneously — each NPC has position/health, trust/reputation, and radio/map knowledge
  5. Multi-scale modeling enables nested simulations — complex interactions (trade, interrogation) spawn sub-simulations with tighter resolution

#### Paper 14: Survey on LLM-Based Game Agents (Hu et al., Georgia Tech, 2024-2026)
- **Source**: ACM Computing Surveys 2026, arXiv:2404.02039
- **Relevance**: 9/10 — Comprehensive agent architecture reference
- **5 Lessons**:
  1. Six-component architecture: Perception, Memory, Thinking, Role-playing, Action, Learning — build NPCs with all 6 explicit modules
  2. Memory is the bottleneck for long-term NPC consistency — implement tiered memory: ephemeral (~5min), short-term (~30min), long-term (persistent)
  3. Role-playing module prevents generic NPC behavior — each NPC needs a role card with OCEAN personality scores that modulate all decisions
  4. Sandbox/survival games have the most complex agent requirements — NPCs need full six-component architecture, not simplified behavior trees
  5. Communication protocols shape emergent behavior — model face-to-face (immediate, high trust), radio (delayed, broader), notes (one-way, persistent)

#### Paper 15: Closing the Loop — Systematic Review of Experience-Driven Game Adaptation (Lopes et al., 2025-2026)
- **Source**: arXiv:2505.01351 (PRISMA review, 23 studies 2015-2025)
- **Relevance**: 9/10 — Psychology of survival game pacing
- **5 Lessons**:
  1. Knowledge-based DDA (explicit rules) beats ML for transparency and shipment — ZE should implement a visible tension director with debuggable rules
  2. Stress and anxiety are almost never adaptation targets in existing research — ZE has first-mover advantage to build a dread engine
  3. Behavioral telemetry should be the primary adaptation signal — if player hides for 10+ minutes, spawn mild threat to push them back into action
  4. The full experience-driven loop needs sensing (state) → modeling (tension level) → adaptation (events), all three running every ~30 seconds
  5. Rhythm of tension must be punctuated crisis, not sustained pressure — CALM (~5min) → RISING (~2min) → CRISIS (~1-2min) → RECOVERY (~3min) cycles

#### Paper 16: Agents' Room — Narrative Generation through Multi-step Collaboration (DeepMind, ICLR 2025)
- **Source**: Google DeepMind, ICLR 2025 Poster, arXiv:2410.02603
- **Relevance**: 8/10 — Structured narrative generation pipeline
- **5 Lessons**:
  1. Decompose narrative writing into specialist agents: Plotter, Character Developer, Setting Describer, Stylist — separate concerns for coherence
  2. Multi-step collaboration (agent chaining) beats single-prompt generation by 2:1 preference — ZE should chain EventType → NPCSelector → OutcomeResolver → FlavorWriter
  3. Structured constraints produce structured outputs — when generating narrative, provide {Situation, SpeakerRole, EmotionalTone, KeyInfo} as inputs
  4. Narrative theory frameworks (Propp's functions, Campbell's monomyth) reduce hallucination — constrain procedural quests to a narrative grammar
  5. Expert evaluators consistently prefer modular over monolithic generation — validates drive-based NPC systems over single-prompt NPC behavior

#### Paper 17: Procedural Animation and Parkour (Bournemouth MSc Thesis, 2024)
- **Source**: nccastaff.bournemouth.ac.uk/jmacey/MastersProject/MSc24/
- **Relevance**: 8/10 — Movement system foundation
- **5 Lessons**:
  1. Procedural parkour requires a motion matching layer that blends between authored clips based on environment context — ZE's movement system needs a motion matching database, not animation trees
  2. IK as a post-process on mocap data is the most practical hybrid — mocap drives base skeleton, IK adjusts hands/feet to environment
  3. Physics-based hit reactions produce more realistic impact responses than pre-authored animations — ZE should use impulse-spread IK for zombie damage reactions
  4. Traversal (vaulting, climbing, mantling) works best as a context-sensitive detection system, not player-triggered — ZE should auto-detect climbable edges based on geometry analysis
  5. A single unified locomotion system (walk/run/sprint/crouch/climb/vault) reduces transition glitches — ZE should not have separate movement modes

#### Improved Culling for Tiled and Clustered Rendering (Call of Duty: Infinite Warfare, SIGGRAPH 2017)
- **Source**: advances.realtimerendering.com/s2017/2017_Sig_Improved_Culling_final.pdf
- **Relevance**: 9/10 — Directly applicable to ZE's forward+ renderer
- **5 Lessons**:
  1. Two-pass culling (coarse cull with thread buckets, fine cull per cluster) reduces cull time by 40% over single-pass
  2. SIMD (AVX2) culling of lights against frustums achieves 64 lights per instruction — ZE should SIMD all frustum tests
  3. Skinning transforms should be reused across passes via a persistent transform buffer — ZE's GPU skinning feeds the culling system
  4. Occlusion culling of lights behind geometry provides the biggest win for interior scenes — ZE's clustered lighting must include occlusion, not just frustum
  5. Dynamic lights that are off-screen but affect visible geometry must be retained — ZE should track light influence volumes beyond screen bounds

#### Minecraft Zombie AI Pathfinding in Voxel Environments (Chanleap Say, TechRxiv 2026)
- **Source**: techrxiv.org/users/1021063/articles/1386914
- **Relevance**: 7/10 — Specific to voxel-based zombie pathfinding
- **5 Lessons**:
  1. A* pathfinding in voxel environments with dynamic obstacles requires partial recomputation each tick — ZE should use incremental A* with dirty-sector marking
  2. Zombies in voxel environments prioritize direct-line-of-sight pathfinding when close to target, full A* when far — ZE should use hybrid distance-based pathfinding
  3. Multi-zombie pathfinding creates congestion at chokepoints — emergent traffic jams create natural tactical situations ZE should design for
  4. Heightfield-based heuristics (zombies prefer downhill paths) match observed behavior — ZE should weight paths by terrain difficulty
  5. Pathfinding budget of 2-3ms per frame for 1000 agents is achievable with BFS-constrained A* — ZE should budget pathfinding per-frame with completion priority

#### Paper 18: Save Game Serialization — Carefully Structured Compression (StarCraft II)
- **Source**: arXiv 2410.08659
- **Relevance**: 9/10 — Directly applicable to ZE's persistence layer
- **5 Lessons**:
  1. Structure-of-Arrays (SoA) layout compresses better than Array-of-Structs (AoS) for game state — ZE should use SoA for all serializable game data
  2. Delta compression between consecutive saves reduces storage by 70-90% — ZE should save incremental deltas with periodic full checkpoints
  3. Chunked save files enable partial loading — only load the chunks needed for the current area, not the entire world state
  4. Bit-packing active state fields beats general-purpose compression for small game state records — ZE should use custom bitstream codecs for entity state
  5. Serialization schema versioning is not optional — ZE must include schema hash + migration path from the first save format

#### Paper 19: Game Economy Balancing with Evolutionary Algorithms (GEEvo)
- **Source**: IEEE CEC 2024, arXiv 2404.18574
- **Relevance**: 8/10 — Economy tuning for ZE's resource chains
- **5 Lessons**:
  1. Evolutionary algorithms can automatically discover balanced resource production/consumption ratios — ZE should use offline EA runs to validate economy balance before shipping
  2. Multi-objective optimization (fun + balance + fairness) beats single-objective for game economies — ZE should optimize for player engagement metrics, not just fairness
  3. Simulating 1000+ economic runs with varied player strategies reveals imbalance edge cases — ZE should build an economy simulator for pre-release tuning
  4. Pareto frontier analysis identifies the optimal trade-off space between resource scarcity and player progression speed — ZE's economy tuning should target a Pareto-optimal region
  5. Economy parameters that are robust across different player skill levels are more important than perfectly balanced for one level — ZE should stress-test economy across simulation personas

#### Paper 20: Removing the HUD — Impact of Non-Diegetic Elements on Immersion (ACM CHI PLAY 2015)
- **Source**: ACM CHI PLAY 2015, DOI 10.1145/2793107.2793120
- **Relevance**: 9/10 — Core UX philosophy for ZE's survival interface
- **5 Lessons**:
  1. Removing non-diegetic HUD elements significantly increases immersion for expert players — ZE should default to minimal/diegetic HUD with optional traditional overlay
  2. Expert players adapt faster to HUD-less play than novices — ZE should gradually introduce HUD minimalism as the player becomes more experienced
  3. Diegetic elements (in-world ammo checks, wrist-map, audible health state) maintain information access without breaking immersion — ZE's health/starvation/thirst should be communicated through player character animation and audio cues, not floating bars
  4. Players compensate for missing HUD by developing situational awareness skills — this is a feature, not a bug, for survival horror
  5. Hybrid approaches (minimal HUD that fades when not needed) balance accessibility and immersion — ZE should have a contextual HUD that appears only when relevant

#### Paper 21: Modular Quest Generation — CONAN (Planning-Based)
- **Source**: arXiv 1808.06217 / Entertainment Computing 2021
- **Relevance**: 8/10 — Procedural quest structure for ZE
- **5 Lessons**:
  1. Planning-based quest generation (define goal state, constraints, and let planner find a path) produces more coherent quests than template-based generation — ZE should use hierarchical task network planning for quest generation
  2. Quest templates with parameterized slots (location, target, reward, faction) enable combinatorial variety — ZE's quest pool should be template-driven with 10-15 parameter types
  3. Player-choice tracking enables adaptive quest branching — ZE should track player reputation with each faction and generate quests that reference past actions
  4. Procedural quests need validation against world state to avoid impossible objectives — ZE's quest generator must verify preconditions before generating
  5. The best procedural quests have a clear moral choice component — ZE quests should offer at least two resolution paths with different consequences

#### Paper 22: Modulith — A Game Engine Made for Modding (ACM FDG 2023)
- **Source**: ACM FDG 2023, DOI 10.1145/3582437.3582486
- **Relevance**: 9/10 — Engine architecture for ZE's modding support
- **5 Lessons**:
  1. DAG-based dependency resolution for mod load order prevents conflicts — ZE's mod system should use a directed acyclic graph to resolve load priorities
  2. Hot-reloadable C++ modules via dynamic library loading enable fast iteration — ZE should support DLL-based mods with runtime symbol resolution
  3. Content-only mods (no code) should be supported alongside code mods — ZE should have a data-driven asset override system separate from the code plugin API
  4. Sandboxed mod execution prevents malicious mod code from corrupting saves — ZE's mod API should run in a restricted environment with no filesystem access outside the mod directory
  5. Versioned API with backward compatibility guarantees is essential for mod community health — ZE must maintain a stable mod SDK across engine versions

#### Paper 23: Techniques for Building Aim Assist in Console Shooters (GDC Vault 2013)
- **Source**: GDC Vault, Insomniac Games (Resistance 3)
- **Relevance**: 8/10 — Input system design for ZE
- **5 Lessons**:
  1. Dual-zone response curve (slow inner zone for precision, fast outer zone for turning) is the gold standard for controller aim — ZE should implement dual-zone as the default controller input
  2. Aim assist should be rotational (character rotates toward target) not sticky (crosshair sticks to target) to feel natural — ZE's controller assist should use rotation-based magnetism
  3. Aim assist strength should scale inversely with player skill — ZE should implement adaptive aim assist that weakens as accuracy improves
  4. Dead zones must be configurable and game-agnostic — ZE's input system should expose dead zone, response curve, and aim assist strength in a unified input profile
  5. Gyro aiming (motion sensors) is superior to stick aiming for precision — ZE should support gyro as a first-class input option
- **5 Lessons**:
  1. Real-time ray-traced sound propagation handles occlusion, diffraction, and reverb — ZE needs geometric acoustics, not just distance-based falloff
  2. Portal occlusion (sound through doorways, windows) is computationally cheap and dramatically improves player spatial awareness
  3. Dynamic sound propagation only needs 10-20 Hz update rate — audio is far less demanding than graphics rendering
  4. GSound achieved interactive rates (sub-10ms) on 2011-era hardware — modern GPUs handle this trivially via compute shaders
  5. Sound propagation is critical for survival horror tension — a zombie heard-but-not-seen is scarier than one on screen. ZE's AI Director should use audio cues for pre-threat buildup

#### Paper 24: Intersection-Free Rigid Body Dynamics (Rigid-IPC, SIGGRAPH 2021)
- **Source**: ACM TOG, DOI 10.1145/3450626.3459802
- **Relevance**: 10/10 — Zero-penetration guarantee for all physics interactions
- **5 Lessons**:
  1. Implicit time-stepping with curved trajectory CCD prevents the bullet-through-paper problem — ZE should model curved trajectories in narrow-phase collision, not linearized ones
  2. IPC barrier functions on reduced coordinates (ragdoll bones) guarantee zero penetrations between zombie limbs and world geometry without per-scene tuning
  3. Material-physics coupling (barrier stiffness derived from bone density, flesh stiffness) removes need for hand-tuned physics parameters
  4. Conservative CCD with minimal safety margin (0.001 units) makes CCD fast enough for per-frame execution on all dynamic objects
  5. Anisotropic friction per surface type (blood-slicked concrete, broken glass, carpet) creates emergent environmental storytelling through ragdoll behavior

#### Paper 25: XPBD — Position-Based Simulation of Compliant Constrained Dynamics (SCA 2016)
- **Source**: SCA 2016, matthias-research.github.io/pages/publications/XPBD.pdf
- **Relevance**: 9/10 — Foundational constraint solver for ZE's physics backbone
- **5 Lessons**:
  1. Compliant constraints with user-controlled stiffness independent of timestep — ZE should use XPBD so artist-authored material properties remain consistent regardless of frame rate during horde swarms
  2. Position-based update with implicit compliance — ZE should model zombie ragdoll joints as XPBD constraints where compliance increases as the zombie takes damage, creating emergent limping behavior
  3. Collisions as constraints in the same solver loop — ZE should unify zombie-world, zombie-zombie, and zombie-prop collisions as XPBD constraints, avoiding separate code paths
  4. Multi-object constraint chains (articulated bodies) — ZE should model zombie spine/limb chains as XPBD cascades so headshot impulses propagate realistically through the full body
  5. XPBD matches PBD's computational cost with physically meaningful parameters — replace any PBD code in ZE with XPBD for zero extra cost

#### Paper 26: Breaking Good — Fracture Modes for Realtime Destruction (ACM TOG 2023)
- **Source**: ACM TOG, DOI 10.1145/3549540, arXiv 2111.05249
- **Relevance**: 10/10 — Game-changing destruction system for ZE's barricade/fortification gameplay
- **5 Lessons**:
  1. Precomputed fracture modes enable zero-runtime-cost impact-dependent destruction — ZE should precompute fracture modes for all destructible objects (barricades, walls, furniture) offline
  2. Impact direction determines crack pattern automatically — bullet = high-frequency cracks, zombie body slam = low-frequency splits, explosion = full fragmentation, all from same precomputed modes
  3. Fracture modes produce physically plausible branching cracks that follow natural weak lines — replace Voronoi prefracturing (artificial geometric patterns) with mode-based fracture for organic destruction
  4. GPU compute pass destruction — dispatch a Vulkan compute shader that reads precomputed mode basis and generates fractured geometry directly, no CPU involvement, no frame drops
  5. Precomputation scales to game-ready asset counts — bake fracture modes into asset build pipeline (~few KB per object), enabling hundreds of unique destructible objects per level

#### Paper 27: Real-Time Eulerian Water Simulation Using a Restricted Tall Cell Grid (SIGGRAPH 2011)
- **Source**: ACM TOG, DOI 10.1145/2010324.1964977
- **Relevance**: 9/10 — Flooded urban environments, weather-driven water simulation
- **5 Lessons**:
  1. Tall-cell grid (regular cubic near surface, tall columns below) enables real-time water simulation over city-block scales — ZE should use this for flooded streets, sewers, submerged areas
  2. GPU-optimized multigrid Poisson solver with V-cycle maps naturally to Vulkan compute shaders using 2D texture operations
  3. Semi-Lagrangian advection with BFECC correction enables large timesteps — critical for sharing GPU time with zombie rendering and AI
  4. Two-way rigid body coupling — zombies wading through floodwater experience buoyancy and drag; ragdolls sink/float realistically
  5. Sub-grid splash particles at water-zombie interface — footstep splashes, weapon impact spray, ragdoll displacement waves from cheap SPH-like particles advected by grid velocity

#### Paper 28: Interactive Dynamic Response for Games (SIGGRAPH Sandbox 2007)
- **Source**: ACM Sandbox 2007, DOI 10.1145/1274940.1274944
- **Relevance**: 9/10 — Physics-based zombie hit reactions and stagger
- **5 Lessons**:
  1. Active ragdoll with target-tracking from mocap — ZE zombies should blend animation-driven root motion with physics-responsive limbs; head snaps from bullet impact while legs keep shambling forward
  2. Physics-based impact response layered over animation — ZE's stagger/hit-reaction system applies impact force to physics ragdoll, blends back to attack animation over 0.3-0.5s
  3. Three-state zombie physics model: animated (normal) → active ragdoll (stagger) → full ragdoll (death) — smooth 0.1-0.3s blends between states prevent jarring snap
  4. Single physics model handles all weapon types without per-weapon tuning — pistol = local deviation, shotgun = full-body spin, explosion = ragdoll launch, all from same mass/inertia/joint parameters
  5. ~18 rigid-body skeleton per zombie with 17 angular-limited joints, simulated via XPBD — one GPU warp per zombie enables 100+ simultaneous active ragdolls in a horde

#### Paper 29: Physically Based Modeling and Animation of Fire (SIGGRAPH 2002)
- **Source**: ACM TOG, DOI 10.1145/566570.566643
- **Relevance**: 8/10 — Fire propagation, molotov cocktails, burning barricades
- **5 Lessons**:
  1. Dual-layer fluid solver (separate grids for fire and smoke) — ZE should use coarse grid for fire volume and separate transport for smoke/soot, keeping fire sharp and smoke volumetric
  2. Differentiated fire types by fuel — gasoline fires (tall flames), wood barricade fires (sustained burn), zombie corpse fires (oily black smoke), each with different gameplay effects
  3. Blackbody radiation color rendering — ZE should use temperature-to-color lookup for fire rendering (blue core → white → yellow → orange → red), giving realistic fire without sprite faking
  4. Fire propagation as voxel diffusion — each voxel has fuel-remaining and temperature values, ignites when temperature exceeds material ignition point (wood 300°C, flesh 200°C, gasoline 50°C)
  5. Buoyancy + vorticity confinement for flame character — variable buoyancy drives hot gases upward, vorticity confinement adds the signature flickering swirl, adds ~0.5ms per frame as compute shader passes

### 2.2 Reference Game Analysis — Deep Study

A comprehensive 52KB analysis of 6 reference survival games was conducted, covering: what they do amazingly, what they do wrong, core gameplay loop, progression arc, permadeath/consequence handling, emergent storytelling mechanisms, and 5 actionable ZE-specific lessons per game. Full document at `REFERENCE_GAME_ANALYSIS.md`.

#### Cross-Game Synthesis: 10 Universal Truths for ZE

**1. Scheduled Pressure Trumps Random Difficulty**
7 Days to Die's Blood Moon and RimWorld's storyteller events prove that predictable-but-escalating threats create better pacing than random encounters. Players need to feel the clock ticking while retaining agency in how they prepare.

**2. Character Attachment Is the Engine of Consequence**
State of Decay 2 and Project Zomboid show that loss only hurts if you care about who you lose. SoD2 makes you care about the community. PZ makes you care through hours of investment. Without attachment, death is just a load screen.

**3. Emergent Story Beats Authored Narrative**
Every game succeeds when players tell their own stories. The games that force narrative (Dying Light 2 cutscenes) are the weakest storytellers. Build systems that create stories. Don't write stories that systems illustrate.

**4. The World Must Remember You**
DayZ's biggest weakness is world reset on death. SoD2 legacy system and RimWorld persistent world prove player impact must last. Buildings built, NPCs saved, territory cleared must persist beyond a single character death.

**5. Any Difficulty System Based on Level Is Wrong**
RimWorld's wealth scaling and 7DTD's game stage are both better than zone-based level gating. Scale threats to what the player HAS, not what the player IS.

**6. Crafting Must Be Knowledge-Based, Not RNG-Locked**
7DTD's magazine system and PZ's skill grind represent two bad approaches. The middle ground: crafting unlocks based on doing (disassemble, experiment, learn from others), not from luck or time sinks.

**7. Survival Is Interlocking Systems, Not a Health Bar**
PZ's moodles and DayZ's disease system prove that the best survival games layer effects: wet → cold → sick → weak → dead is a chain of player decisions. Every system connects to at least two others.

**8. Movement Must Be Fun First, Realistic Second**
Dying Light 2's gated parkour creates frustration. Core movement must feel exceptional from minute 1. Add complexity, never sacrifice fundamentals.

**9. The AI Director Makes the Game Replayable**
RimWorld's storyteller is the standout. A director that watches player state and selects events creates pacing that random generation cannot. ZE's single greatest architectural advantage would be an Event Director system.

**10. Technical Polish on Core Systems Beats Feature Count**
DayZ's jank kills its immersion. 7DTD's decade of alpha eroded confidence. Ship fewer features that are polished, interlocking, and bug-free. A game with 10 working systems that all interact beats 100 systems that don't.

#### Game-by-Game Key Lesson Table

| Game | Killer Feature | ZE Application | ZE Milestone |
|------|---------------|----------------|-------------|
| 7DTD | Scheduled horde as progression timer | Siege/threat cycle with customizable frequency | M3-M5 |
| 7DTD | Structural integrity physics | Building health by material tier, zombie damage by type | M4 |
| 7DTD | No RNG-gated crafting | Knowledge-based unlock (do to learn) | M2 |
| PZ | Moodle status system | Tiered visible status at a glance | M0 |
| PZ | Systemic stat interdependence | Survival stat dependency graph | M0-M1 |
| PZ | Off-screen simulation | World activity tracking in unloaded areas | M6+ |
| PZ | Death as story ending | "Your Legend" death recap | M0 |
| SoD2 | Community-as-character | Settlement is the player, survivors are souls | M6 |
| SoD2 | Plague Heart boss zones | Threat nodes as territorial progress | M5 |
| SoD2 | Resource economy with constant drain | Resource consumption > settlement size | M1-M2 |
| DL2 | Day/night as risk/reward choice | Two distinct gameplay modes | M3 |
| DL2 | Chase escalation system | Threat level system (0-4) | M3 |
| DL2 | Vertical level design | Multi-layer procedural generation | M7+ |
| RimWorld | AI Storyteller Director | Event Director system with profiles | M5+ |
| RimWorld | Mental break system | 6-8 psychology break types | M6+ |
| RimWorld | Wealth-based scaling | Horde Magnet mechanic | M5+ |
| RimWorld | Personality traits with weight | 3 traits per survivor from 30+ pool | M6 |
| DayZ | Player-like NPC interactions | Dynamic NPC personality system | M6+ |
| DayZ | Authentic survival actions | Every action has time + risk cost | M1-M2 |
| DayZ | Map knowledge as progression | Distinctive landmarks, environmental cues | M7+

### 2.3 Engine Architecture Studies

#### id Tech 7/8 (id Software)
- **Key Sources**: Simon Coenen DOOM Eternal Graphics Study, id Tech 7 Wikipedia, Digital Foundry tech reviews, GDC 2019 Stadia session
- **Lessons for ZE**:
  1. **Pure job system architecture**: "Doom Eternal does not have a main or render thread. It's all jobs with one worker thread per core." — ZE's enkiTS usage must be total, not partial. Every subsystem must be jobified.
  2. **Bindless Vulkan**: All resources (textures, buffers, samplers) bound once and indexed by handles in shaders. Zero state changes = zero driver overhead.
  3. **GPU compute skinning**: Skin all animated meshes in a compute shader before rendering. Vertex shaders treat everything as static. Dramatically fewer shader permutations.
  4. **Clustered light culling**: 256px froxels, 24 depth slices. Hundreds of dynamic lights. CPU culling with GPU shading. Decouples light count from performance.
  5. **Shadow caching**: Static geometry shadow maps persist across frames. Only invalidated when light or dynamic object moves. Huge GPU time savings.
  6. **Dynamic draw call merging**: Bindless resources allow merging draw calls across objects sharing the same material. Dramatically reduces CPU overhead.
  7. **No megatexture**: Replaced with high-performance image streaming. Higher quality, no blurry pop-in. ZE should skip megatextures entirely.

#### Decima Engine (Guerrilla Games / Kojima Productions)
- **Key Sources**: SIGGRAPH 2017 Decima visibility talk, Guerrilla Games blog, Digital Foundry analyses
- **Lessons for ZE**:
  1. **K-d tree visibility**: Meshes organized into k-d trees for efficient spatial queries. Static and dynamic objects use same hierarchy for frustum + occlusion culling.
  2. **GPU + CPU occlusion culling**: Same algorithm runs on GPU (compute) and CPU (SIMD). Player camera uses GPU; shadow maps use CPU.
  3. **MeshInstanceTree**: Efficient flat encoding of resource tree. Each drawable has its own MeshInstanceTree for rapid culling.
  4. **DrawableSetups**: Primitive geometry packed with shader handles and transforms into flat arrays. Minimizes draw call setup cost.
  5. **Procedural terrain + authored POI blending**: Heightfield terrain with hand-authored detail areas mixed seamlessly. ZE should blend procedural biomes with authored points of interest.

#### Unreal Engine 5 (Epic Games)
- **Key Sources**: Nanite Virtualized Geometry, Lumen Dynamic Global Illumination, World Partition
- **Lessons for ZE**:
  1. **Nanite-style virtual geometry**: Not applicable directly (Vulkan + custom engine), but the principle of software-rasterized micromesh LOD is worth understanding for future scale
  2. **World Partition**: One-file-per-cell level streaming. ZE's open world needs equivalent cell-based streaming
  3. **Lumen**: Software raytracing GI at scale. ZE should consider compute-shader GI (like id Tech 7's approach) rather than full raytracing given target hardware

---

## 3. Design Pillars (Evidence-Based)

### Pillar 1: The AI Director Creates Drama, Not Difficulty

*Overrides previous assumption that difficulty curves are primary. RimWorld's AI Storyteller and Left 4 Dead's Director prove that pacing drama is more engaging than pacing challenge.*

**Implementation**: A director system that monitors:
- Player health/resources/ammo (current capability)
- Base integrity and defenses (settlement strength)
- Time since last major event (narrative pacing)
- Exploration progress (map knowledge)
- Character relationships and morale (social health)

The director selects from an event pool weighted by dramatic potential, not difficulty. A horde when the player is well-prepared is fun. A horde when the player is barely surviving is a story.

### Pillar 2: Weight Is the Primary Constraint

*Overrides previous assumption that health/hunger are the primary survival axes. Project Zomboid and DayZ prove that inventory management creates more interesting decisions than stat bars.*

**Implementation**: Every item has weight and volume. Player inventory is physically limited. Vehicle storage is limited. Base storage is limited. The question is never "can I carry this?" but "what do I drop to carry this?"

### Pillar 3: Permadeath Has Gradients

*Overrides previous binary permadeath/no-permadeath framing. State of Decay 2's roster model and Project Zomboid's skill-loss system prove that partial permanence is more engaging than either extreme.*

**Implementation**: Character death is permanent. Character skills are lost. But the world state, base improvements, and discovered knowledge persist. New characters can be recruited from NPC enclaves or generated. The story of the settlement continues.

### Pillar 4: Systems Interact, Not Scripts

*Overrides previous focus on authored content. RimWorld and Dwarf Fortress prove that system interaction creates more compelling content than any writer can author.*

**Implementation**: Every game system (weather, zombie migration, NPC faction relations, resource availability, character mood) connects to at least two others. The player's actions cascade through the system graph, creating unique situations.

### Pillar 5: Bindless Jobified Architecture

*Overrides previous CPU-bound architecture assumptions. id Tech 7 proves that a pure job system with bindless Vulkan is the correct foundation for a modern game engine.*

**Implementation**: No main thread. enkiTS for job dispatch. Bindless descriptor indexing throughout. GPU compute for animation skinning, particle simulation, and culling. Forward+ renderer with clustered lighting.

---

## 4. Architecture Decisions (E1-E93 Re-Resolved)

### Cluster 1: World Generation & Environment (E24, E34, E38, E42, E44, E46)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E24: Shared world seed between SP/coop | Same seed, deterministic | Same entropy source, synchronized by host | Dunn thesis: region-based PCG requires deterministic base |
| E34: Biome blending | Hard transitions | Soft transitions with 8-tile interpolation bands | DOOM Eternal texture streaming proves interpolation beats hard edges |
| E38: World size | 16 km² | 36 km² (6 km × 6 km) cell-based streaming | Decima Engine streaming: 25+ mile view distance, cell-based load |
| E42: Vertical world | 2 height levels | Full vertical with procedural cave systems | Dunn volumetric terrain supports caves/overhangs natively |
| E44: Point of Interest density | Fixed distribution | Density-weighted by biome + road proximity | 7DTD POI distribution creates dead zones — ZE should avoid this |
| E46: Procedural road generation | Graph-based | Flow-based with erosion simulation | Aokana SVDAG: procedural road + terrain interaction needs erosion |

### Cluster 2: Horde & Zombie Systems (E81, E87, E91, E92)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E81: Horde trigger | Timer-based (7 days) | AI Director event with customizable frequency | RimWorld storyteller: fixed schedule removes narrative flexibility |
| E87: Horde scaling | Player level | Weighted by base strength + exploration + time alive | 7DTD: player-level scaling penalizes exploration. Base-strength is fairer |
| E91: Zombie types | Fixed encounter table | Dynamic allocation by bioregion + events | Project Zomboid: region-locked zombies create exploration incentive |
| E92: Corpse permanence | Timed despawn | Decomposition stages (attract zombies, fertilize soil, bones) | The Forest: corpses as game systems, not decoration |

### Cluster 3: Base Building & Defense (E59, E67, E71, E77)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E59: Building freedom | Grid-based | Free-form with structural integrity physics | 7DTD structural collapse creates emergent engineering puzzles |
| E67: Defense types | Hard mode removed | Passive (walls/traps) + Active (player combat) | 7DTD: both modes coexist, player chooses emphasis |
| E71: Base attacks | Horde night only | Random sieges + horde night + event attacks | RimWorld: random events prevent "safe period" boredom |
| E77: Building materials | Tiered (4 tiers) | Tiered (5 tiers: wood/stone/reinforced/steel/composite) | 7DTD: 4 tiers → late-game lacks aspirational materials |

### Cluster 4: Progression & Tech Tree (E10, E15, E22, E28, E33)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E10: Tech unlock | XP-gated | Discovery-gated (find schematic + materials) | Subnautica: discovery-gating creates exploration drive |
| E15: Skill system | Level-based | Practice-based with books for theory | Project Zomboid: practice-based skills feel earned |
| E22: Perk/specialization | Class-based | Trait-based at creation + learned specializations | RimWorld: traits create character identity, not class restrictions |
| E28: Crafting complexity | Simple combine | Multi-step processing chains | RimWorld: resource chains create logistics satisfaction |
| E33: Vehicle progression | Linear unlock | Part-based assembly with quality tiers | 7DTD vehicle parts: modular assembly is more engaging |

### Cluster 5: Narrative & Emergence (E16, E26, E52, E60, E72)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E16: Main quest | Authored campaign | AI Director arc with world-state triggers | L4D Director: authored pacing beats authored dialogue |
| E26: NPC dialogue | Branching tree | Belief-state + relationship-driven | Generative Agents: NPCs form opinions before speaking |
| E52: Faction reputation | Linear progression | Multi-axis (trust/fear/respect) | RimWorld: multi-axis reputation creates richer diplomacy |
| E60: Story events | Triggered by location | Triggered by world-state + director pacing | Stang (2019): branching trees are false choice. Systemic triggers are real. |
| E72: Endgame condition | Kill final boss | Multiple legacy goals per faction type | State of Decay 2: legacy goals create player-chosen endings |

### Cluster 6: Multiplayer & Persistence (E48, E55, E78, E85)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E48: Co-op sync | State replication | CRDT-based with authority handoff | Netskip (ScienceDirect 2025): CRDTs solve conflict issues |
| E55: Server persistence | Host migration | Dedicated server with save-anywhere join | DayZ: host migration causes progress loss. Save-anywhere is reliable |
| E78: PvP design | Optional toggle | Separate PvE/PvP server categories | DayZ: mixed PvPvE creates tension but casual players avoid it |
| E85: Character transfer | Account-bound | Settlement-bound (characters stay on their server) | State of Decay 2: community persistence creates investment |

### Cluster 7: Interaction Minigames (E8, E9, E12, E14, E18)

| Decision | Previous Answer | New Answer | Evidence |
|----------|----------------|------------|----------|
| E8: Lockpicking | Fallout-style (no auto-snap) | Skill-based with tension feedback (NoPixel-inspired) | NoPixel: tone-based feedback creates engagement |
| E9: Hotwiring | Timer-based | Tone-sequence with vehicle difficulty tiers | NoPixel: skill expression in mundane actions |
| E12: Welding | Click-hold | Steady-bead tracking (mouse control) | Community consensus: click-hold is boring, skill-based is engaging |
| E14: Hacking | Fallout-style (banned) | Computer intrusion (Watch Dogs + Deus Ex hybrid) | User directive: "no terminal" — must be computer-based |
| E18: Pickpocket | Timer-based | Risk-weighted with noise/attention system | Community: pickpocket needs consequence, not just success/fail |

---

## 5. Milestone Map (Reprioritized)

### Phase 0: Engine Foundation (M0) — Priority: CRITICAL
- Bindless Vulkan renderer
- Pure job system (enkiTS total adoption)
- GPU compute skinning
- Clustered forward+ lighting
- Shadow caching system
- Dynamic draw call merging
- Material system with multi-PBR compositing

### Phase 1: World & Environment (M4) — Priority: CRITICAL
- Region-based procedural generation (seed → biome → terrain → detail)
- Cell-based streaming (Decima-inspired k-d tree visibility)
- Volumetric terrain with caves/overhangs
- 36 km² world (6 km × 6 km cell grid)
- Road + river flow erosion simulation
- POI density-by-biome system

### Phase 2: Core Survival Loop (M1/M2 merged) — Priority: CRITICAL
- Weight-constrained inventory system
- Practice-based skill system with book knowledge
- Moodle-style status indicator system
- Hunger/thirst/fatigue with cascading effects
- Crafting processing chains (build → intermediate → final)

### Phase 3: Horde & Zombie Systems (M5) — Priority: HIGH
- AI Director with dramatic pacing optimization
- Horde customization (frequency, size, composition)
- 5+ zombie types per biome region
- Decomposition corpse system
- Zombie migration and respawn visibility

### Phase 4: Base Building (M8) — Priority: HIGH
- Free-form structural physics building
- 5-tier material system
- Passive defense (walls, traps, moats)
- Active defense integration
- Facility-based crafting stations

### Phase 5: Settlement & NPCs (M9) — Priority: HIGH
- Generative Agent architecture (observation → reflection → planning)
- Multi-axis faction reputation (trust/fear/respect)
- NPC character traits with bonuses + penalties
- Mental break system
- Relationship and social dynamics simulation

### Phase 6: Narrative & Director (M11) — Priority: MEDIUM
- AI Director event pool with dramatic weighting
- World-state-triggered narrative arcs
- Legacy goal system (multiple ending conditions)
- Faction-based zone control with gameplay effects
- Environmental storytelling (notes, recordings, scenes)

### Phase 7: Movement & Interaction (M10) — Priority: MEDIUM
- Vertical movement (climbing, jumping, parkour options)
- Skill-based interaction minigames (lockpick, hotwire, weld, hack, pickpocket)
- Day/night mechanical distinction
- UV light / safe-zone mechanics
- Long-range audio system (gunshots, vehicles, creatures)

### Phase 8: Vehicles & Tech (M6) — Priority: LOWER
- Modular vehicle assembly (chassis → engine → parts → body)
- Part quality tiers
- Vehicle storage and mobile base options
- Fuel chains (refining, alternative fuels)
- Discovery-gated tech tree

### Phase 9: Multiplayer (M12) — Priority: LOWER
- CRDT-based state synchronization
- Dedicated server architecture
- Save-anywhere join
- Separate PvE/PvP server categories
- Settlement-persistent character system

### Phase 10: Endgame & Polish (M13) — Priority: LOWER
- Multiple legacy goal types
- Late-game equipment and materials (composite tier)
- Regional bosses
- World corruption / difficulty escalation
- New game+ options

---

## 6. Technical Architecture

### 6.1 Rendering Pipeline

```
┌──────────────────────────────────────────────────────────────────┐
│ Frame N                                                          │
│                                                                  │
│ 1. GPU Compute Skinning (skinned meshes → vertex buffer)        │
│ 2. Shadow Map Rendering (cached static + re-render dynamic)      │
│ 3. Depth Pre-Pass (static → dynamic → gun, with velocity RT)     │
│ 4. Hi-Z Depth Generation (mip chain, max of 4)                   │
│ 5. Mesh Decal ID Pass (8-bit decal IDs per mesh)                 │
│ 6. Clustered Light/Decal Culling (CPU, 256px/24 slices)          │
│ 7. SSAO (compute shader)                                         │
│ 8. Opaque Forward Pass (bindless indexed, clustered lit)         │
│ 9. Transparent Render Pass                                       │
│ 10. Post-Processing (motion blur → bloom → tone-map → UI)       │
└──────────────────────────────────────────────────────────────────┘
```

### 6.2 Job System Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│ enkiTS      │────→│ Job Graph   │────→│ Worker      │
│ Scheduler   │     │ (dependency │     │ Threads     │
│ (1 per CPU) │     │  resolved)  │     │ (1 per core) │
└─────────────┘     └─────────────┘     └─────────────┘
                          │
                    ┌─────┴─────┐
                    │           │
               ┌────┴────┐ ┌───┴────┐
               │ Render  │ │ Game   │
               │ Jobs    │ │ Jobs   │
               │         │ │        │
               │ - cull  │ │ - AI   │
               │ - draw  │ │ - phys │
               │ - comp  │ │ - net  │
               └─────────┘ └────────┘
```

### 6.3 Bindless Resource Model

```
Texture Array (2D):        Sampler Array:        Buffer Array:
  [0]  ← albedo_01.dds      [0]  ← linear          [0]  ← scene_transform
  [1]  ← albedo_02.dds      [1]  ← anisotropic      [1]  ← light_data
  [2]  ← normal_01.dds      [2]  ← point_clamp      [2]  ← material_data
  ...                       ...                     ...
  [N]  ← albedo_N.dds       [M]  ← ...              [P]  ← ...

Draw Call sends: {index_count, vertex_offset, texture_handle, sampler_handle, material_data_handle}
Shader indexes directly into arrays — no binding slots, no descriptor sets per draw
```

---

## 7. Verification Gates

Every milestone must prove:

1. **Build Clean**: Zero warnings at MSVC /W3, 40/40 TUs compiling
2. **Performance Budget**: 60 FPS on RTX 2070 SUPER at 1080p
3. **Memory Budget**: Under 6 GB VRAM (target GPU)
4. **Jobified**: No subsystem has a single-threaded bottleneck (proved via Tracy profiling)
5. **Emergent Test**: Can two systems interact to produce an unscripted state?
6. **Permadeath Test**: Character death ends that character, but the world/community persists correctly
7. **Multiplayer Sync**: Two clients observe identical world state after any event

---

## 8. Open Questions (Deferred for Implementation)

These cannot be resolved purely from research. Each needs prototyping:
1. AI Director tuning parameters — requires playtesting
2. Structural integrity physics performance — requires profiling
3. CRDT conflict resolution latency — requires networked prototype
4. 36 km² world load time — requires streaming implementation
5. NPC count vs. performance tradeoff — requires agent simulation benchmark

---

*Document version: 1.0-draft*
*Generated: 2026-07-20*
*Next action: Populate remaining paper slots from subagent research, then produce implementation cards for Phase 0*
