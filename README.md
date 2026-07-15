# ZombieEngine — The Endless Quarantine

A 100%-procedural, moddable C++/Vulkan game engine. Solo project, developed across
milestones **M0 → M13**.

## Status

See [`STATUS.md`](STATUS.md) — the durable, session-independent source of truth for
where the project actually stands. **Read it first** on any fresh session.

## Layout

| Path | Contents |
|------|----------|
| `src/` | Engine source — one subdir per module (`core`, `ecs`, `render`, `physics`, `ai`, `audio`, `modding`, `net`, `save`, `slm`, `ui`, `world`, `debug`, `events`) |
| `shaders/` | GLSL/SPIR-V shader sources |
| `assets/` | Runtime assets (models, textures) |
| `tests/`, `src/tests/` | Automated tests (ctest) |
| `scripts/` | One-off build/spec/verify tooling |
| `tools/` | Project tooling |
| `docs/` | Design docs & audits — [`MASTER_PLAN_ENDLESS_QUARANTINE.md`](docs/MASTER_PLAN_ENDLESS_QUARANTINE.md) (canonical milestone spec), [`IDEA.md`](docs/IDEA.md), [`ANTIGRAVITY_RULES.md`](docs/ANTIGRAVITY_RULES.md), `docs/audit/` |
| `spec/` | Per-milestone split spec working set + `llms.txt` index |
| `milestones_M0-M13_antigravity/` | Per-milestone working files |
| `build/`, `build-asan/` | Out-of-source build trees (gitignored) |
| `vcpkg_installed/` | vcpkg dependencies (gitignored) |

## Build

Dependencies are managed via **vcpkg manifest mode** (`vcpkg.json`). Configure and build
out-of-source:

```sh
cmake -S . -B build
cmake --build build --config Debug --target ZombieEngine
```

The ASAN/Release tree is `build-asan` (`CMAKE_BUILD_TYPE=Release`, `ENGINE_SANITIZE=ASAN`).

## Contributing / agents

`AGENTS.md` defines the milestone-gated workflow and repo hygiene rules. Never commit
`build/` or `vcpkg_installed/`; update `STATUS.md` before ending a session that changes
milestone state.
