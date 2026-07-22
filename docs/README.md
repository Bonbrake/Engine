# ZombieEngine Documentation

## Layout

| Path | Contents |
|------|----------|
| `docs/build/build.md` | Build instructions, prerequisites, presets |
| `docs/status/STATUS.md` | Current project status and completion tracking |
| `docs/verify/` | Verification gates, headless smoke notes, test conventions |
| `docs/research/` | Externally sourced research feeding spec decisions |
| `docs/research/engine_architecture_lessons.md` | id Tech / Decima / UE5 architecture lessons |
| `docs/guides/` | Contributor and operator guides |
| `docs/architecture/` | System-level design notes, data-flow, render graph |

## Research-backed planning

The current evidence-heavy planning document is:

- `recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md`

It contains 50 sourced papers, 6 reference games, 3 engine studies, an M0-M13
milestone map, design pillars, verification gates, and explicit research gaps.

## Build quick links

- Root README `README.md` for full build+verify instructions.
- `scripts/build_ze.cmd` for canonical local build on Windows / MSVC / Ninja.
- `docs/verify/verify.md` for headless smoke + test verification steps.

## Active vs archived docs

Active planning documents live in `recon/plans/`. Older planning snapshots are
no longer duplicated under `archive/`; code/docs commits are the durable record.