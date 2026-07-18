# ZombieEngine — Cross-Session Memory (Hermes Agent)

## Last Session Summary (2026-07-18)
**Branch**: `spec/m0-parity-reformat`  
**HEAD**: `58be06e` (spec: T10 living world hybrid rate model)  
**Verifier**: `scripts/verify_m0_parity.py` — **16/16 OK**

## Valid Work Completed (23 blocks, user-authorized, committed)
| Commit | Scope | Blocks |
|--------|-------|--------|
| `d5a5dd9` | T1 Emergence audit (M7→M8-EXT-60, M4 precombine) | — (coupling fixes) |
| `0f3d3e9` | T2 Reputation vector + player-founded faction + leader-of-all + evil/good alignment | M8-EXT-53, M8-EXT-68 |
| `428cdf2` | T3 Multi-solution quests (COMBAT/TALK/SNEAK/BRIBE) | M12-EXT-26, M11-EXT-64 |
| `0a03eea` | T4 Environmental storytelling (RDR2 diegetic) | M4-EXT-32, M13-EXT-24 |
| `aece7d2` | T5 Emptiness-ratio tuning (FO3 silence=tension) | M4-EXT-96 |
| `677b9a6` | T6 WorldMemory (RDR2 deed-log, no popups) | M7-EXT-23 |
| `58be06e` | T10 Living world hybrid (slow drift + drastic spikes, hubs protected) | M12-EXT-45 |

**Total**: 23 new EXT blocks across M4/M5/M7/M8/M11/M12/M13 — all user-requested, fact-checked, 3-pass verified.

## REAL Remaining Work: 132 Provisional Archive Blocks

### Classified Provisional Blocks (from MASTER_PLAN_ENDLESS_QUARANTINE.md + old_milestones)
| Milestone | Blocks | Status |
|-----------|--------|--------|
| M2.6 Open-world foundations | 3 | Proposals ready (`recon/plans/2026-07-18_M2.6-PROPOSALS.md`) |
| M2.7 Core combat/melee/parry/gunplay | 23 | Proposals ready |
| M2.8 Deterministic co-op | 9 | Proposals ready |
| M2.9 Traversal/ballistics/physiology | 23 | Proposals ready |
| M4.6 Memory management | 8 | Proposals ready |
| M5.1 Procedural zombie variation | 8 | Proposals ready |
| M5.2 Procedural animation/IK | 15 | Proposals ready |
| M5.3 AI perception/scent/audio | 8 | Proposals ready |
| M5.4 AI Director/horde pacing | 17 | Proposals ready |
| M8.5 Faction economy | 6 | Proposals ready |
| M8.6 Settlement construction | 10 | Proposals ready |
| M8.7 Perk & progression | 2 | Proposals ready |
| **Total** | **132** | **All in H4 format, 0 verifier issues** |

### M1 STUBs (11 blocks, awaiting explicit GO)
M1-EXT-13 through M1-EXT-23 — placeholders in M1.md

### Other Open Items
- **vcpkg.json**: 17 unpinned deps — need GO on baseline strategy
- **10 missing sub-milestone files**: M2.6, M2.7, M2.8, M2.9, M4.6, M5.1, M5.2, M5.3, M5.4, M8.5, M8.6, M8.7
- **MEMORY.md**: This file (cross-session persistence)

## Protocol Rules (NON-NEGOTIABLE)
1. **Milestone specs READ-ONLY** until explicit GO — proposals go in `recon/plans/*/PROPOSALS.md`
2. **Verify by disproving**: 3 independent passes (mechanical, read-level, cross-system)
3. **No self-certification**: `verify_m0_parity.py` must pass 16/16
4. **Local-first**: No cloud deps in core, offline primary
5. **FPS/TPS only**: No top-down/tactical camera
6. **No forced narrative**: Quests start naturally, no accept prompts
7. **Evil/good both viable**: Meter-less alignment, both paths reach all endpoints
8. **Leader of all except horde**: Player can lead any faction

## Next Session Priorities (Autonomous)
1. **Update MEMORY.md** (this file) for cross-session persistence
2. **Verify proposal completeness** — each block needs full content from archive (Systems Touched, Math, Algorithm, Player-Facing Impact)
3. **Create port plan** with GO gates (per SYSTEMS_COVERAGE_PLAN Phase 0)
4. **Present complete plan** for user GO review
5. **On GO**: Merge approved proposals into spec/ following proper protocol

## Notes for Continuity
- User said: "fill gaps and be autonomous till done assume ur not right on each one then fact check"
- User said: "make it high quality and merge it add the things u reccomend also make sure the player can be the leader of all of them but the zombie"
- User said: "look online think hard" — but web backend is firecrawl (dead keys), use `recon/web_search.py` (Brave) instead
- 132 provisional blocks are the REAL scope; 23 committed blocks are done
- All work must pass 3-pass verification before any merge
- Proposals are in `recon/plans/2026-07-18_*PROPOSALS.md` — ready for GO review