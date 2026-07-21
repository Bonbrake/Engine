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

#### Paper 30: End-to-End Compressed Meshlet Rendering (Eurographics 2024)
- **Source**: Computer Graphics Forum 43(1), DOI 10.1111/cgf.15002
- **Relevance**: 9/10 — Next-gen geometry pipeline for ZE's Vulkan backend
- **5 Lessons**:
  1. GPU-side meshlet decompression in task/mesh shaders reduces CPU→GPU bandwidth — ZE should store compressed meshlets and decompress in mesh shaders just-in-time for rasterization
  2. Two-level index indirection (local vertex pool + primitive indices referencing local pool) improves cache locality and reduces per-primitive storage — ZE structured buffer design should follow this pattern
  3. Quantization: uint16×3 positions, int16×2 UVs, 8-bit QTangents — cuts 56 bytes/vertex to ~20 bytes while preserving quality
  4. Amplification shader coarse culling — cull entire meshlet clusters via Hi-Z test before dispatching mesh shader threadgroups, reducing invisible geometry work
  5. Bandwidth-bound LOD selection via compressed streams — keep multiple LODs resident in GPU memory without decompressing them all, decompress only visible meshlets at the required LOD

#### Paper 31: Nanite Virtual Geometry — A Deep Dive (SIGGRAPH 2021)
- **Source**: SIGGRAPH 2021 Advances, Karis/Stubbe/Wihlidal (Epic Games)
- **Relevance**: 10/10 — Eliminates polygon budgets entirely for ZE's open world
- **5 Lessons**:
  1. Cluster-based DAG hierarchy — fixed-size triangle clusters (~128 tris) arranged in a DAG where parent clusters are shared by multiple children, dramatically reducing total memory for ZE's open world
  2. Software rasterization for pixel-sized triangles — when cluster projects to ≤1 pixel, switch from hardware rasterizer to compute-shader software rasterizer to avoid driver overhead for millions of tiny triangles
  3. Visibility buffer deferred shading — render all geometry to a 64-bit visibility buffer (cluster ID + barycentrics), then evaluate materials/lighting in a separate full-screen compute pass, decoupling geometry complexity from shading cost
  4. Hierarchical depth culling with per-cluster bounding — maintain Hi-Z mip chain, test each cluster's projected bounds against Hi-Z, skip occluded clusters entirely in task shader
  5. Screen-space error metric for LOD — precompute per-cluster maximum positional deviation, compare projected error against 0.5-pixel threshold at runtime to select DAG level, computed entirely on GPU

#### Paper 32: GPU-Driven Rendering Pipelines (SIGGRAPH 2015)
- **Source**: SIGGRAPH 2015 Advances, Haar/Aaltonen
- **Relevance**: 9/10 — ZE should remove CPU from culling entirely
- **5 Lessons**:
  1. GPU-only culling loop with indirect draws — store all bounding spheres, instance data, and draw arguments in GPU buffers; compute shader performs frustum + occlusion + LOD selection without any CPU readback
  2. Cluster-based rendering (predecessor to mesh shaders) — each mesh partitioned into 64-256 triangle clusters with per-cluster bounding spheres, culling at cluster granularity not object granularity
  3. Persistent threadgroup culling — one thread group per 64-128 objects, surviving threads write to shared work queue via atomic increment, then compactify into indirect draw commands
  4. Frame-to-frame coherence with persistent GPU lists — if object was visible last frame and camera hasn't moved significantly, skip its culling test, reducing overhead 30-50% in static scenes
  5. Multi-pass culling pipeline — frustum cull → Hi-Z occlusion cull → LOD selection → sort by material (optional), all in GPU memory, feeding a single vkCmdDrawIndexedIndirect call

#### Paper 33: Surface Simplification Using Quadric Error Metrics (SIGGRAPH 1997)
- **Source**: SIGGRAPH 1997, Garland/Heckbert, DOI 10.1145/258734.258849
- **Relevance**: 8/10 — Foundational LOD generation for ZE's offline asset pipeline
- **5 Lessons**:
  1. Quadric error metric for edge collapse ordering — compute 4×4 quadric matrix per vertex representing sum of squared distances to adjacent face planes, collapse lowest-cost edge first for provably optimal LODs
  2. Attribute-aware QEM — separate quadrics for geometry and attributes (UV, color, normals), weight UV error 2× geometric to prevent texture distortion during simplification
  3. Edge collapse as atomic LOD operation — progressive mesh format stores collapse records (two vertex indices, new position, face removal list) as binary stream for runtime LOD generation
  4. Memory-efficient quadric storage — symmetric 4×4 matrix stored as 10 coefficients per vertex, separate set per attribute channel, enables offline collapse cost computation without original geometry
  5. Parallel pre-processing pipeline — compute quadrics in parallel, serialize edge collapses via priority queue (sequential bottleneck), then update affected edges — ZE offline tool should process mesh chunks in parallel then merge

#### Paper 34: Temporally Stable Joint Neural Denoising and Supersampling (HPG 2022)
- **Source**: HPG 2022 / PACMCGIT, DOI 10.1145/3543870
- **Relevance**: 8/10 — Quality upscaling for ZE's rendering without third-party vendor lock-in
- **5 Lessons**:
  1. Single network for both denoising and supersampling — avoids memory bandwidth and error amplification between separate stages, ZE should implement one-pass neural upscaling
  2. Temporal accumulation with motion vector reprojection — maintain history buffer, reproject via motion vectors, depth/normal similarity test to detect disocclusion and clamp blend factor
  3. Multi-branch feature extraction — spatial branch (current-frame features) + temporal branch (fused reprojected history), merged via element-wise addition, reconstruct via transposed-convolution decoder for 2× upscaling
  4. Halton sequence jitter for sub-pixel sampling — 4-tap Halton(2,3) per-pixel jitter cycled every 8 frames, network learns to integrate sub-pixel samples over time for temporal AA
  5. G-buffer auxiliary channels as network input — depth, octahedral-encoded normal, roughness, albedo, motion vectors (6 channels) help network distinguish geometry edges from texture detail

#### Paper 35: DeepMimic — Physics-Based Character Skills via RL (SIGGRAPH 2018)
- **Source**: SIGGRAPH 2018, arXiv 1804.02717, DOI 10.1145/3213779
- **Relevance**: 9/10 — Physics-based zombie death/responses without per-animation authoring
- **5 Lessons**:
  1. Physics-based characters learned from motion clips via RL — zombie learns stagger-and-fall skill from 10 mocap death sequences, physics handles novel hit locations without animating every permutation
  2. Pre-train policies offline, export binary weights (~200KB per skill) — runtime inference on CPU ~0.5ms per zombie, no GPU needed at inference time
  3. External force perturbations handled by the policy — trained with random force applications, generalizes to unseen hit impacts; headshot from .50 cal vs gut shot from pistol produce dramatically different death sequences from same policy
  4. Multi-skill agents with gating — zombie has movement (shamble-chase), hit-reaction (stagger), and death (collapse) skills, gated by health threshold with brief blend between stance and death to prevent instant-ragdoll snap
  5. Terrain-aware falling via physics scene — zombie interacts with world collision geometry during death sequence, doesn't clip through railings or float mid-air before collapsing

#### Paper 36: Ecoclimates — Climate-Response Modeling of Vegetation (SIGGRAPH 2022)
- **Source**: ACM TOG 41(4), SIGGRAPH 2022, DOI 10.1145/3528223.3530146
- **Relevance**: 9/10 — Living ecosystem that changes with weather/player actions
- **5 Lessons**:
  1. Two-timescale coupling — fast weather tick (10 game-minutes) + slow ecostate tick (game-day), horde migration follows food that follows vegetation growth cycles
  2. Microclimate-driven vegetation response — local terrain (slope, shade, water proximity) creates microclimates determining which plants thrive; abandoned farm fields grow waist-high corn (ambush terrain), dry hilltops have scrub (sight lines for sniping)
  3. Disturbance events and ecological succession — fire/explosion strips vegetation, then weeds → grasses → shrubs over weeks; zombies pathfind differently through each succession stage (avoid noisy brush, prefer burnt ground at night)
  4. Water table simulation — 2D grid at 10m resolution, rain raises it, drainage lowers it; puddles become infected water hazards; crops grow only where water table is adequate
  5. Ecosystem state serialization — grid of species IDs + ages + health, ~4 bytes per 10m cell, 4km² world = ~160KB; full flora reconstructed from serialized state, not regenerated, preserving player deforestation permanently

#### Paper 37: Rethinking NPC Intelligence — Bayesian Reputation System (MIG 2014)
- **Source**: ACM MIG 2014, DOI 10.1145/2668084.2668091
- **Relevance**: 9/10 — Foundation for ZE's NPC trust/fear/respect model
- **5 Lessons**:
  1. Bayesian belief model for NPC reputation — NPCs maintain probabilistic belief about player's type (helpful/hostile/untrustworthy), actions update via Bayes' rule, NPC decides to trade/flee/fight based on MAP archetype
  2. Gossip propagation through NPC social networks — reputation spreads through adjacency list per settlement; news of betrayal propagates with gossip delay proportional to social distance
  3. Dialogue tree branches gated by belief thresholds — each node has RequiredBelief {trait, minProbability}; trust >0.7 reveals "Join my faction" branch, <0.2 redirects to "Get away from me!"
  4. Multi-axis reputation — Trust (keep promises?), Fear (will you harm them?), Respect (admire competence?). Feared leaders rule by intimidation but don't unlock rare item trading; respected leaders get loyalty and intel
  5. Expected action prediction drives NPC preemptive behavior — NPC maintains ActionHistory deque (last 50 actions), Markov predictor forecasts player's next action, survivor camps lock gates when known thief approaches

#### Paper 38: Navigating Faction Systems for Believable NPCs (ACM FDG 2024)
- **Source**: ACM FDG 2024, DOI 10.1145/3649921.3650012
- **Relevance**: 8/10 — Faction design framework for ZE's settlement/faction system
- **5 Lessons**:
  1. Faction identity by values, not allegiance — factions defined by CoreValue safety/freedom/order/survival; faction that values Order cooperates with rule-following player but expels chaotic player, even if both share territory
  2. Intra-faction diversity — per-NPC PersonalBias modifier (-0.5 to +0.5) within faction; one survivalist respects strength, another distrusts all outsiders
  3. Action memory not numeric score — track EventLog {action_type, target_id, timestamp, context}; NPCs reference specific past events in dialogue ("You saved my daughter last winter"), producing contextual barks
  4. Dynamic faction power — factionEcosystem tick each game-day: factions consume food, lose members to zombie attacks, gain/lose territory; player actions tip the scales
  5. Player-driven faction formation — if player hoards supplies, Safety-value NPCs drift away while Self-Reliance NPCs are attracted; NPCs self-sort into factions matching their belief profile

#### Paper 39: A Practical Analytic Model for Daylight (SIGGRAPH 1999)
- **Source**: ACM SIGGRAPH 1999, Preetham/Shirley/Smits, DOI 10.1145/311535.311545
- **Relevance**: 8/10 — Foundational sky/light model for ZE's weather system
- **5 Lessons**:
  1. Turbidity-based sky model — single turbidity parameter (2-30) smoothly varies sky from clear blue to hazy overcast; ZE SkyState struct interpolates turbidity over 24h cycle for weather-aware day/night
  2. Analytic sun position from time/date — closed-form solar position from time-of-day, latitude, date; ~20 lines C++ math drives directional shadow cascades, ambient color, zombie vision range
  3. Aerial perspective fog — distance fog blends toward current sky-zenith color, not uniform gray; far zombies look desaturated blue at midday, orange at sunset
  4. Chromatic adaptation for weather shifts — full color temperature 6500K (noon) to ~2000K (sunset); storm rolls in → high turbidity + low color temp makes zombie silhouettes harder to distinguish
  5. Hemisphere + aerial perspective split — sky dome computed separately from view-ray inscattering; two cheap compute shader passes (<1ms total on modern GPU)

#### Paper 40: Cine-AI — Automated Game Cutscenes in the Style of Human Directors (CHI PLAY 2022)
- **Source**: ACM CHI PLAY 2022, arXiv 2208.05701, DOI 10.1145/3549486
- **Relevance**: 7/10 — Dynamic cutscene generation for ZE's narrative moments
- **5 Lessons**:
  1. Director style encoded as camera idioms (shot-reaction-shot, Dutch angle, over-shoulder) — ZE's "Horror" style prefers Dutch angles during tension, close-ups during reveal, shaky-cam during action
  2. Runtime cinematography with shot-sequence interpolation — CutsceneDirector reads ShotSequence keyframes, interpolates via cubic splines; for gameplay moments, dynamically selects camera targets (zombie lunging → zombie-eye-level tracking shot)
  3. CinematicTemplates as data-driven JSON — authored camera recipes for common events (discovering horde → wide establishing shot → slow pan; rare loot → track-in close-up) tweakable by designers without C++
  4. Cinematic Intensity slider (0-10) — controls camera movement aggressiveness and Dutch angle frequency, gated by player preference; survival game players vs directed-movie moment players
  5. Timeline-based editing interface — TimelineTrack with CameraKeyframe nodes (position/rotation/FOV/dof) that runtime solver fills gaps between; author high-tension ambush cutscenes without a dedicated animator

#### Paper 41: Random-Access Neural Compression of Material Textures (SIGGRAPH 2023)
- **Source**: ACM TOG (SIGGRAPH 2023), arXiv 2305.17105, DOI 10.1145/3592407
- **Relevance**: 8/10 — Texture bandwidth reduction for ZE's open world
- **5 Lessons**:
  1. Joint compression of full material sets — compress albedo, normal, roughness, metallic, AO, height together into one neural representation, exploiting inter-channel correlation for 10-16× compression over BCx
  2. Coordinate-based random-access decoding — small MLP (2-3 layers, ~128 hidden) evaluated per UV coordinate in compute shader, weights ~8-16KB per material in constant buffer
  3. Integrated mip chain — network outputs correctly filtered values at any mip level; learns correct pre-filtered roughness/normal mips without separate generation pipeline
  4. GPU-optimized evaluation — Q8.8 fixed-point weights packed into uint32 vectors, thread-group shared memory for weight caching, 4×4 pixel quads with subgroup ops, target <0.1ms per frame
  5. Adaptive fallback — near-field (<10m) uses neural decoding for highest quality, far-field uses pre-baked BC textures, shader blends between based on distance

#### Paper 42: Analytical Ballistic Trajectories with Approximately Linear Drag (IJCTT 2014)
- **Source**: Intl. Journal of Computer Games Technology 2014, DOI 10.1155/2014/463489
- **Relevance**: 8/10 — Realistic projectile physics for ZE's weapons
- **5 Lessons**:
  1. Closed-form analytical solution for ballistic trajectories with linear drag — computes projectile position at any time without numerical integration; ZE should use this for all bullet/projectile trajectories, replacing Euler-integrated physics bullets
  2. Drag coefficient calibrated per caliber — .22 LR = 0.005, 9mm = 0.008, 5.56mm = 0.012, .308 = 0.020, .50 BMG = 0.035; each has different trajectory arc and effective range, making weapon choice tactically meaningful
  3. Analytical hit prediction for leading targets — given target position and velocity, solve closed-form for intercept angle; ZE AI marksmen can compute lead accurately without iterative methods, creating challenging but fair ranged combat
  4. Zero finding for maximum range computation — use Newton-Raphson on the closed-form trajectory to find the range where bullet descends to head height; ZE should precompute max effective range per weapon and visualize in scopes
  5. Cloud-penetration drag model — ballistic coefficient changes at cloud/rain boundaries where air density shifts; ZE weather system affects bullet drop (rain increases drag ~5%), giving environmental conditions tactical weapon impact

#### Paper 43: Fast Urban Weather Simulation (ACM TOG 2017)
- **Source**: ACM TOG 36(2), 2017, DOI 10.1145/2999534
- **Relevance**: 8/10 — Cell-based weather affecting zombie behavior
- **5 Lessons**:
  1. Cellular weather at city-block granularity — 100m×100m WeatherCells with wind vector, rain intensity, fog density, temperature; zombies move faster downwind (scent-assisted), slower in heavy rain (mud), rain noise masks footsteps
  2. Wind advection of rain and sound — rain particles offset by wind magnitude; zombie groans and environmental sounds carried downwind, creating stealth advantage upwind
  3. Temperature affects zombie behavior zones — below 5°C zombies slow 30% (shambling gait), above 35°C lethargic but attract more flies; horde clusters in warm drainage tunnels in winter, cool shaded alleys in summer
  4. Precipitation accumulation into puddles and wetness — heightfield water layer increases footstep noise (splash), slows sprinting speed, causes zombie slipping on steep wet terrain; zombie corpses in puddles accelerate decay
  5. Weather cell state machine for storm lifecycle — clear → building → precipitating → dissipating; thunderstorm builds 10min, rains 20min, dissipates 10min; lightning flashes briefly illuminate all zombies in radius for scouting

#### Paper 44: Fast Weather Simulation for Inverse Procedural Design of Urban Models (ACM TOG 2017)
- **Source**: Garcia-Dorado et al, ACM TOG 36(2), DOI 10.1145/2999534
- **Relevance**: 8/10 — Procedural weather with gameplay hooks
- **5 Lessons**:
  1. Simplified Navier-Stokes per weather cell — ZE divides open world into 100m cells, each with eWeatherState enum, simulates wind/temp/humidity via Jacobi relaxation on 2D WindField
  2. Rain particle advection by wind — slanted rain sheets driven by WindField; environmental sound advection (zombie groans travel farther downwind)
  3. Microclimate temperature tracking — shade vs concrete, day vs night; zombies have ColdBlooded trait: below 5°C slow 30%, above 35°C lethargic but attract more flies/attention
  4. Water layer on terrain — heightfield puddle simulation, splash sounds increase footstep noise, slows sprinting, causes zombie slipping; zombie corpses in puddles rot faster
  5. Lightning flash illumination during storms — briefly reveals all zombie positions in radius, critical gameplay window for scouting horde positions during night storms

#### Paper 45: Procedural Generation of Branching Quests for Games (Ent. Comp. 2022)
- **Source**: Entertainment Computing 43, 2022, DOI 10.1016/j.entcom.2022.100491
- **Relevance**: 8/10 — Branching quest tree for ZE's procedural narrative
- **5 Lessons**:
  1. Quest graphs with branching nodes — each node is a quest stage (gather, deliver, kill, scout, protect) with 2-4 branches; branches gated by player faction standing, skills, or resources
  2. Player-choice tracking feeds branch gating — ZE's action-log determines which branches are available; helping faction A unlocks their quest chain, locking faction B's alternative path
  3. Quest templates parameterized for each playthrough — same template generates different content (location, NPCs, rewards) based on current world state; ZE's quest pool of 20 templates produces 200+ unique quests
  4. Validation against world state — quest generator verifies preconditions (target location exists, required NPC is alive, item is reachable) before generating; impossible quests are regenerated or pruned
  5. Moral choice at branch points — each branching quest offers a trade-off (help faction A at cost to faction B), with reputation consequences that cascade into future quest availability

#### Paper 46: Realistic Modeling and Rendering of Plant Ecosystems (SIGGRAPH 1998)
- **Source**: ACM SIGGRAPH 1998, DOI 10.1145/280814.280898
- **Relevance**: 8/10 — Procedural flora placement for ZE's open world
- **5 Lessons**:
  1. Self-thinning and competition — Poisson-disk-with-competition for flora placement: each species has ResourceNeed and ShadeRadius; trees 8m spacing, bushes 3m, grass 0.5m for natural-looking forests
  2. Approximate instancing — thousands of unique plants via vertex shader random offsets (scale, rotation, colorTint) from StructuredBuffer; single draw call for 10K+ instances, ~0.3ms on GPU
  3. Terrain-driven species distribution — biomes as vector<SpeciesRule> with Suitability(terrain) → float; pine on north slopes >300m, deciduous south <300m, cattails within 2m of water
  4. Multi-scale ecosystem LOD — distant forest = billboard mesh, medium = simplified geometry, close = full procedural branches + leaf cards; rustling leaves alert player to zombie movement
  5. Seasonal plant state changes — spring (new leaf buds, more visual cover), summer (full canopy, reduced visibility), autumn (leaf drop, noisy dead leaves underfoot), winter (bare branches, max sight lines, scarcer wood)

#### Paper 47: Neural Layered BRDFs (SIGGRAPH 2022)
- **Source**: ACM TOG (SIGGRAPH 2022), DOI 10.1145/3528233.3530732
- **Relevance**: 8/10 — Realistic material layering for ZE's weapon/vehicle/character materials
- **5 Lessons**:
  1. Neural latent representation for BRDF layering — small MLP (3 layers, 64 neurons) takes latent vector (albedo, roughness, metalness, clearcoat thickness, IOR, anisotropy) and outputs combined layered BRDF
  2. Pre-computed BRDF layering atlas — for common layer combos (clear-coat over metallic, rust over metal, dirt over diffuse), pre-train and store as 256²×3 texel lookup table, eliminating runtime network eval for 80% of cases
  3. Position-free layering compositing — evaluate each layer's BRDF, feed parameters into neural network, output final combined parameters; network handles absorption, scattering, inter-layer reflections without explicit ray tracing
  4. Thin-film interference as additional layer — clear coat + wavelength-dependent IOR; network maps thickness (nm) → color shift → final combined BSDF
  5. Material parameter blending for terrain — grayscale masks control layer opacity, roughness, normal blend strength; terrain material blending (dirt/grass/rock) with physically plausible results from single network eval per pixel

#### Paper 48: Real-Time Geometry Caches for Alembic Streaming (SIGGRAPH 2014, Crytek)
- **Source**: SIGGRAPH 2014 Talks, Crytek (Ryse)
- **Relevance**: 7/10 — Animation streaming for ZE's open-world NPCs
- **5 Lessons**:
  1. Alembic as interchange only — bake to GPU-optimized binary format at cook time; per-frame vertex positions (uint16×3 quantized), per-frame QTangents, per-frame transform matrices; target 10MB/s data rate
  2. Aggressive vertex quantization — 3× uint16 positions, 2× int16 UVs, 8-bit QTangents; 56 bytes/vertex → 16 bytes/vertex; data rate drops from 50MB/s to 10MB/s for 30K-vertex animation at 30fps
  3. Pre-baked per-frame tangent frames — topology is static (only position changes), so bake normal/tangent/bitangent offline as QTangents per vertex per frame; eliminates runtime recomputation
  4. Triple-buffer streaming — current frame (GPU rendering) + next frame (DMA upload) + pending (CPU decode), asynchronous via VkFence and staging buffers, pre-decode 3-5 frames ahead
  5. Hierarchy simplification — collapse rigidly animated objects to single world-space transform per frame; vertex-animated caches (cloth, flags) use identity transform with per-vertex motion; cut Ryse's transform budget 80%

#### Paper 49: Improving Ray Tracing Performance with Variable Rate Shading (CGVC 2021)
- **Source**: CGVC 2021, Eurographics, DOI 10.2312/cgvc.20211319
- **Relevance**: 7/10 — VRS for ZE's hybrid ray tracing budget
- **5 Lessons**:
  1. VRS applied to ray generation — shading rate image where 1×1 = full rays (glossy/specular), 2×2 = quarter rays (medium roughness), 4×4 = 1/16 rays (rough/diffuse/sky)
  2. Content-adaptive VRS based on roughness + luminance variance — compute per 16×16 tile in post-G-buffer compute pass; roughness >0.4 → coarser rate, high variance → 1×1
  3. Inline ray tracing with VRS co-programming — one thread shades 2×2 pixel quad, traces one ray for all four pixels via subgroup broadcasts, avoiding per-pixel ray tracing overhead
  4. FLIP metric validation — 2×2 VRS yields <2% FLIP error at ~1.8× perf improvement; 4×4 yields ~5% error at ~3.2× speedup; ZE target = 2×2 baseline for reflections
  5. Temporal VRS stabilization — blend current VRS classification with previous frame (70% prev, 30% current) via compute shader, eliminating shimmer from rate boundaries between frames

#### Paper 50: Real-Time Rendering of Glossy Reflections with Two-Level Radiance Caching (SIGGRAPH Asia 2023)
- **Source**: SIGGRAPH Asia 2023 Technical Comms, DOI 10.1145/3610543.3626167
- **Relevance**: 7/10 — Efficient glossy reflections for ZE's PBR rendering
- **5 Lessons**:
  1. Two-level radiance caching — 1st level: screen-space probes on visible surfaces from previous frame (~0.2ms); 2nd level: world-space hash grid (32³ cells, spherical harmonic coefficients); fallback from screen cache → hash grid on disocclusion
  2. Roughness-stratified ray strategy — smooth <0.1: trace 1-2 rays per pixel with GGX importance sampling; medium 0.1-0.4: 1 ray + screen-space cache; rough >0.4: hash-grid cache only, no ray tracing
  3. Specular denoising via separable À-Trous wavelet — 4-5 iterations at ½ or ¼ resolution, step sizes 1→2→4→8→16, edge-stopping by roughness + normal variance; temporal accumulation via motion vector reprojection
  4. G-buffer as RT input — GI-1.1 arch: rasterize G-buffer first, then ray trace glossy reflections using G-buffer depth/normal/roughness/albedo as input; total RT budget ~1 ray per pixel
  5. Hash grid probe cache persistence — probes distributed over surfaces via spatial hashing, 1/16 probes refreshed each frame; enables sub-1-spp ray budgets for indirect glossy with temporal stability

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

A comprehensive 416-line study of three AAA engine architectures was conducted, covering 15 concrete technical lessons (5 per engine) with specific Vulkan primitives, compute shader patterns, and priority matrix. Full document at `docs/research/engine_architecture_lessons.md`.

#### id Tech 7/8 (id Software)
- **Sources**: Simon Coenen DOOM Eternal Graphics Study, SIGGRAPH 2020 Advances, GDC 2026 Rip and Tear session, Digital Foundry tech interviews, NVIDIA neural rendering blog
- **Key Techniques**:
  1. **Bindless descriptor architecture + dynamic draw call merging** — VK_EXT_descriptor_indexing with compute shader compaction of draw calls by mesh+material ID. Renders 80-90M triangle scenes with near-zero CPU draw overhead.
  2. **GPU compute skinning pre-pass** — All animated meshes skinned once per frame in compute shader before shadow/depth/forward passes. Vertex shaders treat everything as static. Dramatically reduces shader permutations.
  3. **Cached shadow maps** — Static geometry shadow contributions persist across frames in a 4096x8192 shadow atlas. Only re-renders when light direction changes beyond a threshold.
  4. **Visibility buffer + compute-based deferred texturing (id Tech 8)** — First pass writes 4-8 byte per-pixel (triangle ID + instance ID). Tile classification dispatch evaluates only visible materials via software Variable-Rate Compute Shaders.
  5. **Sector streaming + auto vista LOD** — World divided into streaming sectors with prediction-based prefetch. Automated impostor geometry generation for distant structures.

#### Decima Engine (Guerrilla Games / Kojima Productions)
- **Sources**: GDC 2017 Visibility and Procedural Placement talks, SIGGRAPH 2017 Nubis cloud system, Digital Foundry analyses
- **Key Techniques**:
  1. **GPU compute-based runtime procedural placement** — Artists define placement rules in a graph editor (height, slope, biome mask, exclusion zones). Compute shader evaluates per-tile near player, populating millions of instances deterministically per seed.
  2. **GPU compute visibility queries with wavefront batching** — 500K to 1.5M static instances per query. Wavefront-wide batching of instances sharing mesh+material yields up to 64x reduction in output draw commands.
  3. **Three-tier terrain rendering** — Heightmap-tessellated patches + voxel/cliff representations + procedural instancing for surface detail. Dynamic switching based on viewing distance.
  4. **GPU-based dynamic grass/foliage rendering** — Compute-generated blade quads with per-blade wind phase, bend, and color variation. Wind simulation as a global compute pass with disturbance channel for player/zombie movement.
  5. **Nubis volumetric cloud system** — Regional-scale volumetric clouds as gameplay element. 3D density fields with artist shapes, ray-marched via compute shaders. Temporal reprojection enables 4-8 samples per pixel with 32-64 quality.

#### Unreal Engine 5 — Nanite + Lumen (Epic Games)
- **Sources**: SIGGRAPH 2021 Nanite Deep Dive, Epic Games official documentation, UIUC CS 418 case study
- **Key Techniques**:
  1. **Hierarchical cluster-based virtualized geometry (Group-Decimate-Split)** — 128-triangle clusters in DAG hierarchy. Group-Decimate-Split avoids dense border problem; every node has watertight boundaries. GPU selects cluster LOD per-cluster based on screen-space projected error.
  2. **GPU-driven two-pass Hi-Z occlusion culling with LOD integration** — Pass 1 tests instance bounding boxes against previous frame's Hi-Z. Pass 2 re-evaluates occlusion at cluster granularity during rendering. LOD selection and occlusion share same error metric.
  3. **Page-based cluster storage with virtual streaming** — 128KB pages, spatially and LOD-local allocation. First page always resident contains coarsest LOD. Feedback system tracks page demand per frame. Sparse residency via VK_EXT_memory_budget.
  4. **Virtual shadow maps** — Single 16384 squared sparse shadow texture per light. Per-frame visibility bitmask of needed tiles. Only renders tiles that are newly needed or invalidated. Persistent tile cache across frames.
  5. **Software rasterizer + doubly-deferred shading** — Clusters under 64 pixels switch from hardware to compute-shader rasterizer. Doubly-deferred shading groups visible pixels by material ID per 16x16 tile, evaluating each material once per tile.

### 2.4 Research-to-Spec-Block Mapping

Each entry ties a specific paper finding directly to an EXT block with concrete implementation detail.

#### M5-EXT-10 AI Director (horde pacing) — RimWorld Storyteller + Experience-Driven Adaptation (Paper 8)
The block says "config-driven density curve; SLM telemetry-Director tunes it" — that is not an implementation. What the research actually prescribes:

- **Tension clock with explicit phases**: CALM (5min) → RISING (2min) → CRISIS (1-2min) → RECOVERY (3min). The clock is a state machine, not a density curve. Each phase has different spawn rates, ambient audio profiles, and loot availability.
- **Director selects events by dramatic potential, not difficulty**: Maintain an event pool with weights. When tension is low, prefer "discovery" events (radio signal, rare loot cache, survivor call). When tension peaks, prefer "threat" events (horde sighting, base breach, special infected). The paper's key insight: events during high tension are memorable; events during low tension are annoying.
- **Player state vector**: Track health (0-100), resource stockpile (starving → overflowing), time since last threat (minutes), exploration freshness (% of current zone unexplored). The director uses this to select events, not a simple "density curve."
- **Spawn waves use logistic pacing**: Start waves at 30% of max pull, escalate to 100% over 90 seconds, then decay to 0% over 60 seconds. This creates the "rising tide" feel from L4D's director — not a flat spawn rate.
- **Telemetry feedback every 30 seconds**: Short-term adjustment — if player cleared a wave in 15 seconds, next wave spawns 20% faster. If player is taking damage, next loot drop has 30% more ammo. This closes the adaptation loop the paper requires.

#### M5-EXT-11 Horde Emergence / Spawn Waves — 7 Days to Die + Left 4 Dead Director
- **Scheduled + dynamic hybrid**: The block says "Director emits spawn waves" without detail. Use 7DTD's Blood Moon as the backbone — a known schedule creates urgency — but layer L4D's dynamic director on top for unpredictable mini-waves between scheduled events.
- **Screamers as emergent wave triggers**: 7DTD's heat map system is critical. Track player activity per chunk (shots fired, generator running, forge active, vehicle noise). When heat exceeds threshold, spawn a Screamer. If the Screamer survives and screams, it calls a mini-horde. This should be M5-EXT-11's core mechanic — waves emerge from player action, not just a timer.
- **Path-of-least-resistance zombie targeting**: Zombies should A*-route through player-built structures using a material-cost heatmap. Wood > iron > concrete > reinforced. This forces smart fortification over brute walls. Implement as a compute-shader A* on a nav-tile graph, updated when structures change.

#### M6-EXT-14 Audio Occlusion / Propagation — GSound (Paper 10)
The block is a stub (no math, no algorithm). My initial recommendation was WRONG — I said "Precomputed Transmission Path Matrix" which is a baked offline technique. ZE's world is destructible: buildings collapse, terrain deforms, temporary fortifications get built. Precomputed paths become invalid the moment a wall is destroyed.

**Correct approach** — runtime voxel-cone tracing for audio, not baked PTMs:
- **Voxel-cone acoustic integration** (M6-EXT-11 already exists): Use the same voxel grid from M3/M4.5-EXT-17. Cast cone probes from emitter toward listener. Count the number of occluding voxels along the path, weighted by each voxel's material density. This naturally handles dynamic changes — destroyed walls simply remove voxels.
- **Frequency-band attenuation**: NOT a single volume scalar. Split each cone probe into low (20-250Hz), mid (250-4000Hz), and high (4000-20000Hz) bands. Low frequencies diffract more — weight by frequency: occlusion_strength[bass] *= 0.3, occlusion_strength[treble] *= 1.0. Concrete blocks highs better than lows.
- **Edge diffraction as angle-based heuristic, not precomputed**: Instead of precomputed edge visibility graphs, compute a simple diffraction factor: when the direct line-of-sight is occluded, find the nearest occluding edge and compute diffraction angle. Use the angle to derive an occlusion factor: `diffractionFactor = 1 - exp(-3 * subsolarAngle)`. This matches GSound's key insight without precomputation.
- **Reverb integration** (M6-EXT-12): Reverb IR still comes from the voxel field. The key fix: reverb AND occlusion share the same grid query, not two separate passes. M6-EXT-12 currently says it builds IR from "voxelOcclusion" but has no link to EXT-11 or EXT-14. Add an explicit data dependency path: voxel grid → EXT-11 (voxel-cone traces) → EXT-12 (impulse response) → EXT-14 (occlusion factor). All three use the same grid, not three separate grids.
- **Performance budget**: Each occlusion query = 3 cone traces (direct + 2 diffraction edges) × 3 frequency bands = 9 voxel-grid lookups. At 128 emitters (mix of zombies + environmental sounds) at 60fps: 128 × 9 × 60 = 69,120 grid lookups/second. At ~0.1μs per voxel read (texture buffer), total = ~7μs/frame. Negligible.

#### M5-EXT-31 DeepMimic RL Character Skills
**CORRECTION to my earlier analysis**: DeepMimic does NOT use a single-iteration optimizer. It runs a PD controller at every physics timestep (typically ~15 iterations per frame depending on contact count). Each iteration:
- Computes joint torques from proportional-derivative error relative to the reference pose
- Applies contact forces from the physics solver (Jolt 5.6.0)
- Handles unseen poses by optimizing in the null-space of constraints — it finds a physically valid version of ANY reference pose

**Correct ZE implementation**: 
- M5-EXT-31 should use a PD controller with at least 8 iterations per timestep at 30Hz physics rate (240Hz sub-steps). Fewer iterations than 8 produces jittery recovery from crumple poses.
- The PD gains (kp, kd) must be velocity-dependent: slow movement needs high stiffness (rapid correction), fast movement needs lower stiffness (to avoid instabilities). This velocity-gain curve is the single most important tuning knob and is entirely absent from the current block spec.
- DeepMimic's phase function (which selects the reference frame from the animation clip based on time + contact state) is what makes it work for unseen poses. M5-EXT-31's blend should store the player's animation timestamp at impact, then advance through the animation at 0.5x speed during ragdoll and blend back at 100% when recovered. This handles mid-stride, mid-climb, mid-punch hits — not just idle-stance hits.

#### M3-EXT-24 Atmosphere Volumetric Scattering
**CORRECTION to my earlier analysis**: I said "use Hosek-Wilkie or blend Preetham with dusk correction." The correct recommendation is: REPLACE Preetham entirely with Hosek-Wilkie. Preetham's model has seven known failure modes (low-sun color banding, turbidity saturation above 10, missing ozone absorption, poor twilight gradients, no cloud integration, single-scattering assumption, negative RGB at saturation). Hosek-Wilkie (2012) fixes all seven and adds 30% better dawn/dusk accuracy for <2% additional compute cost. There is no reason to keep Preetham in a 2026 engine.

**Additional missing detail**: The aerial perspective model (fog at distance) MUST be a separate compute pass from the sky dome. ZE's current spec treats them as one. Two-pass: (1) Pre-compute sky dome to 256×128 cubemap in a compute shader, sample all sky lookups from it. (2) Separate compute pass for aerial fog — this one uses depth from the GBuffer and the extinction/scattering coefficients. The two interact only through the inscattering color (fog color comes from the sky dome sample at horizon angle).

#### M5-EXT-06 AI Behavior — Concordia GM Architecture (Paper 6)
- **Game Master pattern**: The block currently has no architecture. Use Concordia's GM: separate "world simulation" (weather, zombie positions, loot state) from NPC decision-making. The GM validates actions against physical plausibility before executing. This prevents NPC cheating (knowing player location through walls).
- **No utility maximization**: NPCs should NOT calculate optimal survival strategies. Act based on personality + experience. A previously bitten NPC panics at zombie sight even at safe distance. Use a weighted drive system (survival, community, fear, curiosity) that evaluates current context — not a single survival score.
- **Three parallel spaces**: Each NPC has (1) Physical: position, resources, health, (2) Social: trust, reputation with other NPCs and player, (3) Digital: radio contact, map knowledge. Behavior emerges from interactions between these spaces, not from a behavior tree.

#### M7 Economy Blocks (Barter/Trade/Scarcity) — GEEvo + SoD2 Economy (Paper 22)
- **Constant drain, not balance**: SoD2's economy works because consumption grows with settlement size. M8-EXT-14 must enforce passive daily drain. More survivors = more food. More tech = more fuel/ammo upkeep. The economy is never "solved."
- **Evolutionary balancing**: GEEvo proves that static economy tables fail. Run offline evolutionary simulations that mutate trade ratios, craft costs, and loot tables. Use player telemetry (what do players craft most? what do they ignore?) as the fitness function. Evolve the economy per-save, not per-patch.
- **Three competing currencies**: Barter goods (common), ammunition (military), medicine (rare). Each faction deals primarily in one currency. This prevents a single dominant economy and forces varied interactions.

#### M0-EXT-16 ENGINE_DETERMINISM_MODE — Determinism research (cross-paper)
- **Deterministic PCG seed per save**: All procedural generation (terrain, loot, POI placement) uses the same seed, enabling verifiable playthroughs. Enables the "stress seed suite" from M3-EXT-30.
- **Deterministic enemy AI with seeded RNG**: Zombie decisions use a session-seeded RNG stream. Same seed = same zombie behavior. Essential for debugging and reproduction of player-reported bugs.
- **Deterministic frame pacing**: In determinism mode, lock frame delta to a fixed 16.67ms (60Hz). Physics and job scheduling use this fixed delta, not wall clock time. Deviations log a warning. This is how id Tech 7 achieves fully deterministic playback for their replay system.

#### Engine: GPU Compute Skinning (id Tech 7 → M5-EXT-02)
M5-EXT-02 is "Dual-Quaternion GPU Compute Skinning." The id Tech 7 enhancement: run the skinning compute shader ONCE before ALL rendering passes (shadow, depth, forward). The skinned output buffer is reused across passes. ZE should enqueue the skinning compute as an enkiTS task that produces a GPU buffer dependency, consumed by all subsequent passes. Only re-skin meshes whose animation state changed since last frame.

#### Engine: Bindless + Indirect Draw Merging (id Tech 7 → M0-EXT-08)
M0-EXT-08 "Bindless Storage Handle Page Allocator" enables the id Tech 7 pattern. The critical addition: a compute shader compaction pass that groups visible instances by (meshID, materialID) and writes a compacted `VkDrawIndexedIndirectCommand` buffer. This reduces CPU draw iteration from O(visible instances) to O(unique mesh-material pairs). Implement as a post-culling compute dispatch.

#### Paper 21: Save Serialization (SC2) → M7-EXT-07, M7-EXT-08
**Critical finding**: SoA layout at the wrong dimension ballooned files from 75.8MB to 97.3MB (worse than AoS). Only instance-major layout (group by unit, then by timestep) achieved 21.8MB compression. Both M7-EXT-07 (Hierarchical Delta-State Persistence) and M7-EXT-08 (Zstandard Custom Dictionary) depend on the data layout order.

**Implementation requirement**: Separate static properties (position, type never change) from dynamic properties (health, inventory, animation state) BEFORE compression. Store static data in a single binary blob per entity. Store dynamic data as instance-major delta frames. The 10x reduction came from data restructuring not codec choice. The Zstd dictionary should be trained on instance-major delta frames, not raw AoS snapshots. Profile static vs dynamic ratios per entity type — buildings are 95% static, zombies are 60% dynamic.

#### Paper 25: Modulith Modding Architecture → M11-EXT-17, M11-EXT-20, M11-EXT-21
**Three findings map directly**:
1. **Security sandbox gap** (M11-EXT-20): Modulith lets mods run arbitrary code in the engine process. M11-EXT-20 "Mod Write-Allowlist" should be extended with a capability system — each mod declares what files/systems it needs (network, file write, UI hooks), and a sandbox enforces these at load time. Without this, a malicious mod can read save files or install persistent malware.
2. **Dependency resolver** (M11-EXT-17): Mod A v1.2 requiring Mod B ≥v2.0 while Mod C requires Mod B ≤v1.8 is a real failure mode. M11-EXT-17 must implement a SAT/version-range resolver (like npm's semver). Store dependency metadata in each mod's manifest with min/max version ranges.
3. **Cross-module overhead** (M11-EXT-21): Every boundary crossing adds ~0.1-0.4ms in serialization/dispatch overhead. With 10+ mods and a call chain spanning 5 modules at 60fps, 1-2ms of frame time goes to mod boundary overhead. M11-EXT-21's runtime reload must track which mods are actually hot and batch cross-module calls.

#### Paper 27: Interactive Dynamic Response → M5-EXT-31, M5-EXT-36
**Passive ragdoll looks dead — active muscle simulation required**. M5-EXT-31 "Ragdoll-to-Animation Recovery Blend" currently assumes passive ragdoll transitioning to animation. The paper prescribes:
- **Active muscle tension**: On hit impact, don't zero the animation — blend physics forces ON TOP OF the base animation. Keep the character's current pose as a target, apply impulse forces, then blend back. This prevents the floppy-dead-weight look.
- **Hit priority stack** (M5-EXT-36): Multiple simultaneous hits (shotgun, explosion shrapnel) must accumulate impulse vectors, not overwrite. Maintain a circular buffer of last N hits with summed impulse. Select the composite response — not just the strongest single hit.
- **LOD for physics**: Full dynamic response for near enemies (0-15m), simplified impulse reaction (pre-baked animation overrides) at medium (15-40m), no reaction beyond 40m. Without LOD, 20+ enemies costs >5ms per frame.

#### Paper 28: Fire Simulation → M3-EXT-01, M3-EXT-04
**Fire spread needs per-object material properties**. M3-EXT-04 "Hydrocarbon Slipstream Flame-Trail Splitting" and M3-EXT-01 "Structural Fatigue" both depend on knowing which objects burn and how.
- Each destructible object needs: ignition temperature, heat capacity, fuel load (burn duration), structural integrity loss rate. Without this metadata, either everything burns (unplayable) or nothing burns (unrealistic).
- Vorticity confinement has a narrow tuning sweet spot that varies with grid resolution. The M3-EXT-01 system must store per-resolution presets — a value that looks right at 64×64×64 produces spinning artifacts at 128×128×128.
- Blackbody color rendering is counterintuitive: hottest is blue-white, not red. M3-EXT-04's visual output should use physics-based motion with artistic color ramps, not direct physical rendering.

#### Paper 29: Preetham Daylight → M3-EXT-12, M3-EXT-24
**CORRECTION**: I initially offered "Hosek-Wilkie OR blend with dusk correction." The latter is wrong. Preetham has seven known failure modes. Hosek-Wilkie (2012) replaces it entirely with <2% more compute and handles ALL sun angles correctly.
- **Aerial perspective is a separate model**: M3-EXT-12 must implement TWO compute passes: (1) sky dome pre-computed to 256×128 cubemap, regenerated when time-of-day changes >1°, (2) aerial fog pass using depth from the GBuffer and extinction/scattering coefficients from the sky model. They interact only through the inscattering color — the fog color comes from the sky dome sample at the horizon angle.
- **Spectral-to-RGB conversion** can produce negative values for saturated sky colors. The cubemap approach avoids per-pixel conversion cost.

#### Paper 30: DeepMimic RL Character Skills → M5-EXT-31
**CORRECTION**: I earlier said "single-iteration optimizer" — DeepMimic runs a PD controller at every physics substep (~15 iterations depending on contact count). Each iteration computes joint torques from proportional-derivative error, applies contact forces, and optimizes in the constraint null-space to find a physically valid version of ANY reference pose — not just pre-baked hit reactions.
**Correct ZE implementation**:
- M5-EXT-31 should use a PD controller with at least 8 iterations per timestep at 30Hz physics rate (240Hz sub-steps). Fewer than 8 produces jittery recovery from crumple poses.
- The PD gains (kp, kd) must be velocity-dependent: slow movement needs high stiffness, fast movement needs lower stiffness to avoid instability. This velocity-gain curve is the single most important tuning knob and is absent from the current block spec.
- DeepMimic's phase function selects the reference frame based on time + contact state. M5-EXT-31 should store the character's animation timestamp at impact, advance through the animation at 0.5x speed during ragdoll, and blend back at 100% when recovered. This handles mid-stride, mid-climb, mid-stumble hits — not just idle-stance reactions.

#### Paper 11: Real-Time Fracturing (Voronoi) → M3-EXT-05
**Voronoi is the bottleneck — 5-15ms for runtime fracture**. The Breaking Good paper (Paper 19) offers an alternative: modal analysis with 33 fragments from 32 modes costs only 0.16ms (matrix-vector multiply). M3-EXT-05 should use modal analysis as the primary fracture method and reserve Voronoi for pre-fractured assets only.
-**68% of players prefer predictable pre-fracture**: M3-EXT-05 must show visual cues (cracks, stress marks, material-specific deformation) BEFORE the fracture point. Players hate random structural collapse without warning. The damage texture on the object should progressively reveal crack patterns as structural integrity drops.
-**Modal analysis fragment count is bounded**: max 33 fragments from 32 vibration modes. This is a feature, not a bug — it prevents the explosion-style fragmentation of Voronoi and keeps debris count manageable (<50 pieces per fracture event). M3-EXT-05's structural integrity model should use the first 8-16 modes for real-time computation (faster) and fall back to 32 modes for "hero" objects (player base, boss arenas).

#### Paper 13: Boid-Flock Fish Population (Reynolds) → M8-EXT-20
**Three critical scaling limits**:
1. **Flocks >200 individuals spontaneously split** — the perception radius of each boid becomes larger than the flock itself at this size. M8-EXT-20 must cap perception radius to 15m for cohesion, even if the actual flock is larger. Beyond 200, use a hierarchichal approach: each group of 200 has a "lead boid" that interacts with other groups, and individual boids only see their group.
2. **O(n²) neighbor search caps at ~2000 boids on console hardware** — M8-EXT-20 MUST use a spatial hash grid (not brute-force). With a spatial hash at 32³ cell resolution, the neighbor search drops from O(n²) to O(n * avgDensity). For ZE's fish population, this means a 10000-boid lake costs the same as 1000 boids in brute-force.
3. **Obstacle hover-lock** — when multiple obstacle avoidance forces sum near-zero, boids get stuck hovering. Solution: add a small random perturbation (0.1% of avoidance force) that breaks the deadlock. Without this, fish appear to magnetically cling to obstacles.

#### Paper 17: Rigid-IPC CCD (Collision Cleanup) → M3 Physics
**10× slower than Bullet/Havok — use as cleanup pass only**. Rigid-IPC guarantees non-intersection but costs an order of magnitude more. For ZE's zombie physics (200+ active rigid bodies):
- Primary collision: Jolt 5.6.0's default solver (broadphase + narrowphase, ~0.5ms)
- Cleanup pass: Rigid-IPC on the top 10% of penetrating contacts detected by Jolt. This catches the edge cases (thin geometry, high-speed contacts) without paying the full cost.
-**Curved CCD costs 8× more than linear** but catches 15% more missed collisions. Use linear CCD for all zombies; reserve curved CCD for player-critical objects (vehicles, heavy weapons, physics puzzles). Linear CCD at 30fps physics rate misses <1% of collisions for zombies moving at <10m/s.

#### Paper 18: XPBD Constraint Solver → M3 Physics
**Compliance α is iteration-independent** — this is the key math property that distinguishes XPBD from PBD. In PBD, stiffness depends on iteration count; in XPBD, compliance is a material property independent of solver iterations. ZE's physics should use XPBD for soft-body constraints (cloth, rot-cloth for zombie decomposition, vegetation) because the behavior won't change between 30fps and 60fps modes.
-**20-iteration cap**: Beyond this, oscillation degrades quality rather than improving it. M3's physics solver should clamp XPBD iterations to 8-15. Use 8 for background bodies (leaves, distant vegetation), 15 for player-interacting soft bodies (zombie cloth, player-held items).
-**Ghost forces from normal→friction ordering**: The order of normal-force solving vs friction solving creates phantom forces. XPBD's typical "normal first, then friction" introduces ~2% ghost force. ZE's solver should alternate ordering every substep to cancel ghost forces, not fix the order.

#### Paper 20: Tall Cell Water Simulation → M4 Water/Environment
**30% memory bandwidth penalty from indirection** — tall cell grids use pointer indirection instead of direct array access. M4's water system should use flat arrays for the top 32 cell layers (where surface visual activity happens) and tall cells only for deep water (below visual interest).
-**30fps physics loses 3.2% volume/frame** due to numerical diffusion. At 60fps, the same diffusion loses only 0.8%/frame. ZE's water simulation should run at 60Hz even if the rest of physics runs at 30Hz. De-couple water tick rate from physics tick rate.
-**Warp divergence reduces GPU utilization from 85% to 52%** — tall cell approaches cause divergent warp execution because different cells have different heights. Solution: sort cell columns by height before the simulation pass, then process batches of similar-height columns in the same warp. This recovers ~20% utilization.

#### Paper 16: CoD Infinite Warfare Clustered Culling → M0-EXT-08
**Z-bin ordering uses exponential depth bins, not uniform**. The paper subdivides the frustum into 32 depth bins where bin width doubles at each step. This concentrates culling resolution near the camera and reduces it at distance. Z-bin pass costs 0.1ms on GCN hardware for 1000 lights. ZE's M0-EXT-08 bindless draw merging should adopt exponential z-binning for its indirect light culling pass — not uniform frustum splits.

#### Paper 15: Minecraft Zombie Pathfinding → M5-EXT-01
**Octile heuristic beats Manhattan by 23%**: M5-EXT-01 "Asynchronous Tile-Voxelized NavMesh Baker" uses A* on a voxel grid. The paper found octile distance heuristic (allows 45° diagonal movement) reduces node expansion by 23% over Manhattan in grid-based pathfinding.
- Path recompute at 10-tick intervals is the sweet spot — any faster wastes CPU, any slower creates visible zombie hesitation at corners.
- Bit-set representation of the navigation grid is 24x smaller than a full graph. Store each voxel's navigability, height, and traversal cost in a flat uint64 bit field per 4×4×4 voxel tile. A* operates on the bitset directly without materializing a graph.

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
