# ZombieEngine — Master Plan & Source-of-Truth Index

> **Purpose of this file:** This is the reconciled authority for the recurring
> question *"where is the plan?"* and for every documentation contradiction found
> across the repository. It **does not replace** any existing document — it indexes
> them, disambiguates their roles, and records the corrections made.
>
> **Authoritative build/position facts** still live in the documents this file points
> to (`spec/`, `docs/status/STATUS.md`, `recon/plans/FINAL_COMPREHENSIVE_PLAN.md`).
> This file tells you *which one to open for what*, and *why the older docs disagreed*.

---

## 1. Canonical Plan Hierarchy (resolves the "where is the plan" contradiction)

The project never had a single "the plan" file. It has a **layered set of documents**,
each serving a different purpose. The contradiction came from four files each calling
*themselves* the source of truth. The correct model is a stack:

| Tier | Document | Role | Read it for… |
|------|----------|------|--------------| 
| **0 — Reconciled index** | `PLAN.md` (this file, repo root) | Disambiguates the docs below | "Which file is the plan?", contradiction history |
| **1 — Milestone working set** | `spec/` (M0–M13, M4.5, M6.5, sub-milestones) + `spec/ROADMAP.md` + `spec/_INDEX.md` | **What** to build, **in what order** | The live build map and per-milestone acceptance criteria (1,040 verified EXT blocks) |
| **2 — Master blueprint (research/architecture)** | `recon/plans/FINAL_COMPREHENSIVE_PLAN.md` | Definitive Program Plan: 100-paper research benchmark, EXT architecture, verification gates | Rationale, architecture decisions, research citations |
| **2b — Master Refactor Spec (v8.0)** | `docs/plans/ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v8.0.md` *(also at `C:\Users\jakeb\Documents\Plans\`)* | **198 concrete features across 6 Plain-English Stages + 6 Console Ports (v8.2)**, verified 0-cycle DAG, local couch co-op in Stage 3, dual VRAM budgets (6GB/8GB), thread affinity | The authoritative buildable roadmap — what to build next and in what order |
| **2c — Historical Refactor Spec (v7.0)** | `C:\Users\jakeb\Documents\Plans\ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v7.0.md` | Archived 480-part execution spec (superseded by v8.0 deduplication audit) | Deep algorithmic reference; do NOT use as build roadmap |
| **3 — Current position** | `docs/status/STATUS.md` | Session-independent record of where the project actually stands | "What's done / open right now?" |
| **4 — Historical origin** | `docs/legacy-desktop/ZombieEngine-Plan/USETHISITSV79.md` | The v79 pre-split master spec the milestones were split from (rescued from Desktop) | Cross-reference only; archived by `STATUS.md` |

**Rule of thumb:** Open **`docs/plans/ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v8.0.md`** to see the 6-stage build roadmap, **`spec/ROADMAP.md`** for milestone phases, **`docs/status/STATUS.md`**
to see current progress, and verify math/DAG via **`python scripts/verify_plan_v8.py`**.

---

## 2. Full Planning-Document Inventory (nothing removed)

### 2.1 Active / authoritative
| Path | Role |
|------|------|
| `PLAN.md` | This reconciled index. |
| `docs/plans/ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v8.0.md` | Authoritative Master Refactor Spec v8.2 (198 concrete features, 6 stages + 6 ports, supersedes v7.0). |
| `spec/ROADMAP.md` | Build phases (P0–P4), topological build order, environment, verification tooling. |
| `spec/_INDEX.md` | EXT block ID → file/line index (1079+ entries listed). |
| `spec/M0.md` … `spec/M13.md` | Per-milestone specs (M0–M13). |
| `spec/M2.6.md`, `M2.7.md`, `M2.8.md`, `M2.9.md`, `M4.5.md`, `M4.6.md`, `M5.1.md`–`M5.4.md`, `M6.5.md`, `M8.5.md`–`M8.7.md` | Sub-milestone / expansion specs. |
| `spec/AGENTS.md`, `spec/APPENDICES.md`, `spec/APPENDIX_K.md`, `spec/APPENDIX_L.md`, `spec/APPENDIX_M.md`, `spec/00_PROTOCOL.md`, `spec/_frontmatter.md`, `spec/_v80_presplit.md` | Spec governance, gap-fill EXT blocks, protocol. |
| `docs/status/STATUS.md` | Current build/position record (branch, completed milestones, open items). |
| `C:\Users\jakeb\Documents\Plans\ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v7.0.md` | Historical Master Refactor Spec v7.0 (archived for deep reference). |
| `docs/build/build.md`, `docs/verify/verify.md`, `docs/architecture/README.md`, `docs/research/engine_architecture_lessons.md`, `docs/research/reference_game_analysis.md`, `docs/guides/README.md` | Live build/verify/architecture/research docs. |
| `recon/plans/FINAL_COMPREHENSIVE_PLAN.md` | Definitive Program Plan (research + architecture master blueprint). |
| `recon/plans/EXT_BLOCK_COUNTS.md` | Generated, authoritative EXT block count (1040 total, see §3.4). |
| `scripts/verify_plan_v8.py` | Automated DAG, dependency ordering, and mirror parity validator. |
| `.hermes.md` | Hermes agent workflow rules (applies when cwd is under `C:\ZombieEngine`). |
| `README.md`, `AGENTS.md`, `COMPREHENSIVE_TASKLIST.md`, `HANDOFF.md`, `MEMORY.md`, `SECURITY.md`, `CONTRIBUTING.md` | Repo-root orientation, tasklist, handoff, memory. |

### 2.2 Reference / professionalization drafts (informational, not task lists)
`recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md`,
`recon/plans/2026-07-21_DEEP_EXECUTION_PLAN.md`,
`recon/plans/2026-07-22_MASTER_PLAN_V2.md` … `V7_SPEC.md`,
`recon/plans/2026-07-22_BUILD_REPAIR_PLAN.md`,
`recon/plans/2026-07-22_COMPREHENSIVE_IMPROVEMENT_PLAN.md`,
`recon/plans/APPENDIX_EXECUTION_MAP.md`,
`recon/plans/AUDIT_PROMPT.md`,
`recon/plans/NEXT_SESSION_PROMPT.md`,
`recon/plans/SESSION_HANDOFF_2026-07-21.md`.

### 2.3 Historical / archived (do NOT treat as active)
- `recon/plans/archive/*.md` (≈40 dated draft/proposal files).
- `archive/docs/*` (incl. `MASTER_PLAN_ENDLESS_QUARANTINE.md`, audits).
- `archive/old_milestones/milestones_M0-M13_antigravity/*`.
- `C:\Users\jakeb\Desktop\ZombieEngine-WorkingFiles\ZombieEngine-Plan\USETHISITSV79.md`
  and `MASTER_PLAN_ENDLESS_QUARANTINE.md` (pre-split originals).
- `.hermes/plans/*.md` (per-session agent plans: phase9-consolidation, M0-vulkan-bootstrap, etc.).

---

## 3. Contradictions Found & Resolutions

### 3.1 "Source of truth" — four files disagreed
| File | Old claim |
|------|-----------|
| `.hermes.md` | "`recon/plans/FINAL_COMPREHENSIVE_PLAN.md` — **The live source of truth.** All subagents MUST read this first." |
| `docs/status/STATUS.md` | "**Canonical spec:** `spec/` … the live source of truth for every `[Mx-EXT-nn]` block." |
| `HANDOFF.md` | "The master spec is `…\ZombieEngine-Plan\USETHISITSV79.md`." |
| `README.md` | Calls both `spec/` "the live source of truth" **and** `FINAL_COMPREHENSIVE_PLAN.md` "Master Blueprint." |

**Resolution:** Adopted the **tiered model in §1**. `spec/` = milestone working set;
`FINAL_COMPREHENSIVE_PLAN.md` = research/architecture blueprint (reference, not a task
list); `STATUS.md` = current position; `USETHISITSV79.md` = historical. Each of the four
files has been patched with a pointer to this `PLAN.md` and corrected wording (additive —
original content preserved).

### 3.2 Language / API standard — root `AGENTS.md` was wrong
- **Root `AGENTS.md` claimed:** *"Strict C++23, Vulkan 1.3 … Legacy standards strictly prohibited."*
- **Reality (verified):** Every other doc targets **C++20 / Vulkan 1.4**; the build uses
  **Vulkan SDK 1.4.350.0** (`tools/build_skeleton.bat`, `spec/ROADMAP.md`), MSVC v14.44
  (VS2022). The C++23/Vulkan 1.3 line was a factual error.
- **Resolution:** Corrected root `AGENTS.md` `[REQ-01]` to **C++20 / Vulkan 1.4 / MSVC 2022**,
  with an inline correction note. All other content retained.

### 3.3 Build command — three different instructions
| Source | Said to run |
|--------|-------------|
| `README.md` / `.hermes.md` | `scripts/build_ze.cmd` |
| `HANDOFF.md` | `tools\build_skeleton.bat` |
| `spec/ROADMAP.md` | raw `cmake -B build …` commands |

**Resolution:** Both scripts **exist**. Canonical = **`scripts/build_ze.cmd`** (points at
`C:\ZombieEngine` correctly). `tools/build_skeleton.bat` also exists but has a **known
defect**: it `cd`s to `C:\Users\jakeb\ZombieEngine` (a path that does not exist — the repo
is `C:\ZombieEngine`), so it should not be relied on until that line is fixed. Flagged,
not silently changed (build-script edits are out of scope for this doc reconciliation).

### 3.4 EXT block count — seven different numbers in the repo (all reconciled 2026-08-13)
| Claimed count | Where | Status |
|--------------|-------|--------|
| **1,040** | `recon/plans/EXT_BLOCK_COUNTS.md` (regenerated 2026-08-13 via `scripts/verify_ext_block_counts.py`) | ✅ **AUTHORITATIVE** |
| 1,224 | `docs/status/STATUS.md` "Last Verified Build" (2026-07-20 grep of 16 files) | ❌ stale → corrected to 1,040 |
| 1,080 | `spec/_INDEX.md` header | ❌ stale → corrected to 1,040 |
| 1,035 | `recon/plans/FINAL_COMPREHENSIVE_PLAN.md` status line | ❌ stale → corrected to 1,040 |
| 1,031 | `.hermes.md` "Verified EXT Blocks" | ❌ stale → corrected to 1,040 |
| 762 | `docs/status/STATUS.md` 2026-07-17 note + `spec/AGENTS.md` | historical snapshot → annotated, not current |

**Resolution:** The **authoritative generated count is 1,040** (`EXT_BLOCK_COUNTS.md`,
"Total JSON: 1040 / Total MD: 1040", no warnings). All five stale inline counts above were
corrected in place; the 762 figure is retained as a dated historical note. Regenerate
`EXT_BLOCK_COUNTS.md` after any future spec edit.

### 3.5 Active branch — docs contradicted git HEAD
- Docs (`docs/status/STATUS.md`, `MEMORY.md`) said `spec/m0-parity-reformat`.
- `git branch --show-current` = **`main`** (HEAD `0639c9d`). `spec/m0-parity-reformat` is the
  historical consolidation branch, not current. ✅ Both docs corrected to `main` (with note).

### 3.6 Build script — mandated script is broken
- `AGENTS.md [REQ-02]` and `HANDOFF.md` mandated `tools\build_skeleton.bat`.
- That script does `cd /d C:\Users\jakeb\ZombieEngine` — a path that **does not exist**
  (repo is `C:\ZombieEngine`), so it fails. ✅ `HANDOFF.md` re-pointed to `scripts\build_ze.cmd`.
  ⚠️ `AGENTS.md [REQ-02]` **still** cites the broken script — its edit was blocked by a
  protected-file approval timeout; needs your approval (see §6).

### 3.7 Target hardware — VRAM figure mismatch
- `.hermes.md` listed Tier-0 floor as "RTX 2070 SUPER / 6GB VRAM." The 2070 Super is **8GB**,
  and 6GB is the *floor* (RTX 2060-class). ✅ `.hermes.md` corrected to "RTX 2000-series / 6GB
  floor; dev machine RTX 2070 SUPER (8GB)." `FINAL_COMPREHENSIVE_PLAN.md` and `docs/architecture/README.md`
  already correctly use 8GB for the dev target.

## 4. Master Technology Stack & Modernization Ledger (September 2026)

This ledger establishes the uncompromising technical standard for ZombieEngine. Every component targets the optimal, verified baseline and newest release:

| Subsystem | Master Spec Standard | Host Current State | Modern Target Standard & Rationale | Action / Fix |
|---|---|---|---|---|
| **Graphics API** | Vulkan 1.4 Native Core (`volk` + `Vulkan-Hpp`) | Vulkan SDK 1.4.350.0 installed; Driver `616.92` reports **Vulkan 1.4.351** on RTX 2070S | **Vulkan 1.4 Native Core** (`VK_KHR_dynamic_rendering_local_read`, `VK_KHR_push_descriptor`, `VK_KHR_maintenance5/6/9`, `VK_EXT_descriptor_buffer`, `VK_EXT_shader_object`, Timeline Semaphores, Synchronization2). Zero legacy Vulkan 1.2/1.3 constructs. | Driver supports 1.4.351; Khronos/LunarG SDK 1.4.357.0 available for next SDK update cycle. |
| **Upscaling & Frame Generation** | NVIDIA DLSS 4.5 / AMD FSR 4 / Intel XeSS 2.0+ | Shader architecture integrated; velocity buffer planned | **NVIDIA DLSS 4.5** (Multi-Frame Generation, Transformer Super Resolution & Neural Ray Reconstruction) + **AMD FidelityFX Super Resolution 4 (FSR 4 Neural Upscaling & Frame Gen)** + **Intel XeSS 2.0+**. Vendor-detected runtime dispatch over per-pixel motion velocity buffer. | ✅ Formalized in Master Plan v8.0 Stage 4 (`T4-06`, `T4-42`, `T4-44`). |
| **Color Science & HDR** | AgX Color Science + Khronos PBR Neutral | Shaders implemented in `shaders/` (`agx_tonemap.comp.glsl`) | **AgX Color Science** tonemapping + **Khronos PBR Neutral** fallback; wide-gamut BT.2020 and HDR10 PQ delivery. Eliminates saturation burn-in and hue shifting in high-luminance muzzle flashes and explosions. | ✅ Validated clean on GPU across 3 passes. |
| **Compiler & Toolchain** | MSVC 2022 (v14.44), C++20 | Host verified: MSVC v14.44 (VS2022 BuildTools 17.14.40), CMake 4.4.3, Ninja 1.13 | **MSVC 2022 v17.14 / MSVC v14.44 (C++20)** + **CMake 4.4.3** + **Ninja 1.13**; `/fp:precise` strictly mandated on host for physics determinism. | ✅ Installed, configured, and verified active in `scripts/build_ze.cmd`. |
| **Dual-Core AI Director** | L4D2 Mathematical State Machine (60Hz tick) + Embedded GGUF (`llama.cpp`) on `SlmThread` | `SLMClient` configured for `Qwen2.5-3B-Instruct.Q4_K_M.gguf` | **Qwen2.5-3B-Instruct (Q4_K_M GGUF, ~1.9 GB)** + **Left 4 Dead 2 Intensity Pacing Machine** — Dual-core architecture. Mathematical stress/intensity curves drive real-time spawning, crescendo triggers, and pacing phases on the tick; neural SLM co-processor asynchronously generates dynamic survivor dialogue, emergency radio chatter, crescendo objectives, and procedural lore. Both systems mandatory (no fallback). Baseline: RTX 2070 SUPER 8GB (6.2 GB cap); Low-spec floor: RTX 2060 6GB (4.5 GB cap @ 30 FPS). | ✅ Reconciled. Both systems locked to execute in concert. |
| **Memory Allocator** | Vulkan Memory Allocator (VMA) 3.4.0 | VMA 3.4.0 in vcpkg | **VMA 3.4.0** with 64-byte L1 alignment and defragmentation enabled. Raw `vkAllocateMemory` strictly prohibited. | ✅ Compliant. |
| **Task / Job System** | enkiTS 1.12 | enkiTS 1.12 in vcpkg | **enkiTS Work-Stealing Task Scheduler 1.12**; 8-core CPU affinity (Core 0 Main, Core 1 Render, Core 2 Audio, Core 3 SLM, Cores 4-7 Workers). | Compliant with v8.0 spec. |
| **Physics Engine & Coordinates** | Jolt Physics 5.6.0 (Double-Precision `dvec3` CPU + Camera-Relative `vec3` GPU) | Jolt 5.6.0 active in vcpkg.json | **Jolt Physics 5.6.0** (`JPH_CROSS_PLATFORM_DETERMINISTIC`, `JPH_DOUBLE_PRECISION`). Simulation computes in 64-bit `dvec3` on CPU, translated to camera-relative 32-bit `vec3` ($\mathbf{P}_{\text{rel}} = \mathbf{P}_{\text{world}} - \mathbf{C}_{\text{camera}}$) on GPU. Eradicates FP64 1:32 GeForce gaming silicon performance penalty while guaranteeing zero jitter at 50km+. | ✅ Active in `vcpkg.json` with `cross-platform-deterministic` & `debugrenderer`. |
| **Windowing & Input** | SDL3 | SDL 3.4.12 in vcpkg.json | **SDL3 3.4.12** (Sub-tick 1000Hz raw mouse input, Steam Input integration, gyro aiming, high-precision event polling). | ✅ Active in `vcpkg.json` with `vulkan` feature. |
| **Spatial Audio Architecture** | SDL3 Lock-Free Spatial Audio Streams | AudioEngine initialized | **SDL3 Audio Streams (Sub-tick Lock-Free Mixing)** + HRTF binaural convolution, Supersonic Mach cone acoustics, logarithmic horde roar scaling ($\text{Vol} \propto \log_{10}(1+N)$). | ✅ Active in `src/audio/`. |
| **Asset Parsing** | fastgltf 0.9.0 | Active in vcpkg.json | **fastgltf 0.9.0** for zero-copy DMA streaming from NVMe into GPU VRAM. | ✅ Active in `vcpkg.json`. |
| **Entity Component System** | EnTT 3.16.0 | Active in vcpkg.json | **EnTT 3.16.0** (`view<Transform, MeshComponent>`) for cache-friendly archetype iteration. | ✅ Active in `vcpkg.json`. |
| **Logging & Formatting** | spdlog 1.17.0 | Active in vcpkg.json | **spdlog 1.17.0** for zero-alloc asynchronous ring buffer logging. | ✅ Compliant. |
| **Data Interchange** | nlohmann-json 3.12.0 | Active in vcpkg.json | **nlohmann-json 3.12.0** for fast JSON mod manifests and config loading. | ✅ Compliant. |
| **Unit Testing** | Catch2 3.15.2 | Active in vcpkg.json | **Catch2 3.15.2** (77 test cases, 76 passed, 1 skipped, 300/300 assertions passing). | ✅ Active and verified via `ZombieEngineTests.exe`. |
| **Hardware Target Standard** | Dual-Tier Budget: 8GB Baseline / 6GB Floor | RTX 2070 SUPER 8GB (Host) | **Primary Baseline: RTX 2070 SUPER 8GB** (6.2 GB VRAM hard cap, 1440p 60 FPS target); **Low-Spec Minimum Floor: RTX 2060 6GB** (4.5 GB VRAM hard cap @ 1080p 30 FPS lock, DLSS 4.5 / FSR 4 Performance mode). | ✅ Enforced across engine config and budgets. |

---

#### 4.1 World War Z Swarm Engine (500–1,000+ Zombie Hordes) & AI Director Pacing

The AI Director in *The Endless Quarantine* replaces legacy small-mob limits with **Saber Interactive's World War Z Swarm Engine architecture**, orchestrating massive **500 to 1,000+ zombie hordes** in real time on 8 GB consumer hardware:

```
+-------------------------------------------------------------------------------+
|                             GAME TICK (60 Hz)                                 |
|                                                                               |
|  +-------------------------------------------------------------------------+  |
|  |             L4D2 PACING STATE MACHINE (MAX SURVIVOR STRESS)             |  |
|  |  * Tracks MAX(SurvivorStress) across squad (pins/incaps hold peak)      |  |
|  |  * Pacing State: BuildUp -> SustainPeak (Swarm Wave) -> PeakFade -> Relax|  |
|  |  * Booth Break Condition: Relax timer pauses until engaged mobs cleared |  |
|  |  * Commands 650-1,000+ Swarm Wave during Crescendos & SustainPeak       |  |
|  +-----------------------------------+-------------------------------------+  |
|                                      | Wave Directives
|                                      v
|  +-------------------------------------------------------------------------+  |
|  |                WORLD WAR Z TWO-TIER SWARM ENGINE (1,024 AGENTS)         |  |
|  |  * O(1) Flow-Field Direction Grid (128x128 potential vector field)      |  |
|  |  * Tier A (Macro Flockers): VAT / GPU skinning, flocking repulsion       |  |
|  |  * Tier B (Micro Combat Actors): Promoted within 6m; full skeletal      |  |
|  |    hitboxes, localized dismemberment, dynamic stumbles, & ragdolls       |  |
|  |  * Living Zombie Pyramids: Sockets on walls/fences; structural base     |  |
|  |    damage collapses pyramid into falling ragdolls                       |  |
|  |  * Swarm Cluster Audio: Roar centroid volume scales with log10(1 + N)   |  |
|  +-----------------------------------+-------------------------------------+  |
+--------------------------------------|----------------------------------------+
                                       | Pacing Events & Telemetry
                                       v (Lock-free SPSC Queue)
+-------------------------------------------------------------------------------+
|                   ASYNCHRONOUS SLM CO-PROCESSOR (Qwen2.5-3B)                  |
|  * Dedicated strictly to asynchronous macro-events (zero combat tick blocking)|
|  * Military & civilian emergency radio broadcasts with natural squelch delay |
|  * Procedural safehouse graffiti, survivor journals, & post-mission AAR logs  |
|  * Background pre-generation of dialogue pools during Relax phase             |
+-------------------------------------------------------------------------------+
```

---

### 4.1b Silent Protagonist Mandate & NPC-Only Criteria Response System

To preserve total survival-horror tension and tactical immersion:
- **Silent Protagonist Invariant:** The human player character NEVER speaks unsolicited barks, callouts, or quips (*"Reloading!", "Taking fire!"*). Player vocalizations are strictly biological and non-verbal (stamina exhaustion panting, tachycardia heartbeats, adrenaline gasps, pain grunts).
- **NPC-Only Criteria Response System (`< 0.1ms` dispatch):** All spoken tactical callouts (*"Cover me, reloading!", "Swarm incoming!", "Mutator spotted!"*) are emitted strictly by NPC squadmates, radio dispatchers, and safehouse survivors.
- **Criteria Rules:** Game state evaluations (`is_npc == true`, `speaker != PLAYER_ENTITY_ID`, `health_tier`, `cooldown`, `director_intensity`) query instantaneous response tables, completely decoupling dialogue from SLM latency.

---

### 4.2 True First-Person Tactical Movement & Soft Proportional Free-Aim

Incorporating the visceral movement realism from modern tactical shooters (*Bodycam* v0.8) while delivering pure human eyesight and zero input lag:
- **Direct Eye-Level Perspective:** Positioned at authoritative human eye-socket height ($1.68\text{m}$).
- **Zero Video Artifacts:** Barrel distortion ($k_1=0, k_2=0$), chromatic aberration ($0.0$), CMOS sensor noise ($0.0$), and camera OSD overlays are completely removed. Pure human eyesight.
- **Soft Proportional Free-Aim (75% Gun / 25% Camera):** Inside the deadzone ($\pm 12^\circ$), analog stick movement splits: 75% drives decoupled weapon deflection, and 25% simultaneously rotates the camera head smoothly. Eliminates the "swimming in molasses / input lag" feel while preserving loose tactical weapon feel.
- **Physical Head/Neck Inertia:** Second-order spring-damper mass lag:
  $$\ddot{\theta} + 2 \zeta \omega_n \dot{\theta} + \omega_n^2 \theta = \omega_n^2 \theta_{\text{target}}$$
  with $\omega_n = 14.0\text{ rad/s}$ and $\zeta = 0.85$.
- **Kinetic Footfall Impacts:** Physical vertical displacement ($0.035\text{m}$) and roll tilt ($0.8^\circ$) per footstep.
- **Stress-Coupled Tremor:** Dual-frequency micro-tremor scaling dynamically with Director intensity.

---

### 4.3 Bethesda-Style 3-Tier Item Persistence & Kinematic Soft-Wakeup

Permanent physical world state persistence where every placed, dropped, or disturbed item retains its exact resting transform without save bloat or physics explosions:
- **3-Tier Persistence Architecture:**
  - *Tier 1 (Eternal):* Weapons, ammo boxes, medical gear, containers, and player-placed barricades persist permanently in chunk delta snapshots.
  - *Tier 2 (Clutter LRU):* Empty cans, mugs, bottles, and loose debris. Enforces a strict circular buffer capped at **256 items per chunk**. Oldest untouched clutter recycles upon cell reset, guaranteeing save files remain under $5\text{ MB}$.
  - *Tier 3 (Ephemeral):* Spent brass casings, blood decals, and dismembered gore. Purged on chunk unload.
- **Kinematic Sleep on Load (Zero Havok Explosions):** When a chunk loads, sleeping items initialize as kinematic/static colliders with zero collision response impulses.
- **3-Frame Soft Contact Relaxation:** Dynamic wake transitions apply a 3-frame velocity damping filter ($v_{\text{max}} \le 1.5\text{ m/s}$), bleeding off micro-overlap penetration energy and completely eliminating supersonic launches.
- **Skyrim/Fallout Physics Grab & Arrange:** 6-DOF spring-damper constraint allowing survivors to physically lift, rotate, and neatly arrange items on shelves, desks, or barricades.

---

### 4.4 Primary Controller Input Architecture, Steam Input & Gyro Aiming

Authoritative controller ergonomics engineered for tactical free-aim gunplay:
- **Steam Input API (`ISteamInput`) + SDL3 Fallback:** Native Steamworks integration ensuring day-one **Steam Deck Verified** compatibility and flawless PS5 DualSense / Switch Pro support.
- **Dynamic Platform Glyphs:** Automatically resolves native glyphs (`DualSense` $\times/\square/\triangle/\bigcirc$, `Xbox` $A/B/X/Y$, `Steam Deck`, `Switch` $B/A/Y/X$).
- **Gyro Aiming (Motion Aiming) & Flick Stick:** 1:1 gyroscope micro-corrections during Aim-Down-Sights (ADS) for mouse-like precision without arcade auto-aim snapping, with optional flick stick for instant $180^\circ$ snap turns.
- **Dual-Zone Radial Stick Response Curve:** Exponential curve ($\gamma = 2.4$) for fine free-aim and acceleration ramp for torso turning.
- **Contextual Hold-Timer Discrimination:** Tap reload ($< 300\text{ms}$) executes tactical reload; hold reload ($\ge 300\text{ms}$) initiates physical magazine check; hold interact ($\ge 350\text{ms}$) activates physics grab.

---

### 4.5 Ultra-Realistic Ballistic Acoustics & Foley Stack

- **Supersonic Mach Cone Acoustics:** Projectiles traveling $> 343\text{ m/s}$ generate conical shockwaves ($\sin\theta_M = c/v$), separating crack from delayed muzzle blast. Subsonic projectiles ($< 343\text{ m/s}$) produce turbulence whiz-bys with pitch Doppler plunge.
- **Hierarchical Swarm Roar:** Density centroid emitter scaling volume logarithmically ($\text{Vol} \propto \log_{10}(1+N)$) combined with localized spatial snarling for the closest threats.
- **Multi-Stage Mechanical Weapon Foley:** Distinct audio stages for sear release, chamber detonation, bolt carrier slam, spent casing extraction, and bolt return.
- **Surface-Dependent Brass Casing Impacts:** Concrete, wood, metal, carpet, and earth resonant filter profiles.
- **Kinetic Gear Rattle:** Plate-carrier, Molle, and sling clatter driven by player linear acceleration and torso angular jerk.

---

### 4.6 Tactical Procedural Animation, Weapon Collision & Point-Shooting Tuck

- **Compressed High-Ready Point-Shooting:** When clearance $< L_{\text{barrel}} \approx 0.75\text{m}$, the weapon compresses backward into the armpit at chest level. **Firing is NOT disabled:** survivors can panic point-shoot with higher spread and recoil penalties. Only when jammed flat against a solid static wall ($< 0.15\text{m}$) is fire physically obstructed.
- **Weapon Mass Rotational Inertia:** Second-order spring-damper barrel lag proportional to turning velocity, simulating physical firearm weight.
- **Physical Magazine Inspection State Machine:** Real-time multi-stage inspection (`DroppingToPalm` $\to$ `InspectingWitnessHoles` $\to$ `ReseatingMagazine`), providing visual ammo awareness (`Full`, `ApproximatelyHalf`, `Low`, `Empty`) without arcade HUD numbers.
- **Stress-Reactive Pacing Integration:** Elevated breathing sway frequency and amplitude scaling directly with Left 4 Dead 2 Director intensity (`SustainPeak`).

---

### 4.7 Steamworks Native Input, Steam Deck Architecture & Cloud Persistence

- **Steam Input API (`ISteamInput`) Action Sets:** Defines an authoritative action manifest (`assets/steam/game_actions.vdf`) isolating hardware inputs into contextual action sets (`InGame`, `Menu`, `TacticalWheel`). Eliminates direct button hardcoding and ensures seamless dynamic glyph resolution (`Steam Deck`, `DualSense`, `Xbox`).
- **Steam Deck Native Profile:** Automated detection of Steam Deck APU and 16:10 aspect ratio ($1280 \times 800$). Enforces readable HUD/text scaling ($\ge 1.25\times$), native gyro aiming layers active during ADS, and frame-pacing capped at 40/60Hz for battery efficiency.
- **`.zesave` Binary Chunk Cloud Persistence:** Bethesda-style 3-tier persistence serialized into compact binary chunk snapshots compressed via LZ4/Zstandard. Chunk checksums (CRC32/XXH64) prevent corrupted cloud syncs and guarantee save payloads stay under $5\text{ MB}$ per quarantine sector.

---

### 4.8 Asset Cooking Pipeline, BC7/BC5 Compression & Vertex Animation Textures (VAT)

- **Proprietary Binary Package Format (`.zepak`):** High-speed container format (magic `0x5A45504B` / `ZEPK`) enabling zero-copy direct memory-mapping (DMA) from NVMe straight into GPU staging buffers via fastgltf and VMA.
- **Hardware Texture Compression:**
  - **BC7 (`VK_FORMAT_BC7_UNORM_BLOCK` / `SRGB`):** 4:1 compression for albedo, metallic, roughness, and ambient occlusion textures.
  - **BC5 (`VK_FORMAT_BC5_UNORM_BLOCK`):** Dedicated dual-channel two-component tangent-space normal maps ($R, G$ channels; $B = \sqrt{1 - R^2 - G^2}$).
  - Full pre-computed mip-chain generation in cook time, eradicating runtime GPU downsampling stalls.
- **Vertex Animation Textures (VAT) for Swarm Flockers:** Offline baking of skeletal run/crawl/climb animation cycles into 16-bit half-float position and 8-bit SNORM normal textures. 1,000 macro zombies evaluate bone transforms entirely inside vertex shaders via texture lookups, completely removing CPU skinning bottlenecks.
- **Pre-Baked Jolt Collision Hulls:** Offline baking of complex level geometry and debris into pre-computed Jolt convex hull and compound shapes, allowing $O(1)$ physics instantiation without runtime mesh cooking.

---

### 4.9 Tactical NPC Squad Command Wheel & Autonomous Coordination

- **Gamepad Radial Command Interface:** Holding `LB` / `L1` opens an analog radial command selector; right thumbstick vector selects one of 6 tactical directives:
  1. `HoldPosition`: Orders squadmate to establish a 360-degree security perimeter.
  2. `BreachAndClear`: Commands squadmate to kick open a closed door and clear the immediate interior.
  3. `BarricadeWindow`: Designates a compromised window or doorway for physical reinforcement.
  4. `FocusFire`: Designates a high-threat target (e.g., special infected, mutator) for concentrated squad fire.
  5. `ScavengeSupplies`: Directs squadmate to search the target room/container for Tier 1 ammo/medical items.
  6. `FallBack`: Immediately breaks combat engagement and regroup on the player's position.
- **Contextual World Target Raycasting:** The command wheel casts an environmental raycast into the scene geometry, resolving the exact target object (door, window, infected entity, loot container).
- **Sub-Millisecond Response Integration:** Selected orders immediately trigger tactical NPC acknowledgment barks through `ResponseSystem` (*"Roger that, reinforcing barricade!"*, *"Breaching door, cover me!"*), reinforcing squad cohesion without SLM latency.

---

### 4.10 Vulkan 1.4 Pipeline Pre-Warming & Zero-Stutter Shader Cache

- **Boot-Time Pipeline Pre-Warming:** During the initial boot sequence, the engine iterates over all active material/mesh permutations and compiles them against the active `VkPipelineCache` using `VK_EXT_pipeline_creation_cache_control`.
- **Eradication of In-Combat Compilation Stutter:** Guarantees that muzzle flashes, dynamic blood splatter passes, explosive particles, and new zombie types are already resident in GPU pipeline memory before gameplay commences.
- **Persistent Disk Cache (`pipeline_cache.bin`):** Validates driver UUID and pipeline version header on launch. If the driver or GPU changes, cache invalidation regenerates the pipeline database cleanly without crashes.

---

### 4.11 Hardware VRAM Budget Guard (Dual Target: 8 GB Baseline & 6 GB Floor)

To guarantee flawless stability across consumer gaming hardware while accounting for Windows Desktop Window Manager (DWM), Discord, and streaming applications:

#### Tier 1: Primary Engineering Baseline — RTX 2070 SUPER 8 GB (1440p @ 60 FPS)
- **Maximum Allocated Game VRAM:** **$6.2\text{ GB}$ Hard Ceiling**
  - Asynchronous SLM Co-Processor (`Qwen2.5-3B-Instruct.Q4_K_M.gguf`): **$1.9\text{ GB}$**
  - Vulkan 1.4 Render Targets, Depth Stencils & G-Buffers: **$0.8\text{ GB}$**
  - Geometry, Vertex Buffers & Swarm VAT Instance Buffers: **$0.7\text{ GB}$**
  - Mip-Streamed BC7/BC5 Textures: **$2.5\text{ GB}$**
  - Jolt Physics Buffers & Audio Spatial Ring Buffers: **$0.3\text{ GB}$**
- **Reserved OS / Background Headroom:** **$1.8\text{ GB}$** permanently reserved to prevent driver-level paging thrashing.
- **Active Mipmap Streaming:** Textures stream resident mip levels dynamically based on camera distance and screen-space footprint, shedding higher mips when approaching the $6.2\text{ GB}$ ceiling.

#### Tier 0: Low-Spec Minimum Hardware Floor — RTX 2060 6 GB (1080p @ 30 FPS Lock)
- **Maximum Allocated Game VRAM:** **$4.5\text{ GB}$ Hard Ceiling**
  - Asynchronous SLM Co-Processor (`Qwen2.5-3B` Q3_K_M / Q2_K or Host CPU RAM offload): **$1.2\text{ GB}$** (or $0.0\text{ GB}$ if CPU-pinned)
  - Vulkan 1.4 Render Targets (540p internal render resolution + DLSS 4.5 / FSR 4 / XeSS 2.0+ Performance upscaling to 1080p): **$0.4\text{ GB}$**
  - Geometry, Vertex Buffers & Swarm VAT Buffers: **$0.6\text{ GB}$**
  - Mip-Streamed BC7/BC5 Textures (Aggressive mip bias / 1K clamp): **$1.4\text{ GB}$**
  - Jolt Physics Buffers & Audio Spatial Ring Buffers: **$0.3\text{ GB}$**
  - Dynamic Safety Headroom: **$0.6\text{ GB}$**
- **Reserved OS / Background Headroom:** **$1.5\text{ GB}$** permanently reserved.
- **Frame Pacing:** Locked 30 FPS cap via timeline semaphore frame pacing to guarantee thermal and frame-time stability on 6 GB silicon.

---

### 4.12 Data-Driven Attribute Architecture & Modding Tables

- **Decoupled Data Architecture:** All firearm ballistic profiles (chamber pressure, muzzle velocity, recoil impulses, sear cycle timings), zombie attributes (health, stumble resistance, dismemberment thresholds), and L4D2 Director pacing coefficients are defined in human-readable JSON files (`data/weapons/*.json`, `data/director/*.json`).
- **Zero-Recompile Gameplay Tuning:** Allows instantaneous modification of game feel, weapon recoil balance, and horde wave intensities without rebuilding C++ source binaries.
- **Modding Directory Support:** Game automatically scans and mounts overrides from a `mods/` directory, laying the foundation for custom community weapons, sound replacements, and quarantine sectors.

---

## 5. Current Engine Status & Verification State

- **Active Branch:** `main` (HEAD).
- **EXT Corpus:** Exactly **1,040 verified logic blocks** across 33 specification files (`recon/plans/EXT_BLOCK_COUNTS.md`).
- **Engine Verification Baseline:**
  - **M0 (Vulkan Bootstrap):** PASS (`[REQ-04]`). Clean validation layers on RTX 2070 SUPER, pipeline cache active, memory budget logged via VMA, `HEADLESS_BOOT_OK`.
  - **Subsystem Architecture:**
    - All 17 modular engine subsystems (`ze-ai`, `ze-audio`, `ze-combat`, `ze-core`, `ze-debug`, `ze-ecs`, `ze-events`, `ze-modding`, `ze-net`, `ze-physics`, `ze-render`, `ze-save`, `ze-slm`, `ze-survival`, `ze-ui`, `ze-vehicle`, `ze-world`) are fully integrated and linked in `CMakeLists.txt`.
    - Engine boots, verifies core singletons, initializes Jolt 5.6.0 physics (`dvec3` double-precision, cross-platform deterministic), EventBus, DamageSystem, AudioEngine, BiomeGraph, ChunkStreamer, Survival, SaveSystem, Modding, L4D2 Director, SwarmEngine, SLMClient, and VehicleSystem with zero validation layer exceptions.
  - **Unit Test Suite:** Catch2 v3.15.2 test harness (`build\tests\ZombieEngineTests.exe`) fully active: 77 test cases (76 passed, 1 skipped for headless video, 300/300 assertions passing, 0 failures).
- **Capability Tier (RTX 2070 Super):**
  - `descriptorBuffer` ✅ | `shaderObject` ✅ | `unifiedImageLayouts` ✅ | `meshShaders` ✅ | `queryTimestamps` ✅ | `rtPipeline` (Turing hardware RT cores present, scheduled for Stage 4 activation).

---

## 6. Build & Verify Commands (canonical)

```bat
REM Canonical build (initializes MSVC 2022 BuildTools x64, CMake 4.4.3, Ninja):
cmd.exe /c "C:\ZombieEngine\scripts\build_ze.cmd"

REM Headless runtime verification (must exit 0 with HEADLESS_BOOT_OK, [REQ-04]):
cmd.exe /c "cd /d C:\ZombieEngine\build && EndlessQuarantine.exe --headless"

REM Standalone Catch2 unit test suite (77 tests, 300 assertions):
cmd.exe /c "cd /d C:\ZombieEngine\build\tests && ZombieEngineTests.exe"

REM Sanity and SLM harness:
cmd.exe /c "cd /d C:\ZombieEngine\build && sanity_suite.exe"

REM Full canonical validation pipeline (enforces [REQ-01], [REQ-02], [REQ-04]):
cmd.exe /c "C:\ZombieEngine\scripts\test_all.cmd"

REM Spec block count verification (1,040 verified blocks):
python scripts/verify_ext_block_counts.py

REM Master Refactor Plan v8.0 DAG & mirror parity verification:
python scripts/verify_plan_v8.py
```

Environment: Windows 11, AMD Ryzen 7 5700G, RTX 2070 SUPER (8GB VRAM), MSVC 2022 (v14.44), Ninja 1.13, CMake 4.4.3, Vulkan SDK 1.4.350.0.

---

## 7. Document-Hygiene Notes
- `recon/plans/archive/` and `archive/old_milestones/` are **historical** — do not open them
  expecting current tasks.
- The root `AGENTS.md` carries a `PROFILE GUARD` stating it applies only to a `zombieengine`
  profile that does not exist (only `default` exists); per `.hermes.md` that guard made the
  file effectively dead. Its *rules* are still useful and have been retained, with the
  version error corrected; live workflow rules live in `.hermes.md`.
- This file (`PLAN.md`) is the single entry point for "where is the plan." Link to it from
  new sessions rather than re-litigating the four old claims.

*Reconciliation performed 2026-08-13, updated September 2026 — additive and reconciled.*

