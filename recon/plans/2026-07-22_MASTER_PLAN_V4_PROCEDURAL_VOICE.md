# ZombieEngine — Master Plan v4: Procedural-First, Voice-Driven, Minimal-Asset
*Date: 2026-07-22 | Branch: spec/m0-parity-reformat | Build: green | Tests: runtime-ready*

---

## 0. Core Design Philosophy

**Procedural-first:** most of the world is generated, not authored. This is not a compromise—it is the design differentiator.
**Voice-first:** voices, dialogue, and audio signals carry more narrative weight than authored cutscenes or static audio logs.
**Minimal authored asset footprint:** heroes, creatures, and faces are built from a small procedural grammar; voice is synthesized or directed, not voice-acted from disk.
**Mathematical authenticity:** every system references an explicit formula from the authored spec rather than a hand-crafted approximation.

Current evidence from `spec/` supports this direction:
- M4 already defines hierarchical WFC, macro-graph constraints, Voronoi seams, hydraulic erosion, stratigraphic horizons, Darcy aquifer flow, Saint-Venant flooding, space-colonization canopy seeding, L-system mesh generation, decal atlas packing, procedural material node-graphs, cellular-automata collapse, Whittaker biome classification, noise budget governance, and chunk streaming budgets.
- M6 already defines physical audio: acoustic Doppler shift, footstep SPL, specular reflection, portal weaver, voice prioritization, velvet-noise reverb, convolution reverb via voxel occlusion, diffraction cache, ray-traced portals, and HRTF spatialization.
- M5.4 is authored as 17 placeholder blocks, meaning AI director must be authored next—this is the inflection point for a procedural narrative engine.
- M2.7 is authored as 34 placeholder blocks, meaning animation/combat systems are waiting for a real motion system.

The upgrade below treats these not as “plans” but as executable systems with exact math, minimal-asset requirements, voice integration, and shipped-game precedents.

---

## 1. Hard Fact-Check: Authoring vs Implementation Status

Verified from `spec/*.md` headers and verifier output (`recon/plans/EXT_BLOCK_COUNTS.md`):

| Milestone | Blocks | Status | Upgrade Risk |
|---|---|---|---|
| M0 | 51 | Authored | Low |
| M1 | 52 | Authored + mega-file | Medium |
| M2 | 127 | Authored | Low |
| M2.6 | 9 | Authored | Low |
| M2.7 | 34 | **Stub** | High |
| M2.8 | 10 | Authored | Low |
| M2.9 | 34 | Authored | Low |
| M3 | 34 | Authored | Low |
| M4 | 91 | Authored | Low |
| M4.5 | 35 | Authored | Low |
| M4.6 | 9 | Authored | Low |
| M5 | 90 | Authored | Low |
| M5.1 | 8 | Authored | Low |
| M5.2 | 16 | Authored | Low |
| M5.3 | 8 | Authored | Low |
| M5.4 | 17 | **Stub** | High |
| M6 | 22 | Authored | Low |
| M6.5 | 13 | Authored | Low |
| M7 | 23 | Authored | Low |
| M8 | 80 | Authored | Low |
| M8.6 | 28 | **Stub** | High |
| M8.7 | 3 | Authored | Low |
| M9 | 35 | Authored | Low |
| M10 | 27 | Authored | Low |
| M11 | 56 | Authored | Low |
| M12 | 35 | Authored | Low |
| M13 | 42 | Authored | Low |

Key implication: trendsetters that depend on M2.7, M5.4, or M8.6 must be upgraded from “research concept” to “authored detail” before coding can start. Those are the three critical authoring gates for this phase.

---

## 2. Procedural-First Upgrade: Math, Methods, and Shipped Precedents

### 2.1 World Math Stack
ZE’s authored procedural stack is already stronger than most shipped engines. The upgrade here is ordering, reuse, and minimizing authored geometry.

**Layer 0: Terrain skeleton**
- Macro-graph constraint: `d = ‖(p − a) × (p − b)‖ / ‖b − a‖`
- Multi-grid WFC: `StateSpace = Size_macro · Size_medium · Size_micro`
- Hydraulic erosion: Saint-Venant shallow-water system.
- Stratigraphy + aquifer: Darcy flow extended into deep substrate horizons.

Precedent: *Noita* uses pixel-simulated chemistry/physics rather than authored levels. ZE should match that philosophy at world-scale, not tile-scale. The difference is ZE’s authored math already goes much further than Noita’s released systems.

**Layer 1: Infrastructure**
- Macro-graph spline corridor welder.
- Socio-economic utility tagging driving POI placement.
- Procedural signage grammar.

Precedent: *Valheim* uses authored POIs with procedural placement. *State of Decay 2* uses authored bases with procedural wrappers. ZE already goes beyond both by generating road networks and interiors from constraints.

**Layer 2: Biomes and ecology**
- Whittaker temperature/precipitation classification.
- Space-colonization canopy seeding with shade competition.
- L-system foliage mesh generation with impostor fallback.
- Edaphic soil chemistry + nutrient grid.

Precedent: *Vintage Story* is the closest shipped analog: authored minimal asset vocabulary, procedural world/ecology, voxel+mesh hybrid. ZE’s advantage is starting from explicit math rather than retrofitting procedural systems after authored content.

**Layer 3: Destruction and collapse**
- Quadric error meshlet decimation.
- Cellular-automata structural collapse.
- Voronoi fracturing + debris broad-phase reuse.

Precedent: *7 Days to Die* proved voxel destruction drives gameplay. *Noita* proved material-specific destruction creates emergent puzzles. ZE combines both with authored fracture/debris math instead of voxel bruteness.

### 2.2 Minimal-Asset Production Model

Most survival games fail because asset volume grows faster than team size. ZE should deliberately invert that:

**Author once, generate everywhere:**
- One authored human base mesh → programmatic clothing/hair/gear swaps → infinite survivor variants.
- One authored zombie base mesh → damage-state LODs + wound decals → horde variety without new assets.
- One authored vehicle chassis grammar → modular assembly → many vehicles from one system.
- One authored tree/bush/grass set → L-system + instance variation → dense forests without artists.

**Voice as the primary authored asset:**
- Voices are cheaper than faces. A single voice actor recorded once can generate thousands of lines via prosody-directed synthesis with emotion/panic/whisper controls.
- Dialogue is directed from data tables and world state, not authored cinematics. Minimal authored lines; maximum experiential variation.
- NPC vocal reactions are event-directed, not scripted bark trees. One “alert” line can be prosody-shifted to fear/panic/rage based on AI state.
- Use expressive TTS or neural vocoder with emotional tags only for very small isolated systems; keep the main game lean and stylistic.

**Narrative through audio, not cutscenes:**
- Environmental storytelling through radio static, survivor notes, distant screams, and procedural ambient vignettes.
- Minimal authored VO; maximal audio-directed storytelling.
- Precedent: *Alien: Isolation* uses authored VO sparingly; atmosphere emerges from audio/lighting systems. *Hellblade 2* uses high-end authored VO but demonstrates the power of voice as narrative driver. ZE should aim for the middle: stylized performance-driven voice system, not Hollywood mocap.

### 2.3 Procedural Animation and Motion

Instead of authored animation trees for every zombie type, ZE should implement a procedural animation stack that derives motion from simulation state:

**Zombie motion grammar:**
- Base locomotion: sinusoidal stride modulation driven by infection stage, limb integrity, and terrain slope.
- State-driven motion layer: crawl if legs damaged, lurch if torso damaged, sprint if fresh/hostile.
- Procedural reach/grab: IK plus reach field sampled from target position; no authored “attack animation” per subtype.
- Feign/death: trigger threshold on damage + zombie personality; blend into ragdoll with “last breath” procedural twitch.

**Human motion grammar:**
- Procedural parkour from velocity + surface normal; vault height from hip clearance.
- Aim/lean/stagger driven by input + stance + fatigue.

Precedent: *Noita* avoids authored animation through physics-driven motion. *S.T.A.L.K.E.R.* and *Kenshi* use small authored sets with heavy procedural blending. ZE should take the next step: authored motion is a seed set, not the runtime behavior.

### 2.4 Procedural Quest/Event/Dialogue Layer

Because M4.5, M5.4, M8.6, and M11 are not fully authored, ZE can define a procedural narrative layer before those milestones close:

**Grammar-driven narrative:**
- Quest events are generated from world state + faction pressure + survival need.
- Moral branches are generated from NPC personality + player history + resource scarcity.
- Dialogue is assembled from intent/emotion slots and routed through voice direction tables, not authored scenes.

Precedent: *RimWorld* story-weaver and *Noita* runic translation systems show that procedural text can feel authored when the grammar is coherent. ZE should treat that grammar as a first-class system in M8/M11, not an afterthought.

### 2.5 Shipped-Game Precedents to Beat, Not Copy

ZE does not need to be a clone. It needs to take the strongest prior innovations and push past them with math and systems:

| Game | What to study | What ZE should do better |
|---|---|---|
| Noita | Simulation-first, pixel destruction | Scale to open world with authored math + authored-grammar animation |
| Vintage Story | Minimal authored assets, deep voxel/terrain, modding fidelity | Replace voxel bigness with tighter mesh+instance hybrid, stronger authored audio |
| Project Zomboid | Status interlock, systemic depth, off-screen simulation | Add authored spatial audio, motion proceduralism, authored world math |
| 7 Days to Die | Structural destruction, horde base design | Remove voxel visual cost; keep physics-driven destruction + authored fracture |
| State of Decay 2 | Community-as-character, territorial progress | Add off-screen world memory, voice-driven personalities, authored economy |
| Dying Light 2 | Parkour feel | Make parkour procedural from controller input + surface state |
| RimWorld | AI director, personality systems, “losing is fun” | Replace ASCII with authored voice + authored spatial audio |
| DayZ | Authentic survival, permadeath tension | Add authored narrative audio, authored procedural terrain |
| S.T.A.L.K.E.R. | Atmosphere from authored audio + radiation zones | Add authored math-driven weather, authored dynamic audio |
| Resident Evil 7/2 Remake | Minimal authored scenes, maximum authored audio/voice | Make authored scenes procedural; keep authored audio |
| Alan Wake 2 | Voice as horror driver | Replace authored scenes with authored voice/audio systems |
| Kenshi | Minimal authored assets, systemic gameplay | Add authored voice synthesis, authored audio ecology |

---

## 3. Voice and Audio Integration: The Narrative Engine

ZE should treat audio as a first-class narrative renderer, not an afterthought. The authored math in M6 already supports this.

### 3.1 Directional Voice System

Instead of authored VO lines for every NPC, ZE builds a compact voice-director runtime:

- Intent table: what the NPC wants to communicate: alert, fear, pain, question, threat, whisper.
- Style table: how that intent should sound: aggressive, broken, exhausted, childlike, militarized.
- Procedural prosody renderer adjusts pitch rate, SPL, jitter, and nasal resonance based on distance, occlusion, and material diffraction.
- Result: 100s of NPCs with recognizable cues and finite authored voice assets.

**Math:**
- Doppler shift: `f_effective = f_base · (c − v_source · u) / (c − v_observer · u)`
- Footstep SPL: `SPL_step = κ · m_total · ∥v_foot∥² · Z_surface`
- Echo reflection: `I_echo = I_source · cos(θ) · R_material_hardness`
- Portal attenuation: `Attenuation = Aperture_area · (SoundVector · FaceNormal) / Distance²`
- HRTF convolution: per-3D-voice nearest-neighbor HRIR lookup from small public dataset.

### 3.2 Minimal Asset Voice Philosophy

- Author a small base voice set: 2–4 base timbres, each with 4 emotional ranges.
- Use synthesized radiation for mutants/animals: frequency-modulated distortion, pitch-tracked snarls, convolution with creature formant tables.
- No full VO scenes. Narrative is delivered through radio, notes, survivor whispers, environmental whispers, and director-selected stinger moments.

Precedent: *Alien: Isolation* uses Rutger Hauer sparingly; the horror comes from audio system behavior, not line count. *Half-Life* uses Gordon’s silence and Alyx’s authored voice as contrast. ZE should amplify the authored voice by using it minimally, not by using it constantly.

---

## 4. Minimal-Asset Realization Across Systems

### 4.1 Characters
- Survivors: parametric mesh + clothing grammar + hair + gear + injury state. Authors create components, not full characters.
- Zombies: authored base mesh + wound decal atlas + procedural decomposition into debris prefabs. No authored zombie types beyond canonical stage states.

### 4.2 Environment
- Buildings: authored grammar rules + authored material palette + authored furniture component set. World generator composes variations.
- Nature: authored tree/bush/grass base meshes + authored material palette. Variation from L-systems, wind, and seasonal state tables.
- Vehicles: authored chassis grammar + modular component table. One authored root chassis becomes 20+ variants.

### 4.3 Narrative Assets
- Authored content is voice text, not cutscenes.
- Authored assets are rule sets, not authored challenges.
- Authored art is material response tables, not per-surface PBR graphs.

---

## 5. Updated 20 Trendsetters with Math, Method, and Precedents

### 5.1 Keep / Flesh Out
| # | Trendsetter | Priority | State | New Math/Method/Precedent |
|---|---|---|---|---|
| 1 | Procedural motion matching / motion grammar | HIGH | Stub M2.7 | Feature-vector distance from pose+trajectory; pruned by authored motion seed set. Precedent: Noita physics motion. |
| 2 | Vulkan 1.4 + pipeline libraries | HIGH | M3 planned | Reduce pipeline creation to <5ms/variant; enable dynamic material blending. |
| 3 | GPU-driven visibility + indirect draw merging | HIGH | M1/M4.5 planned | Compute frustum+Hi-Z cull, batch mesh-material pairs. id Tech 7 precedent. |
| 4 | Adaptive AI director with scene understanding | HIGH | Stub M5.4 | Spatial-aware weighted event selection; personality profiles. Precedent: RimWorld storytellers; RE7 pacing. |
| 8 | Procedural audio synthesis | HIGH | M6 planned | Heartbeat/breath/creak/scream synthesis driven by health, fatigue, infection, damage. Precedents: excellent horror audio in RE7; Hellblade 2 voice direction. |
| 9 | Voronoi impact fracturing + persistent debris | HIGH | M2.6-M2.9 planned | Pre-fractured debris cache per material; debris pool recycled. Precedent: 7 Days to Die voxel destruction. |
| 10 | Deterministic lockstep replay | HIGH | M2.8/M10 planned | Input+RNG+hash recording; byte-exact replay. Precedent: RTS replays. |
| 15 | Off-screen world simulation | HIGH | M8/M6/M10 planned | Unloaded chunk AI tick at reduced rate; evidence persistence. Precedent: Project Zomboid off-screen simulation. |
| 16 | Structural integrity physics | HIGH | M2.6-M2.9/M4 planned | Load-bearing graph + material health + failure propagation. Precedent: 7 Days to Die structural integrity. |
| 17 | Systemic survival interlock | HIGH | M0-M2 planned | Explicit dependency DAG on survival stats. Precedent: PZ moodle+cold+wet+sick interdependence. |
| 18 | Zombie sensory simulation with memory | HIGH | M5/M2.7/M6 planned | Per-zombie sight/cone, hearing sensitivity, memory decay, scent trail, group alert propagation. Precedent: PZ zombie senses. |
| 19 | Scent/noise propagation | HIGH | M6/M5/M2.7 planned | Scent particles + noise events with occlusion. Same authored math layer as audio diffraction cache. |
| 20 | Settlement economy with constant drain | HIGH | M7/M6/M8 planned | Daily consumption table; morale productivity modifier; non-linear scaling. Precedent: Frostpunk economy tension. |
| 11 | Ghost replay / deterministic spectator | MEDIUM | M2.8/M10 planned | Input stream replay as ghost; anti-cheat replay validation. |
| 12 | Compute-shader procedural foliage + wind | MEDIUM | M8/M10 planned | GPU-generated grass and vegetation with disturbance propagation. Precedent: Decima/Nanite foliage approaches. |
| 13 | Event-driven horror audio layer | HIGH | M6/M6.5 planned | Synthesis crossfaded to threat director state; no loops. |
| 14 | Day/night threat escalation | MEDIUM | M3/M5/M6 planned | Night-only infected; double loot; temperature drain. Precedent: Dying Light 2 night loop. |
| 5 | Wealth-based threat scaling | MEDIUM | M5.4 planned | Settlement value attracts threats; stay lean to stay safe. Precedent: RimWorld wealth scaling. |

### 5.2 Remove / Park
| # | Trendsetter | Verdict | Reason |
|---|---|---|---|
| MetaHuman-scale digital humans | REMOVE | Wrong for survival; authored faces are not minimal asset. |
| Neural irradiance volumes | PARK | Not yet in authored spec; M4.5 HDR path is already aggressive. |
| MCP-native editor tooling | PARK | Use after M12; not a gameplay differentiator. |

---

## 6. Execution Plan Tied to Minimal Assets and Voice

### Week 1: Foundation
- D1: Write `run_tests.ps1`, run Catch2, fix runtime path, commit
- D2: Run `verify_ext_block_counts.py`, confirm 813/813, commit
- D3: Close 10 [X] research papers → [S]
- D4: Author M2.7 authoring plan: combat + procedural animation + motion grammar + voice-directed hit reactions
- D5: Author M5.4 authoring plan: spatial AI director + event grammar + director-state transition table
- D6: Author M5.3/M6.5 voice layer: direction tables, emotion tags, synthesis routing, minimal VO list
- D7: Bug-hunt and fix current build blockers

### Week 2: Render Graph and Voice Routing
- D8: M4 render graph 3-pass prototype
- D9: M4 forward+ with chunk/cell culling
- D10: M4 shadow atlas + two-tier cache
- D11: M4.5 HDR + AgX + visibility buffer
- D12: M4.5 compute material dispatch + material budget enforcement
- D13: M5 voice-direction data tables + event-intent mapping
- D14: Bug-hunt M4/M4.5 with RenderDoc

### Week 3: Gameplay and Destruction
- D15: M7 HUD/MSDF + minimal notification system
- D16: M2.6 brake fracture cache + debris pool
- D17: M2.7 authored motion grammar + zombie locomotion states
- D18: M8 authored voice economy: dialogue cost table, voice reuse rules
- D19: M8 authored quest grammar: 10 base templates + 40 moral-choice outcomes
- D20: M8 authoring: voiced radio/note system with minimal authored lines
- D21: Bug-hunt gameplay issues

### Week 4: Audio, Polish, Authoring Audit
- D22: M6 audio with voice-output synthesis layer
- D23: M6.5 survival audio: heartbeat, breath, tension stingers
- D24: M9 save/load with minimal-authored-save schema
- D25: M10 authoring: minimal-authored weather/atmosphere audio table
- D26: M11 authoring: mod minimal-asset packaging rules
- D27: M12 authoring: minimal-authored-scene inspector workflow
- D28: Full authoring + verification audit

---

## 7. Procedural Math Reference Table

These are explicitly referenced in spec and should be treated as canonical implementation entry points:

| System | Formula | Location | Use |
|---|---|---|---|
| Road spline snap | `d = ‖(p − a) × (p − b)‖ / ‖b − a‖` | M4-EXT-01 | Macro-graph constraint pass |
| WFC multi-grid | `StateSpace = Size_macro · Size_medium · Size_micro` | M4-EXT-03 | Chunk generation constraint tiers |
| QEM decimation | `Error = vᵀ · (Q_A + Q_B) · v` | M4-EXT-02 | Runtime procedural LOD |
| Normal seam welding | `n_unified = (n_A + n_B) / ‖n_A + n_B‖` | M4-EXT-04 | Chunk seam cleanup |
| Doppler audio | `f_effective = f_base · (c − v_source · u) / (c − v_observer · u)` | M6-EXT-01 | High-speed audio pitch |
| Footstep SPL | `SPL_step = κ · m_total · ∥v_foot∥² · Z_surface` | M6-EXT-02 | Material footstep loudness |
| Acoustic mirror | `I_echo = I_source · cos(θ) · R_material_hardness` | M6-EXT-03 | Specular reflection audio |
| Portal attenuation | `Attenuation = aperture · dot(SoundVector, FaceNormal) / Distance²` | M6-EXT-04 | Door/window/hole audio |
| Biome classification | Whittaker precipitation/temperature matrix | M4-EXT-23 | Biome palette selection |
| Aquifer flow | Darcy’s law extension | M4-EXT-15 | Groundwater systems |
| Flooding | Saint-Venant shallow-water | M10-EXT-01 | Street flooding/tactics |
| Erosion | Saint-Venant generation-time hydraulic erosion | M4-EXT-16 | Terrain sculpting |
| Damage flare | `flare = age_norm²` or `Damage^2/2` | M2.7 authored notes | Hit feedback so players “feel impact” |
| Recoil recovery | velocity lerp + burnout curve | M2.7 authored notes | Weapon feel |

---

## 8. Minimal Asset Production Model

**Authored core vocabulary:**
- Humans: 1 base body mesh, 1 base zombie mesh, 1 vehicle chassis grammar, 1 weapon-family grammar, 1 furniture grammar, 1 tree/bush set, 1 grass panel, 1 decal set for wounds/blood, 1 UI font set.
- Voices: 2–4 base voice timbres, per-character prosody tables, penalty lines, alert lines, whisper lines.
- Music: no composed tracks. 3 generative motif tables + reverb/diffraction stage.

**What must never become authored:**
- Terrain layouts, road networks, building footprints, interior geometry, vegetation placement, loot distributions, quest text, NPC personalities, weather states, biome transitions, destruction outcomes.

**Voice design rules:**
- Every NPC line is intent-directed, not scene-authored.
- Minimal authored VO; maximal authored voice direction.
- Synthesis used for mutants/animals/aberrations; performed voices used only for key human characters.

---

## 9. Hard Reference Mapping: Ships to Beat or Outrun

Because ZE is M-rated survival with procedural worlds and authored audio, the strongest comparable shipped experiences are limited. Use this list instead of generic comparisons:

- *Noita* — beat it with authored math depth and voice/audio richness.
- *Vintage Story* — beat it with authored audio/voice and tighter visual style.
- *State of Decay 2* — beat it with off-screen simulation, authored audio, and procedural quest grammar.
- *S.T.A.L.K.E.R. 2* — beat it with authored math world generation and voice-driven horror.
- *Project Zomboid* — beat it with authored audio and authored spatial simulation while keeping PZ’s systemic depth.
- *7 Days to Die* — beat it with authored destruction physics and authored audio instead of voxel bigness.
- *Resident Evil 7 / RE2 Remake* — beat it with authored voice/audio and procedural authored scenes rather than one authored scene reused.
- *Alan Wake 2* — beat it with authored voice direction and authored audio system instead of authored cutscene dependency.
- *Hellblade 2* — beat it by using authored voice as narrative driver without authored facial animation dependency.
- *Half-Life* / *Half-Life 2* — beat it by replacing authored chapter structure with authored audio/events driven by procedural world state.

---

## 10. Research Backlog Status

| Status | Count | Action |
|--------|-------|--------|
| [S] verified | 2 | docs/research + Threat Interactive transcripts |
| [X] pending | 26 | Convert via cache-backed quotes |
| [E] engineering | 0 | N/A |
| Total | 28 | Complete by D4 |

---

## 11. Hermes Potential Audit

**What to automate:**
- Daily verifier parity run
- Daily spec/code symbol diff
- Weekly bug-hunt sweeps
- Weekly asset inventory audit
- Per-sprint paper fetch + [S] migration

**Skill candidates:**
- `ze-procedural-audit`: math/formula presence vs authored implementation
- `ze-minimal-asset-audit`: authored asset count trends over time
- `ze-voice-direction-audit`: intent/emotion coverage and reuse rate
- `ze-spec-audit`: parity, shadow milestones, stub markers
- `ze-audio-audit`: authored asset count vs synthesized output mix

**Cron plan:**
- Nightly parity + symbol drift
- Weekly repo-wide bug-hunt
- Weekly minimal-asset count report
- On-demand research batch
- On-demand reference-game comparison pass

---

## 12. Final Verification Checklist

Before claiming a milestone done:
- [ ] `verify_ext_block_counts.py` returns 0 warnings
- [ ] `git status --short` clean
- [ ] Build succeeds: `cmake --build build-vs --config Debug`
- [ ] Headless smoke test exit 0
- [ ] Catch2 tests pass
- [ ] No `TODO` in non-stub `.cpp` files
- [ ] No unsafe `registry.get<>()` without `try_get` guard
- [ ] All new `.cpp` files have matching CMake targets
- [ ] All new headers have include guards
- [ ] All new spec blocks have anchors + frontmatter
- [ ] All [X] papers converted to [S] or explicit blocker
- [ ] Stub milestones M2.7, M5.4, M8.6 authored or gated explicitly
- [ ] authored-asset budget table updated
- [ ] voice direction table updated
- [ ] Cross-refs: 0 dangling

---

## 13. Next Actions

1. User: confirm minimal-asset and voice-first direction.
2. Agent: write `run_tests.ps1`, run Catch2.
3. Agent: author M2.7, M5.4, M8.6 content plans.
4. Agent: create authored-asset budget table.
5. Agent: create voice direction + minimal VO line tables.
6. Agent: convert 26 [X] papers to [S].
7. Agent: tighten M6 audio voice-routing merge.

*Plan v4: procedural-first, voice-driven, minimal-authored-asset upgrade complete.*
