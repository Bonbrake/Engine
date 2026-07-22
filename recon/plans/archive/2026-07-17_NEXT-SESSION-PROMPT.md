# ZombieEngine Spec — Next Session Handoff & Continuation Prompt

> Copy everything between the `=== PROMPT START ===` and `=== PROMPT END ===`
> markers and paste it into the new session as the opening message.

=== PROMPT START ===

CONTINUATION HANDOFF — ZombieEngine Design Spec: Verify → Fix Bugs → Fill Gaps

You are continuing work on the ZombieEngine game design specification. No prior
conversation is in context. DO NOT rely on memory alone — the authoritative plan
documents and spec files are ON DISK. Read them first (exact paths below).

## FILE MAP (read these first — they are the source of truth)
Repo root: `C:/ZombieEngine`
- SPEC (authoritative): `spec/M0.md` … `spec/M13.md`, plus `spec/M4.5.md`,
  `spec/M6.5.md`, and `spec/APPENDICES.md` (index of all block IDs). Total = 694
  EXT blocks. `llms.txt` at repo root declares this the single source of truth.
- DESIGN PLAN docs (read BOTH before doing anything):
  - `recon/plans/2026-07-16_SIX-DECISIONS-RESOLUTION-PLAN.md` — the resolved design
    decisions / either-or ledger (E48, E62–E93, etc.). This is where "why" lives.
  - `recon/plans/2026-07-16_PROPOSED-EXT-BLOCKS.md` — the 62 plan-fleshing blocks
    that were MERGED into the spec (marked MERGED). Use to confirm nothing was dropped.
- LATEST CLEAN BACKUP: `archive/plans/2026-07-17_ze-gameplan-v45-restore-point/`
  (14 spec files + llms.txt). Restore from here if a sweep corrupts the doc.
- GIT: branch `spec/m0-parity-reformat`, origin `github.com/Bonbrake/Engine.git`.
  Working changes are UNCOMMITTED since commit `019c5d4` (immersive-layer commit).
  Do NOT push without user confirmation.

ACTION ON START: before any edit, run:
  terminal: `cd /c/ZombieEngine && ls spec/ && wc -l spec/M*.md`
  read_file: `recon/plans/2026-07-16_SIX-DECISIONS-RESOLUTION-PLAN.md` (skim)
  read_file: `recon/plans/2026-07-16_PROPOSED-EXT-BLOCKS.md` (skim)
This grounds you in the actual state instead of trusting this handoff's summary.

## CURRENT STATE (end of previous session)
- Structurally CLEAN: 0 duplicate anchors, 0 duplicate headings, 0 blocks missing
  the AI-first skeleton, 0 orphan cross-references, all `ext_blocks:` frontmatter
  counts match actual block counts. Backup at
  `archive/plans/2026-07-17_ze-gameplan-v45-restore-point/`.
- Immersive interaction layer ADDED: M9-EXT-30..35 (welding, hotwire, lockpick,
  circuit-hack, pickpocket, molotov), M2-EXT-78/79/80/81 (interaction shell,
  proficiency gating, limb damage/dismemberment, bleeding), M8-EXT-66 (tool economy).
- Jolt Physics pinned at **v5.6.0** (latest, released 2026-07-11) — confirmed in
  M1.md and matches the current upstream release. All physics blocks (M2 char
  controller, M5 ragdoll, M9 vehicles) correctly reference Jolt primitives.
- User STILL reports seeing bugs ("fix bugs i stil see some"). Structural QA passes,
  but content / visual / logic bugs may remain that only surface on read. These have
  NOT yet been hunted.

## HARD CONSTRAINTS (never violate)
- LOCAL-FIRST: offline play primary, friends co-op secondary. No cloud dependency in
  core systems. (Decision E87 locked — local-per-save.)
- MODERN-DAY ONLY: no magic, plasma rifles, jetpacks, teleporters, force fields.
  Weapons/tech must be real-world plausible for a present-day setting.
- NO PLAYER-FACING DEV CONSOLE: the dev consoles (M10-EXT-13, M11-EXT-24) are
  engine-debug tooling only, compiled out of retail builds. Never make them player-facing.
- "Don't change anything about it" means: fix bugs / fill gaps / ADD — do NOT silently
  rewrite blocks that are fine. Preserve the user's decisions and voice.

## DESIGN PRINCIPLE — IMPROVE, DON'T COPY (critical, user-emphasized)
When adding or fleshing systems, DO NOT just lift a mechanic from another game.
Take the BEST parts of State of Decay 2, Dying Light 2, Project Zomboid, 7 Days to
Die, Watch Dogs, Deus Ex, Cyberpunk, and IMPROVE them. Fix the known failure modes:
- No Man's Sky "vast-but-empty" → layered density, every space has a reason.
- Starfield "sameness" → procedural variation with authored beats, not copy-paste POIs.
- Fallout terminal-hack is clunky → our hacking is a BETTER computer minigame (see below).
Every interaction/loop must be the genre-best version, not a clone. The user said:
"improve the systems from other games not just copy."

## HACKING MINIGAME — REWORK REQUIRED (user correction)
M9-EXT-33 is currently named "Circuit Hack Minigame." This is WRONG direction.
The user clarified: "no terminal" meant DO NOT copy Fallout's terminal-hack minigame.
The hacking interaction should instead be a COMPUTER-based intrusion minigame that is
BETTER than what other games ship — inspired by (not cloned from) Watch Dogs
(network/device hijack), Deus Ex (multi-path hack), Cyberpunk (breach protocol).
Required for the rework:
- Rename M9-EXT-33 away from "Circuit Hack"; make it a computer/network intrusion
  minigame (e.g. "Computer Intrusion Minigame" or similar).
- Design it as an IMPROVEMENT: e.g. real-time pressure + multiple valid solution paths
  + consequences that cascade into the world (disable cameras → stealth route opens,
  spoof alarms → faction standing shifts). Not a static word-guessing puzzle.
- Keep the AI-first skeleton; tie it to M8-EXT-66 (tools: laptop/exploit-kit) and
  M2-EXT-78/79 (shell + proficiency). It should still align with the immersive
  interaction layer (world never pauses).
NOTE: this is a REWORK of an existing block, not a new ID — keep M9-EXT-33's ID.

## ENVIRONMENT QUIRKS (learned the hard way — follow these)
- The `search_files` tool is BROKEN for `spec/` paths (rg: `os error 3`,
  "The system cannot find the path specified"). Use terminal `grep -F` / `grep -nE`
  instead. Also bracket-heavy regexes break rg — use `grep -F` for literal IDs.
- Web egress: `curl -o /absolute/msys/path` FAILS (curl connects, HTTP 200, but the
  file write errors / 0 bytes). It WORKS via: (a) `curl -sS URL -o ./relative.html`
  (relative path), or (b) Python `urllib.request` writing to a relative path. Use
  these for any online research. Do NOT declare egress dead — it works, just not with
  absolute `-o` paths.
- AI-first normalization regex lesson: when parsing `depends-on: [A], [B], [C]` lists,
  use `\[[^\]]*\]` (negated class) NOT `\[(.*?)\]`. The lazy `.*?` stops at the FIRST
  `]`, truncating the list and producing false "broken reverse-link" reports. The doc
  is actually fine; the verifier was buggy.
- All new EXT IDs must be collision-checked BEFORE writing: grep the target file's max
  ID AND `APPENDICES.md` (it re-lists provisional IDs, so true max can be higher than
  the last block in the file). Known true maxes post-merge: M2=81, M4=82, M5=63,
  M8=66, M9=35, M12=26, M13=42.

## THE PLAN (execute in THIS ORDER — PLAN FIRST, then do the work)
1. PLAN: write a `todo` list breaking the session into phases. Present scope to the
   user for sign-off BEFORE any bulk edits. Phases:
   (a) Verify nothing missed (structural + live-source + reverse-graph + claim dupes).
   (b) BUG HUNT + FIX — PLANNED, not ad-hoc (see below).
   (c) Fill gaps (add missing systems as new blocks, improve-don't-copy).
   (d) Rework M9-EXT-33 hacking minigame (computer-based, better-than-others).
   (e) Final QA + refresh backup (v46).
   (f) Commit + push to `spec/m0-parity-reformat` ONLY after user confirms.

2. VERIFY NOTHING MISSED:
   - Re-run structural QA (confirm still PASS).
   - Pull LIVE sources via Python urllib for each major system domain and confirm the
     spec covers current best practice: survival loop, immersive interactions, base
     fortification, faction/settlement macro-sim, vehicle physics, modding, hacking.
     Flag any genuinely missing system.
   - Cross-check every `depends-on`/`depended-by` resolves (no orphans, no dangling).
   - Hunt duplicate/conflicting claims (two blocks stating different world size,
     different XP rates, etc.).
   - Hunt content bugs the user can see on read (see phase b categories).

3. BUG HUNT + FIX (PLANNED — distinct from structural QA):
   The user sees READ-LEVEL bugs structural QA can't catch. Categorize the hunt:
   - C1 Contradictions: `tl;dr` vs `ctx` vs `Algorithm` disagree; `Examples` contradict
     the `Algorithm`; a block says X but its `depends-on` target says not-X.
   - C2 Broken refs: `depends-on`/`depended-by` points at a wrong or non-existent block
     (e.g. the known M3-EXT-09 vs M8-EXT-09 inventory citation trap — inventory is
     M8-EXT-09, NOT M3-EXT-09).
   - C3 Math errors: `##### Math` uses variables never defined; formula dimensionally
     wrong; `Failure Modes` contradict the `Algorithm`.
   - C4 Duplicated/near-duplicate blocks: same system defined twice with drift.
   - C5 Principle violations: a block leaks non-modern tech, or implies cloud-dependency
     in core, or describes a player terminal.
   Build a categorized bug list, get sign-off on which to fix (some may be intended),
   then fix EACH WITH EVIDENCE: read the actual block, patch, re-run the relevant
   verifier. Never blind-fix. Report count fixed per category.

4. FILL GAPS (improve-don't-copy): add missing systems as new EXT blocks
   (collision-checked IDs), cohesive with existing blocks, tied to real `depends-on`,
   keeping the full AI-first skeleton. For each: note which game inspired it and what
   we do BETTER.

5. REWORK M9-EXT-33 hacking minigame per the DESIGN PRINCIPLE + HACKING MINIGAME
   sections above. Keep ID M9-EXT-33.

6. FINAL QA + refresh backup (v46).

7. Commit + push to `spec/m0-parity-reformat` ONLY after the user confirms.

## WHAT IS NOT YET DONE
- Nothing has been committed/pushed since the immersive-layer commit `019c5d4`. The
  AI-first normalization + 3 new blocks (M2-EXT-80/81, M9-EXT-35) + frontmatter fixes
  + the untouched M9-EXT-33 are UNCOMMITTED working changes on branch
  `spec/m0-parity-reformat` (origin: github.com/Bonbrake/Engine.git).
- User mentioned "Joel's update" added new systems beyond Jolt. We confirmed Jolt 5.6.0
  is in the doc. If Joel is a source with MORE than Jolt, the user must paste those
  specifics — do not invent them.
- Content-level bug hunt NOT yet done (structural QA can't catch read-level bugs).
- M9-EXT-33 hacking minigame NOT yet reworked to computer-based/better-than-others.

## LOAD-BEARING BLOCK MAP (so you know the territory)
- M0 = gold-standard format reference (replicate its structure).
- M1 = engine/principles; Jolt 5.6.0 pinned ~line 668.
- M2 = physics / character / interactions: M2-EXT-48 (context radial), 78 (interaction
  shell), 79 (proficiency gating), 80 (limb damage/dismemberment), 81 (bleeding).
- M3 = inventory/crafting base. NOTE: old blocks cited `M3-EXT-09` for inventory but the
  correct inventory block is `M8-EXT-09` — verify citations point at M8-EXT-09.
- M4 = worldgen: M4-EXT-23 biome sizing; E48 RESOLVED = 25 km².
- M5 = AI / scent / fear / horde / ragdoll.
- M6 / M6.5 = audio.
- M7 = persistence / mods.
- M8 = settlement / economy / NPC: M8-EXT-09 (inventory cache), 14/15 (barter/currency),
  16 (NPC residents), 18 (skill/XP), 53 (faction standing), 56 (psyche), 59 (settlement
  sim), 60 (world-state cascade), 61 (trade routes), 66 (tool economy).
- M9 = vehicles / interactables: M9-EXT-09 (fuel — welding spark can ignite it),
  30–35 (interaction minigames; M9-EXT-33 = hacking, TO BE REWORKED).
- M10 = systems2: M10-EXT-13 (dev console, ship-disabled).
- M11 = UI: M11-EXT-24 (dev console, ship-disabled).
- M12 = game loop / horde nights.
- M13 = modding: M13-EXT-60 (SLM quest generator).

## FIRST ACTION
1. Run the ACTION ON START commands above (ls spec/, skim both plan docs). Confirm
   the doc state matches this handoff (694 blocks, v45 backup exists).
2. Rebuild the `todo` list from step 1 of THE PLAN below.
3. Present the plan (including bug-hunt categories + M9-EXT-33 rework) for sign-off
   BEFORE any edit. Do NOT bulk-edit until the user approves scope.
4. If anything in this handoff conflicts with the actual plan docs
   (SIX-DECISIONS-RESOLUTION-PLAN.md / PROPOSED-EXT-BLOCKS.md), the on-disk plan docs
   WIN. Re-read them rather than guessing.

=== PROMPT END ===
