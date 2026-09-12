@echo off
cd /d C:\Users\jakeb\ZombieEngine
rmdir /s /q build
if exist build (echo BUILD_STILL_EXISTS) else (echo BUILD_REMOVED)
