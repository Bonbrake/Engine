# AUDIT: TriangleRenderer.cpp mislabels legacy occlusion code as [M1-EXT-03]

**Date:** 2026-07-13
**Auditor:** Hermes (ZombieEngine spec/batch work)
**Category:** source-comment defect — spec/code cross-reference mismatch (code's fault, not spec's)
**Status:** RESOLVED (verified 2026-07-15) — see note below.

> **RESOLVED — fixed in tree.** Commit `b591acf` (author: Ltmonkeysmash) retagged the three occlusion-query comments from `[M1-EXT-03]` to `[M1-EXT-28]` (GPU Software Occlusion Rasterizer / HZB feeder). `git grep` confirms 0 `[M1-EXT-03]` and 3 `[M1-EXT-28]` in `src/render/TriangleRenderer.cpp`; `cmake --build Debug` exits 0. The original finding below is historically accurate but the defect is now closed in code.

## Summary

Three comments in `src/render/TriangleRenderer.cpp` tag legacy occlusion-query code with the
`[M1-EXT-03]` milestone ID. `[M1-EXT-03]` in the spec is **Multi-Threaded Command Pool Matrix**
(`milestones_M0-M13_antigravity/01_M1.md:121`) — i.e. command-buffer pool allocation. The tagged
code is unrelated: it is the Hi-Z occlusion-query path that the M1 base Implementation Steps bullet
now marks **superseded by `[M4.5-EXT-08]`** (compute + atomic-counter design, no CPU readback to
double-buffer against). So the comments point `EXT-03` at the wrong system.

## Evidence (literal file:line)

`src/render/TriangleRenderer.cpp`:
```
486:    // [M1-EXT-03] Occlusion Query Pools
751:    // [M1-EXT-03] Occlusion Query Double-Buffering
785:        // [M1-EXT-03] Occlusion Query Double-Buffering
```

Spec side (`milestones_M0-M13_antigravity/01_M1.md`):
```
121: #### [M1-EXT-03] Multi-Threaded Command Pool Matrix
```

M1 base-step annotation (the struck occlusion bullet), `01_M1.md` line ~41:
```
* Occlusion query double-buffering for Hi-Z culling: ... — **superseded by `[M4.5-EXT-08]`'s
  compute + atomic-counter design, which has no CPU readback to double-buffer against; not
  implemented separately.**
```

## Why it matters

- Any future reader grepping `[M1-EXT-03]` in source lands on occlusion-query pools, not command-pool
  allocation — the opposite of what the spec says EXT-03 is. This is the same class of silent
  mislabeling the broader project history already flags (cf. AGENTS.md "Never reconstruct file content
  from another branch's history" / audit discipline).
- The occlusion-query path tagged here is the one the M1 base step now says is **not implemented
  separately** (superseded). The comments should reflect that, not imply EXT-03 owns them.

## Recommended fix (NOT applied — out of scope for this spec batch, pending engineer go-ahead)

1. Change the three comments to reference the correct system, e.g.:
   - `:486` `// [M4.5-EXT-08] Occlusion Query Pools (legacy Hi-Z path; superseded by EXT-08 compute+atomic design)`
   - `:751` / `:785` `// [M4.5-EXT-08] Occlusion Query Double-Buffering (legacy; superseded — see M1 base step)`
   (Exact target ID depends on whether the occlusion *query* pools are owned by `[M4.5-EXT-08]` or
   another system — confirm before editing.)
2. Do NOT touch `M1-EXT-03` in the spec; the spec is correct.

## Scope notes

- This audit is **separate** from milestone batch v3 (the 11 EXT entries). The batch's EXT-23 prose
  ("`[M1-EXT-03]` allocates the command-buffer pools only") is **correct** — the defect is in the
  source comments, not the spec.
- No source files were modified by this audit. Changes are documentation-only, working-tree, uncommitted.
- Unrelated: the repo working tree is dirty with many pre-existing modifications not part of this batch;
  this audit does not reconcile them.
