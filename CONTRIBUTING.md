# Contributing to ZombieEngine

## Setup
- Windows 11, MSVC 14.44, Ninja, vcpkg.
- Build: `scripts/build_ze.cmd build-ninja`
- Tests: `./build-ninja/ZombieEngineTests.exe`
- Sanity: `./build-ninja/sanity_suite.exe`
- Headless smoke: `./build-ninja/ZombieEngine.exe --headless --quit-frame 1`

## Workflow
- Work on `spec/m0-parity-reformat` or a named feature branch off it.
- Keep commits small and scoped.
- Never edit `spec/M*.md` without explicit approval.
- If you touch EXT blocks, also update `recon/plans/APPENDIX_EXECUTION_MAP.md`.

## Quality bar
- Compiler warnings treated as errors in CI.
- New systems need a unit test and/or reference implementation.
- Plan claims need `[S]` sourced/cached evidence, `[E]` with linked evidence, or `[X]` with fetch target named.
