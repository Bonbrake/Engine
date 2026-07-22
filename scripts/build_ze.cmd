@echo off
setlocal ENABLEDELAYEDEXPANSION

set "SOURCE_DIR=C:\ZombieEngine"
set "BUILD_DIR=%~1"
if "%BUILD_DIR%"=="" set "BUILD_DIR=C:\ZombieEngine\build"

cd /d "%BUILD_DIR%"
set "OLD_PATH=%PATH%"
set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\x64;C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64;%OLD_PATH%"
set "INCLUDE=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\ATLMFC\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared"
set "LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\x64"
set "LIBPATH=%LIB%"

if exist CMakeCache.txt del /f CMakeCache.txt
if exist CMakeFiles rd /s /q CMakeFiles

echo === CONFIGURE ===
cmake "%SOURCE_DIR%" -G "Ninja" ^
  -DCMAKE_TOOLCHAIN_FILE="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg\scripts\buildsystems\vcpkg.cmake" ^
  -DCMAKE_CXX_COMPILER=cl.exe ^
  -DCMAKE_C_COMPILER=cl.exe ^
  -DCMAKE_RC_COMPILER=rc.exe ^
  -DCMAKE_MT=mt.exe ^
  -DSDL3_DIR="C:\ZombieEngine\build\vcpkg_installed\x64-windows\share\sdl3"

echo === BUILD ===
cmake --build . --parallel 4
endlocal
