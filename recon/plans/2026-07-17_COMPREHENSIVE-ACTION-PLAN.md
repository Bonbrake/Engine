# ZombieEngine — Comprehensive Action Plan: Zombie AI, Survival Needs, Persistence, Design Gaps, and Everything Else

> **Author:** Jenny (architect mode)
> **Status:** PLAN — for user review and GO. Milestone spec files remain READ-ONLY per the standing gate. Nothing below is edited yet — these are proposed EXT blocks for your approval.
> **Research basis:** Online-verified against Project Zomboid, DayZ, Dying Light, State of Decay 2, 7 Days to Die, Fallout 4, and modern survival-game best practices (2026).

---

## 0. EXECUTIVE SUMMARY — what I'm proposing

| Area | What | Where | Priority |
|------|------|-------|----------|
| **Zombie AI** | Fill the 3 stub blocks + add 3 new ones | M5 | 🔴 Critical (zombie game with no zombie AI) |
| **Survival needs** | Expand hunger/thirst/stamina + add boredom | M2-EXT-18, M8-EXT-56 | 🔴 Critical (survival game with no survival loop) |
| **Persistence** | Fallout-grade reference registry + change-buffer + precombine | M7, M4, M12 | 🟡 High (your "exact same spot" ask) |
| **Design-layer** | G-BASE, G-DREAD, G-EMERGE, G-LOOTRISK, G-INFECTFEEL, G-JUICE | M8, M11, M12 | 🟡 High (fun/realism synthesis) |
| **Tooling** | Build the canonical test command + dep pinning | CMake, vcpkg | 🟢 Medium (professional polish) |

---

## 1. ZOMBIE AI — fill the gaps (M5)

### Current state (verified by reading actual files):
- `M5-EXT-09` AI Perception System — **STUB**: "No sight/hearing/visibility model"
- `M5-EXT-13` Zombie FSM States — **STUB**: "States idle/wander/chase/attack not defined"
- `M5-EXT-14` Hearing Perception — basic radius only, no directional cone
- `M5-EXT-15` Short-Term Memory — basic last-known-pos only

### Proposed blocks:

#### [M5-EXT-09] AI Perception System (FILL STUB)
Replace the current 1-line stub with a real perception system:
- **Sight:** Vision cone with range, angle, and LOS-check via `RayBatchQuery` (M1-002). Zombies see `R_sight` meters forward within `θ_fov` degrees. FOV narrows at night or in fog (M10 day/night).
- **Hearing:** Directional hearing cone (not just radius). `Dist(z,snd) < hearR` AND `angle(sndDir, facingDir) < hearCone`. Back-facing zombies have reduced detection range.
- **Smell:** Blood scent from open wounds (M2-EXT-80/81) → scent particle diffuses downwind. Zombies downwind of fresh blood have `detectRange *= 2 + bloodAge_debuff`.
- **Alertness state:** Unaware → Suspicious (heard something, investigating) → Alert (confirmed threat, chasing). Uses M5-EXT-15 memory.
- **Depends-on:** M1-002, M2-EXT-80, M2-EXT-81, M10 day/night

#### [M5-EXT-13] Zombie FSM States (FILL STUB)
Replace the current stub with:
- **Dormant** (new): Zombie slumped/lying in buildings, not active. Wakes on sound threshold, proximity, or light. This is the "sleeper" mechanic — buildings are unpredictable.
- **Idle:** Standing, swaying, looking around. Occasional moan/groan.
- **Wander:** Random patrol within spawn zone. 
- **Alert:** Heard/saw something, moving to investigate. Memory timer.
- **Chase:** Active pursuit. Speed varies by zombie type (shambler vs runner).
- **Attack:** In melee range. Swipe, bite, grab.
- **Flee:** (rare) On fire, severe damage, or fear stimulus.
- **Depends-on:** M5-EXT-09, M5-EXT-14, M5-EXT-15

#### [M5-EXT-14] Hearing Perception (FILL STUB)
Replace the current 1-line algorithm with:
- Sound events propagate as spheres with frequency-dependent attenuation.
- **Directional hearing:** Zombie facing direction determines detection cone (±120°). Sounds behind zombie have 50% range penalty.
- **Sound types:** Footsteps (quiet), gunshots (loud), explosions (map-wide), whispers, doors opening, building collapse.
- Each zombie type has different hearing range and cone width.
- **Depends-on:** M5-EXT-09, M6 audio

#### [M5-EXT-17] Sleeper Zombie Spawn & Wake System (NEW)
- On worldgen, a percentage of interior zombies spawn as **dormant** (slumped, lying, leaning). Set by `wakeFlags` bitmask.
- **Wake triggers:** Sound threshold, player proximity within 2m, light shone on face, damage, building collapse. 
- **Wake animation:** Emerge sequence (climbing out of bed, rising from floor, breaking through door) — uses M5-EXT-12.
- **Dormant detection:** Players can stealth-kill dormant zombies (one-hit takedown, no sound). Risk-reward for clearing buildings.
- **Re-dormancy:** Cleared building stays dormant-free for `N` days, then repopulates via M5 director.
- **Depends-on:** M5-EXT-13, M5-EXT-12, M4 (worldgen interior placement)

#### [M5-EXT-18] Zombie Scent / Pheromone Trail (NEW)
- Player blood, open wounds, sweat, and rotting gear emit a **scent value** (0.0–1.0) per tick.
- Scent diffuses via wind direction (M10 weather) into a scent field grid.
- Zombies in the scent field move up the gradient (scent chemotaxis).
- **Scent strength:** Fresh blood > old blood > sweat > rotting gear. Rain washes scent away.
- **Player countermeasures:** Mud/water covering reduces scent. Scent-blocking gear (hazmat, raincoat). Wind direction awareness.
- **Depends-on:** M2-EXT-80, M2-EXT-81, M10 weather, M5-EXT-09

#### [M5-EXT-19] Stealth Approach System (NEW)
- **Noise meter:** Player movement speed + surface type + encumbrance → noise level (0 = silent crouch on grass, 1.0 = sprinting on metal).
- **Visibility meter:** Light level (M10 night/day) + player stance (crouch/prone/stand) + cover (partial/full) + clothing camo → visibility %.
- **Detection check:** Zombie perception vs (noise + visibility) thresholds. Thresholds modulated by zombie alertness state.
- **Stealth kills:** Crouch + approach from behind + hold-breath → one-hit takedown. Risk: noise from takedown attracts nearby zombies.
- **Depends-on:** M5-EXT-09, M5-EXT-13, M10 day/night, M2-EXT-18 (encumbrance)

---

## 2. SURVIVAL NEEDS — expand hunger/thirst/stamina + add boredom (M2, M8)

### Current state:
- `M2-EXT-18` Survival Physiology — **STUB** (1-line algorithm: "Data-driven stamina/hunger/thirst decay feeding gameplay")
- `M8-EXT-56` Survivor Psyche — exists but boredom is missing

### Proposed blocks:

#### [M2-EXT-18] Survival Physiology (FILL STUB)
Replace the current 1-line stub with:
- **Hunger:** `hunger += dt * baseRate * activityMultiplier`. Hunger > threshold → penalties (lower stamina regen, reduced strength, damage over time). Satiated from food.
- **Thirst:** `thirst += dt * baseRate * (1 + temperaturePenalty)`. Thirst > threshold → stamina drain, vision blur, damage over time. Hydrated from water.
- **Stamina:** `stamina = f(rest, hunger, thirst, encumbrance, exertion)`. Sprinting/combat drains stamina. Sitting/standing recovers slowly, sleeping recovers fast.
- **Encumbrance:** Each item has weight. Total weight vs strength → movement speed penalty, stamina drain rate, noise increase.
- **Rest/fatigue:** `fatigue += dt * wakefulness`. Fatigue > threshold → forced sleep. Sleep = 8-hour cycle (M7 epoch clock). Interrupted sleep = partial recovery.
- **Depends-on:** M7 (save/epoch), M10 (day/night), M2-EXT-80 (injury)

#### [M8-EXT-56] Survivor Psyche — add boredom (FILL EXTENSION)
Append to existing psyche block:
- **Boredom:** `boredom += dt * indoorMultiplier`. Increases when indoors, inactive, or repetitive activity. Decreases when outdoors, exploring, panicking, or in combat.
- **Boredom effects:** High boredom → stress increases faster, happiness decreases, dialogue becomes irritable. Very high boredom → character takes risks (peek out windows, ignore danger).
- **Boredom break:** Player can't sleep if bored (PZ-style). Must go outside, read a book, listen to radio, or interact with NPCs.
- **Depends-on:** M2-EXT-18 (physiology), M8-EXT-56 (existing psyche)

---

## 3. PERSISTENCE ARCHITECTURE — Fallout-grade (M7, M4, M12)

### Proposed blocks (from earlier architecture plan):

#### [M7-EXT-20] Stable Reference Registry
- Every world object gets a stable WorldRefID (chunk-based, NOT ECS entity ID)
- Reference table maps WorldRefID → {archetype, transform, owner, state}
- Deterministic from seed, survives save/load cycles

#### [M7-EXT-21] Persistent Cell Change-Buffer
- Per-chunk delta log: moved, removed, player-placed, state-overridden
- Save = serialize change-buffers only (not full world)
- Load = base chunk gen + replay deltas → exact same spot

#### [M7-EXT-22] Bit-Exact Transform Determinism
- Jolt double-precision determinism → stored transforms reproduce to the centimeter
- Co-op peers and reloads agree on position

#### [M4-EXT-83] Edit-Aware Precombine Bake
- Bake static chunk geometry into frozen combined meshes
- Tag each cluster with its static-only refs
- Player edit → invalidate only touched cluster → async re-bake

#### [M4-EXT-84] Previs Occlusion Bake
- GPU-driven visibility precomputation per chunk cluster
- Reuse M4.5 occlusion pipeline

#### [M12-EXT-29] Encounter Zones (Layered Density)
- Each region gets a curated density + threat budget
- Cleared zone repopulates on cooldown, not instantly
- Prevents "vast but empty" (NMS failure mode)

#### [M12-EXT-30] Threat-Scaled Loot Budget
- Dangerous zones = better loot
- Ties to M8-EXT-01 brand economy + Appendix C socio-economic tags

---

## 4. DESIGN-LAYER GAPS — fun/realism synthesis (from SIX-DECISIONS plan)

### [M12-EXT-31] G-BASE: Player-Built Fortifications
- Player-placeable walls, spikes, barricades, sandbags, turrets
- Uses M8-EXT-32 funnel pathing + M5 horde AI
- **Horde-night:** Scheduled wave every N in-game days (7DTD blood-moon)

### [M11-EXT-63] G-DREAD: Tension Loop Design
- Darkness = rising fear scalar (M10 night)
- One bite = visible infection clock (SEIR countdown)
- Audio swell + vignette + controller pulse (not HUD numbers)
- **I Am Legend inspiration:** Dusk = the board-up moment

### [M12-EXT-32] G-EMERGE: Emergent Storytelling
- Player choices ripple through settlement/brand state
- Quest resolve → mutation of NPC standing, settlement defense, brand tier
- Appendix A journal generator surfaces the narrative

### [M8-EXT-67] G-LOOTRISK: Threat-Scaled Loot
- Dangerous zones (high zombie density, irradiated, military) → better loot tables
- Uses Appendix C socio-economic tags
- Risk-reward calculation: gear up for dangerous run

### [M11-EXT-64] G-INFECTFEEL: Infection as Dread
- SEIR progression shown as visible timed countdown (not a number)
- Vignette darkens, pulse quickens, audio distorts
- Antidote/cure is rare, meaningful, and a journey

### [M11-EXT-65] G-JUICE: Game Feel
- Screen shake on explosions, hits, near-misses
- Hit-pause on melee impact
- Damage animation (stagger, flinch)
- Health bar with smooth interpolation (not instant)

---

## 5. TOOLING & PROFESSIONAL POLISH

### Dependency pinning
- Add `version>=` constraints to `vcpkg.json` for all 17 deps
- Pin to the live-newest from `recon/check_versions.py`
- Strategy: per-dep `version>=` (most standard vcpkg approach)

### Canonical test command
- `cd build && cmake --build . && ctest --output-on-failure` 
- Document in `STATUS.md` and `00_PROTOCOL.md`

### TOC sync
- Several milestone TOCs miss blocks that exist in the file (M1, M2, M4, M5, M8, M9, M10, M11, M12, M13)
- Regenerate TOCs from actual block headers using `scripts/regen_sidecars.py`-like logic

---

## 6. EXECUTION ORDER (my recommendation)

| Batch | What | Files touched | Risk |
|-------|------|---------------|------|
| **1** | Fill M5 zombie AI stubs (M5-EXT-09, 13, 14) + add sleeper (M5-EXT-17) | spec/M5.md | Low — fills existing stubs |
| **2** | Add zombie scent (M5-EXT-18) + stealth (M5-EXT-19) | spec/M5.md | Low — new blocks, no conflicts |
| **3** | Fill M2-EXT-18 survival physiology + add boredom to M8-EXT-56 | spec/M2.md, spec/M8.md | Low — fills existing stubs |
| **4** | Write persistence architecture blocks (M7-EXT-20/21/22 + M4-EXT-83/84) | spec/M7.md, spec/M4.md | Medium — your "exact spot" ask |
| **5** | Write encounter zones + threat-scaled loot (M12-EXT-29/30) | spec/M12.md | Medium — ties to existing systems |
| **6** | Write design-layer blocks (G-BASE, G-DREAD, G-EMERGE, G-LOOTRISK, G-INFECTFEEL, G-JUICE) | spec/M8.md, M11.md, M12.md | Medium — extends existing systems |
| **7** | Dep pinning + TOC sync + canonical test command | vcpkg.json, spec/*.md, CMakeLists.txt | Low — mechanical |
| **8** | Commit batch to `spec/m0-parity-reformat` | All | — |

Each batch is 2–5 blocks, independently verifiable. I author each block fully (Systems Touched, Math, Algorithm, Examples, Failure Modes, Player-Facing Impact) with cross-references to existing IDs.

---

## 7. OPEN QUESTIONS FOR YOU

1. **Sleeper zombie density:** What % of interior zombies should be dormant? PZ-style = ~30% buildings have sleepers. More? Less?
2. **Boredom severity:** Should it be a soft pressure (like PZ, forces you outside) or a hard need (debuffs if ignored)?
3. **Persistence save model:** I recommended per-chunk change-buffers (FO4-style). OK, or do you want a global delta log?
4. **Horde-night frequency:** 7 Days to Die uses every 7 days. Do you want the same, or configurable?
5. **Execution order:** I recommend Batches 1→2→3→4→5→6→7→8 (zombie AI first, then survival, then persistence, then design-layer). Does that order work for you?