# Design Decisions — ZombieEngine Core Systems

## Decision 1: Horde Cadence → Telegraphing Tiered Horde System

**Problem:** How often do horde nights occur? How does alignment express in gameplay?

**Solution:** 7-day baseline cycle with 3 telegraphing layers:
- **Day 1-5 (Drift):** Slow world-state change (faction patrols, resource migration, zombie density creep)
- **Day 6 (Scout):** Audio/visual tells — distant howls, scout zombies probing perimeter, radio chatter spikes
- **Day 7 (Siege → Breach):** Full horde night. Two phases: Siege (wave pressure, structure testing) → Breach (interior infiltration if perimeter fails)
- **Sanctuary Mechanic:** Player-founded hubs (M8-EXT-53) emit "Sanctuary" field — horde AI deprioritizes during Drift/Scout, full pressure only on Breach. Protects player investment without trivializing threat.

**Why:** 7DTD Blood Moon (predictable, exploitable) + SoD2 Plague Hearts (emergent, location-based) → tiered telegraphing solves both. Alignment (Dual-Axis Fame/Infamy) modulates Scout intensity: high Infamy = earlier/more scouts; high Fame = militia callouts.

---

## Decision 2: 5th Built-In Faction → Remnant Military

**Problem:** 5th faction slot TBD. Must foil Raiders (chaos) and Militia (survival/community).

**Solution:** **Remnant Military** — pre-war continuity government.
- **Doctrine:** "Order through strength" — centralized command, heavy armor, air assets, NBC gear
- **Visual:** Digital woodland → urban camo transition; vehicle convoys; fortified FOBs
- **Tech:** Working comms, drone recon, artillery strikes, pre-war logistics
- **Ideology:** Civilians are "assets to secure"; Raiders are "insurgents to eradicate"; Horde is "biological contamination to contain"
- **Numbers:** Low population, high individual power, slow replenishment

**Why:** FO3 Enclave / NV NCR / FO4 Brotherhood lineage — but distinct: not genocidal (Enclave), not expansionist (NCR), not tech-hoarding (BoS). "Continuity government" is a fresh angle. High-tech/low-numbers creates asymmetric gameplay vs. Raider swarms.

---

## Decision 3: Alignment → Dual-Axis Reputation (Fame/Infamy per Faction)

**Problem:** "Meter-less, action-based" (M8-EXT-68) — but how does it gate content?

**Solution:** Two independent axes per faction, no global meter:
- **Fame** — helped faction, defended their people, completed their objectives
- **Infamy** — killed their members, stole their assets, sabotaged their ops

**Mechanical Locks:**
| Axis | High Fame Unlocks | High Infamy Unlocks |
|------|-------------------|---------------------|
| Raiders | — | Raider cache locations, fear aura, tribute demands |
| Military | Supply drops, artillery call-ins, gear requisition | — |
| Civilian | Trade discounts, safehouse access, recruit quality | — |
| Remnant | Advanced tech, drone support, orbital strike beacon | — |
| Horde | — | Zombie Beacon crafting, horde-night Scout skip, mutation access |
| Player-Founded | All of the above (your faction) | — |

**Why:** No "karma meter" to game. Actions speak. Both paths reach all mechanical endpoints (different flavor). Faction-specific — betraying Military doesn't hurt Civilian standing.

---

## Decision 4: Player-Zombie Leadership → Zombie Beacon

**Problem:** "Player can lead all except zombies" — but horde nights need player agency.

**Solution:** **Zombie Beacon** — craftable/looted device.
- **Acquisition:** Horde-night loot (Breach phase), Remnant tech cache, high-Infamy Raider trade
- **Usage:** Deploy → emits pheromone/acoustic signature for 60-120s, 15-25m radius, 10-min cooldown
- **Effect:** Horde AI within radius gains "Beacon" priority target — moves toward beacon, attacks beacon's target, breaks formation to swarm
- **Constraints:** Horde-night only; cannot direct specific zombies; no RTS camera; beacon is destructible; using it broadcasts player position to ALL factions

**Why:** Horror-consistent (you're baiting, not commanding). Skill expression (placement, timing, escape). Mutates horde behavior without breaking "player doesn't lead zombies." High Infamy = better beacons.

---

## Cross-Cutting: Perspective Constraint

**Strict FPS/TPS only.** No top-down, no tactical camera, no RTS mode.
- Main = first person
- Third person = parity toggle (same mechanics, different camera)
- All above systems designed for FPS/TPS player experience