# ZombieEngine — System Interconnection Map (Cross-Links)

> Every system attaches to the **World-Graph Backbone [M4-EXT-74]**. This document is the web that makes "all connected." When implementing, follow these edges so the world behaves as one organism, not a pile of features.

## The Spine
- **World-Graph [M4-EXT-74]** = nodes (region / POI / hub / safe-zone) + edges (road / tunnel / smuggle-route), weighted by distance + threat.
- **World-State [M5.4-EXT-10]** = persistent mutation log; every action writes here.
- **SLM Harness [M13-EXT-14..26]** = generates all text; schema-validated + deterministic.

## Edge Map (system → what it feeds / is fed by)

### Procedural Generation (M4)
- M4-EXT-58 POI → graph nodes
- M4-EXT-74 backbone ← all POIs, factions, encounters
- M4-EXT-63 environmental storytelling → SLM note [M13-EXT-24] on every POI
- M4-EXT-56 wildlife ↔ M8.5-EXT-07 snares, M4-EXT-59 regrowth
- M4-EXT-57 rain ↔ M2.9 water, M4-EXT-83 water treatment
- M4-EXT-60 season / M4-EXT-61 tide → M10 weather, M2.9 farming
- M4-EXT-64..86 POIs → each links: graph [74] + faction influence [M8.6-EXT-12] + encounter [M5.4-EXT-16] + loot [M8-EXT-14] + SLM story [M13-EXT-24]

### Factions (M8.6)
- M8.6-EXT-12 turf war → graph node field, M5.4 events
- M8.6-EXT-13 reputation/vendors ← player acts, → price
- M8.6-EXT-18 civil war → M5.4 event
- M8.6-EXT-24 forced-labor / M5.4-EXT-11 cannibal → world-state morality
- M8.6-EXT-28 hub ← M7 save, M2.9 rest, M7-EXT-16 stash
- M8.6-EXT-22 smuggling ↔ M5.4 encounters

### Survival (M2.9)
- day/night [M2.9-EXT-16] → M6 lighting, M5.4 aggro, M2.9 farm ticks
- blood/bandage/splint [M2.9-EXT-18..20] ← M8 med items, M4-EXT-77/86 pharmacy
- spoilage [M2.9-EXT-21] → M8.5 canning
- sleep/fatigue [M2.9-EXT-24] → rest UI [M11-EXT-59], hub [M8.6-EXT-28]
- body heat [M2.9-EXT-25] → M10 weather, fire/shelter

### Combat / Player (M2, M2.7)
- thrown-object [M2.7-EXT-30] → M5.4 fear-field (any small object, real physics)
- shiv FOLDED into crafting [M8-EXT-16] → M2-EXT-24 lockpick, M2-EXT-25 traps
- holster/swap [M2.7-EXT-29] → no menu mid-combat
- companion [M5.1-EXT-09] ← M8.6 recruitment, → SLM barks [M13-EXT-48]

### Threat (M5.4)
- horde migration [M5.4-EXT-12] ← noise field, → graph edges
- mutant strains [M5.4-EXT-13] ← zone heat [M8.6-EXT-12]
- ambient encounters [M5.4-EXT-16] ← world-state, → graph edges
- karma [M5.4-EXT-17/18] ← moral choices [M13-EXT-31/32], → factions [M8.6-EXT-13], SLM broadcasts [M13-EXT-36/37]

### UI (M11) — diegetic, TLOU/Metro
- minimal HUD [M11-EXT-52] fades by context
- watch [M11-EXT-53] ← M2.9 time, M11-EXT-42 marker
- journal [M11-EXT-54] ← POI + SLM lore [M13-EXT-24]
- weapon inspect [M11-EXT-55] ← M8-EXT-13 condition
- map annotation [M11-EXT-57] ← M4-EXT-62 surface map
- skill sheet [M11-EXT-58] ← M2.9 XP, M2-EXT-26 respec

### SLM (M13) — the connective tissue
- lore [M13-EXT-24] ← POI, → journal/graffiti/signage
- missions [M13-EXT-31/32] ← world-state, → karma [M5.4-EXT-17]
- rumor [M13-EXT-42] / diary [M13-EXT-41] / memory [M13-EXT-50] ← NPC graph
- broadcasts [M13-EXT-36/37] ← events, → factions
- all gated by [M13-EXT-16/17] validate, [M13-EXT-19] deterministic

## Skill-by-Use Progression (Skyrim principle, fit to THIS game)
- Every action skill (reload/melee/lockpick/throw/medicine/craft/drive) improves by DOING it [M8.7-EXT-02].
- Reload starts fumbling (bad hand-position, dropped mag) [M2.7-EXT-19] and tightens with reps.
- Perks [M2-EXT-21] raise ceilings, not grant skill. Traits set starting ceilings.
- One Fumble Animation Director [M8.7-EXT-03] shows the terrible-start visually across all actions.

## Realism Rules (no "that's dumb" moments)
1. Thrown objects use the SAME rigid-body physics as everything (M4) — noise scales with mass/speed/surface.
2. Crafting is ONE data-driven table [M8-EXT-09]; shiv/molotov/spike-bat are recipes, not magic keys.
3. Factions have ideology (SLM charter [M13-EXT-43]) — they fight for reasons, not spawn as targets.
4. World-state persists — cleared zones stay cleared, burned stays burned, helped gangs stay friendly.
5. Karma changes the world, not just a number.
6. Every POI is a graph node with influence + loot + story, so distance and territory mean something.
