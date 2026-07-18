# User-Requested Proposals (preserved from unauthorized edits)
> These 23 blocks were added to spec/ without GO. Preserved here per protocol.
> Each will be folded into the correct milestone on approved port plan.


--- M11-EXT-65 from M11.md ---
#### [M11-EXT-65] G-DREAD: Tension Loop Design

> **tags** · HUD, horror, audio
> **tl;dr** · A design pillar that frames existing systems (darkness, infection, audio) as felt dread — not numbers. Dusk = the board-up moment (I Am Legend).
> **ctx** · G-DREAD: Tension Loop Design -- M11. New block. Implements the G-DREAD gap from the SIX-DECISIONS plan. Ties to M10 (day/night) + M8-EXT-03 (SEIR) + M4.5 (atmo) + M6.5 (audio) + M11-EXT-39 (haptics).
> **meta** · depends-on: [M10-EXT-23], [M8-EXT-03], [M4.5-EXT-26], [M6.5-EXT-12], [M11-EXT-39] · depended-by: [M11-EXT-66]

##### Systems Touched
M11 HUD/feedback + M10 time-of-day + M4.5 atmosphere + M6.5 acoustics. A `dreadScalar` (0..1) is computed from ambient darkness, nearby-zombie proximity (M5), and infection progress (M8-EXT-03). It drives vignette darkness, low-frequency audio swell (M6.5), controller pulse (M11-EXT-39), and desaturation — NOT a HUD number.

##### Math
`dread = clamp(w_dark * darkness + w_prox * nearbyThreat + w_inf * infectionProgress, 0, 1)`
- `darkness = 1 - daylight(M10)`; `nearbyThreat = zombiesWithin(20m) / 20m_cap`
- Dusk ramp: `dread += duskWindow ? 0.3 : 0` (the board-up beat)

##### Algorithm
Each tick compute `dreadScalar`. At dusk (M10 day/night crossing), spike dread + play a rising-tension chime (M6.5). As darkness deepens, vignette closes + ambient audio lowers then swells with sub-bass. Nearby zombie proximity tightens the vignette. Infection progression (M8-EXT-03 SEIR) adds a sickly desaturation. Controller pulses on dread tier-up. No numeric HUD — dread is somatic.

##### Examples
- Player survives to dusk in a safe house → dread spikes → chime → they board windows (emergent behavior, not instructed).
- Player deep in a dark sewer with 3 zombies nearby → vignette tight, pulse racing → tension without a health bar.
- Player infected, day 2 → world desaturates slightly → they feel the clock (G-INFECTFEEL).

##### Failure Modes
- Dread perma-high in safe interiors → gate `nearbyThreat` to actual proximity, not zone type.
- Audio swell clips → cap sub-bass gain, duck music.
- Dusk spike every cycle annoys → only spike if player is exposed (outdoors/ungated).

##### Player-Facing Impact
The game is scary because the systems agree. Darkness, death, infection — all felt, not read. You board up at dusk because it feels right.



--- M11-EXT-66 from M11.md ---
#### [M11-EXT-66] G-INFECTFEEL: Infection as Dread

> **tags** · HUD, horror, infection
> **tl;dr** · The SEIR infection (M8-EXT-03) is surfaced as a visible, timed countdown — a clock you feel ticking, not a stat.
> **ctx** · G-INFECTFEEL: Infection as Dread -- M11. New block. Implements the G-INFECTFEEL gap. Extends M8-EXT-03 (SEIR) + M11-EXT-65 (dread). Inspired by PZ/TLoU infection tension, tone only.
> **meta** · depends-on: [M8-EXT-03], [M11-EXT-65] · depended-by: [M11-EXT-67]

##### Systems Touched
M11 feedback + M8-EXT-03 SEIR model. As the player progresses E→I (exposed→infected), the `infectionProgress` (0..1) drives visual + audio feedback: vein darkening on hands/arms (M4.5 RVT), heartbeat audio swell, occasional vision tunnel, and a hard `dread` contribution (M11-EXT-65).

##### Math
`infectionProgress = (tSinceBite − latentPeriod) / (incubationPeriod − latentPeriod)` clamped [0,1]
- `dread += 0.4 * infectionProgress` (feeds M11-EXT-65)
- Visual: `veinDark = mix(skin, corpseBlue, infectionProgress)` on exposed skin meshes

##### Algorithm
On bite (M8-EXT-03 exposure event), start the latent timer. During incubation, ramp `infectionProgress`; surface it via skin shader (M4.5) + heartbeat audio (M6.5) + vision tunnel on progress > 0.7. Antidote/cure (rare, a journey) resets progress to 0. If progress hits 1 → death (M12-EXT-34 permadeath) OR zombie-turn (design choice, M5 archetype). The point: the player feels the clock from bite to consequence.

##### Examples
- Player bitten at noon → latent 6h → by dusk, hands bruise, heartbeat audible → they panic-search for meds.
- Player finds antibiotics day 1 → progress resets → relief, but the scar remains (M7-EXT-21 STATE_OVERRIDE).
- Player ignores it → day 3, vision tunnels, dread max → turns (or dies).

##### Failure Modes
- Progress visible as a number → violates "felt not read" → only shader/audio, no HUD stat.
- Cure too common → no tension → gate cure behind dangerous quest (M12-EXT-38 loot).
- Turn-on-death vs permadeath conflict → design flag, default permadeath (M12-EXT-34).

##### Player-Facing Impact
One bite is a countdown you feel in your chest. The cure is out there, rare, and worth the run. Infection is horror, not a debuff icon.



--- M11-EXT-67 from M11.md ---
#### [M11-EXT-67] G-JUICE: Game Feel

> **tags** · HUD, juice, feedback
> **tl;dr** · Screen shake, hit-pause, damage flinch, smooth health interpolation — the tactile layer that makes combat land.
> **ctx** · G-JUICE: Game Feel -- M11. New block. Implements the G-JUICE gap (R42 research). Extends M11-EXT-30 (screen shake exists) + M2.9 (damage events) + M6.5 (impact VFX).
> **meta** · depends-on: [M11-EXT-30], [M2-EXT-59], [M6.5-EXT-01], [M11-EXT-39] · depended-by: -

##### Systems Touched
M11 feedback + M2.9 ballistics + M6.5 gore. On `DamageEvent` (M2.9), apply: hit-pause (3–5 frame freeze on heavy hit), screen shake (trauma model, M11-EXT-30), blood spray (M6.5-EXT-01/13), target flinch animation, and health-bar smooth interpolation (lerp, not snap).

##### Math
`trauma = min(trauma + hitWeight, 1)`; `shake = trauma^2 * maxAngle`; `trauma *= 0.9 per frame`
- `hitPause(ms) = hitWeight > 0.5 ? 80 : 30` (heavy hit = longer freeze)
- `healthBar += (hp - healthBar) * lerpRate` (smooth, ~0.15/frame)

##### Algorithm
On confirmed `DamageEvent`, trigger hit-pause (brief `timeScale` dip), add trauma (shake), spawn blood VFX (M6.5), play target flinch. Health UI lerps toward true HP so damage feels weighty, not instant. Near-misses (bullet whiz) get a smaller shake + audio tick. Kills get a stronger shake + kill-feed (M11-EXT-30). All gated by settings (M11-EXT-37) for accessibility.

##### Examples
- Player lands a shotgun blast → hit-pause + big shake + blood spray + flinch → impact lands.
- Player takes a glancing hit → small shake + health lerp dips → feels hurt, not numbers.
- Near-miss bullet → tiny shake + whiz audio → tension without damage.

##### Failure Modes
- Hit-pause on every frame of a multi-hit → stutter → cap hit-pause frequency (max 1/100ms).
- Shake accumulates to max permanently → trauma decays (0.9/frame) handles it.
- Juice masks real damage state → health lerp still converges to true HP, never lies.

##### Player-Facing Impact
Combat has weight. Every shot lands, every hit hurts, every kill matters. The game feels like a game, not a spreadsheet.




--- M12-EXT-37 from M12.md ---
#### [M12-EXT-37] Encounter Zones (Layered Density)

> **tags** · world, density, ai
> **tl;dr** · Each region gets a curated density + threat budget so the world is never empty; cleared zones repopulate on cooldown, not instantly.
> **ctx** · Encounter Zones (Layered Density) -- M12. New block. Steals Fallout 4's encounter-zone idea to answer the No Man's Sky "vast-but-empty" failure mode the user flagged. Ties to M5 (director/horde) + M8 (settlement) + M4-EXT-96 (POI density).
> **meta** · depends-on: [M5-EXT-10], [M5-EXT-63], [M4-EXT-96], [M8-EXT-59] · depended-by: [M7-EXT-22], [M12-EXT-38]

##### Systems Touched
M12 game-feel + M5 director + M4 worldgen. Each zone (city block, suburb, wilderness, landmark) carries a `densityCurve` (base spawn cap) × `threatBudget` (zombie archetype mix) × `repopCooldown`. Director (M5-EXT-10) spawns within budget; cleared zone repopulates after cooldown via M5-EXT-11 waves.

##### Math
`spawnCap(zone) = densityCurve(zoneType) * (1 + difficultyMult)`
`repopTimer = repopCooldown * (1 - clearProgress)` — faster repop if zone was barely cleared
- `threatBudget`: shambler 0.6, runner 0.25, sleeper (M5-EXT-64) 0.15 — mixed, not uniform

##### Algorithm
On chunk stream-in, zone reads its `densityCurve` + `threatBudget` from M4-EXT-96. Director maintains live count vs cap; spawns toward cap during lulls, surges during horde-nights (M8-EXT-51). Player clears a zone → `clearProgress` set → repopTimer starts → after cooldown, director trickles new zombies back in (believable, not instant respawn). Dense zones stay dense; wilderness stays sparse-but-present. No dead empty streets.

##### Examples
- Player clears a city block (12 zombies → 0) → repopTimer 3 days → trickle of 1–2/day returns → block feels alive but not punishing.
- Wilderness zone: low densityCurve → sparse wanderers, not a horde field.
- Horde-night hits a settlement → Director surges spawnCap ×3 within threatBudget → defend or die.

##### Failure Modes
- Repop too fast → zone never feels cleared → cap repopTimer floor (e.g. 2 days min).
- Density curve too high in wilderness → empty biome feels crowded → separate wilderness curve (low).
- Director overshoots cap during surge → clamp to `spawnCap * surgeMax` (e.g. ×3).

##### Player-Facing Impact
The world is alive everywhere. A cleared street breathes again. A forest hides few. You are never safe from nothing, never drowned in everything.



--- M12-EXT-38 from M12.md ---
#### [M12-EXT-38] Threat-Scaled Loot Budget

> **tags** · loot, risk, economy
> **tl;dr** · Dangerous zones (high density, irradiated, military) yield better loot tables; safe zones yield scraps. Risk/reward, not checklist.
> **ctx** · Threat-Scaled Loot Budget -- M12. New block. Implements G-LOOTRISK from the SIX-DECISIONS plan. Ties to M8-EXT-01 (brand-tier loot) + Appendix C (socio-economic tags) + M12-EXT-37 (encounter zones).
> **meta** · depends-on: [M8-EXT-01], [M4-EXT-33], [M12-EXT-37] · depended-by: [M8-EXT-51]

##### Systems Touched
M12 loot economy + M8 brand-tier + M4 loot tables. Each zone's `threatBudget` (M12-EXT-37) maps to a loot-tier multiplier applied at container/POI roll time (M4-EXT-33).

##### Math
`lootTier(zone) = baseTier + k * threatScore(zone) + irradiatedBonus + militaryBonus`
- `threatScore = zombieCount/spawnCap + archetypeDangerAvg`
- Roll: `item = LootTable(lootTier(zone), socioTag(M4-EXT-33))` — high tier → military_surplus/rare meds; low tier → improvised/scrap

##### Algorithm
At POI/container spawn, compute `lootTier` from the zone's threat (M12-EXT-37) + flags (irradiated from M12-EXT-36 nuke, military from landmark type). Roll loot against `LootTable(lootTier)` weighted by socio-economic tag (Appendix C). Dangerous zone → military_surplus, antibiotics, ammo crate. Safe suburb → canned food, tools, scrap. Player chooses: raid the hospital (high threat, high reward) or scavenge the quiet street (low threat, low reward).

##### Examples
- Player raids a military checkpoint (high threatBudget) → lootTier high → rolls body armor, ammo, grenades.
- Player loots a quiet house (low threat) → lootTier low → rolls canned beans, a wrench, a book (boredom cure, M8-EXT-56).
- Irradiated crater zone (M12-EXT-36) → irradiatedBonus + still-high threat → best loot, worst environment.

##### Failure Modes
- Loot tier scales infinitely → cap `lootTier` at max (no god-tier drops in safe zones).
- Socio tag missing on a POI → default to zone threat tier, log warning.
- Player farms one dangerous POI repeatedly → POI exhausts (M7-EXT-21 STATE_OVERRIDE looted) → must move on.

##### Player-Facing Impact
The good stuff is where the dead are. Risk is the economy. You don't grind loot — you earn it by walking into the mouth of hell.



--- M12-EXT-39 from M12.md ---
#### [M12-EXT-39] G-BASE: Player-Built Fortifications

> **tags** · base, build, defense
> **tl;dr** · Player-placeable walls, spikes, sandbags, barricades, turrets; horde-night assaults them. The agency layer over settlement AI.
> **ctx** · G-BASE: Player-Built Fortifications -- M12. New block. Implements the G-BASE gap from the SIX-DECISIONS plan. Uses M8-EXT-32 (funnel pathing) + M5 (horde AI) + M7-EXT-20/21 (persist placement) + M4-EXT-101 (precombine).
> **meta** · depends-on: [M8-EXT-32], [M5-EXT-10], [M7-EXT-20], [M4-EXT-101], [M8-EXT-51] · depended-by: [M12-EXT-37]

##### Systems Touched
M12 base-building + M8 settlement + M5 horde + M7 persistence + M4 precombine. Player places fortification entities (wall/spike/sandbag/turret) via the interaction shell (M2-EXT-78/79). Each placed object gets a `WorldRefID` (M7-EXT-20) and writes to the chunk change-buffer (M7-EXT-21). Horde-night (M8-EXT-51) directs zombies along M8-EXT-32 funnels toward the base; fortifications block/redirect them.

##### Math
`defenseScore = Σ (wallIntegrity * coverage) + turretDPS * arcs`
`hordeBreach = hordeSize > defenseScore * breachThreshold`
- Each fortification: `wall` blocks pathing (M5-EXT-07 navmesh), `spike` damages on contact, `sandbag` provides cover + partial block, `turret` (M1-EXT-31/M9) auto-fires within arc.

##### Algorithm
Player enters build mode (interaction shell), selects a fortification, places it (snaps to grid, validates footprint). Placement allocates `WorldRefID`, writes change-buffer, flags precombine cluster dirty (M4-EXT-101). On horde-night, Director (M5-EXT-10) spawns the wave along M8-EXT-32 funnels; zombies path to the base, blocked/redirected by walls, damaged by spikes/turrets. Base holds if `defenseScore` exceeds `hordeBreach`; else zombies pour through. Built bases persist (M7-EXT-21).

##### Examples
- Player walls a house doorway + spikes outside + a turret on the roof → horde-night hits → zombies funnel into spikes, turret cuts them → base holds.
- Player builds nothing → horde-night → zombies enter freely → player must fight or flee.
- Player builds a maze of sandbags → zombies path around (M5-EXT-07) → longer kill- corridor → player picks them off.

##### Failure Modes
- Build exploits (wall off the whole map) → cap fortification count per chunk + funnel requirement (M8-EXT-32).
- Precombine thrash from heavy building → M4-EXT-101 cluster budget handles it.
- Turret aimbot unfair → turret DPS tuned below player DPS, limited arcs.

##### Player-Facing Impact
The apocalypse is what you build. A boarded house is a home. A walled compound is a kingdom. Defend it, or lose it.



--- M12-EXT-40 from M12.md ---
#### [M12-EXT-40] G-EMERGE: Emergent Storytelling

> **tags** · narrative, emergence
> **tl;dr** · Player choices ripple through settlement/brand/NPC state; the world remembers and reacts. Emergence over deep simulation (DayZ lesson).
> **ctx** · G-EMERGE: Emergent Storytelling -- M12. New block. Implements the G-EMERGE gap from the SIX-DECISIONS plan. Ties to M8-EXT-56 (psyche) + M8-EXT-53 (standing) + M12-EXT-26 (side-quests) + Appendix A (narrative).
> **meta** · depends-on: [M8-EXT-56], [M8-EXT-53], [M12-EXT-26], [M8-EXT-16] · depended-by: -

##### Systems Touched
M12 narrative + M8 settlement/NPC + Appendix A journal. A `ChoiceResolver` applies player quest/action choices as deltas to settlement state (defense, wealth, population), brand-tier (M8-EXT-01), NPC standing (M8-EXT-53), and Appendix A journal entries.

##### Math
`ApplyChoice(questId, branch) → Δ(settlementDefense, brandTier, standing, journalEntry)`
- `settlementMood = f(Σ recentChoices)` → drives NPC dialogue tone (M8-EXT-16) + spawn events

##### Algorithm
On quest resolve or major action (help vs scavenge, warn vs silence, save vs abandon), `ChoiceResolver` writes deltas. Settlements thrive or rot from player decisions; NPCs reference past choices in dialogue; Appendix A generates journal entries from the accumulated state. The story is the sum of player agency, not a scripted rail (per design: no main story). Co-op: choices sync via M12 shared save.

##### Examples
- Player saves a trapped NPC → standing↑, that NPC becomes a loyal settler → settlement grows.
- Player loots a refuge instead of helping → standing↓, journal logs "the betrayal" → NPCs cold, fewer recruits.
- Player clears a zone then builds a base → settlement expands there (M12-EXT-39) → emergent town.

##### Failure Modes
- Choice deltas conflict (two players, co-op) → last-writer-wins via StableId owner (M7-EXT-20).
- Journal spam from minor choices → only major branches write entries.
- Mood calculation swings → clamp + hysteresis.

##### Player-Facing Impact
You are not playing a story. You are making one. The world remembers, reacts, and tells it back to you.



--- M12-EXT-41 from M12.md ---
#### [M12-EXT-41] No-Forced-Narrative Design Pillar

> **tags** · design, agency, anti-rail
> **tl;dr** · The game never forces the player down a story, quest, or behavior. The only mandatory loop is survival; everything else is emergent and opt-in. Quests start *naturally* from world state — no accept prompt, no active tracking required, most are untimed with no penalty.
> **ctx** · No-Forced-Narrative Design Pillar -- M12. New block. Direct user directive: "I want everyone's game to be different, so don't force a player to do anything except the gameplay loop" + "some quests naturally started, not an accept, but you don't have to do them or track them actively; some timed with penalty, most won't." Codifies that into a binding design constraint. Ties to M12-EXT-26 (side-quests, discovery-based) + M12-EXT-40 (G-EMERGE) + M13 (SLM proposes, player curates).
> **meta** · depends-on: [M12-EXT-26], [M12-EXT-40], [M13-EXT-60] · depended-by: -

##### Systems Touched
M12 game design + M13 SLM + M12-EXT-26 quests. A hard rule: no scripted main-story gate, no forced quest acceptance, no behavior mandate (build/fight/flee/avoid all valid). The SLM (M13) may *propose* arcs; the player *curates* (M12-EXT-26) — never auto-commits. Survival (M2-EXT-18) is the only loop the game requires; how you survive is yours.

##### Math
`forcedNarrativeEvents = 0` (invariant). `agencyScore = f(distinctPlayerChoices)` — higher when the player's path diverges from any template.
- Quest trigger is passive: `worldState crosses threshold → quest available` (no Accept button). Declining = simply not engaging; no penalty. Timed quests (minority) flag a soft deadline + consequence; majority are open-ended.

##### Algorithm
All narrative is discovery-based (M12-EXT-26, no waypoints). The SLM generates situational arcs from world state; the player accepts, ignores, or rewrites them via G-EMERGE choices. Horde-nights (M8-EXT-51), encounters (M12-EXT-37), and settlements (M8-EXT-59) happen regardless of player participation — the world runs without the player. The game measures a session by what the *player* did, not what was scripted.

##### Examples
- Player ignores every quest, builds a silent cabin in the woods, farms, survives 100 days → valid win-state (survived).
- Player accepts one quest, abandons it mid-way → no penalty, world adjusts (G-EMERGE).
- Player never builds, never fights, just evades and scavenges → valid (tense, but valid).

##### Failure Modes
- SLM auto-starts a quest → violates opt-in → gate all SLM offers behind explicit Accept.
- Survival loop becomes a chore (forced grinding) → tune M2-EXT-18 rates to ~2–3 day buffer.
- "Everyone's game different" compromised by shared seed → shared worldSeed (M4-EXT-92) gives common map, but player *actions* diverge → difference is in play, not map.

##### Player-Facing Impact
Your game is yours. No rail, no script, no "you must." Survive your way — builder, wanderer, warlord, ghost. The apocalypse is a sandbox with teeth.



--- M12-EXT-42 from M12.md ---
#### [M12-EXT-42] FO3 / NV-Style Free-Roam Movement & Direction-Agnostic Play

> **tags** · design, movement, open-world
> **tl;dr** · Fallout 3 / New Vegas movement ethos: go any direction, the world responds, no corridor. Honors classic isometric roots (FO1/FO2) via optional tactical/top-down mode.
> **ctx** · FO3 / NV-Style Free-Roam Movement -- M12. New block. User: "I love the Fallout movement and go-in-any-direction from FO3 and NV." Captures that as a movement + world-response design pillar. Ties to M2 (player controller) + M4 (seamless map) + M5 (roaming hordes) + M10 (streaming).
> **meta** · depends-on: [M2-EXT-64], [M4-EXT-34], [M5-EXT-10], [M10-EXT-31] · depended-by: -

##### Systems Touched
M12 design + M2 controller + M4 worldgen + M5 AI + M10 streaming. The player moves in **first-person (primary) with third-person as a 1:1 parity toggle** — both are fully AAA, identical mechanics, no other perspective (no top-down, no tactical cam; those violate the strictly FPS/TPS mandate). The world is a seamless shared map (M4-EXT-34, M10-EXT-31 streaming) with no invisible walls or quest-gating of geography. Direction-agnostic: north, south, into the sewers, onto the rooftops — all valid.

##### Math
`worldResponse(pos, dir) = Σ (encounterZone(pos) + hordePressure(pos) + POIdensity(pos))` — independent of approach vector
- `blockedTiles = 0` outside physical geometry (no narrative gates) — verified by M4-EXT-34 stream bounds

##### Algorithm
Movement is unconstrained by narrative: the player can walk past the "main" threat, skip the "key" location, or never visit the nuke (M12-EXT-36). World systems (M5 director, M12-EXT-37 zones, M8 settlements) respond to *position*, not *progress*. FO3/NV/RDR2/Skyrim taught: the joy is the walk, the detour, the abandoned house you chose to enter. First-person is the default lens; third-person is a 1:1 toggle (same animations, same hitboxes, same feel) for players who prefer it — never a different game.

##### Examples
- Player ignores the stadium (M4-EXT-100), walks due east for 2 km, finds a quiet pond, builds there → valid, world responds to that position.
- Player never touches a quest, just free-roams scavenging → encounters still happen (M12-EXT-37), survival still pressures (M2-EXT-18).
- Player prefers third-person for a sewer fight → toggles to TPS (1:1, same sim) → same game, their preferred lens.

##### Failure Modes
- Narrative gate blocks a region → violates direction-agnostic → all geography open from spawn (M4-EXT-34).
- Top-down mode desyncs from FPS sim → same ECS, only camera differs (no second sim).
- Free-roam + horde director overwhelms → M12-EXT-37 density caps prevent wall-to-wall.

##### Player-Facing Impact
Go anywhere. The map is real, not a hallway. FO3's "I wonder what's over there" is the core loop. The classics' tactical soul lives in the optional view.



--- M12-EXT-43 from M12.md ---
#### [M12-EXT-43] Engine Influences — Salvage the Useful, Reject the Hated

> **tags** · design, influences, fallout
> **tl;dr** · Explicit Fallout lineage policy: steal FO3/NV strengths (free-roam, emergent faction war, companion legacy, player-driven consequence); salvage FO4/FO76's *useful* systems (settlement building, deep crafting, weapon modding, modding API) while rejecting their hated parts (FO4 precombine fragility → fixed in M4-EXT-101; FO76 always-online → blocked by local-first M7).
> **ctx** · Engine Influences — Salvage the Useful, Reject the Hated -- M12. New block. User: "love FO3/NV + originals, hate FO4 + FO76, but they might have useful stuff — add those, flesh them out." This is the written policy so future edits stay consistent.
> **meta** · depends-on: [M12-EXT-41], [M12-EXT-42], [M4-EXT-101], [M7-EXT-20], [M8-EXT-29], [M8-EXT-58], [M13-EXT-55] · depended-by: -

##### Systems Touched
M12 design policy + M8 (settlement/crafting/weapon-mod) + M4-EXT-101 (precombine fix) + M7 (local-first) + M13 (modding). Documents which borrowed systems are in, which are out, and why.

##### Math
`adopt(score) = useful − hated`; only `adopt > 0` enters spec.
- FO4 settlement building: useful (base agency, M12-EXT-39) − hated (precombine cliff) → ADOPT building, FIX cliff (M4-EXT-101).
- FO76 always-online: useful (co-op) − hated (mandatory cloud) → ADOPT co-op (M12), REJECT cloud (local-first M7).
- FO4 dialogue-wheel: hated (cartoonish) → REJECT, keep M13 text/choice.

##### Algorithm
**Adopt (and flesh out):**
- FO3/NV: free-roam (M12-EXT-42), faction war with consequences (M8-EXT-57), companion loyalty/legacy (M8-EXT-16, M12-EXT-34 permadeath stakes), karma/reputation (M8-EXT-53), faction-shaped world state.
- FO4 (useful only): settlement building → M12-EXT-39 + M8-EXT-29 walls; crafting depth → M8-EXT-02; weapon modding → M8-EXT-58; robust modding API → M13-EXT-55 (Forge). Precombine fragility → FIXED via edit-aware re-bake (M4-EXT-101).
- FO76 (useful only): co-op peer model → M12 (friends co-op, local-first); timed events → M5-EXT-63 director variants; free-placement bases → M12-EXT-39.

**Reject (explicit):**
- FO4 precombine fragility (whole-cell disable on edit) → already solved (M4-EXT-101 edit-aware clusters).
- FO76 mandatory always-online / monthly sub → blocked by local-first (M7, E87).
- FO4 dialogue-wheel / voiced-protagonist cartoonish tone → keep M13 text, silent or typed protagonist.
- FO4/76 quest-marker hand-holding → use M12-EXT-26 no-waypoint discovery.

##### Examples
- Player builds a C.A.M.P.-style outpost (FO76 useful) that persists exactly (M7-EXT-21) and doesn't nuke framerate (M4-EXT-101) → adopted + fixed.
- Player plays fully offline, solo (FO76 rejected part) → local-first works, no account needed.
- Player gets faction-war consequences (FO3/NV) → siding with a faction closes others' quests (M8-EXT-57) → adopted.

##### Failure Modes
- "Useful" slips into "hated" (e.g. building becomes FO4 grind) → cap build scope (M12-EXT-39 funnel limit).
- Modding API becomes FO76 paid-creator store → keep M13-EXT-55 local/creator-owned, no store mandate.
- Reject list drifts → this block is the written gate; any FO4/76 borrow must cite "useful − hated > 0".

##### Player-Facing Impact
The best of every Fallout, none of the worst. FO3's freedom, NV's consequences, the classics' tactics — plus FO4/76's building and modding, with their cliffs and cages removed. Your apocalypse, your rules.



--- M12-EXT-44 from M12.md ---
#### [M12-EXT-44] Natural-Start Quests (no accept, no tracker, mostly untimed)

> **tags** · quest, design, anti-rail
> **tl;dr** · Quests begin from world events you witness or stumble into — no "Accept?" prompt, no mandatory tracker, no waypoint. Most are open-ended with no penalty for ignoring; a minority are timed with real consequences.
> **ctx** · Natural-Start Quests -- M12. New block. Direct user directive: "I want some quests to be naturally started, not an accept — but you don't have to do them or track them actively. Some will be timed and have a penalty, most won't." Codifies quest UX. Extends M12-EXT-41 (no forced narrative) + M12-EXT-26 (discovery quests) + M13 (SLM proposes).
> **meta** · depends-on: [M12-EXT-41], [M12-EXT-26], [M13-EXT-60], [M11-EXT-53] · depended-by: -

##### Systems Touched
M12 quest design + M13 SLM + M11 POI markers. A quest becomes *available* when world state crosses a threshold the player can observe (a scream, a note, a blocked road, a dying NPC). No popup, no accept. The player may engage or walk away. A passive journal (Appendix A) records what they've seen — readable, never a forced tracker. Timed quests (e.g. a collapsing building, a hostage about to die) flag a soft deadline via ambient cues (M6.5 audio, M11-EXT-55 HUD), not a UI timer.

##### Math
`questState ∈ {dormant, available, engaged, resolved, expired}`
- `available` triggers on `worldEvent observed` (not player action) → no Accept gate
- `timedFraction = |timedQuests| / |allQuests|` kept small (target ≤ 15%) per user "most won't [be timed]"
- `ignorePenalty = 0` for untimed; `consequence ∈ {reputation, worldState, lifeLoss}` for timed

##### Algorithm
The SLM (M13-EXT-60) watches world state and emits *available* quests when conditions arise (a settlement starving, a zombie horde forming, a stranger trapped). These surface as world cues (M11-EXT-53 POI pin, M6.5 ambient), never a modal. The player chooses to engage. If they do, the quest tracks in the passive journal only. Timed quests (minority) escalate via environmental pressure (M11-EXT-55), not a countdown bar. Ignoring an untimed quest = nothing happens; ignoring a timed one = the consequence plays out (NPC dies, building falls) and the world moves on (M12-EXT-45).

##### Examples
- Player hears a child scream from a basement → walks over → finds a trapped kid → quest *available* (no popup) → frees them → settlement standing up (M8-EXT-53). Or walks away → kid is eaten → world continues.
- Player finds a ransom note on a corpse → no "Accept Rescue Mission" → they can act or not. No tracker forces them.
- A dam is about to break (timed) → ambient creaks + M11-EXT-55 alert → player has minutes to evacuate/brace → ignore → flood reshapes the valley (M12-EXT-45).

##### Failure Modes
- Player misses every quest (too subtle) → world cues must be legible (audio + visual + POI pin), not hidden.
- Timed quests feel unfair → keep ≤15%, telegraph clearly via M11-EXT-55, never a hidden timer.
- Passive journal becomes a tracker → journal is read-only recall, not an objective list; no auto-nav.

##### Player-Facing Impact
The world hands you stories. You pick them up or you don't. No accept button, no blinking objective. The apocalypse unfolds whether you watch or not.



--- M12-EXT-45 from M12.md ---
#### [M12-EXT-45] Living, Dynamic World (drastic map changes)

> **tags** · world, dynamic, simulation
> **tl;dr** · The map itself changes over time — collapses, floods, burns, rebuilds, zombie-swarmed. A living world that reacts to the director, the player, and its own systems.
> **ctx** · Living, Dynamic World -- M12. New block. Direct user directive: "I want drastic map changes throughout the game — a living, dynamic world." Ties to M4 (worldgen) + M5 (director/hordes) + M7 (persistence + RVT deformation) + M12-EXT-36 (nuke) + M12-EXT-37 (encounter zones).
> **meta** · depends-on: [M4-EXT-34], [M5-EXT-10], [M7-EXT-21], [M4.5-EXT-26], [M12-EXT-36], [M12-EXT-37] · depended-by: [M12-EXT-44]

##### Systems Touched
M12 world sim + M4 worldgen + M5 director + M7 persistence (change-buffer) + M4.5 RVT (terrain deformation). The world is not static geometry — it is a simulation that mutates. Director events (M5-EXT-10), player actions (M7-EXT-21), and environmental systems drive persistent map changes: building collapses, forest fires spread, floods reshape valleys, zombie hordes strip a district, settlements rebuild (M8-EXT-59), nukes crater a region (M12-EXT-36).

##### Math
`mapState(t) = applyEvents(worldSeed base, Σ directorEvents[0..t], Σ playerActions[0..t])`
- `deformation` stored in M4.5 RVT + M7-EXT-21 change-buffer (persists across sessions)
- `eventMagnitude ∈ {minor (looted building), major (district stripped), drastic (dam breach, nuke crater)}`

##### Algorithm
The director (M5-EXT-10) schedules world events on a slow clock: a neighborhood slowly overtaken by zombies (M12-EXT-37 density shift), a fire spreading through a dry district (M4.5 VFX + navmesh update), a bridge collapsing under horde weight. Player actions also mutate the map: a blown dam floods the valley (M4.5 RVT deform + M7-EXT-21), a cleared zone repopulates (M12-EXT-37). All changes write to the change-buffer (M7-EXT-21) so they persist — the world you return to is not the world you left. Co-op: both peers see the same mutated world (M7-EXT-20 stable refs).

##### Examples
- Player ignores a city for 30 days → director fills it with a zombie district (M12-EXT-37) → returning, the streets are wall-to-wall dead, loot picked clean.
- Player blows a dam to escape a horde → valley floods → M4.5 RVT deforms terrain → that low road is now underwater (persists).
- A forest fire (director event) burns a biome → ash biome spawns → different loot, different threats.
- Player's settlement (M8-EXT-59) grows a walled town → map permanently shows their mark (M7-EXT-21).

##### Failure Modes
- Constant change = chaos, no stable home → cap event rate; some regions stable (player hubs).
- Deformation desync in co-op → M7-EXT-20 stable refs + change-buffer replay keep peers in sync.
- Performance from live terrain morph → M4.5 RVT streaming deforms in pages, not whole-world.

##### Player-Facing Impact
The world is alive. It grows, burns, floods, and remembers. You left a town — you come back to ruins or a fortress. The map is a character, not a backdrop.

**Hybrid rate model (user-approved):** Most regions undergo **slow drift** — a district slowly overtaken by zombies, a settlement gradually rebuilding. **Drastic spikes** hit on director triggers (famine, plague, horde, nuke) or major player actions (blown dam, burned forest). Player hubs are **protected** — your home base stays stable unless you act. Events are permanent via M7-EXT-21 change-buffer. This gives a living world without constant fatigue.



--- M4-EXT-101 from M4.md ---
#### [M4-EXT-101] Edit-Aware Precombine Bake

> **tags** · world, render, perf
> **tl;dr** · Bake static chunk geometry into frozen combined meshes; tag each cluster by its static-only refs so a player edit invalidates only that cluster (not the whole cell).
> **ctx** · Edit-Aware Precombine Bake -- M4. New block. Steals Fallout 4's precombine idea but fixes its famous fragility: FO4 disables precombines for the whole cell on any edit → perf cliff. We re-bake only the touched cluster. Ties to M7-EXT-20 (stable refs) + M7-EXT-21 (change-buffer).
> **meta** · depends-on: [M7-EXT-20], [M7-EXT-21], [M4-EXT-25], [M4.5-EXT-26], [M8-EXT-60] · depended-by: [M7-EXT-22]

##### Systems Touched
M4 worldgen + M4.5 material/RVT + M1 render. On chunk stream-in, static refs (no player-edit flag) are merged into per-cluster frozen meshes (precombine) + previs occlusion. Each cluster stores the list of `WorldRefID`s it baked. A player edit (M7-EXT-21 MOVED/PLACED/REMOVED) flags the owning cluster dirty.

##### Math
`cluster.dirty = any(ref.stateFlags.edited for ref in cluster.refs)`
`reBake(cluster) = Merge(staticRefs(cluster) where !edited)` — edited refs excluded, become dynamic entities.
- Cost: `O(refsInCluster)`, not `O(allRefsInCell)` (FO4's flaw).

##### Algorithm
At chunk bake time, partition static refs into spatial clusters. Bake each cluster's geometry into one combined mesh + previs. Store `cluster.refList`. Runtime: if a ref is edited (player moves/places/removes), mark its cluster dirty → async re-bake (off-frame, M1 secondary CB) excluding the edited ref (it becomes a live ECS entity). Unedited clusters stay frozen → draw calls stay low. This makes player-built bases (M12-EXT-37 fortifications) cheap: only the wall the player just placed re-bakes.

##### Examples
- Player builds a 10-wall base → 10 clusters flagged dirty over time → each re-bakes independently → base stays at near-precombine draw cost.
- Player moves one couch in a furnished house → only that couch's cluster re-bakes → rest of house frozen.
- FO4对比: FO4 would disable ALL precombines in the house → ZE re-bakes 1 cluster → no cliff.

##### Failure Modes
- Cluster too large → one edit re-bakes huge mesh → size clusters by ref-count budget (e.g. ≤64 refs/cluster).
- Async re-bake race with render → double-buffer cluster mesh, swap on completion.
- Edited ref left in frozen mesh → exclude edited refs from bake (already handled by `!edited` filter).

##### Player-Facing Impact
Build anywhere. The world stays fast. Fallout's precombine cliff is gone — your fortification doesn't nuke the framerate.



--- M4-EXT-102 from M4.md ---
#### [M4-EXT-102] Previs Occlusion Bake

> **tags** · world, render, occlusion
> **tl;dr** · GPU-driven per-cluster visibility precomputation; reuses M4.5 occlusion pipeline so unedited clusters cull cheaply.
> **ctx** · Previs Occlusion Bake -- M4. New block. Pairs with M4-EXT-101 (precombine). Fallout previs idea, modernized on the Vulkan/GPU-driven stack. Ties to M4.5 (RVT/occlusion) + M1 render.
> **meta** · depends-on: [M4-EXT-101], [M4.5-EXT-26], [M1-EXT-08], [M8-EXT-60] · depended-by: [M7-EXT-22]

##### Systems Touched
M4 worldgen + M4.5 occlusion + M1 bindless. Each precombine cluster gets a previs volume (occlusion bounds). Runtime GPU-driven culling tests cluster previs vs camera → skips whole clusters.

##### Math
`visible(cluster) = OcclusionTest(cluster.previsVolume, cameraFrustum, hiZ)`
- Reuse M4.5 hi-Z depth pyramid (M4.5-EXT-something) for cluster-level occlusion.
- Previs volume = AABB of baked cluster geometry.

##### Algorithm
At bake time (with M4-EXT-101), compute each cluster's previs AABB. Runtime: GPU-driven cluster culling (M1 bindless + indirect draw) tests previs against hi-Z + frustum → only visible clusters dispatch draws. Edited clusters (M7-EXT-21) fall back to per-object culling until re-baked. This is FO4 previs without the edit-fragility: only dirty clusters lose previs, not the cell.

##### Examples
- Player inside a building → exterior clusters occluded by walls → GPU skips them → interior renders at full speed.
- Player edits one wall → that cluster loses previs, others keep it → minimal overdraw.
- Far-view (DS2-style) → distant clusters previs-culled behind hills → no pop, no waste.

##### Failure Modes
- Previs volume too tight → visible cluster culled (hole) → pad AABB by max-LOD extent.
- hi-Z not ready on first frame → fallback to frustum-only cull until pyramid warms.
- Cluster re-bake invalidates previs → recompute AABB on re-bake (M4-EXT-101 step).

##### Player-Facing Impact
See far, render cheap. The city is there, but only what you can see costs you. Fallout's occlusion, modernized.



--- M5-EXT-64 from M5.md ---
#### [M5-EXT-64] Sleeper Zombie Spawn & Wake System

> **tags** · AI, spawn, horror
> **tl;dr** · A fraction of interior zombies spawn Dormant (slumped/leaning); they wake on sound, proximity, light, or damage. Buildings are unpredictable.
> **ctx** · Sleeper Zombie Spawn & Wake System -- M5. New block. Fills the "sleeper" gap vs Project Zomboid / Dying Light. Ties to M5-EXT-13 (Dormant state) + M5-EXT-12 (emerge anim) + M4 (interior placement).
> **meta** · depends-on: [M5-EXT-13], [M5-EXT-12], [M4-EXT-32], [M5-EXT-14] · depended-by: [M5-EXT-13], [M11-EXT-62]

##### Systems Touched
M5 spawn pipeline + FSM. On worldgen, interior cells (M4-EXT-32 POI placement) roll `sleeperChance` (default 0.30) per zombie → `wakeFlags` bitmask. Director (M5-EXT-10) respawns dormant pool on a cooldown after a building is cleared.

##### Math
`wakeScore = max(soundLevel/wakeThresh, proximityPenalty, lightOnFace, damageTaken)`
- `sleeperChance = 0.30` interior, `0.0` exterior (sleepers only spawn indoors)
- `wakeThresh`: whisper 0.05, door-slam 0.4, gunshot 0.9, proximity < 2 m = 1.0, flashlight-on-face = 0.7

##### Algorithm
At spawn, set `state = Dormant` and `wakeFlags`. Each tick while Dormant: poll sound (M5-EXT-14), proximity (player within 2 m), light (M10 day/night or player flashlight hitting face), damage. If any exceeds its threshold → `Dormant → Idle` (M5-EXT-13) and play emerge animation (M5-EXT-12). Cleared building stays dormant-free for `respawnCooldown` days, then Director repopulates the dormant pool. Stealth takedown (M5-EXT-19) on a Dormant zombie = instant kill, no sound — high risk/reward for clearing.

##### Examples
- Player enters a quiet house; 3 dormant zombies lean in corners. Player moves silently → passes. Player knocks a shelf → door-slam sound → all 3 wake.
- Player shines flashlight on a dormant zombie's face → lightOnFace 0.7 > wakeThresh → wakes, grab animation.
- Player sneaks behind a dormant zombie, hold-breath, takedown → instant kill, no alert.

##### Failure Modes
- Dormant zombies block navmesh baking (M5-EXT-01) → treat Dormant as static obstacle until woken.
- Respawn cooldown too short → building never feels "cleared" → tune per POI size.
- Wake chain reaction (one wakes, screams, wakes all) → cap scream propagation to 1 ring.

##### Player-Facing Impact
A cleared room can still kill you. Search every corner. Move quiet. The dark is not safe.



--- M5-EXT-65 from M5.md ---
#### [M5-EXT-65] Zombie Scent / Pheromone Trail

> **tags** · AI, perception, scent
> **tl;dr** · Player blood, open wounds, and sweat emit a scent value diffused downwind; zombies chemotax up the gradient. Rain and water wash it away.
> **ctx** · Zombie Scent / Pheromone Trail -- M5. New block. Fills the "smell/pheromone" gap. Ties to M2-EXT-80/81 (injury/wound) + M10 (weather/wind) + M5-EXT-09 (smellScore).
> **meta** · depends-on: [M2-EXT-80], [M2-EXT-81], [M10-EXT-23], [M5-EXT-09] · depended-by: [M5-EXT-09], [M5-EXT-13]

##### Systems Touched
M5 scent field. A coarse grid (cell = 4 m) per chunk stores `scentValue` diffused each tick by wind direction (M10 weather). Player emits scent from wound state (M2-EXT-80/81) + exertion. Zombies sample the field in M5-EXT-09.

##### Math
`scentEmit = baseEmit * woundSeverity * (1 - hygiene)`; `baseEmit`: freshBlood 1.0, oldBlood 0.4, sweat 0.3, rottingGear 0.2
- Diffusion: `scent[cell] += wind * (neighborUpwind - scent[cell]) * dt * diffuseRate`
- Decay: `scent *= exp(-decayRate * dt)`; rain (M10) multiplies decayRate ×5
- `smellScore = scentSample(pos) * windAlignmentFactor` (M5-EXT-09)

##### Algorithm
Each tick, sources (bleeding players, carcasses) inject `scentEmit` into their cell. The field diffuses downwind and decays. Zombies in M5-EXT-09 sample `scentSample(pos)`; if above `smellThreshold`, they steer toward the gradient (chemotaxis) even without sight/hearing. Player countermeasures: submerge in water (wash), cover with mud, wear scent-blocking gear (hazmat/raincoat), or move upwind.

##### Examples
- Player takes a deep bite, bleeds, stands downwind of a pack → scent ramps → pack converges up-gradient within 30 s.
- Player wades through a river → scent washed → trail breaks → pack loses track.
- Player mud-camouflages → scentEmit ×0.3 → survives closer passes.

##### Failure Modes
- Scent field O(n) per zombie per tick → sample only 1 cell (no loop) → O(1).
- Field stale after player leaves → decay handles it; verify decayRate > emitRate at rest.
- Wind direction jump → smooth wind vector (M10) before diffusion step.

##### Player-Facing Impact
You can't see the smell, but they can. A bleeding player is a beacon. Clean up, cover up, or get downwind.



--- M5-EXT-66 from M5.md ---
#### [M5-EXT-66] Stealth Approach System

> **tags** · AI, player, stealth
> **tl;dr** · Player noise + visibility meters gate detection; crouch/prone + cover + hold-breath enable silent takedowns on unaware zombies.
> **ctx** · Stealth Approach System -- M5. New block. Fills the "stealth approach" gap. Ties to M5-EXT-09 (perception) + M5-EXT-13 (states) + M2-EXT-18 (encumbrance) + M10 (day/night).
> **meta** · depends-on: [M5-EXT-09], [M5-EXT-13], [M2-EXT-18], [M10-EXT-23] · depended-by: [M5-EXT-09], [M5-EXT-19]

##### Systems Touched
M5 player-stealth model. Player emits `noiseLevel` (movement × surface × encumbrance) + `visibility` (light × stance × cover × camo). Both feed zombie `detect` in M5-EXT-09. Takedown resolves via M2.9 damage event.

##### Math
`noiseLevel = moveSpeedNorm * surfaceNoise * (1 + encumbrancePenalty)`
- `surfaceNoise`: grass 0.4, concrete 0.8, metal 1.2, snow 0.6
- `visibility = lightLevel * stanceMult * coverMult`; `stanceMult`: stand 1.0, crouch 0.5, prone 0.25
- `takedownWindow`: behind + within 1.5 m + holdBreath > 0.8 s → instant kill, no sound

##### Algorithm
Per tick, compute player `noiseLevel` + `visibility`. Zombies within range in M5-EXT-09 compare against their thresholds. Crouch reduces both; prone minimizes; full cover (wall) blocks LOS entirely. A stealth takedown requires: zombie in Dormant/Idle/Wander (not Alert), player behind (angle > 120°), within 1.5 m, hold-breath held > 0.8 s → silent instant kill (M2.9). Failed takedown (zombie Alert) → it turns + Attack. Noise from the takedown itself (body fall) emits a small SoundEvent (M5-EXT-14) that can alert neighbors.

##### Examples
- Player crouch-walks on grass behind an Idle zombie at 1 m, holds breath → takedown → silent kill.
- Player sprints on metal past a Wander zombie → noiseLevel high → Alert → Chase.
- Player prone in tall grass at dusk (low light) → visibility 0.1 → passes 3 m from a shambler unseen.

##### Failure Modes
- Takedown through a wall (LOS not checked) → require LOS-clear in takedown check.
- Hold-breath exploit (perma-hold) → stamina drain forces breath release (M2-EXT-18).
- Noise from takedown alerts whole block → cap emitted SoundEvent amplitude low.

##### Player-Facing Impact
The quiet approach is its own weapon. Move slow, stay low, watch the wind, and the dead won't know you're there — until they're already down.

---



--- M7-EXT-20 from M7.md ---
#### [M7-EXT-20] Stable WorldRefID Registry

> **tags** · persistence, save, identity
> **tl;dr** · Every world object gets a stable WorldRefID (chunk-based, not ECS entityId) so it survives reloads and co-op sessions unchanged.
> **ctx** · Stable WorldRefID Registry -- M7. New block. Extends M7-EXT-07 (delta persistence) with a stable identity layer so player-placed/dropped/moved objects keep one identity across save/load. Fallout FormID idea, modernized.
> **meta** · depends-on: [M7-EXT-07], [M4-EXT-34], [M8-EXT-60] · depended-by: [M7-EXT-21], [M7-EXT-22], [M4-EXT-101]

##### Systems Touched
M7 persistence core. A `WorldRefID = (chunkX, chunkY, localIndex)` or content-hash FormID. Reference table maps `WorldRefID → {archetype, transform, owner, stateFlags}`. ECS entities are spawned *from* the reference table on chunk load — entityId churn never loses identity.

##### Math
`WorldRefID = hash(worldSeed, chunkX, chunkY, localIndex)` (deterministic, content-addressable)
`RefTable[WorldRefID] = {archetype, transform, ownerStableId, stateFlags}`
- Player-placed object: `localIndex` allocated from a per-chunk free-list; persists in chunk change-buffer (M7-EXT-21).

##### Algorithm
On chunk stream-in, base worldgen produces static refs (loot, props) with stable IDs derived from seed. Player actions (drop/pick/place/move) allocate a new `WorldRefID` and write it to the chunk change-buffer. On save (M7-EXT-07), only the change-buffer + ref table deltas serialize. On load, base gen is deterministic from seed; change-buffer replays; ECS entities spawn from the resolved ref table. Co-op peers share the same `WorldRefID` space so an object placed by player A exists identically for player B.

##### Examples
- Player drops a rifle on a rooftop → allocated `WorldRefID`, written to chunk change-buffer → weeks later, same rooftop, same rifle, same spot.
- Player builds a barricade → `WorldRefID` + transform persisted → co-op partner sees it in the exact place.
- Reload after crash → ref table replays → no object duplicated or lost.

##### Failure Modes
- `localIndex` collision on heavy player building → per-chunk free-list with wrap + uniqueness check.
- Ref table bloat from thousands of dropped items → tombstone collected items; cap per-chunk refs.
- Co-op ID race (both players place same tick) → server/peer arbitrates by `StableId` owner precedence.

##### Player-Facing Impact
The world remembers what you did. Drop it, build it, move it — it stays. Your mark on the world is permanent, not a session illusion.



--- M7-EXT-21 from M7.md ---
#### [M7-EXT-21] Persistent Chunk Change-Buffer (exact-spot store)

> **tags** · persistence, save, streaming
> **tl;dr** · Each streamed chunk carries a delta log of moved/removed/placed/state-overridden objects; load = base gen + replay → object at exact same spot.
> **ctx** · Persistent Chunk Change-Buffer (exact-spot store) -- M7. New block. Extends M7-EXT-07 with a per-chunk delta log specialized for world-object placement. This is the "leave an item, come back, exact same spot" guarantee.
> **meta** · depends-on: [M7-EXT-07], [M7-EXT-20], [M4-EXT-34], [M8-EXT-60] · depended-by: [M4-EXT-101], [M7-EXT-22]

##### Systems Touched
M7 save/load + M4 chunk streaming. Each chunk owns a `ChangeBuffer` list: `{WorldRefID, op, transform, stateFlags}`. Ops: `PLACED`, `MOVED`, `REMOVED`, `STATE_OVERRIDE` (damaged, looted, bled). Serialized compressed (M7-EXT-08 dict).

##### Math
`loadState(chunk) = RebuildBase(worldSeed, chunkX, chunkY) then ∀ δ∈ChangeBuffer: Apply(δ)`
- `Apply`: `PLACED` → spawn ref at transform; `MOVED` → update ref transform; `REMOVED` → tombstone ref; `STATE_OVERRIDE` → set flags.
- Save size = `O(|ChangeBuffer|)`, independent of world size (matches M7-EXT-07's O(|Δ|)).

##### Algorithm
On chunk unload, serialize its `ChangeBuffer` (compressed) to the save store. On load, rebuild base chunk from seed (deterministic, M4-EXT-34) then replay the buffer in order. Because Jolt is `JPH_CROSS_PLATFORM_DETERMINISTIC` + `JPH_DOUBLE_PRECISION` (llms.txt rule 3), base gen is bit-identical across sessions → replayed deltas land objects at the exact same transform. Co-op: the buffer is the shared delta both peers apply.

##### Examples
- Player moves a couch 3 m → `MOVED` delta with new transform → reload → couch exactly 3 m from original.
- Player loots a drawer → `STATE_OVERRIDE(looted)` → reload → drawer still open, empty.
- Player kills zombie, corpse flattens (M7-EXT-01) → `STATE_OVERRIDE(corpseFlat)` → reload → flattened corpse persists.

##### Failure Modes
- Replay order dependence (MOVE before PLACE) → buffer is append-only, ops reference existing refs; validate ref exists before MOVED.
- Base gen drift after engine update → pin worldgen version in save header (M7-EXT-19 migration).
- Buffer corruption → CRC per buffer; on fail, rebuild from checkpoint (M7-EXT-07 step 3).

##### Player-Facing Impact
Your stuff is where you left it. Always. The apocalypse persists — your fortifications, your stashes, your mistakes.



--- M7-EXT-22 from M7.md ---
#### [M7-EXT-22] Bit-Exact World Transform Determinism

> **tags** · persistence, determinism, math
> **tl;dr** · Stored transforms reproduce to the centimeter across sessions/machines because Jolt determinism makes base gen + delta replay bit-exact.
> **ctx** · Bit-Exact World Transform Determinism -- M7. New block. Closes the "exact spot" guarantee mathematically. Depends on llms.txt rule 3 (Jolt determinism) + M7-EXT-21 change-buffer.
> **meta** · depends-on: [M7-EXT-21], [M4-EXT-34], [M8-EXT-60] · depended-by: [M4-EXT-101], [M12-EXT-37]

##### Systems Touched
M7 save/load + M4 worldgen + Jolt physics. The resolved transform of every persisted object is stored in the change-buffer as `transform = {pos: vec3, rot: quat, scale: vec3}`.

##### Math
`transform_load = transform_base(gen(seed, chunk)) ⊕ replay(Δ)` where `⊕` is deterministic float replay under `JPH_CROSS_PLATFORM_DETERMINISTIC` + `JPH_DOUBLE_PRECISION`.
- Position error bound: `‖pos_load − pos_placed‖ < 1e-6 m` (double precision, no cross-platform drift).
- Because base gen is seed-deterministic and delta replay is order-deterministic, `pos_load ≡ pos_placed` to the bit.

##### Algorithm
On placement, store the object's resolved `transform` (post-physics-settle) in the change-buffer. On load, base gen reproduces the same initial transform, then replay applies the stored delta → identical result. Co-op peers and reloads agree to the centimeter because both run the same deterministic float path. No floating-point ambiguity: double precision + deterministic broadphase (M5-EXT-01) + locked solver iterations.

##### Examples
- Player places a sniper nest on a rooftop ledge → transform stored → 3 sessions later, same ledge, same aim line.
- Co-op: player A places a trap; player B's client computes the same transform from the shared delta → no desync.
- Determinism lockstep (M12 co-op) relies on this for object positions to stay in sync.

##### Failure Modes
- Solver iteration count differs across builds → lock `JPH` iteration constants in build config (llms.txt rule 3) → non-negotiable.
- Stored transform from a pre-determinism build → migration marks those as `approximate`, re-snaps on next settle.
- Scale drift on procedural meshes → store scale as discrete LOD tier, not continuous float.

##### Player-Facing Impact
The world is reproducible. Your traps, your nests, your grave markers — they are exactly where you made them, on every machine, every time.



--- M7-EXT-23 from M7.md ---
#### [M7-EXT-23] WorldMemory (RDR2-style reactivity, no popups)

> **tags** · memory, reactivity
> **tl;dr** · Systems remember player actions across sessions; NPCs reference past deeds without UI popups — you see it in their tone and behavior.
> **ctx** · WorldMemory -- M7. New block. Implements RDR2's "non-scripted interactions, NPC remembers you" principle. Reuses M7-EXT-20 (stable refs) + M7-EXT-21 (change-buffer) + M8-EXT-60 (world-state graph). Stores player deeds as `{StableId, action_type, timestamp, location}`; NPCs sample on encounter.
> **meta** · depends-on: [M7-EXT-20], [M7-EXT-21], [M8-EXT-60] · depended-by: [M8-EXT-16], [M11-EXT-65]

##### Systems Touched
M7 persistence + M8 standing + M11 status UI. A deed-log indexed by `StableId` stores `{action: HELPED_FACTION, timestamp, location}` etc. On NPC spawn/read (M8-EXT-16 residents), sample recent deeds → adjust dialogue tone, quest offers, price modifiers. No popups — tone shift is the feedback.

##### Math
`deedScore(npc) = Σ(weight_action * decay(t_now - t_action))` for actions in region ± deeds in same faction. Decay half-life: 30 days. Threshold triggers: `deedScore > 5` → "I remember you..."; `deedScore < −10` → "You again..."

##### Algorithm
Every player action (quest complete, kill, gift, steal) writes to the deed-log (compressed, survives save/load). On NPC first encounter or when M8-EXT-16 resident state loads, sample relevant entries. High positive → friendly opener; high negative → hostile warning. Subtle audio cues + facial animation shift. Tied to dread (M11-EXT-65): extreme scores (positive or negative) increase G-DREAD.

##### Examples
- Player helped faction X last session → trader greets "Back again, friend?" (tone + discount).
- Player raided faction Y 2 sessions ago → guard says "I remember your face" (hostile stance).
- Player ignored everyone → generic neutral opener, but boredom (M8-EXT-56) rises.

##### Failure Modes
- Memory overload (too many deeds) → prune old entries, keep extremes.
- Onscreen popups sneak in → validate: no UI event, only tone/behavior shift.
- Deed leakage across factions → faction-scoped reads only.

##### Player-Facing Impact
The world knows what you did. Not with popups, but with looks, words, and prices. You walk into a trader and they nod — or flinch.



--- M8-EXT-67 from M8.md ---
#### [M8-EXT-67] G-LOOTRISK: Threat-Scaled Loot Economy

> **tags** · loot, risk, economy
> **tl;dr** · The loot economy is framed as risk/reward: dangerous zones yield better gear; safe zones yield scraps. The design-layer framing over M8-EXT-01 + M12-EXT-38.
> **ctx** · G-LOOTRISK: Threat-Scaled Loot Economy -- M8. New block. Implements the G-LOOTRISK gap from the SIX-DECISIONS plan. The mechanical loot-tier math lives in M12-EXT-38; this block is the design pillar + economy integration. Ties to M8-EXT-01 (brand-tier) + M4-EXT-33 (loot tables) + M12-EXT-37 (encounter zones).
> **meta** · depends-on: [M8-EXT-01], [M4-EXT-33], [M12-EXT-38], [M12-EXT-37] · depended-by: -

##### Systems Touched
M8 loot/economy + M12 threat zones. The brand-tier loot system (M8-EXT-01) is the supply side; M12-EXT-38 is the zone-risk multiplier. This block ensures the *player understands* loot as a risk calculus: the best gear is behind the worst threats, and scarcity (M8-EXT-15) makes it precious.

##### Math
`perceivedValue(item) = baseValue(item) * (1 + threatOfSourceZone)`
- Player risk-calc: `expectedLoot(zone) vs expectedThreat(zone)` → rational players raid dangerous zones for tier-1 gear
- Scarcity (M8-EXT-15): post-loot, item leaves world (M7-EXT-21) → permanent scarcity, not respawn-farm

##### Algorithm
The economy surfaces risk/reward through world signals: dangerous zones *look* dangerous (M4-EXT-96 density, M6.5 audio, M11-EXT-55 encounter HUD). The loot tables (M12-EXT-38) deliver the payoff. Scarcity (M8-EXT-15) means once looted, an item is gone — the player carries the risk/reward with them. Traders (M8-EXT-14) price by brand-tier + scarcity, so risky-looted military gear sells high. The loop: risk → rare loot → economy leverage → better survival.

##### Examples
- Player raids a hospital (high threat, M12-EXT-37) → rolls antibiotics + trauma kit (M12-EXT-38) → saves a life or sells for premium.
- Player farms quiet suburbs → canned food + tools → survives but never upgrades.
- Player loots a military convoy → body armor → trades at a hub → buys a welder (M8-EXT-66) → builds better base.

##### Failure Modes
- Risk/reward unclear to player → world signals (audio/visual) must telegraph threat before loot.
- Scarcity too harsh → economy stalls → traders (M8-EXT-14) inject measured supply.
- Loot tier caps break (god drops in safe zones) → M12-EXT-38 hard cap enforces.

##### Player-Facing Impact
Loot is a decision, not a chore. The best gear costs the most to get. The apocalypse pays risk in gear.



--- M8-EXT-68 from M8.md ---
#### [M8-EXT-68] Evil / Good Alignment (meter-less, action-based)

> **tags** · social, alignment, morality
> **tl;dr** · Subtle alignment track (no visible meter) — actions tilt you toward altruist, warlord, cannibal, or ghost. Both paths reach all mechanical endpoints; alignment shapes NPC tone and event triggers, not locks.
> **ctx** · Evil / Good Alignment -- M8. New block. Implements the "player can be evil or good" mandate. Reuses M5-EXT-11 (moral framing), M8-EXT-16 (psyche), M12-EXT-34 (permadeath stakes). Inspired by Skyrim's subtle tone (no meter) + Witcher's consequence model (every choice echoes). Ties to M8-EXT-53 (standing vector) + M8-EXT-50 (events) + M11-EXT-65 (dread).
> **meta** · depends-on: [M5-EXT-11], [M8-EXT-16], [M12-EXT-34], [M8-EXT-53] · depended-by: -

##### Systems Touched
M8 alignment + standing + psyche. An `alignmentScore` (internally −1..+1, never shown) tracks actions: aid +1, kill +0.3, betray +0.7, consume_ally −0.5, cannibal +0.9. No meter — alignment surfaces via NPC tone ("savior" vs "monster"), world events (M8-EXT-50), and dread (M11-EXT-65). Both extreme paths reach all mechanical endpoints; alignment is flavor + friction, not content gating.

##### Math
`alignment ∈ {altruist (+0.6..+1), protector (+0.2..+0.6), neutral (−0.2..+0.2), selfish (−0.6..−0.2), monster (−1..−0.6)}`
- `dread_contribution = 0.1 * (1 − |alignment|)` → feeds G-DREAD
- `NPC_address_scale = alignment * tone_factor` → "savior" to "monster"
- `event_branch_factor` → some events tilt toward altruist or monster

##### Algorithm
Each moral action (quest choice, combat on civilian/surrender, sharing/hoarding, cannibal) writes Δalignment. NPCs read alignment to choose tone: altruist → "savior," monster → "monster." Events (M8-EXT-50) branch: a monster path may offer easier but cruel choices, altruist path harder but generous. Dread (M11-EXT-65) feeds on distance from neutral, not absolute evil — being *pure* anything makes the world wary. No lock ensures both paths stay viable.

##### Examples
- Player heals civilians, shares food → alignment → +0.7 → NPCs call "savior," settlement recruits flock.
- Player hoards, kills surrender → alignment → −0.8 → traders fear, guards hunt, but warlord quests open.
- Player cannibal (M5-EXT-15) → alignment → −0.9 → feared, but can still trade with raider faction.

##### Failure Modes
- Alignment becomes a lock (good path only) → ensure all gear/bases reachable on both paths.
- Monster path too easy → raise price, raise dread, fewer recruits.
- Tone feels fake → tie tone to real standing deltas (M8-EXT-53), not cosmetic.

##### Player-Facing Impact
The world sees you. Savior or monster, the living remember. But no path is shut — survive your way, live with the judgment.



