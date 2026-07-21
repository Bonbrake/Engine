# ZombieEngine — Strategic Recommendations (2026-07-20)
## From the Six Decisions Plan: what's best for the game

### Preamble
This document resolves the 47+ either/or (E1–E93) decisions from the Six Decisions
plan. Each recommendation is grounded in: what makes this game *unique*, what serves
the player's stated vision (M-rated, photoreal, procedural, permadeath, co-op, deep
simulation + emergent storytelling), and what is practically buildable.

---

## 🔴 TIER 1 — Identity-Defining Decisions (make or break the game)

### E48: Map Size → 40 km² (six contiguous biomes)
25 km² is too small for 6 biome regions + transition zones (each region ~4 km² would
blend into its neighbor — no distinct "entering the desert" feeling). 64 km² needs
AI-generated content to fill — your "dense, not empty" bar means density drops at that
scale. 40 km² gives each biome ~6.5 km² of walkable territory with room for seam
zones. Enshrouded full-release proved 40 km² is dense-fun with procedural filling.
This aligns with M4-EXT-31 seamless blending and M4-EXT-50 lookahead streaming.

### E35: Quest Generation → Constrained Generator (not hand-author, not pure AI)
12 authored story-arc templates × ~17 procedural variants = 200+ quests. The templates
guarantee narrative coherence (R51's "heavily constrained or it breaks narrative").
Pure SLM generation creates incoherent junk. All hand-authored is impossible at 200+
depth. You curate the templates, SLM fills the variants, you place them on the map.
Reuses M13-EXT-16/17/18/19/21/23 (SLM pipeline) + M13-EXT-60 (SLM quest gen).

### E37: HUD → Diegetic-with-Toggle (default diegetic, option for classic HUD)
Metro-style immersion is the game's identity. But accessibility (colorblind, deaf,
non-native players) and genre newcomers need a fallback. Default = diegetic watch +
world-embedded readouts (M11-EXT-60/63). Option = classic health/stamina/ammo HUD.
One bool in Config::cvars, not a new system.

### E38: Horde-Night → Scheduled (7DTD blood-moon model)
Free-clear removes the tension architecture. The dread loop (G-DREAD) NEEDS a known
deadline — "the night is coming, prepare." This is your I Am Legend board-up moment.
Scales with SettlementDefense deficit (more loot = bigger horde = risk/reward loop).
Pathing reuses M8-EXT-32 funnels. Verdict of Part 15 (coherence check): ✓

### E45: Forge Scope → Full (FC3-style object palette + prefab + scripting + co-op edit)
The Forge is the long-term content engine — modders, co-op builders, and YOU use it
to place quests and shape the world. A lightweight version would be a toy. Full scope
reuses M13-EXT-55/56/57/58 + M4-EXT-39 palette + M7-EXT-19 persistence. Build once,
serve the whole game's lifecycle.

### E85: Endgame Trigger → Emergent (not deterministic score)
Your game is built on emergence: AI war (M8-EXT-57), trade cascade (M8-EXT-60),
settlement growth (M8-EXT-59). The endgame should emerge from accumulated world
state, not a score counter. Every playthrough ends differently. Reuses M8-EXT-60
cascade engine + M12-EXT-35 divergence engine.

---

## 🟡 TIER 2 — High-Impact Design Decisions

### E65/E66 — LOCKED (your prior decisions confirmed):
- Endless + permadeath ✅ (character deleted, world persists)
- No fast-travel ✅ (dread/honest tension)
- One dynamic difficulty (M1-EXT-36) ✅
- These are correct. Do not reopen.

### E59: Audio → Custom procedural + Wwise for authored music/VO
Custom M6/M6.5 procedural audio (acoustics RVT, blood spatter, env ambience) is your
TI-aligned path. ADD Wwise only for authored music (horde-night crescendo, dread loop
trigger) and sparse voice (M11 quest fragments, M12 narrative). Not a replacement,
an overlay.

### E73: Modding VM → LuaJIT (not Lua 5.4)
Factorio/RimRoom/Don't Starve all use LuaJIT — it's ~10× faster than stock Lua 5.4.
For a game targeting idTech-quality perf, every millisecond matters. Sandbox via
M13-EXT-16/19 (SLM bounds). Reuses M13-EXT-61.

### E60: AI Co-op Model → Client-prediction (not lockstep)
Lockstep is brittle, high-latency, complex to debug. Client-prediction (each client
sims AI locally, server reconciles) is the modern standard (CoD Zombies, L4D). More
initial work but the right call for a co-op game. Reuses M12-EXT-22 dedicated server.

### E62: Build Snap → Grid-snap with free-place override
Free-place leads to janky physics glitches. Grid-snap with 'hold to override' is the
polished approach (Subnautica, Valheim). Reuses M12-EXT-30 build placement.

### E64: Wildlife → Both passive + infected variants
Passive (deer, birds, rabbits) for food + world-life. Infected variants (zombie wolves,
diseased crows) for dread. Reuses zombie AI + SEIR infection — cheaper than separate
system. Reuses M5-EXT-62.

### E71: Parody Tone → Satirical-but-dark (Borderlands + WALL-E, M-rated)
Not cartoonish. Not preachy. The humor is the bitterness of a dead world, the absurdity
of consumerism gone feral. Think Fallout's dark satire — laugh because it's true, then
shudder because it happened here. Reuses M8-EXT-55 (corp decay).

### E81: Settlement Autonomy → Player-directable (not full autonomy, not full manual)
The sweet spot: set directives ("defend north wall", "prioritize farming", "expand
housing to 20") and let settlement AI execute. You guide, they do. Satisfying without
being micromanagement. Reuses M8-EXT-59 + M8-EXT-16 (NPCs).

### E86: Nuke → Player-only trigger
The nuclear ending (M12-EXT-36) is the ultimate player choice — the "fuck it" button
with permanent consequences. Letting AI factions fire it dilutes player agency.
Co-op: majority vote to launch.

---

## 🔵 TIER 3 — Quality-of-Design Decisions

### E34: Quest Count → 200 (12×17 variants). Templates guarantee coherence.
### E36: Discovery → Fully discovery-based, no ping. M10-EXT-24 supports "no objectives."
### E40: Seed → Single pinned seed (shared canonical world). E40 RESOLVED: shared.
### E41: Biomes → Contiguous macro-climate regions (not per-cell speckle).
### E42: City Theming → Region-skinned (reuse facade CGA styles per biome).
### E43: Gore → Full M-rated, unflinching (reuse M6.5-EXT-13/M7-EXT-01).
### E46: Co-op Edit → Real-time merge (chunk-authority model M1-EXT-26).
### E47: Split-screen → Include (optional, reuse M12 local-split).
### E49: AI-Quest Ratio → SLM 80% generate, you 20% curate + place all.
### E50: Placement → You place all (curation ensures quality).
### E51: Vehicle Build → Full Earn-to-Die weld-everything (M9-EXT-24).
### E52: Handling → Tuned-realistic (GTA-feel, not arcade).
### E53: Interactables → Everything Jolt-rigid-body (M9-EXT-25).
### E54: Import → Real OSM city as base canvas + Forge-edit.
### E55: Dress → Full procedural skyscraper dress (reuse M4 CGA + palette).
### E56: Asset Ingest → Poly Haven + Sketchfab-PBR + Wikimedia (not Kenney/OA for hero).
### E57: Variation → Procedural variation on photoreal base (one texture → many).
### E58: Hero Filter → Strict PBR (reject non-PBR for hero pipeline, allow for props).
### E61: M5 AI → Full navmesh + director (not simplified). Core gameplay needs it.
### E63: Server Browser → LAN + internet (community is the game's longevity).
### E68: Settlement Growth → Quest-gated (rewards exploration, M12-EXT-26/M11-EXT-64).
### E69: Casino Risk → Pure-economy (no rigged-deck minigame — keep tone consistent).
### E70: Haven Breach → Provoke-only (NPCs calm unless you shoot/steal).
### E72: Corp Decay Density → Everywhere (biome-scattered, reuses M4-EXT-39).
### E75: Weapon Mods → Slot-limited (3-5 slots). Balance > free-for-all.
### E76: War Map → Fog-of-war revealed by exploration/broker (diegetic).
### E77: Passenger Gunner → AI companion (reuse M8-EXT-56 psyche).
### E78: Weld Slots → 3-5 frame-slot limit (balance + perf).
### E79: Car Rarity → Ultra-rare (side-quest reward, M12-EXT-26/M11-EXT-64).
### E80: Durability → Severe (tension from scarcity).
### E82: Cascade Model → Event-driven (cascade fires from specific triggers, not const sim).
### E83: Trade Routes → Visible (roads on map, convoys move).
### E84: State Weighting → Full (director considers all world state, not limited).
### E87: NPC Registry → Global cloud (persistent NPCs across co-op worlds).
### E88: Trait Depth → Compact 12-facet (DF 51 is overkill for procgen filler).
### E89: Caste → Fluid (player actions shift tiers organically).
### E90: Shanty → Procedural-stacked (reuse M3 prefabs + M4-EXT-25).
### E91: Land → Market-bought (economy depth + player choice).
### E92: Routine → Keyframe (wake/job/eat/sleep ticks, not minute-by-minute sim).
### E93: Landmark Pool → Fixed roster of 7 (M4-EXT-100), seeded variant per play.

---

## Final Priority Order for Build Phases

1. **Foundation**: M4 shared seed + biomes + region streaming (M4-EXT-92/93/94)
2. **Core Loop**: M8 horde-night + base build + settlement (M8-EXT-50/51, M12-EXT-30)
3. **Dread + Tone**: M11 diegetic HUD + watch + dread surface (M11-EXT-60/62/63)
4. **Content**: M12 side-quests + environmental fragments (M12-EXT-26, M11-EXT-61)
5. **Character**: M12 permadeath + endless (M12-EXT-34)
6. **World Depth**: M8 faction standing + settlement growth + cascade (M8-EXT-53/54/59/60)
7. **Vehicles**: M9 build + combat + scarcity (M9-EXT-24/26/27/28/29)
8. **AI + Horde**: M5 navmesh + director + wildlife (M5-EXT-60/61/62/63)
9. **Tech**: Audio + LuaJIT + server browser (E59/E73/M12-EXT-31)
10. **Endgame**: M12 divergence + nuke (M12-EXT-35/36)
11. **Editor**: Forge full scope + modding SDK (M13-EXT-55/56/57/58/59/61)
12. **Polish**: Corp decay + NPC stratification + shanties + war map + landmarks
    (M8-EXT-55/62/63/64/65, M12-EXT-33, M4-EXT-100)
