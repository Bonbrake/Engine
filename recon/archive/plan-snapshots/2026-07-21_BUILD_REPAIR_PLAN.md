# Build Repair Plan — ZombieEngine
**Date:** 2026-07-21  
**Constraint:** No changes outside `C:\ZombieEngine`. No installers, no system changes.  
**Goal:** Make `cmake --build . --parallel 4` succeed from the existing toolchain.

---

## Diagnosis (from actual tool output)
- `cl.exe` exists: `C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`
- `rc.exe` / `mt.exe` exist: `C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/`
- `kernel32.lib` exists: `C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64/kernel32.Lib`
- SDL3 is installed in vcpkg: `C:/ZombieEngine/build/vcpkg_installed/x64-windows/share/sdl3/SDL3Config.cmake`
- vcpkg toolchain exists: `C:/Program Files/Microsoft Visual Studio/2022/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake`
- CMake can find the compiler after `vcvars64.bat`, but then `find_package(SDL3)` fails because CMake isn’t loading the vcpkg toolchain

**Root cause:** The earlier `configure_and_build.bat` was mangled by shell quoting (`cmd.exe /c /c/...`), and the vcpkg toolchain file wasn’t being passed through. The missing `SDL3_DIR` is the actual blocker.

---

## Plan

### Step 1 — Write a clean, minimal build script
Create `C:\ZombieEngine\build\build_ze.bat` that:
- Calls `vcvars64.bat` directly
- Deletes `CMakeCache.txt` + `CMakeFiles/` to reset state
- Runs `cmake .. -G "Ninja"` with:
  - Explicit compiler paths
  - Explicit `CMAKE_TOOLCHAIN_FILE` pointing to the vcpkg cmake wrapper
  - Explicit `SDL3_DIR` pointing to the installed SDL3 cmake config
  - Explicit `CMAKE_RC_COMPILER` and `CMAKE_MT`
- Runs `cmake --build . --parallel 4`

**Blast radius:** Only `C:\ZombieEngine\build\` contents modified. No registry, no system PATH, no installers.

### Step 2 — Execute the batch from `cmd.exe /c` with the ABSOLUTE path
Avoid any relative-path or quoting issues by calling the batch file by full path.

### Step 3 — Inspect output
If configure succeeds but build fails, I’ll read the actual compiler error and fix it.
If configure fails, I’ll read the CMake error and adjust.
If it succeeds, I’ll confirm `ZombieEngine.exe` exists in `build/`.

### Step 4 — Only if build succeeds: complete header cleanup
- Delete old headers from `src/` (the `include/ze/` copies are already in place)
- Update any remaining old-style includes
- Rebuild to confirm zero warnings
- Commit

### Step 5 — Push + hand back control
Tag you in when the build is green.

---

## Rollback safety
- The batch file only touches `C:\ZombieEngine\build\`
- Git already has all source changes committed (commit `afe805e`)
- If anything goes wrong, delete `build/` and re-run cmake from scratch — original source is untouched

---

## What I will NOT do
- Run any installer or modify Visual Studio
- Touch anything outside `C:\ZombieEngine`
- Delete anything without first confirming it’s in `build/` or `src/` old headers that have `include/ze/` copies
- Claim success without showing real build output
