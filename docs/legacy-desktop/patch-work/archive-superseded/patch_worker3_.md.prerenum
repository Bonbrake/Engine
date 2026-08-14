PROVISIONAL PATCH — NOT YET VERIFIED
=====================================
IDs below (M0-EXT-21 through M0-EXT-27) have NOT been swept against the
Desktop draft files. Re-check live IDs in the actual v78 file before
merging; renumber if anything collides.
=====================================

## [M0-EXT-21] Persistent World-Epoch Clock with Deterministic Offline Fast-Forward

##### Systems Touched
M7 (persistence) and every future decay/weather system — declared at
the timekeeping layer now so offline elapsed-time simulation doesn't get
special-cased into the save system later.

##### Math
`ElapsedSimSeconds_onLoad = (WallClockNow − WallClockAtLastSave) × TimeScale`

##### How It Works
Stores a `WorldEpoch` (fixed in-game start timestamp) plus accumulated
simulated-seconds. On load, computes real-world elapsed time since the
last save and deterministically fast-forwards weather/decay/etc. by that
amount, rather than that logic living downstream in M7.

##### Reference Implementation
```cpp
struct WorldClock { uint64_t epochUnixSeconds; double accumulatedSimSeconds; };

double ComputeOfflineFastForward(uint64_t wallClockAtSave, uint64_t wallClockNow, double timeScale) {
    return static_cast<double>(wallClockNow - wallClockAtSave) * timeScale;
}
```

##### Player-Facing Impact
The world keeps existing while the game isn't running — weather
progresses, food decays — the Zomboid-style "it's a real place" feel.

---

## [M0-EXT-22] Monotonic-vs-Wall-Clock Tamper Cross-Check

##### Systems Touched
Extends [M0-EXT-21] — prevents trivially defeating offline decay/weather
by winding the system clock back.

##### How It Works
At boot, compares the OS monotonic uptime delta against the wall-clock
delta since last recorded session; a mismatch beyond a small tolerance
flags the wall clock as untrusted for that session's offline
fast-forward calculation.

##### Reference Implementation
```cpp
bool IsWallClockTrusted(uint64_t monotonicDeltaMs, uint64_t wallClockDeltaMs, uint64_t toleranceMs) {
    uint64_t diff = (monotonicDeltaMs > wallClockDeltaMs)
        ? (monotonicDeltaMs - wallClockDeltaMs) : (wallClockDeltaMs - monotonicDeltaMs);
    return diff <= toleranceMs;
}
```

##### Player-Facing Impact
Closes an easy exploit for dodging decay/weather consequences.

---

## [M0-EXT-23] Extended Adaptive-Quality Watchdog

##### Systems Touched
Builds directly on the existing frame-pacing EMA (M0-EXT-09) — adds a
second, slower-moving average and an ordered fallback ladder instead of
a single hard-coded quality drop.

##### Math
`Δt_avgLong = (1−λ_long)·Δt_avgLong + λ_long·Δt_current`, `λ_long = 0.02`
(slower time constant than M0-EXT-09's `λ = 0.10`, so this reacts to
*sustained* overbudget frames, not single hitches).

##### How It Works
When `Δt_avgLong` crosses a sustained-overbudget threshold, triggers a
described, ordered quality-reduction ladder (shadow resolution →
particle density → draw distance) instead of the current implicit
"nothing happens" or a single hard fallback.

##### Reference Implementation
```cpp
enum class QualityTier { Full, ReducedShadows, ReducedParticles, ReducedDrawDistance };

QualityTier EvaluateQualityLadder(float smoothedLongDelta, float targetStep, QualityTier current) {
    if (smoothedLongDelta > targetStep * 1.5f) {
        return static_cast<QualityTier>(std::min(static_cast<int>(current) + 1,
                                                  static_cast<int>(QualityTier::ReducedDrawDistance)));
    }
    return current;
}
```

##### Player-Facing Impact
A horde crush or physics spike degrades gracefully in a defined order
instead of an unpredictable frame-rate collapse at the worst moment.

---

## [M0-EXT-24] Single-Instance Boot Lock

##### Systems Touched
Boot sequence — prevents two copies of the game writing to the same
save concurrently.

##### How It Works
A named OS mutex (or lock file) is checked and held at boot; a second
instance detects it and refuses to start (or warns and exits).

##### Reference Implementation
```cpp
bool AcquireSingleInstanceLock(const std::string& lockName) {
#if defined(_WIN32)
    HANDLE h = CreateMutexA(nullptr, TRUE, lockName.c_str());
    return h != nullptr && GetLastError() != ERROR_ALREADY_EXISTS;
#else
    int fd = open(("/tmp/" + lockName).c_str(), O_CREAT | O_EXCL, 0644);
    return fd != -1;
#endif
}
```

##### Player-Facing Impact
Prevents a corrupted save from two instances writing at once.

---

## [M0-EXT-25] Per-Fiber Exception Containment for enkiTS Jobs

##### Systems Touched
Every background job dispatched through enkiTS — a bad chunk-gen task
or corrupt-asset job shouldn't take the whole process down.

##### How It Works
Wraps the top of every enkiTS job dispatch in a try/catch (or SEH filter
on Windows), logging and safely dropping the failed job instead of
propagating into the scheduler.

##### Reference Implementation
```cpp
void SafeJobDispatch(std::function<void()> job) {
    try {
        job();
    } catch (const std::exception& e) {
        Spdlog::Error("Job threw: {}", e.what()); // one dropped job, not a crashed process
    }
}
```

##### Player-Facing Impact
One malformed background task logs an error instead of crashing the
game mid-session.

---

## [M0-EXT-26] Shader-Compile Timeout & Safe-Mode Boot Fallback

##### Systems Touched
M1's async shader compilation path, and M0's pipeline-cache/mod
integrity validation (M0-EXT-03).

##### How It Works
The async shader-compile path gets a watchdog timer; a hung compile logs
and falls back to a cached/default shader instead of hanging boot
indefinitely. Separately, if pipeline-cache validation or a mod fails
integrity checks, the game boots in safe mode (mods disabled, cache
rebuilt) instead of the current implicit "just don't boot."

##### Reference Implementation
```cpp
bool CompileShaderWithTimeout(ShaderCompileJob& job, std::chrono::milliseconds timeout) {
    auto future = std::async(std::launch::async, [&]{ return job.Compile(); });
    if (future.wait_for(timeout) == std::future_status::timeout) {
        Spdlog::Error("Shader compile timed out, falling back to cached shader.");
        return false;
    }
    return future.get();
}
```

##### Player-Facing Impact
A hung shader compile or corrupted mod/cache never fully blocks boot —
the game degrades to safe mode instead of not starting at all.

---

## [M0-EXT-27] Foreground/Background I/O and Thread Priority Tiers

##### Systems Touched
Extends the existing file-handle ring buffer (M0-EXT-04) and enkiTS
worker pool with a simple priority split.

##### How It Works
Requests and jobs are tagged `Foreground` (render/input-critical) or
`Background` (streaming/AI/autosave); foreground work is serviced first
under contention, preventing a heavy streaming burst from starving a
frame-critical job.

##### Reference Implementation
```cpp
enum class JobPriority { Foreground, Background };
// Scheduler services Foreground-tagged work before Background-tagged work
// when both are ready, using existing enkiTS priority/affinity hooks.
```

##### Player-Facing Impact
Autosaves and streaming bursts no longer cause a frame hitch during
gameplay-critical moments.
