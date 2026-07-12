# M0–M2 Audit — findings log

Status: AUDIT COMPLETE. Fixes for #1 and #2 applied and verified on local RTX 2070 SUPER hardware
(see AUDIT_FIX_STATUS.md for build/test/run evidence). This file is the raw findings log.
Method: ran the real binaries on local RTX 2070 SUPER hardware; every API claim
fact-checked against authoritative sources before being marked CONFIRMED.

Runs performed:
- `ZombieEngineTests.exe` (Catch2 unit suite)
- `ZombieEngine.exe --headless --dev --quit-frame 60` (M0 boot-smoke / CI path)
- `ZombieEngine.exe --dev --quit-frame 180` (windowed: real swapchain/present + render self-tests)

---

## Finding #1 — CommandPoolMatrix::Initialize segfaults on null device  [CONFIRMED — real bug]

- **Where:** `src/render/CommandPoolMatrix.h:19-26` (`Initialize`); triggered by `src/tests/Test_MathFormulas.cpp:48`.
- **Symptom:** `ZombieEngineTests.exe` crashes SIGSEGV on the FIRST test (`[M1-EXT-03]`).
  Process death means any test after it in one run never executes.
- **Proof it's isolated:** running `ZombieEngineTests.exe "~[M1-EXT-03]"` (exclude that tag) =>
  **all other 5 test cases pass, 28 assertions.** This is the ONLY failing test.
- **Root cause:** `Initialize` unconditionally calls `vkCreateCommandPool(device, ...)` with
  `device == VK_NULL_HANDLE`. Test deliberately passes VK_NULL_HANDLE (line 47 comment:
  "We don't need to Initialize with a real device to test GetPoolIndex").
- **Fact-check (authoritative):** Vulkan-Loader issue #384 (Khronos) — loader deliberately does
  NOT null-check handles. Vulkan loader architecture docs — a dispatchable handle (VkDevice) is a
  pointer whose first member points to the dispatch table; the loader trampoline reads the dispatch
  table from that first member. Null VkDevice => null deref => SIGSEGV. Matches observed crash exactly.
- **Contract inconsistency:** sibling `StagingRingBuffer::Initialize(VK_NULL_HANDLE, 1024)` (same
  test file, line 97) IS null-tolerant. CommandPoolMatrix should honor the same contract.
- **Secondary nit:** `CommandPoolMatrix.h` throws `std::out_of_range` (line 41) without
  `#include <stdexcept>`; compiles only because the test happens to include it first.
- **Candidate fix (NOT APPLIED):** after `pools.resize(...)`, `if (device == VK_NULL_HANDLE) return;`
  then the create loop; add `#include <stdexcept>`. Approach A vs B still to be chosen by user.

---

## Finding #2 — Spurious "Descriptor set layout mismatch at index 0" warning  [RESOLVED — benign dead code]

- **Where emitted:** `src/render/PipelineCompatibility.h:18` (LOG_WARN in `ValidateLayoutCompatibility`).
- **Call site:** `src/render/TriangleRenderer.cpp:390` compared `graphicsSetLayouts` vs
  `computeSetLayouts` (graphics pipeline layouts vs the compute *cull* pipeline layouts).
- **Symptom:** every windowed run logged `Pipeline Compatibility Warning: Descriptor set
  mismatch at index 0!` (twice) and the misleadingly-worded "VERIFICATION SUCCESS" line.
- **Root cause (verified against shader sources + downstream usage):**
  Graphics set-0 (`simple.vert`+`simple.frag`: 1 binding — `InstanceBuffer`) and compute-cull
  set-0 (`cull.comp`: 4 bindings — `InstanceBuffer`, `IndirectBuffer`, `CountBuffer`,
  `depthPyramid` sampler) are DIFFERENT shader stages that **legitimately** have different
  layouts. The `ValidateLayoutCompatibility` call compared set-0 of two *independently-built*
  `VkDescriptorSetLayout` handles (`buildLayouts` called twice → always distinct handles), so it
  ALWAYS returned "differs". Its result was used ONLY for the log line — `createDescriptorSets`
  consumes `graphicsSetLayouts[0]`/`computeSetLayouts[0]` independently, never assuming equality.
  The check had **zero functional effect**.
- **Resolution (Option 1, user-approved):** removed the `ValidateLayoutCompatibility` call + the
  PASS/WARN log block, deleted the now-unused `src/render/PipelineCompatibility.h` and its include.
  Verified: incremental rebuild BUILD_EXIT 0; windowed run confirms the per-run WARN no longer fires.
- **Conclusion:** NOT a defect — benign false-positive scaffolding (apples-to-oranges by design).
  M1's "No validation warnings" criterion is satisfied once this dead code is gone.

---

## Finding #3 — Third-party Vulkan layer warnings  [CONFIRMED BENIGN — not an engine defect]

- **Symptom:** `--dev` runs print 5 `[WARNING: General]` messages: `GalaxyOverlayVkLayer*` (x3),
  `VK_LAYER_OBS_HOOK`, `VK_LAYER_RTSS`.
- **Fact-check:** /r/vulkan — GalaxyOverlayVkLayer is an implicit layer installed by the GOG
  client, "safely ignore." OBS GitHub issue #3145 — OBS's Vulkan hook injects these; removing OBS
  removes them. RTSS = RivaTuner/Afterburner overlay.
- **Conclusion:** environmental (GOG Galaxy / OBS / RivaTuner overlays injected into the process),
  NOT produced by engine code. No action in-repo; can be silenced by disabling those overlays when
  capturing a "clean validation" screenshot for the milestone gate.

---

## Positive verifications (M0/M1/M2 exit criteria that DO pass on local hardware)

- **M0 boot-smoke (headless --dev):** clean init + orderly RAII shutdown + "HEADLESS CI SMOKE TEST:
  SUCCESS". Capability tier detected: descriptorBuffer=true, shaderObject=true,
  unifiedImageLayouts=true, meshShaders=true, rtPipeline=false (Tier fallback correct on 2070S),
  queryTimestamps=true.
- **M0 pipeline cache:** "Saved pipeline cache to pipeline_cache.bin".
- **M1 triple-buffer:** "MAX_FRAMES_IN_FLIGHT == 3 confirmed"; index selection 0,1,2,0,1 correct.
- **M1 dense geometry compression:** verified, position error 0.0029, normal cosine 0.99999.
- **M2 physics/EventBus:** DamageSystem applied 150 dmg -> entity destroyed -> mesh swap to handle 2
  -> collider body removed -> "M2 Physics and EventBus verified." (covers 3 of 5 M2 exit criteria in
  the self-test path).
- **M2 soft-asset mitigation:** "Failed to load texture (soft failure, using fallback):
  assets/textures/nonexistent.png" — graceful degrade path exercised (intentional negative self-test).
- **Self-test negative paths also exercised (intentional, log as warning/error but are PASS
  behavior):** MaterialSystem overflow at 4096 cap; MetaRegistry rejects unregistered component.

---

## Still NOT verified (require live visual confirmation, cannot be asserted from logs alone)

- M2 "Debug drawing maps Jolt collision hulls over visual draws" — needs eyes on the window /
  RenderDoc capture; the headless/quit-frame runs don't prove the overlay renders correctly.
- M1 "ImGui panel updates cvar behaviors live" — interactive, needs manual confirmation.
- M1 "Hi-Z culling logs measurable draw call reduction" — not yet located in the run output.

---

## Recommended fix order (once user approves)
1. Finding #1 (real crash, blocks the whole test suite) — decide Approach A vs B.
2. Finding #2 (violates M1 "no warnings" criterion) — decide intent first, then fix or remove.
3. Finding #3 — no code change; document as environmental for the milestone gate.
