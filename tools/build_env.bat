@echo off
REM ZombieEngine build environment launcher.
REM Usage: build_env.bat <command>   (command runs with MSVC + WinSDK on PATH)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
%*
