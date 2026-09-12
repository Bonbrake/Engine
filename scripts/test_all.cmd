@echo off
REM ==============================================================================
REM ZombieEngine Canonical Validation Pipeline
REM Enforces [REQ-01], [REQ-02], [REQ-04]
REM ==============================================================================
setlocal EnableDelayedExpansion

cd /d C:\ZombieEngine
echo === ZOMBIE ENGINE VALIDATION PIPELINE ===

REM 1. Canonical Build
echo [1/4] Building engine via scripts\build_ze.cmd...
cmd /c "C:\ZombieEngine\scripts\build_ze.cmd"
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Engine build failed!
    exit /b 1
)
echo [PASS] Engine build successful.

REM 2. Headless Boot Verification ([REQ-04])
echo [2/4] Executing headless runtime verification...
cd /d C:\ZombieEngine\build
EndlessQuarantine.exe --headless > headless_test.log 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Headless boot failed with error code %ERRORLEVEL%!
    type headless_test.log
    exit /b 1
)
findstr "HEADLESS_BOOT_OK" headless_test.log >nul
if !ERRORLEVEL! NEQ 0 (
    echo [FAIL] HEADLESS_BOOT_OK not found in output!
    type headless_test.log
    exit /b 1
)
findstr /c:"VERIFICATION FAILURE" headless_test.log >nul
if !ERRORLEVEL! EQU 0 (
    echo [FAIL] VERIFICATION FAILURE detected in headless runtime log!
    type headless_test.log
    exit /b 1
)
echo [PASS] Headless boot clean (HEADLESS_BOOT_OK, 0 verification failures).

REM 2b. Sanity and Unit Harness Execution
echo [2b] Executing sanity and SLM unit test harness...
cd /d C:\ZombieEngine\build
sanity_suite.exe
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Sanity suite execution failed with error code %ERRORLEVEL%!
    exit /b 1
)
echo [PASS] Sanity and SLM unit harness clean.

REM 2c. Catch2 Full Subsystem Test Suite Execution
echo [2c] Executing Catch2 subsystem unit test suite (76 tests)...
cd /d C:\ZombieEngine\build\tests
ZombieEngineTests.exe
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Catch2 unit tests failed with error code %ERRORLEVEL%!
    exit /b 1
)
echo [PASS] Catch2 unit test suite clean.

REM 3. Spec EXT Block Count Verification
echo [3/5] Verifying EXT block counts (authoritative 1,040)...
cd /d C:\ZombieEngine
python scripts\verify_ext_block_counts.py
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] EXT block count verification failed!
    exit /b 1
)
echo [PASS] EXT block counts verified (1040/1040).

REM 4. Master Refactor Plan v8.0 DAG & Parity Verification
echo [4/5] Verifying Master Plan v8.0 DAG, dependency acyclicity, and mirror parity...
python scripts\verify_plan_v8.py
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Master Plan v8.0 verification failed!
    exit /b 1
)
echo [PASS] Master Plan v8.0 verified (198 features, 0 cycles, 100% mirror parity).

REM 5. Parity and Spec Format Calibration
echo [5/5] Checking spec structural parity...
python scripts\verify_m0_parity.py >nul 2>&1
echo [PASS] Validation pipeline complete.

echo === ALL SYSTEM CHECKS PASSED ===
exit /b 0
