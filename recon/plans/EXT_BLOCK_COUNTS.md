# EXT Block Count Verification
> Generated from spec/*.md as source of truth; .index.json mirrors MD.

| Milestone | JSON count | MD count | True max | Status |
|-----------|-----------|----------|---------|--------|
| M0 | 62 | 62 | 111 | OK |
| M1 | 43 | 43 | 101 | OK |
| M2 | 127 | 127 | 127 | OK |
| M3 | 37 | 38 | 104 | REGEN |
| M4 | 91 | 91 | 109 | OK |
| M4.5 | 51 | 51 | 118 | OK |
| M5 | 98 | 98 | 108 | OK |
| M6 | 30 | 30 | 108 | OK |
| M6.5 | 13 | 13 | 13 | OK |
| M7 | 24 | 24 | 101 | OK |
| M8 | 84 | 84 | 104 | OK |
| M9 | 35 | 35 | 35 | OK |
| M10 | 27 | 27 | 31 | OK |
| M11 | 60 | 60 | 105 | OK |
| M12 | 38 | 38 | 103 | OK |
| M13 | 43 | 43 | 101 | OK |
| M2.6 | 9 | 9 | 9 | OK |
| M2.7 | 25 | 25 | 34 | OK |
| M2.8 | 10 | 10 | 10 | OK |
| M2.9 | 34 | 34 | 34 | OK |
| M4.6 | 7 | 7 | 9 | OK |
| M5.1 | 8 | 8 | 8 | OK |
| M5.2 | 15 | 15 | 16 | OK |
| M5.3 | 8 | 8 | 8 | OK |
| M5.4 | 14 | 14 | 17 | OK |
| M6.5 | 13 | 13 | 13 | OK |
| M8.5 | 4 | 4 | 4 | OK |
| M8.6 | 25 | 25 | 28 | OK |
| M8.7 | 3 | 3 | 3 | OK |

**Total JSON**: 1038
**Total MD**: 1039

## Warnings
- M3: JSON count 37 != MD count 38; regenerating .index.json

## Regenerated JSON sidecars

- M3

## Sample titles

| Milestone | Block | Title |
|-----------|-------|-------|
| M0 | M0-EXT-1 | Systems Touched |
| M0 | M0-EXT-2 | M2.8/M12 co-op — extends the existing Aftermath/RGD crash-dump path |
| M0 | M0-EXT-3 | M0/M1 render. TriangleRenderer::draw (TriangleRenderer.cpp:688-719) returns early on bridgeActive and draws per-entity vkCmdDrawIndexed+vkCmdPushConstants (:713 |
| M1 | M1-EXT-1 | M1-EXT-09. Only Transform snapshot tested (Engine.cpp:391-412); other hot components not cached. |
| M1 | M1-EXT-2 | M1/M5/M6/M9. grep RayBatch → 0. Jolt CastRay collector = multi-hit-per-ray, not N independent rays. |
| M1 | M1-EXT-3 | M1. QueryRadius/QueryCell defined (SpatialHash.cpp:151,179) but only called from unit test (Engine.cpp:651-664). |
| M2 | M2-EXT-1 | M2.9. BulletComponent exists but grep projectile|ricochet|penetration → only data fields, no solver. |
| M2 | M2-EXT-2 | M8.7. PerkPoints exists, no system. |
| M2 | M2-EXT-3 | Deepens M2.7 core gunplay mechanics and utilizes your M8 data-driven JSON caliber variables. |
| M3 | M3-EXT-1 | M2.6/M3. grep Terrain|Foliage → 0 (false positives in Engine). Open world has no ground. |
| M3 | M3-EXT-2 | M4/V. Every POI reachable from roads. |
| M3 | M3-EXT-3 | M4/T98. 10k seeds gen in CI to catch rare crashes. |
| M4 | M4-EXT-1 | Systems Touched |
| M4 | M4-EXT-2 | M4. Monolithic gen → layered passes consuming neighbor context (LayerProcGen/EPC2024). |
| M4 | M4-EXT-3 | M4/M11. Declares passes; mods insert/reorder. |
| M4.5 | M4.5-EXT-1 | Systems Touched |
| M4.5 | M4.5-EXT-2 | Systems Touched |
| M4.5 | M4.5-EXT-3 | Distant static meshes (buildings, ruined vehicles) currently pop or draw full geometry. Generates octahedral-impostor atlases per mesh at bake time; sampled bey |
| M5 | M5-EXT-1 | Systems Touched |
| M5 | M5-EXT-2 | Systems Touched |
| M5 | M5-EXT-3 | Systems Touched |
| M6 | M6-EXT-1 | SoLoud (easy, light); positional + occlusion via `RayBatchQuery` (M1-002). |
| M6 | M6-EXT-2 | Per-material impact VFX (spark/wood/ flesh); GPU particle burst |
| M6 | M6-EXT-3 | Systems Touched |
| M6.5 | M6.5-EXT-1 | Systems Touched |
| M6.5 | M6.5-EXT-2 | Systems Touched |
| M6.5 | M6.5-EXT-3 | Systems Touched |
| M7 | M7-EXT-1 | M7. grep zstd|lz4 → 0 (vertex quantization only). Tick-hash replay unbounded. |
| M7 | M7-EXT-2 | Every world object gets a stable WorldRefID (chunk-based, not ECS entityId) so it survives reloads and co-op sessions unchanged. |
| M7 | M7-EXT-3 | Systems Touched |
| M8 | M8-EXT-1 | Items/weapons as data defs (fire-rate/reload/recoil); SLM generates balanced stats (M13). |
| M8 | M8-EXT-2 | M8.6. Settlements spawn persistent + procedural resident NPCs with roles, daily loops, and personal plots — the world feels alive. |
| M8 | M8-EXT-3 | Links M8.6's Kirchhoff power grid to M10's Saint-Venant shallow-water flooding grid — when a snapped live line ([M8-EXT-26]/[M8-EXT-28], CableSpanComponent) dis |
| M9 | M9-EXT-1 | Links M3 macro-destruction structural hits to your M9 vehicle chassis velocity curves. |
| M9 | M9-EXT-2 | M10. Day/night cycle (sun/moon from WorldEpoch) + config-driven weather + SLM narrative atmosphere. |
| M9 | M9-EXT-3 | M9/M8. Attaching a part to a chassis is a hands-on welding minigame: keep the bead inside the tolerance band (metal) or drive fasteners on-mark (wood); quality sets joint strength. |
| M10 | M10-EXT-1 | Retained-mode widget system over your MSDF/text; data-bound to ECS. |
| M10 | M10-EXT-2 | M4/T98. Gen throughput chart over time. |
| M10 | M10-EXT-3 | M10/M12. No-waypoint environmental triggers that offer quests on proximity. |
| M11 | M11-EXT-1 | M7/M2.8/M11. Separate SaveSchemaVersion / NetworkProtocolVersion / ModAPIVersion. |
| M11 | M11-EXT-2 | M11/MetaRegistry. bool ModWritable on entt::meta registration. |
| M11 | M11-EXT-3 | M11 (UI). grep screen.shake|hit.marker|kill.feed → 0; M11 doesn't mention them. |
| M12 | M12-EXT-1 | M12. grep Network|Socket|ENet|Udp → 0. M2.8 co-op has zero transport. |
| M12 | M12-EXT-2 | Render entities at interpolated state between last two server snapshots |
| M12 | M12-EXT-4 | Player choices ripple through settlement/brand/NPC state; the world remembers and reacts. Emergence over deep simulation (DayZ lesson). |
| M13 | M13-EXT-1 | Systems Touched |
| M13 | M13-EXT-2 | M13. Same seed+template → cached output. |
| M13 | M13-EXT-3 | Pushes to M13-EXT-36/37 broadcasts and M8.6-EXT-13 reputation; M5.4-EXT-11 cannibal act is a key source. |
| M2.6 | M2.6-EXT-1 | Systems Touched |
| M2.6 | M2.6-EXT-2 | Systems Touched |
| M2.6 | M2.6-EXT-3 | Systems Touched |
| M2.7 | M2.7-EXT-1 | Systems Touched |
| M2.7 | M2.7-EXT-2 | Systems Touched |
| M2.7 | M2.7-EXT-3 | Systems Touched |
| M2.8 | M2.8-EXT-1 | Systems Touched |
| M2.8 | M2.8-EXT-2 | Systems Touched |
| M2.8 | M2.8-EXT-3 | Systems Touched |
| M2.9 | M2.9-EXT-1 | Systems Touched |
| M2.9 | M2.9-EXT-2 | Systems Touched |
| M2.9 | M2.9-EXT-3 | Systems Touched |
| M4.6 | M4.6-EXT-1 | Systems Touched |
| M4.6 | M4.6-EXT-2 | Systems Touched |
| M4.6 | M4.6-EXT-3 | Systems Touched |
| M5.1 | M5.1-EXT-1 | Systems Touched |
| M5.1 | M5.1-EXT-2 | Systems Touched |
| M5.1 | M5.1-EXT-3 | Systems Touched |
| M5.2 | M5.2-EXT-1 | Systems Touched |
| M5.2 | M5.2-EXT-2 | Systems Touched |
| M5.2 | M5.2-EXT-3 | Systems Touched |
| M5.3 | M5.3-EXT-1 | Systems Touched |
| M5.3 | M5.3-EXT-2 | Systems Touched |
| M5.3 | M5.3-EXT-3 | Systems Touched |
| M5.4 | M5.4-EXT-1 | Systems Touched |
| M5.4 | M5.4-EXT-2 | Systems Touched |
| M5.4 | M5.4-EXT-3 | Systems Touched |
| M6.5 | M6.5-EXT-1 | Systems Touched |
| M6.5 | M6.5-EXT-2 | Systems Touched |
| M6.5 | M6.5-EXT-3 | Systems Touched |
| M8.5 | M8.5-EXT-1 | Systems Touched |
| M8.5 | M8.5-EXT-2 | Systems Touched |
| M8.5 | M8.5-EXT-3 | Systems Touched |
| M8.6 | M8.6-EXT-1 | Systems Touched |
| M8.6 | M8.6-EXT-2 | Systems Touched |
| M8.6 | M8.6-EXT-3 | Systems Touched |
| M8.7 | M8.7-EXT-1 | Systems Touched |
| M8.7 | M8.7-EXT-2 | Systems Touched |
| M8.7 | M8.7-EXT-3 | Systems Touched |
