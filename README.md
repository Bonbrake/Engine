# ZombieEngine — The Endless Quarantine

> A 100%-procedural, fully-moddable **C++ / Vulkan 1.4** game engine.
> Solo project, developed across milestones **M0 → M13**.

ZombieEngine is built on one design pillar above all others: **procedural variety
over authored content**. Every system feeds from many places, cost scales with what
is near and visible, and infrastructure that is not a focus area reuses proven
defaults. The spec is the source of truth; the code follows it.

## Status

See [`STATUS.md`](STATUS.md) — the durable, session-independent record of where the
project actually stands. **Read it first** on any fresh session. Binding agent rules
and repo navigation live in [`.hermes.md`](.hermes.md).

## Repository layout

| Path | Contents |
|------|----------|
| `src/` | Engine source — one subdir per module: `core`, `ecs`, `render`, `physics`, `ai`, `audio`, `modding`, `net`, `save`, `slm`, `ui`, `world`, `debug`, `events` |
| `shaders/` | GLSL → SPIR-V shader sources (HLSL + DXC toolchain) |
| `assets/` | Runtime assets (models, textures) |
| `tests/`, `src/tests/` | Automated tests (Catch2 / doctest, ctest) |
| `scripts/` | Build / spec / verify tooling (incl. `verify_m0_parity.py`) |
| `tools/` | Project tooling |
| `spec/` | Per-milestone split spec working set + `llms.txt` index — **the live source of truth** |
| `recon/` | Threat Interactive research + `ti_debug.py` design-by-contract debug gate (see below) |
| `docs/` | (archived) Design docs & audits — now in [`archive/docs/`](archive/docs/) |
| `archive/` | Archived milestones, audits, and runtime junk |
| `build/`, `build-asan/` | Out-of-source build trees (gitignored) |
| `vcpkg_installed/` | vcpkg dependencies (gitignored) |
| `custom-ports/`, `custom-triplets/` | vcpkg port/triplet overrides |

## Build

Dependencies are managed via **vcpkg manifest mode** (`vcpkg.json`). Configure and
build out-of-source:

```sh
cmake -S . -B build
cmake --build build --config Debug --target ZombieEngine
```

- **ASAN tree:** `build-asan` (`CMAKE_BUILD_TYPE=Release`, `ENGINE_SANITIZE=ASAN`).
- **Shader toolchain:** HLSL + DXC → SPIR-V. No GLSL-via-glslang side path.
- **Profiler:** Tracy (RenderDoc-capturable at M0).

## Verify the spec

After any spec change, the milestone spec must pass the mechanical verifier:

```sh
python scripts/verify_m0_parity.py
```

Block counts, anchor integrity, ascending EXT-ID order, and code-fence balance must
all be clean before a change is called done.

## TI research & debug tooling (`recon/`)

`recon/` holds the Threat Interactive (TI) rendering research and the contract-debug
gate that keeps the plan honest:

- **`ti_debug.py`** — parse-from-prose design-by-contract gate. Verifies every
  `COVERED` / `PARTIAL` claim in the plan is backed by a real spec block, flags bogus
  GAP targets, and detects proposal collisions. JSON-first (agent-consumable) with
  `--human`, `--jsonl`, `--claim`, `--kind`, `--gha`, and a built-in `--selftest`.
- **`PLAN_threat_interactive_gospel_2026-07-16.md`** — the canonical TI rendering
  rules (gospel, not suggestions).
- **`PLAN_gap_fill_ext_proposals_2026-07-16.md`** — proposed new EXT blocks to close
  gaps the TI research surfaced.
- **`pull_ti_transcripts.py`**, **`analyze_transcripts.py`** — TI YouTube transcript
  pipeline (`recon/transcripts/`).

Run the gate after any plan edit:

```sh
python recon/ti_debug.py          # JSON (piped) or human view (TTY)
python recon/ti_debug.py --selftest
```

## Contributing / agents

`.hermes.md` defines the milestone-gated workflow and repo hygiene rules. In short:
never commit `build/` or `vcpkg_installed/`; update `STATUS.md` before ending a
session that changes milestone state; milestone spec files are read-only to the agent
except inside explicitly-approved plan batches.
