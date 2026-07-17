# ZombieEngine Spec — Production-Ready Parity Plan

> **For Hermes:** bite-sized tasks, exact paths, verify each gate. DRY/YAGNI/TDD. Stage-gate: do NOT advance a phase until its gate passes.

**Goal:** Bring the entire ZE spec corpus (625 EXT blocks across M0–M13 + M4.5 + APPENDICES) to verified M0-parity production quality, commit cleanly, reconcile branches, lock naming — with documented scope (in AND out) so nothing is silently missed.

**Architecture:** M0 is the gold-standard format (H4 `#### [Mx-EXT-NN]` blocks, 5-subsection skeleton, TOC + 4 `###` Clusters, per-block `tags/tl;dr/meta/ctx`, `<a id>` anchors, YAML frontmatter, `Mx.index.json` sidecar). The verifier `scripts/verify_m0_parity.py` is the gate. Additive `name:` codename field (not rename) preserves 625 block refs.

**Whole-project scope (ground truth this session):** ZE is a LIVING Vulkan 1.4 engine, not a spec doc. `src/` has 13 modules (render 35 / core 24 / ecs 10 / physics 4 / ai,audio,debug,events,modding,net,save,slm,tests,ui,world 1 each). `CMakeLists.txt` builds `ZombieEngine` exe + `ZombieEngineTests` (Catch2, `catch_discover_tests`). Two build dirs exist and COMPILE NOW (`build/`, `build-ninja/`, `build-asan/`). `assets/` (fonts/models/prefabs/textures) + `tests/` + `recon/transcripts/` (11 TI videos). Rendering is HARD-CONSTRAINED by Threat Interactive gospel (`recon/PLAN_threat_interactive_gospel_2026-07-16.md`) — but that gospel is STALE vs TI Video 29 (deferred-MSAA pipeline, "MSAA was NOT the bottleneck"). The spec is the *design contract*; the engine is the *implementation*; both must stay in lockstep.

**Tech Stack:** Python 3.11 (scripts/verify_m0_parity.py, recon/*.py), Git (Gitflow: feature→beta→main, tag per milestone), VS2022 + CMake (build gate), free curl+Brave web (recon/web_search.py).

---

## Scope Statement (prevents missed scope — LU3)

**IN (this plan):**
- Commit verified work: M1 placeholders, 16-file `name:` field, 3 recon scripts, plans.
- Extend parity verification to ALL 15 other milestones (same depth-audit M1 got).
- Lock naming scheme (additive `name:` + legend in llms.txt/AGENTS.md).
- Reconcile branches (main 114 ahead, beta 91 ahead) + 3 subagent worktrees before merge.
- Build gate (cmake + Tests.exe) before "done" — engine COMPILES NOW (build/, build-ninja/ exist).
- Resolve strays: `.hermis.md` edit, M0-EXT-21..25 dangling.
- **Refresh Threat Interactive gospel to Video 29** (deferred-MSAA pipeline finding; current gospel pre-dates it). Rendering is a HARD CONSTRAINT on the engine — spec and engine must stay in lockstep.
- **Engine sprint framing:** align spec-parity milestones with a playable-build milestone (game-dev best practice: weight milestone payments toward end, align sprints with playable build).

**OUT (explicitly excluded — YAGNI):**
- No file/ID rename of milestones or `Mx-EXT-NN` blocks (preserves refs; user "make a place holder" not "rename everything").
- No new heavy citation-graph tool (verifier + grep suffice at this scale; 625 blocks, not 6M).
- No spec CONTENT rewriting/fact-check beyond parity structure (user waived M0 fact-check; reformat only).
- No modding-engine feature work — spec-only + gospel refresh (no new renderer code this plan).
- No CI pipeline setup (local verifier + build gate is the gate).

---

## Stage Gates (LU2: creeping commitment — gate must PASS before next phase)

| Gate | Pass condition |
|---|---|
| G0 Branch reconcile | main/beta divergence (main 114 ahead, beta 91 ahead) + 3 worktrees inventoried; merge order decided |
| G1 Commit batches | each batch ≤3 files; `verify_m0_parity.py` → all OK, 0 FIX |
| G2 Parity depth | all 15 non-M1 milestones pass same depth-audit M1 did |
| G2.5 TI gospel | gospel refreshed to Video 29 (deferred-MSAA); engine renderer still complies |
| G3 Naming lock | 16 `name:` present; legend added; verifier still OK |
| G4 Build gate | `cmake --build` rc=0; Tests.exe 17 pass/1 skip |
| G5 Merge | PR→beta→main; post-merge verifier == pre-merge |

---

## Tasks (bite-sized, 2–5 min each)

### Phase 0 — Branch reconcile (Gate G0)
- **T0.1** Inventory divergence: `git rev-list --count main..HEAD` and `beta..HEAD` (expect 114 / 91); `git worktree list` (expect 3 subagent worktrees @804cb55).
- **T0.2** Decide merge order: this branch → `beta` (tag `parity-m0`) → `main`. Write decision to plan.
- **T0.3** For each subagent worktree: `git log` last 5; decide merge or close. Record.

### Phase 1 — Commit verified work (Gate G1)
- **T1.1** Commit `.hermis.md` alone (stray 5-line rules note). Verify: `git status` shows only this.
- **T1.2** Commit A: `spec/M1.md` + `spec/M1.index.json`. Run verifier → all OK.
- **T1.3** Commit B: 15 `name:` files (M0, M2–M13, M4.5). Run verifier → all OK.
- **T1.4** Commit C: `recon/add_m1_placeholders.py`, `recon/add_milestone_names.py`, `recon/web_search.py`.
- **T1.5** Commit D: `recon/PLAN_m1_reconcile_placeholders_2026-07-16.md` + this plan.
- **T1.6** Commit E (your call): `archive/PLAN_outstanding_work_2026-07-16.md` (track) or leave disk-only.

### Phase 2 — Parity depth audit, all milestones (Gate G2)
- **T2.1** For M0, M2–M13, M4.5: run the SAME weld/fence/empty/5-subsection scan M1 got. (Reuse `recon/diag_m1.py` pattern per file — read-only, no new heavy tool.)
- **T2.2** For any gap found: write a small fix script (like `author_m1.py`) OR mark STUB-exempt. Verify per file.
- **T2.3** Produce parity scorecard: per-milestone `blocks | 5-sub OK | STUB | dangling`. Target: 0 dangling.

### Phase 2.5 — Refresh Threat Interactive gospel (Gate G2.5)
- **T2.5.1** Pull TI Video 29 + recent transcripts via `recon/pull_ti_transcripts.py` (or web); extract deferred-MSAA / Crysis3 pipeline finding.
- **T2.5.2** Patch `recon/PLAN_threat_interactive_gospel_2026-07-16.md` to add R14 (deferred-MSAA: "MSAA was NOT the bottleneck") — additive, not overwrite.
- **T2.5.3** Verify engine renderer (`src/render/`) still complies with gospel + R14 (grep anti-TAA / MSAA-prepass usage). Flag any drift.

### Phase 3 — Lock naming (Gate G3)
- **T3.1** Confirm `grep -cE '^name:' spec/*.md` == 16.
- **T3.2** Additive legend line to `spec/llms.txt` + `spec/AGENTS.md` (name map). No file rename.
- **T3.3** Run verifier → all OK.

### Phase 4 — Build gate (Gate G4)
- **T4.1** `cmake --build build --config Debug` → expect rc=0.
- **T4.2** Run `Tests.exe` → expect 18 cases, 17 pass / 1 skip headless.
- **T4.3** If build breaks: STOP, flag, do not claim done.

### Phase 5 — Merge + tag (Gate G5)
- **T5.1** PR `spec/m0-parity-reformat` → `beta`; tag `parity-m0`.
- **T5.2** `beta` → `main`. Post-merge: re-run verifier + scorecard on `main`.

### Phase 6 — Strays + debt
- **T6.1** `.hermis.md` already committed (T1.1). Confirm clean.
- **T6.2** M0-EXT-21..25 dangling: investigate source, define or cross-link, file as Issue.
- **T6.3** Archive stale plans per "always archive unless huge."

---

## Files likely to change
- Commit: `spec/M1.md`, `spec/M1.index.json`, `spec/M0,M2..M13,M4.5.md`, `recon/*.py`, `archive/`, `.hermis.md`
- Audit (Phase 2): `spec/M0,M2..M13,M4.5.md` (fix scripts, autonomous)
- Naming (Phase 3): `spec/llms.txt`, `spec/AGENTS.md` (additive)
- No `build/`, `vcpkg_installed/` ever committed.

## Verification (per gate)
- G1/G3: `python scripts/verify_m0_parity.py` → all `OK`, 0 `FIX`.
- G2: per-milestone depth scan; scorecard 0 dangling.
- G4: `cmake` rc=0; `Tests.exe` 17/1.
- G5: post-merge verifier == pre-merge.

## Risks / tradeoffs
- Branch divergence = logical conflicts possible (Gitflow: no workflow avoids reconciling). Mitigate: G0 inventory first.
- Build gate may expose pre-existing compile issues unrelated to spec — flag, don't block spec commit on them.
- Naming additive (not rename) trades "pretty filenames" for zero ref-breakage. User-approved.

## Open questions
1. Phase 0 GO — reconcile before spec edits?
2. T1.6 archive plan — track in git or disk-only?
3. Phase 2 — authorize per-milestone fix scripts?
4. Phase 4 build gate — run it (compiles ~minutes)?
5. Memory — retry persisting Web-default/stop-and-ask/Bing-ban (failed 4× earlier)?

---
*Generated by Jenny. Research-backed (LU1 TOC+scope, LU2 stage-gate, LU3 scope-exclusions). Re-verify each gate before advancing. NOTE: this file was re-created after an accidental `rm -rf` on a case-insensitive path typo deleted the plans dir; content reconstructed from session chat.*
