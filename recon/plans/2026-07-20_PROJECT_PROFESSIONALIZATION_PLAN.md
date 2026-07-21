# ZombieEngine: Professionalization Plan
**Based on:** actual repo state audit 2026-07-20  
**Constraints:** C++20, Vulkan 1.3, enkiTS (no fibers/dynamic groups), Jolt 5.6.0, 40-TU zero-warnings, RTX 2070S 8GB  
**Goal:** Remove root pollution, standardize layout, align docs/tests/build to engine best practices from the 50-paper corpus.

---

## Actual State (Verified)

| Area | Current | Problem |
|------|---------|---------|
| Source | `src/` with 12 subsystems | No `include/` separation; headers mixed with cpp |
| Tests | `src/tests/` has 6 files; `tests/` empty | Wrong location; not discoverable |
| Build artifacts | Root has `.exe`, `.pdb`, `.log`, `.bat`, `.ini` | Root pollution; shouldn’t be tracked |
| Logs | `logs/session.jsonl`, `engine.log`, `val*.log` | Runtime output in repo |
| Docs | `docs/research/` has 1 file; `recon/` has plans | Scattered; plans should be in `docs/plans/` |
| Spec | `spec/` with M0–M13 + APPENDICES | Good; keep read-only |
| Assets | `assets/models`, `assets/textures`, `assets/fonts`, `assets/prefabs` | Good location |
| Build trees | `build/`, `build-asan/`, `build-ninja/` | Multi-config OK for now, but should be gitignored |
| Shaders | `shaders/` at root with `.spv` | Good location; `.spv` should not be tracked |
| vcpkg | `vcpkg_installed/`, `custom-ports/`, `custom-triplets/` | Good; add `vcpkg.json` manifest |
| Hermes | `.hermes/`, `.hermis/` temp dirs at root | Should be gitignored |
| CMake | Root `CMakeLists.txt` | Needs subdirectory structure |
| CI | None visible | Needs `.github/workflows/` |

---

## Phase 1: Root Cleanup ( immediate win )
**Goal:** Remove 11 root-pollution files before anything else

Files to remove from repo:
- `engine.log`
- `val.log`
- `val_final.log`
- `_bld4.log`
- `_cfg4.log`
- `_bld4.bat`
- `imgui.ini`
- `pipeline_cache.bin`
- `C:LocalCoder_verify_out.txt`
- `CDB.txt` / `CDB2.txt` if present

Add to `.gitignore`:
- `build*/`, `*.pdb`, `*.ilk`, `*.log`, `*.bat`, `*.ini`, `imgui.ini`
- `engine.log`, `pipeline_cache.bin`, `C:*`
- `.hermes/`, `.hermis/`
- `vcpkg_installed/` (keep custom-ports, custom-triplets)

Commit: `chore: remove root pollution files + update .gitignore`

---

## Phase 2: Header Separation
**Goal:** Split `src/` into `src/` (cpp) + `include/ze/` (public headers)

Current: `src/core/Engine.h`, `src/ecs/Entity.h`, etc.  
Target: `include/ze/core/Engine.h`, `include/ze/ecs/Entity.h`

Steps:
1. Create `include/ze/` with matching subdirectories
2. Move all `.h`/`.hpp` from `src/` to `include/ze/`
3. Update all `#include "core/Engine.h"` → `#include "ze/core/Engine.h"`
4. Update CMake `target_include_directories` to point to `include/`
5. Verify 40-TU build still compiles zero-warnings

Commit: `refactor: split headers into include/ze/`

---

## Phase 3: Test Reorganization
**Goal:** Move `src/tests/` → `tests/unit/`, populate `tests/integration/` and `tests/perf/`

Steps:
1. Move `src/tests/*` → `tests/unit/`
2. Create `tests/integration/README.md` and one skeleton test
3. Create `tests/perf/README.md` and one perf baseline template
4. Update CMake to add `tests/` subdirectory
5. Wire `ctest` to run both unit + integration

Commit: `test: reorganize into tests/unit tests/integration tests/perf`

---

## Phase 4: CMake Modernization
**Goal:** Professional multi-config CMake with options and CI-ready presets

Steps:
1. Add `cmake/` directory with:
   - `cmake/ZeHelpers.cmake` — shared macros, compiler warnings
   - `cmake/ZeCompilerFlags.cmake` — MSVC/Clang/GCC flag tables
   - `cmake/presets/` — Debug/RelWithDebInfo/ASAN presets
2. Refactor root `CMakeLists.txt` to use `add_subdirectory(src)` pattern
3. Add options: `ZE_BUILD_TESTS`, `ZE_BUILD_TOOLS`, `ZE_ENABLE_ASAN`, `ZE_ENABLE_PROFILING`
4. Add `vcpkg.json` at root for exact dependency pinning
5. Verify `cmake --preset` works for all 3 configs

Commit: `build: modernize CMake, add presets, add vcpkg manifest`

---

## Phase 5: GitHub Actions CI
**Goal:** Automated 40-TU zero-warnings + ASAN + tests gate

Create `.github/workflows/ci.yml` with 4 jobs:
1. **Windows MSVC Debug** — 40-TU build, zero-warnings check
2. **Windows MSVC RelWithDebInfo** — release-like build
3. **Windows ASAN** — memory safety check
4. **clang-tidy** — static analysis scan

Each job:
- Checkout code
- Setup vcpkg
- Configure with preset
- Build with `--parallel 40`
- Run tests via `ctest`
- Upload artifacts on failure

Commit: `ci: add GitHub Actions 40-TU + ASAN + tests + tidy gate`

---

## Phase 6: Documentation Standards
**Goal:** Professional docs layout aligned to 50-paper research corpus

Steps:
1. Move `recon/plans/*` → `docs/plans/`
2. Move `recon/research/*` → `docs/research/`
3. Move `spec/` → `docs/spec/`
4. Create `docs/ARCHITECTURE.md` — bindless jobified Vulkan overview
5. Create `docs/CONTRIBUTING.md` — branch naming, commit rules, CI expectations
6. Create `docs/API.md` — public header index (Doxygen-ready)
7. Create `docs/guides/` — platform setup, profiling, debugging
8. Add `CODEOWNERS` file: `docs/spec/` = @jakeb, `engine/` = @hermes
9. Create per-subsystem README in `src/*/README.md` explaining ownership

Commit: `docs: restructure into docs/{plans,research,spec,api,guides} + CODEOWNERS`

---

## Phase 7: Engine Quality Gates
**Goal:** Align code to 50-paper + user constraints

Steps:
1. Add `ze/core/ZEAssert.h` with `ZE_ENSURE`, `ZE_CHECK`, `ZE_VERIFY`
2. Add namespace enforcement to clang-tidy (`google-build-namespaces`, `modernize-namespaces`)
3. Add `engine/include/ze/` to public include path; enforce no relative `../` escapes
4. Document enkiTS constraints in `src/jobs/README.md`:
   - No fibers
   - No dynamic task group creation per-frame
   - All task groups registered at init
5. Document physics constraints in `src/physics/README.md`:
   - Jolt fixed-depth-threshold contacts
   - Symmetric Gauss-Seidel XPBD
6. Add Vulkan validation layer docs in `src/render/README.md`:
   - AMD 8×8 VRS preferred
   - `VK_EXT_memory_budget` enforcement for 6GB VRAM
7. Add `docs/memory-invariants.md` from plan section 9

Commit: `feat: add engine quality gates + subsystem READMEs`

---

## Phase 8: Asset Pipeline + Git Hygiene
**Goal:** Clean asset workflow and git hygiene

Steps:
1. Split `assets/` into `assets/source/` and `assets/cooked/`
2. Add cook step in `scripts/tools/cook_assets.py`
3. Gitignore `assets/cooked/**`
4. Add `.gitattributes` for binary diff suppression
5. Add `scripts/tools/validate_spec.py` — verify spec block IDs don't collide
6. Add `scripts/tools/verify_milestones.py` — check milestone coverage

Commit: `chore: asset pipeline split + validation scripts`

---

## Execution Order
1. Phase 1: root cleanup (5 min, immediate win)
2. Phase 2: header separation (10 min, build-verification)
3. Phase 3: test reorganization (5 min)
4. Phase 4: CMake modernization (15 min)
5. Phase 5: CI setup (10 min)
6. Phase 6: docs (10 min)
7. Phase 7: quality gates (10 min)
8. Phase 8: asset pipeline (5 min)

**Total estimated: ~70 minutes of focused work**

Each phase is:
- Independently committable
- Build-verified before moving to next
- Reversible via git if needed
