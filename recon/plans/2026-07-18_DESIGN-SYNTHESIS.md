# Design Synthesis: Horde Nights, Factions, Alignment & Player-Zombie Leadership

**Date:** 2026-07-18  
**Source:** 5 targeted web searches + project constraints  
**Purpose:** Convert research into actionable recommendations for 4 open design decisions gating 139 remaining provisional blocks

---

## 1. Horde Dynamics — Dynamic Open-World Migration (No Fixed Cycle)

### Research Summary
| Source | Key Finding |
|--------|-------------|
| 7DTD Blood Moon | Every 7 days; forced routing, sprinting, block destruction; scales with game stage — **predictable, exploitable** |
| SoD2 Plague Hearts | Dormant → Stirred → Awakened; triggered by Screamer screams + nearby kills; awakened hearts spawn periodic hordes to empty buildings — **emergent, location-based** |
| Urban Strife | Screamers emit screams alerting all zombies in expanded radius; single Screamer converts passive horde to pursuit in 2 turns — **alert propagation, not timer** |

### Recommendation: **Dynamic Open-World Horde Migration**

**No fixed calendar. No "day 7."** Horde threat emerges continuously from world state:

- **Wandering Horde Entities** — persistent horde groups that migrate across the map, drawn by emergent signals
- **Attraction Signals** (additive, no schedule):
  - Player noise (M5-EXT-14 hearing-cone radius, M11-EXT-64/65 stealth-approach)
  - Scent trails (M5-EXT-65 pheromone) — fresh kills, blood, player camps
  - Screamer-type alerts (M5-EXT-64 sleeper) — single scream propagates through horde network
  - Faction combat nearby — gunfire/explosions act as dinner bells
  - Time-of-day + weather — night/rain increases horde boldness and detection range
- **Escalation = Probability Stack** — each active signal increases horde aggression tier probability:
  - **Idle** (baseline): hordes wander, scavenge, ignore players beyond detection
  - **Scout** (1–2 signals): perimeter probes, audio/visual tells (howls, silhouettes on ridge)
  - **Siege** (3+ signals): directed movement, structure testing, coordinated pressure
  - **Breach** (sustained Siege + player in structure): interior infiltration, window/door focus

**Sanctuary Mechanic (Player Hub Protection):** Claimed player-founded bases (M8-EXT-53) emit low-level deterrent field — reduces wandering-horde pathing weight by 60% within 100m. Does NOT prevent Siege/Breach if player generates strong signals inside. No day-7 shield — protection is behavioral, not temporal.

**Horde Size Math (Emergent):**
```
base_density = zone_base + (days_survived * 0.1) + (player_level * 0.5)
signal_bonus = sum(active_signal_strength)  // noise, scent, screamers, combat
horde_size = base_density * (1 + signal_bonus * 0.3)  // capped by zone max
```

**Why:** 7DTD's fixed Blood Moon is exploitable (players cheese day 6). SoD2's Plague Hearts prove emergent, location-based escalation works. Urban Strife proves alert propagation > central command. Dynamic migration = always-on tension, no calendar-gaming, horde feels like a living ecosystem.

---

## 2. 5th Built-In Faction

### Constraints from Project
- 6 faction slots: 5 built-in + 1 player-founded
- Current 4: **Raiders** (chaotic), **Military** (order), **Civilians** (survival), **Infected Horde** (mindless)
- Player leads all EXCEPT Horde (M8-EXT-53, M8-EXT-68)
- Alignment = action-based, no meter (Fame/Infamy dual-axis, FNV style)

### Research-Informed Options

| Option | Archetype | Fills Gap | Interaction with 4 Existing |
|--------|-----------|-----------|------------------------------|
| **A: The Remnant** (Tech-Cult Ex-Military) | Preserved pre-war tech, hoards knowledge, views infection as "evolution" | Intel/tech niche; distinct from Military (loyal to old chain of command) | Raiders hate them (tech theft), Military wary (rogue), Civilians trade, Horde ignores |
| **B: Wasteland Syndicate** (Organized Crime / Trade Cartel) | Controls supply routes, mercenary contracts, information brokerage | Economic/political niche; non-ideological | Raiders = competitors, Military = bribe targets, Civilians = customers, Horde = hazard |
| **C: The Cult of the New Dawn** (Religious / Ideological) | Believes infection is divine judgment; "guides" horde via rituals | Spiritual/ideological niche; can indirectly influence horde | Raiders fear them, Military dismisses, Civilians split, Horde = tool |

### Recommendation: **Option A — The Remnant**

**Why:** 
- Completes the ideological spectrum: Chaos (Raiders) ↔ Order (Military) ↔ Survival (Civilians) ↔ **Knowledge/Preservation (Remnant)** ↔ Mindless (Horde)
- Enables **tech progression** tied to faction rep (unique weapon mods, pre-war blueprints)
- FNV dual-axis rep works cleanly: Fame = access to vault-tech; Infamy = assassins sent
- Distinct from Military: Remnant = rogue splinter cell, not chain-of-command

**Faction Rep Matrix (Dual-Axis, FNV Style):**
```
                | Raiders | Military | Civilians | Horde | Remnant | Player-Founded
----------------|---------|----------|-----------|-------|---------|---------------
Player Action   |         |          |           |       |         |
Kill Raider     |  Infamy |  Fame    |  Fame     |  —    |  Fame   |  Context
Kill Military   |  Fame   |  Infamy  |  Context  |  —    |  Context|  Context
Help Civilian   |  Infamy |  Context |  Fame     |  —    |  Context|  Fame
Clear Heart     |  Context|  Fame    |  Fame     |  Infamy| Fame   |  Fame
Trade Tech      |  Context|  Context |  Context  |  —    |  Fame   |  Context
```

---

## 3. Alignment Expression (Action-Based, No Meter)

### Research: Fallout New Vegas Dual-Axis
- **Fame** (positive) + **Infamy** (negative) tracked SEPARATELY per faction
- Combined → Reputation tier: **Vilified → Hated → Disliked → Neutral → Liked → Respected → Idolized**
- Companions react to specific faction rep (leave if faction hates player)
- No global "karma" meter — only per-faction consequences

### Implementation (Aligns with M8-EXT-68)

```python
# Per-faction reputation state
class FactionReputation:
    fame: int = 0      # only increases
    infamy: int = 0    # only increases
    
    @property
    def tier(self) -> ReputationTier:
        net = self.fame - self.infamy
        if net <= -200: return VILIFIED
        if net <= -100: return HATED
        if net <= -50:  return DISLIKED
        if net <=  50:  return NEUTRAL
        if net <= 100:  return LIKED
        if net <= 200:  return RESPECTED
        return IDOLIZED
    
    @property
    def is_hostile(self) -> bool:
        return self.tier <= HATED or (self.tier == DISLIKED and self.infamy > 100)
```

**Player-Facing Expression:**
- **No HUD meter** — only dialogue barks, vendor prices, patrol behavior, companion chatter
- **Faction-specific consequences:** Military IDOLIZED = supply drops; Raiders VILIFIED = ambush squads
- **Both paths reach endpoints:** High Fame with Remnant = vault access; High Infamy with Military = black-market contacts

---

## 4. Player-Zombie Leadership

### Research: Urban Strife Screamer Mechanics
- Screamers **alert all zombies in expanded radius** → converts passive horde to pursuit
- AI prioritizes distance + scream emission over direct attack
- Horde coordination = emergent from alert propagation, not central command

### Constraint: Player Leads All EXCEPT Horde (M8-EXT-53)

### Recommendation: **Indirect "Beacon" Control — No Direct Command**

| Mechanism | How It Works | Limits |
|-----------|--------------|--------|
| **Pheromone Grenade** (craftable, M5-EXT-65 scent) | Throws scent cloud; nearby zombies investigate cloud location for 30s | Radius 15m; cooldown 60s; attracts ferals too |
| **Screamer Corruption** (M5-EXT-64 sleeper + M11-EXT-64 hearing) | Player "turns" a Screamer via stealth takedown + tech (Remnant blueprint); corrupted Screamer screams on player command | 1 active at a time; 5-min duration; loud = alerts humans |
| **Horde Redirect Beacon** (late-game, Remnant tech) | Deployable device mimics horde "food signal"; pulls horde to beacon location | Expensive; single-use; horde arrives aggressive (not controlled) |

**Explicitly NOT:**
- ❌ RTS-style unit selection / move orders
- ❌ "Zombie companion" that follows player
- ❌ Direct horde pathing control

**Why:** Preserves "Horde = mindless force of nature" (core pillar), gives player **tactical influence** not strategic command, fits stealth/scent/hearing systems already spec'd (M5/M11).

---

## Cross-References to Existing Spec

| Decision | Spec Blocks Affected |
|----------|---------------------|
| Horde Cadence | M8-EXT-67 (drift), M8-EXT-68 (alignment), M5-EXT-64/65 (hearts/scent) |
| 5th Faction (Remnant) | M8-EXT-53 (leadership), M8-EXT-68 (rep), M13-EXT-55/59/61 (modding hooks) |
| Alignment | M8-EXT-68 (action-based), all faction interaction blocks |
| Player-Zombie | M5-EXT-64/65 (scent/sleeper), M11-EXT-64/65 (hearing/stealth), M8-EXT-53 |

---

## Next Gate: GO-2 Recommendations

**Vertical Slice Critical Path (highest leverage):**

| Priority | Sub-Milestone | Blocks | Why |
|----------|---------------|--------|-----|
| **1** | **M5.2** (Scent/Pheromone) | ~14 | Enables horde redirect beacon, screamer corruption, sleeper awakening |
| **2** | **M5.3** (Stealth/Approach) | ~7 | Completes player toolset for indirect horde influence |
| **3** | **M8.5** (Faction Rep Systems) | ~4 | Implements dual-axis fame/infamy for all 5 built-in + player faction |
| **4** | **M2.9** (Weapon/Combat Polish) | ~10 | Finishes M2.7 integration; needed for Remnant tech rewards |

**Deferred (depend on above):**
- M2.6 (Navigation mesh extensions) — needs M5 scent for horde pathing
- M4.6 (World gen / horde spawners) — needs cadence math finalized
- M8.6/8.7 (Advanced faction/horde AI) — needs Remnant defined

---

## Appendix: Raw Research Notes

### 7DTD Blood Moon (xgamingserver.com)
> "On Blood Moon night the game ignores its normal spawn budget and instead deliberately routes large, escalating waves of zombies directly at every player, all of them in their fastest, most aggressive state. They do not lose interest, they sprint, and they actively pathfind toward you and destroy blocks to reach you."

### SoD2 Plague Hearts (Fandom)
> "If you kill enough plague zombies and Freaks nearby or a Screamer screams nearby, the Heart will become stirred and then fully awakened after which it will send out hordes periodically to take over an empty building or invade outposts."

### FNV Reputation (Fallout Wiki)
> "Reputation is separated into two categories: Fame and Infamy. Fame is positive and Infamy is negative. Both types of reputation are tracked separately, the combination of which determines one's reputation with any given faction."

### Urban Strife Zombie AI (urbanstrife.wiki)
> "Instead, their AI prioritizes maintaining distance from survivors while emitting screams that alert all zombies within an expanded radius. A single Screamer can convert an entire passive horde into pursuit mode within two turns."

---

**Status:** Recommendations ready for user sign-off. Awaiting GO-2 on M5.2/M5.3/M8.5/M2.9 batch or alternative.