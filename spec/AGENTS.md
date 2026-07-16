# AGENTS.md — ZombieEngine Spec Navigation

> File map only. **The binding rules live in `.hermes.md`** (auto-loaded every session) — that is
> the single master; do not duplicate rules here. This file tells you HOW to load context efficiently.

## How to find anything
1. Open `llms.txt` — the canonical index. It lists every milestone file with its EXT-block count.
2. Grep `llms.txt` (or the file's own top `<details>` block index) for the block ID or subsystem you need.
3. Open ONLY that one file (e.g. `M4.md`). Do NOT load the whole `spec/` corpus.
4. Inside a milestone file, read ONLY the `#### [ID]` block you need. Each block is a complete, self-contained unit (Systems Touched / Math / How It Works / Reference Implementation / Player-Facing Impact).

## File map
- `llms.txt` — canonical AI index (H1 + summary + per-milestone file lists). Start here.
- `M0.md` … `M13.md` — one file per milestone (incl. `M2.6`, `M2.7`, `M2.8`, `M2.9`, `M4.5`, `M4.6`). Each has a collapsible block index at top + `### Cluster` dividers.
- `APPENDIX_K.md` / `APPENDIX_L.md` / `APPENDIX_M.md` — AAA-parity gap-fill EXT blocks (K/L/M style: bold subsections, not the M 5-subsection skeleton).
- `00_PROTOCOL.md` — changelogs, how-to-work, specificity protocol, engineering pillars, budgets. Read ONCE.
- `APPENDICES.md` — §5.x reference prose, not-yet-merged, Part A, close-out.
- `_v79_presplit.md` — untouched backup of the original 19.5k-line master spec.

## Editing rules (keep it agent-readable)
- Every EXT block header is `#### [ID] Title` (H4). Do not mix in `## [ID]`.
- Keep the 5-subsection skeleton in order. Keep blocks self-contained (explicit `[Mx-EXT-NN]` cross-refs, no "the method mentioned earlier").
- Keep IDs in ascending numeric order within a milestone.
- Strip process noise: `*(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*`, `*(gap-file ref: …)*`.
- After ANY structural edit, run a mechanical verifier (real line evidence): 0 duplicate defs, 0 dangling refs, 0 orphan bodies, code fences balanced. Do not declare done on a thrown script or a summary.

## Total scope
609 EXT design blocks across the milestone + appendix files. Vulkan 1.4 + 100% procedural + modding engine.
