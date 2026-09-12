@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d C:\ZombieEngine
set VULKAN_SDK=C:\VulkanSDK\1.4.350.0
set VCPKG_INSTALLED_DIR=C:\ZombieEngine\vcpkg_installed
set VCPKG_PREFIX=%VCPKG_INSTALLED_DIR%\x64-windows
echo PREFIX=%VCPKG_PREFIX%
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH=%VCPKG_PREFIX% ^
  -DSDL3_DIR=%VCPKG_PREFIX%/share/sdl3 ^
  -DVulkanMemoryAllocator_DIR=%VCPKG_PREFIX%/share/VulkanMemoryAllocator ^
  -DEnkiTS_DIR=%VCPKG_PREFIX%/share/enkiTS ^
  -Dspdlog_DIR=%VCPKG_PREFIX%/share/spdlog ^
  -Dfmt_DIR=%VCPKG_PREFIX%/share/fmt ^
  -Dnlohmann_json_DIR=%VCPKG_PREFIX%/share/nlohmann_json ^
  > tools\cfg.log 2>&1
echo CFG_EXIT=%ERRORLEVEL% >> tools\cfg.log
cmake --build build --config Release >> tools\build.log 2>&1
echo BUILD_EXIT=%ERRORLEVEL% >> tools\build.log
