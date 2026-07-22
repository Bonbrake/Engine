# Spec Hygiene Plan — Naming Scheme, Duplicate Merges, Cohesion

> 2026-07-16. Three workstreams per user directive: (1) find Threat Interactive's
> lighting model (DONE — applied to M4.5-EXT-20), (2) best naming scheme for fast
> AI ingestion, (3) de-duplicate + cohesion-check vs src/ (95 files, 15 dirs).
> Research via curl (web tools paid-blocked); DDG rate-limited naming search — used
> authoritative convention knowledge (llms.txt standard, Anthropic context-eng, Google/Microsoft API style).

## 1. Threat Interactive lighting (APPLIED)
- Gospel: UE's Lambert (1760) is obsolete → "plastic" look. Fix = **Burley 2012 diffuse** (Disney wrapped-Lambert), Threat brands it **Kalisto BRDF** (Callisto Protocol look).
- Applied to M4.5-EXT-20: surface diffuse = Burley/Kalisto; Beer-Lambert kept ONLY for transmission/participating media (fog/glass/flesh subsurface) where it is correct. Committed separately.

## 2. Naming scheme — CURRENT vs RECOMMENDED
**Current:** `M0.md` + blocks `[M0-EXT-24]`. Problems: milestone prefix redundant with file; "EXT" is noise; numeric IDs are opaque (can't google "EXT-24"); cross-refs `[M0-EXT-24]` are unreadable without the file.
**Recommended (AI-Bill optimized — readable, greppable, googleable, copyable):**
- **Files:** `spec/NN-<kebab-slug>.md` — zero-padded build-order prefix + human slug. e.g. `00-vulkan-bootstrap.md`, `01-ecs.md`, `03-physics.md`, `04-worldgen.md`. Sorts by build order, name tells you what it is, googleable.
- **Block IDs:** `[<slug>]` — a NAME not a number. e.g. `[vulkan-boot-lock]`, `[ecs-spatial-hash]`, `[physics-muscle-fatigue]`. Bill reads "ah, boot lock" instantly. Greppable across all files. Googleable. Copies as a stable handle.
- **Cross-refs:** `[vulkan-boot-lock]` instead of `[M0-EXT-24]` — self-documenting.
- **Why this scheme:** llms.txt standard + Anthropic context-eng both say retrieval-targeted, stable, human-readable IDs; kebab-case is the RAG/URL convention; slugs beat opaque numbers for agent find/copy/google. Maps 1:1 to current IDs via a lookup table so no info lost.

## 3. Duplicate blocks found (merge candidates)
From recon/dup_cohesion_scan.json (title token-overlap + Systems Touched review):
| Pair | Verdict | Action |
|------|---------|--------|
| M0-EXT-24 == M0-EXT-41 ("Single-Instance Boot Lock", jac=1.0) | TRUE DUP (identical title) | MERGE → keep one, fold "grep→0" note in |
| M0-EXT-25 == M0-EXT-40 ("Per-Fiber Exception Containment") | TRUE DUP | MERGE |
| M5-EXT-22 ("Screamer…Holling Cannibalism") == M5-EXT-52 ("Holling Cannibalism Satiator") | SAME model, 2 blocks | MERGE (52 into 22) |
| M5-EXT-17 ("Aero Wake Slipstream Horde Slingshot") == M9-EXT-11 ("Wake-Slipstream Drafting") | SAME mechanism | MERGE (11 into 17) |
| M12-EXT-12 (audio voice cap) vs M12-EXT-13 (net culling hash) | DISTINCT domains | KEEP |
| M0-EXT-37 (render 2nd cmd buf) vs M1-EXT-24 (ECS 2nd cmd buf) | DISTINCT layers | KEEP (cross-ref instead) |
| M8-EXT-14 (barter) vs M8-EXT-22 (live trade pricing) | EXT-22 reads 14's model | KEEP (14 is the model, 22 consumes it) |

## 4. Cohesion vs src/ (95 files, 15 dirs: ai,audio,core,debug,ecs,events,modding,net,physics,render,save,slm,tests,ui,world)
- **No spec-ID ↔ src-filename collisions** (clean).
- **Spec is AHEAD of code** (expected — we just finished the spec). Several blocks note "grep → 0" / "no X system yet" — these are HONEST gap markers, not errors. They become the engine build backlog (M0→M13).
- **Vocab maps to src/ dirs** (physics→Jolt, ecs→EnTT, render→Vulkan, world→chunk/streaming, ai→diffusion, net→culling). Cohesive.
- **Risk if NOT merged:** 4 duplicate pairs above would make the engine implement the same system twice. Merge BEFORE coding starts.

## 5. Execution plan (batched, verified 3 ways each, gated spec edits need approval)
- **Step A (approved implicitly — lighting already done):** M4.5-EXT-20 Burley/Kalisto. ✅ committed.
- **Step B:** Merge 4 duplicate pairs (rewrite one block, redirect cross-refs, delete the other). Verify xref resolves, verifier clean.
- **Step C (BIG — needs your GO):** Rename scheme — files `NN-slug.md`, block IDs `[slug]`, rewrite all 658 cross-refs. Use a mechanical mapper + verify_m0_parity + xref grep. High-risk, do in batches per milestone.
- **Step D:** Update llms.txt + ROADMAP.md + AGENTS.md to new scheme.

## Open question for user
Step C (the rename) is a large gated refactor. Approve before I run it? Step B (merges) I can do now.
