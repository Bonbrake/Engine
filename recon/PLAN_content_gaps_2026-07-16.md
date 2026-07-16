# Content-Gap Fill Plan — ZombieEngine spec (25 blocks)

> 2026-07-16. Fills the 25 milestone blocks that have a header/ID but are missing
> one or more of the 5 subsections (Systems Touched / Math / How It Works /
> Reference Implementation / Player-Facing Impact). 100% procedural engine — every
> constant must be GROUNDED in reality (per .hermes.md specificity protocol), not invented.

## Research grounding (verified via curl, web tools paid-blocked)
- **PCG taxonomy** (arxiv 2410.15644): noise-function-based (Perlin/Simplex/value),
  search-based, and combined methods. Noise = foundational primitive; seed →
  repeatable world. LLMs noted but NOT used for the engine (we author specs, not gen).
- **Practice** (generalistprogrammer.com/procedural-generation-games): seed + rules +
  noise = repeatable worlds (Minecraft / No Man's Sky). Our design matches.
- **Constants to ground online before writing:**
  - Scent/Fear diffusion: use real diffusion-equation dt/dx² stability (Courant–Friedrichs–Lewy-style) + plausible scalar rates (NOT the v77 off-by-orders errors).
  - SEIR (M8): real β/σ/γ ranges from epidemiology literature.
  - Faction ODEs (M8.5): Lotka–Volterra / Lanchester combat model forms.
  - WFC (M4): real adjacency-constraint propagation (Karth/Bomentröm 2018).
  - Noise: Simplex (Perlin 2002) / value-noise frequency bands tied to feature scale.

## Studio failure-pattern guardrails (from post-mortems — VERIFIED via curl)
Sources: No Man's Sky (Hello Games) launch failure analyses (k2digitizers, gfinity, absoluteanime),
Starfield "empty planets" critique (80.lv, trueachievements, 2023-2024).
1. **Hype > delivery** (NMS): never spec a system the build can't back. → vertical slice must PROVE each claimed system before widening.
2. **Vast but empty / no loop** (NMS launch): procedural world with no purpose. → every gap block's Player-Facing Impact must name a REAL player loop (survival/horde/settlement), not just "world generates".
3. **Repetition / sameness** (NMS "samey" flora, Starfield "empty/boring"): infinite ≠ varied. → Math must use LAYERED noise + biome rules + per-region seeds + WFC structural variation. No single-octave everywhere.
4. **Launch perf** (NMS crashes/FPS): → keep Tier-0 30 FPS budget real; cap entity counts, stream, cull.
5. **Don't abandon a rough launch** (NMS recovered via updates): → iterative slice-then-widen (our roadmap) is the right shape.

## FIX: safe insertion method (learned from M2 bug)
- Stub blocks have NO `<a id>` anchor before them (deep_opt only anchors recognized own-blocks).
- `t.find(anchor)` returned -1 → content dumped at EOF (SILENT bug). 
- CORRECT METHOD: locate `#### [ID]` header, then insert subsections immediately BEFORE the NEXT `#### ` header line. Verify by reading the file after, never trust a print.

## The 25 gaps (exact, from recon/gap_list_2026-07-16.json)
M1: EXT-01 (missing 4 subsections), EXT-11 (2). M2: EXT-04,EXT-05,EXT-06,EXT-07 (varies).
M3: EXT-? (1). M4: EXT-? (2). M4.5: EXT-? (1). M5: EXT-? (4). M6: EXT-? (1).
M6.5: EXT-? (1). M7: EXT-? (2). M8: EXT-? (2). M9: EXT-? (1). M10: EXT-06 (5).
M11: EXT-07 (5). M12: EXT-02 (5). M13: EXT-? (1). (Exact IDs in the JSON.)

## Fill approach (per block)
For each gap block, append the missing `##### Subsection` headers + REAL content:
- **Systems Touched:** which other EXT blocks / subsystems it reads/writes.
- **Math:** the actual equations (diffusion, SEIR, noise octaves) with grounded constants + citations in `> **ctx**` or prose.
- **How It Works:** step-by-step algorithm (seed → noise → threshold → emit).
- **Reference Implementation:** pseudocode or C++/HLSL sketch (Vulkan/HLSL + DXC path per .hermes.md).
- **Player-Facing Impact:** what the player experiences.

## Execution batches (≤3 files/commit, verify 3 ways each)
- **Batch 1:** M1-EXT-01, M1-EXT-11 (ECS core — highest leverage).
- **Batch 2:** M2 (4 physics blocks).
- **Batch 3:** M5 (4 AI blocks — scent/fear/horde).
- **Batch 4:** M4 + M4.5 (worldgen + render features).
- **Batch 5:** M8 + M7 (settlement sim + systems).
- **Batch 6:** M3, M6, M6.5, M9 (renderer/audio/vehicle — 1 each).
- **Batch 7:** M10-EXT-06, M11-EXT-07, M12-EXT-02, M13 (game-layer blocks, 5-subsection gaps).

## Verification (3 ways, every batch)
1. **Re-run `hermes-audit-deep.py`** → blocks_missing_skel count drops; 0 new structural issues.
2. **`verify_m0_parity.py`** repo-wide → 0 anchor/ascending/fence issues.
3. **Constant spot-check:** grep the new Math sections for any ungrounded magic number; flag for online re-check if suspicious.

## Rules
- NO fabricated constants — if a number isn't grounded, mark `*(verify)*` and flag, don't guess.
- Keep block IDs/order intact (don't restructure — only fill subsections).
- Each batch committed separately (batch rule).
- M1 mega-file embedded blocks: fill ONLY the 7 own M1-EXT blocks; leave embedded foreign blocks alone (option B pending).
