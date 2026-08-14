# ZOMBIE ENGINE: ENTERPRISE ARCHITECTURE POLICY (AGENTS.MD)
**CLASSIFICATION:** RESTRICTED / BOARD LEVEL
**SCOPE:** All Autonomous Subagents & Primary Architect
**PROFILE GUARD:** [CRITICAL] THESE RULES APPLY *ONLY* TO THE `zombieengine` PROFILE. IF YOU ARE THE `default` PROFILE (Jake's Personal AI), IGNORE THIS ENTIRE FILE.

## 1. CORE ENGINEERING DIRECTIVES (ZERO-DEFECT STANDARD)
Any deviation from these parameters is considered a **CRITICAL SEVERITY (CVSS 10.0)** violation of engine stability.

*   **[REQ-01] Stack Authority:** Strict C++20, Vulkan 1.4, MSVC 2022 (v14.44) Compiler. *(Corrected 2026-08-13: the prior "C++23 / Vulkan 1.3" wording contradicted the actual toolchain — the build uses Vulkan SDK 1.4.350.0 and all other engine docs target C++20 / Vulkan 1.4.)* Legacy standards are strictly prohibited.
*   **[REQ-02] Build Pipeline:** Never invoke raw CMake commands. All build orchestration MUST route through the canonical build script: `scripts\\build_ze.cmd`. *(Corrected 2026-08-13: `tools\\build_skeleton.bat` `cd`s to a nonexistent path `C:\\Users\\jakeb\\ZombieEngine`; the repo root is `C:\\ZombieEngine`.)*
*   **[REQ-03] Memory Architecture:** Vulkan Memory Allocator (VMA) is mandatory. The use of raw `vkAllocateMemory` is an immediate audit failure.
*   **[REQ-04] Verification Standard:** No PR or branch merge is accepted until verified via `build\EndlessQuarantine.exe --headless` (Zero Validation Layer Exceptions).
*   **[REQ-05] Architectural Alignment:** All structural changes must strictly mirror `ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v6.1.md`. Hallucination of architecture is forbidden.

## 2. 5 C'S DEBUGGING PROTOCOL (MANDATORY)
When encountering a compilation or runtime failure, Subagents MUST execute the following framework before modifying C++ source:
*   **Condition:** Ingest raw compiler/runtime traceback.
*   **Criteria:** Formulate a strict, evidence-based hypothesis.
*   **Cause:** Reproduce the failure in an isolated test environment.
*   **Consequence:** Evaluate blast radius (memory leak, race condition, VRAM saturation).
*   **Corrective Action:** Deploy a targeted surgical fix and re-verify against `[REQ-04]`.

## 3. ASYNCHRONOUS ORCHESTRATION
*   **Parallel Execution:** Utilize the `subagent-driven-development` pipeline for all non-blocking module refactors.
*   **Blackbox Intervention:** Route heavily complex algorithmic optimizations (e.g. Cache Serialization) through the `blackbox` multi-model Judge.
