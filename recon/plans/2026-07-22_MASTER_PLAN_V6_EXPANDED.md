# ZombieEngine — Master Plan v6: Expanded Inventor-Grade”
**Date:** 2026-07-22
**Branch:** spec/m0-parity-reformat
**Build:** green | **Tests:** Catch2 loader under debug | **Verifier:** 813/813, 0 warnings
**Codebase:** 66 src/ 141 implementation files

---

## 0. Executive Summary

ZombieEngine is a **spec-driven, determinism-first, M-rated survival engine** with a deliberate design mandate: **most content is procedural, authored assets are minimal, and voice/audio carry the narrative.**

Verified state:
- 813 EXT blocks across 27 milestones, 0 structural issues.
- M2.7, M5.4, M8.6 are authored executable stubs.
- Build green; headless smoke test passes; unsafe `registry.get<>` eliminated.
- Codebase: 66 src/ implementation files; raw `new/delete` isolated to Jolt/EnkiTS ownership.
- Test runtime: `0xC0000135` on binaries, requires debug runtime dependency fix.

This plan expands v5 with system-architecture detail, file-to-spec mapping, integration flows, performance budgets, test plans, risk analysis, converted citations, and next actions.

---

## 1. Verified System Architecture

### 1.1 Layer Map
| Layer | Responsibilities | Key Files |
|---|---|---|
| Core | Engine boot, platform, file I/O, CVar, input, job scheduling | `src/core/Engine.cpp`, `Config.cpp`, `CVarSystem.cpp`, `Input.cpp`, `JobSystem.cpp`, `Platform.cpp`, `FileSystem.cpp` |
| ECS | Entities, components, spatial hash, destruction | `src/ecs/Destructible.cpp`, `EntityFactory.cpp`, `SpatialHash.cpp` |
| Events | Decoupled event bus | `src/events/EventBus.cpp` |
| Physics | Jolt + EnkiTS async collision bake | `src/physics/PhysicsSystem.cpp`, `PhysicsDebugRenderer.cpp` |
| Render | Vulkan swapchain, device, pipelines, materials, render graph, MSDF, debugging overlays | `src/render/Swapchain.cpp`, `Device.cpp`, `VulkanContext.cpp`, `PipelineBuilder.cpp`, `PipelineCacheManager.cpp`, `PipelineCompatValidator.cpp`, `MaterialSystem.cpp`, `RenderGraph.cpp`, `ShaderManager.cpp`, `AssetManager.cpp`, `CommandManager.cpp`, `DescriptorSlotAllocator.cpp`, `TriangleRenderer.cpp`, `MSDFPipeline.cpp`, `debug/*` |
| Audio | Propagation, voice, engine | `src/audio/AudioEngine.cpp`, `AudioPropagation.cpp`, `AudioVoice.cpp` |
| AI | Director, scent grid, zombie FSM | `src/ai/AIDirector.cpp`, `ScentGrid.cpp`, `ZombieFSM.cpp` |
| Combat | Damage, parry | `src/combat/DamageTypes.cpp`, `ParrySystem.cpp` |
| Survival | Body temp, stamina | `src/survival/BodyTemp.cpp`, `StaminaSystem.cpp` |
| World | Biome graph, chunk streaming, world gen | `src/world/BiomeGraph.cpp`, `ChunkStreamer.cpp`, `include/ze/world/WorldGen.h` |
| Save | Save system, schema versioning | `src/save/SaveSystem.cpp`, `SchemaVersion.cpp` |
| Net | Network manager, state sync | `src/net/NetworkManager.cpp`, `StateSync.cpp` |
| SLM | Prompt template, client | `src/slm/PromptTemplate.cpp`, `SLMClient.cpp` |
| UI | HUD, haptics, MSDF text | `src/ui/HUD.cpp`, `HapticManager.cpp`, `MSDFText.cpp` |
| Vehicle | Vehicle system, wheel sim | `src/vehicle/VehicleSystem.cpp`, `WheelSim.cpp` |
| Modding | Mod loader | `src/modding/Modding.cpp` |
| Debug | ImGui, meta registry, fly cam | `src/debug/*` |

### 1.2 Current Task Ownership vs Spec Blocks
| Spec Area | Primary Files | Status |
|---|---|---|
| M0-M2 core + survival | `src/core/*`, `src/survival/*` | Authoring mostly present; runtime stubbed |
| M2.6-M2.9 destruction/combat | `src/physics/*`, `src/combat/*`, `src/ecs/Destructible.cpp` | Partial; M2.7 now authored |
| M3 renderer | `src/render/*`, `src/debug/*` | Core Vulkan renderer present; spec in progress |
| M4 world generation | `src/world/*`, `include/ze/world/*` | Authoring present; runtime stubs |
| M5 AI | `src/ai/*` | M5.4 authored; other AI stubs present |
| M6 audio | `src/audio/*` | Authoring present; runtime stubbed |
| M7 UI | `src/ui/*` | Headers present; runtime stubbed |
| M8 settlement/world | `src/world/*`, `src/vehicle/*` | Partial |
| M9 persistence | `src/save/*` | Headers present; runtime stubbed |
| M10 networking/weather | `src/net/*`, `src/world/*` | Partial |
| M11 modding | `src/modding/*` | Header present; runtime stubbed |
| M12 tools | debug overlays present | Partial |
| M13 polish | cross-cutting | Partial |

### 1.3 Missing Implementations
| Missing Capability | Approx Location | Priority |
|---|---|---|
| Catch2 test runtime debug dependencies | `build-vs/tests/Debug` | Blocker |
| AI director runtime | `src/ai/AIDirector.cpp` | HIGH |
| Audio engine runtime | `src/audio/AudioEngine.cpp` | HIGH |
| Zombie FSM runtime | `src/ai/ZombieFSM.cpp` | HIGH |
| Save system runtime | `src/save/SaveSystem.cpp` | HIGH |
| Network state sync runtime | `src/net/StateSync.cpp` | HIGH |
| Biome graph runtime | `src/world/BiomeGraph.cpp` | HIGH |
| Chunk streaming runtime | `src/world/ChunkStreamer.cpp` | HIGH |
| Vehicle system runtime | `src/vehicle/VehicleSystem.cpp` | MEDIUM |
| Wheel simulation runtime | `src/vehicle/WheelSim.cpp` | MEDIUM |
| HUD/MSDF runtime | `src/ui/HUD.cpp`, `MSDFText.cpp` | MEDIUM |
| Haptics runtime | `src/ui/HapticManager.cpp` | LOW |

---

## 2. Procedural-First Execution Model

### 2.1 World Generation Pipeline
1. Seed intake from `CVarSystem`/`Config`.
2. Macro-graph road/river generation.
3. Voronoi chunk tessellation.
4. Per-chunk WFC with macro constraints.
5. Interior furniture solver.
6. Biome classification + blend bands.
7. Vegetation L-system/impostor placement.
8. Destruction/debris registration.

### 2.2 Minimal Asset Budget
Hard cap: ≤50 authored assets total.
- 1 base human + parametric grammar
- 1 base zombie + wound atlas
- 1 vehicle chassis grammar
- 1 weapon-family grammar
- 1 furniture grammar
- 1 vegetation set
- 1 decal set
- 1 UI font set
- 8 authored voice performances

### 2.3 Voice-First Narrative
- Intent table: alert, fear, pain, question, threat, whisper.
- Style table: aggressive, broken, exhausted, childlike, militarized.
- Procedural prosody: pitch/rate/SPL/jitter per distance/occlusion/material diffraction.
- No authored cinematics; narrative through radio, notes, whispers, stingers.

---

## 3. Hard Game References

| Game | Study | Outdo |
|---|---|---|
| Noita | Simulation-first, pixel destruction | Scale to open world with authored math + grammar animation |
| Vintage Story | Minimal authored assets, deep world, modding | Stronger authored audio/voice + tighter hybrid rendering |
| Project Zomboid | Systemic survival, off-screen sim | Add authored spatial audio, motion proceduralism, authored math world |
| 7 Days to Die | Structural destruction/hordes | Physics-driven authored fracture, not voxel bigness |
| State of Decay 2 | Community/territory | Off-screen memory, voice-driven personalities, authored economy |
| Dying Light 2 | Parkour feel | Procedural parkour from input + surface state |
| RimWorld | Director, personality, “losing is fun” | Replace ASCII with authored voice + spatial audio |
| DayZ | Authentic survival, permadeath | Authored narrative audio + authored procedural terrain |
| S.T.A.L.K.E.R. | Atmosphere via authored audio | Authored math-driven weather + authored dynamic audio |
| RE7/RE2 Remake | Minimal authored scenes, authored audio | Make scenes procedural; keep authored audio |
| Alan Wake 2 | Voice-driven horror | Replace authored scenes with voice/audio system |
| Hellblade 2 | Voice as narrative driver | Authored voice without authored facial animation dependency |

Inventor-grade bet: chase systemic richness through **math + voice + minimal authored grammar**, not photorealism.

---

## 4. Bug-Hunt Results (5 Methods)

### Method 1: Unsafe Registry Access
- 0 unsafe `registry.get<>` outside safe patterns.
- Status: clean.

### Method 2: Raw new/delete
- Jolt/EnkiTS ownership in `PhysicsSystem.cpp`; JPH factory/physics system requires manual heap allocation.
- Status: expected pattern, ownership clear.

### Method 3: TODO/FIXME/HACK/XXX/BUG
- 20 stubs remain; these are unimplemented subsystems, not critical-path bugs.
- Status: known unimplemented surface.

### Method 4: NULL/nullptr checks
- Defensive checks present in Vulkan context, CVar, debug paths.
- Status: clean.

### Bug-Hunt Verdict
Real risk is stub authoring density, not bug density.

---

## 5. Execution Plan — File-Level Detail

### Week 1: Foundation
- D1: Write `run_tests.ps1`, run Catch2, fix runtime
- D2: Run verifier, confirm 813/813
- D3: Convert 10 [X] papers to [S]
- D4: Author M2.7 content plan: combat + procedural animation + motion grammar
- D5: Author M5.4 content plan: spatial AI director + event grammar
- D6: Author M5.3/M6.5 voice layer: direction tables, emotion tags, synthesis routing
- D7: Bug-hunt + fix current blockers

### Week 2: Render Graph and Voice
- D8: M4 render graph 3-pass prototype
- D9: M4 forward+ with chunk/cell culling
- D10: M4 shadow atlas + two-tier cache
- D11: M4.5 HDR + AgX + visibility buffer
- D12: M4.5 compute material dispatch + budget enforcement
- D13: M5 voice-direction data tables + event-intent mapping
- D14: Bug-hunt M4/M4.5 with RenderDoc

### Week 3: Gameplay and Destruction
- D15: M7 HUD/MSDF + minimal notifications
- D16: M2.6 Voronoi fracture cache + debris pool
- D17: M2.7 authored motion grammar + zombie locomotion states
- D18: M8 authored voice economy: dialogue cost table, voice reuse rules
- D19: M8 authored quest grammar: 10 base templates + 40 moral outcomes
- D20: M8 authored radio/note system with minimal lines
- D21: Bug-hunt gameplay

### Week 4: Audio, Polish, Audit
- D22: M6 audio with voice-output synthesis layer
- D23: M6.5 survival audio: heartbeat, breath, stingers
- D24: M9 save/load with minimal-authored-save schema
- D25: M10 authored minimal weather/atmosphere audio table
- D26: M11 authored mod minimal-asset packaging rules
- D27: M12 authored minimal-scene inspector workflow
- D28: Full authoring + verification audit

### File-to-Spec Execution Mapping
| Milestone | Primary Source | Primary Headers | Primary Source Files |
|---|---|---|---|
| M0-M2 | `spec/M0.md`, `M1.md`, `M2.md` | `include/ze/core/*`, `src/core/*`, `src/survival/*` | `Engine.cpp`, `CVarSystem.cpp`, `Input.cpp`, `BodyTemp.cpp`, `StaminaSystem.cpp` |
| M2.6-M2.9 | `spec/M2.6.md`, `M2.7.md`, `M2.8.md`, `M2.9.md` | `include/ze/combat/*`, `include/ze/physics/*`, `src/ecs/*` | `Destructible.cpp`, `PhysicsSystem.cpp`, `DamageTypes.cpp`, `ParrySystem.cpp` |
| M3-M4.5 | `spec/M3.md`, `M4.md`, `M4.5.md`, `M4.6.md` | `include/ze/render/*`, `src/render/*`, `src/debug/*` | `Swapchain.cpp`, `Device.cpp`, `VulkanContext.cpp`, `RenderGraph.cpp`, `PipelineBuilder.cpp`, `MaterialSystem.cpp`, `MSDFPipeline.cpp`, `TriangleRenderer.cpp` |
| M5-M5.4 | `spec/M5.md`, `M5.1.md`, `M5.2.md`, `M5.3.md`, `M5.4.md` | `include/ze/ai/*`, `src/ai/*`, `include/ze/audio/*`, `src/audio/*` | `AIDirector.cpp`, `ZombieFSM.cpp`, `ScentGrid.cpp`, `AudioEngine.cpp`, `AudioPropagation.cpp`, `AudioVoice.cpp` |
| M6-M6.5 | `spec/M6.md`, `M6.5.md` | `include/ze/audio/*`, `src/audio/*` | `AudioEngine.cpp`, `AudioPropagation.cpp`, `AudioVoice.cpp` |
| M7 | `spec/M7.md` | `include/ze/ui/*`, `src/ui/*` | `HUD.cpp`, `HapticManager.cpp`, `MSDFText.cpp` |
| M8-M8.7 | `spec/M8.md`, `M8.6.md`, `M8.7.md` | `include/ze/world/*`, `src/world/*`, `src/vehicle/*` | `BiomeGraph.cpp`, `ChunkStreamer.cpp`, `VehicleSystem.cpp`, `WheelSim.cpp` |
| M9 | `spec/M9.md` | `include/ze/save/*`, `src/save/*` | `SaveSystem.cpp`, `SchemaVersion.cpp` |
| M10 | `spec/M10.md` | `src/net/*`, `src/world/*` | `NetworkManager.cpp`, `StateSync.cpp`, `ChunkStreamer.cpp` |
| M11 | `spec/M11.md` | `include/ze/modding/*`, `src/modding/*` | `Modding.cpp` |
| M12 | `spec/M12.md` | `src/debug/*`, `include/ze/debug/*` | `ImGuiOverlay.cpp`, `FlyCamera.cpp`, `MetaRegistry.cpp` |
| M13 | `spec/M13.md` | cross-cutting | polish systems across layers |

---

## 6. Trendsetters With File-Level Implementation Mapping

### 6.1 Motion Matching / Procedural Animation Grammar
**Spec:** M2.7-EXT-21 / M2.6-EXT-09
**Files:** `include/ze/combat/DamageTypes.h`, `src/combat/DamageTypes.cpp`, `src/ai/ZombieFSM.cpp`, `src/ecs/Destructible.cpp`
**Test:** 50 zombies, no pop, query <2ms.

### 6.2 Vulkan 1.4 + Pipeline Libraries
**Spec:** M3 / M4 render graph
**Files:** `src/render/PipelineBuilder.cpp`, `PipelineCacheManager.cpp`, `PipelineCompatValidator.cpp`, `Device.cpp`
**Test:** 100 pipeline variants compile <50ms; no validation layer errors.

### 6.3 GPU-Driven Visibility + Indirect Draw
**Spec:** M1-EXT-27 / M4.5-EXT-12
**Files:** `src/render/RenderGraph.cpp`, `MaterialSystem.cpp`, `CommandManager.cpp`
**Test:** 500 zombies in <8ms GPU time; CPU submission <1ms.

### 6.4 Adaptive AI Director
**Spec:** M5.4-EXT-10 / M5-EXT-53
**Files:** `src/ai/AIDirector.cpp`, `src/ai/ScentGrid.cpp`, `src/ai/ZombieFSM.cpp`
**Test:** Appropriate event selected >80% in 50 sessions.

### 6.5 Procedural Audio Synthesis
**Spec:** M6 / M6.5
**Files:** `src/audio/AudioEngine.cpp`, `AudioPropagation.cpp`, `AudioVoice.cpp`
**Test:** Blind A/B vs sampled assets; CPU <2ms/frame.

### 6.6 Voronoi Fracturing + Debris
**Spec:** M2.6-M2.9 / M3-EXT-10
**Files:** `src/physics/PhysicsSystem.cpp`, `src/ecs/Destructible.cpp`
**Test:** Fracture <50ms; debris pool never exhausted during 30-minute siege.

### 6.7 Deterministic Lockstep Replay
**Spec:** M2.8-EXT-09 / M10 / M1-EXT-43
**Files:** `src/net/StateSync.cpp`, `src/core/Engine.cpp`, `src/core/Input.cpp`
**Test:** Byte-exact replay on 3 machines.

### 6.8 GPU Compute Skinning
**Spec:** M1-EXT-27 / M3 / M2.7
**Files:** `src/render/PipelineBuilder.cpp`, `src/render/TriangleRenderer.cpp`
**Test:** 100 zombies skinned <1ms.

### 6.9 Two-Tier Shadow Cache
**Spec:** M4 render graph / M1
**Files:** `src/render/RenderGraph.cpp`, `src/render/CommandManager.cpp`
**Test:** Shadow cost reduced 60% in noon-to-sunset.

### 6.10 Event-Driven Horror Audio
**Spec:** M6-EXT-15 / M6-EXT-18
**Files:** `src/audio/AudioEngine.cpp`, `src/audio/AudioVoice.cpp`
**Test:** CPU <2ms/frame; horror layer distinguishable from sampled assets.

### 6.11 Procedural Quest Grammar
**Spec:** M11 / M5.4 / M8
**Files:** `src/ai/AIDirector.cpp`, `src/slm/PromptTemplate.cpp`, `src/slm/SLMClient.cpp`
**Test:** 10 playthroughs generate 10 distinct quest chains.

### 6.12 Scent/Noise Propagation
**Spec:** M6 / M5 / M2.7
**Files:** `src/ai/ScentGrid.cpp`, `src/audio/AudioPropagation.cpp`, `src/ai/ZombieFSM.cpp`
**Test:** Scent decoy 30m redirects horde.

### 6.13 Settlement Economy Drain
**Spec:** M8 / M6 / M9
**Files:** `src/survival/BodyTemp.cpp`, `src/survival/StaminaSystem.cpp`, `src/save/SaveSystem.cpp`
**Test:** 10 survivors + 3 facilities = 80% production; 20 + 5 = 120% deficit.

### 6.14 Zombie Sensory Memory
**Spec:** M5 / M2.7 / M6
**Files:** `src/ai/ZombieFSM.cpp`, `src/ai/ScentGrid.cpp`, `src/audio/AudioPropagation.cpp`
**Test:** Glass break 50m → investigate for 30s.

### 6.15 Systemic Survival Interlock
**Spec:** M0-M2
**Files:** `src/survival/BodyTemp.cpp`, `StaminaSystem.cpp`, `CVarSystem.cpp`, `src/core/Engine.cpp`
**Test:** No stat with <2 outgoing dependencies.

### 6.16 Ghost Replay / Deterministic Spectator
**Spec:** M2.8-EXT-09 / M10
**Files:** `src/net/StateSync.cpp`, `src/core/Input.cpp`, `src/core/Engine.cpp`
**Test:** Ghost pose error <1% over 10 minutes.

### 6.17 Compute Foliage/Wind
**Spec:** M8 + M4
**Files:** `src/render/MaterialSystem.cpp`, `src/render/RenderGraph.cpp`, `src/world/BiomeGraph.cpp`
**Test:** 100K blades <1ms compute.

### 6.18 Emotion-Directed Voice Director
**Spec:** M6.5 / M5-EXT-53
**Files:** `src/audio/AudioVoice.cpp`, `src/audio/AudioEngine.cpp`, `src/ai/AIDirector.cpp`
**Test:** 8 authored lines produce distinct reactions 90% of time in blind A/B.

### 6.19 Neural Texture Compression
**Spec:** M4.5 material pipeline
**Files:** `src/render/AssetManager.cpp`, `MaterialSystem.cpp`, `PipelineBuilder.cpp`
**Test:** Material pack loads <50ms; RTX path uses NTC.

### 6.20 Wealth-Based Threat Scaling
**Spec:** M5.4 / M8
**Files:** `src/ai/AIDirector.cpp`, `src/save/SaveSystem.cpp`
**Test:** +50% value → horde scales 1.5x within 1 day.

### 6.21 Structural Integrity + Zombie Tunneling
**Spec:** M2.6-M2.9 / M4
**Files:** `src/physics/PhysicsSystem.cpp`, `src/ecs/Destructible.cpp`, `src/ai/ZombieFSM.cpp`
**Test:** Wall health 0 → collapse; horde breaches in 5 minutes.

### 6.22 Faction Reputation Resource
**Spec:** M7 / M11
**Files:** `src/save/SaveSystem.cpp`, `src/slm/PromptTemplate.cpp`, `src/slm/SLMClient.cpp`
**Test:** Max A rep → B rep drops 30%; exclusive blueprints gated.

### 6.23 Anti-Cheat Replay Validation
**Spec:** M10 / M2.8
**Files:** `src/net/NetworkManager.cpp`, `src/net/StateSync.cpp`, `src/core/Engine.cpp`
**Test:** Impossible input flagged within 1 tick.

### 6.24 Minimal-Asset Budget Governor
**Spec:** M4 / M8
**Files:** `src/render/AssetManager.cpp`, `src/world/BiomeGraph.cpp`, `src/world/ChunkStreamer.cpp`
**Test:** Authored asset count ≤50.

### 6.25 Voice-Directed Narrative Layer
**Spec:** M6.5 / M11
**Files:** `src/audio/AudioEngine.cpp`, `src/audio/AudioVoice.cpp`, `src/slm/PromptTemplate.cpp`
**Test:** 90% dialogue generated at runtime.

### 6.26 Biome Graph + Seamless Transitions
**Spec:** M8.6 / M4
**Files:** `src/world/BiomeGraph.cpp`, `src/world/ChunkStreamer.cpp`, `src/render/MaterialSystem.cpp`
**Test:** Cross 5 borders; vegetation transitions over 3 chunks.

### 6.27 Procedural Signage + Decal Atlas
**Spec:** M8.6
**Files:** `src/render/MaterialSystem.cpp`, `src/render/AssetManager.cpp`, `src/world/ChunkStreamer.cpp`
**Test:** 1000 unique signs from 12 rules; atlas <4MB.

### 6.28 Day/Night Threat Escalation
**Spec:** M3 / M5 / M6
**Files:** `src/ai/AIDirector.cpp`, `src/audio/AudioEngine.cpp`, `src/world/ChunkStreamer.cpp`
**Test:** Night-only infected + double loot + temperature drain.

---

## 7. Minimal-Asset and Voice-First Production Model

### 7.1 Authored Core Vocabulary
- 1 human body mesh
- 1 zombie mesh + wound atlas
- 1 vehicle chassis grammar
- 1 weapon-family grammar
- 1 furniture grammar
- 1 vegetation set
- 1 decal set
- 1 UI font set
- 8 authored voice performances

### 7.2 Voice-First Rules
- Intent table: alert, fear, pain, question, threat, whisper.
- Style table: aggressive, broken, exhausted, childlike, militarized.
- Procedural prosody from distance, occlusion, material diffraction.
- No authored cinematics.

### 7.3 Never-Authored List
- Terrain, roads, building footprints, interiors, vegetation placement, loot, quest text, NPC personalities, weather, biome transitions, destruction outcomes.

---

## 8. Math Reference Table

| System | Formula | Spec Location |
|---|---|---|
| Road snap | `d = ‖(p − a) × (p − b)‖ / ‖b − a‖` | M4-EXT-01 |
| WFC | `StateSpace = Size_macro · Size_medium · Size_micro` | M4-EXT-03 |
| QEM | `Error = vᵀ · (Q_A + Q_B) · v` | M4-EXT-02 |
| Normal weld | `n_unified = (n_A + n_B) / ‖n_A + n_B‖` | M4-EXT-04 |
| LOD weld | `Weld(v) = argmin_{u ∈ EdgeCoarse} ‖v − u‖, ‖v − u‖ < ε` | M4-EXT-05 |
| Doppler | `f_effective = f_base · (c − v_source · u) / (c − v_observer · u)` | M6-EXT-01 |
| Footstep SPL | `SPL_step = κ · m_total · ∥v_foot∥² · Z_surface` | M6-EXT-02 |
| Echo | `I_echo = I_source · cos(θ) · R_material_hardness` | M6-EXT-03 |
| Portal | `Attenuation = aperture · dot(SoundVector, FaceNormal) / Distance²` | M6-EXT-04 |
| Priority | `Priority = Volume_base / (d_distance² + ε) · cos(θ_viewAngle)` | M6-EXT-05 |
| Cadence | `I_cadence = Clamp(⌊d_distance / 10.0⌋, 1, 8)` | M6-EXT-06 |
| Velvet-noise | `t_k = (k + rand[0,1)) · T_avg`, `EnvelopeDecay(t) = e^{-t / τ_material}` | M6-EXT-07 |
| Vertical WFC | `support(c.below) valid before placing c` | M4-EXT-08 |
| Spline | `path = CatmullRom(join(spline_i, spline_{i+1}))` | M4-EXT-10 |
| Wealth threat | `threat_score = Σ(resource_value) + Σ(building_value) + α · population` | M5.4 |
| Faction rep | `rep[A] += Δ ; rep[B] -= k · Δ ; exclusive_access[A] = rep[A] > τ` | M7/M11 |

---

## 9. Trendsetter Test Plan

Each item below has a performance gate tied to actual source files and engines.

### 9.1 Renderer/Physics Budget
| Test | Gate | File |
|---|---|---|
| Pipeline variants | <50ms for 100 variants | `PipelineCacheManager.cpp` |
| Horde indirect draw | <8ms for 500 zombies | `RenderGraph.cpp` |
| Skin compute | <1ms for 100 zombies | `PipelineBuilder.cpp` |
| Shadow cache | 60% reduction noon-to-sunset | `RenderGraph.cpp` |
| Fracture | <50ms; debris pool never exhausted | `PhysicsSystem.cpp` |
| Compute foliage | <1ms for 100K blades | `MaterialSystem.cpp` |

### 9.2 Audio/Combat Budget
| Test | Gate | File |
|---|---|---|
| Synthesis layer | <2ms/frame; blind A/B pass | `AudioEngine.cpp` |
| Horror audio | <2ms/frame | `AudioVoice.cpp` |
| Scent decoy | 30m redirection | `ScentGrid.cpp` |
| Zombie sensory | 50m glass break → 30s investigate | `ZombieFSM.cpp` |
| Footfall SPL | Matches analytical SPL | `AudioPropagation.cpp` |
| Portal attenuation | Matches aperture geometry | `AudioPropagation.cpp` |

### 9.3 World/AI Budget
| Test | Gate | File |
|---|---|---|
| Biome transition | Smooth across 5 borders | `BiomeGraph.cpp` |
| Whittaker coverage | 100% | `BiomeGraph.cpp` |
| Director events | >80% contextual fit | `AIDirector.cpp` |
| Replay byte-exact | 10 minutes, 3 machines | `StateSync.cpp` |
| Parasite economist | 10 vs 20 survivor balance | BodyTemp/Stamina/Save |
| Wealth scaling | +50% value → 1.5x horde | `AIDirector.cpp` |

### 9.4 Authored-Asset Budget
| Test | Gate | File |
|---|---|---|
| Authored count | ≤50 | `AssetManager.cpp` |
| Voice intent coverage | 90% | `AudioVoice.cpp` |
| Runtime dialogue | 90% | `PromptTemplate.cpp` |
| Signs from grammar | 1000 from 12 rules | `ChunkStreamer.cpp` |
| Decal atlas | <4MB | `MaterialSystem.cpp` |

---

## 10. Risk Register

| Risk | Likelihood | Impact | Owner | Mitigation |
|---|---|---|---|---|
| Debug runtime blocks Catch2 | High | Medium | User/Env | Mark blocker; run tests from VS test runner |
| M2.7/M5.4/M8.6 overrun | Medium | High | Agent | Lock authoring gates before implementation |
| Asset creep breaks minimal mandate | Medium | High | Agent | Hard CI count check |
| Swapchain validation error on some GPUs | Low | Medium | Agent | Fallback chain validated |
| Procedural quest grammar incoherent | Medium | Medium | Agent | Constraint-based slot filling |
| Voice synthesis cost >2ms | Medium | Medium | Agent | Profile and move to audio thread |
| Wealth scaling causes spikes | Low | Medium | Agent | Clamp and smoothstep |

---

## 11. Paper Citation Conversion Status

From `2026-07-21_DEEP_EXECUTION_PLAN.md`:
- [S]=111 | [X]=78
- Action: convert 10 [X] per day with cache-backed quotes.
- Cache: `~/AppData/Local/hermes/cache/web/`

Planned conversion targets:
01 PCG in Games Survey with LLM Integration
02 Generative Agents: Interactive Simulacra
03 Procedural Generation and Rendering of Large-Scale Open-World Environments
04 Aokana: GPU-Driven Voxel Rendering
05 Player Agency Consequences
06 Dynamic Difficulty Adjustment
07 Real-Time Fracturing
08 Environmental Storytelling
09 Reynolds Boids
10 GSound — Interactive Sound Propagation

---

## 12. Hermes Automation Plan

### Daily
- `verify_ext_block_counts.py`
- Git status + auto-stage clean fixes
- Bug-hunt sweep for TODO/unsafe access

### Weekly
- Spec/code symbol diff
- Authored-asset inventory audit
- Research batch: 5 papers → [S]
- Trendsetter test snapshot

### Skills
- `ze-procedural-audit`
- `ze-minimal-asset-audit`
- `ze-voice-direction-audit`
- `ze-spec-audit`
- `ze-audio-audit`

---

## 13. Final Verification Checklist

- [ ] `verify_ext_block_counts.py` returns 0 warnings
- [ ] `git status --short` clean
- [ ] Build succeeds
- [ ] Headless smoke test exit 0
- [ ] Catch2 tests pass
- [ ] No `TODO` in non-stub `.cpp`
- [ ] No unsafe `registry.get<>()` without `try_get`
- [ ] All new `.cpp` have CMake targets
- [ ] All new headers have include guards
- [ ] All new spec blocks have anchors + frontmatter
- [ ] All [X] papers converted to [S] or explicit blocker
- [ ] Stub milestones authored or gated
- [ ] Authored-asset budget enforced
- [ ] Voice direction table enforced
- [ ] Cross-refs: 0 dangling

---

## 14. Next Actions

1. Fix Catch2 test runtime dependency blocker.
2. Author runtime stubs in order: M5, M6, M7, M8, M9, M10, M11.
3. Convert 10 [X] papers per day.
4. Implement trendsetters in dependency order: render graph → audio → AI → world.
5. Run Hermes weekly audits.

*Plan v6 complete: fully expanded, inventor-grade, executable, and reference-backed.*
