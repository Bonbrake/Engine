# SESSION ENDPOINT — ZombieEngine Spec Parity + Systems Coverage (2026-07-16)

> **Purpose:** clean handoff so a NEW session can continue without re-reading the whole chat.
> **Branch:** `spec/m0-parity-reformat`  **Repo:** `C:\ZombieEngine`  **Model/runtime:** Hermes Agent
> **Read `.hermis.md` (repo root) FIRST** — it is the authoritative spec-navigation + editing-rules file. Milestone spec files (`spec/Mx.md`, `APPENDICES.md`) are READ-ONLY to the agent unless explicit GO.

---

## 1. WHERE WE ARE (verified on disk, end of session)

**A. Spec parity (M0-format reformat) — DONE, uncommitted**
- `spec/M1.md` = 52 `#### [M1-EXT-NN]` blocks (1–28, 29–38 are PURE STUBS, 39–53). Rebuilt from intact base `baaf5cb` (HEAD `43aae1e` was corrupted 42→7). Verified `M1|52|OK` by `scripts/verify_m0_parity.py`.
- 15 other milestone files (`M0,M2–M13,M4.5`) each got an additive `name:` codename field (NOT renamed — preserves refs). `spec/M1.index.json` = 52 entries (10 STUB).
- `verify_m0_parity.py` runs clean: **15/15 OK, 0 FIX** (real run this session).
- All above are UNCOMMITTED (working tree modified: 16 spec files + `.hermis.md`).

**B. Systems coverage audit — DONE (audit only, not ported)**
- Live spec = **625 EXT blocks** (H4 headers, matches verifier).
- 10-archive sweep (early→latest) re-verified H4-vs-H4: archive union 586, of which **165 not-in-live = 107 curated provisional (master `MASTER_PLAN_ENDLESS_QUARANTINE.md` + `IDEA.md`) + 58 earliest-only orphans**.
- **10 ENTIRELY-MISSING sub-milestones** (H4 in archive, absent in live): M2.6, M2.7, M2.9, M4.6, M5.1, M5.2, M5.3, M8.5, M8.6, M8.7 (107 blocks).
- Plan: `recon/plans/2026-07-16_Systems-Coverage-Plan.md` (stage-gated G0–G3, audit+flag, NO auto-port/invent).

**C. Production-ready plan — DONE**
- `recon/plans/2026-07-16_Production-Ready-Parity-Plan.md` — IEEE29148 SRS + stage-gate + scope IN/OUT + bite-sized tasks. Covers whole project (engine `src/` 13 modules, Khronos 2026 bar, TI Video 29 deferred-MSAA gospel refresh at G2.5).

**D. Web tooling — FIXED**
- `recon/web_search.py` = free curl+Brave (key in `C:\Users\jakeb\AppData\Local\hermes\.env`, temporary, free tier). Bing BANNED. DDG/SearXNG blocked. Playwright Chromium installed.

---

## 2. OPEN DECISIONS (user must answer before execution)

| # | Question | Options / default |
|---|---|---|
| 1 | Commit the verified spec work (batches 0→A→B→C→D→E)? | GO / hold |
| 2 | `.hermis.md` stray 5-line edit — commit separate / leave / revert? | commit-separate (recommended) |
| 3 | `archive/PLAN_outstanding_work_*.md` — track in git or disk-only? | track (recommended) |
| 4 | Run build gate (`cmake --build build --config Debug` + `Tests.exe`)? | yes (compiles now) |
| 5 | Systems: **port** the 107 (create 10 missing sub-milestone files) or just **flag**? | flag-for-now (GO-gated edit) |
| 6 | 10 missing sub-milestones — new files or fold into M2/M4/M5/M8? | new files (recommended) |

---

## 3. KEY GROUND TRUTH (don't re-derive; trust these)

- **Branch divergence:** `main` 114 ahead, `beta` 91 ahead of `spec/m0-parity-reformat`. 3 subagent worktrees (NOT 2) all @`804cb55`: Core-Code-Auditor, ECS-and-Debug-Auditor, Render-Code-Auditor — under `C:\Users\jakeb\.gemini\antigravity\brain\...\system_generated\worktrees\`. Reconcile before merge (Gitflow: this→beta tag parity-m0→main).
- **Engine reality:** living Vulkan 1.4 engine. `src/` = 13 modules (render 35, core 24, ecs 10, physics 4, rest 1 cpp). `CMakeLists` builds `ZombieEngine` + `ZombieEngineTests` (Catch2). `build/`, `build-ninja/`, `build-asan/` exist + compile.
- **TI gospel:** `recon/PLAN_threat_interactive_gospel_2026-07-16.md` STALE vs Video 29 (deferred-MSAA Crysis3, "MSAA was NOT the bottleneck"). Plan G2.5 refreshes (add R14). Crysis 3 = real game (Crytek 2013 FPS) — legit reference.
- **Memory:** 8 entries, 91% usage. Key: ZE-SPEC (625 live), SYSTEMS COUNT v2 (107 curated/58 orphans/10 missing sub-milestones), BRANCH/WORKTREE, WHOLE-PROJECT SCOPE, TI GOSPEL UPDATE, BEST PLAN FORMAT.

---

## 4. LESSONS THIS SESSION (so new session doesn't repeat)

- **Regex discipline:** count EXT systems with H4 `#### [Mx-EXT-NN]` headers ONLY (matches `verify_m0_parity`). Bare body-mention `[Mx-EXT-NN]` over-counts (the "583" vs "107" bug). Always H4-vs-H4.
- **"627" was wrong** (live=625). "161" was wrong (counted old_milestones dupes). "583" was wrong (body-mention inflation). **107 curated is the verified number.**
- **Path typo trap — CRITICAL:** Windows is CASE-INSENSITIVE on paths, so `.hermis` and `.hermis` are the SAME directory. Do NOT `rm -rf` a typo'd variant — it deletes the real dir. The correct Hermes profile plans dir is `.hermis/plans/` (WITH r). This session an `rm -rf .hermis` destroyed the plans dir; recreate it and re-write the plans from chat content. NEVER rm a hermes dir without absolute certainty of the exact path.
- **User directives (permanent):** be autonomous EXCEPT milestone-spec edits (need GO); fact-check 3 ways; assume-you're-wrong + disprove-3-ways (debug-5 if one fails); batch commits ≤3 files; archive don't delete; don't make shit up.
- **STOP-AND-ASK rule:** if a tool/integration fails, STOP and ask — don't silently workaround (Bing ban enforced this way).

---

## 5. NEXT SESSION STARTER (paste or just say "continue")

"Continue ZombieEngine spec parity. Read `.hermis.md` + `recon/plans/2026-07-16_Production-Ready-Parity-Plan.md` + `recon/plans/2026-07-16_Systems-Coverage-Plan.md`. Verify git state, then await my GO on the 6 open decisions. Do NOT edit `spec/Mx.md` without explicit approval."

---
*Endpoint written by Jenny. All numbers above verified against live disk + real sources this session. No fabricated counts. NOTE: plans dir was accidentally deleted via rm -rf on a case-insensitive path typo and recreated; Systems-Coverage + Production-Ready + this endpoint re-written from session chat content. The older spec-m0-parity-reformat.md and phase9-consolidation.md from prior sessions were NOT recoverable (never git-tracked) — regenerate if needed.*
