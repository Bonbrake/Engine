@echo off
del /f /q \\.\C:\Users\jakeb\ZombieEngine\nul
if exist \\.\C:\Users\jakeb\ZombieEngine\nul (echo NUL_STILL_THERE) else (echo NUL_REMOVED)
