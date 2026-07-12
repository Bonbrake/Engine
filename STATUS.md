# The Endless Quarantine — Engine Status

## Active Branch
`Main`

## Current Milestone
**M0/M1 Audit & Fix Pass** — COMPLETE

## Last Session Summary
Full codebase audit (40 bugs identified) + 3-batch fix rollout. All three
batches built clean and passed headless verification on RTX 2070 Super.

## Verification Results (last run: 2026-07-11)
- `--headless` auto-exit: **PASS** (`HEADLESS CI SMOKE TEST: SUCCESS`)
- Core tests: **PASS** (`DIAGNOSTIC SUCCESS: Core systems verified.`)
- ECS tests: **PASS** (`DIAGNOSTIC SUCCESS: ECS and cache safeguards verified.`)
- Render tests: **PASS** (`MAX_FRAMES_IN_FLIGHT == 3`, triple-buffer index check)
- Record/replay determinism (10 frames): **PASS** (TickHash matches)
- Capability tier (RTX 2070 Super): descriptorBuffer=true shaderObject=true
  unifiedImageLayouts=true meshShaders=true rtPipeline=false queryTimestamps=true

## Commits This Pass
- `1ee9145` — Update engine with latest changes (Batch 1 + 2 + full codebase)
- `eb19957` — Batch 3 (Render Systems): extension gating, triple-buffer fix, render tests

## Bugs Fixed (from 40-item audit)
### Critical
- [x] #2  TriangleRenderer triple-buffer race (indirectBuffer/countBuffer/countReadbackBuffer → arrays[3])
- [x] #7  Material system startup saturation (DestroyMaterial + cleanup after overflow test)
- [x] #8  Push constant merge offset bug (cache oldOffset before mutating range.offset)
- [x] #15 Descriptor buffer requested unconditionally → gated via enable_extension_if_present + forceTier0

### Medium
- [x] #1  VK_EXT_shader_object unconditional → conditionally enabled + feature struct gated
- [x] #2  GLTF non-indexed geometry crash (indicesAccessor.has_value() guard + sequential fallback)
- [x] #3  VK_KHR_UNIFIED_IMAGE_LAYOUTS unconditional → conditionally enabled + feature struct gated

### Low / Other (Batches 1–2)
- [x] FileHandleRing modulo bug, ThreadAffinity clamp, EnkiTSPinning alignment,
      Input replay limits, ECS division-by-zero, EnTTCache bounds/lifetime,
      SpatialHash Euclidean mod + re-bucketing, GenerationalTable slot clear,
      EntityFactory fallback, ImGuiOverlay pool leak, MetaRegistry array checks,
      JobSystem thread pinning for replay, headless auto-exit gating,
      VK_ERROR_DEVICE_LOST exit codes, main.cpp null-safe logger
- [x] Low #9  Profiler timing flicker (only clear lastFrameTimings_ on VK_SUCCESS)
- [x] Low #10 Unused static sampler s_AtlasSampler in MSDFPipeline
- [x] Low #13 VMA buffer allocation leak on index buffer failure in AssetManager

## Known Remaining / Next Steps
- Manual windowed smoke test + RenderDoc capture (requires display)
- M2: Physics (Jolt 5.5.0), JobSystem integration
- Spike A (determinism) — tick hashes are 0; need actual state XOR to confirm
- STATUS.md exists at repo root from this session onward
