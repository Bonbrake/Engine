# ZombieEngine Audit Trail — Index

This folder is the **permanent milestone audit record** for ZombieEngine. Per
`AUDIT_HANDOFF.md`, these files are never deleted — they are the evidence trail
for findings, fixes, and open questions across M0–M13. Update in place; do not
prune. This README is the map; the individual files are the source of truth.

> **Convention:** every "DEAD" / "0 code" / "PARTIAL" claim in these docs is
> backed by a literal grep command + its actual output (no summary-of-a-search
> passes as evidence). Treat commit hashes as **asserted-until-eyes-on** — the
> original auditors could not independently inspect the repo; verify against
> `git show <hash>` before relying on a claim.

## Status legend
- ✅ RESOLVED — fix applied + verified (build/test/run evidence in-file)
- 🔶 OPEN — finding stands, deferred pending go-ahead
- 📋 RECORD — informational paper trail, no action item

## Files

| File | Covers | Status | Key refs |
|---|---|---|---|
| `AUDIT_HANDOFF.md` | Starting point for next audit session; sanitiser mapping (RESOLVED), open item M0-EXT-15/16/17 | 📋 + 🔶 | `26a91cc`, `5125d8c`, `e5b40f6` |
| `AUDIT_FOUNDATION_GAP.md` | **Canonical** M0/M1 vs M2–M13 forward-dependency audit (637 lines). Every system M2–M13 assumes exists, checked against the substrate. | 📋 | Vulkan-Loader #384 |
| `AUDIT_M0-M2.md` | Raw M0–M2 findings log (Finding #1 null-device segfault, #2 spurious WARN). Superseded in detail by `AUDIT_FOUNDATION_GAP.md` + `AUDIT_FIX_STATUS.md` — kept as the original log. | ✅ | `5125d8c`, `e5b40f6` |
| `AUDIT_FIX_STATUS.md` | Build/test/run evidence for the M0–M2 fixes (17 deps restored, 7 test cases pass). | ✅ | build `BUILD_EXIT=0` |
| `AUDIT_M0.md` | M0 audit — Vulkan 1.4 native bootstrap + capability tiering. | 📋 | branch `m2/physics-destruction` |
| `AUDIT_M1.md` | M1 audit — GPU-driven ECS framework. | 📋 | |
| `AUDIT_M2.md` | M2 audit — Jolt 5.5.0 physics, EventBus, destructible test entity. | 📋 | |
| `AUDIT_M1_SCENE_RENDER_GAP.md` | **MAJOR FINDING:** M1 exit criteria not met — scene renderer was never built; M2 verified on top of the gap. Review deliverable, fixes deferred. | 🔶 | |
| `AUDIT_TRIANGLE_RENDERER_EXT03_MISLABEL.md` | `TriangleRenderer.cpp:486/751/785` mislabeled legacy occlusion code as `[M1-EXT-03]` (spec defines EXT-03 as Command Pool Matrix). | 🔶 | shares surface with FOUNDATION_GAP T2 |
| `AUDIT_DEV_MODE_VALIDATION_BASELINE.md` | Validation-layer error baseline observed at dev-mode launch. | 📋 | |
| `endless_quarantine_v77_cleaned.md` | **Source spec (not an audit):** v77 real-world constant-grounding pass — physics/physiology constants verified against literature, 2 magnitude bugs fixed. 9,895 lines. | 📋 | supersedes `endless_quarantine_v76*` |

## Open items (do not lose)
1. **M0-EXT-15/16/17 + "Remediation Directive"** — never verified against the
   spec; may be *fabricated scope / spec drift*. Per `AUDIT_HANDOFF.md` §3, grep
   the spec for these IDs first; if absent, lead with "Major Finding: Fabricated
   Scope." Unresolved as of last audit session.
2. **M1 scene renderer gap** (`AUDIT_M1_SCENE_RENDER_GAP.md`) — no ECS-driven
   geometry path exists; all windowed "scenes" are hardcoded demo data. M2's
   physics/ECS/EventBus work is independently real and NOT cast into doubt.
3. **TriangleRenderer EXT-03 mislabel** — spec/code tag collision; reconcile
   before any renderer work claims M1-EXT-03 coverage.

## How to use this trail
- Starting a new audit? Read `AUDIT_HANDOFF.md` first, then
  `AUDIT_FOUNDATION_GAP.md` for the canonical findings.
- Looking for a fix's proof? `AUDIT_FIX_STATUS.md` has the build/test/run
  evidence; commit hashes are in `AUDIT_M0-M2.md`.
- Adding a new finding? Create `AUDIT_<topic>.md` here, then add a row above and
  a status tag. Never delete an existing file.
