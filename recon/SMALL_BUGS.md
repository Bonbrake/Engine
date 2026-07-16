# Small Bugs / Gaps — Reconciliation Findings (Phase 5.5, report-only)

These were discovered while executing the M0-parity format base (Phases 0–5).
**None were edited** — milestone files are read-only without approval. These are
handed to the user for the content-gap / authoring pass.

## Severity: HIGH (systemic, pre-existing)

### BUG-01 — Duplicate / misplaced `<a id>` anchors in M1
- **File:** `spec/M1.md` (and likely others — not yet swept).
- **Evidence:** baseline `spec/M1.md` (git tag `spec-reformat-baseline`) contains
  **77** `<a id="M1-EXT-NN"></a>` occurrences for **42** blocks = **35 duplicate/misplaced**.
- **Pattern:** each block's anchor appears BOTH as a standalone line immediately
  before its `#### [M1-EXT-NN]` header (correct) AND embedded mid-sentence inside
  the *previous* block's last subsection (e.g. M1-EXT-02's anchor sits inside
  M1-EXT-01's `Player-Facing Impact` prose at the original line 48).
- **Impact:** Harmless to the current verifier (it only checks the standalone
  anchor preceding the header), but pollutes the file, breaks naive anchor-based
  tooling, and the duplicate IDs are invalid HTML. Should be cleaned during the
  content-gap pass: remove the mid-prose inline anchors, keep one per block.
- **Self-ref in markdown `meta` (was also broken, NOW FIXED):** `apply_deep_opt.py`
  previously emitted `depends-on: [M1-EXT-01]` self-references on all 42 blocks
  (bracketed-ID vs unbracketed-`bid` comparison bug). Fixed in the script;
  re-running on M1 now yields 0 self-refs. The pre-existing *duplicate anchors*
  themselves remain and are the item above.

## Severity: MEDIUM (pre-existing, surfaced by sort)

### BUG-02 — Misplaced anchor before M11-EXT-08 (and siblings)
- **File:** `spec/M11.md`.
- **Evidence:** `<a id="M11-EXT-09"></a>` sits on the line immediately before
  `#### [M11-EXT-08]` (verifier reported `M11-EXT-08: missing/!preceding anchor`).
- **Status:** **RESOLVED by the anchor-regression fix** (Phase 4 re-sort dropped
  the orphaned anchor line; `apply_deep_opt.py` re-injected one correct anchor per
  block for all 10 sorted files). Verified 0 anchor issues repo-wide after the fix.
  Logged here for traceability of what the regression was.

## Severity: LOW (benign, noted)

### BUG-03 — 2 M1 blocks have tl;dr == title
- **Blocks:** 2 of 42 M1 blocks (exact IDs TBD on authoring pass).
- **Cause:** the block's first real body line equals the block title (e.g. a code
  signature that opens the body), so the fallback collapses to the title.
- **Impact:** none for tooling; a slightly less informative tl;dr. Optional polish.

### BUG-04 — `cross_file_deps` frontmatter value is approximate
- **Files:** all files re-run through `apply_deep_opt.py`.
- **Cause:** the script counts distinct external `[Mx-EXT-NN]` reference tokens
  found in block bodies (a reasonable proxy), not a resolved dependency graph.
  M1 reports 518 (plausible for a foundational milestone everything references);
  M0 reports 4.
- **Impact:** informational only; not used by the verifier. Acceptable.

## Regression introduced & fixed during this session (for transparency)

- **REG-01 (introduced in Phase 4, fixed in FX):** `sort_blocks.py` extracted each
  block span starting at the `####` header, excluding the standalone anchor line
  *before* the header. Sorting therefore dropped/misplaced anchors in the 10
  sorted files (M3, M4, M4.5, M4.6, M6, M7, M8, M9, M11, M12). Fixed by (a) patching
  `sort_blocks.py` to carry a preceding `<a id>` line with its block, and (b)
  re-running `apply_deep_opt.py` on all 10 files, which strips + re-injects one
  correct anchor per block. Verified: **0 anchor issues repo-wide**, block counts
  unchanged (653 before/after), no `ext_blocks` mismatches.
