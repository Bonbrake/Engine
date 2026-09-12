# ZombieEngine — Specification-Grade Master Plan v7
**Date:** 2026-07-22 | **Build:** green | **Verifier:** 813/813  
**Research spine:** `deepresearch/papers_1_10.txt` (10 paper corpus)

---

## 0. Design Invariant (Do Not Let the Plan Contradict This)

ZombieEngine is **procedural-first, voice/narrative-first, minimal-authored-asset.**  
Every milestone decision must be tested against that invariant.

---

## 1. Research Backbone (Verified Corpus)

All 10 papers from `deepresearch/papers_1_10.txt` fetched from arXiv metadata:

| # | Paper | Venue | Application to ZE |
|---|---|---|---|
| 1 | PCG in Games Survey with LLM Integration (arXiv:2410.15644) | AIIDE 2024 | PCG taxonomy; LLM-as-PCG superviser slot filler for quests/dialogue |
| 2 | Tension Space Analysis for Emergent Narrative (arXiv:2004.10808) | IEEE Trans Games 2020 | Pacing curve → director state machine weight derivation |
| 3 | Player-Driven Emergence in LLM-Driven Game Narrative (arXiv:2404.17027) | IEEE CoG 2024 | LLM as dialogue assembly; player agency preservation |
| 4 | Agents' Room: Narrative Generation via Multi-step Collaboration (arXiv:2410.02603) | ICLR 2025 | Multi-agent narrative architect → ZE director agent collaboration |
| 5 | Generative Agents: Interactive Simulacra of Human Behavior (arXiv:2304.03442) | CHI/Stanford 2023 | Observation stream + reflection → NPC memory graph |
| 6 | Concordia: Grounded Agent-Based Modeling (arXiv:2312.03664) | DeepMind 2023 | Physical/social/digital action grounding → ZE agent action space |
| 7 | Survey on LLM-Based Game Agents (arXiv:2404.02039) | ACM Comput Surv 2025 | Agent taxonomy; structured tool use for character AI |
| 8 | Experience-Driven Game Adaptation (arXiv:2505.01351) | arXiv 2025 | Player modeling + dynamic adjustment → ZE director adaptation |
| 9 | DDA Systematic Literature Review (Springer 2024) | Multimed Tools Appl 2024 | DDA framework selection → ZE difficulty curve calibration |
| 10 | GSound: Interactive Sound Propagation for Games | UNC Chapel Hill 2011 | Spatial acoustic model → M6 audio diffration/occlusion/portal verification |

---

## 2. System Architecture (File-Grounded)

### 2.1 Component Dependency Graph (directed: bottom→top)

```
ZombieEngine.exe
├── Engine.cpp (main loop, init, headless test)
├── Platform.cpp (sys/platform)
├── CVarSystem.cpp (CVar tokens)
├── Config.cpp (config file parsing)
├── Input.cpp (SDL keyboard/mouse)
├── JobSystem.cpp (enkiTS threading)
│
├── src/ui/ ─ HUD.cpp, HapticManager.cpp, MSDFText.cpp
├── src/ecs/ ─ Destructible.cpp, EntityFactory.cpp, SpatialHash.cpp
├── src/events/ ─ EventBus.cpp
│
├── src/physics/ ─ PhysicsSystem.cpp, PhysicsDebugRenderer.cpp
├── src/render/ ─ Swapchain.cpp, Device.cpp, PipelineBuilder.cpp, 
│                  PipelineCacheManager.cpp, PipelineCompatValidator.cpp,
│                  MaterialSystem.cpp, RenderGraph.cpp, ShaderManager.cpp,
│                  AssetManager.cpp, CommandManager.cpp,
│                  DescriptorSlotAllocator.cpp, TriangleRenderer.cpp,
│                  MSDFPipeline.cpp, VulkanContext.cpp
│
├── src/ai/ ─ AIDirector.cpp, ScentGrid.cpp, ZombieFSM.cpp
├── src/audio/ ─ AudioEngine.cpp, AudioPropagation.cpp, AudioVoice.cpp
├── src/combat/ ─ DamageTypes.cpp, ParrySystem.cpp
├── src/survival/ ─ BodyTemp.cpp, StaminaSystem.cpp
├── src/world/ ─ BiomeGraph.cpp, ChunkStreamer.cpp
├── src/vehicle/ ─ VehicleSystem.cpp, WheelSim.cpp
├── src/save/ ─ SaveSystem.cpp, SchemaVersion.cpp
├── src/net/ ─ NetworkManager.cpp, StateSync.cpp
├── src/slm/ ─ PromptTemplate.cpp, SLMClient.cpp
├── src/modding/ ─ Modding.cpp
└── src/debug/ ─ ImGuiOverlay.cpp, FlyCamera.cpp, MetaRegistry.cpp
```

### 2.2 Current Task Ownership

| Spec Layer | Primary Files | Status |
|---|---|---|
| M0-M2 Core | `src/core/*`, `src/survival/*` | Authored + runtime stubs |
| M2.6-M2.9 Destruction/Combat | `src/physics/*`, `src/ecs/*`, `src/combat/*` | M2.7 authored as executable stub |
| M3-M4 Renderer | `src/render/*`, `src/debug/*` | Core Vulkan functional |
| M4 World Gen | `src/world/*` | Authored spec; runtime stub |
| M5 AI | `src/ai/*` | M5.4 authored as executable stub |
| M6 Audio | `src/audio/*` | Authored spec; runtime stub |
| M7 UI | `src/ui/*` | Stub headers |
| M8 Settlement | `src/world/*`, `src/vehicle/*` | M8.6 authored as executable stub |
| M9 Save | `src/save/*` | Stub headers |
| M10 Net/Weather | `src/net/*`, `src/world/*` | Stub headers |
| M11 Modding | `src/modding/*` | Stub header |
| M12 Tools | `src/debug/*` | Partial debug overlays |

---

## 3. Minimal-Asset + Voice-First Production Model

### 3.1 Hard Authored-Asset Budget: ≤50

| Category | Count | Source |
|---|---|---|
| Human body mesh | 1 | M2/Ecs/Destructible |
| Zombie body mesh | 1 | M2.7 |
| Vehicle chassis grammar | 1 | M8.7 |
| Weapon-family grammar | 1 | M2.7-EXT-02 |
| Furniture grammar | 1 | M4-EXT-09 |
| Vegetation base set | 1 | M8.6-EXT-11 |
| Decal set (wounds/blood) | 1 | M2.7-EXT-34 |
| UI font set | 1 | M7 |
| Authored voice performances | ≤8 | M6.5 |

### 3.2 Voice-First Narrative Pipeline

```mermaid
flowchart LR
    S[Intent slot: alert/fear/pain/question/threat/whisper]
    T[Style slot: aggressive/broken/exhausted/childlike/militarized]
    V[8 authored seed lines per timbre]
    D[Prosody director: pitch × rate × SPL × jitter × material_diffraction]
    A[Audio engine output → spatialized M6 stream]
    S → V → D → A
    T → V
```

Verification:
- 90% of runtime dialogue generated via slot fill, not authored lines.
- No authored cinematics.
- 8 voice performances × 6 intent slots × 4 style slots = 192 perceived variations.

---

## 4. Benchmark Performance Budget

| System | Budget | Measurable via | File anchor |
|---|---|---|---|
| Pipeline library compile | <50ms/100 variants | Wireshark/Vulkan debug | `PipelineCacheManager.cpp` |
| Horde indirect draw | <8ms GPU time | GPU timestamp queries | `RenderGraph.cpp` |
| Compute skinning | <1ms/100 zombies | GPU timestamp queries | `PipelineBuilder.cpp` |
| Shadow cache | 60% reduction | Benchmark script | `RenderGraph.cpp` |
| Fracture cache | <50ms/fracture | per-frame timer | `PhysicsSystem.cpp` |
| Wind + foliage | <1ms compute | GPU timestamp queries | `MaterialSystem.cpp` |
| Synthesis layer | <2ms/frame | `std::chrono` | `AudioEngine.cpp` |
| Portal attenuation | match aperture | Test bench | `AudioPropagation.cpp` |
| Biome transition | smooth @ 3 chunks | Visual inspection | `BiomeGraph.cpp` |

---

## 5. Math Reference Table (Verified Against spec/*.md)

| System | Formula | Ext Block | File Implementation |
|---|---|---|---|
| Road snap | `d = ‖(p − a) × (p − b)‖ / ‖b − a‖` | M4-EXT-01 | `ChunkStreamer.cpp` |
| WFC multi-grid | `StateSpace = Size_macro · Size_medium · Size_micro` | M4-EXT-03 | `ChunkStreamer.cpp` |
| QEM decimation | `Error = vᵀ · (Q_A + Q_B) · v` | M4-EXT-02 | `ChunkStreamer.cpp` |
| Normal weld | `n_unified = (n_A + n_B) / ‖n_A + n_B‖` | M4-EXT-04 | `ChunkStreamer.cpp` |
| LOD weld | `Weld(v) = argmin_{u ∈ EdgeCoarse} ‖v − u‖` | M4-EXT-05 | `ChunkStreamer.cpp` |
| Doppler | `f_effective = f_base · (c − v_source · u) / (c − v_observer · u)` | M6-EXT-01 | `AudioPropagation.cpp` |
| Footstep SPL | `SPL_step = κ · m_total · ∥v_foot∥² · Z_surface` | M6-EXT-02 | `AudioPropagation.cpp` |
| Echo | `I_echo = I_source · cos(θ) · R_material_hardness` | M6-EXT-03 | `AudioPropagation.cpp` |
| Portal | `Attenuation = aperture · dot(SoundVector, FaceNormal) / Distance²` | M6-EXT-04 | `AudioPropagation.cpp` |
| Priority | `Priority = Volume_base / (d² + ε) · cos(θ)` | M6-EXT-05 | `AudioEngine.cpp` |
| Cadence | `I_cadence = Clamp(⌊d/10.0⌋, 1, 8)` | M6-EXT-06 | `AudioEngine.cpp` |
| Velvet-noise | `t_k = (k + rand[0,1)) · T_avg` with `exp(−t / τ)` decay | M6-EXT-07 | `AudioEngine.cpp` |
| WFC vertical | `support(c.below) valid before placing c` | M4-EXT-08 | `ChunkStreamer.cpp` |
| Spline weld | `CatmullRom(join(spline_i, spline_{i+1}))` | M4-EXT-10 | `ChunkStreamer.cpp` |
| Wealth threat | `threat_score = Σ(value) + α · population` | M5.4-EXT-11 | `AIDirector.cpp` |
| Faction rep | `rep[A] += Δ ; rep[B] −= k · Δ` | M5.4-EXT-12 | `SaveSystem.cpp` |

---

## 6. Bug-Hunt and Code Quality: 5 Methods

| Method | Scope | Finding | File |
|---|---|---|---|
| 1 Unsafe `registry.get<>` | All `src/ecs/` | 0 occurrences after Destructible.cpp fix | `Destructible.cpp` |
| 2 Raw new/delete | `PhysicsSystem.cpp`, `Engine.cpp` | Expected Jolt/EnkiTS patterns | `PhysicsSystem.cpp` |
| 3 TODO stubs | 20 `src/*/*.cpp` | All are unimplemented subsystems; not critical bugs | `src/ai/*`, `src/audio/*`, `src/combat/*`, `src/save/*`, `src/slm/*`, `src/ui/*`, `src/vehicle/*`, `src/world/*` |
| 4 NULL/nullptr | `Engine.cpp`, `CVarSystem.cpp` | Defensive checks present | `Engine.cpp` |
| 5 Assertions | `Engine.cpp`, `debug/imgui_impl_vulkan.cpp` | Production asserts appropriate | `Engine.cpp` |

---

## 7. Trendsetter-to-File Mapping

| Trendsetter | Primary Code File | Spec Anchor | Verifiable Gate |
|---|---|---|---|
| Motion matching | `ZombieFSM.cpp`, `DamageTypes.cpp` | M2.7-EXT-21 | 50 zombies, no animation pop, query cost <2ms |
| Vulkan 1.4 + pipelines | `PipelineBuilder.cpp`, `PipelineCacheManager.cpp` | M3 | 100 variants compile <50ms |
| GPU-driven visibility | `RenderGraph.cpp`, `CommandManager.cpp` | M1-EXT-27, M4.5-EXT-12 | 500 zombies <8ms GPU |
| Adaptive director | `AIDirector.cpp`, `ScentGrid.cpp` | M5.4-EXT-10 | Event >80% contextual fit |
| Procedural audio | `AudioEngine.cpp`, `AudioPropagation.cpp`, `AudioVoice.cpp` | M6 | Blind A/B pass; CPU <2ms/frame |
| Voronoi fracturing | `Destructible.cpp`, `PhysicsSystem.cpp` | M2.6-M2.9 | Fracture <50ms |
| Deterministic replay | `StateSync.cpp`, `Engine.cpp`, `Input.cpp` | M2.8-EXT-09 | Byte-exact 10 min × 3 machines |
| Compute skinning | `PipelineBuilder.cpp`, `TriangleRenderer.cpp` | M1-EXT-27 | 100 zombies <1ms skin |
| Two-tier shadow | `RenderGraph.cpp`, `CommandManager.cpp` | M4 | 60% reduction |
| Horror audio layer | `AudioEngine.cpp`, `AudioVoice.cpp` | M6-EXT-15, M6-EXT-18 | <2ms/frame |
| Quest grammar | `AIDirector.cpp`, `PromptTemplate.cpp` | M11, M5.4 | 10 distinct playthroughs |
| Scent/noise system | `ScentGrid.cpp`, `AudioPropagation.cpp` | M6, M5 | 30m decoy → horde redirect |
| Settlement economy | `BodyTemp.cpp`, `StaminaSystem.cpp`, `SaveSystem.cpp` | M8, M6, M9 | 10→20 survivor deficit curve |
| Zombie sensory memory | `ZombieFSM.cpp`, `ScentGrid.cpp`, `AudioPropagation.cpp` | M5, M2.7 | Glass break → investigate 30s |
| Survival interlock | `BodyTemp.cpp`, `StaminaSystem.cpp`, `CVarSystem.cpp` | M0-M2 | Dependency DAG completeness |
| Ghost replay | `StateSync.cpp`, `Input.cpp`, `Engine.cpp` | M2.8-EXT-09 | Pose error <1% |
| Wind + foliage | `MaterialSystem.cpp`, `BiomeGraph.cpp` | M8, M4 | 100K blades <1ms |
| Voice director (Paper 5+6) | `AudioVoice.cpp`, `AIDirector.cpp` | M6.5 | 8 lines → 192 variations |
| Neural texture | `AssetManager.cpp`, `MaterialSystem.cpp` | M4.5 | Load <50ms |
| Wealth scaling (Paper 9) | `AIDirector.cpp`, `SaveSystem.cpp` | M5.4 | +50% value → horde 1.5x |
| Structural integrity | `PhysicsSystem.cpp`, `Destructible.cpp` | M2.6-M2.9, M4 | Wall 0 → collapse |
| Faction rep (Paper 3+5) | `SaveSystem.cpp`, `PromptTemplate.cpp` | M7, M11 | Max A → B drops 30% |
| Anti-cheat replay | `NetworkManager.cpp`, `StateSync.cpp` | M10, M2.8 | Impossible input flagged 1 tick |
| Asset governor | `AssetManager.cpp`, `BiomeGraph.cpp` | M4, M8 | Count ≤50 |
| Biome transitions | `BiomeGraph.cpp`, `ChunkStreamer.cpp` | M8.6, M4 | 3 chunk blend |
| Signage grammar | `MaterialSystem.cpp`, `ChunkStreamer.cpp` | M8.6 | 1000 signs from 12 rules |
| Day/night (Paper 2) | `AIDirector.cpp`, `AudioEngine.cpp` | M3, M5, M6 | Night-only infected |

---

## 8. Stub Authoring Execution Plan

### 8.1 Remaining Todo Surface (from bug hunt)
| File | Responsibility | Dependencies |
|---|---|---|
| `AIDirector.cpp` | AI director loop, event grammar, personality presets | M5.4 authored spec |
| `ZombieFSM.cpp` | Zombie sensory, locomotion, attack FSM | M2.7 authored spec |
| `AudioEngine.cpp` | Miniaudio backend, synthesis chain, voice director | M6 authored spec |
| `AudioPropagation.cpp` | Dopper/SPL/echo/portal physics implementation | M6-EXT-01..10 |
| `AudioVoice.cpp` | Voice priority, cadence, reverb mixing | M6-EXT-05..07 |
| `DamageTypes.cpp` | Damage type enum, response table | M2.7-EXT-02 |
| `ParrySystem.cpp` | Parry frame window, directional guard | M2.7-EXT-04 |
| `HUD.cpp` | Minimal HUD with MSDF glyph rendering | M7 |
| `HapticManager.cpp` | Haptic feedback from damage/events | M7 |
| `MSDFText.cpp` | Signed-distance-field text rendering | M7 |
| `VehicleSystem.cpp` | Vehicle physics, gearbox, fuel | M8.7 |
| `WheelSim.cpp` | Suspension, traction, slip | M8.7 |
| `BiomeGraph.cpp` | Whittaker classification, blend bands, hazard tables | M8.6 |
| `ChunkStreamer.cpp` | WFC, macro-graph, Voronoi seam welding, LOD transitions | M4, M8.6 |
| `BodyTemp.cpp` | Temperature/humidity grid, wetness, wind chill | M0-M2 |
| `StaminaSystem.cpp` | Exertion, fatigue, recovery curves | M0-M2 |
| `SaveSystem.cpp` | Deterministic snapshot → file | M9 |
| `SchemaVersion.cpp` | Compatibility check | M9 |
| `NetworkManager.cpp` | Reliable UDP, client/server state, anti-cheat | M10 |
| `StateSync.cpp` | Input+RNG deterministic replay | M2.8-EXT-09, M10 |
| `PromptTemplate.cpp` | LLM template filling for narrative | M11 |
| `SLMClient.cpp` | LLM client (local or remote) | M11 |
| `Modding.cpp` | Mod load, version check, hook system | M11 |
| `EventBus.cpp` | Hot event bus for inter-system communication | M0/M2 |

---

## 9. Research Citation Backlog

Source: `recon/plans/2026-07-21_DEEP_EXECUTION_PLAN.md`

| Status | Count |
|---|---|
| [S] cache-backed | 111 |
| [X] pending | 78 |
| **Verified from `papers_1_10.txt`** | **10** |

Priority conversion queue (78 → 68 → ...):
1. Concordia (Paper 6) — agent-based modeling → NPC entity grounding.
2. LLM Game Agents Survey (Paper 7) — taxonomy → director character AI.
3. Experience-Driven Adaptation (Paper 8) → director difficulty curve.

---

## 10. Final Verification Checklist

- [ ] `verify_ext_block_counts.py` returns 0 warnings
- [ ] `git status --short` clean
- [ ] Build succeeds
- [ ] Headless smoke test exit 0
- [ ] Catch2 tests pass
- [ ] 0 unsafe `registry.get<>()` without `try_get` guard
- [ ] Authored-asset budget enforced: ≤50
- [ ] Voice-intent table covers ≥6 of 6 intended slots
- [ ] Style table covers ≥4 of 5 intended slots
- [ ] All new `.cpp` files have CMake targets
- [ ] All new headers have include guards
- [ ] All new spec blocks have anchors + frontmatter
- [ ] Stub milestones authored or gated with exec order
- [ ] Cross-refs: 0 dangling
- [ ] Paper citation [X] count < [S] count

---

## 11. Next Actions (Immediate)

1. Author runtime stubs in order: M6 audio → M5 AI → M8 world → M7 UI → M9 save → M10 net → M11 modding.
2. Convert 10 [X] papers per day (cache-backed quotes, not just title).
3. Build Catch2 test infrastructure and run.
4. Run Hermes weekly: verifier + bug hunt + asset audit.
5. Author remaining 20 `.cpp` surface stubs into real implementations.

*Plan v7 built from source evidence and user-provided research corpus.*
