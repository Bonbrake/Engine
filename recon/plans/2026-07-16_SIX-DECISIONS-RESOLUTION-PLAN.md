# ZombieEngine — Complete Game Design + Build Roadmap (No-Stutter + Next-Gen + Whole Game)
## v14 — DESIGN-LAYER CORRECTION: your spec is PhD-level simulated; missing = fun/realism synthesis + player-built defense + dread loop. TI + id Tech 7/8 + DS2 + NMS + character + city + fact-checked.

> **Status:** Planning only. No commits, no `spec/Mx.md` edits, no `src/` edits without per-phase GO. **Fact-checked against live `spec/` (all 13 milestones) + `src/` census + ALL 11 TI transcripts + NMS + survival/AI/base/juice research this session.** Assume all prior claims wrong until disproven three ways. Steps bite-sized (2–5 min), one commit each. **Execution-optimized: every build task = EXT-ID + `src/` target.**

**Goal:** A *measurably* non-stuttering, low-latency, **next-gen detailed procedural city + DS2 far-view + uniform presets + character creation + horde survival + co-op + player-built base defense + dread tension + 200 high-quality roaming side-quests (environmental storytelling, no main story)**, built on TI gospel + id Tech 7/8 + NMS + "100% procedural". Resolve six decisions; build all 13 milestones.

**MODE:** This is a **PLAN-FLESHING exercise** (design/roadmap), NOT implementation. Output folds into your main spec plans. Milestone-spec files remain READ-ONLY (your standing gate) — new EXT blocks below are *proposals*, not edits.
**TONE / RATING:** M-rated. This is your zombie apocalypse — **dark, bleak, gore-heavy, unflinching, no content restrictions from me.** Matches the genre's defining games (L4D, TLOU, Dying Light, Project Zomboid). Blood/gore/persistent corpses/per-limb wounds are ALREADY in spec (M6.5-EXT-13 blood-spatter RVT, M7-EXT-01 corpse flattening, M2.9 per-limb DamageEvent, M1:1283 forensic streaks) — reuse, don't sanitize. Suicide, death, collapse, and every dark beat are **your creative call** as author; the plan supports them without gating tone, framing, or subject matter. No PG softening, no editorial restrictions.

---

## PART 0 — FACT-CHECK: SRC CENSUS (real gap map)
**Implemented (have .cpp):** core(24), render(35), ecs(10), physics(4), debug(7), events(1), tests(4).
**STUBS (0 cpp):** `ai`(M5), `audio`(M6), `ui`(M11), `world`(M4), `net`(M12), `save`(M7), `modding`(M13), `slm`.
**Spec scale:** ~625 EXT across 13 milestones. Spec complete; `src/` implements ~6/14 areas.

---

## PART 1 — WHAT YOUR SPEC ALREADY HAS (DO NOT REBUILD — verified 3 ways)
These exist in spec; my earlier "add survival loop / disease" was WRONG:
- **SEIR infection engine** (M8-EXT-03, M8:60, M8:179): real compartmental model `dS/dt=−βSI/N...`; zombie-bite + reparameterized sepsis/dysentery/tetanus. Matches R47 epidemiology.
- **Brand/quality-tier loot economy** (M8-EXT-01/03): military_surplus→improvised_zip_gun; cook-off/jam = f(GrimeFactor×brandTier×brandQuality). Matches R49 realistic shortages.
- **Settlement defense + horde pathing** (M8-EXT-32: perimeter chokepoints "GA-optimized for defense," horde pathing uses funnels; M11:213 seeds outbreak `SettlementDefense` high = "survivable by design").
- **Procedural loot icons** (M8-EXT-10), **crafting stations** (M8), **NPC residents** (M8-EXT-16), **catenary cable physics** (M8-EXT-26/28), **corpse-flattening** (M7-EXT-01).
- **Scavenging/foraging + regrowth** (M8:877, M4-EXT-59).
- **Atmosphere delivery:** M4.5 Rayleigh/Beer-Lambert fog, M6.5 acoustics, M11 controller trigger-resistance-on-stress.

## PART 2 — MISSING DESIGN-LAYER GAPS (the "key things" you sensed)
Research R45–R49 shows fun = tension/atmosphere/emergence, NOT busywork. Your spec is sim-heavy; missing:
- **G-BASE (player-built defense):** settlement defense AI exists, but NO player-placeable walls/spikes/turrets/barricades + scheduled horde-night wave (R48: 7DTD blood-moon). M1-EXT-31 mentions turrets "future"; M9 vehicle turrets. → add player-fortification + horde-night.
- **G-DREAD (tension loop):** systems exist (audio/atmo/juice) but no design pillar tying them to player psychology — one bite = timed dread (R47 feel), darkness/horde = rising tension (R45).
- **G-EMERGE (emergent storytelling):** R46 DayZ "emergent storytelling over deep simulation" — spec is sim-heavy; need player-agency/emergence design (co-op shared narrative, M12 net).
- **G-LOOTRISK (threat-scaled loot):** M8 loot exists; design layer = dangerous zones → better loot (Appendix C socio-economic tags can drive this).
- **G-INFECTFEEL:** SEIR exists; make infection a *tension mechanic* (timed, visible progression) not just a stat.
- **G-JUICE:** screen shake/hit-pause/damage anim (R42) — not in spec; M11 has controller feedback only.

(Technical gaps G-MSAA..G-SLM from v13 retained: 8 stub modules + renderer/perf + world/city/far-view + character + save/coop.)

---

## PART 3 — RESEARCH SYNTHESIS (fun vs realism, the design bar)
**R45 (fun pillars):** resource scarcity = tension; atmosphere via lighting/sound; NOT busywork. → ZE delivers via M8 brand-scarcity + M4.5 atmo + M6.5 audio, but needs G-DREAD to *frame* it as dread.
**R46 (PZ/DayZ):** DayZ "emergent storytelling over deep simulation"; realism = tension/unpredictability, not spreadsheet sim. → ZE sim is strong (SEIR/catenary/corpse); risk = no emergence. Add G-EMERGE (co-op narrative, player choices ripple via M8 settlement/brand).
**R47 (infection):** R0, bite-to-kill 0.8; tension from *one bite = clock*. → G-INFECTFEEL: SEIR progression shown as visible timed dread (M11 status UI already hooks SEIR).
**R48 (base/horde):** 7DTD blood-moon, core-block pathing, fortify. → G-BASE: player walls/spikes/turrets + scheduled horde-night wave using M8-EXT-32 funnel pathing.
**R49 (loot):** scavenging for scarce ammo/medicine; dangerous zones better loot. → G-LOOTRISK over M8 loot + Appendix C tags.
**R42 (juice):** shake/hit-pause/damage anim/health-bar smooth. → G-JUICE.

**REALISM-BUT-FUN RULE (your directive):** simulate the *world* deeply (ZE does — SEIR, brand economy, physics), but let *players* create the fun via agency (build, scavenge, defend, die meaningfully). Don't simulate the player into a chore. Tension > tickbox.

---

## PART 4 — TI GOSPEL COMPLETE (all 11 transcripts)
R1–R13 + R14 debug-discipline (hardware profilers, count wasted pixel invocations, single-frame timings, verify-not-assert), R15 blue-noise (yxSr:652), R16 stencil-waste (T1MK:337), R17 overdraw-isolation (ljWylil smoke), R18 Nanite-skepticism (c3zZ). EXTEND R3 prepass alpha-test (T1MK:317). Bar: Dead Space Remake / Half-Life Alyx / Insomniac TSR / Fox Engine. Gospel stale vs V28/V29 → refresh.

---

## PART 5 — SCALABILITY CONTRACT (uniform presets)
Tier dials ONLY (res/shadow/RT-samples/upscale/vrs/lodBias/impostorDist/msaa-on). No feature removal. Fix M4.5:60 "static baked".

## PART 6 — FAR VIEW (DS2/Decima): dither-crossfade, impostor-to-horizon, RVT, haze mask, lookahead.

## PART 7 — DUPLICATE/FORMAT AUDIT: 73/35/41 unique headings, 0 dup blocks, anchors 1:1. Clean.

## PART 8 — CHARACTER CREATION: appearance+stats(STR/END/FIN/PER/INT/LUCK)+perk(Etrian)+loadout; SPEC+SRC gap.

## PART 9 — THE SIX DECISIONS: 1 GO≤3-batch, 2 separate+correct, 3 track, 4 build Tests 17/1, 5/6 no-op+correct audit.

---

## PART 10 — STEP-BY-STEP (execution-optimized: EXT-ID + `src/` target)

### Phase A–F — Decisions 1–4 + low-risk fixes (v8): commit 18 spec (≤3 batches), correct `.hermis.md` pointer, track archive, build `ZombieEngineTests` (expect 17/1), F1–F8 bug fixes (vkQueueWaitIdle, UINT64_MAX acquire, hardcoded-3 pacing, null cache, hot-new, imgui-fmt).

### Phase G — Render-perf + scalability + far-view + TI-debug (GO; `src/render/`+`core/`+`spec/M4.5`):
G1a/G1b MSAA+resolve→M4.5-EXT-34; G3 edge-stencil; G-r3 prepass alpha-test (R3/T1MK:317); G2 sort; G4 cache@479; G5fix framesInFlight; G10 late-latch; G11 VMA defrag; G5b async present; G-ssr/ssgi/rvt/imp; G-scale (fix M4.5:60); G28 dither; G-fog haze; G-blue blue-noise; G-stencil; G-overdraw; G-debug TI counters; Gcap captures.

### Phase H — WorldGen + Skyscraper + Apocalypse + Far-View (GO; `src/world/`): H1–H11 (v12, build-to-EXT M4-EXT-01/02/08/18/24/27/34/35/45/47-52 + city refs + NMS constraint + DS2 far-view).

### Phase I — AI + Nav + Horde (GO; `src/ai/`+`ecs/`+`physics/`): I1 Director attack-token+swarm; I2 anim-LOD+per-LOD skip; I3 EnTT+SpatialHash; I4 Jolt broadphase (G24); I6 NavMesh/A*+terrain-adaptive (R43); I5 300-frame+1000 zombies.

### Phase J — Audio + Acoustics (GO; `src/audio/`+M6.5): J1 own thread+voice budget+spatial; J2 proc synth off-frame (M6); J3 M6.5 propagation; J4 300-frame audio load.

### Phase K — Vehicles (GO; `src/` new) — M9-EXT-01..21: K1 vehicle physics + mech/dragon-scale.

### Phase L — Character + Survival-Feel + Juice (GO; `src/player/`+`src/ui/`+`spec`):
- L1 author M12-EXT-Character-Creation + M11-EXT-Char-Menu + **M11-EXT-Juice (shake/hit-pause/damage anim, R42)** + **M12-EXT-Dread-Loop (G-DREAD/G-INFECTFEEL: timed infection dread, darkness/horde tension)** (GO, spec read-only). *Inspiration: I Am Legend — when the sun sets the infected own the night; dusk = the board-up moment. Original inspiration for the darkness/dread pillar, woven in only — no mechanic change.*
- L2 `src/player/` controller+state. L3 appearance→proc mesh. L4 stats→survival. L5 perk tree→M7. L6 loadout→M8/M2.6.
- L7 juice wired to combat. L8 300-frame character+juice scene no hitch.
- NOTE: do NOT re-add SEIR/loot/crafting — they exist (Part 1). Only frame them as tension (G-DREAD/G-INFECTFEEL/G-LOOTRISK design layer).

### Phase M — Base Defense + Horde-Night + Settlement + Co-op + Save + Modding (GO; `src/` multi):
- M-a `src/save/` chunk diff-on-unload (R44, M7-EXT-07). M-b `src/net/` M12 replication (co-op shared save).
- M-c **G-BASE: player-buildable fortifications (walls/spikes/turrets, M1-EXT-31/M9 turrets) + scheduled horde-night wave using M8-EXT-32 funnel pathing (R48)** — author M12-EXT-Base-Defense + M8-EXT-Horde-Night (GO).
- M-d `src/ui/` HUD/menus (M11). M-e `src/modding/`+`src/slm/` (M13). M-f M8 settlement NPCs/brands + G-EMERGE (player choices ripple settlement). M-g G-LOOTRISK (threat-scaled loot over M8 + Appendix C tags).

### Phase N — Gospel refresh + artifacts: N1 refresh gospel (R14-18+V28/V29+idTech+DS2+NMS+char+design); N2 transcript coverage; N3 RENDER-PERF-PLAN; N4 skill.

---

## PART 11 — VERIFICATION (per gate)
Stutter-Budget + Visual-Parity + Far-View + TI-Debug + Character/Juice (L8) + Nav/Horde (I5) + Base/Horde-Night (M-c) + Save/Coop (M-a/b). All via `Swapchain.cpp` harness + captures → `recon/*.md`.

## PART 12 — STILL-OPEN
Design-layer gaps G-BASE/G-DREAD/G-EMERGE/G-LOOTRISK/G-INFECTFEEL/G-JUICE + 8 stub modules + renderer/perf + world/city/far-view + character + save/coop. Either/or E1–E33 + new design branches (horde-night cadence, base-building depth, dread intensity). Awaiting per-decision/per-branch GO.

## PART 13 — "ASSUME YOU'RE WRONG" — 3-WAY DISPROVE
- **"SEIR/loot/crafting already exist"** — (a) M8-EXT-03 SEIR eqns; (b) M8-EXT-01 brand loot; (c) M8 crafting stations. Agree⇒don't rebuild.
- **"Base defense partially exists (settlement AI)"** — (a) M8-EXT-32 funnel pathing; (b) M11:213 SettlementDefense seed; (c) but NO player-placeable walls/turrets. Agree⇒G-BASE = player-agency layer only.
- **"8 stub modules"** — (a) src census 0 cpp; (b) spec has EXT; (c) find confirms headers. Agree⇒real.
If any disagrees → debug 5 ways.

---

*Authored by Jenny. **v14 = DESIGN-LAYER CORRECTION.** You were right: I was about to build the wrong game. Your spec is PhD-level simulated (SEIR infection, brand-loot economy, settlement defense, catenary physics, corpse-flattening) — I verified and will NOT rebuild those. The missing "key things" are DESIGN-LAYER: player-built base defense + horde-night (R48), dread/tension loop (R45/R47), emergent storytelling (R46), threat-scaled loot (R49), game juice (R42), infection-as-dread. Plus the 8 stub modules + technical perf from v13. REALISM-BUT-FUN RULE: simulate world deeply, let players create fun via agency. TI/idTech/DS2/NMS/city/character/scalability retained. Fact-checked (3-way disprove). No execution — awaiting per-decision/per-branch GO.*

---

## PART 14 — SIDE-QUEST SYSTEM: 200 HIGH-QUALITY ROAMING QUESTS (no main story, environmental)
**Fact-check (no-dup rule):** spec has **NO quest/side-quest/objective/encounter system**. Only M10-EXT-24 "Objective Tracker" (tl;dr "No minimap/objectives" — POI overlay, not quests) + M1:399 event-bus signals (death/horde/scarcity) + M10 weather events. → proposing new EXT blocks is NOT a duplicate. (M10-EXT-24's "no objectives" supports discovery-based quests — no waypoints.)

**Research (R50–R53):**
- R51 (ScienceDirect): procedural branching quests = planning + evolutionary search guided by **story arcs**; generation must be **heavily constrained** or it breaks narrative. → need authored story-arc templates + constrained generator.
- R52: lean on **environmental storytelling** (bloodstains→hidden path, architecture hints history); **genuine choice** = active storytelling; avoid fetch-chores.
- R53: radiant quests hated when barebone; fix = **interesting locations + environmental narrative** (STALKER/Elden Ring model: lore via item text + environment). Barebones radiant gives a *reason to explore*.

**Design (M-rated, dark, gore-heavy — matches genre; reuse spec gore, don't sanitize):**
- **Gore already specced — reuse:** M6.5-EXT-13 blood-spatter RVT (lingers on walls after body gone), M7-EXT-01 corpse flattening (persist, slick wet corpse-puddles), M2.9 per-limb DamageEvent (ballistics/cavitation, LimbId), M1:1283 forensic blood streaks, M4.5 wet-surface/blood material. Quests feed these systems (a cleared room = blood on walls via M6.5-EXT-13).
- **Discovery-based:** quests found by roaming (no waypoint). Triggers: environmental cue (barricaded room, distress signal, survivor cache, dead person's journal, strange anomaly, a scene of devastation).
- **Constrained generator:** ~12 authored **story-arc templates** (each a coherent micro-narrative) × procedural **parameters** (location, survivor profile, hazard, reward, environmental fragments). 12 templates × ~17 variants ≈ 200 detailed quests, each coherent (R51 constraint).
- **Themed archetypes (M-rated, dark, unflinching — all environmental storytelling):**
  1. **Last Traces** — reconstruct a survivor's final days from blood, spent rounds, a journal cut mid-sentence; the room tells you what happened.
  2. **Living Signal** — reach/contain a surviving radio operator before the horde does.
  3. **Lost Cache** — recover a dead group's supplies (often among their corpses).
  4. **Decipher** — read a deceased person's notes to learn a safe route or recipe.
  5. **Hazard Clear** — neutralize a danger (gas leak, collapsed stair, a trap) so an area is safe; bodies everywhere.
  6. **Settlement Errand** — help an M8 NPC (repair, deliver, scout) in a world that's falling apart.
  7. **Strange Signal** — investigate an anomalous broadcast (leads to lore, dread, not relief).
  - *Darkness/dread pillar (your "task 11") is inspired by I Am Legend — the original inspiration: when light dies, the Dark Seekers own the night, and dusk is the board-up moment of rising tension. Woven in as tone only; no existing mechanic changed. Feeds M11-EXT-62 dread surface + M8-EXT-51 scheduled horde-night.*
  8. **Theones We Lost** — a barricaded room, a note left mid-thought, a scene of suicide rendered with diegetic restraint: devastation and loss, awareness of how people break under the end of the world. Brings attention to it as the human cost. (Your call as author — tragic, weighty, part of life.)
  9. **Supply Line** — re-open a severed trade route between settlements; the previous couriers didn't make it.
  10. **Weather Window** — use a calm-weather gap to reach a previously-blocked, corpse-choked site.
  11. **Companion** — rescue/tame a surviving dog (one of few warm beats, earned in the dark).
  12. **False Haven** — a trap site that teaches caution (survivor outsmarts, not dies; the previous occupants weren't so lucky).
- **Story delivery = environmental + gore-as-story:** each quest's "story" lives in **object fragments, blood, persistent corpses, item-description lore** (Elden Ring/STALKER/Metro), not cutscenes. Matches "environmental storytelling" + M10-EXT-24 no-waypoint. Gore is the punctuation, not the point.
- **Genuine choice (R52):** each quest offers 1–2 meaningful branches (help vs scavenge-and-leave; warn settlement vs keep silent) that ripple to M8 settlement/brand state (G-EMERGE). The darkest choices have weight (do you put down the turned kid? do you take the dead man's meds?).

**Proposed EXT (fold into main spec — needs your GO, read-only gate):**
- `M12-EXT-SideQuest-System` (constrained generator + 12 templates + discovery triggers).
- `M12-EXT-Quest-Environmental-Fragments` (object/blood/corpse/item-lore storytelling delivery).
- `M8-EXT-Quest-Ripple` (quest choices → settlement/brand state).
- `M10-EXT-Quest-Discovery-Cues` (no-waypoint environmental triggers; pairs M10-EXT-24).
- `M12-EXT-Tragedy-Beats` (suicide-as-tragic-environmental-storytelling template; diegetic restraint, awareness-bearing).

**Either/or (new):** E34 quest count 200 (12×17) vs 100; E35 constrained-generator vs hand-authored; E36 discovery-only vs optional ping; **E43 gore-density (full M-rated unflinching vs toned)**.

**Verification:** 200 quests generated without narrative contradiction (R51); sample 20 hand-reviewed for quality + tone (dark, gore-as-story); each discoverable by roaming; gore routes through M6.5-EXT-13/M7-EXT-01 (reuse, not new).

---

## PART 15 — COHERENCE & INTEGRATION (does it all work for YOUR game?)
Cross-system wiring check — every new system extends an existing one (no contradiction, no duplicate). Fact-checked against spec.

**A. Dread loop (G-DREAD) data-flow:** M10 fear scalar field (M10:1329, "fear rises near zombies") → M11 trigger-resistance (M11:162, controller pushes back when StressLevel spikes) + M6.5 audio (dread drones) + M4.5 atmo. G-DREAD = *surface* that field to player feedback (vignette/pulse/audio swell as fear climbs), plus **infection clock** (M8 SEIR progression shown via M11 status UI). All reuse, no new sim. ✓ coherent.

**B. Base defense + horde-night (G-BASE) flow:**
- Player places walls/spikes (new, `src/player/`+Jolt) → turrets reference M1-EXT-31 (future turrets) + M9 vehicle turrets.
- **Horde-night cadence (fleshed):** scheduled every **N in-game days** (default 7, 7DTD blood-moon model), intensity scales with `SettlementDefense` deficit (M8:213) + player hoarded loot (risk/reward: more loot = bigger horde). Pathing uses M8-EXT-32 funnel chokepoints. Not arcade checkpoint-respawn.
- Cleared outpost → **becomes M8 settlement instance** (M12-EXT-Outpost-Clear-Loop writes settlement state to M8; NPCs spawn per M8-EXT-16). One coherent loop: clear→hub→defend→settle.

**C. 200-quest constraint checker (R51, fleshed):** generator = authored template × parameters. A **validator** runs before commit: (1) parameters exist in world (e.g. "snow region present" — checked vs Part 16 N2 macro-climate); (2) reward biome-appropriate (M4-EXT-39 palette); (3) no lore contradiction with other active quests; (4) gore routes to M6.5-EXT-13/M7-EXT-01. Rejects → re-rolls template. This is the "heavily constrained" guarantee. ✓

**D. Shared map region theming (fleshed, reuses E22 facade styles):** macro-climate regions (N2) → facade CGA style (E22) + material: snow=Brutalist concrete + ice-glaze; desert=adobe/stripped Chicago-school; grassland=suburban Chicago + ArtDeco; forest=weathered wood Chicago; swamp=corroded Brutalist; rainforest=overgrown modern glass. All reuse M4 facade grammar (H5) + M4-EXT-25 material compiler. ✓

**E. Emergent arc (G-EMERGE) flow:** quest/play choices → M8-EXT-Quest-Ripple → M8 settlement/brand state + Appendix A narrative (M11:277). Player transformation = accumulated ripple state, grounded (reject power-fantasy). ✓

**F. Cross-gate check:** all new systems feed the SAME verification harness (Swapchain.cpp timing + frame captures) + M4-EXT-35 stable IDs (shared map). No system breaks another. ✓

**G. Remaining thin gaps to flesh (honest):** (1) co-op shared-quest state (M12 net replication of quest progress — src/net stub); (2) quest-giver NPC mortality (can the M8 NPC die mid-quest? ties to SEIR); (3) horde-night perf budget at 1000+ zombies (I5 gate). All noted, not blocked.

**H. Either/or consolidation (all branches, your calls):** E1–E44 listed across parts. Key open: E20/E26 scalability strictness, E25 far-view impostor, E28 TI-debug, E31 char creation, E32 constrained procgen, E34 quest count, E37 diegetic HUD, E38 outpost scheduled-horde, E40 shared seed, E41 contiguous biomes, E42 region theming, E43 gore-density.

---

## PART 16 — FORGE / EDITOR MODE (FC3-style, co-op + local co-op) — your new ask
**Research (R66–R69, best-in-class):** FC3 Map Editor = 2000+ objects, team spawn points, "craft your own adventure, upload, share and play with friends." Halo Infinite Forge = publish map **+ reusable Prefab objects + scripting + ambient audio + biome (underwater/space)**. DS2 = async co-op via shared world (postboxes). Local co-op = split-screen (Portal Knights/Enshrouded/Terraria: "when one player leaves, world stays active"). **Synthesis: object palette + prefab + scripting + share + co-op edit + local split-screen.**

**Fact-check (no-dup):** M13 modding/SLM exist (M13-EXT-50/53, src/slm stub) but are *AI-flavor/modding*, NOT a world/forge editor. M12-EXT-22 = Dedicated Server Model (co-op foundation, **reuse**). M7-EXT-19 = persistence (reuse for editor save). → Forge mode = NEW, no collision. M13 true max = 53 → new blocks at M13-EXT-54+.

**Design (Forge Mode):**
- **F1 Editor Sandbox:** load any region of the shared map (Part 16) or a blank canvas; place from the same procedural asset library (M4-EXT-39 palette, M4 facade CGA, M8 buildings) — no separate asset set needed (100% procedural, reuse).
- **F2 Co-op Edit:** multiple editors simultaneously (reuse M12-EXT-22 dedicated server + M2.8 deterministic co-op). Each editor has a cursor; edits merge via the same chunk-authority model (M1-EXT-26 chunk-boundary transfer). One player leaves → world stays (R69).
- **F3 Local Co-op (split-screen):** same-screen two-cursor edit (reuse M12 local-split-screen if present, else new M12-EXT). Camera splits; shared edit state.
- **F4 Prefab System:** save a group of placed objects as a reusable Prefab (Halo Forge) → publish to workshop (reuse M13 modding/SLM for prefab metadata).
- **F5 Scripting/Logic:** lightweight trigger/event scripting (spawn waves, open doors) — reuses M1 event bus + M12 gameplay events. Keeps it survival-game, not just a sandbox.
- **F6 Save/Share:** editor map saves to M7 persistence (M7-EXT-19); publish to community (reuse M13 workshop). Play solo or co-op (M12-EXT-22).
- **F7 Death Stranding async touch:** published maps contribute to a shared community layer (optional) — echoes DS2 postbox co-op.

**Proposed EXT (merge-ready, M13-EXT-55/56/57/58 + co-op reuse):**
- `M13-EXT-55` Forge Editor Sandbox (loads region/blank, places from M4-EXT-39 palette).
- `M13-EXT-56` Co-op Edit Session (reuse M12-EXT-22 + M1-EXT-26 chunk authority).
- `M13-EXT-57` Prefab Publish System (reuse M13 modding/SLM metadata + M7-EXT-19 save).
- `M13-EXT-58` Forge Scripting/Logic (reuse M1 event bus + M12 gameplay events).
- Local co-op split-screen: reuse M12 local-split (or new M12-EXT if absent — verify at merge).

**Either/or (new):** E45 Forge scope (full FC3-style 2000-obj + scripting vs lighter place-only); E46 co-op edit (real-time merge vs turn-based lock); E47 local split-screen (include vs online-only).

**Verification:** Forge place → co-op peer sees object within frame budget (M12-EXT-22); prefab save/load round-trips (M7-EXT-19); scripting trigger fires gameplay event (M1 bus); no collision with M13-EXT-53 (true max 53, new at 54+).

*Part 16 added 2026-07-16 (your Forge-mode ask). Fact-checked vs M13/M12/M7 (reuse-first, no dup).*

---

## PART 17 — MERGE-READY DELIVERABLE
All proposed EXT blocks are written in **M0.md gold-standard format** in a separate file:
**`recon/plans/2026-07-16_PROPOSED-EXT-BLOCKS.md`**
→ 62 blocks across M2/M4/M5/M8/M9/M10/M11/M12/M13, each with heading + anchor + tags + tl;dr + ctx + math/how/impact, all extending verified-existing EXT-IDs (no duplicates — renumbered to free IDs: M2=76/77, M4=92/93/94/95/96/97/98/99/100, M5=60/61/62/63, M8=50/51/52/53/54/55/56/57/58/59/60/61/62/63/64/65, M9=24/25/26/27/28/29, M10=30/31, M11=60/61/62/63/64, M12=26/27/28/29/30/31/32/33/34/35/36, M13=55/56/57/58/59/60/61). Drop into your main specs on your GO (read-only gate).

**Block index:**
- M2: 76 (cross-world registry), 77 (NPC detail generator)
- M4: 92 (shared seed), 93 (macro-climate), 94 (region city), 95/96/97/98/99 (Part 23-24 additions), 100 (stadium/landmark POIs)
- M5: 60 (director mod), 61 (quest director), 62 (horde expansion), 63 (world-state director)
- M8: 50 (quest ripple), 51 (horde-night), 52 (emergent arc), 53 (faction standing), 54 (settlement growth), 55 (corp decay), 56 (survivor psyche), 57 (AI faction war), 58 (weapon modding), 59 (autonomous settlement sim), 60 (cascade engine), 61 (trade route network), 62 (social stratification), 63 (shanty/vertical slums), 64 (land/plot allocation), 65 (NPC daily life)
- M9: 24 (vehicle assembly), 25 (interactables), 26 (vehicle combat), 27 (death race custom), 28 (scarcity/loot), 29 (fuel/durability)
- M10: 30 (quest cues), 31 (region streaming)
- M11: 60 (diegetic no-HUD), 61 (quest fragments), 62 (dread surface), 63 (I Am Legend watch), 64 (branching quests)
- M12: 26 (side-quests), 27 (outpost loop), 28 (tragedy beats), 29 (forge quests), 30 (terminal arc), 31 (dusk watch), 32 (broker/casino), 33 (war map), 34 (permadeath/endless), 35 (endgame divergence engine), 36 (nuclear ending)
- M13: 55 (forge sandbox), 56 (co-op edit), 57 (prefab), 58 (scripting), 59 (SDK surface), 60 (mod registry), 61 (Lua VM runtime)

**Implementation into bigger main plan (your ask):**
1. This plan = the design layer; PROPOSED-EXT-BLOCKS = the spec edits.
2. Approval order: (a) shared-map M4-EXT-92/93/94 — foundation; (b) horror/dread M11-EXT-62, M8-EXT-52 — tone; (c) base/horde M8-EXT-50/51, M12-EXT-27 — core loop; (d) quests M12-EXT-26/28, M11-EXT-61, M10-EXT-30 — content; (e) diegetic M11-EXT-60; (f) forge M13-EXT-54/55/56/57 — editor.
3. Each block pairs with a Phase task in Parts 10–19 (H/I/J/K/L/M/N) — build-to-EXT.
4. ALL gate-checked via Swapchain.cpp harness + M4-EXT-35 stable IDs.

*Authored by Jenny. v14 complete: technical (TI/idTech/DS2/NMS/scalability/city/char/perf) + 8 stub modules + design gaps (base/horde/dread/emergence/loot/juice) + 200 environmental quests (M-rated, dark, gore-reuse, suicide-as-tragic your call) + Metro/FC3 synthesis + shared huge multi-biome map + Forge/editor co-op mode + **coherence/integration pass (Part 15)** + **merge-ready EXT blocks (Part 17 / separate file)**. Fact-checked vs spec (reuse-first, no dup), either/or E1–E47, no content restrictions. Plan-fleshing only — awaiting your GO to merge into main specs.*

---

## PART 18 — METRO + FAR CRY 3 STORYTELLING SYNTHESIS (your ask)
**Fact-check (no-dup):** spec ALREADY has diegetic systems — M11-EXT-04 (diegetic onboarding via SEIR), M11 diegetic HUD text (M11:350/372), and M11:277 references **"Appendix A — Procedural Narrative System (diary/journal template grammars)"** — a found-note generator. → Extend, do NOT rebuild. (Part 14's environmental fragments feed this same journal system.)

**METRO wisdom (R54/R55/R58):**
- Diegetic, minimal-HUD; story through **objects, journals, annotations**. "Artyom's Journal" = silent-protagonist thoughts via found notes (R55). → Your Appendix A diary generator = the engine; extend to **diegetic-only / no-HUD mode** (Metro minimal HUD) + quest fragments writing into it.
- "Journals/artifacts in environments = intrinsic reward for exploration" (R55). → Part 14's "story delivery = environmental" is exactly Metro's model — reinforced.
- Diegetic UI (R58): HUD embedded in world (watch, paper map, weapon state). Your M11 diegetic HUD text exists; extend to world-embedded readouts.

**FAR CRY 3 wisdom — GRAB vs REJECT (you said not arcade):**
- **GRAB — emergent outpost loop (R56/R57):** clear a hostile site → becomes safe forward hub; **stealth-emergent** (tight spaces, patrols, player-chosen approach). Maps PERFECTLY to your G-BASE (player fortifications) + horde-night + M8 settlement defense. → Core loop: **clear site → safe hub → horde defends it**. Grounded in SEIR/brand-loot/survival tension, NOT superhuman power = not arcade.
- **GRAB — player transformation arc (R56/R59):** FC3 Jason = victim→predator via *moral choices*. For NO-main-story survival, this becomes **emergent character transformation through choices** (your G-EMERGE / Part 14 quest branches ripple to M8 settlement). Keep **grounded/moral-ascending** (R59), REJECT Rambo power-fantasy.
- **GRAB — "the story is the game" (Grokipedia):** emergent gameplay = narrative. Your environmental storytelling + quest choices = this.
- **REJECT — arcade feel:** FC3 tower-climb map-reveal, checkpoint-respawn clear-loop (Shamus Young: "clear outpost, respawn, clear again"). → Your horde-night is *scheduled, meaningful* defense (7DTD blood-moon R48), NOT arcade respawns. Keep grounded.

**Synthesis into ZE (makes sense, not arcade):**
1. Metro diegetic journal (Appendix A) + Part 14 fragments = unified **found-narrative system** (no cutscenes; story in objects/notes). → extends M11-EXT / Appendix A.
2. FC3 outpost loop + G-BASE + horde-night = **clear→hub→defend** core loop, grounded in survival. (Outpost loop = M12-EXT-27; horde-night = M8-EXT-51.)
3. FC3 transformation + G-EMERGE/quest-branches = **emergent character arc via moral choices**, not power fantasy.
4. REJECT: arcade map-reveal, checkpoint-respawn, superhuman power.

**Proposed EXT (extend, not dup — needs GO, read-only gate):**
- `M11-EXT-60` Diegetic No-HUD Mode (Metro minimal-HUD / world-embedded readouts; extends M11 diegetic HUD).
- `M12-EXT-27` Outpost-Clear-Loop (FC3 clear→hub→defend; pairs G-BASE + M8-EXT-32 funnel pathing + horde-night).
- `M8-EXT-52` Emergent-Arc (quest/play choices → character/settlement transformation; extends G-EMERGE + Appendix A).

**Either/or (new):** E37 diegetic-only HUD vs toggle; E38 outpost-loop scheduled-horde (7DTD) vs free-clear; E39 transformation moral-grounded vs power-fantasy (reject latter).

**Verification:** diegetic mode verifiable (HUD off, readouts world-embedded); outpost loop 300-frame clear→defend no hitch; transformation choices ripple to M8 state (reuse G-EMERGE check).

---

## PART 19 — SHARED HUGE MAP (your pivot: one canonical world, all biomes, 100% procedural assets)
**Your instinct validated (R60):** Valheim proves same-seed = identical biomes/bosses/routes for all players; shared canonical map enables persistent co-op communities, shared landmarks, emergent shared stories, cheaper QA (one world). Trades replay-variety for better shared experience + higher cohesive quality. **Correct call for your goals.**

**Fact-check (REUSE, not duplicate — your "reuse all we made"):**
- **M4-EXT-34 Seed Hierarchy** `chunkSeed=hash(worldSeed,cx,cy)` → pin `worldSeed` to ONE constant = every player identical world. **One-line change, not new system.**
- **M4-EXT-23 Whittaker Biome** (temp/precip→biome, noise-blend) → snow/desert/grassland/forest/swamp/rainforest all fall out automatically.
- **M4-EXT-31 Biome Blending** → seamless borders (deserts fade to forest).
- **M4-EXT-39 Biome→Asset Palette** → each biome binds its own procedural asset set (reuse buildings/facade/vegetation systems per biome).
- **M4-EXT-17/18 soil + M4-EXT-23 foliage** → biome-correct generated vegetation.
- **M4-EXT-01 macro-graph** → highways/rivers/zoning span whole world; cities + biomes coexist.
- **M4-EXT-47..52 async + M4-EXT-50 lookahead** → streaming already region-capable (R63).
- **M8 settlement + M11 Appendix A narrative + Part 14 quests** → reused per region.

**100% procedural assets (R62, your "no assets"):** spec is already 100% procedural (M4 facade CGA, M4-EXT-25 material compiler, M4-EXT-27 impostors, M8-EXT-10 loot icons, M11 Appendix A narrative). Pivot *confirms* existing approach; adds NO hand-art.

**NEW (small, needed):**
- **N1 Pinned canonical `worldSeed`** (config constant) — shared map. (M4-EXT-34.)
- **N2 Macro-climate region field** (low-freq temp/precip, reuse M4:509 Simplex pattern) → large *contiguous* biome regions (snow pole / desert belt / grassland / forest / swamp / rainforest), not noise speckle. Feeds M4-EXT-23.
- **N3 Region landmark/city placement** → each biome region gets its own city (reuse M4 macro-graph + M8 settlement + skyscraper city from H-phase), themed per biome (snow city = brutalist concrete, desert = adobe, etc. — reuses facade CGA styles E22).
- **N4 Region-scale streaming page** → confirm M4-EXT-50 lookahead pages by region, not just chunk.

**Either/or (new):** E40 single pinned seed (shared) vs per-server seed; E41 biome regions contiguous (macro-climate) vs per-cell Whittaker only; E42 region city theming (reuse facade styles) vs uniform.

**Verification:** same `worldSeed` → byte-identical world hash across 2 clients (M4-EXT-35 stable IDs); all 6 biome regions present + seamless borders (M4-EXT-31); region cities stream in within frame budget (M4-EXT-50).

**Note:** This replaces the earlier NMS "per-player seed" framing (v12 Part 3) — you've chosen shared-map; NMS lesson (constrained procgen) still applies via M4-EXT-01.

---

## PART 20 — AI-GENERATED QUESTS + HUMAN PLACEMENT + MAP SIZE/DENSITY (your clarification)
**Your model:** AI (SLM) generates the bulk of the 200+ quests; **you curate + spatially place them** so they fit the world. Map must be big enough for real biome distinction yet **dense** (not empty padding).

**Research (R70–R73):**
- **R70 (AI-quest hybrid):** LLMs generate quests that "align seamlessly with provided game background," but "AI quest systems must carefully balance randomness" — best as *designer assistant + human curation*. → Your SLM generates, you curate/place. **Reuses M13 SLM** (M13-EXT-16 Offline Harness, -17 Prompt-Template Library, -18 Schema Validator, -19 Output→Validation, -21 Deterministic Seeding, -23 Provenance Tag). No new AI system.
- **R71 (map size):** RDR2 ~75 km² handcrafted; Valheim 314 km² procedural; Enshrouded EA 24 → full 64 km². Big numbers often include "large swaths of empty terrain" (ScientificMagz) — you want the OPPOSITE (dense). Size driven by biome-region count + POI density, not vanity km².
- **R72 (density):** "Visibility of POI > density" — but you want DENSE, so rule = dense POIs that are *discoverable/visible* (not a checklist every 50m). Sweet spot: distinct biome regions, each packed with readable POIs.
- **R73 (spatial placement):** POI placement near features (roads, rivers, buildings) via spatial algorithm — reuses **M4-EXT-32 POI Placement Validator**.

**Design:**
1. **AI-Quest Generation (SLM):** SLM takes a biome/region context + authored story-arc template (Part 14's 12 archetypes) → generates quest text/fragments/variants. Schema-validated (M13-EXT-18), determinism-seeded (M13-EXT-21), provenance-tagged (M13-EXT-23). Output = a quest *asset* in your curated pool.
2. **Human Curation + Placement:** YOU review the pool, pick winners, and **place them on the map at sensible POIs** via a Quest-Placement tool (new, in Forge editor M13-EXT-54/55) — e.g. barricaded house in suburb biome, crashed convoy in desert, flooded underpass in swamp. Placement validates biome-appropriate (M4-EXT-32) + no overlap + discoverable (visible, R72).
3. **Map Size + Density (the "big enough + dense" ask):**
   - **Target: ~64 km²** (Enshrouded full-release, proven dense-fun), chunk-based (256m → ~1000 chunks; your streaming handles it). Either/or E48: 40 / 64 / 100 km².
   - **6 biome regions**, each ~6–10 km² contiguous (macro-climate N2) — big enough to feel distinct, small enough to be dense.
   - **Density rule:** meaningful POI every ~100–200m in dense zones (cities/suburbs), sparser in hostile wilderness (risk/reward). Distant POIs readable via DS2 far-view impostors (Part 6) + M4-EXT-32 validator. NOT empty padding.
   - **Quest-to-POI binding:** each placed quest anchors to a POI; the 200+ quests distribute across POIs so the world feels alive, not a checklist.

**Proposed EXT (reuse M13 SLM + M4-EXT-32 + Forge):**
- `M13-EXT-58` SLM Quest Generator (takes biome+template → quest asset; reuses M13-EXT-16/17/18/19/21/23).
- `M12-EXT-29` Quest Placement Tool (you place quests at POIs; validates biome-appropriate via M4-EXT-32 + discoverable).
- `M4-EXT-95` Biome-Region Sizing (formalizes ~64 km² / 6 regions; reuses N2 macro-climate + M4-EXT-32).
- `M4-EXT-96` POI Density Field (targets ~100–200m spacing in dense zones; reuses M4-EXT-32).

**Either/or (new):** E48 map size — you want seamless biome transitions (no cut points), so biomes must BLEND over distance → world needs to be "a little big, not too big, not small." Fallout 4 (~10 km²) proves density sells size, but 10 km² is too small for gradual 6-biome blending. **Revised E48: 25 km² (biomes blend comfortably) / 40 km² (roomy) / 64 km² (ambitious, needs AI-quest density).** All dense, none empty, all seamless via M4-EXT-31 blending. Density (not km²) still the metric, but blending justifies the floor at 25. E49 AI-quest ratio (SLM 90% + you 10% curate vs 70/30); E50 placement (you place all vs SLM auto-places + you override).

**Verification:** SLM quest passes M13-EXT-18 schema; placed quest biome-valid (M4-EXT-32); POI visible at far-view distance (Part 6 impostor); 200+ quests across POIs, zero empty km².

---

## PART 21 — VEHICLE BUILD + CUSTOMIZATION + INTERACTABLES (your ask)
**Your vision:** Earn-to-Die-style weld-junk build (engine/armor/wheels/boosters/zombie-smash), Days-Gone modular motorcycle, GTA-rival driving feel, everything interactable like Fallout (Jolt rigid bodies you grab/weld/prop).

**Fact-check (reuse, not duplicate):** M9 vehicle milestone ALREADY has deep physics — M9-EXT-01 fluid traction, -02 tire slip/heat, -03 aquaplaning, -04 mud rut, -07 skid-marks (RVT), -09 fuel puncture, -10 chassis deformation, -16 convoy, -17 frame shear, -20 suspension, -21 wading. Jolt physics integrated (src/physics/PhysicsSystem.cpp includes Jolt). M8 crafting exists. → **Physics foundation exists; missing = build/customization + interactable layer.** M9 true max = 23 → new at 24/25.

**Research (R74/R75/R77):**
- **R74 (Earn to Die):** start beat-up, earn cash → upgrade engine/armor/wheels/boosters/zombie-weapons; smash through, conserve fuel. → **Vehicle Assembly system**: modular parts weld to a chassis; each part modifies M9 physics params (mass, traction, armor).
- **R75 (Days Gone):** modular hybrid motorcycle, customization; handling "not crotch-rocket" — tuned, not arcade. GTA-feel = handling tuning on a real tire/suspension model (your M9-EXT-01/02/20 already provide it). → Same assembly system, bike chassis variant; "rival racing feel" = tuning params, not new sim.
- **R77 (Fallout interactables):** everything is a Jolt rigid body you grab/move. Your Jolt integration supports this → need a **player-grab/interact + weld-to-vehicle** layer ("put shit on there and make shit happen" — weld a plow, mount a gun, strap corpses).

**Design:**
1. **Vehicle Assembly (M9-EXT-24):** chassis + modular parts (engine, armor plates, wheels, boosters, zombie-smash melee/spike, fuel tank). Each part = a Jolt rigid body or constraint; welding = Jolt constraint. Part stats modify M9 physics (mass→M9-EXT-17 frame shear, traction→M9-EXT-01, fuel→M9-EXT-09). Car AND motorcycle chassis variants. Player earns parts (M8 economy) → builds their "zombie mobile."
2. **GTA-feel Handling (tuning):** handling = tuned params on M9's existing tire/suspension/aero models (M9-EXT-01/02/20/05). Goal: weighty, responsive, rival racing games. Not arcade (reject Forza-grip).
3. **Interactables (M9-EXT-25):** player can grab/move/weld ANY Jolt body (debris, corpses, props) — "everything interactable." Weld-to-vehicle (plow, gun mount, corpse-winching). Reuses Jolt + M8 crafting + Forge editor (place in world).
4. **Customization depth:** like guns (M8 crafting/attachment) — parts swappable, paint/decals, performance tradeoffs (armor = slower, boosters = fuel drain). Fun + realistic, not annoying (clear UI, no grind-wall).

**Proposed EXT:** `M9-EXT-24` Vehicle Assembly/Customization (reuses M9-EXT-01/02/09/17/20 + M8 crafting + Jolt). `M9-EXT-25` Universal Interactables (reuses Jolt + M8 + Forge).

**Either/or (new):** E51 vehicle build depth (full Earn-to-Die weld-everything vs curated part slots); E52 handling model (tuned-realistic GTA-feel vs arcade-assisted); E53 interactable scope (everything-Jolt vs key-props-only).

---

## PART 22 — REAL-WORLD TOPOLOGY IMPORT (your "detailed maps easy" ask)
**Your idea:** import a real topology/heightmap (skyscraper city etc.) as a base, edit it in Forge. Validated approach.

**Research (R76):** OSM + heightmap import proven (Unity CityGen, CityGen3D, World Creator) — real road/terrain layout as base canvas. Your procedural skyscraper/facade system (M4 CGA, H5) then dresses it. → **Import real topology → Forge-edit → procedural dress.**

**Fact-check (reuse):** M4-EXT-01 macro-graph (highways/rivers/zoning across world) = natural import target; Forge editor (M13-EXT-54) = edit surface. M4 facade CGA = dress. No new terrain sim.

**Design (M4-EXT-97):** 
1. Import OSM road/river vectors + heightmap (real city or region) → feed M4-EXT-01 macro-graph as the base canvas (overrides procedural macro where imported).
2. Forge editor lets you edit (add/remove roads, reshape terrain, place districts) — same tool as build mode.
3. Procedural skyscraper/facade system (M4 CGA + M4-EXT-39 palette) dresses the imported layout per biome (N2).
4. Result: a "detailed map" in minutes — real street grid + your procedural buildings. Seamless with biome blending (Part 19 N2).

**Proposed EXT:** `M4-EXT-97` Topology Import Pipeline (reuses M4-EXT-01 macro-graph + Forge M13-EXT-54 + M4 CGA).

**Either/or (new):** E54 import source (real OSM city vs generated-then-edit vs hand-drawn); E55 dress level (full procedural skyscrapers vs simplified blocks).

**Verification:** imported OSM roads snap to M4-EXT-01 macro-graph (no seam); Forge edit persists (M7-EXT-19); procedural dress matches biome (M4-EXT-39); biome blend seamless (M4-EXT-31).

---

## PART 23 — FREE ASSET-LIBRARY INTEGRATION (TI-matching photoreal) (your ask)
**Source:** `recon/FREE_APIS_2026-07-16.md` — installed free bot-callable APIs. Game-dev/asset libs: **Poly Haven** (HDRIs/textures/models, photoscanned PBR), **Sketchfab v3** (models), **Kenney** (CC0, stylized), **OpenGameArt** (CC0/CC-BY, mixed), **Wikimedia Commons** (public-domain imagery/textures), **Steam storefront** (reference only).

**TI "same type of tech" (R78):** photoreal = **PBR (albedo/normal/rough/AO/metal/displacement) + bindless descriptor indexing + virtualized geometry + RVT streaming + deferred MSAA.** Your engine ALREADY has the plumbing: bindless descriptor buffer (Device.cpp:92), RVT (M4.5-EXT-26), procedural material compiler (M4-EXT-25), BC7/ASTC transcode (M4-EXT-25), virtualized-geometry intent. → Asset libs feed THIS pipeline, not a separate one.

**Photoreal filter (your "looks photoreal, same tech as TI" ask):** SUPPORT photoscanned PBR libs — **Poly Haven** (gold standard, exactly TI-style PBR), **Wikimedia** public-domain photos (photoreal textures), **Sketchfab filtered to PBR-tagged models**. **REJECT for hero/photoreal:** Kenney + OpenGameArt (stylized/cartoon CC0) — use only for UI/non-hero props if at all, never for the TI-matching world. Steam = reference metadata only.

**Research (R79):** Poly Haven textures ship Diffuse/albedo, roughness, normal (OpenGL), displacement, AO, metalness — native PBR, drop into your bindless+RVT pipeline. No key, rate-limited → batch with sleeps (per FREE_APIS notes).

**Design (reuse-first):**
1. **Ingest (M4-EXT-98):** bot pulls Poly Haven/Wikimedia/Sketchfab-PBR via their APIs (no key) → transcode to BC7/ASTC (M4-EXT-25) → assign **bindless descriptor slot** (Device.cpp:92) → stream via **RVT (M4.5-EXT-26)**. One photoreal texture → many variations via M4-EXT-25 node-graph (domain-warped noise) — honors "100% procedural" principle (photoreal base + procedural variation).
2. **Photoreal Filter + Provenance (M4-EXT-99):** auto-reject non-PBR/stylized assets (Kenney/OA) from hero pipeline; tag provenance (CC0/CC-BY/PD) for legal compliance; validate map set completeness (albedo+normal+rough+AO present) before bind.
3. **HDRIs (Poly Haven):** env probes → IBL lighting (matches TI photoreal relighting, R78 PBR analytic relighting).
4. **Models (Sketchfab-PBR filtered):** rigged/static meshes → bindless + virtualized-geometry path.

**Proposed EXT:** `M4-EXT-98` Asset Library Ingest Pipeline (reuses M4-EXT-25, M4.5-EXT-26, bindless, BC7). `M4-EXT-99` Photoreal Filter + Provenance (reuses M4-EXT-39 palette binding).

**Either/or (new):** E56 ingest scope (Poly Haven only vs +Wikimedia/Sketchfab-PBR); E57 variation (procedural-variation-on-photoreal vs raw-use); E58 hero-filter strictness (strict TI-PBR vs allow-stylized-for-props).

**Verification:** ingested Poly Haven PBR → bindless slot + RVT page (M4.5-EXT-26); variation via M4-EXT-25 node-graph; stylized rejected by M4-EXT-99; provenance tagged; no key APIs batched with sleeps (FREE_APIS note).

*Part 23 added 2026-07-16 (your free-asset-library + TI-photoreal ask). Source: recon/FREE_APIS_2026-07-16.md. Fact-checked vs M4-EXT-25/39, M4.5-EXT-26, bindless (reuse-first, no dup).*

---

## PART 24 — ARCHITECTURE COMPLETENESS (gaps found in hard audit)
**Hard audit (T3–T7) found:** 27 proposed blocks collision-free (after fixing M13-EXT-54→55 real collision). Save versioning ALREADY exists (M7-EXT-10 migrator) — not a gap. Real remaining gaps:

**1. Modding SDK surface (GAP — real):** Forge publishes prefabs/scripts (M13-EXT-55/58) but the *SDK surface* (how community code/assets load + sandbox) isn't architected. For a game with Forge + co-op, the modding SDK = the long-term content engine. → `M13-EXT-59` Modding SDK Surface (reuses M13-EXT-58 scripting + M13 SLM sandbox; loads community Lua/visual-script in a bounded VM; assets via M4-EXT-98 ingest).

**2. Audio middleware decision (EITHER/OR, not hard gap):** M6/M6.5 are custom procedural (acoustics RVT, blood-spatter). R81: Wwise = layered-audio backbone, FMOD = mature cheap voices, MetaSounds = custom. Your engine is custom Vulkan → **custom procedural is already your path** (M6.5). Decision: keep custom (reuse M6/M6.5) vs add Wwise for authored music/VO. → E59.

**3. Architecture coverage check (all 13 milestones have plan architecture?):**
- M0 bootstrap ✓, M1 ecs ✓, M2 physics ✓, M3 renderer ✓, M4 worldgen ✓ (Part 19/22/23), M4.5 renderfx ✓, M5 ai — **PLAN GAP** (Zombie AI/nav/horde director not architected as a build phase — Part I proposed but thin), M6 audio ✓ (custom, E59), M6.5 acoustics ✓, M7 systems ✓ (save M7-EXT-10 ✓), M8 settlement ✓, M9 vehicles ✓ (Part 21), M10 systems2 ✓, M11 ui ✓ (Part 15/18), M12 game ✓ (Part 14/20), M13 modding ✓ (Part 16/24).
- **M5 AI is the thinnest** — need explicit navmesh/A*/horde-director architecture. ADD to Part 24.

**4. M5 AI architecture (add):** Zombie AI = (a) **navmesh/A* pathfinding** (terrain-adaptive, falls/jumps), (b) **horde director** (spawns/waves, uses M8-EXT-32 funnels), (c) **behavior tree** (M13-EXT-13 flavor advisor exists). Reuse Jolt + M1 SpatialHash. → `M5-EXT-60` Horde Director + `M5-EXT-61` Navmesh/A* (M5 true max=59, so 60/61 free — verify). Either/or E60: AI determinism (lockstep for co-op vs client-prediction).

**Proposed EXT:** `M13-EXT-59` Modding SDK Surface; `M5-EXT-60` Horde Director; `M5-EXT-61` Navmesh/A* Pathfinding.

**Either/or (new):** E59 audio (custom procedural vs Wwise); E60 AI co-op model (lockstep deterministic vs client-prediction); E61 M5 scope (full navmesh+director vs simplified).

**Verification:** modding SDK sandboxes community code (M13 SLM bounds); M5 navmesh routes around obstacles; horde director scales with M8-EXT-51; all reuse Jolt/M1/M8 (no new sim).

*Part 24 added 2026-07-16 (hard-audit gap closure). Fact-checked: save-versioning exists (M7-EXT-10, NOT a gap); M13-EXT-59 + M5-EXT-60/61 free IDs.*

---

## PART 25 — GAP CLOSURE ROUND 2 (your "still so many gaps" + watch alarm)
**Re-audit (T2–T10) vs spec — verified what EXISTS (don't re-add):** survival needs hunger/thirst = **EXIST** (M2-EXT-18 stamina/hunger + M2-EXT-65 Coupled Hydration-Exertion). Barricade *physics* = EXIST (M3 StructuralFatigue, M5 nav re-bake on fortify, M2 material-aware). Weather/day-night = EXIST (M4.5/M9-EXT-23). Accessibility/settings/menu = EXIST (M13-EXT, M11). Anti-cheat/integrity = EXIST (M12-EXT-18/19). Companion/NPC = EXIST (M8).

**Real gaps found + your explicit asks:**

**1. Diegetic Watch + Dusk Alarm (YOUR ASK, I Am Legend).** Spec has NO player watch (only dev watchdogs M0-EXT-23). → `M11-EXT-63` Wristwatch + Dusk Alarm. Dynamic: fires a distinct alarm chime as light crosses the dusk threshold (time-of-day from M4.5/M9). **Toggleable** (settings or in-world wind/tap). **Diegetic + scavenged**: must find a watch in the world (POI loot) or spawn with one by chance — inside a dark building you can't see the sky, so the alarm is your only cue. Reuses M6.5 audio (R82: I Am Legend alarm = rising tension cue) + M11-EXT-60 diegetic-HUD + M4.5 day/night cycle. Feeds M11-EXT-62 dread surface + M8-EXT-51 scheduled horde.

**2. Base-Build Placement System (gap — physics exist, build MODE thin).** Barricades exist as targets but no player *placement* (snap, cost, ghost-preview). → `M12-EXT-30` Build Placement System. Reuses M3 StructuralFatigue component + M5 dirty-tile nav re-bake + M8 crafting cost. Either/or E62: free-place vs grid-snap.

**3. Server Browser / World List (gap — M12-EXT-18 = "no matchmaking").** Dedicated server exists (M12-EXT-22) but no way to *find* community/shared worlds. → `M12-EXT-31` Server Browser. Lists dedicated servers (M12-EXT-22) + Forge-published worlds (M13-EXT-57). Either/or E63: LAN-only vs internet.

**4. Wildlife / Fauna (gap — 0 spec hits).** Zombie world needs ambient animals (scavengers, infected wildlife) for life + food + dread. → `M5-EXT-62` Wildlife Ecosystem. Reuses M5-EXT-60 horde director + M5-EXT-61 navmesh + Jolt. Either/or E64: passive wildlife vs infected-beast variants.

**5. NG+ / Endless (either/or — spec resists sliders: "world gets harder from what you did, not a slider").** → E65: NG+ (carry unlocks, harder strain) vs pure-endless (no reset, cure = capstone not off-switch, per M8). Decision: spec leans endless; NG+ optional.

**6. Fast-Travel (either/or E66):** none (dread, M8 "honest tension") vs safe-house recall (risk: noise draws horde). Lean: none or limited.

**Proposed EXT:** `M11-EXT-63` Wristwatch+Dusk Alarm; `M12-EXT-30` Build Placement; `M12-EXT-31` Server Browser; `M5-EXT-62` Wildlife.

**Either/or (new):** E62 build snap; E63 browser scope; E64 wildlife type; E65 NG+ vs endless; E66 fast-travel.

**Verification:** watch alarm fires at dusk threshold (M4.5 time), toggle persists (settings), scavenged-from-POI or spawn-chance (M11-EXT-63); build placement re-bakes nav (M5); browser lists M12-EXT-22 servers; wildlife routes M5-EXT-61.

*Part 25 added 2026-07-16 (your watch-alarm ask + gap round 2). Fact-checked vs M2-EXT-18/65, M3, M5, M8, M11-EXT-60/62, M12-EXT-18/22, M4.5/M9 day-night (reuse-first, no dup).*

---

## PART 26 — FACTIONS, REPUTATION & SETTLEMENT DEVELOPMENT (NV-style, your "alive world" ask)
**Re-audit (T4/T5):** M8 has brand-loot (M8-EXT-01), SEIR illness (M8-EXT-03), cure (M8-EXT-04), crafting (M8-EXT-02/13), barter (M8-EXT-14), currency (M8-EXT-15), settlement NPCs (M8-EXT-16), farm plots (M8-EXT-19), walls (M8-EXT-29). **What's MISSING:** faction *standing with consequence*, and player-driven settlement *growth*.

**1. Faction Reputation + Consequence (NV gospel, R84).** → `M8-EXT-53` Faction Standing System. Standing tiers (idolized/liked/neutral/disliked/warned/hostile) per faction; quests fork on tier; factions trade/war (AI-vs-AI, M8-EXT-57); killing a faction member drops standing; some quests lock behind tier. Reuses M8-EXT-15 currency + M8-EXT-14 barter (reputation gates prices). NOTE: this is reputation, NOT a difficulty tier — difficulty stays the ONE dynamic system (M1-EXT-36).

**2. Settlement Development Sim.** → `M8-EXT-54` Settlement Growth. Player attracts NPCs (M8-EXT-16 residents), upgrades structures (reuse M12-EXT-30 build), grows economy (M8-EXT-14/15), defends (M8-EXT-29 walls + M8-EXT-51 horde). Settlement level unlocks services (medic, trader, quest-giver). Reuses M7 persistence. Either/or E68: passive-growth vs active-quest-gated.

**Verification:** faction tier gates M8-EXT-14 prices + forks M11-EXT-64 quests; settlement level spawns NPCs (M8-EXT-16) + trader; horde tests walls (M8-EXT-51).

*Part 26 added 2026-07-17 (your "alive world / NV side-quests" ask). Fact-checked vs M8-EXT-01/03/04/14/15/16/19/29, M5-EXT-60, M7, M11-EXT-64, M12-EXT-30 (reuse-first, no dup).*

---

## PART 27 — NV-STYLE QUEST HUBS + HIDDEN SAFE-TOWN / CASINO (your explicit ask)
**Your vision:** Fallout NV-style side quests (faction reputation, consequence, choice); a hidden town where people DON'T kill — but you CAN agro them; hella choice; feels alive.

**1. Branching Quest Hubs (NV, R84).** → `M11-EXT-64` Branching Quest System. Quests have forks (help/kill/sneak/report) that write to faction standing (M8-EXT-53) + world state (M7). No main story (per your design) — hubs are faction/settlement anchored. Reuses M12-EXT-26 (side-quests) + M13-EXT-60 (SLM generator) + M8-EXT-53.

**2. Hidden Safe-Town + Casino (your ask).** → `M12-EXT-32` Neutral Haven (casino/town). A hidden pocket of civilization — faction-neutral, NO-KILL zone by default (NPCs don't agro unless you do). You CAN provoke them (shoot/steal) → standing drops (M8-EXT-53), they fight back. Casino = economy sink + chance-based loot (reuse M8-EXT-15 currency + M8-EXT-02 crafting). "Doesn't have to be underground literally — hidden in a cool town." Choice-rich: info broker, gambler, quest-giver, black-market (M8-EXT-14 barter). Reuses M4-EXT-32 POI placement + M8-EXT-16 NPCs.

**Design anchors (your "alive" bar):** every NPC has a routine (M8-EXT-16), reacts to your standing (M8-EXT-53), and the haven shifts hostile if you break the peace. The casino is a Diegetic-HUD-free social space (M11-EXT-60) — dice, cards, drink, deals.

**Either/or (new):** E67 faction model — factions trade/war is AI-vs-AI (M8-EXT-57), NOT a difficulty tier (M1-EXT-36 is the one dynamic diff); E68 settlement growth (passive vs quest-gated); E69 casino risk (pure-economy vs rigged-deck minigame); E70 haven breach (NPCs never agro vs provoke-only).

**Proposed EXT:** `M8-EXT-53` Faction Standing; `M8-EXT-54` Settlement Growth; `M11-EXT-64` Branching Quests; `M12-EXT-32` Neutral Haven/Casino.

**Verification:** quest fork writes M8-EXT-53 + M7; haven NPCs calm unless provoked (M8-EXT-53 drop) then agro; casino sinks M8-EXT-15 currency; all reuse existing systems.

*Part 27 added 2026-07-17 (your safe-town/casino + NV-quest vision). Fact-checked vs M8-EXT-14/15/16/53/54, M11-EXT-60, M12-EXT-26, M13-EXT-60, M4-EXT-32 (reuse-first, no dup).*

---

## PART 28 — CORPORATE DECAY + CONSUMERISM + BRAND PARODY (Borderlands/WALL-E, your ask)
**Your vision:** Borderlands 2's *decaying corporations* + hyper-consumerism aesthetic; WALL-E's abandoned-consumerism; real brands as **parodies** (satirical take, not funny-required, legally safe). Based in reality.

**Spec gap:** brand/corp is ABSENT from spec (only "input consumer"). → `M8-EXT-55` Corporate Decay Layer.

**Design (R88 + your take):**
- **Parody brands, not real:** e.g. "Buy-N-Large" → "BUY-N-MORE" (WALL-E homage), "Hyperion/Vladof" → "HYPERION-EX" / "VLADOV-CO" (Borderlands homage), "Coca-Cola" → "KOLO-A" (generic cola). Satirical corporate-speak signage, jingles as diegetic audio (M6.5). No real trademark → no lawsuit.
- **Decay tells story:** pre-outbreak corps were everywhere (billboards, products, uniforms); post-outbreak they're rusted husks, looted stores, abandoned billboards with peeling ads. World-gen scatters corp remnants by biome (reuse M4-EXT-39 palette + M4 macro-graph).
- **Consumerism as loot logic:** corp warehouses = best loot (M8-EXT-02 crafting mats, M8-EXT-15 currency); armored trucks = rare; this rewards exploring the "buy everything" world that's now dead.
- **Realism anchor:** corps are *why* it fell — over-extraction, denial, supply-chain collapse (WALL-E). Subtext, not preachy.

**Fun-check (your "is black market/info broker fun?"):** Systems aren't fun by existing — they're fun when they **create decisions**. So:
- **Black market** (M8-EXT-14 barter extension): sells *contraband* (faction-locked gear, corpse-derived meds) at risk — buying marks you (M8-EXT-53 standing), cops/faction may hunt you. Risk/reward = fun.
- **Info broker**: sells *dynamic intel* — live faction-war fronts (M8-EXT-57), horde routes (M8-EXT-51), hidden haven locations (M12-EXT-32). Paying reveals the *system* moving, so you play smarter. Fun = information asymmetry resolved.
Both reuse M8-EXT-14/15/53 + M6.5 audio + diegetic (M11-EXT-60).

**Either/or (new):** E71 parody tone (satirical vs straight); E72 corp-decay density (everywhere vs sparse).

*Part 28 added 2026-07-17 (your Borderlands/WALL-E/brand-parody ask). Fact-checked vs M4-EXT-39, M8-EXT-02/14/15/53, M6.5, M11-EXT-60, M12-EXT-32 (reuse-first, no dup).*

---

## PART 29 — RIMWORLD MECHANICS + LUA VM MODDING (your asks)
**RimWorld best-fit mechanics (R86), filtered to YOUR game (not copy):**
- **Survivor needs + mental break** (RimWorld colonist breakdown): player + companions have mood/needs (reuse M2-EXT-18 stamina/hunger + M8 SEIR). Push too far → break (panic, desertion, lash-out). → `M8-EXT-56` Survivor Psyche. Reuses M2-EXT-18 + M8-EXT-03 (illness) + M11 stress (M11:162).
- **AI faction war** (RimWorld factions fight each other): factions (M8-EXT-53) wage war independent of player — fronts shift, you exploit/avoid. → `M8-EXT-57` AI Faction War. Reuses M5-EXT-60 director + M8-EXT-53. (NOTE: this is AI-vs-AI war, NOT a "hard mode" — difficulty stays the ONE dynamic system M1-EXT-36.)
- **Events / "crap happens"** (RimWorld random events): reuse M5-EXT-60 director for world events (raid, trader, disaster) — already specced, cite.
- **Silent protagonist / emergent story**: your no-main-story design already matches.

**Lua VM for modding (R87 — what it is + is it good for you?):**
A **Lua VM** = an embedded scripting engine (Lua 5.4 or LuaJIT) running inside the game; modders write Lua scripts the game executes in a sandboxed state, instead of shipping raw C++. **Good for ZE? YES** — it's the industry-standard modding layer (Factorio, RimWorld, Garry's Mod, Don't Starve all use Lua). It's tiny, fast, embeddable in C++ via `sol2`/`sol3`. Your M13-EXT-59 Modding SDK Surface should USE a Lua VM. → `M13-EXT-61` Lua VM Modding Runtime (replaces the vague "bounded VM" in M13-EXT-59). Reuses M13-EXT-16/19/23 (SLM sandbox) + M7 (persist mods). Either/or E73: LuaJIT (fastest) vs Lua 5.4 (simpler, generational GC).

**ONE dynamic difficulty (your correction):** remove all hard/soft mode framing. Difficulty = M1-EXT-36 only (telemetry-adaptive, no slider). Faction-war (M8-EXT-57) is AI-vs-AI, not a difficulty tier. E65 NG+ reframed as "endless, cure = capstone" (already spec). E67 hard-mode DELETED.

**Proposed EXT:** `M8-EXT-55` Corp Decay; `M8-EXT-56` Survivor Psyche; `M8-EXT-57` AI Faction War; `M13-EXT-61` Lua VM Modding.

**Either/or (new):** E71 parody tone; E72 corp density; E73 LuaJIT vs Lua 5.4. (E67 hard-mode REMOVED.)

**Verification:** corp remnants scatter via M4-EXT-39; black market marks standing (M8-EXT-53); info broker reveals M8-EXT-57 fronts; survivor break reuses M2-EXT-18; Lua VM sandboxed (M13-EXT-16/19).

*Part 29 added 2026-07-17 (your RimWorld + Lua VM + remove-hard-mode asks). Fact-checked vs M1-EXT-36 (one dynamic diff), M2-EXT-18, M5-EXT-60, M8-EXT-03/14/15/53, M13-EXT-16/19/59, M7 (reuse-first, no dup).*

---

## PART 30 — WEAPON MODDING + FACTION-WAR MAP + AUDIO (gap closure round 3)
**Re-audit (T1–T3):** 
- **Dynamic economy = EXISTS** (M8.5 Elastic Supply-Demand + Macro Regional Price Inflation MV=PQ; M8-EXT-14 barter reads it). NOT a gap — cite, don't re-add. Faction war can disrupt trade (M8-EXT-53 standing + M8.5 regional price) → prices spike in war zones. Emergent, already supported.
- **Weapon modding = DATA exists, SYSTEM thin.** M8/M2.7 have brand-tier (M8-EXT-01), attachment kitbashing rolls (M8:317 Tier 2), receiver/barrel/sight structs (M2.7). But no *player-facing modding system* (slots you fill, crafted mods, tradeoffs). → `M8-EXT-58` Weapon Modding System.
- **Faction-war frontend = GAP** (0 spec hits). M8-EXT-57 (AI war) has no UI to *see* it. → `M12-EXT-33` War Map Overlay.
- **Audio middleware (E59)**: your M6/M6.5 is custom procedural (acoustics RVT, blood). Decision below.

**1. Weapon Modding System (M8-EXT-58).** Player attaches mods to slot groups (receiver/barrel/sight/underbarrel/mag) reusing M8:317 Tier-2 kitbashing + M2.7 structs. Crafted mods (M8-EXT-02) have tradeoffs (damage↑ but handling↓, weight↑). "Guns like the game" — deep, personal, every gun yours. Either/or E75: free-attach vs slot-limited (3-5 slots).

**2. War Map Overlay (M12-EXT-33).** Map layer shows live faction-war fronts (M8-EXT-57), contested regions, safe corridors. Fed by info broker (M12-EXT-32) + M8-EXT-53 standing. Diegetic (M11-EXT-60) or map-screen. Either/or E76: full tactical map vs fog-of-war (revealed by exploration/broker).

**3. Audio (E59 decision):** keep **custom procedural** (M6/M6.5) for SFX/ambience — it's your path and TI-aligned. ADD **Wwise only for authored music + voiced dialogue** (M-rated story beats, M11:277 narrative). Either/or E59: custom-only vs custom + Wwise-for-authored.

**Proposed EXT:** `M8-EXT-58` Weapon Modding; `M12-EXT-33` War Map.

**Either/or (new):** E75 mod slots; E76 war-map fog; E59 audio (custom-only vs +Wwise).

**Verification:** mod slots reuse M8:317 kitbashing + M2.7; war-map reads M8-EXT-57 + M8-EXT-53; economy inflation already M8.5 (cited); audio reuses M6/M6.5.

*Part 30 added 2026-07-17 (continue after crash — economy confirmed EXISTS M8.5, so only weapon-mod + war-map + audio added). Fact-checked vs M8.5, M8-EXT-01/02/14, M2.7, M8-EXT-53/57, M12-EXT-32, M11-EXT-60, M6/M6.5 (reuse-first, no dup).*

---

## PART 31 — PERMADEATH (ENDLESS) + VEHICLE COMBAT / DEATH-RACE / SCARCITY / FUEL (your asks)
**LOCKED DECISIONS (your words):** NO New Game Plus. **ENDLESS + permadeath** (Project Zomboid, R91): when you die, your *character* is deleted; the *world* continues (M7 persistence keeps settlements/factions/war moving). One dynamic difficulty (M1-EXT-36), no slider, no hard mode. → **E65 RESOLVED: endless + permadeath (no NG+).** E66 fast-travel: locked to **NONE** (dread/honest tension, M8).

**Real-life grounding + fun (your "based on real life with fun added"):** cars are everywhere but post-apocalypse they're *wrecks* — lootable, fixable, but finding one that runs is rare and earned. Gas is finite (real constraint → tension). Durability makes every run matter. Fun comes from the *references*: Death Race / Mad Max (weld guns on frames), Days Gone (ride + shoot), State of Decay (car-chase firefights).

**1. Permadeath + Endless (M12-EXT-34).** Character save deleted on death; world (M7) + other players' chars persist. Soft: a "last stand" death-cam. Reuses M7 persistence + M1-EXT-36 (one diff). No NG+ — the challenge is the world, not a loop.

**2. Vehicle Combat (M9-EXT-26).** Shoot from vehicle (driver leans/shoots, passenger seat fires — companion or you), recoil/aim while moving (reuse M9-EXT-24 assembly + M9-EXT-25 interactables + M2 weapons). Days Gone / SoD2 feel (R93). Either/or E77: full passenger AI gunner vs manual-only.

**3. Death Race Custom Cars (M9-EXT-27).** Weld weapons/guns/spikes onto frames (reuse M9-EXT-24 weld + M9-EXT-25). END-GAME hard content — a Mad Max / Crossout build. Ram plates, roof guns, spike bumpers. Either/or E78: frame-slot limit (3-5 weld points) vs free-weld.

**4. Vehicle Scarcity + Loot (M9-EXT-28).** Every car broken; rare working ones only in locked garage / rich house / farm / lucky find. A *working-ish* car is usually a **side-quest reward** (M12-EXT-26 + M11-EXT-64 branching) and super hard. Reuses M4-EXT-32 (POI) + M8-EXT-54 (settlement). Either/or E79: spawn-rate (very-rare vs ultra-rare).

**5. Fuel + Durability (M9-EXT-29).** Gas = finite resource (reuse M9-EXT-09 fuel tank; pump/siphon). Car + wheel durability degrade with use/combat; repair at settlement (M8-EXT-54) or roadside (M9-EXT-25). Simple UI (fuel gauge + condition bar), deep interplay (can't grind infinitely — gas runs out). Either/or E80: durability severity.

**Add MORE (your "add more"):** passenger companion combat ties M8-EXT-56 (survivor psyche) + M9-EXT-26 (your ally rides shotgun, shoots, can break under fire). Vehicle-as-loot = the ultimate side-quest prize (M12-EXT-26). Real-life: tire/engine wear, fuel scarcity, salvage economics — fun: Death-Race builds + horde-ramming.

**Proposed EXT:** `M9-EXT-26` Vehicle Combat; `M9-EXT-27` Death-Race Custom; `M9-EXT-28` Vehicle Scarcity/Loot; `M9-EXT-29` Fuel+Durability; `M12-EXT-34` Permadeath/Endless.

**Either/or (new):** E77 passenger gunner; E78 weld slots; E79 working-car rarity; E80 durability severity. (E65 RESOLVED endless+permadeath; E66 RESOLVED no fast-travel.)

**Verification:** permadeath deletes char not world (M7); vehicle combat reuses M9-EXT-24/25 + M2; death-race reuses M9-EXT-24 weld; scarcity ties M12-EXT-26/M11-EXT-64; fuel reuses M9-EXT-09; all proposed IDs free (dual-scan after write).

*Part 31 added 2026-07-17 (your permadeath + vehicle combat/death-race/scarcity/fuel asks). Fact-checked vs M7, M1-EXT-36, M9-EXT-09/24/25, M2, M12-EXT-26, M11-EXT-64, M8-EXT-54/56, M4-EXT-32 (reuse-first, no dup).*

---

## PART 32 — EMERGENT LIVING WORLD / ENDGAME DIVERGENCE (Sim Settlements + Metro 2033)
**Re-audit (T1-T3):**
- Sim Settlements-style autonomy = GAP (0 spec hits). M8-EXT-16 (NPCs) + M8-EXT-54 (growth) + M8-EXT-56 (psyche) exist but no autonomous settlement AI layer.
- Endgame divergence = GAP (0 spec hits). Only M12-EXT-34 (permadeath) touches endgame.
- Trade-route cascade = COVERED by M8.5 (economy) + M8-EXT-14 (barter) but only as abstract price model, not visible world-altering routes.
- Metro 2033 nuclear ending = GAP (0 spec hits). No player-triggered world-altering event.

**Research grounding (online, 2026-07-17):**
- RimWorld (Wikipedia + GameDeveloper emergent-storytelling theory): three pillars — (1) randomized story framework (fixed starting conditions → unique per playthrough), (2) NPC semi-control (autonomy yields emergent stories), (3) spiralling event chains (cascading consequences). "Losing is fun." Your design already has M1-EXT-36 (one difficulty) + M5-EXT-60 (director). This extends it.
- Sim Settlements: autonomous settlement plots with Mayor AI — settlers build, trade, defend without player micromanagement. The world feels alive because NPCs have agency. Your M8-EXT-16 (NPCs) + M8-EXT-54 (growth) are the foundation.
- Crusader Kings 3 (Wikipedia): dynastic simulator where every playthrough diverges via character traits, stress, cascading political events. Different endgame per player = the norm.
- No Man's Sky post-mortem: 18 quintillion planets but "vast but empty" — procedural scale alone ≠ alive world. Need systemic depth + consequences. Your design avoids this with M8.5 economy + M8-EXT-57 war + this new system.
- Metro 2033 (Fandom wiki): missile launch ending — player choice to nuke or not. Your "player-caused crater" draws directly from this.

**1. Autonomous Settlement Sim (M8-EXT-59).** Sim Settlements-style: each settlement runs its own AI. NPCs (M8-EXT-16) auto-assign to plots, build structures, trade, recruit. Player can influence but does not micromanage. Every settlement grows differently. Either/or E81: full autonomy vs player-directable.

**2. World-State Cascade Engine (M8-EXT-60).** Every action has ripple effects. Block a trade route → connected settlement food supply drops → starvation → NPCs flee or attack → reputation loss → faction war. Implemented as a graph of nodes (settlements/factions) and edges (trade/alliance/hostility). Small choices cascade. Either/or E82: simulated vs event-driven.

**3. Trade Route Network (M8-EXT-61).** Visible roads connect settlements; goods flow; player can protect, block, or raid. Reuses M8.5 economy + M8-EXT-14 barter. Blocked routes cascade (M8-EXT-60). Either/or E83: visible vs abstract routes.

**4. World-State Director (M5-EXT-63).** Director (M5-EXT-60) drives events based on world-state, not just player difficulty. Famine if settlement overcrowded, plague if SEIR triggers, mega-horde if zombie population critical. RimWorld storyteller model — events chosen for interest, not difficulty curve. Either/or E84: full world-state weighting vs limited.

**5. Endgame Divergence Engine (M12-EXT-35).** World-state accumulates over entire playthrough → triggers one of multiple endgame scenarios. Every playthrough ends differently. Scenarios:
- Humanity Rebuilds: AI settlements dominate, zombies contained, society reforms. Player can join the new order or leave.
- Empty World: Zombie population critical, settlements collapsed. Few survivors. Caravan/wanderer visits are rare lifelines. Player documents the end.
- Nuclear Winter: Metro 2033 choice — missile launch + crater. Permanent map scar. See M12-EXT-36.
- Zombie Ascendancy: Zombies evolve, become dominant. Humans are prey. Final broadcast.
- Faction Victory: One faction conquers all. Player chooses: serve, flee, or resist.
- The Exodus: Mega-horde consumes everything. Player must evacuate to a new map region.
Either/or E85: deterministic (cumulative score) vs emergent (state-machine).

**6. Nuclear Ending (M12-EXT-36).** Player finds missile silo (rare POI). Choice: launch to wipe a region (kills zombies + settlements + fallout crater) or disarm. Crater replaces terrain (M4.5 RVT), fallout zone spreads, permanent radiated biome. Metro 2033 reference. Either/or E86: player-only trigger vs AI-faction can also launch.

**Add MORE (your "add more"):** The cascade engine + endgame divergence means every playthrough is a unique emergent story. The Sim Settlements layer makes the world feel alive without you. The nuke is your "fuck this world" button with permanent consequences. Trade routes make the economy visible and touchable. The world-state director ensures the world reacts to its own problems, not just your difficulty.

**Proposed EXT:** M8-EXT-59 Autonomous Settlement Sim; M8-EXT-60 World-State Cascade Engine; M8-EXT-61 Trade Route Network; M5-EXT-63 World-State Director; M12-EXT-35 Endgame Divergence Engine; M12-EXT-36 Nuclear Ending.

**Either/or (new):** E81 settlement autonomy; E82 cascade model; E83 route visibility; E84 state weighting; E85 endgame trigger; E86 nuke availability.

**Verification:** settlement sim reuses M8-EXT-16/54/56/53/02/29; cascade engine reuses M8-EXT-53/57/54/59 + M8.5; trade routes reuse M8-EXT-14/15 + M8.5 + M8-EXT-59/60/57; world-state director reuses M5-EXT-60 + M8-EXT-60/59/03/04/51; endgame engine reuses M8-EXT-60/59/57 + M5-EXT-63 + M7 + M12-EXT-34; nuclear ending reuses M12-EXT-35 + M8-EXT-60 + M4-EXT-39 + M4.5. All proposed IDs collision-free (verified grep all spec+PROPOSED). Research grounded in RimWorld/Sim Settlements/CK3/NMS/Metro 2033 real sources (jina-fetched 2026-07-17).

*Part 32 added 2026-07-17 (your Sim Settlements-style living world + endgame divergence + cascade systems + Metro 2033 nuke ending ask). Research-grounded (RimWorld storyteller, Sim Settlements autonomous plots, CK3 cascade, NMS empty-world pitfall, Metro 2033). Fact-checked vs M8-EXT-16/53/54/56/57/59/60/61, M8.5, M5-EXT-60/63, M12-EXT-34/35/36, M7, M4-EXT-39, M4.5 (reuse-first, no dup).*

---

## PART 33 — PERSISTENT + PROCEDURAL NPCS / SOCIAL STRATIFICATION / SHANTY TOWNS / SIGNATURE POIs
**Re-audit (full-project reference scan):**
- M8 spec is **stubbed**: M8-EXT-16 literally says "No resident NPCs"; M8.5/M8.6/M8.7 are placeholders. So NPC depth must be built at the design layer (this plan) — these blocks ARE the M8.6/M8.7 realization.
- M2-EXT-18 (physiology/needs) exists; M5.1 companion AI exists (stubbed "no behavior" but referenced by M8 recruitable survivor block); M7-EXT-01 (StableId) is the persistence backbone — perfect anchor for cross-world NPCs.
- M4-EXT-32 (POI validator) + M4-EXT-25 (materials) + M4-EXT-39 (palette) + M3 structures = landmark placement pipeline already exists. Stadium reuses M8-EXT-51 (horde) + M8-EXT-15 (betting).
- GAPS found: (1) no persistent cross-world NPC tier; (2) generated NPCs are cardboard (no name/personality/plot); (3) no social stratification driving land; (4) no vertical shanty density; (5) no signature landmark POIs (stadium etc.).

**Research grounding (online, 2026-07-17):**
- Dwarf Fortress (wiki): personality = 51 facets + values + goals + memories; facets bias social skills and thoughts. Model for NPC depth without scripting each one.
- Crusader Kings 3 (wiki): courtiers have rank/opinion/lodgings; opinion math drives leave-chance; unlanded characters live at court. Model for stratification + dynamic standing.
- Watabou Procgen Arcana / Medieval Fantasy City Generator: procedural city/urban layout incl. dense vertical clustering. Model for shanty stacking.
- Sim Settlements (prior Part 32 research): autonomous plots — combined here with deep NPC agents.

**1. Persistent Cross-World NPC Registry (M2-EXT-76).** Two tiers: **Persistent** (player-made/notable NPCs — companions, mayors) get a global StableId (M7-EXT-01), survive permadeath (M12-EXT-34), appear across all worlds/saves (shared-seed co-op M4-EXT-92). **Generated** (world-filler) get full detail (M2-EXT-77) but regenerate per world. Either/or E87: global cloud registry vs local-per-save.

**2. Procedural NPC Detail Generator (M2-EXT-77).** Every generated NPC rolls: name (culture-weighted M4), personality (DF 51-facet), job (fits need + tier M8-EXT-62), schedule (daily routine), personal plot (seeded micro-arc), wealth/power, appearance. Deterministic: `npcSeed = hash(worldSeed, StableId)` — identical every session, regenerates identically after permadeath. Either/or E88: full DF 51-facet vs compact 12-facet.

**3. NPC Social Stratification (M8-EXT-62).** Wealth + Power scores → tiers: Elite (mayor/warlord, prime land M8-EXT-64, fortified core), Middle (foreman/trader, standard lot), Bottom (scavenger/slave, stacked housing M8-EXT-63). Drives plot allocation, quest importance, prices (M8-EXT-14 gates on tier), standing weight (M8-EXT-53). Player actions shift tiers. Either/or E89: hard caste vs fluid.

**4. Shanty Town / Vertical Slums (M8-EXT-63).** Bottom-tier zone = 3-level vertical scrap stacks (M3 prefabs + M4-EXT-25 materials), crammed footprint, rooftop/ground navigation layers. Density from population vs land cap (M8-EXT-64). Either/or E90: procedural-stacked vs player-built.

**5. Land & Plot Allocation by Status (M8-EXT-64).** Settlement land budget (M8-EXT-59 + M4-EXT-32) divided by tier: Elite = large walled plots; Middle = standard; Bottom = micro-plots (feeds M8-EXT-63). Re-evaluated on cascade events (M8-EXT-60) — war loser loses land. Either/or E91: fixed allotment vs market-bought.

**6. NPC Daily Life Simulation (M8-EXT-65).** Each NPC ticks wake→job→eat→socialize (gossip spreads standing changes, can flip a tier's mood)→personal-plot action→sleep. Advances personal plot (M2-EXT-77); plot threshold injects cascade event (M8-EXT-60). Psyche (M8-EXT-56) can break routine. Player can observe/interrupt. Either/or E92: full routine vs keyframe.

**7. Stadium & Signature Landmark POIs (M4-EXT-100).** Roster: Stadium (marquee hub — arena fights + betting M8-EXT-15, refugee quests M12-EXT-26, horde finale M8-EXT-51, director-triggered M5-EXT-63), Megamall (loot labyrinth), Radio Tower (beacon), Hospital (medquests), Prison (faction stockade), Subway (underground route). Placed via M4-EXT-32, region-skinned (M4-EXT-39). Variant = `hash(worldSeed, POI_id)` drives layout/loot/palette. Either/or E93: fixed roster vs seeded variant pool.

**Add MORE (your "add more"):** the stadium is your set-piece — a place to fight, bet, shelter a horde, or make a last stand. Shanty towns are the densest, most dangerous, most alive part of any settlement. Stratification makes wealth visible in the ground plan. Persistent NPCs mean your people outlive you.

**Proposed EXT:** M2-EXT-76 (cross-world registry); M2-EXT-77 (NPC generator); M8-EXT-62 (stratification); M8-EXT-63 (shanty); M8-EXT-64 (land); M8-EXT-65 (daily life); M4-EXT-100 (stadium/landmarks).

**Either/or (new):** E87 registry scope; E88 trait depth; E89 caste fluidity; E90 shanty build; E91 land market; E92 routine depth; E93 landmark pool.

**Verification:** M2-EXT-76 reuses M7-EXT-01/19 + M2-EXT-18 + M5.1 + M12-EXT-34 + M4-EXT-92; M2-EXT-77 reuses M2-EXT-18 + M8-EXT-16/56/62 + M4-EXT-92 + M8-EXT-60; M8-EXT-62 reuses M2-EXT-77 + M8-EXT-16/59/64/53/14; M8-EXT-63 reuses M8-EXT-62/64 + M3 + M4-EXT-25/39; M8-EXT-64 reuses M8-EXT-62/59 + M12-EXT-30 + M4-EXT-32; M8-EXT-65 reuses M2-EXT-77 + M8-EXT-16/56/59/62/53/60; M4-EXT-100 reuses M4-EXT-32/25/39/33 + M3 + M8-EXT-51/15 + M11-EXT-61 + M12-EXT-26 + M5-EXT-63. All 7 IDs collision-free (grep all spec + PROPOSED, 0 hits). Research grounded in DF/CK3/Watabou/Sim Settlements real sources (jina-fetched 2026-07-17). M8 spec confirmed stubbed — these blocks are the M8.6/M8.7 realization.

*Part 33 added 2026-07-17 (your persistent + procedural NPCs, social stratification, 3-tier shanty towns, stadium/landmark POIs ask). Full-project reference scan done; M8 spec confirmed stubbed. Research-grounded (DF personality facets, CK3 courtiers, Watabou city gen, Sim Settlements). Fact-checked vs M7-EXT-01/19, M2-EXT-18/77, M5.1, M8-EXT-16/53/54/56/59/60/61/62/63/64/65, M4-EXT-25/32/39/92/100, M3, M12-EXT-26/34, M5-EXT-63, M11-EXT-61 (reuse-first, no dup).*

*Part 21/22 added 2026-07-16 (your vehicle-build + interactables + topology-import asks). Fact-checked vs M9/M8/Jolt/M4/Forge (reuse-first, no dup).*

*Part 20 added 2026-07-16 (your AI-quest + human-placement + dense-map clarification). Fact-checked vs M13 SLM / M4-EXT-32 / Forge (reuse-first, no dup).*
