> **MERGED 2026-07-17**: all 62 blocks below were merged into their milestone spec files (M4/M8/M12/M11/M13/M5/M9/M10/M2). E87 locked to local-first (offline primary, friends-co-op optional, no cloud). M8-EXT-16 stub rewritten to resolve "No resident NPCs" contradiction. See archive/plans/2026-07-17_ze-gameplan-v35-restore-point/ for pre-merge backup.

# Proposed EXT Blocks — Merge-Ready (drop into main specs)
## ZombieEngine plan-fleshing output · format = M0.md gold standard

> These are PROPOSALS for your approval (milestone-spec files are READ-ONLY per your standing gate).
> Each block follows the M0.md format: `#### [MX-EXT-NN]` heading + `<a id>` anchor + `> **tags**` + `> **tl;dr**` + `> **ctx**` + Systems Touched / Math / How It Works / Reference Implementation / Player-Facing Impact.
> All extend existing EXT-IDs (verified present in spec) — no duplicates.

---

## M4 — Worldgen (shared-map + biome region additions)

#### [M4-EXT-92] Canonical Shared-World Seed Pin
> **tags** · general
> **tl;dr** · M4/D. Pin `worldSeed` to one constant so every client derives an identical world.
> **ctx** · Canonical Shared-World Seed Pin -- M4/D. Extends M4-EXT-34 (Seed Hierarchy). Replaces per-player random seed with a single shipped constant.
##### Systems Touched
M4/D world→region→chunk→entity derivation (M4-EXT-34).
##### How It Works
`worldSeed` is a build-time constant (not `hash(time)`). `chunkSeed = hash(worldSeed, cx, cy)` unchanged — all clients identical.
##### Player-Facing Impact
Everyone explores the same world: same biomes, cities, landmarks. Enables persistent co-op communities + shared emergent stories.

#### [M4-EXT-93] Macro-Climate Region Field
> **tags** · general
> **tl;dr** · M4 PCG. Low-frequency temp/precip field assigns large contiguous biome regions (snow/desert/grass/forest/swamp/rainforest).
> **ctx** · Macro-Climate Region Field -- M4 PCG. Extends M4-EXT-23 (Whittaker) by adding a region-scale driver so biomes form big areas, not noise speckle. Reuses M4:509 low-freq Simplex pattern.
##### Math
`regionTemp = simplex(worldX*0.0008, worldZ*0.0008, oct=2)`; `regionPrecip = simplex(worldX*0.0008+offset, ...)`; feeds `Whittaker(regionTemp, regionPrecip)`.
##### How It Works
One low-freq field per world (not per chunk). M4-EXT-23 classifies per-cell but reads this field for the macro band, so transitions occur over kilometers, not meters.
##### Player-Facing Impact
Vast distinct regions (snow pole, desert belt, rainforest basin) you cross on foot/vehicle — the "huge map" feel.

#### [M4-EXT-94] Region-Themed City Placement
> **tags** · general
> **tl;dr** · M4/M8. Each biome region gets its own themed city via M4 macro-graph + M8 settlement + facade CGA styles (E22).
> **ctx** · Region-Themed City Placement -- M4/M8. Extends M4-EXT-01 (macro-graph) + H5 facade grammar. Snow=Brutalist concrete+ice; desert=adobe/stripped Chicago; grassland=suburban Chicago+ArtDeco; forest=weathered wood; swamp=corroded Brutalist; rainforest=overgrown glass.
##### How It Works
Macro-graph seeds city footprints per region; facade CGA (H5) + M4-EXT-25 material compiler bind region-appropriate style/palette (M4-EXT-39).
##### Player-Facing Impact
Every region's skyline looks born of its climate — cohesive, believable, varied.

---

## M8 — Settlement (quest ripple + horde-night + emergent arc)

#### [M8-EXT-50] Quest-Choice Settlement Ripple
> **tags** · narrative
> **tl;dr** · M8/M12. Quest/play choices mutate settlement/brand state + Appendix A narrative.
> **ctx** · Quest-Choice Settlement Ripple -- M8/M12. Extends G-EMERGE + M11:277 Appendix A. A choice (help vs scavenge, warn vs silence) writes to M8 settlement state + brand-tier + narrative log.
##### How It Works
On quest resolve, `ApplyChoice(questId, branch)` → adjusts `SettlementDefense` / `brandTierMultiplier` / Appendix A journal entry.
##### Player-Facing Impact
The world remembers what you did. Settlements thrive or rot from your decisions.

#### [M8-EXT-51] Scheduled Horde-Night
> **tags** · combat
> **tl;dr** · M8/M5. Every N in-game days a horde assaults the settlement/outpost; intensity scales with defense deficit + hoarded loot.
> **ctx** · Scheduled Horde-Night -- M8/M5. Uses M8-EXT-32 funnel pathing. 7DTD blood-moon model. Not arcade checkpoint-respawn.
##### Math
`hordeSize = base * (1 + lootHoarded*k) * (1 + defenseDeficit*m)`; fires every `N` days (default 7).
##### How It Works
Timer in M7 epoch clock; spawns I-phase Director wave along M8-EXT-32 funnels; player defenses (G-BASE) engage.
##### Player-Facing Impact
Regular, escalating, winnable-with-prep defense nights — the core tension beat.

#### [M8-EXT-52] Emergent Character Arc
> **tags** · narrative
> **tl;dr** · M8/M12. Accumulated quest/play choices form the player's transformation (grounded, not power-fantasy).
> **ctx** · Emergent Character Arc -- M8/M12. Extends M8-EXT-50 + G-EMERGE. Arc state = sum of ripples; surfaces via M11 status UI + Appendix A.
##### Player-Facing Impact
You become who you chose to be — victim, protector, scavenger, monster. No main story needed.

---

## M11 — UI (diegetic no-HUD + quest fragments + dread surface)

#### [M11-EXT-60] Diegetic No-HUD Mode
> **tags** · ui
> **tl;dr** · M11. Metro-style minimal HUD; readouts embedded in world (watch, paper map, weapon state).
> **ctx** · Diegetic No-HUD Mode -- M11. Extends M11 diegetic HUD text (M11:350/372). Toggle or default-off.
##### How It Works
HUD overlay hidden; world objects (wristwatch mesh, annotated map item, weapon condition) carry state. M10-EXT-24 POI still no-waypoint.
##### Player-Facing Impact
Maximum immersion; story told by the world, not widgets.

#### [M11-EXT-61] Quest Environmental Fragments
> **tags** · narrative
> **tl;dr** · M11/M12. Quest story delivered via object/blood/corpse/item-lore, not popups.
> **ctx** · Quest Environmental Fragments -- M11/M12. Feeds M11:277 Appendix A + M6.5-EXT-13 (blood) + M7-EXT-01 (corpse). Part 14.
##### How It Works
Quest trigger spawns fragment entities (note, blood pattern, corpse tableau) readable via Appendix A journal generator.
##### Player-Facing Impact
Quests are discovered, not assigned. The room is the story.

#### [M11-EXT-62] Dread Surface (Fear/Infection Feedback)
> **tags** · ui, horror
> **tl;dr** · M11/M10/M8. Surfaces fear scalar + infection clock as felt tension (vignette/pulse/audio swell), not HUD numbers.
> **ctx** · Dread Surface -- M11/M10/M8. Extends G-DREAD. Reads M10:1329 fear scalar + M8 SEIR progression; drives M4.5 atmo + M6.5 audio + controller (M11:162). *Inspired by I Am Legend — original inspiration: when the sun dies, the infected own the night; dusk is the board-up moment of rising tension. Tone only, no mechanic change.*
##### Player-Facing Impact
Dread is felt, not read. One bite = a clock you feel ticking.

---

## M12 — Game (side-quest system + outpost loop + tragedy beats)

#### [M12-EXT-26] Side-Quest System (200, constrained)
> **tags** · narrative
> **tl;dr** · M12. Constrained generator: ~12 authored story-arc templates × parameters ≈ 200 coherent roaming quests.
> **ctx** · Side-Quest System -- M12. Part 14 + R51. Discovery-based (no waypoint, M10-EXT-24). Validator rejects lore/contradiction → re-roll.
##### Math
`quest = Template[rand(12)] × Params(region, survivor, hazard, reward, fragments)`; `Validate(quest)` gates commit.
##### How It Works
On chunk stream-in, generator offers 0–N quests whose params exist in that region (checked vs M4-EXT-41 macro-climate + M4-EXT-39 palette). Gore routes to M6.5-EXT-13/M7-EXT-01.
##### Player-Facing Impact
200 hand-quality side stories found by walking, not waypoints. M-rated, dark, gore-as-story.

#### [M12-EXT-27] Outpost-Clear-Loop
> **tags** · gameplay
> **tl;dr** · M12/M8. Clear hostile site → becomes safe hub → horde defends it (M8-EXT-51).
> **ctx** · Outpost-Clear-Loop -- M12/M8. FC3 emergent outpost (R56/R57) + G-BASE + M8-EXT-32. Cleared site writes M8 settlement instance (NPCs per M8-EXT-16).
##### How It Works
On clear, spawn M8 settlement; on horde-night, M8-EXT-51 assaults it. Player fortifications (G-BASE) engage.
##### Player-Facing Impact
The map is won in pieces — every cleared site is a foothold.

#### [M12-EXT-28] Tragedy Beats (suicide-as-environmental-storytelling)
> **tags** · narrative
> **tl;dr** · M12. Author's call: suicide portrayed as tragic human reality via environmental storytelling (barricaded room, journal cut mid-thought).
> **ctx** · Tragedy Beats -- M12. Part 14. Diegetic restraint; awareness-bearing; part of life at the end of the world. No content restrictions from planner.
##### How It Works
Template in M12-EXT-26 generator; spawns fragment tableau (M11-EXT-61) + Appendix A entry.
##### Player-Facing Impact
The human cost is shown, not hidden. Brings attention to it.

---

## M10 — Systems2 (quest discovery cues + region streaming)

#### [M10-EXT-30] Quest Discovery Cues
> **tags** · narrative
> **tl;dr** · M10/M12. No-waypoint environmental triggers that offer quests on proximity.
> **ctx** · Quest Discovery Cues -- M10/M12. Pairs M10-EXT-24. Extends Part 14 discovery.
##### How It Works
On chunk stream, evaluate cue predicates (barricaded room, distress signal, corpse tableau) → offer quest to M12-EXT-23.
##### Player-Facing Impact
Quests reveal themselves as you explore. No map-ping chore.

#### [M10-EXT-31] Region-Scale Streaming Page
> **tags** · streaming
> **tl;dr** · M10/M4. Lookahead (M4-EXT-50) pages by macro-climate region, not just chunk.
> **ctx** · Region-Scale Streaming Page -- M10/M4. Extends M4-EXT-50. Confirms huge-map streaming budget.
##### How It Works
Streaming budget allocated per region (M4-EXT-41); pre-load adjacent region on approach.
##### Player-Facing Impact
Crossing from forest to desert is seamless, no hitch.

---

## M13 — Modding (Forge / Editor Mode — co-op + local co-op)

#### [M13-EXT-55] Forge Editor Sandbox
> **tags** · editor
> **tl;dr** · M13/M4. Load any shared-map region (Part 19) or blank canvas; place from the same procedural asset library — no separate assets.
> **ctx** · Forge Editor Sandbox -- M13/M4. Extends FC3 Map Editor (R66) + Halo Forge (R67). Reuses M4-EXT-39 palette + M4 facade CGA + M8 buildings (100% procedural, no new art).
##### How It Works
Editor loads a region (M4-EXT-41 macro-climate) or blank; placement tool instantiates procedural objects (M4-EXT-39). Save to M7-EXT-19.
##### Player-Facing Impact
Build your own zombie world from the same parts as the main game.

#### [M13-EXT-56] Co-op Edit Session
> **tags** · editor, co-op
> **tl;dr** · M13/M12. Multiple editors edit the same map live; edits merge via chunk authority.
> **ctx** · Co-op Edit Session -- M13/M12. Reuses M12-EXT-22 (Dedicated Server) + M1-EXT-26 (chunk-boundary transfer) + M2.8 deterministic co-op. One player leaves → world stays (R69).
##### How It Works
Each editor = a cursor; placements broadcast over M12-EXT-22; chunk authority (M1-EXT-26) resolves conflicts. Shared edit state persisted (M7-EXT-19).
##### Player-Facing Impact
Build with friends in real time, same map.

#### [M13-EXT-57] Prefab Publish System
> **tags** · editor, modding
> **tl;dr** · M13. Save placed groups as reusable Prefabs; publish to workshop.
> **ctx** · Prefab Publish System -- M13. Halo Forge Prefab model (R67). Reuses M13 modding/SLM metadata (M13-EXT-50/53) + M7-EXT-19 save.
##### How It Works
Group select → serialize as Prefab (M13 SLM metadata shell, template-bounded) → publish via M13 workshop. Others drop it in their map.
##### Player-Facing Impact
Community-built parts; endless variety.

#### [M13-EXT-58] Forge Scripting / Logic
> **tags** · editor
> **tl;dr** · M13/M1. Lightweight trigger/event scripting (spawn waves, open doors) keeps it a survival game, not just a sandbox.
> **ctx** · Forge Scripting -- M13/M1. Reuses M1 event bus + M12 gameplay events. FC3/Forge scripting model.
##### How It Works
Editor places triggers → bind to M1 EventBus signals (death, horde-encounter, resource-scarcity). Fires gameplay logic in published map.
##### Player-Facing Impact
Make horde defenses, puzzles, stories — not just static builds.

#### [M13-EXT-59] Modding SDK Surface
> **tags** · modding
> **tl;dr** · M13. Bounded VM loads community Lua/visual-script + assets; the long-term content engine.
> **ctx** · Modding SDK Surface -- M13. Part 24. Reuses M13-EXT-58 scripting + M13 SLM sandbox (M13-EXT-16/19/23) + M4-EXT-98 asset ingest + M13-EXT-61 (Lua VM runtime). Sandboxed (template-bounded prompt per M13 safety scope).
##### How It Works
Community code runs in a bounded VM (Lua/visual-script); assets via M4-EXT-98. Forge-published content (M13-EXT-55/57) becomes SDK mods. No raw filesystem/network escape.
##### Player-Facing Impact
Infinite community content — the game lives forever via mods.

---

## M5 — AI (cont.) — Horde Director + Navmesh

#### [M5-EXT-60] Horde Director
> **tags** · ai, combat
> **tl;dr** · M5/M8. Spawns/scales zombie waves; uses M8-EXT-32 funnel pathing; co-op-aware.
> **ctx** · Horde Director -- M5/M8. Part 24 + R43/R48. Reuses M8-EXT-32 (funnel), M8-EXT-51 (scheduled horde), Jolt + M1 SpatialHash.
##### How It Works
Director queries threat (player loot, defense deficit) → spawns waves along M8-EXT-32 funnels; scales with M8-EXT-51 cadence. Co-op: synced via M12-EXT-22.
##### Player-Facing Impact
Intelligent, escalating hordes — not random spam.

#### [M5-EXT-61] Navmesh / A* Pathfinding
> **tags** · ai
> **tl;dr** · M5. Terrain-adaptive navmesh + A*; zombies fall/jump/avoid obstacles.
> **ctx** · Navmesh/A* Pathfinding -- M5. Part 24 + R43. Reuses Jolt collision + M1 SpatialHash broad-phase. Either/or E60: lockstep deterministic vs client-prediction.
##### How It Works
Navmesh baked per chunk (M4 streaming); A* over nodes; terrain-adaptive (slope/height → fall/jump tags). Obstacle avoidance via M1 SpatialHash.
##### Player-Facing Impact
Zombies navigate the real world — climb rubble, avoid your barricades.

#### [M5-EXT-62] Wildlife Ecosystem
> **tags** · ai
> **tl;dr** · M5. Ambient animals (scavengers, infected beasts) for life + food + dread.
> **ctx** · Wildlife Ecosystem -- M5. Part 25. Reuses M5-EXT-60 (horde director) + M5-EXT-61 (navmesh) + Jolt. Either/or E64: passive vs infected-beast.
##### How It Works
Spawn points seeded by biome (M4-EXT-23); animals use M5-EXT-61 navmesh; some variants infected (reuse M8 SEIR). Flee from player/zombie noise (M6.5).
##### Player-Facing Impact
A living world — and a new thing to fear in the dark.

---

## M11 — UI (cont.) — Wristwatch + Dusk Alarm

#### [M11-EXT-63] Wristwatch + Dusk Alarm
> **tags** · ui, horror, items
> **tl;dr** · M11. Diegetic watch; dynamic alarm at dusk; toggleable; scavenged or spawn-chance. Inside, it's your only time cue.
> **ctx** · Wristwatch + Dusk Alarm -- M11. Part 25 + R82 (I Am Legend). Reuses M6.5 audio + M11-EXT-60 diegetic-HUD + M4.5/M9 day-night. Inspired by I Am Legend (watch = going-dark alarm). Your ask.
##### How It Works
Time-of-day crosses dusk threshold (M4.5) → watch plays rising-tension chime (M6.5). Toggle in settings or in-world (wind crown). Item spawns at POIs (M4-EXT-32) or small chance on player (M11-EXT-62). Inside/underground: no sky visible, alarm is the only cue. Feeds M11-EXT-62 dread + M8-EXT-51 horde.
##### Player-Facing Impact
The board-up moment, audible. You feel night fall even when you can't see it.

---

## M12 — Game (cont.) — Build + Browser

#### [M12-EXT-30] Build Placement System
> **tags** · building, gameplay
> **tl;dr** · M12/M3/M5. Player places barricades/walls (ghost-preview, snap, cost); nav re-bakes (M5).
> **ctx** · Build Placement System -- M12/M3/M5. Part 25. Reuses M3 StructuralFatigue + M5 dirty-tile nav re-bake (M5:85) + M8 crafting cost. Either/or E62: free-place vs grid-snap.
##### How It Works
Build mode: select part (M8 cost), ghost-preview snaps to grid/surface, place → Jolt body + M3 StructuralFatigue; M5 flags nav tile dirty → re-bake.
##### Player-Facing Impact
Build your fortress. The horde tests it (M8-EXT-51).

#### [M12-EXT-31] Server Browser / World List
> **tags** · multiplayer
> **tl;dr** · M12. Lists dedicated servers (M12-EXT-22) + Forge worlds (M13-EXT-57); find community/shared maps.
> **ctx** · Server Browser -- M12. Part 25. Reuses M12-EXT-22 (dedicated server) + M13-EXT-57 (published worlds). Either/or E63: LAN vs internet.
##### How It Works
Query registered servers (M12-EXT-22) + Forge publish list (M13-EXT-57); show name/players/ping/mods; join. Either/or E63: LAN-only (safe) vs internet (needs M12-EXT-19 integrity).
##### Player-Facing Impact
Find your friend's world or a community Forge map.

#### [M12-EXT-32] Neutral Haven (Hidden Safe-Town + Casino)
> **tags** · social, world
> **tl;dr** · M12/M8/M4. Hidden faction-neutral town; NPCs don't kill unless provoked; casino = economy + choice.
> **ctx** · Neutral Haven -- M12/M8/M4. Part 27 (your ask). Reuses M4-EXT-32 (POI placement) + M8-EXT-16 (NPCs) + M8-EXT-14 (barter) + M8-EXT-15 (currency) + M8-EXT-53 (standing). Either/or E69/E70.
##### How It Works
Hidden POI (M4-EXT-32) in a "cool town" biome. NPCs run routines (M8-EXT-16), neutral by default; provoke (shoot/steal) → M8-EXT-53 standing drops → they agro. Casino: dice/cards sink currency (M8-EXT-15), pay loot/crafting (M8-EXT-02). Info broker + black-market (M8-EXT-14).
##### Player-Facing Impact
A breath of (tense) civilization. The one place you can lower your guard — if you don't blow it.

---

## M8 — Settlement (cont.) — Factions + Growth

#### [M8-EXT-53] Faction Standing System
> **tags** · social, systems
> **tl;dr** · M8. Per-faction standing tiers; quests fork; prices gate; kill = drop.
> **ctx** · Faction Standing -- M8. Part 26 + R84 (NV). Reuses M8-EXT-14 (barter) + M8-EXT-15 (currency) + M11-EXT-64 (branching quests) + M5-EXT-60 (war spawns). Either/or E67: soft vs hard-war.
##### How It Works
Tiers: idolized/liked/neutral/disliked/warned/hostile. Quest forks read tier; barter prices scale; killing faction member drops standing; hard-mode: faction-war state boosts M5-EXT-60 spawns nearby.
##### Player-Facing Impact
The world remembers what you did. Factions are friends, rivals, or enemies — by your hand.

#### [M8-EXT-54] Settlement Growth Sim
> **tags** · systems
> **tl;dr** · M8. Player grows settlement: attract NPCs, upgrade, economy, services.
> **ctx** · Settlement Growth -- M8. Part 26. Reuses M8-EXT-16 (residents) + M12-EXT-30 (build) + M8-EXT-14/15 (economy) + M8-EXT-29 (walls) + M8-EXT-51 (horde) + M7 (persist). Either/or E68: passive vs quest-gated.
##### How It Works
Settlement level rises with residents (M8-EXT-16) + built structures (M12-EXT-30) + economy (M8-EXT-14/15); unlocks medic/trader/quest-giver; horde (M8-EXT-51) tests walls (M8-EXT-29). Persisted (M7).
##### Player-Facing Impact
Build a real community. It lives or dies by your defense.

---

## M11 — UI (cont.) — Branching Quests

#### [M11-EXT-64] Branching Quest System
> **tags** · ui, narrative
> **tl;dr** · M11/M8. Quests fork (help/kill/sneak/report); write faction standing + world state.
> **ctx** · Branching Quests -- M11/M8. Part 27 + R84 (NV). Reuses M12-EXT-26 (side-quests) + M13-EXT-60 (SLM generator) + M8-EXT-53 (standing) + M7 (world state). Either/or E70.
##### How It Works
Quest node graph with forks; each choice writes M8-EXT-53 standing + M7 world-state delta; no main story (per design) — hubs faction/settlement anchored. SLM (M13-EXT-60) proposes, you curate.
##### Player-Facing Impact
Every choice matters. The story is yours, not a rail.

---

## M8 — Settlement (cont.) — Corps + Psyche + AI War

#### [M8-EXT-55] Corporate Decay Layer
> **tags** · world, narrative
> **tl;dr** · M8/M4. Parody-brands + decaying-corp world layer (Borderlands/WALL-E); loot logic.
> **ctx** · Corporate Decay -- M8/M4. Part 28 (your ask). Reuses M4-EXT-39 (palette) + M4 macro-graph + M8-EXT-02 (craft mats) + M8-EXT-15 (currency) + M6.5 (diegetic jingles). Parody brands (BUY-N-MORE, HYPERION-EX, KOLO-A) — no real trademark.
##### How It Works
World-gen scatters corp remnants by biome (M4-EXT-39): billboards, warehouses, armored trucks, peeling ads. Warehouses = best loot; trucks = rare. Decay = story subtext (over-extraction collapse).
##### Player-Facing Impact
A dead consumer world to pick clean. The corps that ended it are still watching, rusted.

#### [M8-EXT-56] Survivor Psyche (needs + break)
> **tags** · systems
> **tl;dr** · M8/M2. Mood/needs; push too far → break (panic/desert/lash-out). RimWorld-inspired.
> **ctx** · Survivor Psyche -- M8/M2. Part 29 + R86. Reuses M2-EXT-18 (stamina/hunger) + M8-EXT-03 (illness) + M11:162 (stress). Either/or E74: break severity.
##### How It Works
Needs (rest/food/safety/social) feed a mood scalar; low mood → break event (panic flee, ally desert, reckless lash-out). Reuses M2-EXT-18 decay + M11 stress controller.
##### Player-Facing Impact
Your people are human. Push them and they break — manage them, don't grind them.

#### [M8-EXT-57] AI Faction War
> **tags** · systems, ai
> **tl;dr** · M8/M5. Factions fight each other independent of player; fronts shift; exploit/avoid.
> **ctx** · AI Faction War -- M8/M5. Part 29 + R86. Reuses M8-EXT-53 (standing) + M5-EXT-60 (director). NOT a difficulty tier (M1-EXT-36 is the one dynamic diff).
##### How It Works
Factions (M8-EXT-53) run war state machine; director (M5-EXT-60) spawns fronts/missions; player can arms-deal, lure, or hide. Info broker (M12-EXT-32) sells live fronts.
##### Player-Facing Impact
A world at war with itself. You're one survivor in someone else's conflict.

---

## M13 — Modding (cont.) — Lua VM

#### [M13-EXT-61] Lua VM Modding Runtime
> **tags** · modding
> **tl;dr** · M13. Embedded Lua VM (LuaJIT/Lua 5.4 via sol3) runs sandboxed mod scripts; industry standard.
> **ctx** · Lua VM Modding -- M13. Part 29 + R87. Replaces vague "bounded VM" in M13-EXT-59. Reuses M13-EXT-16/19/23 (SLM sandbox) + M7 (persist mods) + M13-EXT-59 (SDK surface). Either/or E73: LuaJIT vs Lua 5.4.
##### How It Works
Game embeds Lua (sol3 binding); mods are .lua files executed in a sandboxed state (no raw FS/network); Forge-published content (M13-EXT-55/57) becomes Lua mods. Exposes M1 event bus + M8-EXT-14/15 economy hooks.
##### Player-Facing Impact
Infinite community content — the game lives forever, safely.

---

## M8 — Settlement (cont.) — Weapon Modding

#### [M8-EXT-58] Weapon Modding System
> **tags** · systems, items
> **tl;dr** · M8/M2.7. Player fills mod slots (receiver/barrel/sight/underbarrel/mag); crafted mods, tradeoffs.
> **ctx** · Weapon Modding -- M8/M2.7. Part 30. Reuses M8:317 Tier-2 kitbashing + M2.7 receiver/barrel/sight structs + M8-EXT-02 (crafting) + M8-EXT-01 (brand-tier). Either/or E75: free vs slot-limited.
##### How It Works
Weapon has slot groups; player attaches crafted mods (M8-EXT-02) → mutates Tier-2 instance (M8:317) within clamped bounds. Tradeoffs: damage↑/handling↓/weight↑. Brand-tier (M8-EXT-01) gates reliability.
##### Player-Facing Impact
Every gun is yours. Build the zombie-mobile's companion piece.

---

## M12 — Game (cont.) — War Map

#### [M12-EXT-33] War Map Overlay
> **tags** · ui, war
> **tl;dr** · M12/M8. Map shows live faction-war fronts (M8-EXT-57) + contested/safe zones.
> **ctx** · War Map -- M12/M8. Part 30. Reuses M8-EXT-57 (AI war) + M8-EXT-53 (standing) + M12-EXT-32 (broker intel) + M11-EXT-60 (diegetic). Either/or E76: full vs fog-of-war.
##### How It Works
Overlay reads M8-EXT-57 war state + M8-EXT-53 standing → draws fronts/contested/safe. Broker (M12-EXT-32) reveals more. Either/or E76: fog-of-war (explore to reveal) vs full tactical.
##### Player-Facing Impact
See the war move. Pick your fights, or get caught in someone else's.

> **Local Co-op (split-screen):** reuse M12 local-split-screen if present, else add `M12-EXT-<next>` at merge (verify against M12 true max 22). Same-screen two-cursor edit; camera splits; shared state.

---

## M13 — Modding (cont.) — AI-Quest Generation

#### [M13-EXT-60] SLM Quest Generator
> **tags** · narrative, ai
> **tl;dr** · M13. SLM generates quest assets from biome context + authored story-arc template; you curate.
> **ctx** · SLM Quest Generator -- M13. R70 hybrid AI-quest (designer assistant + human curation). Reuses M13-EXT-16 (Offline Harness), -17 (Prompt-Template Library), -18 (Schema Validator), -19 (Output→Validation), -21 (Deterministic Seeding), -23 (Provenance Tag). No new AI system.
##### How It Works
For each region, SLM takes (biome context + Part 14 story-arc template + world lore) → generates quest text/fragments/variants. Output schema-validated (M13-EXT-18), determinism-seeded (M13-EXT-21), provenance-tagged (M13-EXT-23). You review pool, keep winners.
##### Player-Facing Impact
200+ quests authored by AI, curated by you — bulk generated, human-shaped.

---

## M12 — Game (cont.) — Quest Placement

#### [M12-EXT-29] Quest Placement Tool
> **tags** · narrative, editor
> **tl;dr** · M12/M4. You place curated quests onto map POIs; validates biome-appropriate + discoverable.
> **ctx** · Quest Placement Tool -- M12/M4. Part 20. Reuses M4-EXT-32 (POI Placement Validator) + Forge editor (M13-EXT-54/55). You drag quests onto sensible POIs.
##### How It Works
Placement UI (in Forge) drops a quest at a POI; validator checks biome-match (M4-EXT-32), no overlap, visibility (R72, far-view impostor Part 6). Quest anchors to POI.
##### Player-Facing Impact
Quests live where they make sense — barricaded house in suburbs, crashed convoy in desert.

---

## M4 — Worldgen (cont.) — Region Sizing + Density

#### [M4-EXT-95] Biome-Region Sizing
> **tags** · general
> **tl;dr** · M4. Formalizes world size / 6 contiguous biome regions; dense not empty. Fallout 4 = ~10 km² yet feels massive via density.
> **ctx** · Biome-Region Sizing -- M4. Part 20. Reuses N2 macro-climate (M4-EXT-93) + M4-EXT-32. Either/or E48: 10 / 25 / 40 km² (Fo4 / Enshrouded EA / full). Density is the metric, not km².
##### How It Works
Macro-climate field (M4-EXT-93) partitions world into 6 regions; each sized for distinction + density. Chunk count scales with size (256m chunks). Streaming (M4-EXT-50) handles it.
##### Player-Facing Impact
Big enough for real biome difference, dense enough to feel alive everywhere — Fo4 proves 10 km² can feel huge.

#### [M4-EXT-96] POI Density Field
> **tags** · general
> **tl;dr** · M4. Targets POI spacing ~100–200m in dense zones, sparser in hostile wilderness (risk/reward).
> **ctx** · POI Density Field -- M4. Part 20 + R72. Reuses M4-EXT-32 POI Placement Validator. Visible (not checklist) via DS2 far-view (Part 6).
##### How It Works
Density field modulates POI spawn probability by zone type (city/suburb dense, wilderness sparse). Each POI validates discoverable (visible at far-view).
##### Player-Facing Impact
World is packed with readable, discoverable POIs — no empty padding, no checklist fatigue.

#### [M4-EXT-97] Topology Import Pipeline
> **tags** · general, editor
> **tl;dr** · M4. Import real OSM road/river + heightmap as base canvas; Forge-edit; procedural dress.
> **ctx** · Topology Import Pipeline -- M4. Part 22 + R76. Reuses M4-EXT-01 macro-graph (import target), Forge M13-EXT-54 (edit), M4 CGA (dress). No new terrain sim.
##### How It Works
OSM vectors + heightmap → feed M4-EXT-01 macro-graph as base (overrides procedural macro where imported). Forge edits layout. M4 facade CGA + M4-EXT-39 palette dress per biome (N2). Seamless with M4-EXT-31 blending.
##### Player-Facing Impact
Detailed real-world maps in minutes — real street grid + your procedural skyscrapers.

---

## M9 — Vehicles (cont.) — Assembly + Interactables

#### [M9-EXT-24] Vehicle Assembly / Customization
> **tags** · vehicles, crafting
> **tl;dr** · M9/M8. Weld modular parts (engine/armor/wheels/boosters/zombie-smash) to car OR motorcycle chassis; parts modify M9 physics.
> **ctx** · Vehicle Assembly -- M9/M8. R74 (Earn to Die) + R75 (Days Gone) + GTA-feel (R75). Reuses M9-EXT-01 (traction), -02 (tire slip), -09 (fuel), -17 (frame shear), -20 (suspension), -05 (aero); M8 crafting; Jolt constraints.
##### How It Works
Chassis = Jolt body; parts = Jolt bodies/constraints welded on. Part stats map to M9 physics params (mass→M9-EXT-17, traction→M9-EXT-01, fuel→M9-EXT-09). Car + motorcycle chassis variants. Player earns parts via M8 economy.
##### Player-Facing Impact
Build your zombie mobile — welded plow, bike, armored hauler. GTA-rival handling via M9 tuning, not arcade.

#### [M9-EXT-25] Universal Interactables
> **tags** · vehicles, physics
> **tl;dr** · M9. Player grabs/moves/welds ANY Jolt body (debris, corpses, props); weld-to-vehicle.
> **ctx** · Universal Interactables -- M9. R77 (Fallout-style). Reuses Jolt (src/physics) + M8 crafting + Forge (place in world).
##### How It Works
Player interaction raycast → Jolt body grab/move; weld constraint to vehicle or world. "Put shit on there and make shit happen" — plow, gun mount, corpse-winching.
##### Player-Facing Impact
Everything is physical. The world is a toy box of rigid bodies you weaponize.

---

## M4 — Worldgen (cont.) — Asset Library Integration

#### [M4-EXT-98] Asset Library Ingest Pipeline
> **tags** · general, assets
> **tl;dr** · M4. Bot pulls photoreal PBR from free libs → transcode BC7/ASTC → bindless + RVT. TI-matching photoreal.
> **ctx** · Asset Library Ingest Pipeline -- M4. Part 23 + R78/R79. Reuses M4-EXT-25 (material compiler + BC7/ASTC), M4.5-EXT-26 (RVT), bindless descriptor buffer (Device.cpp:92). Source: recon/FREE_APIS_2026-07-16.md (Poly Haven/Wikimedia/Sketchfab-PBR, no key).
##### How It Works
Bot pulls Poly Haven/Wikimedia/Sketchfab-PBR via APIs (batched with sleeps, FREE_APIS note) → transcode to BC7/ASTC (M4-EXT-25) → bindless descriptor slot → RVT page (M4.5-EXT-26). One photoreal texture → many variations via M4-EXT-25 node-graph (domain-warped noise) — photoreal base + procedural variation (honors "100% procedural").
##### Player-Facing Impact
Photoreal, TI-style world from free PBR libs — no hand-art, same tech as the renderer.

#### [M4-EXT-99] Photoreal Filter + Provenance
> **tags** · general, assets
> **tl;dr** · M4. Auto-reject stylized (Kenney/OA) from hero pipeline; tag license provenance; validate PBR map set.
> **ctx** · Photoreal Filter + Provenance -- M4. Part 23. Reuses M4-EXT-39 (palette binding). Legal compliance + TI-photoreal gate.
##### How It Works
Ingested asset checked: PBR map-set complete (albedo+normal+rough+AO)? stylized/cartoon? → reject from hero world (allow only for UI/non-hero props). Tag CC0/CC-BY/PD provenance for compliance.
##### Player-Facing Impact
World stays photoreal + legally clean — only TI-matching assets reach the hero pipeline.



---

## M9 — Vehicles (cont.) — Combat + Custom + Scarcity + Fuel

#### [M9-EXT-26] Vehicle Combat
> **tags** · systems, combat
> **tl;dr** · M9/M2. Shoot from vehicles — driver leans, passenger fires, recoil/aim while moving.
> **ctx** · Vehicle Combat -- M9/M2. Part 31. Reuses M9-EXT-24 (assembly) + M9-EXT-25 (interactables) + M2 weapons. Days Gone / SoD2 feel (R93). Either/or E77: full passenger AI gunner vs manual-only.
##### How It Works
Driver leans out one-handed sidearm (limited aim); passenger seat fires primary weapon (full aim). Recoil/aim affected by vehicle speed/terrain. Passenger AI gunner shoots at nearest threat.
##### Player-Facing Impact
Drive-and-shoot, chase-and-escape. The car is a weapon platform, not just a box with wheels.

#### [M9-EXT-27] Death Race Custom Cars
> **tags** · systems, build
> **tl;dr** · M9/M8. Weld weapons/guns/spikes onto vehicle frames. END-GAME hard content.
> **ctx** · Death Race Custom Cars -- M9/M8. Part 31. Reuses M9-EXT-24 (weld) + M9-EXT-25 (interactables). Mad Max / Crossout build. Either/or E78: frame-slot limit (3-5 weld points) vs free-weld.
##### How It Works
Weld points on frame: ram plate (front), roof gunner, spike bumpers, side harpoons. Each weld point has weight/drag tradeoff. Only craftable at high-tier settlement forge (M8-EXT-54).
##### Player-Facing Impact
Your signature death machine. Mad Max meets custom hot-rodding.

#### [M9-EXT-28] Vehicle Scarcity + Loot
> **tags** · loot, world
> **tl;dr** · M9/M4. Every car starts broken; rare working ones are side-quest rewards. Hard to earn.
> **ctx** · Vehicle Scarcity -- M9/M4. Part 31. Reuses M4-EXT-32 (POI) + M8-EXT-54 (settlement) + M12-EXT-26 (side-quests) + M11-EXT-64 (branching). Either/or E79: spawn-rate (very-rare vs ultra-rare).
##### How It Works
World-gen spawns 100% wrecked cars. A working-ish car is a specific side-quest reward (M12-EXT-26) from a faction or settlement. Refuel+repair to keep it running. Lootable wrecks yield parts but not drivetrains.
##### Player-Facing Impact
A working car is a triumph — you earned it, you maintain it, it can die. Every run feels different based on what you find.

#### [M9-EXT-29] Fuel + Durability
> **tags** · systems, survival
> **tl;dr** · M9/M8. Finite gas; car/wheel durability degrades with use/combat; repair at settlement or roadside.
> **ctx** · Fuel + Durability -- M9/M8. Part 31. Reuses M9-EXT-09 (fuel tank) + M9-EXT-25 (roadside repair) + M8-EXT-54 (settlement repair) + M8-EXT-02 (crafting). Either/or E80: durability severity.
##### How It Works
Fuel gauge + condition bar. Gas found at pumps, siphons, rare fuel caches. Tire/engine/drivetrain wear with distance + combat. Repair at settlement forge (M8-EXT-54) or roadside with parts (M9-EXT-25). Cannot grind infinitely — gas runs out.
##### Player-Facing Impact
Every trip matters. You plan routes, carry fuel, and the car is a precious resource you protect.

#### [M12-EXT-34] Permadeath + Endless
> **tags** · systems, meta
> **tl;dr** · M12. Character deleted on death; world persists. No NG+ — endless challenge.
> **ctx** · Permadeath + Endless -- M12. Part 31. Reuses M7 (world persistence) + M1-EXT-36 (one difficulty). No NG+ — the world IS the challenge. Project Zomboid (R91) mode.
##### How It Works
Character save deleted on death (soft "last stand" death-cam before deletion). World state — settlements, factions, war, economy — persists (M7). Next character spawns into the same world at a new start point. One dynamic difficulty (M1-EXT-36) adapts to survivors, not power-level.
##### Player-Facing Impact
Every life is precious. You die, the world does not restart — it remembers you were here. The next character inherits your legacy.

---

## M8 — Settlement (cont.) — Living World / Endgame Divergence

#### [M8-EXT-59] Autonomous Settlement Sim
> **tags** · systems, ai
> **tl;dr** · M8. Settlements run their own AI: NPCs auto-build, trade, recruit, defend. Sim Settlements-style.
> **ctx** · Autonomous Settlement Sim -- M8. Part 32. Reuses M8-EXT-16 (settlement NPCs) + M8-EXT-54 (growth) + M8-EXT-53 (standing) + M8-EXT-56 (psyche) + M8-EXT-02 (crafting) + M8-EXT-29 (walls). Either/or E81: full autonomy vs player-directable.
##### How It Works
Each settlement runs a simulation tick: assess needs (food/safety/population) → auto-assign NPCs to plots (residential/agricultural/industrial/defensive) → build structures from prefab kit → trade excess with connected settlements. Player can influence by gifting resources, assigning priorities, or building specific plots. Plots level up over time (shack → house → fort) based on prosperity. NPCs have individual schedules (M8-EXT-56) that contribute to the settlement output.
##### Player-Facing Impact
Settlements live without you. You are the visitor, fixer, and ally — not the janitor. Every settlement grows differently.

#### [M8-EXT-60] World-State Cascade Engine
> **tags** · systems, world
> **tl;dr** · M8. Every action has ripple effects: blocked trade route → starved town → raid → reputation collapse → faction war.
> **ctx** · World-State Cascade Engine -- M8. Part 32. Reuses M8-EXT-53 (standing) + M8-EXT-57 (faction war) + M8-EXT-54 (growth) + M8-EXT-59 (settlement sim) + M8.5 (economy). Either/or E82: simulated vs event-driven.
##### How It Works
World-state graph tracks nodes (settlements/factions) and edges (trade routes, alliances, wars). Each player action — blocking a road, killing a leader, gifting supplies, destroying a zombie nest — propagates through the graph. Starvation → raid → casualties → reputation loss → abandoned settlement → power vacuum → new faction. Cascades are tracked over days/weeks, not instantly. The director (M5) can inject events that accelerate cascades.
##### Player-Facing Impact
Block a road and a town starves. They attack you. You kill them. Their allies hate you. Trade stops. War starts. Every choice is a stone dropped in a pond.

#### [M8-EXT-61] Trade Route Network
> **tags** · systems, world
> **tl;dr** · M8. Visible roads connect settlements; goods flow; player can protect, block, or establish routes.
> **ctx** · Trade Route Network -- M8. Part 32. Reuses M8-EXT-14 (barter) + M8-EXT-15 (currency) + M8.5 (economy) + M8-EXT-59 (settlement sim) + M8-EXT-60 (cascade engine) + M8-EXT-57 (faction war carries risk). Either/or E83: visible vs abstract.
##### How It Works
Roads (scavenged from M4 world-gen + player-built) connect settlements. Goods flow along routes: food, ammo, meds, materials. Each route has a throughput cap, risk level (zombie ambush, bandit raid), and owner. Player can patrol (lower risk), block (cut supply), establish (connect two allied settlements), or raid (short-term loot, long-term reputation hit). Blocked routes cascade (M8-EXT-60): starved settlement → desperate raids → instability.
##### Player-Facing Impact
You see the supply lines. Cut one and watch a town starve. Protect one and build allies. The economy is visible and touchable.

---

## M5 — Director (cont.) — World-State Events

#### [M5-EXT-63] World-State Director
> **tags** · systems, ai
> **tl;dr** · M5. Director drives events based on world-state, not just player state. Famine, plague, migration, mega-horde.
> **ctx** · World-State Director -- M5. Part 32. Reuses M5-EXT-60 (director) + M8-EXT-60 (cascade engine) + M8-EXT-59 (settlement sim) + M8-EXT-03 (SEIR illness) + M8-EXT-04 (cure) + M8-EXT-51 (horde). Either/or E84: full world-state weighting vs limited.
##### How It Works
Director reads world-state graph: if settlement overcrowded → plague event (SEIR, M8-EXT-03); if food shortage across region → famine + migration wave; if faction war unresolved → refugee caravan; if zombie population critical → mega-horde (M8-EXT-51). Events are chosen by what makes the most interesting emergent story (RimWorld storyteller model), not by difficulty scaling. Player actions affect the probability of each event type.
##### Player-Facing Impact
The world reacts to its own problems, not just yours. Starve a settlement and the director sends a plague. Ignore the zombie buildup and the horde comes.

---

## M12 — Game (cont.) — Endgame Divergence

#### [M12-EXT-35] Endgame Divergence Engine
> **tags** · systems, meta, narrative
> **tl;dr** · M12. World-state accumulates over long playthrough → triggers one of multiple endgame scenarios. Every playthrough ends differently.
> **ctx** · Endgame Divergence Engine -- M12. Part 32. Reuses M8-EXT-60 (world-state cascade) + M8-EXT-59 (settlement sim) + M8-EXT-57 (faction war) + M5-EXT-63 (world-state director) + M7 (persistence) + M12-EXT-34 (permadeath). Either/or E85: deterministic (cumulative score) vs emergent (state-machine).
##### How It Works
World-state graph tracked over entire playthrough — settlement count, faction standing, zombie population, trade health, player kills, infrastructure built. After threshold (X hours played or Y world events), the engine evaluates the graph and selects an endgame scenario from a pool. Examples:
- **Humanity Rebuilds**: AI-controlled settlements dominate, zombies contained, faction alliances strong → society reforms. End-state: faction alliance forms a provisional government. Player can join or leave.
- **Empty World**: Zombie population critical, most settlements collapsed, few survivors. Caravan/wanderer visits are rare lifelines. End-state: traveler journal — player is the last human, records what happened.
- **Nuclear Winter**: Player-triggered missile launch (Metro 2033 choice) → massive crater, fallout zone, radiated terrain. End-state: the crater is a permanent scar. Player can survive in bunker or flee.
- **Zombie Ascendancy**: Zombies evolve (M5-EXT-60 director escalation), become dominant species, humans are prey. End-state: final broadcast — player documents the fall of humanity.
- **Faction Victory**: One faction conquered all others, player must choose — serve, flee, or resist. End-state: new world order, for better or worse.
- **The Exodus**: Mega-horde consumes entire map, player must evacuate on a crafted vehicle/convoy to a new map region. End-state: you escaped, but the old world is gone.
##### Player-Facing Impact
You start the same as everyone else. Ten thousand choices later, your ending is yours alone. Trade routes, settlements, wars, mercy, cruelty — it all adds up. You never know which endgame is coming until it arrives.

#### [M12-EXT-36] Nuclear Ending (Metro 2033)
> **tags** · systems, narrative, world
> **tl;dr** · M12. Player can trigger a missile launch that creates a massive crater + fallout zone. Permanent map scar.
> **ctx** · Nuclear Ending -- M12. Part 32. Reuses M12-EXT-35 (endgame engine) + M8-EXT-60 (cascade) + M4-EXT-39 (palette) + M4.5 (RVT crater). Either/or E86: player-only trigger vs AI-faction can also launch.
##### How It Works
Player finds a missile silo (rare POI, M4-EXT-32) or nuclear warhead cache. Choice: launch to wipe a region (kills zombies, also kills settlements, creates fallout zone) or disarm. If launched: crater replaces terrain (M4.5 RVT deformation), fallout zone spreads, NPCs die, faction standing destroyed, permanent radiated biome. The crater is a permanent landmark — a scar on the shared world. Director (M5-EXT-63) may trigger faction-launched nukes if war escalates.
##### Player-Facing Impact
The nuke is a reset button that costs everything. The crater is a permanent reminder. Metro 2033 made you choose — your game makes you live with it.---

## M2 — Character (cont.) — Persistent + Procedural NPCs

#### [M2-EXT-76] Persistent Cross-World NPC Registry
> **tags** · systems, persistence, npc
> **tl;dr** · M2/M7. Named NPCs persist across all worlds/saves via StableId; survive permadeath, travel between servers.
> **ctx** · Persistent Cross-World NPC Registry -- M2/M7. Part 33. Reuses M7-EXT-01 (StableId) + M7-EXT-19 (save compat) + M2-EXT-18 (physiology) + M5.1 (companion AI). Two tiers: persistent (player-made / notable) vs generated (world-filler, respawn on world-gen). Either/or E87: global registry (cloud) vs local-per-save.
##### How It Works
Two NPC classes. (1) **Persistent NPCs**: players author or promote NPCs (companions, settlement leaders) → assigned a global StableId, stored in a cross-save registry (M7 backend). They persist across permadeath (M12-EXT-34) and appear in any world the player joins (co-op/shared-seed, M4-EXT-92). (2) **Generated NPCs**: world-filler pop (guards, traders, residents) spawned per-world from M2-EXT-77; not persisted globally, regenerate on new world. Persistent NPCs carry their personality/wealth/standing (M8-EXT-62) across worlds; generated NPCs are deep but disposable.
##### Player-Facing Impact
The companion you raised in one life is still there next life. A settlement mayor you built up exists in your friend's world too. Your people are permanent.

#### [M2-EXT-77] Procedural NPC Detail Generator
> **tags** · systems, npc, procgen
> **tl;dr** · M2. Generated NPCs get full depth: name, personality facets, job, schedule, personal plot, wealth/power, appearance — not cardboard.
> **ctx** · Procedural NPC Detail Generator -- M2. Part 33. Reuses M2-EXT-18 (needs) + M8-EXT-16 (residents) + M8-EXT-56 (psyche) + M8-EXT-62 (stratification) + M4-EXT-92 (seed). DF personality-facet model (51 traits + values + goals + memories) + CK3 courtier opinion/rank. Either/or E88: trait depth (full DF 51-facet vs compact 12-facet).
##### How It Works
On settlement spawn, generator rolls: (1) **Name** from culture-weighted name tables (M4 region theme). (2) **Personality**: DF-style facets (altruism, greed, bravery, diligence, etc.) → biases behavior + dialogue tone. (3) **Job**: fits settlement need + stratification tier (M8-EXT-62) — top tier = mayor/warlord; mid = trader/foreman; bottom = scavenger/slave. (4) **Schedule**: daily routine (sleep/work/eat/social) mapped to plot + job. (5) **Personal plot**: a seeded micro-arc (debt, lost family, ambition) that the cascade engine (M8-EXT-60) can escalate. (6) **Wealth/Power**: determines land/plots (M8-EXT-64). (7) **Appearance**: procedural face/clothes from M4 palette + status. All deterministic: `npcSeed = hash(worldSeed, StableId)`, then every roll (name/personality/job/plot/look) is a seeded PRNG draw — identical NPC every session, regenerates identically after permadeath (M12-EXT-34).
##### Player-Facing Impact
Every face in the crowd is a person with a name, a grudge, a job, a story. The world is populated, not decorated.

---

## M8 — Settlement (cont.) — NPC Society / Shanty / Land

#### [M8-EXT-62] NPC Social Stratification
> **tags** · systems, npc, society
> **tl;dr** · M8. Settlements have class tiers (elite/mid/shanty) driven by wealth + power; determines plots, housing, quests.
> **ctx** · NPC Social Stratification -- M8. Part 33. Reuses M2-EXT-77 (NPC gen) + M8-EXT-16 (residents) + M8-EXT-59 (auto-sim) + M8-EXT-64 (land) + M8-EXT-53 (standing). CK3 courtier rank/opinion model. Either/or E89: hard caste (locked) vs fluid (merit/money moves you).
##### How It Works
Each NPC has Wealth (resources owned) + Power (position/influence) scores. Tiers: **Elite** (mayor, warlord, top trader — owns prime land M8-EXT-64, lives in fortified core), **Middle** (foreman, medic, skilled trader — decent housing), **Bottom/Shanty** (scavenger, refugee, slave — stacked housing M8-EXT-63). Stratification drives: plot allocation, quest giver importance, prices (M8-EXT-14 barter gates on tier), and standing (M8-EXT-53) weights. Player actions can shift an NPC's tier (save them → rise; rob them → fall).
##### Player-Facing Impact
A real society with haves and have-nots. Where you stand in it changes what you can do, buy, and who talks to you.

#### [M8-EXT-63] Shanty Town / Vertical Slums
> **tags** · world, settlement, procgen
> **tl;dr** · M8/M4. Bottom-tier NPCs build dense vertical shanty stacks — depth, not flat sprawl.
> **ctx** · Shanty Town / Vertical Slums -- M8/M4. Part 33. Reuses M8-EXT-62 (stratification) + M8-EXT-64 (land) + M4-EXT-25 (material compiler) + M3 (structures) + M4-EXT-39 (palette). Watabou-style vertical city stacking. Either/or E90: procedural-stacked vs player-built.
##### How It Works
Bottom-tier zone generates 3-level vertical slum clusters: scrap huts stacked on top of each other (reuse M3 structure prefabs + M4-EXT-25 materials), connected by ladders/walkways, crammed into a small footprint. Density driven by settlement population vs land cap (M8-EXT-64). Each floor = a household (M2-EXT-77 NPC family). Verticality creates navigation depth (rooftop routes, ground alleys), ambush layers, and a distinct visual identity vs the elite core.
##### Player-Facing Impact
You climb through stacked shacks to reach the fixer on the third floor. The slums have more life per square meter than anywhere else — and more danger.

#### [M8-EXT-64] Land & Plot Allocation by Status
> **tags** · systems, settlement
> **tl;dr** · M8. Land area + plot count per NPC/faction scaled by wealth/power tier.
> **ctx** · Land & Plot Allocation -- M8. Part 33. Reuses M8-EXT-62 (stratification) + M8-EXT-59 (auto-sim) + M12-EXT-30 (build) + M4-EXT-32 (POI). Either/or E91: fixed-tier allotment vs dynamic (market buys land).
##### How It Works
Settlement land budget (from M8-EXT-59 growth + M4-EXT-32 space) divided by tier: Elite gets large walled plots (forecourt, multiple buildings), Middle gets standard lots, Bottom gets stacked micro-plots (M8-EXT-63). Factions (M8-EXT-53) claim territory by power. Player can buy/earn land (M8-EXT-15 currency) to move up. Allocation re-evaluated on cascade events (M8-EXT-60) — war loser loses land.
##### Player-Facing Impact
Wealth is visible in the ground plan. The mayor's compound versus the stacked slum tells you everything about who holds power.

#### [M8-EXT-65] NPC Daily Life Simulation
> **tags** · systems, npc, sim
> **tl;dr** · M8. NPCs run autonomous daily routines (work/social/sleep) that build their personal plot over time.
> **ctx** · NPC Daily Life Simulation -- M8. Part 33. Reuses M2-EXT-77 (NPC gen) + M8-EXT-16 (residents) + M8-EXT-56 (psyche) + M8-EXT-59 (auto-sim) + M8-EXT-62 (stratification). Either/or E92: full routine sim vs keyframe-only.
##### How It Works
Each NPC ticks a daily schedule: wake → job (mapped to their role/plot) → eat → socialize (gossip spreads standing M8-EXT-53 changes between NPCs — a rumor of a blocked trade route M8-EXT-61 can flip a whole tier's mood) → personal-plot action (debt payment, ambition step) → sleep. Routine contributes to settlement output (M8-EXT-59) and advances their personal plot (M2-EXT-77); when a plot hits a threshold it injects an event into the cascade engine (M8-EXT-60). Psyche (M8-EXT-56) can break a routine (panic, desertion). Player can observe or interrupt — talking to an NPC mid-routine can divert it.
##### Player-Facing Impact
The town breathes on a schedule. You watch a merchant open shop, a guard rotate, a lover sneak across the slum. Life happens without you.

---

## M4 — Worldgen (cont.) — Signature Landmark POIs

#### [M4-EXT-100] Stadium & Signature Landmark POIs
> **tags** · world, poi, procgen
> **tl;dr** · M4. Hand-authored-feel signature POIs (stadium, tower, mall) as quest hubs + landmark variety.
> **ctx** · Stadium & Signature Landmark POIs -- M4. Part 33. Reuses M4-EXT-32 (POI validator) + M4-EXT-93 (macro-climate) + M4-EXT-25 (materials) + M4-EXT-39 (palette) + M11-EXT-61 (quest fragments) + M12-EXT-26 (side-quests). Either/or E93: fixed roster vs seeded variant pool.
##### How It Works
Beyond generic POIs, world-gen places a roster of signature landmarks scaled to region: **Stadium** (arena — quest hub: fights, refugee camp, horde finale), **Megamall** (multi-tier loot labyrinth), **Radio Tower** (broadcast/quest beacon), **Hospital** (medquests), **Prison** (faction stockade), **Subway Station** (underground route). Each is a large prefab (M3 structures) placed via M4-EXT-32 with region-themed skin (M4-EXT-39). The **Stadium** is the marquee hub: arena fights (spectator betting reusing M8-EXT-15 currency), refugee-camp side-quests (M12-EXT-26), and a natural horde-night finale stage (M8-EXT-51) — a set-piece the director (M5-EXT-63) can trigger. Variant detail = `hash(worldSeed, POI_id)` drives layout scatter, loot-table seed (M4-EXT-33), and graffiti/palette so no two stadiums are identical.
##### Player-Facing Impact
A stadium to clear, a mall to loot, a tower to broadcast from. Real places with history, not copy-paste ruins. Quest designers get anchors.