# PLAN — M1.md Content Pass (author missing subsections)

> Companion to `recon/PLAN_fix_M1_structural_2026-07-16.md`. The structural plan's
> Phase 2-4 (un-weld, strip foreign, add anchors+layers, regen sidecar) is BUILT and
> DRY-RUN VERIFIED (`recon/fix_m1.py` → `recon/_m1_fixed.md`). Running it on the real
> file yields **36 FIX** on the verifier, because 35 of the 41 M1-EXT blocks are
> **missing real subsections in the source** — not a weld artifact. A reformat cannot
> invent them (forbidden to fabricate technical content). This plan authors them.
>
> **READ-ONLY GATE:** `spec/M1.md` is a milestone spec file. This plan edits it.
> Execute only after explicit GO (double-stored: this plan + agent memory).

## Measured defect split (from `recon/_m1_content_needs.json`, post-structural)
- 41 M1-EXT blocks total.
- **3 blocks need ONLY mechanical fixes** (malformed `#####` header / code not wrapped):
  `M1-EXT-07`, `M1-EXT-12`, `M1-EXT-23`. No prose authoring.
- **35 blocks need AUTHORED prose** for genuinely-absent subsections:
  - `Math` absent: 28 blocks (01-06, 09, 10, 13, 15-28, 39-42, 45, 46)
  - `Systems Touched` absent: 29 blocks (01-06, 09, 10, 13, 15-28, 43-51)
  - `How It Works` absent: 8 blocks (01-06, 09, 13)
  - `Player-Facing Impact` malformed (header weld): 9 blocks — **mechanical**, not authoring
  - `Reference Implementation` missing/uncwrapped: 8 blocks (01-06, 09, 13 + 1 wrap) — mostly **mechanical**
- `M1-EXT-13` is fully empty (0 body lines) → needs all 5 subsections authored.

## Authoring standard (real prose, grounded — NO fabrication)
Each missing subsection is written from the block's **own existing code** + legitimate
cross-references. Hard rules:
- **Systems Touched** — enumerate concrete engine subsystems/modules this EXT reads or
  writes; cite real `[Mx-EXT-NN]` deps/consumers already in the block's `> **meta**`.
  No speculative systems.
- **Math** — if the block's code encodes a formula/algorithm, document variables + units.
  If there is genuinely no closed-form math, write exactly:
  `Algorithmic — see Reference Implementation (no closed-form equation).` Do NOT invent an equation.
- **How It Works** — 2-4 numbered steps describing the mechanism, grounded in the existing
  code/Reference Implementation. No new behavior invented.
- **Reference Implementation** — ensure existing code is wrapped under
  `##### Reference Implementation` + a ``` fence. (mechanical)
- **Player-Facing Impact** — ensure the existing impact sentence sits under a clean
  `##### Player-Facing Impact` header, alone on its line. (mechanical)
- No new features, no scope expansion. If a block's code can't support a real `Math`
  line, use the algorithmic fallback above.

## Execution order
1. **Mechanical pass (`fix_m1.py` already covers un-weld/strip/anchor/layer/TOC/sidecar).
   ADD a `fix_m1_post.py` (or extend `fix_m1.py`) to:**
   - Split malformed `##### X text…` → `##### X` + `text` line.
   - Wrap orphan code blobs under `##### Reference Implementation`.
   - Wrap orphan `Player-Facing Impact` sentences under `##### Player-Facing Impact`.
   - Re-run verifier on the copy; expect only the 35 *authored* gaps remain.
2. **Author the 35 blocks' prose** in batches (see batching). Each batch: insert the
   missing subsections into the cleaned copy, re-verify, then (on GO + apply) commit.
3. **Verify 3-way** after each batch (below).

## Batching (per user rule: ≤3 operations/commit, never 6+)
Single file, so "operations" = blocks authored. Commit **≤3 blocks per commit**
(≈12 commits for 35 blocks). Each commit independently verified + revertible.
> If you want larger batches (e.g. 5-7) since it's one file, say so — rule is literal ≤3.

## Verification (mandatory, 3-way, every batch)
1. `python scripts/verify_m0_parity.py` → M1 reads **0 FIX** (cumulative; each batch
   reduces the count, final = 0).
2. `python recon/ti_debug.py --selftest` → still **7/7 PASS** (M1 is not parsed by it).
3. `M4.md` + `M4.5.md` still PASS (untouched by this pass).
4. Cross-ref: every `[Mx-EXT-NN]` cited in M1 resolves to a real definition.

## Rollback
- `git tag M1-content-pre` before the first content commit.
- Any bad batch: `git revert <hash>` or `git checkout M1-content-pre -- spec/M1.md`.

## Out of scope
- No foreign-block re-insertion. No M4/M4.5/M2 edits. No new EXT IDs.
- Does NOT re-audit already-clean blocks (39-53 already have skeletons? verify at run).

## Status
- **BLOCKED on GO.** Plan + helper scripts drafted; no `spec/M1.md` text written yet.
- Prerequisite structural script: `recon/fix_m1.py` (dry-run verified, spec untouched).
