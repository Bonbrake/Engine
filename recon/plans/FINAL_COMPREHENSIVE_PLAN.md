# ZombieEngine — Definitive Program Plan
Revision: Final Verified
Scope: spec/M0..M13 plus referenced appendices; 50-paper research backbone; Tier-0 target RTX 2070 SUPER / 6GB VRAM / 30 FPS floor; 33.3 ms/frame budget.
Status: Planning phase. No spec files are edited by this document. All structural claims are backed by disk evidence or marked [X].

# 0. Program Charter

ZombieEngine is a modern-day C++20/Vulkan 1.4 survival engine built on Jolt 5.6, enkiTS, SDL3, Dear ImGui, and GameNetworkingSockets. The target is a professional product with the quality bar of an experienced solo-dev studio, not a prototype.

Current verified truth:
- 812 unique EXT IDs across 16 milestone files.
- M0 declared=50, index=50. M1 declared=52, index=52. M2 declared=127, index=127. M3 declared=34, index=34. M4 declared=91, index=91. M4.5 declared=35, index=35. M5 declared=90, index=90. M6 declared=22, index=22. M6.5 declared=13, index=13. M7 declared=23, index=23. M8 declared=80, index=80. M9 declared=35, index=35. M10 declared=27, index=27. M11 declared=56, index=56. M12 declared=35, index=35. M13 declared=42, index=42.
- 9 missing sub-milestone index sidecars.
- Multiple dangling cross-references in text.
- 50-paper research backbone partially sourced; many entries remain [X].
- Build status: main build green, 20 passed/1 skipped/83 assertions green; ASAN green; clang-tidy upstream wiring committed, local executable unverified [X].

Program goals:
- Eliminate all preventable research and structural gaps before coding begins.
- Create a deterministic pipeline from paper citation to implementation block.
- Achieve Tier-0 performance stability before the first vertical slice.
- Maintain strict code, verification, and tracking discipline from day one.
- Ship a product that competes with the best indie survival games through systems depth, polish, and emergent gameplay.

This document is revision-final until a structural change in `spec/` or `recon/plans/` invalidates a claim.

# 1. What Is Actually Wrong — Honest Gap Audit

These are the real gaps, not theoretical ones. They block implementation agents.

## 1.1 Structural Gaps

| ID | Gap | Severity | Blocking |
|----|-----|----------|----------|
| G-1 | M2 MD ceiling: JSON has IDs through M2-EXT-127, MD only through M2-EXT-99 | High | Yes — agent confusion |
| G-2 | M4/M4.5/M8/M10/M11/M12/M13 MD ceiling gaps | Medium | Yes — may hide missing blocks |
| G-3 | Missing sub-milestone indexes: M2.7, M2.8, M2.9, M4.6, M5.1, M5.2, M5.3, M5.4, M8.6 | High | Yes — cross-refs unresolved |
| G-4 | Dangling cross-references: M0-EXT-40, M11-EXT-42, M12-EXT-03/05/06, M13-EXT-14/54, M5.2 single-digit, M8.6/M8.7 high variants | High | Yes — broken links |
| G-5 | Earlier plan artifacts cite wrong totals (1,224, 691); true count is 812 | Medium | No — but must retire |

## 1.2 Research Gaps

| ID | Gap | Severity | Blocking |
|----|-----|----------|----------|
| R-1 | ~30 of 50 papers remain [X]; no cache-backed [S] quotes | High | Yes — implementation agents need [S] |
| R-2 | Paper-to-block mapping is milestone-level, not block-level | Medium | Yes — execution ambiguity |
| R-3 | Cross-game truths (7DTD, SoD2, PZ, DayZ, DL2, RimWorld) remain [X] | Medium | No — but weakens design pillars |

## 1.3 Execution Gaps

| ID | Gap | Severity | Blocking |
|----|-----|----------|----------|
| E-1 | No concrete audio pipeline plan despite M6/M6.5 being major milestones | High | Yes — audio is core survival feature |
| E-2 | No per-milestone acceptance criteria with measurable pass/fail | Medium | Yes — quality gates undefined |
| E-3 | No risk indicators with thresholds | Medium | No — but needed for proactive mitigation |
| E-4 | No timeline with dates, only phases | Low | No — but needed for execution |

# 2. Target Hardware and Performance Budgets

Tier-0 floor: RTX 2000-series / RX 6000-series / Arc A580 equivalent; 6 GB VRAM; 6-core/12-thread CPU.
Floor target: 30 FPS / 33.3 ms frame.
Preferred dev target: 60 FPS on RTX 2070 SUPER.

## 2.1 Frame Budget (Tier-0 Floor)

| Stage | Budget |
|-------|--------|
| Render | 13.0 ms |
| Physics | 5.0 ms |
| AI | 5.0 ms |
| Animation | 2.0 ms |
| Audio | 0.5 ms |
| Headroom | 5.3 ms |
| Total | 33.3 ms |

## 2.2 Memory Budgets

- VRAM baseline: 4.0 GB single-viewport, 3.3 GB split-screen on 6 GB floor.
- System RAM: 12 GB floor.
- Material/texture payload: RVT compression first, mip-LOD second, resolution fallback third.

## 2.3 Build Targets

- MSVC 14.44.35207 / Ninja / vcpkg / SDL3 / Vulkan 1.4 / Jolt 5.6.0 / enkiTS / Dear ImGui / GameNetworkingSockets
- MSVC `/W3` zero-warning target
- Debug builds with Vulkan validation layers enabled
- ASAN build on demand
- `./build-ninja/ZombieEngineTests.exe`: 20 passed, 1 skipped, 83/83 assertions green as the minimum gate

# 3. Research Backbone

Discipline:
- [S] = exact cache-backed quote or direct source attached
- [E] = ZE engineering reasoning tied to spec or appendix requirement
- [X] = named blocker, next-action, and/or citation target

## 3.1 Paper-to-Milestone Map — Block-Level Detail

| # | Paper | Status | Exact Block Mapping | Primary Lesson(s) |
|---|-------|--------|---------------------|-------------------|
| 1 | PCG in Games Survey + LLM | [X] | M4-EXT-*, M13-EXT-* | LLM-as-PCG-supervisor; hybrid MCTS/noise POI layout |
| 2 | Generative Agents | [X] | M5-EXT-*, M13-EXT-* | Observation stream, reflection layer, dynamic retrieval; 25-agent benchmark |
| 3 | ProcGen/Rendering Thesis | [X] | M4-EXT-*, M4.5-EXT-* | Region-based hierarchy, cached/runtime terrain, impostor vegetation |
| 4 | Aokana GPU Voxel | [X] | M4.5-EXT-*, M1-EXT-*, M13-EXT-* | SVDAG compression, hybrid voxel-plus-mesh, LOD streaming |
| 5 | Player Agency | [X] | M5-EXT-*, M11-EXT-* | Persistent systemic consequence; avoid false-choice trees |
| 6 | DDA Systematic Review | [X] | M5-EXT-*, M11-EXT-* | Rule-based director preferred over opaque ML for shipping |
| 7 | Real-Time Fracturing | [X] | M3-EXT-* | Material-dependent fracture response; prefracture small objects |
| 8 | Environmental Storytelling | [X] | M11-EXT-* | Embedded/emergent/interpretive narrative layers |
| 9 | Boids/Swarm Intelligence | [X] | M5-EXT-* | Fully decentralized horde control; scent/sound goal rule add-on |
| 10 | GSound | [X] | M6-EXT-*, M13-EXT-* | Geometric audio with portal-aware diffraction; 10-20 Hz cadence |
| 11 | Tension Space Analysis | [X] | M5-EXT-*, M11-EXT-* | Tension = player options minus threat; state-machine pacing |
| 12 | Player-Driven Emergence | [X] | M11-EXT-*, M5-EXT-* | Runtime telemetry exposes dead narrative branches |
| 13 | Concordia | [X] | M5-EXT-*, M12-EXT-* | GM-layer action validation; physical/social/digital state spaces |
| 14 | LLM Game Agents Survey | [X] | M5-EXT-* | 6-component NPC architecture; tiered memory bottleneck |
| 15 | Closing the Loop | [X] | M11-EXT-*, M13-EXT-*, M5-EXT-* | Sense-model-adapt loop; ~30s cadence; four-phase pacing |
| 16 | Agents' Room | [X] | M11-EXT-*, M13-EXT-* | Narrative grammar chain: EventType → NPCSelector → OutcomeResolver → FlavorWriter |
| 17 | Procedural Animation/Parkour | [X] | M5-EXT-* | Motion matching + IK post-process unified locomotion state machine |
| 18 | Save Game Serialization | [X] | M7-EXT-* | SoA layout, delta compression, periodic full checkpoint, schema versioning |
| 19 | Game Economy Balancing EA | [X] | M8-EXT-*, M13-EXT-* | Offline Pareto-optimized economy; per-save seeded variation |
| 20 | Removing the HUD | [S] | M11-EXT-* | Diegetic HUD only; contextual information only |
| 21 | Modular Quest Generation CONAN | [X] | M11-EXT-*, M13-EXT-* | Planner-based quests validated against world state |
| 22 | Modulith | [X] | M7-EXT-*, M13-EXT-* | DAG load order with cycle detection, capability sandbox, versioned mod API |
| 23 | Aim Assist Console Shooters | [X] | M11-EXT-* | Dual-zone controller response; gyro-first input; rotational assist |
| 24 | Intersection-Free Rigid Body | [S] | M2-EXT-*, M9-EXT-* | Curved-trajectory CCD; barrier stiffness from surface material |
| 25 | XPBD | [X] | M2-EXT-*, M5-EXT-* | Unified collision/ragdoll/constraints; compliance increases with damage |
| 26 | Breaking Good Fracture | [S] | M3-EXT-* | Precomputed fracture modes at asset build; GPU compute for cracked geometry |
| 27 | Real-Time Eulerian Water | [S] | M6.5-EXT-*, M10-EXT-* | Tall-cell grid; two-way buoyancy coupling; compute Poisson solve |
| 28 | Interactive Dynamic Response | [S] | M2-EXT-*, M5-EXT-* | Blended active ragdoll; three-state physics modes; deterministic transitions |
| 29 | Fire Modeling | [S] | M6.5-EXT-* | Dual-layer fire/smoke; fuel metadata per material; temperature-to-color |
| 30 | Meshlet Rendering | [X] | M3-EXT-*, M4.5-EXT-* | Compressed meshlets; Hi-Z coarse cull before task shader dispatch |
| 31 | Nanite Virtual Geometry | [X] | M4.5-EXT-*, M0-EXT-*, M13-EXT-* | Cluster DAG with screen-space error; visibility buffer deferred shading |
| 32 | GPU-Driven Rendering | [X] | M1-EXT-*, M4.5-EXT-* | GPU-only frustum/occlusion/LOD cull into indirect draw buffer |
| 33 | Quadric Error Metrics | [S] | M4-EXT-*, M3-EXT-* | Surface simplification with attribute-aware weighting; progressive mesh |
| 34 | Neural Denoising+Upscale | [S] | M4.5-EXT-*, M13-EXT-* | Single-pass neural denoise+upscale; temporal reprojection; Halton jitter every 8 frames |
| 35 | DeepMimic | [S] | M5-EXT-*, M13-EXT-* | Physics-based skill policy weights gated by health/state tier |
| 36 | Ecoclimates | [S] | M4-EXT-*, M10-EXT-* | Two-timescale weather/ecology coupling; suitability-function placement |
| 37 | Bayesian Reputation | [S] | M5-EXT-*, M11-EXT-* | Bayesian reputation with gossip; threshold-gated dialogue |
| 38 | Faction Systems | [S] | M8-EXT-*, M11-EXT-* | Faction values over allegiance; belief-state gating for cooperation/expulsion |
| 39 | Daylight Model | [S] | M10-EXT-* | Analytic sun position; turbidity sky model; sky-zenix aerial perspective |
| 40 | Cine-AI | [X] | M11-EXT-*, M13-EXT-* | Camera idioms as director style; runtime shot interpolation by gameplay moment |
| 41 | Neural Layered BRDF | [S] | M3-EXT-* | Neural layered BRDF atlas; thin-film interference as extra input |
| 42 | Ballistic Trajectories | [S] | M2-EXT-*, M3-EXT-* | Closed-form ballistic drag per caliber; intercept-angle lead prediction |
| 43 | Fast Weather Simulation | [S] | M10-EXT-* | City-block weather cells |
| 44 | Weather Simulation duplicate | [S] | M10-EXT-* | Same as 43; duplicate DOI warning retained |
| 45 | Branching Quests | [S] | M11-EXT-*, M13-EXT-* | World-state parameterized quest templates with offline validation |
| 46 | Vegetation Modeling | [S] | M4-EXT-* | Poisson-disk-with-competition placement; seasonal flora state machine |
| 47 | Neural Layered BRDF duplicate | [S] | M3-EXT-* | Duplicate of 41 |
| 48 | Geometry Caches | [X] | M4.5-EXT-* | Bake Alembic to GPU-optimized binary; triple-buffer streaming |
| 49 | VRS | [S] | M4.5-EXT-*, M13-EXT-* | Roughness-adaptive VRS tile grid with temporal stabilization |
| 50 | Glossy Reflections | [X] | M4.5-EXT-* | World-space radiance fallback when screen cache disoccludes; roughness-stratified budget |

## 3.1 Remaining Research Actions — Must Complete Before Coding

These must move to [S] before any implementation agent uses them as requirements.

**High-priority exact extraction actions:**
1. Paper 1: fetch `arXiv:2410.15644` abstract, extract LLM-as-PCG-supervisor specifics → `[S]`
2. Paper 2: fetch `arXiv:2304.03442`, extract observation-stream, reflection-layer, dynamic retrieval wording → `[S]`
3. Paper 3: fetch CalPoly thesis or cached direct URL; extract region-based hierarchy, terrain streaming, impostor vegetation → `[S]`
4. Paper 4: fetch `arXiv:2505.02017`, extract SVDAG compression, hybrid voxel-plus-mesh, LOD streaming → `[S]`
5. Paper 5: fetch `gamestudies.org/1901/articles/stang`, extract consequence-cascade wording → `[S]`
6. Paper 6: fetch DDA systematic review, extract pacing/emotion wording → `[S]`
7. Paper 7: fetch fracture paper, extract material-dependent mode taxonomy → `[S]`
8. Paper 8: fetch IntechOpen env storytelling chapter, extract layer taxonomy → `[S]`
9. Paper 9: fetch Reynolds SIGGRAPH 1987 paper or archive page → `[S]`
10. Paper 10: fetch GSound white paper directly → `[S]`
11. Paper 11: fetch `arXiv:2004.10808`, extract tension formula units → `[S]`
12. Paper 12: fetch player-driven emergence paper, extract telemetry/dead-branch mechanism → `[S]`
13. Paper 13: fetch `arXiv:2312.03664`, extract state-space design → `[S]`
14. Paper 14: fetch LLM game agent survey, confirm 6-component taxonomy → `[S]`
15. Paper 15: fetch `arXiv:2505.01351`, extract sense-model-adapt cadence → `[S]`
16. Paper 16: fetch `arXiv:2410.02603`, confirm chain ordering → `[S]`
17. Paper 17: fetch parkour thesis, extract locomotion state machine wording → `[S]`
18. Paper 18: fetch `arXiv:2410.08659`, confirm delta compression wording → `[S]`
19. Paper 19: fetch `arXiv:2404.18574`, extract Pareto front wording → `[S]`
21. Paper 21: fetch `arXiv:1808.06217`, confirm planner validation → `[S]`
22. Paper 22: fetch Modulith paper, confirm sandbox/versioning wording → `[S]`
23. Paper 23: fetch GDC Vault aim-assist talk index → `[S]`
30. Paper 30: fetch meshlet paper, confirm Hi-Z cull wording → `[S]`
31. Paper 31: attach Nanite SIGGRAPH excerpt from cache → `[S]`
32. Paper 32: fetch GPU-Driven Rendering paper from backup → `[S]`
40. Paper 40: fetch `arXiv:2208.05701`, attach cache → `[S]`
48. Paper 48: fetch SIGGRAPH 2014 geometry-cache paper → `[S]`
50. Paper 50: fetch SIGGRAPH Asia 2023 glossy reflections → `[S]`

**Cross-game truths remaining [X]:**
- 7DTD Blood Moon cadence
- State of Decay 2 roster-loss systems
- Project Zomboid moodle interaction matrix
- DayZ persistence reset mechanics
- 7DTD game-stage scaling thresholds
- Dying Light 2 traversal-first design
- RimWorld storyteller cadence and weighting
- DayZ/7DTD alpha postmortem scope discipline sentences

# 4. Audio Pipeline Plan — Detailed

This is the concrete plan for M6/M6.5 audio. It is detailed because audio is the primary survival feedback channel and must be right.

## 4.1 Audio System Architecture

```
Audio Sources → Spatial Mixer → Propagation Graph → Effect Bus → Output
                 ↓                ↓                  ↓
            3D Panning      Occlusion/      Reverb/HRTF/
                            Diffraction     Convolution
```

## 4.2 M6 Audio Milestone Blocks

**M6-EXT-01: Core Audio Engine**
- 48kHz/24-bit fixed-point audio pipeline.
- Platform abstraction: WASAPI on Windows, PulseAudio on Linux, CoreAudio on macOS.
- Voice pool: 128 concurrent voices, priority-ordered by distance/importance.
- Mixer: 2D + 3D submix; 3D uses LJ-RTF HRTF by default.

**M6-EXT-02: Spatial Audio Panning**
- Listener-centric spherical panning.
- Doppler shift for moving sources.
- Distance attenuation: inverse-square with min/max clamp.
- Cone attenuation for directional sources.

**M6-EXT-03: Acoustic Propagation Graph**
- Portal-aware diffraction based on GSound principles.
- Geometry-driven acoustic zones from world collision mesh.
- Frequency-band diffraction: low frequencies bend, high frequencies shadow.
- Update cadence: 10-20 Hz for ambient; 50 Hz for dynamic sources.

**M6-EXT-04: Voice Culling and Priority**
- Distance/importance scoring per source per frame.
- N highest-scoring sources occupy hardware voices.
- Culled sources remain logically active; resume when scope re-enters.
- UI indicator for culled sources (debug only).

**M6-EXT-05: Reverb and Convolution**
- Convolution reverb from voxel occlusion (M6-EXT-12).
- Zone-based reverb presets: outdoor, indoor, tunnel, vehicle.
- Wet/dry mix driven by listener position and portal exposure.

**M6-EXT-06: Dynamic Audio Events**
- EventBus integration: gameplay events fire audio events.
- Footstep synthesis from surface material + velocity.
- Weapon report with material-dependent reflection.
- Zombie moan with stress-level pitch modulation.

**M6-EXT-07: Audio Middleware Integration**
- FMOD or Wwise integration layer.
- Event-driven audio replaces hand-tied sources.
- Platform-agnostic event names; implementation per platform.

**M6-EXT-08: Ray-Traced Acoustic Diffraction Node Topology Cache**
- Precompute diffraction nodes from portal geometry.
- Cache per acoustic zone; invalidate on structural change.
- Node lookup O(1) per source-listener pair.

**M6-EXT-09: Velvet-Noise Late Reverb Interleaved Mixing Buffer**
- Late reverb using velvet-noise decorrelation for diffuse tail.
- Interleaved mixing reduces buffer count.
- CPU budget: <0.2ms per frame for full reverb tail.

**M6-EXT-10: Acoustic Convection Wave Refraction Filter**
- Temperature-gradient refraction for outdoor audio.
- Wind direction affects sound propagation direction.
- Simple analytic model: Snell's law at air-mass boundary.

**M6-EXT-11: Convolution-Reverb from Voxel Occlusion**
- Voxelized acoustic occlusion field from world geometry.
- Convolution IR per listener position.
- Update at 10-20 Hz; interpolate between updates.

## 4.3 M6.5 VFX Audio Integration

**M6.5-EXT-14: Audio-Reactive VFX**
- Fire crackle drives particle emission rate.
- Explosion shockwave drives screen shake + audio sub-bass.
- Weather rain audio masks zombie footstep audio by ~6dB.

## 4.4 Audio Verification Gates

| Gate | Test | Pass Criteria |
|------|------|---------------|
| M6-EXT-01 | Build audio engine | No validation errors; 48kHz output confirmed |
| M6-EXT-03 | Propagation graph | 100 sources processed in <10ms |
| M6-EXT-05 | Convolution reverb | RT60 matches reference IR within 5% |
| M6-EXT-08 | Diffraction cache | Hit rate >95% after warmup |
| M6-EXT-09 | Velvet reverb | CPU cost <0.2ms/frame |
| M6-EXT-10 | Convection filter | Audio direction偏移 matches wind vector |
| M6-EXT-11 | Voxel convolution | Occlusion transitions audible; no artifacts |

## 4.5 Audio Failure Modes

- Dropout: voice pool exhaustion; check importance ranking cutoff.
- Delay: propagation update on main thread; offload to worker.
- Memory leak: inspect event handle lifetime; validate pool recycling.
- Platform crash: WASAPI/PulseAudio/CoreAudio handle leak; validate release path.

# 5. Paper-Driven Implementation Queue

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

# 6. Critical Implementation Order

The implementation order follows the repo build topology and explicit `depends-on` chains in `APPENDICES.md`.

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

# 7. Acceptance Criteria and Verification Gates

## 7.1 Spec Documentation Gates (Must Pass Before Coding)

- [ ] `verify_ext_block_counts.py` reports 0 warnings across all milestones
- [ ] No unresolved `[X]` in research backbone unless tracked as a named blocker
- [ ] Every cited paper has exact block-level mapping
- [ ] Every cross-reference in `spec/M*.md` resolves to a real live block
- [ ] Every missing sub-milestone index is either created or cross-references are reconciled

## 7.2 Implementation Readiness Gates

- [ ] All `depends-on` chains in `APPENDICES.md` trace to real live IDs
- [ ] Build path green: `cmake -B build -G Ninja` → `cmake --build build --target ZombieEngine`
- [ ] Tests green: `ZombieEngineTests.exe` 20 passed, 83/83 assertions
- [ ] Headless smoke test exit 0
- [ ] Tier-0 performance baseline established: 30 FPS minimum on RTX 2070 SUPER

## 7.3 Functional Verification Gates

- [ ] M0: Triangle example renders on Tier-0 hardware with no validation errors
- [ ] M1: 10,000 entities rendered via indirect draw in under 10ms
- [ ] M2: 50-body physics simulation within 5ms with deterministic replay
- [ ] M3: Wall fractures into debris with static-load propagation
- [ ] M4: 1 km² chunk generated deterministically with road connectivity
- [ ] M5: 200 zombies update with scent diffusion in under 5ms
- [ ] M6: 100 concurrent audio sources with occlusion in under 10ms
- [ ] M7: Save/load round-trip under 50ms with schema migration
- [ ] M8: 500 items generated with knowledge-gated unlock
- [ ] M9: Vehicle with traction, damage, and fluid wading
- [ ] M10: Full day/night cycle with weather and atmosphere
- [ ] M11: Full HUD with MSDF text, input remap, and haptics
- [ ] M12: 2-client co-op with deterministic state sync
- [ ] M13: NPC dialogue with local LLM under 200ms latency

# 8. Professional Standards and CI/CD

## 8.1 Code Review

- Every change requires at least one peer review before merge.
- Review focus: correctness, performance, Vulkan validation, test coverage.
- Use pull-request workflow on GitHub; branch `spec/m0-parity-reformat` for spec work, `main` for integration-ready code.

## 8.2 Continuous Integration

- GitHub Actions runs on every push to `spec/*` and `main`.
- Jobs: configure with vcpkg toolchain, MSVC via `ilammy/msvc-dev-cmd`, Ninja build, `ZombieEngineTests.exe`, headless smoke.
- Artifacts: `ZombieEngine.exe`, `ZombieEngineTests.exe` retained for 7 days.
- Target: zero warnings, tests green, smoke pass.

## 8.3 Testing Strategy

- Unit tests: `tests/unit/Test_*.cpp`, Catch2 framework, auto-discovered.
- Integration tests: headless smoke run in CI.
- Validation tests: Vulkan validation layers enabled in Debug; ASAN build on demand.
- Performance baseline: RTSS + Afterburner telemetry captured at key milestones.

## 8.4 Documentation

- Spec files are READ-ONLY; changes require explicit approval.
- Plan files in `recon/plans/` are working documents; committed for branch history.
- Research path document exists at `recon/plans/2026-07-21_RESEARCH_PATH_FIX.md`.
- Fetcher script at `scripts/fetch_research.py` for free offline-friendly research.

## 8.5 Source Tracking Rule

- Every claim must carry one of: [S] sourced, [E] engineering reasoning, [X] verification needed.
- No permanent [E] assumptions without linked evidence.
- Research backlog moves items from [X] to [S] or remains [X] as a blocker.

# 9. Risk Register and Mitigations

| Risk | Probability | Impact | Mitigation | Indicator |
|------|-------------|--------|------------|-----------|
| M0 descriptor fragmentation | Medium | High | Defrag guard + overflow pool | Allocation latency spike under load |
| M2 XPBD numerical drift | Medium | High | Energy conservation test suite | Replay mismatch after 60s sim |
| M4 VRAM budget exceeded | High | Critical | RVT compression + mip-LOD | VRAM usage exceeds 6GB on Tier-0 |
| M5 AI performance at scale | Medium | High | GPU offload + spatial hash | Frame time exceeds 5ms AI budget |
| M12 network desync | Medium | Critical | Deterministic replay + CRDT | Divergent world state after 30s |
| M5.4 missing index | High | High | Create index or reconcile refs | Cross-ref audit still failing |
| Dangling M2.7/M2.8/M2.9 refs | Medium | Medium | Audit and resolve sub-milestone bodies | Cross-ref count > 0 |
| Cross-ref drift in appendices | Medium | Medium | Canonical copy discipline; remove duplicates | Stale duplicate detected in diff |
| Paper extraction backlog | Medium | Medium | Parallel fetch track with cache-backed [S] goal | [X] count still > 0 |
| Audio pipeline schedule slip | Medium | High | Stub M6-EXT-01..11 in order; validate each gate before next | Audio verification gate failure |

# 10. Short-Term Action Plan — Next 10 Days

This is the executable closure plan. Everything else is execution after this.

## Days 1-2: Spec Structural Closure

- [ ] Read `spec/M0.md` blocks 40-53 directly; identify true count vs M0.index.json.
- [ ] Read `spec/M2.md` appendix section; list every referenced ID and check existence.
- [ ] Read `spec/M5.md` sub-milestone references; confirm whether M5.1/5.2/5.3/5.4 bodies exist.
- [ ] Create priority-ordered fix queue: create missing indexes OR remove stale refs.
- [ ] Re-run `scripts/verify_ext_block_counts.py` after fixes.

## Days 3-4: Research Backbone Closure

- [ ] Fetch papers 1-19, 21-23, 30-32, 40, 48, 50 directly.
- [ ] Extract [S] quotes for each; attach cache path to plan.
- [ ] Replace milestone-level paper mapping with exact block-level mapping.
- [ ] Cross-check 8 cross-game truths; extract or archive as design-only context.

## Day 5: Audio Pipeline Plan Approval

- [ ] Review Section 4 (Audio Pipeline Plan) with user.
- [ ] Approve M6-EXT-01..11 block sequence and verification gates.
- [ ] Flag any audio blocks that conflict with existing spec.

## Days 6-7: Verification

- [ ] Re-run cross-reference audit after structural fixes.
- [ ] Re-run paper extraction audit.
- [ ] Confirm 0 unresolved blockers or document remaining named blockers.

## Days 8-10: Execution Handoff

- [ ] Finalize acceptance criteria per milestone.
- [ ] Create implementation branch `spec/audio-pipeline-closure`.
- [ ] Hand off to implementation agent with exact block read order.

# 11. Glossary

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
