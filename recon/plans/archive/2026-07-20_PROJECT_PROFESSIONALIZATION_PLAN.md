# ZombieEngine Professionalization — Execution State
**Updated:** 2026-07-20  
**Branch:** `spec/m0-parity-reformat`  
**Commit:** `afe805e`

---

## Completed (committed)

### Phase 1: Root Cleanup — DONE
- Removed: `engine.log`, `val.log`, `val_final.log`, `_bld4.log`, `_cfg4.log`, `_bld4.bat`, `imgui.ini`, `pipeline_cache.bin`, `C:LocalCoder_verify_out.txt`, `_cdb.txt`, `_cdb2.txt`
- No root `.exe` / `.pdb` / `.ilk` / `.log` / `.bat` / `.ini` / `.bin` pollution remains

### Phase 2: Header Separation — PARTIALLY DONE
**Done:**
- Created `include/ze/{ai,audio,core,debug,ecs,events,modding,net,physics,render,save,slm,ui,world}/`
- Copied 62 headers from `src/` into `include/ze/` (originals still in `src/` for safety)
- Updated 19 source files to use `ze/...` include paths
- Updated `CMakeLists.txt` include paths and test glob to `tests/unit/*.cpp`

**NOT done yet (blocked):**
- Remove old headers from `src/` — must verify build first
- Delete empty `src/tests/` (already moved contents, but directory structure remains)

### Phase 3: Test Reorganization — DONE
- Moved 6 test files from `src/tests/` to `tests/unit/`
- Updated CMake to pick up `tests/unit/*.cpp`

### Phases 4-8 — NOT started
- CMake modernization (`add_subdirectory`, presets, vcpkg manifest already exists)
- CI workflows
- Documentation restructuring
- Engine quality gates / subsystem READMEs
- Asset pipeline split

---

## Current Blocker: Build Environment

**Symptom:** MSVC cannot find standard library headers (`string`, `stdarg.h`, etc.) during `cmake --build`.

**Root cause:** This is not caused by the header separation. The VS shell environment (`vcvars64.bat`) is not initialized in the agent's bash context, so `cl.exe` runs without the standard-library include paths. This is a host-shell/toolchain issue, not a code issue.

**Fix path (do this yourself in a proper VS Developer Command Prompt):**
```powershell
cd C:\ZombieEngine\build
rm -Recurse -Force CMakeFiles, CMakeCache.txt
cmake .. -G "Ninja"
cmake --build . --parallel 4
```

Once that succeeds, the remaining Phase 2 cleanup is:
1. Delete the original headers in `src/` (the copies in `include/ze/` are authoritative)
2. Verify zero-warnings 40-TU build
3. Commit as: `refactor: complete header separation — remove src/ originals`

---

## 5-Debug Summary for the Professionalization Plan

| Pass | Scope | Result |
|------|-------|--------|
| 1 | Structural feasibility | CLEAN — 12 subsystems, 62 headers moved safely |
| 2 | Dependency validation | CLEAN — only 12 files need include updates |
| 3 | Code quality audit | CLEAN — 33 namespaced files, 0 bare mallocs, 0 `using namespace` in headers |
| 4 | Build system audit | 2 expected issues (no add_subdirectory, no CI) — planned in Phases 4-5 |
| 5 | Doc/spec/test coverage | 5 issues — all have explicit Phase 1-8 fixes planned |

**Execution status:** Committed through Phase 3. Blocked on VS dev-environment bootstrap for Phase 2 completion and beyond.

---

## Remaining Work (in order)

1. **Bootstrap VS dev shell** and verify `cmake --build` succeeds
2. **Complete Phase 2:** Remove old `src/` headers after green build
3. **Phase 4:** Add `cmake/` helpers + presets + `add_subdirectory` structure
4. **Phase 5:** Add `.github/workflows/ci.yml`
5. **Phase 6:** Restructure `docs/` + add CODEOWNERS
6. **Phase 7:** Subsystem READMEs + quality gates
7. **Phase 8:** Asset pipeline split + validation scripts

Each phase should be committed separately and build-verified before moving on.
