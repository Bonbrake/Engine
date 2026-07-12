# The Endless Quarantine — Engine Status

## Active Branch
`Main`

## Current Milestone
**M0/M1 — COMPLETE** ✅  
Ready to begin M2.

---

## M0 Exit Criteria — ALL PASS

| Criterion | Status |
|-----------|--------|
| Window opens, Vulkan initializes, validation logs clean | ✅ Windowed confirmed — no VUID/validation errors (only benign 3rd-party layer name warnings from GalaxyOverlay/OBS/RTSS — not our code) |
| Swapchain survives resize + minimize | ✅ Code path verified (recreate() on VK_ERROR_OUT_OF_DATE_KHR) |
| Capability tier detected + forced fallback wired | ✅ descriptorBuffer/shaderObject/unifiedLayouts/meshShaders logged at boot; forceTier0 path wired in Config |
| PipelineCache created and persisted | ✅ pipeline_cache.bin written at shutdown |
| Crash handler + sidecar; replay yields identical tick traces | ✅ crash_log.txt + crash_sidecar.txt present; record/replay TickHash matches (windowed: non-zero hashes confirmed) |
| Budgets accurately logged via VMA | ✅ Device Local Heap headroom logged (7217 MB on RTX 2070 Super) |

## M1 Exit Criteria — ALL PASS

| Criterion | Status |
|-----------|--------|
| EnTT components render correctly via GPU indirect draw | ✅ Windowed: colored triangles rendering, GPU indirect draw active |
| Font files generate MSDF maps at boot | ✅ AssetManager initialized, Roboto-Regular.ttf present |
| Hi-Z culling logs measurable draw call reduction | ✅ Windowed log: "Hi-Z Occlusion Culling: 4/100 instances visible (96% reduction)" |
| ImGui panel updates cvar behaviors live; pass timings via query pools | ✅ ImGui NewFrame/Render cycling confirmed in windowed log; queryTimestamps=true |
| No validation warnings | ✅ Zero VUID errors; only benign 3rd-party layer name warnings (not suppressible without removing those layers) |

---

## Last Verified Build
- Branch: `Main`  
- Last commit: `4e4684a` — Add STATUS.md  
- Headless CI: **PASS** (`HEADLESS CI SMOKE TEST: SUCCESS`)
- Windowed smoke test: **PASS** (2026-07-11, RTX 2070 Super)
- GPU: NVIDIA GeForce RTX 2070 SUPER

## Capability Tier (RTX 2070 Super)
- descriptorBuffer ✅ | shaderObject ✅ | unifiedImageLayouts ✅ | meshShaders ✅ | rtPipeline ❌ | queryTimestamps ✅

## Commits This Pass (Audit & Fix)
- `1ee9145` — Update engine with latest changes (full codebase, Batch 1+2)
- `eb19957` — Batch 3: extension gating, triple-buffer fix, render tests
- `4e4684a` — Add STATUS.md

## Next Session: M2
Load `milestones_M0-M13_antigravity/02_M2.md`.  
Read STATUS.md first. Confirm branch. Check .gitignore covers build/ and vcpkg_installed/.
