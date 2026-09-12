@echo off
setlocal ENABLEDELAYEDEXPANSION

set "SOURCE_DIR=C:\ZombieEngine"
set "BUILD_DIR=%~1"
if "%BUILD_DIR%"=="" set "BUILD_DIR=C:\ZombieEngine\build"

if not exist "%BUILD_DIR%" md "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

set "PATH=C:\Program Files\CMake\bin;%PATH%"

if exist CMakeCache.txt del /f CMakeCache.txt
if exist CMakeFiles rd /s /q CMakeFiles

echo === CONFIGURE ===
cmake "%SOURCE_DIR%" -G "Ninja" -DCMAKE_BUILD_TYPE=Release

echo === BUILD ===
cmake --build . --parallel 4
endlocal
