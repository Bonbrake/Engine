# Feature/System Completeness Audit — Phase 9 consolidation fallout check

**Date:** 2026-07-16  **Scope:** every EXT system/feature from the v79 master must still
exist in the consolidated spec after archive + Phase 9 merge.

## Method 1 — title-presence vs v79 master
- `_v79_presplit.md` (original 19.5k-line master) contains **374 unique EXT IDs**.
- Current `spec/` contains **658 distinct EXT IDs** (374 v79 + 49 planned + gap-fill).
- Title-based match (markdown-normalized, substring fallback): **0 v79 systems missing.**
- 36 raw-title mismatches are normalization noise (case / stray `*`); all resolve by
  substring to a real current block.

## Method 2 — git deletion check (beta → HEAD)
- `git grep` EXT IDs: beta=100, HEAD=101.
- 21 IDs "gone at HEAD" are exactly the Phase 9 renumbered child blocks
  (M2.7-EXT-*, M4.6-EXT-02, M5.x-EXT-*, M8.x-EXT-*).
- Content-preservation check (title of each renumbered block searched in HEAD):
  **0 genuinely lost** — all 21 survive under their new parent IDs (M2/M4/M5/M8-EXT).

## Method 3 — structural verifier
- `verify_m0_parity.py` repo-wide: **0 structural issues** (anchors, ascending IDs,
  clusters, fences, frontmatter all clean).
- Cross-reference resolution: **0 unresolved** across 658 citations.

## Verdict
**EVERY single feature/system from the v79 master is present in the consolidated spec.**
Nothing was deleted or lost in the archive move or Phase 9 merge. The only ID changes are
intentional renumbers (child → parent namespace), all content-preserving.

## Notes / non-loss items (for traceability)
- 49 "planned/retired" IDs (M11-EXT-52..59, M13-EXT-31..50, etc.) were NEVER in a
  versioned spec — they live only in the antigravity `_PLAN_INDEX`/`_RETIRED` reference
  files. Not losses. Phase 7.6 added the 49 real planned systems as new Mx-EXT blocks.
- M1.md embeds 318 foreign-block definitions (mega-file reference pattern) — these are
  duplicates of home-file defs + APPENDICES refs, not losses. Flagged separately for
  possible extraction (option B), pending user approval (gated spec-file edit).
- 113 blocks remain content-gap stubs (missing subsections) — authoring backlog, not
  missing systems.
