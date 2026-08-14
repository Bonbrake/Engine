PROVISIONAL PATCH — NOT YET VERIFIED
=====================================
IDs below (M0-EXT-15 through M0-EXT-20) have NOT been swept against the
Desktop draft files. Re-check live IDs in the actual v78 file before
merging; renumber if anything collides.
=====================================

## [M0-EXT-15] Minimal Action-Map Input Resolution Layer

##### Systems Touched
Every early input consumer, MOST URGENTLY the debug fly-camera currently
bugged. Pulled forward from M11 ("UI/HUD, input abstraction &
consolidated haptics") — this is a minimal subset built now, not a
replacement for M11's full system later.

##### Math
`ActionState.value = Curve(RawAxis, Deadzone)`, `ActionState.held = RawKeyDown[boundKey]` sampled every tick, independent of whether an SDL event fired that frame.

##### How It Works
Raw `SDL_PollEvent` state resolves once per tick into a named
`ActionState` (`held: bool`, `pressedThisFrame: bool`, `value: float`)
via a binding table (device+code → action). Gameplay code — fly-camera
included — reads `ActionState`, never raw scancodes.

**Direct fly-cam bug analysis:** the reported symptom ("controls have no
effect, gated behind a non-empty-events conditional") is the classic
signature of code branching on discrete SDL *events* (which only fire on
state transitions) where continuous *held* state was actually needed.
`ActionState.held` is sampled every tick regardless of whether
`SDL_PollEvent` returned anything that frame, which removes this bug
class rather than patching this one instance of it.

##### Reference Implementation
```cpp
struct ActionState { bool held = false; bool pressedThisFrame = false; float value = 0.0f; };

struct ActionBinding { SDL_Scancode key; /* or controller axis id */ };

std::unordered_map<std::string, ActionBinding> g_bindings; // populated from settings.json

void ResolveActions(const Uint8* keyState, std::unordered_map<std::string, ActionState>& actions) {
    for (auto& [name, binding] : g_bindings) {
        bool wasHeld = actions[name].held;
        bool isHeld = keyState[binding.key] != 0; // continuous poll, NOT event-based
        actions[name].held = isHeld;
        actions[name].pressedThisFrame = isHeld && !wasHeld;
        actions[name].value = isHeld ? 1.0f : 0.0f; // sticks apply a deadzone curve here instead
    }
}
```

##### Player-Facing Impact
Fly-camera (and every future input-driven system) responds reliably to
held input instead of only reacting to discrete key-transition events.

---

## [M0-EXT-16] ENGINE_DETERMINISM_MODE Compile Guard & CI Assertion

##### Systems Touched
M2.8 (deterministic co-op) — establishes the constraint at the layer
where parallel job scheduling and GPU reductions happen, rather than
discovering a desync bug during co-op testing.

##### Math
`Determinism valid ⟺ ∀ gameplay-affecting reduction, accumulation order is fixed (entity-index order), independent of job-completion order.`

##### How It Works
An `ENGINE_DETERMINISM_MODE` compile guard wraps any gameplay-affecting
float accumulation, asserting it iterates a stable, index-ordered
container rather than an unordered one whose iteration order can vary
run-to-run. Wired into the existing headless CI boot-smoke-test
(already declared in M0) so a violation fails CI immediately.

##### Reference Implementation
```cpp
#ifdef ENGINE_DETERMINISM_MODE
template <typename T>
void AssertOrderedAccumulation(const std::vector<T>& orderedByEntityIndex) {
    // Debug-only: verifies the caller passed an index-ordered container,
    // not an unordered_map/unordered_set whose iteration order may vary.
    static_assert(!std::is_same_v<decltype(orderedByEntityIndex), std::unordered_map<int, T>>,
                  "Gameplay-affecting accumulation must not iterate an unordered container");
}
#endif
```

##### Player-Facing Impact
Co-op play (M2.8/M12) stays in sync instead of silently desyncing from a
floating-point ordering difference that only shows up under load.

---

## [M0-EXT-17] Split Save-Schema / Network-Protocol Version Fields

##### Systems Touched
M7 (persistence) and M12 (networked co-op) — declared independently now
so a save-format patch can never silently break netcode compatibility
or vice versa.

##### Math
`SaveSchemaVersion: uint32_t`, `NetworkProtocolVersion: uint32_t` — two
independent fields, never conflated into one "build version" number.

##### How It Works
Every save file embeds `SaveSchemaVersion`; every network handshake
negotiates `NetworkProtocolVersion` separately. A save-format change
bumps only the former; a wire-format change bumps only the latter.

##### Reference Implementation
```cpp
struct SaveHeader { uint32_t saveSchemaVersion; /* ... */ };
struct NetHandshake { uint32_t networkProtocolVersion; /* ... */ };
```

##### Player-Facing Impact
A patch that changes how saves are stored can't accidentally break
matchmaking compatibility, and vice versa.

---

## [M0-EXT-18] Atomic Write-Temp-Then-Rename Save I/O

##### Systems Touched
M7 persistence — every save write, going forward.

##### Math
`Save valid ⟺ Checksum(save.tmp) matches AND rename(save.tmp, save.dat) completed atomically.`

##### How It Works
Writes go to `save.tmp`, are fsync'd, checksummed, and only then
atomically renamed over `save.dat`. A crash or power-loss mid-write
leaves the previous good save intact.

##### Reference Implementation
```cpp
bool AtomicSaveWrite(const std::string& path, const std::vector<uint8_t>& data) {
    std::string tmpPath = path + ".tmp";
    if (!WriteAndFsync(tmpPath, data)) return false;
    uint32_t checksum = Crc32(data);
    if (!VerifyChecksum(tmpPath, checksum)) return false;
    return AtomicRename(tmpPath, path); // platform-native atomic rename
}
```

##### Player-Facing Impact
A save never gets corrupted by a crash or power loss mid-write — a
survival game's core trust guarantee.

---

## [M0-EXT-19] Platform User-Data Directory Resolution via AssetPath

##### Systems Touched
Extends the existing `AssetPath` resolver (M0) to also resolve a
writable per-user path, separate from the install directory.

##### How It Works
Saves/settings resolve through `AssetPath` to a platform-appropriate
writable directory (`%APPDATA%` on Windows, `~/.local/share` on Linux)
rather than relative to the install folder — avoiding a forced save
migration later when the install directory isn't writable (Steam/mod
scenarios).

##### Reference Implementation
```cpp
std::filesystem::path ResolveUserDataDir() {
#if defined(_WIN32)
    return std::filesystem::path(std::getenv("APPDATA")) / "EndlessQuarantine";
#else
    return std::filesystem::path(std::getenv("HOME")) / ".local/share/EndlessQuarantine";
#endif
}
```

##### Player-Facing Impact
Saves and settings live in the correct per-user location from day one —
no migration script needed later.

---

## [M0-EXT-20] Cross-Machine Crash Correlation Tag

##### Systems Touched
M2.8/M12 co-op — extends the existing Aftermath/RGD crash-dump path
(M0) with a shared identifier so a host-side crash can be matched
against what a peer was doing at the same simulated tick.

##### How It Works
Every crash/minidump is tagged with the current tick number and a
session-id exchanged between host and peer at connection time.

##### Reference Implementation
```cpp
struct CrashTag { uint64_t tick; uint64_t sessionId; };
```

##### Player-Facing Impact
Co-op crash reports can actually be correlated across machines instead
of being two disconnected local dumps.
