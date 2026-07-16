# Milestone Audit — Deep Per-File Checklist

> Generated 2026-07-16. Every milestone scored on 8 dimensions. Re-run `hermes-audit-deep.py` to refresh.

## Summary

- Milestones audited: **16**

- Total own-label blocks: **567**

- Build-order sequence: `[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]` (duplicates: none)

- Frontmatter schema complete on all: **True**

## Per-milestone scorecard

| MS | blocks | ascending | dup IDs | anchor-before | inline-anchor | skeleton-gaps | fences | unresolved-xref | embedded-foreign |
|----|--------|----------|---------|---------------|---------------|---------------|--------|-----------------|------------------|
| M0 | 52 | Y | 0 | 52 | 0 | 0 | Y | 0 | 0 |
| M1 | 7 | Y | 0 | 7 | 0 | 2 | Y | 0 | 318 |
| M10 | 25 | Y | 0 | 25 | 0 | 1 | Y | 0 | 0 |
| M11 | 48 | Y | 0 | 48 | 0 | 1 | Y | 0 | 1 |
| M12 | 15 | Y | 0 | 15 | 0 | 1 | Y | 0 | 0 |
| M13 | 34 | Y | 0 | 34 | 0 | 1 | Y | 0 | 0 |
| M2 | 75 | Y | 0 | 75 | 0 | 4 | Y | 0 | 0 |
| M3 | 34 | Y | 0 | 34 | 0 | 1 | Y | 0 | 0 |
| M4 | 71 | Y | 0 | 71 | 0 | 2 | Y | 0 | 0 |
| M4.5 | 29 | Y | 0 | 29 | 0 | 1 | Y | 0 | 0 |
| M5 | 59 | Y | 0 | 59 | 0 | 4 | Y | 0 | 0 |
| M6 | 22 | Y | 0 | 22 | 0 | 1 | Y | 0 | 0 |
| M6.5 | 13 | Y | 0 | 13 | 0 | 1 | Y | 0 | 0 |
| M7 | 19 | Y | 0 | 19 | 0 | 2 | Y | 0 | 0 |
| M8 | 41 | Y | 0 | 41 | 0 | 2 | Y | 0 | 0 |
| M9 | 23 | Y | 0 | 23 | 0 | 1 | Y | 0 | 0 |

## Legend

- **blocks**: own-label `Mx-EXT` headers in the file.

- **ascending**: block IDs in numeric order (no gaps-violation / no out-of-order).

- **dup IDs**: duplicate own-label IDs (should be 0).

- **anchor-before**: `#### [ID]` headers that have a correct `<a id>` anchor immediately preceding them.

- **inline-anchor**: misplaced anchors inside block bodies (should be 0).

- **skeleton-gaps**: blocks missing one or more of the 5 subsections (Systems Touched/Math/How It Works/Reference Implementation/Player-Facing Impact) — these are the deferred content-authoring backlog, NOT structural bugs.

- **fences**: code-fence balance (should be Y).

- **unresolved-xref**: cited `[Mx-EXT-NN]` with no definition anywhere (should be 0).

- **embedded-foreign**: `#### [OtherMilestone-EXT]` blocks defined inside this file (M1 is a reference mega-file with 318; flagged for optional extraction).

## Known non-defects (tracked, not fixed autonomously)

1. **Skeleton gaps** across all files = the 113-block content-gap backlog (authoring, deferred).

2. **M1 mega-file** (318 embedded foreign blocks) — duplicate-def pattern; extraction (option B) pending user approval (gated spec-file edit).

3. **Frontmatter `ext_blocks`** counts OWN blocks only; embedded foreign blocks are not counted (M1 corrected to 7).

4. **APPENDICES.md** (K/L/M gap-fill) audited separately — not a milestone, excluded from this table.