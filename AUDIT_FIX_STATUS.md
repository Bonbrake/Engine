# Audit Fix Status — M0–M2

## VERIFICATION: COMPLETE (real build + test + run evidence)

Build performed: TEMPORARY MSVC/v143 build (see caveat below). vcpkg binary cache
now populated at `C:\vcpkg\archives` (root-cause fix for repeated full Jolt rebuilds).

### Build evidence (real, not assumed)
- `VCPKG_EXIT=0` — 17 deps + Jolt 5.5.0 restored from/build into cache.
- Configure (native WIN toolchain path `C:/vcpkg/...`) + build = `BUILD_EXIT=0`.
- Binaries on disk: `build/Debug/ZombieEngine.exe`, `build/Debug/ZombieEngineTests.exe`.
- Zero compiler/linker errors.

### Test evidence (real)
```
ZombieEngineTests.exe -> "All tests passed (35 assertions in 7 test cases)"  exit 0
  - CommandPoolMatrix Index Mapping        [M1-EXT-03]  (index-math asserts) PASS
  - CommandPoolMatrix Initialize rejects null device [M1-EXT-03] (new throw test) PASS (1 assertion)
```

### Finding #2 re-run (observe-only, operands untouched)
Windowed `--dev --quit-frame 180`, cwd=project root (shaders resolve from build/shaders/).
Exit 0. Corrected log fires twice, reports:
```
Pipeline layout compatibility check: graphics vs compute set 0 DIFFERS.
Note: graphics and compute-cull are distinct shader stages with legitimately
different layouts — this needs diagnosis before treating it as a defect.
```
=> Original Finding #2 signal is REAL (not a log artifact). The old code lied with
always-"VERIFICATION SUCCESS"; the new log correctly reports the difference.
=> Operands (graphicsSetLayouts vs computeSetLayouts) NOT diagnosed, per standing
   instruction. Defect-vs-legitimate is still an open question.

## Three fixes applied (all verified by the run above)
1. `src/render/CommandPoolMatrix.h` — `ConfigureLayout(numFrames,numThreads)` seam
   (sets threadCount + pools.resize); `Initialize` now throws `std::invalid_argument`
   on `VK_NULL_HANDLE` before pool-creation loop; removed dead in-loop null guard;
   added `#include <stdexcept>`.
2. `src/tests/Test_MathFormulas.cpp` — index-math test uses `ConfigureLayout(3,4)`
   retaining all `GetPoolIndex` asserts; new test asserts `Initialize(VK_NULL_HANDLE,…)`
   throws `std::invalid_argument`.
3. `src/render/TriangleRenderer.cpp:391` — replaced always-"VERIFICATION SUCCESS" log
   with result-gated `LOG_INFO`(PASS)/`LOG_WARN`(DIFFERS).

## CAVEAT — TEMPORARY BUILD, NOT BASELINE
- This verification used `-T v143` (plain MSVC), NOT the repo's recorded ClangCL toolset.
- ClangCL install is BLOCKED: OS build 10.0.26220 is outside VS2022's supported range
  (confirmed via VS setup log: payloads reject this OS). `vs_installer.exe modify` exits
  87 silent; `winget --override` no-ops. Tried both, both failed on env, not perms/network.
- Therefore the SANITIZER PATH (ASan/UBSan on Windows, x64-windows-asan triplet) is
  UNVERIFIED — ClangCL is the toolchain that path requires. This temp MSVC build must
  NOT become the new baseline; ClangCL install remains a pending follow-up.
- This caveat is logged so the workaround is never silently promoted to standard.

## Follow-ups still open
- [ ] Install ClangCL (or otherwise get a sanitizer-capable toolchain) to verify Section
      3/4 sanitizer matrix. Blocked by OS-vs-VS2022 support range; needs user decision
      (e.g. VS upgrade, or accept MSVC-only local verification).
- [ ] Diagnose Finding #2 operands (defect vs legitimate) — user-deferred, not started.
- [ ] After fixes verified + audit clean: commit 3 source fixes + docs on
      `m2/physics-destruction`, update PR #11. M2.6 still FORBIDDEN until then.
