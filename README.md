# ZombieEngine — The Endless Quarantine

> A 100%-procedural, fully-moddable **C++ / Vulkan 1.4** game engine.
> Solo project, developed across milestones **M0 → M13**.

ZombieEngine is built on one design pillar above all others: **procedural variety
over authored content**. Every system feeds from many places, cost scales with what
is near and visible, and infrastructure that is not a focus area reuses proven
defaults. The spec is the source of truth; the code follows it.

## Status

See [`STATUS.md`](STATUS.md) — the durable, session-independent record of where the
project actually stands. **Read it first** on any fresh session. Repo navigation
and agent workflow rules are described in [`.hermes.md`](.hermes.md).

## Repository layout

| Path | Contents |
|------|----------|
| `src/` | Engine source — one subdir per module: `core`, `ecs`, `render`, `physics`, `ai`, `audio`, `modding`, `net`, `save`, `slm`, `ui`, `world`, `debug`, `events` |
| `shaders/` | GLSL → SPIR-V shader sources |
| `assets/` | Runtime assets (models, textures) |
| `tests/` | Automated tests (Catch2 / ctest) |
| `scripts/` | Build / spec / verify tooling |
| `tools/` | Project tooling |
| `spec/` | Per-milestone split spec working set + `llms.txt` index — **the live source of truth** |
| `recon/` | Research plans, audits, and professionalization plans |
| `recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md` | Research-backed plan: 50 papers, 6 reference games, 3 engine architectures, milestone map, verification gates |
| `docs/` | Live build, verification, architecture, and research docs |
| `docs/build.md` | Local build instructions |
| `docs/verify/verify.md` | Verification checklist |
| `docs/research/engine_architecture_lessons.md` | id Tech / Decima / UE5 architecture lessons |
| `archive/` | Archived milestones, audits, and historical backups |
| `build/`, `build-asan/` | Out-of-source build trees (gitignored) |
| `vcpkg_installed/` | vcpkg dependencies (gitignored) |

## Build

Dependencies are managed via **vcpkg**. Configure and build out-of-source with the
canonical build script:

```sh
cmd.exe /c "C:\ZombieEngine\scripts\build_ze.cmd"
```

This builds the default `Debug` target. Use a separate build directory for ASan:

```sh
mkdir build-asan && cd build-asan
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DENGINE_SANITIZE=ASAN -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 4
```

Key facts:
- Runtime target is **Windows 11 / MSVC v14.44 / Ninja**
- Vulkan 1.4 via volk + vk-bootstrap
- Primary test target: `build/tests/ZombieEngineTests.exe`

## Verify the spec

After any spec change, run the mechanical verifier:

```sh
python scripts/verify_m0_parity.py
```

Block counts, anchor integrity, ascending EXT-ID order, and code-fence balance must
all be clean before a change is called done.

## Contributing / agents

Milestone spec files are read-only to automation except inside explicitly-approved
plan batches. Update `STATUS.md` before ending a session that changes milestone
state. Never commit `build/`, `build-asan/`, `vcpkg_installed/`, or `logs/`.