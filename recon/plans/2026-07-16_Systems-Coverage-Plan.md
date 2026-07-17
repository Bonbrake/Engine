# ZombieEngine — Systems Coverage Plan (live spec vs 10 archives)

> **For Hermes:** bite-sized, exact paths, verify each gate. DRY/YAGNI/TDD. Fact-check ONLY against real sources (live spec/, 10 archive units) — never invent a system.

**Goal:** Confirm ZE has ALL its spec'd systems, by mapping live `spec/` EXT blocks against **10 archive units (early→latest)**, flagging any system in archive but NOT live (provisional, not ported) — without fabricating anything.

**Ground truth (H4-header vs H4-header, matches `verify_m0_parity`, re-verified 2026-07-16):**
- **Live spec = 625 EXT blocks** (H4 `#### [Mx-EXT-NN]` headers; matches the verifier exactly).
- **Actual engine = 13 `src/` modules** (render 35, core 24, ecs 10, physics 4, ai/audio/debug/events/modding/net/save/slm/tests/ui/world 1 each).
- **10-archive sweep (early→latest):** ARCHIVE H4 union = 586, of which **165 NOT in live** = **107 curated provisional** (master `MASTER_PLAN_ENDLESS_QUARANTINE.md` + `IDEA.md`/`docs/audit`, H4) + **58 earliest-milestone-only orphans** (renumbered/dropped, not in master). Spans **10 ENTIRELY-MISSING dotted sub-milestones** (H4 in archive, absent in live): M2.6, M2.7, M2.9, M4.6, M5.1, M5.2, M5.3, M8.5, M8.6, M8.7 (107 blocks). Live spec tops at M13 + M4.5.
- `MASTER_PLAN_ENDLESS_QUARANTINE.md` = 1482 EXT refs, changelog v75→v78.1. `IDEA.md` = 1091. `docs/audit` ≈ `IDEA.md` (don't double-count). `phase1_verify` + `runtime_junk` = 0 systems (junk).
- **Crysis 3 = real game (Crytek 2013 FPS)** — TI studies its renderer as gospel reference (confirmed Wikipedia/X).

**Conclusion:** We do NOT yet have all systems in live spec. Archive is AHEAD by 107 curated provisional (across 10 missing sub-milestone files) + 58 earliest-only orphans. Plan = port-audit, not invent.

---

## Scope (IN / OUT)

**IN:**
- Sweep 10 archive units (early→latest), count H4 EXT systems, diff vs live (625).
- For each curated: confirm REAL in master/IDEA (cite block), classify (port-now / defer / drop).
- Flag 10 entirely-missing sub-milestone files.
- Correct memory: "627"→625; retire "583" over-count (107 curated is correct).
- Coverage scorecard: `live | curated | orphans | ported | deferred | dropped`.

**OUT (YAGNI — do NOT):**
- Auto-invent or auto-port systems. Audit + flag only.
- Edit live `spec/Mx.md` (separate GO-gated plan).
- Rewrite engine code.
- New heavy citation-graph tool (H4 grep/diff suffices at 625+165 scale).

---

## Stage Gates
| Gate | Pass condition |
|---|---|
| G0 Inventory | live=625 confirmed; 10-unit sweep done; 107 curated + 58 orphans; 10 missing sub-milestones listed |
| G1 Realness | every curated ID traced to a real master H4 block (cite), 0 invented |
| G2 Classify | each 107 classified port-now/defer/drop with reason |
| G3 Scorecard | scorecard printed; memory "627"→625, "583"→107 fixed |

---

## Tasks (bite-sized)

### Phase 0 — 10-Archive Sweep (Gate G0)
- **T0.1** Count live H4 EXT (expect 625).
- **T0.2** Sweep 10 units H4-vs-H4: `milestones_M0-M13_antigravity` (earliest), its 3 backups (155628/160150/161150), `MASTER_PLAN_ENDLESS_QUARANTINE.md`, `IDEA.md`, `ARCHIVE_2026-07-15_cleanup.md`, `docs/audit`, `phase1_verify`, `runtime_junk`. Diff each vs live.
- **T0.3** List 10 entirely-missing sub-milestone files (confirm no `spec/Mx.y.md` exists).

### Phase 1 — Realness check (Gate G1)
- **T1.1** For each of 107: `grep -l "\[ID\]" archive/docs/*.md` → must hit master/IDEA/audit. 0 misses = pass.
- **T1.2** Spot-audit 10 random IDs: open master H4 block, confirm 5-subsection skeleton. Record.

### Phase 2 — Classify (Gate G2)
- **T2.1** Port-now: IDs whose live milestone EXISTS but block missing (expanded M11/M12/M13). List.
- **T2.2** Defer: IDs in master-only sub-milestones (M2.6/2.7/2.9/M4.6/M5.1/5.2/5.3/M8.5/8.6/8.7) — need new milestone file first. List.
- **T2.3** Drop: IDs dupes/obsolete per master changelog v75→v78.1. List with reason.

### Phase 3 — Scorecard + memory fix (Gate G3)
- **T3.1** Print: `live=625 | curated=107 | orphans=58 | port-now=N | defer=M | drop=K`. N+M+K≈107.
- **T3.2** Fix memory: "627"→625; "583" over-count retired (107 curated correct).

---

## Files
- Read-only: `spec/M*.md`, `archive/old_milestones/*`, `archive/docs/*`
- Output: `recon/SYSTEMS_COVERAGE_2026-07-16.md`
- NO edit to live `spec/` (separate GO-gated plan if porting approved)

## Verification (per gate)
- G0: counts 625 / 107 / 58 reproduced by re-running H4 sweep; 10 missing files confirmed absent.
- G1: 107/107 traced to master; 10/10 spot-audited have skeleton.
- G2: every ID in exactly one bucket; reasons cited.
- G3: N+M+K == 107; memory entries corrected.

## Risks
- "627" memory wrong (625 real) — fixed. "583" was over-count (body-mention vs H4) — retired; 107 curated is correct.
- Archive 107/58 ≠ "lost" — PROVISIONAL/renumbered, not ported. Don't panic-invent.
- `docs/audit` ≈ `IDEA.md` — don't double-count.
- Master changelog v75→v78.1 fixed bugs/grounded constants — if porting later, use master v78.1 text.

## Open questions
1. Port-now list — merge into live spec (needs GO-gated plan)?
2. 10 missing sub-milestones — create new milestone files, or fold into M2/M4/M5/M8?
3. Memory "627"→"625", "583"→"107" — confirm correction?

---
*Generated by Jenny. 10-archive sweep early→latest, H4-vs-H4 (matches verify_m0_parity), fact-check-efficient: only grep/diff vs real sources, no invented systems. Verified ground truth: live=625, curated=107 (+58 orphans), 10 missing sub-milestones, master ahead.*
