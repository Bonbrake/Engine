# ZombieEngine — Comprehensive Execution Tasklist

## Priority Legend
**P0**: Blocker — must fix immediately
**P1**: Core — needed for next milestone
**P2**: Important — scheduled within 2 weeks
**P3**: Stretch — valuable but not blocking

---

## PHASE 1: FOUNDATION FIXES (P0)

### P0.1 Fix Catch2 Test Runtime (DONE)
- [x] Build Release binary with vcpkg DLLs staged
- [x] Verified: 29 test cases, 28 passed, 1 skipped, 110/110 assertions passed
- [x] Binary runs via PowerShell Start-Process with output redirection

### P0.2 Fix Swapchain Surface Format (DONE)
- [x] Reverted to `VK_FORMAT_R8G8B8A8_UNORM` with RTSS workaround comment
- [x] Build verified green

### P0.3 Fix Engine.h Include (DONE)
- [x] Replaced `../debug/ImGuiOverlay.h` with `ze/debug/ImGuiOverlay.h`

### P0.4 Fix Destructible.cpp Unsafe Registry Access (DONE)
- [x] Replaced `registry.get<>` with `registry.try_get<>` after `all_of` check
- [x] Balanced braces restored

### P0.5 Author M2.7, M5.4, M8.6 Specs (DONE)
- [x] M2.7: 34 EXT blocks, 3 clusters, 4 acceptance criteria
- [x] M5.4: 17 EXT blocks, 2 clusters, 6 acceptance criteria
- [x] M8.6: 28 EXT blocks, 3 clusters, 5 acceptance criteria

---

## PHASE 2: CORE IMPLEMENTATIONS (P1)

### P1.1 M6 Audio Runtime
- [ ] `src/audio/AudioEngine.cpp`: Miniaudio init, synthesis chain, voice director interface
- [ ] `src/audio/AudioPropagation.cpp`: Doppler SPL echo portal (M6-EXT-01..04)
- [ ] `src/audio/AudioVoice.cpp`: Priority cadence reverb (M6-EXT-05..07)

### P1.2 M5 AI Runtime
- [ ] `src/ai/AIDirector.cpp`: Director state machine, personality presets, event grammar
- [ ] `src/ai/ZombieFSM.cpp`: Zombie sensory/locomotion/attack FSM based on M2.7 spec
- [ ] `src/ai/ScentGrid.cpp`: Particle-based scent + noise propagation

### P1.3 M8 World Runtime
- [ ] `src/world/ChunkStreamer.cpp`: WFC, macro-graph, Voronoi seam welding, LOD transitions
- [ ] `src/world/BiomeGraph.cpp`: Whittaker classification, palette blend, hazard tables
- [ ] `src/vehicle/VehicleSystem.cpp`, `WheelSim.cpp`: Vehicle physics, gearbox, fuel

### P1.4 M4 Render Graph
- [ ] Implement 3-pass render graph
- [ ] Implement forward+ with chunk/cell culling
- [ ] Implement shadow atlas + two-tier cache
- [ ] Implement HDR + AgX + visibility buffer (M4.5)

### P1.5 Convert 30 [X] Papers to [S]
- [ ] Convert Papers 1-5 (user's `papers_1_10.txt` top 5)
- [ ] Convert Papers 6-10
- [ ] Convert 20 more from backlog

---

## PHASE 3: SURFACE IMPLEMENTATIONS (P2)

### P2.1 Survival Runtime
- [ ] `BodyTemp.cpp`: Temperature/humidity grid, wetness, wind chill
- [ ] `StaminaSystem.cpp`: Exertion, fatigue, recovery curves (M0-M2)

### P2.2 UI Runtime
- [ ] `HUD.cpp`: Minimal HUD with MSDF glyphs
- [ ] `HapticManager.cpp`: Haptic feedback from damage/events
- [ ] `MSDFText.cpp`: Signed-distance-field text rendering

### P2.3 Save Runtime
- [ ] `SaveSystem.cpp`: Deterministic snapshot → file I/O
- [ ] `SchemaVersion.cpp`: Binary compat check

### P2.4 Combat Runtime
- [ ] `DamageTypes.cpp`: Damage type enum + material response table
- [ ] `ParrySystem.cpp`: Parry frame window, directional guard

### P2.5 EventBus Runtime
- [ ] `EventBus.cpp`: Hot event bus for cross-system communication

---

## PHASE 4: ADVANCED SYSTEMS (P2)

### P2.6 Net Runtime
- [ ] `NetworkManager.cpp`: Reliable UDP, client/server state
- [ ] `StateSync.cpp`: Input+RNG deterministic replay

### P2.7 SLM Runtime
- [ ] `PromptTemplate.cpp`: LLM template filling for narrative
- [ ] `SLMClient.cpp`: LLM client (local/remote)

### P2.8 Modding Runtime
- [ ] `Modding.cpp`: Mod load, version check, hook system

---

## PHASE 5: EXECUTION & AUDIT (P3)

### P3.1 Implement Trendsetters
- [ ] Motion matching (M2.7-EXT-21)
- [ ] GPU-driven visibility (M1-EXT-27, M4.5-EXT-12)
- [ ] Compute skinning (M1-EXT-27)
- [ ] Voronoi fracturing (M2.6-M2.9)
- [ ] Deterministic lockstep replay (M2.8-EXT-09)
- [ ] Procedural audio synthesis (M6-EXT-15, M6-EXT-18)
- [ ] Procedural quest grammar (M11, M5.4)
- [ ] Scent/noise propagation (M6, M5)
- [ ] Settlement economy (M8, M6, M9)
- [ ] Wealth threat scaling (M5.4)
- [ ] Structural integrity (M2.6-M2.9, M4)
- [ ] Faction reputation (M7, M11)
- [ ] Ghost replay (M2.8-EXT-09)
- [ ] Wind + foliage compute (M8, M4)
- [ ] Biome transitions (M8.6, M4)
- [ ] Day/night escalation (M3, M5, M6)

### P3.2 Hermes Automation
- [ ] `ze-procedural-audit` skill: math/formula verification
- [ ] `ze-minimal-asset-audit` skill: asset count trends
- [ ] `ze-voice-direction-audit` skill: intent/emotion coverage
- [ ] `ze-spec-audit` skill: parity, shadow milestones, stub markers
- [ ] Daily verifier cronjob
- [ ] Weekly bug-hunt cronjob
- [ ] Weekly asset inventory cronjob

### P3.3 Verification Pass
- [ ] Verify all 813 EXT blocks have matching JSON indexes
- [ ] Verify 0 unsafe `registry.get<>` in production code
- [ ] Verify all headers have include guards
- [ ] Verify all `.cpp` have CMake targets
- [ ] Verify authored asset budget ≤50
- [ ] Verify voice intent coverage ≥90%
- [ ] Run full Catch2 suite

---

## Current Status (2026-07-22)
**Blockers:** Catch2 test runtime (Debug DLL path), some stub impls
**In progress:** Release binary works silently, plan v7 written, tasklist created
**Ready:** Next task: M6 audio engine runtime implementation
