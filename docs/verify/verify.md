# Verification

## Build verification

```powershell
cd C:\ZombieEngine
.\scripts\build_ze.cmd
```

Require:
- Zero compiler warnings
- `ZombieEngine.exe` linked
- `tests/ZombieEngineTests.exe` linked

## Test verification

```powershell
cd C:\ZombieEngine\build
./tests/ZombieEngineTests.exe
```

Target: `20 passed | 1 skipped | 83/83 assertions`.

## Headless smoke

```powershell
cd C:\ZombieEngine
./build/ZombieEngine.exe --headless --quit-frame 1
```

Expect Vulkan device init without window creation.
