# Build Instructions

## Prerequisites

- Windows 10/11 64-bit
- Visual Studio 2022 Community (MSVC v14.44)
- CMake 3.20+
- Ninja
- vcpkg with SDL3/Jolt/volk/vk-bootstrap/glm/EnTT/spdlog/enkiTS/cxxopts/msdfgen/fastgltf/spirv-reflect/freetype/imgui/catch2
- Vulkan SDK 1.4.x

## Canonical build

```powershell
cd C:\ZombieEngine\build
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --parallel 4
```

Artifacts:
- `build/ZombieEngine.exe`
- `build/tests/ZombieEngineTests.exe`

## Presets

```powershell
# Debug
cmake --preset windows-debug
cmake --build build --parallel 4

# ASan
cmake --preset windows-asan
cmake --build build-asan --parallel 4
```
