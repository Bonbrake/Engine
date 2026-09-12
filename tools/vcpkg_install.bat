@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d C:\Users\jakeb\ZombieEngine
set VULKAN_SDK=C:\VulkanSDK\1.4.350.0
vcpkg install --triplet x64-windows > tools\vcpkg_install.log 2>&1
echo DONE_EXIT=%ERRORLEVEL% >> tools\vcpkg_install.log
