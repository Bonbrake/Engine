# ZombieEngine — Build Roadmap & Milestone Format Spec

> Rebuilt 2026-07-16 from Phase 9 consolidation. Research basis: vertical-slice
> methodology (Nineva Studios, Unity, Rami Ismail milestone framework) + Anthropic
> context-engineering (lean, retrieval-targeted spec for agents). **Engine first,
> then game.** One thin end-to-end slice before widening.

## Build philosophy (from research)
- **Vertical slice first:** get boot → render → one system → playable before widening.
  Don't build all of M0 then all of M4 then discover integration is broken.
- **Milestones force clarity + recurring progress checks** (Rami Ismail). Each phase
  must produce something runnable, not just "more code."
- **Lean spec, retrieval-targeted** (Anthropic): agent loads ONE milestone file per task.

## Build phases (engine → game)
| Phase | Name | Milestones | What it delivers |
|-------|------|-----------|------------------|
| P0 | Foundation | M0, M1 | Vulkan 1.4 bootstrap + capability tiering; ECS core. Nothing runs without these. |
| P1 | Core engine | M2, M3 | Jolt physics + EventBus; renderer (draw, passes, debug-draw). First triangle on screen. |
| P2 | World & simulation | M4, M4.5, M5, M6, M6.5 | Procedural worldgen + streaming; render features; AI (scent/fear/horde); audio. **VERTICAL SLICE TARGET: walkable procedural world with one zombie, 30 FPS Tier-0.** |
| P3 | Systems & content | M7, M8, M9, M10 | [M7], settlement sim (SEIR/faction/truss), vehicles, [M10]. Horde behavior, villages, driving. |
| P4 | Game layer | M11, M12, M13 | UI/HUD, [M12], modding. The playable GAME (not just engine). |

Build order (topological, engine-first):
`M0 → M1 → M2 → M3 → M4 → M4.5 → M5 → M6 → M6.5 → M7 → M8 → M9 → M10 → M11 → M12 → M13`

## Canonical milestone frontmatter (NEW schema)
Every `spec/Mx.md` MUST carry:
```yaml
---
title: <real human title, not "Mx milestone spec">
milestone: Mx
layer: engine | game
build_phase: P0-foundation | P1-core | P2-world | P3-systems | P4-game
build_order: <int 1..16>
ext_blocks: <n>
clusters: <n>
cross_file_deps: <n>
self_contained: true
index: see llms.txt or the <details> block index below
---
```
- `layer` / `build_phase` / `build_order` are NEW — they tell the agent WHEN to build.
- `title` must be specific (M0 = "Vulkan 1.4 native bootstrap + capability tiering").

## llms.txt format (NEW)
Group bullets under `## Phase Pn — Name` headers so the agent sees build order at a glance.
Each bullet keeps `ext_blocks` count.

## Vertical-slice acceptance gate (end of P2)
- Compiles on Tier-0 (RTX 2060 6GB). Boots to a procedural world. One AI zombie
  navigates via scent/fear. Debug-draw works. 30 FPS floor. No silent hangs.
- Until this gate passes, do NOT start P3 content breadth.

## Notes
- Cross-file "depends-on" in block meta is often a reference cite (M1 embeds foreign
  blocks), not a hard build edge — build order above follows true engine architecture.
- Appendix K/L/M blocks live in APPENDICES.md (gap-fill, not a build phase).
