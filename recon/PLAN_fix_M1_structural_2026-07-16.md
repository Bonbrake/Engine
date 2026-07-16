# PLAN — Repair M1.md Structural Corruption (2026-07-16)

## Status
- **BLOCKING**: `spec/M1.md` is corrupted in **HEAD** (already committed). The debug tool (`recon/ti_debug.py`) does NOT parse M1 and is unaffected — verified: 0 references to M1 in `ti_debug.py`, `--selftest` is 7/7 PASS.
- This plan requires editing a **milestone spec file** (read-only gate). Execute only after explicit GO.

## What Is Broken (measured, not guessed)
| Check | Result |
|---|---|
| `#### [M1-EXT-` headers in M1.md | 41 |
| `#### [M4-EXT-` headers in M1.md (foreign — belong in M4.md) | 32 |
| `#### [M4.5-EXT-` headers in M1.md (foreign — belong in M4.5.md) | 29 |
| frontmatter `ext_blocks` | **7** (wrong — should be 41) |
| Same M4/M4.5 blocks present in their OWN files? | YES (M4-EXT-12/13/14/15 each appear once in M4.md) → M1 copies are pollution |
| `git show HEAD:spec/M1.md` grep `M4-EXT` | 32 → corruption is committed, not working-copy only |
| Run-on lines (single line holds whole block, no newlines) | pervasive from ~line 107 onward |
| `Player-Facing Impact` + next `#### [M1-EXT-` welded on one line | 7 lines |

The verifier reports only 4 "missing subsection" errors on M1, but that is a **false positive** caused by the welded/run-on lines desyncing its parser. The real defect is the foreign-block pollution + frontmatter + run-on formatting.

## Root Cause (hypothesis)
A bad paste in an earlier session welded M4 + M4.5 milestone content into M1.md (likely during a "merge everything" operation). The `fc9c9b2 "repair M1 frontmatter (ext_blocks 334->42)"` commit suggests M1 was already abnormal; the 32+29 foreign blocks were never stripped.

---

## Execution Plan (phased, reversible, ≤3-file batches)

### Phase 0 — Safety net (no spec edits yet)
- `git tag M1-corrupt-snapshot` on current HEAD so the corrupted state is recoverable.
- Record baseline counts (above table) in this plan's "Evidence" section.

### Phase 1 — Verify-before-delete gate (do NOT skip)
For every foreign header (`M4-EXT-*`, `M4.5-EXT-*`) found in M1.md:
- Assert the same ID exists in `spec/M4.md` or `spec/M4.5.md`.
- If ANY foreign block is NOT present in its home file → STOP. That block is real content, not pollution; escalate to user.
- (Already confirmed for 12/13/14/15; script will check all 61.)

### Phase 2 — Extract & remove foreign blocks from M1
- Python script (reversible, writes to a temp, diff-able): walk M1.md, split on `#### [` boundaries, drop any block whose ID starts with `M4-EXT-` or `M4.5-EXT-`.
- Bodies are run-on (no newlines) → split logic must use the NEXT `#### [` as the block terminator (same rule as `verify_m0_parity.py`).
- Output: M1.md containing ONLY `M1-EXT-*` blocks.
- Sanity: resulting file must have exactly 41 `#### [M1-EXT-` headers, 0 foreign headers.

### Phase 3 — Fix M1-internal defects
1. **Un-merge 7 welded lines**: split `##### Player-Facing Impact <text> #### [M1-EXT-XX]` → paragraph line + header line.
2. **Re-break run-on subsections**: insert newline before each `##### ` (Systems Touched / Math / How It Works / Reference Implementation / Player-Facing Impact) and each ```` ``` ```` fence, but ONLY when mid-line (preceded by non-newline text). Must NOT touch `#####` inside prose (e.g. "Player-Facing Impact:" mid-sentence) — use a targeted regex anchored on `##### ` at start-of-content-position, not mid-word.
3. **Fix frontmatter**: `ext_blocks: 7` → `ext_blocks: 41`.
4. **Validate skeleton**: every M1-EXT block has `tags`+`tl;dr`+`meta`+`ctx`, the 5 `#####` subsections, and an `<a id="M1-EXT-XX">` anchor; IDs ascending.

### Phase 4 — Regenerate M1.index.json sidecar
- Rebuild `spec/M1.index.json` to match cleaned block set (count + 7 keys + anchors). Mirror what was done for M4/M4.5.

### Phase 5 — Verify (3-way, mandatory before "done")
1. `python scripts/verify_m0_parity.py` → **M1 must read OK (0 FIX)**.
2. Cross-ref: every `[Mx-EXT-NN]` cited in M1 resolves to a real definition (the stripped M4/M4.5 citations like `[M4-EXT-12]` still resolve because those blocks exist in M4.md — no broken refs introduced).
3. M4.md + M4.5.md still PASS (untouched by Phase 2).
4. **Debug-tool guarantee**: `python recon/ti_debug.py --selftest` → still 7/7 PASS. Additionally add a guard test asserting ti_debug never opens `M1.md` (or, if later extended to scan M1, tolerates run-on lines). Document this invariant.

### Phase 6 — Commit in ≤3-file batches
- Batch A: `spec/M1.md` (cleaned) + `spec/M1.index.json`.
- Batch B: this plan file + any `ti_debug.py` invariant test added in Phase 5.4.
- Each batch verified independently before commit.

---

## Rollback
If Phase 3/4 produces a worse state: `git checkout M1-corrupt-snapshot -- spec/M1.md spec/M1.index.json`.

## Out of scope
- Re-auditing M4 / M4.5 content (already verified clean, 73/35 OK).
- The TI gap-fill EXT loop (already complete: 8 blocks pasted, verified).
- Any new feature work.

## Evidence (baseline, captured 2026-07-16)
- M1-EXT: 41 | M4-EXT in M1: 32 | M4.5-EXT in M1: 29 | frontmatter ext_blocks: 7
- ti_debug.py references to M1: 0 | ti_debug --selftest: 7/7 PASS
- M4.md / M4.5.md verifier: 73 OK / 35 OK
