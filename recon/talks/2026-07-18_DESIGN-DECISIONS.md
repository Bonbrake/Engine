# Design Decisions — ZombieEngine Core Systems

## Decision 1: Horde Dynamics → Dynamic Open-World Migration (No Fixed Cycle)

**Problem:** How do hordes threaten players without a predictable calendar? How does alignment express in gameplay?

**Solution:** **Dynamic Open-World Horde Migration** — no fixed day-7 cycle. Horde threat emerges continuously from world state:

- **Wandering Horde Entities** — persistent groups that migrate across the map, drawn by emergent signals
- **Attraction Signals** (additive, no schedule):
  - Player noise (M5-EXT-14 hearing-cone, M11-EXT-64/65 stealth-approach)
  - Scent trails (M5-EXT-65 pheromone) — fresh kills, blood, camps
  - Screamer alerts (M5-EXT-64 sleeper) — single scream propagates through network
  - Faction combat nearby — gunfire/explosions as dinner bells
  - Time-of-day + weather — night/rain increases boldness/detection
- **Escalation = Probability Stack** — each signal increases aggression tier:
  - **Idle** (baseline): hordes wander, scavenge, ignore distant players
  - **Scout** (1–2 signals): perimeter probes, audio/visual tells
  - **Siege** (3+ signals): directed movement, structure testing
  - **Breach** (sustained Siege + player in structure): interior infiltration

**Sanctuary Mechanic:** Claimed player bases (M8-EXT-53) emit deterrent field — reduces wandering-horde pathing weight 60% within 100m. Does NOT prevent Siege/Breach if player generates strong signals inside. Protection is behavioral, not temporal.

**Horde Size Math (Emergent):**
```
base_density = zone_base + (days_survived * 0.1) + (player_level * 0.5)
signal_bonus = sum(active_signal_strength)
horde_size = base_density * (1 + signal_bonus * 0.3)  // capped by zone max
```

**Why:** 7DTD's fixed Blood Moon is exploitable (cheese day 6). SoD2's Plague Hearts prove emergent, location-based escalation works. Urban Strife proves alert propagation > central command. Dynamic migration = always-on tension, no calendar-gaming, horde feels like living ecosystem.

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