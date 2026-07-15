# ZombieEngine — Execution Specification (Master Index)

> **Status:** 707 buildable systems. 0 duplicate definitions · 0 incomplete blocks · 0 malformed code · 0 placeholder math · 0 dangling references. 3 retired records archived in `_RETIRED.md` (excluded from build scope).
> **Scope:** Vulkan + EnTT ECS, double-precision transform. Hybrid horde-shooter + Project-Zomboid survival. Realistic-but-fun, 100% procedural, cohesive, 1st/3rd person. Offline SLM (MiniCPM5-1B) integrated across systems. Co-op PvE+PvP: local split-screen + online, 4 players default (up to 16). References: The Last of Us Part I, Metro Exodus.

---

## 1. Engineering Principles (binding)

1. **Order is contractual.** Build only the active phase. A system may be implemented once its stated prerequisites are GREEN. No cross-phase parallelism.
2. **Lock foundational contracts in Phase 0; never mutate them after.** ECS component layout, `glm::dvec3` world transform, spatial-hash key, RNG stream, fixed-timestep simulation. Post-lock changes are mass rework.
3. **Single block per build + verify.** Implement → compile (`cmake --build build-asan --config Release --target ZombieEngine`) → headless smoke (`ZombieEngine.exe --headless --quit-frame 45` → `HEADLESS CI SMOKE TEST: SUCCESS`) → mark complete.
4. **Performance is a gate, not a goal.** Floor: 6 GB VRAM, 1080p, 60 fps. Levers: M0-EXT-40 dynamic resolution, M4.6-EXT-05/06 GPU cull + indirect draw, M12-EXT-12 interest management.
5. **Determinism is enforced, not assumed.** Co-op simulation state is fixed-point (M12-EXT-06); cross-machine lockstep validated by the CI replay gate (M12-EXT-18). Floating-point alone is not deterministic across CPUs.
6. **Realism serves fun.** Survival needs are meaningful but never "uselessly cumbersome" (M2.9-EXT-40 realism-fun floor).
7. **Co-op is fair by default.** Shared loot (M12-EXT-19), friendly fire off in PvE (M12-EXT-20), PvP opt-in + zone-confined (M12-EXT-21), no-team-sabotage balance (M12-EXT-22).
8. **Stop and report on build failure. Never fabricate verification output.**

---

## 2. Dependency Contracts (why this order prevents system conflict)

- **ECS + Transform (M1, M2-EXT-27)** are the spine. All state is EnTT components; defined first, frozen after Phase 0.
- **Determinism (RNG M0, Jolt defines) before AI / world-gen.** Reproducible horde + procedural generation.
- **Render path (M4.6 cull/draw, M0-EXT-40) before world-gen (M4).** World must be cheaply viewable to be testable.
- **Physics (M2.6/M2) before AI (M5).** AI reads physics transforms + raycasts the physics world.
- **World-Graph Backbone (M4-EXT-74) before POIs/factions/encounters.** All attach to it; nothing attaches to nothing.
- **Survival sim (M2.9) is pure data** — isolated from render, safe post-Phase 0.
- **SLM (M13) is a pure consumer** of world-state snapshots — never blocks gameplay.

---

## 3. Build Sequence

| Phase | Objective | Prerequisites | Key Systems | Exit Gate |
|------|-----------|---------------|-------------|-----------|
| **0** | Foundation + walking skeleton | none | M0-EXT-01, M1-EXT-10/11/12, M2-EXT-27/28, M2.7-EXT-33/34, M2.7-EXT-19, M8.7-EXT-02/03, M4.6-EXT-05/06, M0-EXT-40 | Move/look/reload (fumbling)/skill-rise visible; 60 fps on floor |
| **1** | World exists (procedural, connected) | P0 + render path | M4-EXT-10/11/08/09, M4-EXT-74, M4-EXT-64..95, M2.6 | Connected explorable map generates + walkable at 60 fps |
| **2** | Threat & combat | P1 + physics stable | M5.4-EXT-05/07/12, M5.3-EXT-08, M2.7-EXT-30/27, M2-EXT-24/25, M3-EXT-09 | Zombies hear/follow/swarm; bait + fight; 100+ agents holds fps |
| **3** | Survival pressure | P0 (data-isolated) | M2.9-EXT-09/10/26/29/30/31/32..40, M2-EXT-22/26/21, M8-EXT-09/16/10/17/18, M4-EXT-57, M2.9-EXT-19/20 | Eat/manage weight/craft; can die from neglect; no render coupling |
| **4** | Living world | P1 (graph) + P2 (threat) | M8.6 (safehouse/turf/rep/vendors/civil war), M5.4-EXT-16/17/18, M5.1-EXT-09/10, M4-EXT-63/75..86, M13-EXT-53 | Factions fight; world reacts to deeds; companion follows; content-drought guard active |
| **5** | Frame / UI / Audio / SLM | P0–P4 | M6 (+M6-EXT-09/10), M6.5, M11-EXT-52..65, M13, M10 | Looks/sounds alive; SLM narrates from real world-state; no gameplay logic in SLM |
| **6** | Persist & co-op & evolve | all prior | M7+M7-EXT-17/18, M9+M9-EXT-22, M12-EXT-01..22, M4-EXT-59/90..95, M5.4-EXT-13/15/20..25 | Save/load; local-split + online co-op (4, scalable 8/16); host migration; prediction; world evolves forever |

---

## 4. Cohesion Model

- Every POI / faction / encounter / route = a node/edge on **M4-EXT-74** World-Graph (see `_CROSSLINKS.md`).
- Thrown-object = ANY small world object (M2.7-EXT-30), real physics. Shiv = crafting recipe (M8-EXT-16). Listen Mode + Cover System = removed per design decision.
- SLM (M13) plugs into: missions, broadcasts, diary, rumors, charters, tombstones, epitaphs, wanted posters, barker lines, scavenge memory, dead memory, danger music, tattoos/brands — all fed from world-state snapshots.
- Skill-by-use (M8.7-EXT-02): reload/melee/lockpick/throw/aim/medicine/craft/drive level by doing. Terrible start via Fumble Director (M8.7-EXT-03). Perks (M2-EXT-21) raise ceilings only.

---

## 5. Verification Methodology

- **Static:** full block audit (5-section completeness, code-fence pairing, duplicate-ID definition check, dangling cross-reference check, placeholder-math check).
- **Build:** MSVC Release build of `ZombieEngine` + `ZombieEngineTests` → exit 0.
- **Runtime:** headless smoke (`--headless --quit-frame 45` → SUCCESS) + Catch2 suite (17 cases passing; co-op-specific suites added per Phase 6).
- **Netcode:** deterministic replay gate (M12-EXT-18) — same seed + inputs must produce bit-identical fixed-point state across two runs; CI-enforced.
- Each phase ends GREEN on all four before the next begins.

---

## 6. Archived

- `_RETIRED.md` — 3 superseded/audited records (M0-EXT-14, M1-EXT-26, M4.5-EXT-04). Excluded from build scope and from the 707-system count.

---

**Default start command:** `start phase 0` → foundation + walking skeleton, each block verified with a real MSVC build.
