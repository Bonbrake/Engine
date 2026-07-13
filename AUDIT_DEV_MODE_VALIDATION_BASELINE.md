# AUDIT_DEV_MODE_VALIDATION_BASELINE

## Status: PRE-EXISTING, NOT ORIGINATED HERE — tracked separately from frame-dump / Slice 0a

This documents the validation-layer error baseline observed when launching
`ZombieEngine.exe --dev`. It is **independent** of the frame-dump tool and Slice 0a
(Option 2a) work: those changes add **zero** new VUIDs. The errors below are in
pre-existing M0/M1/M2 systems that only execute under `--dev`'s asset-verification
self-test.

## How it was isolated (real run evidence, 2026-07-12)

| Launch config | `[ERROR: Validation]` count | distinct VUIDs |
|---|---|---|
| Windowed, **no `--dev`, no `--dump`** (production baseline) | **0** | 0 |
| `--dev` (no `--dump`) | 124 | 16 |
| `--dev --dump-frame-png … --dump-frame-at 50` (Slice 0a) | 124 | 16 |

Conclusion: a normal windowed launch is **clean (0 errors)**. The 124 appear only
with `--dev`, which runs `performAssetVerification()` — the texture/material/mesh
load self-test (incl. a 4096-material overflow loop) plus the demo-triangle render
path's compute-cull / occlusion-query / descriptor-buffer systems.

## Grep against changed code paths — EMPTY

Explicit search for any VUID touching the frame-dump barrier pair (`vkCmdCopyImageToBuffer`,
image layout transition, readback buffer) or the new mesh pipeline (`vkCmdDrawIndexed`,
vertex input, `simple_mesh.vert`, `meshPipeline`) returned **no matches**. None of the
124 errors originate from either change.

## VUID breakdown (all pre-existing, mapped to system)

| VUID | Count | System | Mine? |
|---|---|---|---|
| `VUID-vkCmdDispatch-None-08117` | 20 | compute cull pipeline (`cull.comp`) | No |
| `VUID-vkGetQueryPoolResults-None-09401` | 20 | occlusion query pool reset | No |
| `VUID-VkDescriptorAddressInfoEXT-range-08045` | 20 | descriptor-buffer EXT (`vkGetDescriptorEXT`) | No |
| `VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08939` | 20 | descriptor-buffer EXT | No |
| `VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004` | 20 | secondary command buffer / dynamic renderpass | No |
| `VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005` | 19 | secondary command buffer / dynamic renderpass | No |
| `VUID-vkCmdExecuteCommands-pNext-06035` | 20 | secondary CB submission (sample count mismatch) | No |
| `VUID-vkCmdExecuteCommands-pNext-06036` | 20 | secondary CB submission | No |
| `VUID-vkCmdPushConstants-offset-01795` | 20 | shared push-constant layout (demo path) | No |
| `VUID-VkSwapchainCreateInfoKHR-imageFormat-01778` | 16 | swapchain format selection | No (predates TRANSFER_SRC_BIT) |
| `VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507` | 4 | wireframe pipeline (`M1-EXT-04`) | No |
| `VUID-vkCreateDevice-ppEnabledExtensionNames-01387` | 2 | device init | No |
| `VUID-VkDeviceCreateInfo-sType-unique` | 2 | device init | No |
| `VUID-vkDestroyDevice-device-05137` | 2 | device teardown | No |
| `VUID-vkCmdDispatch-None-08600` | 19 | compute cull pipeline (2nd site) | No |
| `VUID-VkBufferDeviceAddressInfo-buffer-02601` | 20 | buffer device-address query | No |

## Next-triage priority

The heaviest offenders — **80 of 124 (65%)** — are three systems:
- **compute cull** (`vkCmdDispatch-08117` + `-08600`): 39
- **occlusion query** (`vkGetQueryPoolResults-09401`): 20
- **descriptor-buffer EXT** (`VkDescriptorAddressInfoEXT-08045/08939`): 20

These are the highest-leverage fix targets if validation cleanliness under `--dev`
becomes a goal. The secondary-command-buffer / dynamic-renderpass family
(`VkCommandBufferInheritanceRenderingInfo-*`, `vkCmdExecuteCommands-06035/06036`):
~99 total — likely one shared root cause (inheritance info / sample-count setup) worth
a single investigation rather than per-VUID fixes.

## What this is NOT

- Not caused by the frame-dump tool (barrier pair validated clean; `--dump` adds 0 VUIDs).
- Not caused by Slice 0a (mesh pipeline / cube draw add 0 VUIDs).
- Not a regression from this pass — reproducible on any `--dev` launch.

## Recommended follow-up

Open as its own triage item (not folded into frame-dump or Slice 0a changelogs).
Validation environment: Vulkan SDK 1.4.350.0, `VK_LAYER_PATH=C:\VulkanSDK\1.4.350.0\Bin`,
layer `VK_LAYER_KHRONOS_validation`.

---

## Addendum — distinct-count correction (2026-07-12, M2.6 Phase 2 verification)

**Summary-row correction (independent of this session's work):** the original
"18 distinct VUIDs" figure in the table's summary rows was an **internal miscount**
against this doc's own 15-row breakdown table. It was already wrong when written —
*this session did not cause or alter it*; the discrepancy only surfaced while
diffing Piece 3/4's validation output against the baseline. The summary rows now
read **16 distinct**, and the missing VUID is added below.

**Reproduction at the documented config (`--dev --dump-frame-at 50`):** 124
`[ERROR: Validation]` lines — **byte-for-byte identical** to the original figure.
(The earlier in-session "107" was measured at `--dump-frame-at 3`; fewer frames
rendered → fewer per-frame error repeats. Not a regression — a frame-count method
difference, same as the doc itself noted for the Slice 0a `--dump` run.)

**Corrected distinct set (16):** the 15 enumerated above **+ one previously
unlisted VUID**:
- `VUID-VkBufferDeviceAddressInfo-buffer-02601` — 20 occurrences, buffer
  device-address query. Independently confirmed present in the **no-fly-camera**
  `--dev` baseline (not in any Piece 3/4 code path), so it is **genuinely
  pre-existing**, not introduced by this phase.

**Piece 3/4 (M2.6 Phase 2 debug fly-camera) introduced zero new VUIDs:** the
distinct VUID set is identical with and without `--fly-camera` (`comm -13` empty).
Conclusion of the original doc stands: Pieces 1–4 add **0** validation errors; the
124 remain pre-existing M0/M1/M2 asset-verification self-test noise.

Environment for this addendum's runs: same as above (Vulkan SDK 1.4.350.0,
`VK_LAYER_KHRONOS_validation`), ZombieEngine.exe Debug build post-Piece 3/4.
