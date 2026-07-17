# AGENTS.md — ZombieEngine Spec Navigation

> File map only. **The binding rules live in `.hermes.md`** (auto-loaded every session) — that is
> the single master; do not duplicate rules here. This file tells you HOW to load context efficiently.

## How to find anything
1. Open `llms.txt` — the canonical index. It lists every milestone file with its EXT-block count.
2. Grep `llms.txt` (or the file's own top `<details>` block index) for the block ID or subsystem you need.
3. Open ONLY that one file (e.g. `M4.md`). Do NOT load the whole `spec/` corpus.
4. Inside a milestone file, read ONLY the `#### [ID]` block you need. Each block is a complete, self-contained unit (Systems Touched / Math / How It Works / Reference Implementation / Player-Facing Impact).

## File map
- `ROADMAP.md` — build phases (P0–P4), engine-first→game order, vertical-slice gate. Read this to know WHAT TO BUILD NEXT.
- `llms.txt` — canonical AI index (phased, per-milestone file lists with `build_order`). Start here.
- `M0.md` … `M13.md` — one file per milestone (incl. `M4.5`). Each has a collapsible block index at top + `### Cluster` dividers. Sub-milestones (M2.6/2.7/2.8/2.9, M4.6, M5.1–5.4, M8.5–8.7) were merged into their parent files in Phase 9.
- `APPENDICES.md` — merged AAA-parity gap-fill (K/L/M) EXT blocks + reference/protocol index.
- `00_PROTOCOL.md` — changelogs, how-to-work, specificity protocol, engineering pillars, budgets. Read ONCE.
- `_v79_presplit.md` — untouched backup of the original 19.5k-line master spec.

## Editing rules (keep it agent-readable)
- Every EXT block header is `#### [ID] Title` (H4). Do not mix in `## [ID]`.
- Keep the 5-subsection skeleton in order. Keep blocks self-contained (explicit `[Mx-EXT-NN]` cross-refs, no "the method mentioned earlier").
- Keep IDs in ascending numeric order within a milestone.
- Strip process noise: `*(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*`, `*(gap-file ref: …)*`.
- After ANY structural edit, run a mechanical verifier (real line evidence): 0 duplicate defs, 0 dangling refs, 0 orphan bodies, code fences balanced. Do not declare done on a thrown script or a summary.

## Total scope
694 EXT design blocks across the milestone + appendix files (grep-verified 2026-07-17; spec split into per-milestone M0–M13 + M4.5 + M6.5). Vulkan 1.4 + 100% procedural + modding engine.
