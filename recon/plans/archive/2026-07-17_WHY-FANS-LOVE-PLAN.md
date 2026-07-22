# ZombieEngine — "Why Fans Love These Games" Deep-Dive & 10-Step System Translation Plan

> **Author:** Jenny (architect mode), 2026-07-17
> **Status:** PLAN — for user review/GO before any new spec block is authored.
> **Research basis:** live web search (Brave via `recon/web_search.py`) of fan/designer consensus on Fallout 3, Fallout: New Vegas, Red Dead Redemption 2, The Elder Scrolls V: Skyrim, + emergent-gameplay design theory. Quotes cited inline.

---

## PART A — WHAT FANS ACTUALLY LOVE (research-grounded)

### A.1 Fallout: New Vegas — *consequence & faction agency*
- "every skill point, faction loyalty, and dialogue choice reshapes the Mojave" (Games Genie)
- "The game makes you take a good look at the factions and think" (Reddit r/Fallout)
- "multiple solutions, choices & consequences" — quests resolvable by talk, fight, sneak, or bribe (Total War Center)
- Reputation on **multiple axes** (Fame/Infamy per faction) — killing Powder Gangers changes how strangers treat you (Blog of Arcane Secrets)
- **The lesson:** agency is felt through *reputation that changes NPC behavior* + *quests with several valid solutions*, not through a branching tree the player can see.

### A.2 Fallout 3 — *exploration & environmental storytelling*
- "encouraged exploration by making the world feel dangerous and alive" (daveslocker.net)
- "environmental storytelling" — stumbling on a row of skeletons tells a story with zero exposition (Rob Donovan, systems designer)
- "perfect content-to-emptiness ratio that encourages exploration" (Reddit r/Fallout) — the wasteland is *interesting despite being empty*
- **The lesson:** the joy is the *walk and the discovery*, not the destination. Subtle POIs + danger density + no hand-holding.

### A.3 Red Dead Redemption 2 — *reactivity, detail, "show don't tell"*
- "complex, non-scripted interactions and the absence of traditional health bars, which forces players to observe environmental cues" (ingamenews)
- "how interactive everything is… it really did feel like you were in the world" (Reddit r/PS4)
- "environmental storytelling… the most important" form of exposition (dualshockers)
- **The lesson:** reactivity = *systems that respond without a script* (NPC remembers your hat, shopkeeper recognizes you, weather changes animal behavior). Immerse by *removing UI*, not adding it.

### A.4 Skyrim — *endless freedom + modding longevity*
- "unparalleled player freedom… design philosophy that prioritizes exploration over restrictive gameplay loops" (ingamenews)
- "You can become Archmage and wipe out the Dark Brotherhood — or lead the assassins yourself" (g2a.com) — parallel paths, no forced class
- Modding community = "artistic freedom, innovation" — keeps the game alive 15 years on (medium.com, cbr.com)
- **The lesson:** freedom is *parallel quest paths with no main-quest gate*; longevity is *a creator platform*, not DLC.

### A.5 Emergent-gameplay theory (cross-cutting)
- "giving players agency in shaping story or world state leads to a personal stake… enhances immersion" (Medium, Player-Generated Worlds)
- "True reactivity is born from systemic design… interconnected rules for AI, economics, and ecology that interact organically" (betechit)
- ECS architectures "naturally support emergent gameplay by separating data from behavior" (practicalmedia — directly relevant: ZE uses EnTT)
- **The lesson:** emergence comes from *loosely-coupled systems with shared state*, not from authored events.

---

## PART B — THE 10-STEP PLAN

Each step: **Goal → Research-backed principle → ZE system to build/extend → Block(s) → Verify**.

### STEP 1 — Reputation on multiple axes (NV lesson)
- **Goal:** standing changes how *every* NPC treats you, not just quest givers.
- **Principle:** NV's Fame/Infamy per-faction model — actions write to a vector, NPCs read it.
- **ZE system:** extend `M8-EXT-53` (standing) from single scalar → per-faction {fame, infamy} vector; NPC dialogue/price/aggro keys off it (already partly in `M8-EXT-16` psyche + `M12-EXT-40` G-EMERGE).
- **Blocks:** M8-EXT-53 (rewrite to vector), M8-EXT-16 (read vector), M12-EXT-40 (ChoiceResolver writes vector).
- **Verify:** unit-test that killing faction X members lowers fame + raises infamy → shopkeeper Y (different faction) price shifts per cross-faction bias table.

### STEP 2 — Multi-solution quests (NV lesson)
- **Goal:** every major quest resolvable by fight / talk / sneak / bribe.
- **Principle:** "multiple solutions, choices & consequences" — the *path* is player-owned.
- **ZE system:** quest *outcome* is a set of flags, not a script. Each flag reachable via ≥2 mechanics (combat, social, stealth, economic).
- **Blocks:** M12-EXT-26 (discovery quests → add solution-vector field), M12-EXT-44 (natural-start → add solution taxonomy), M13-EXT-60 (SLM proposes, not prescribes path).
- **Verify:** for 3 representative quests, prove ≥2 distinct mechanic paths reach the resolved state in the sim.

### STEP 3 — Environmental storytelling layer (FO3 lesson)
- **Goal:** the world tells stories with zero exposition.
- **Principle:** "stumbling on a row of skeletons tells a story" — baked diegetic detail.
- **ZE system:** a POI *narrative dressing* pass — corpse poses, bullet holes, dragged loot, graffiti (M13-EXT-24 lore drops) placed by worldgen + director, not by hand.
- **Blocks:** M4-EXT-32 (POI placement → add narrative-dressing variant), M13-EXT-24 (lore pin from dressed scene), M12-EXT-45 (dressing persists via change-buffer).
- **Verify:** spawn 50 POIs, assert each has ≥1 diegetic narrative marker readable without UI.

### STEP 4 — Content-to-emptiness ratio (FO3 lesson)
- **Goal:** the map is interesting *despite* being a wasteland — no empty padding.
- **Principle:** "perfect content-to-emptiness ratio" — danger + discoverable POI density, not checklist.
- **ZE system:** `M12-EXT-37` encounter zones already set density; tune `POIdensity` (M4-EXT-96) + `horrorAmbience` (M6.5) so even empty stretches have a sound/visual tell.
- **Blocks:** M4-EXT-96 (density field → add "silence tells"), M6.5 (ambient tells in empty zones), M12-EXT-37 (repop keeps it alive).
- **Verify:** measure POI-discovery rate per km² vs danger-event rate; assert both > 0 in every biome (no dead zones).

### STEP 5 — Reactivity without scripts (RDR2 lesson)
- **Goal:** NPCs/systems remember you, weather changes behavior, no UI hand-holding.
- **Principle:** "non-scripted interactions… forces players to observe environmental cues" — systemic memory.
- **ZE system:** a `WorldMemory` store (persistent, M7-EXT-20) recording player deeds + world state; NPCs (M8-EXT-16) + economy (M8-EXT-15) + director (M5-EXT-10) read it. Remove reliance on health bars / objective trackers (M12-EXT-41 already bans trackers).
- **Blocks:** M7-EXT-20 (extend to WorldMemory), M8-EXT-16 (read memory), M5-EXT-10 (director reads memory), M11 (HUD: show *cues*, not *bars*).
- **Verify:** simulate player helps faction → 5 sessions later, unrelated NPC references the deed (memory lookup returns hit).

### STEP 6 — Show-don't-tell exposition (RDR2 lesson)
- **Goal:** lore delivered through world + interaction, never text dumps.
- **Principle:** "environmental storytelling is the most important" exposition form.
- **ZE system:** all lore gated behind interaction (M2-EXT-78/79 shell) or dressed scene (Step 3); no codex auto-fill. SLM (M13) emits *found* notes, not *told* history.
- **Blocks:** M13-EXT-24 (lore = found object), M2-EXT-78 (interact to learn), M12-EXT-41 (no forced lore moments).
- **Verify:** grep spec — confirm no block mandates a lore *popup*; all lore is opt-in discoverable.

### STEP 7 — Parallel quest paths, no class gate (Skyrim lesson)
- **Goal:** be the healer or the warlord; no forced archetype; no main-quest lock.
- **Principle:** "become Archmage and wipe out the Dark Brotherhood — or lead the assassins" — paths coexist.
- **ZE system:** quest/role availability keyed to *actions*, not *class*. M12-EXT-41 (no forced narrative) + M12-EXT-44 (natural-start) already enforce; add explicit "role convergence" rule: any role reachable from any start.
- **Blocks:** M12-EXT-41 (add role-convergence clause), M2-EXT-26 (respec frees path), M8-EXT-59 (settlement role independent of combat path).
- **Verify:** assert a pure-builder start can still reach the "warlord" ending via alliance, and vice-versa.

### STEP 8 — Creator platform / modding as longevity (Skyrim lesson)
- **Goal:** the game outlives the studio via a real creator API.
- **Principle:** Skyrim's 15-year life = "artistic freedom, innovation" modding scene.
- **ZE system:** Forge (M13-EXT-55 editor + M13-EXT-59 SDK + M13-EXT-61 Lua runtime) is first-class, not bolted on. Local-first (M7) means mods don't need store approval.
- **Blocks:** M13-EXT-55/58/59/61 (already exist — this step = *verify they're sufficient* + add creator-ownership clause rejecting FO76 paid-store).
- **Verify:** build a toy mod via the Lua SDK in a test harness; assert it loads + persists offline.

### STEP 9 — Loosely-coupled systems for emergence (theory lesson)
- **Goal:** emergence from shared state, not authored events.
- **Principle:** "ECS architectures naturally support emergent gameplay by separating data from behavior" — ZE already uses EnTT.
- **ZE system:** audit that M5 (AI) / M8 (settlement) / M12 (game) / M7 (persistence) share EnTT state without hard couplings; director (M5-EXT-10) is the *only* orchestrator, reading world-state graph (M8-EXT-60).
- **Blocks:** M8-EXT-60 (world-state graph — confirm it's the single source), M5-EXT-10 (director reads graph), M12-EXT-45 (map mutation from graph).
- **Verify:** dependency-graph check — no block writes another system's state directly; all via the world-state graph or change-buffer.

### STEP 10 — Living world that remembers (synthesis of all)
- **Goal:** the map + NPCs + factions evolve from player + director + time; returns changed.
- **Principle:** FO3 danger + NV consequence + RDR2 reactivity + Skyrim freedom, fused.
- **ZE system:** `M12-EXT-45` (living dynamic world) + `M7-EXT-21` (change-buffer persists) + `M5-EXT-10` (director mutates) + `M8-EXT-60` (world-state graph) + `M12-EXT-40` (G-EMERGE choice ripples).
- **Blocks:** M12-EXT-45 (extend events list), M7-EXT-21 (confirm all mutations persist), M8-EXT-60 (graph drives mutation).
- **Verify:** 30-day sim with no player input → assert map state at day 30 ≠ day 0 (director acted); with player input → assert player's specific deeds appear in day-30 world-state.

---

## PART C — EXECUTION ORDER (architect's call)

1. Step 9 (emergence audit) — *first*, because every other step depends on clean coupling.
2. Step 1 (reputation vector) — foundational, everything reads standing.
3. Step 5 (WorldMemory) — enables reactivity (Step 5 feeds 1, 2, 10).
4. Step 2 (multi-solution quests) — needs reputation + memory.
5. Step 3 + 6 (environmental storytelling + show-don't-tell) — content layer.
6. Step 4 (emptiness ratio) — tuning pass.
7. Step 7 (parallel paths) — design clause.
8. Step 8 (modding longevity) — verify Forge sufficiency.
9. Step 10 (living world) — integration + final sim verify.
10. Cross-pass bug-hunt (3 independent passes, assume-wrong) before commit.

---

## PART D — WHAT THIS PLAN DOES NOT DO
- Does **not** add top-down/tactical camera (violates FPS/TPS-only mandate — already enforced in M12-EXT-42).
- Does **not** force narrative or accept-prompts (M12-EXT-41/44).
- Does **not** mandate always-online (local-first, M7).
- Every new/changed block is reviewed by `verify_m0_parity.py` + 3-pass bug-hunt before commit.

## PART E — OPEN QUESTIONS FOR USER
1. Reputation vector size: how many factions at launch? (NV had ~7; ZE's settlement sim M8-EXT-59 implies a few — confirm count.)
2. Modding: fully open Lua (Skyrim-style) vs curated (FO4 Creation Kit style)? Forge (M13) currently implies open — confirm.
3. Step 10 "living world" event rate: aggressive (RDR2-like constant change) vs restrained (FO3-like slow burn)? Affects perf budget (M4.5 RVT deform).
