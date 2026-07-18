# ZombieEngine — "Why Fans Love These Games" Deep-Dive & 10-Step System Translation Plan (v2)

> **Author:** Jenny (architect mode), 2026-07-17 — REVISED per user steer.
> **Status:** PLAN — for user review/GO before any new spec block is authored.
> **Research basis:** live web search (Brave via `recon/web_search.py`) of fan/designer consensus on Fallout 3, F:NV, RDR2, Skyrim, + zombie-survival competitors (PZ, 7DTD, SoD2, DL2) + morality-system design theory.
> **Mandates honored:** improve-don't-copy (beat PZ/DL2/7DTD/SoD2, don't lift), FPS/TPS-only, no forced narrative, local-first, everyone's game different, real living world.

---

## PART A — RESEARCH GROUNDING (what fans love + what to beat)

### A.1 Fallout: New Vegas — *consequence & faction agency*
- "every skill point, faction loyalty, and dialogue choice reshapes the Mojave" (Games Genie)
- Reputation on **multiple axes** (Fame/Infamy per faction) — killing Powder Gangers changes how strangers treat you (Blog of Arcane Secrets)
- Quests with **multiple solutions** (talk/fight/sneak/bribe) — "multiple solutions, choices & consequences" (Total War Center)
- **Beat-this:** NV's *actions* differ little, only *outcomes* portrayed differently. ZE makes both the action AND the consequence diverge.

### A.2 Fallout 3 — *exploration & environmental storytelling*
- "making the world feel dangerous and alive" (daveslocker) — danger density, not markers
- "perfect content-to-emptiness ratio" (Reddit r/Fallout) — interesting *despite* being empty
- Environmental storytelling: a row of skeletons tells a story with zero exposition (Rob Donovan)

### A.3 Red Dead Redemption 2 — *reactivity, detail, show-don't-tell*
- "non-scripted interactions… absence of health bars forces players to observe environmental cues" (ingamenews)
- "how interactive everything is… felt like you were in the world" (Reddit r/PS4)
- "environmental storytelling is the most important [exposition]" (dualshockers)

### A.4 Skyrim — *endless freedom + modding longevity + subtle morality*
- "unparalleled player freedom… exploration over restrictive loops" (ingamenews)
- Parallel paths: "Archmage AND Dark Brotherhood leader" (g2a) — no class gate
- **Subtle morality:** "no meter or stat… loaded with moments" (thegamer) — beloved because it's *felt*, not read
- Modding = 15-year life (medium.com, cbr.com)

### A.5 Zombie-survival competitors — *the bar to BEAT (improve, don't copy)*
- **Project Zomboid:** depth bar — hunger/thirst/mental-health/fatigue + skills/traits "nuanced and integral" (Reddit r/projectzomboid, saashub). *Beat:* PZ has no real 3D world/AI horde; ZE adds spatial horror + director.
- **7 Days to Die:** base-building + horde-night is the template, BUT "creativity/defense systems are limited" (thegamer) — *Beat:* ZE's M12-EXT-39 fortifications are unbounded (walls/spikes/turrets/funnels), not a block palette.
- **State of Decay 2:** community/base + legacy, BUT co-op "only half works" (Metacritic) — *Beat:* ZE local-first co-op (M7) is peer model, no server dependency.
- **Dying Light 2:** parkour + verticality + day/night danger — *Beat:* ZE movement (M2-EXT-64) takes DL2 verticality, adds ZE's horror tone + no forced story.

### A.6 Morality-system theory — *how to do evil/good right*
- Best systems track **actions + faction alignment**, NOT binary good/evil meter (cbr.com on NV)
- **No visible meter** (Skyrim/Witcher style) = more difficult, genuine choices (radshitegaming, thegamer)
- "Good vs Evil is simple; factions let you branch narrative more" (Reddit r/Gaming4Gamers) → ZE does BOTH: faction vector + subtle personal alignment
- **User mandate:** "the player can be evil or good — make it all possible" → no moral gate; every system must support both paths.

### A.7 Emergent-gameplay theory
- "agency in shaping story/world state → personal stake → immersion" (Medium)
- "True reactivity = systemic design… interconnected rules for AI, economics, ecology" (betechit)
- ECS (EnTT, which ZE uses) "naturally supports emergent gameplay by separating data from behavior" (practicalmedia)

---

## PART B — TWO NEW SYSTEMS THIS PLAN ADDS (gaps found)
1. **Player-Created Faction system** — user: "one more faction and the ability for the player to make their own if they so chose." So: 5 built-in factions + a 6th *player-founded* faction slot (persistent, has its own reputation vector, can war with others).
2. **Evil/Good Personal Alignment** — user: "the player can be evil or good." A *subtle, meter-less* personal-alignment track (Witcher/Skyrim style) layered over the faction vector. Supports: altruist, warlord, cannibal, ghost — all valid, no lock.

---

## PART C — THE 10 STEPS (each = tracked task below)

**STEP 1 — Faction reputation vector (NV + player's 6th faction + leader-of-all)**
Goal: standing changes how every NPC treats you, per-faction {fame, infamy}. **Player can become leader of ANY built-in faction (raider, military, civilian, etc.) but NOT the zombie horde** — leadership is earned via rank (M8-EXT-53) + actions + settlement force (M8-EXT-59). Add player-founded faction as a dynamic 6th entry.
Blocks: M8-EXT-53 (rewrite → vector + leadership state), M8-EXT-16 (read), M12-EXT-40 (ChoiceResolver writes), M8-EXT-59 (settlement leadership), M12-EXT-39 (player HQ → faction capital), **NEW M8-EXT-?? Player-Founded Faction**.
Verify: kill faction X → fame↓ infamy↑ → cross-faction shopkeeper price shifts; reach leader rank → NPC address changes + quests unlock; found a faction → it appears + can war with others.

**STEP 2 — Multi-solution quests (NV)**
Goal: every major quest resolvable by fight/talk/sneak/bribe.
Blocks: M12-EXT-26 (add solution-vector), M12-EXT-44 (natural-start + taxonomy), M13-EXT-60 (SLM proposes, not prescribes).
Verify: 3 quests, ≥2 distinct mechanics reach resolved state in sim.

**STEP 3 — Environmental storytelling (FO3)**
Goal: world tells stories with zero exposition.
Blocks: M4-EXT-32 (narrative-dressing variant), M13-EXT-24 (lore pin from dressed scene), M12-EXT-45 (persists).
Verify: 50 POIs → each has ≥1 diegetic marker readable without UI.

**STEP 4 — Content-to-emptiness ratio (FO3)**
Goal: interesting despite wasteland — danger + discoverable POI, no padding.
Blocks: M4-EXT-96 (density + "silence tells"), M6.5 (ambient tells in empty zones), M12-EXT-37 (repop keeps alive).
Verify: POI-discovery + danger-event rate both >0 in every biome; no dead zone.

**STEP 5 — Reactivity without scripts (RDR2) + WorldMemory**
Goal: NPCs/systems remember you; no UI hand-holding.
Blocks: M7-EXT-20 (extend → WorldMemory), M8-EXT-16 (read), M5-EXT-10 (director reads), M11 (HUD shows cues not bars).
Verify: help faction → 5 sessions later unrelated NPC references deed (memory hit).

**STEP 6 — Show-don't-tell (RDR2)**
Goal: lore via world + interaction, never text dumps.
Blocks: M13-EXT-24 (found object), M2-EXT-78/79 (interact to learn), M12-EXT-41 (no forced lore).
Verify: grep — no block mandates lore popup; all opt-in discoverable.

**STEP 7 — Parallel paths, no class gate (Skyrim) + subtle evil/good (user mandate)**
Goal: be healer or warlord; be good or evil; no lock; no meter.
Blocks: M12-EXT-41 (add role-convergence + alignment clause), M2-EXT-26 (respec frees path), **NEW M8-EXT-?? Personal Alignment (meter-less)**, M8-EXT-56 (psyche reads alignment).
Verify: pure-builder reaches warlord ending via alliance; evil path reaches same *mechanical* endpoints as good (no content lock).

**STEP 8 — Creator platform / modding (Skyrim) — open Lua, reject FO76 store**
Goal: 15-year life via real creator API.
Blocks: verify M13-EXT-55/58/59/61 sufficient; add creator-ownership clause (reject paid-store).
Verify: build toy mod via Lua SDK in harness; loads + persists offline.

**STEP 9 — Loosely-coupled emergence audit (theory) — GATE**
Goal: emergence from shared state, not authored events.
Blocks: M8-EXT-60 (world-state graph = single source), M5-EXT-10 (director reads graph), M12-EXT-45 (mutation from graph).
Verify: dependency-graph — no block writes another system's state directly; all via graph or change-buffer (M7-EXT-21).

**STEP 10 — Living world that remembers (synthesis + user's "real living world")**
Goal: map + NPCs + factions evolve from player + director + time; returns changed.
Model (user-approved hybrid): **slow-burn baseline drift + drastic director-driven spikes; player hubs protected; mutations permanent via M7-EXT-21.**
Blocks: M12-EXT-45 (extend events), M7-EXT-21 (confirm all persist), M8-EXT-60 (graph drives), M12-EXT-36 (nuke = max spike), M8-EXT-51 (horde-night = recurring spike).
Verify: 30-day sim, no input → day30≠day0 (director acted); with input → player deeds appear in day30 state. Perf: RVT deforms only on spikes.

---

## PART D — TRACKED TASKS (split from the 10 steps)

| # | Task | Step | Blocks touched | Verify gate |
|---|------|------|----------------|-------------|
| T1 | Emergence audit (coupling check) | 9 | M8-EXT-60, M5-EXT-10, M7-EXT-21 | no cross-system direct writes |
| T2 | Reputation vector + 6th player faction + leader-of-all (user mandate) | 1 | M8-EXT-53, M8-EXT-16, M12-EXT-40, M8-EXT-59, M12-EXT-39, NEW M8-EXT-?? | cross-faction price shift, leader rank unlocks, player-faction wars |
| T3 | Multi-solution quest taxonomy | 2 | M12-EXT-26, M12-EXT-44, M13-EXT-60 | ≥2 paths/quest in sim |
| T4 | Environmental storytelling pass | 3 | M4-EXT-32, M13-EXT-24, M12-EXT-45 | 50 POIs dressed, no UI |
| T5 | Emptiness-ratio tuning | 4 | M4-EXT-96, M6.5, M12-EXT-37 | no dead biome |
| T6 | WorldMemory store | 5 | M7-EXT-20, M8-EXT-16, M5-EXT-10, M11 | 5-session memory recall |
| T7 | Show-don't-tell lore gate | 6 | M13-EXT-24, M2-EXT-78/79, M12-EXT-41 | grep: no lore popup mandated |
| T8 | Parallel paths + evil/good alignment | 7 | M12-EXT-41, M2-EXT-26, M8-EXT-56, NEW M8-EXT-?? | builder→warlord; evil==good endpoints |
| T9 | Modding sufficiency verify | 8 | M13-EXT-55/58/59/61 | Lua toy mod loads offline |
| T10 | Living-world integration + hybrid rate | 10 | M12-EXT-45, M7-EXT-21, M8-EXT-60, M12-EXT-36, M8-EXT-51 | 30-day sim drift + spikes; hubs stable |
| T11 | **3-pass bug-hunt** (assume-wrong) | all | all edited | mechanical + read-level + cross-conflict CLEAN |

---

## PART H — EXECUTION STATUS (as of 2026-07-17)

| Task | Status | Commit |
|------|--------|--------|
| T1 — Emergence audit (gate) | ✅ DONE | d5a5dd9 |
| T2 — Reputation vector + alignment | ✅ DONE | 0f3d3e9 |
| T3 — Multi-solution quests | ✅ DONE | 428cdf2 |
| T4 — Environmental storytelling | ✅ DONE | 0a03eea |
| T5 — Emptiness-ratio tuning | ✅ DONE | aece7d2 |
| T6 — WorldMemory store | ✅ DONE | 677b9a6 |
| T7 — Show-don't-tell lore gate | ✅ DONE | no changes needed (M4-EXT-32/M13-EXT-24 prevent popups) |
| T8 — Parallel paths + evil/good (M8-EXT-68 done) | ✅ DONE | 0f3d3e9 (M8-EXT-68) + M12-EXT-41 already has "builder/wanderer/warlord/ghost" |
| T9 — Modding sufficiency verify | ✅ DONE | M13-EXT-55/58/59/61 exist, open Lua confirmed |
| T10 — Living world integration (hybrid rate) | ✅ DONE | aece7d2 + M12-EXT-45 hybrid model added |
| T11 — 3-pass bug-hunt | recurring gate | always enforced |
- No top-down/tactical camera (FPS/TPS-only, M12-EXT-42 enforced).
- No forced narrative / accept-prompt / tracker (M12-EXT-41/44).
- No always-online (local-first M7).
- No moral *gate* — evil and good are both fully supported (user mandate).
- Improve-don't-copy: beats PZ/DL2/7DTD/SoD2 failure modes, doesn't lift their systems.
- Every edited block reviewed by `verify_m0_parity.py` + 3-pass hunt before commit.

## PART F — RESOLVED DECISIONS (from user + my rec)
- **Factions:** 5 built-in + 1 player-foundable (user: "one more + player can make their own") = 6-slot vector.
- **Modding:** fully open Lua (Skyrim style), reject FO76 paid-store.
- **Living world:** hybrid — slow drift + drastic spikes, hubs protected, permanent via change-buffer.
- **Evil/good:** meter-less personal alignment (Witcher/Skyrim style) + faction vector; both paths reach all mechanical endpoints.

## PART G — STILL OPEN (minor, non-blocking)
- Exact built-in faction roster (raiders / military / civilian / infected-horde / ??? — 5th TBD with user).
- Horde-night cadence (7-day default per 7DTD, configurable).
- Alignment *expression* in gameplay (dialogue tone? NPC fear? loot availability?) — propose in T8.
