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
| **1 — Milestone working set** | `spec/` (M0–M13, M4.5, M6.5, sub-milestones) + `spec/ROADMAP.md` + `spec/_INDEX.md` | **What** to build, **in what order** | The live build map and per-milestone acceptance criteria |
| **2 — Master blueprint (research/architecture)** | `recon/plans/FINAL_COMPREHENSIVE_PLAN.md` | Definitive Program Plan: 100-paper research benchmark, EXT architecture, verification gates | Rationale, architecture decisions, research citations |
| **3 — Current position** | `docs/status/STATUS.md` | Session-independent record of where the project actually stands | "What's done / open right now?" |
| **4 — Historical origin** | `C:\Users\jakeb\Desktop\ZombieEngine-WorkingFiles\ZombieEngine-Plan\USETHISITSV79.md` | The v79 pre-split master spec the milestones were split from | Cross-reference only; archived by `STATUS.md` |

**Rule of thumb:** Open **`spec/ROADMAP.md`** to see the build order, **`docs/status/STATUS.md`**
to see current progress, **`recon/plans/FINAL_COMPREHENSIVE_PLAN.md`** for the research/architecture
bible, and **`USETHISITSV79.md`** only when you need the original pre-split wording.

---

## 2. Full Planning-Document Inventory (nothing removed)

### 2.1 Active / authoritative
| Path | Role |
|------|------|
| `PLAN.md` | This reconciled index. |
| `spec/ROADMAP.md` | Build phases (P0–P4), topological build order, environment, verification tooling. |
| `spec/_INDEX.md` | EXT block ID → file/line index (1079+ entries listed). |
| `spec/M0.md` … `spec/M13.md` | Per-milestone specs (M0–M13). |
| `spec/M2.6.md`, `M2.7.md`, `M2.8.md`, `M2.9.md`, `M4.5.md`, `M4.6.md`, `M5.1.md`–`M5.4.md`, `M6.5.md`, `M8.5.md`–`M8.7.md` | Sub-milestone / expansion specs. |
| `spec/AGENTS.md`, `spec/APPENDICES.md`, `spec/APPENDIX_K.md`, `spec/APPENDIX_L.md`, `spec/APPENDIX_M.md`, `spec/00_PROTOCOL.md`, `spec/_frontmatter.md`, `spec/_v80_presplit.md` | Spec governance, gap-fill EXT blocks, protocol. |
| `docs/status/STATUS.md` | Current build/position record (branch, completed milestones, open items). |
| `docs/build/build.md`, `docs/verify/verify.md`, `docs/architecture/README.md`, `docs/research/engine_architecture_lessons.md`, `docs/research/reference_game_analysis.md`, `docs/guides/README.md` | Live build/verify/architecture/research docs. |
| `recon/plans/FINAL_COMPREHENSIVE_PLAN.md` | Definitive Program Plan (research + architecture master blueprint). |
| `recon/plans/EXT_BLOCK_COUNTS.md` | Generated, authoritative EXT block count (1040 total, see §3.4). |
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

---

## 4. Current Build Status (mirrored from `docs/status/STATUS.md`, 2026-08-13)

- **Active branch:** `main` (working set was consolidated on `spec/m0-parity-reformat`).
- **M0** — CLOSED as PASS (2026-07-12), 7 carried open items (driver blocklist, focus-probe
  runtime, debug-utils naming scope).
- **M1** — CLOSED (2026-07-12); ECS→render bridge built + unit-tested; several carried open
  items (material batching, skinned animation, glTF loader).
- **M2** — EXIT criteria mostly resolved (mesh-swap GPU-verified dev-gated; F2/F3 closed);
  carried open items noted.
- **M2.6** — **Phase 1 COMPLETE** ✅ and **Phase 2 (fly-camera + self-verify) COMPLETE** ✅
  with carried caveats (yaw-sign GPU confirm, far-cube steadiness under live flight).
- **EXT corpus:** 1,040 verified blocks across 33 spec files (per `EXT_BLOCK_COUNTS.md`).
- **Capability tier (RTX 2070 Super):** descriptorBuffer / shaderObject / unifiedImageLayouts
  / meshShaders / queryTimestamps ✅; rtPipeline ❌.

---

## 5. Build & Verify Commands (canonical)

```bat
REM Canonical build (verified present, points at C:\ZombieEngine):
cmd.exe /c "C:\ZombieEngine\scripts\build_ze.cmd"

REM Spec structural verification (run after any spec change):
python scripts/verify_m0_parity.py
python scripts/verify_ext_block_counts.py

REM Unit tests:
build\tests\ZombieEngineTests.exe        REM (or ctest; see STATUS.md harness note)
```

Environment: Windows 11, MSVC 2022 (v14.44), Ninja, vcpkg (29 pkgs), Vulkan SDK 1.4.350.0,
target x64. See `docs/build/build.md` and `spec/ROADMAP.md` for full detail.

---

## 5. Document-Hygiene Notes
- `recon/plans/archive/` and `archive/old_milestones/` are **historical** — do not open them
  expecting current tasks.
- The root `AGENTS.md` carries a `PROFILE GUARD` stating it applies only to a `zombieengine`
  profile that does not exist (only `default` exists); per `.hermes.md` that guard made the
  file effectively dead. Its *rules* are still useful and have been retained, with the
  version error corrected; live workflow rules live in `.hermes.md`.
- This file (`PLAN.md`) is the single entry point for "where is the plan." Link to it from
  new sessions rather than re-litigating the four old claims.

*Reconciliation performed 2026-08-13 — additive only, no source documents removed.*
