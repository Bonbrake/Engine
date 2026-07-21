# Appendix Execution Map
> Version: 2026-07-21
> Source: spec/APPENDICES.md parsed on disk
> Block count: 51 unique provisional EXT IDs
> Blocks with reference implementation: 24
> Blocks with explicit MATH DONE marker: 0
> Rule: all new IDs are provisional; verify against live milestone files before merge

## 1. How to use this file
This is the implementation order for appendix blocks, not a replacement for milestone ordering. Execute in numbered batches. Each batch has an acceptance test.

## 2. Block inventory by readiness tier

### Tier 1 — reference implementation present, implementation-ready
These blocks have concrete code or math already; build and verify first.

Batch T1-A: render infra
- M1-EXT-11 Compute-to-Indirect-Draw Execution Barrier [line 148]
- M4.5-EXT-28 Screen-Space Reflections SSR [line 461]
- M4.5-EXT-29 Screen-Space Global Illumination SSGI Fallback [line 490]
- M4.5-EXT-30 Impostor LOD Octahedral for Distant Meshes [line 517]
- M4.5-EXT-31 SDF Shadow Cascade [line 542]
- M4.5-EXT-26 Runtime Virtual Texture RVT Base System [line 409]
- M4.5-EXT-27 SSAO Scalable Ambient Obscurance [line 433]

Batch T1-B: world gen and destruction
- M4-EXT-08 3D WFC Vertical Structural Dependency Guard [line 613]
- M4-EXT-09 Deterministic Interior Furniture Spatial Constraint Solver [line 657]
- M4-EXT-10 Macro-Graph Vector Spline Corridor Welder [line 696]
- M4-EXT-11 WFC Contradiction Horizon Recovery [line 735]
- M4-EXT-23 Whittaker Temperature/Precipitation Biome Classification [line 772]
- M4-EXT-24 Cellular Automata Structural Collapse and Rubble Debris Fields [line 804]
- M4-EXT-25 Procedural Material Node-Graph Compiler [line 842]
- M4-EXT-26 Procedural Decal Atlas Packing and Runtime Projection [line 875]
- M4-EXT-27 Vegetation Impostor Foliage-Specific [line 1809]
- M4-EXT-28 Procedural Foliage L-System Mesh Generator [line 1830]

Batch T1-C: physics and vehicles
- M2-EXT-57 Fixed-Point Mesh-Vector Quantization Factory [line 305]
- M2-EXT-68 Kinematic Character Flood Buoyancy and Drag Bridge [line 331]
- M5-EXT-38 XPBD Rope/Tether Constraint [line 1538]
- M9-EXT-22 RVT Skid-Mark/Tire-Track Injector [line 1564]

Batch T1-D: audio VFX and misc
- M6.5-EXT-13 Capillary Blood-Spatter RVT Projection [line 1587]
- M6-EXT-12 Convolution-Reverb from Voxel Occlusion [line 1703]
- M8-EXT-10 Procedural Loot Icon Generation [line 922]
- M12-EXT-03 Bitstream Delta-Encoded Packet Replay Fragment Reassembler [line 959]
- M3-EXT-11 Fracture-Debris Broad-Phase Reuse [line 1636]
- M8-EXT-33 Settlement NavMesh from GA Layout [line 1658]
- M12-EXT-13 Interest-Management Spatial Hash Net Culling [line 1681]
- M10-EXT-11 Volumetric Cloud and Participating-Medium Scattering [line 1610]
- M5-EXT-53 Fear-Field Diffusion via Spatial Hash [line 1750]
- M4-EXT-89 BC7/Block-Texture Compression and Transcode [line 1726]

### Tier 2 — purpose and systems scoped, math/reference impl pending
These blocks declare systems and purpose, but lack MATH DONE or reference implementation. Treat as design-complete, implementation pending.

Notable Tier 2 blocks:
- M1-EXT-27 Uniform-Grid Spatial Hash Broad-Phase [line 1490]
- M1-EXT-28 GPU Software Occlusion Rasterizer HZB Feeder [line 1512]
- M3-EXT-10 Spherical-Harmonics Visibility Pre-Filter Grid [line 1636]
- M5.4-EXT-10 Procedural Mission and Event Director [line 1775]
- M5-EXT-23 ORCA Local Avoidance Solver [line 573]
- M5-EXT-50 Reaction-Diffusion Grid Sub-sampled Boundary Welder [line 583]
- M5-EXT-46 Visual Occlusion Sector Ray-March Pre-Filter [line 577]
- M7-EXT-08 Zstandard Custom Dictionary Static Compiler [line 905]
- M7-EXT-09 Atomic File-Swap Append-Only State Transaction Logger [line 909]
- M7-EXT-10 Binary Save Format Structural Schema Migrator [line 913]
- M7-EXT-11 Zstd Save-Compression and Streaming Store [line 1866]
- M8-EXT-09 Memory-Mapped FlatBinary Inventory Cache Dictionary [line 917]
- M9-EXT-04 Non-Newtonian Mud Silt Advection soil rut tracking [line 7002 implied]
- M9-EXT-20 Anti-Roll Torsional Suspension Stabilizer [line 323 implied]
- M9-EXT-21 Fluid Hydrodynamic Wading Resistance Modulator [line 328 implied]
- M10-EXT-01 Saint-Venant 2D Shallow Water PDE [line 912 implied]
- M10-EXT-02 Keplerian Sun or Moon Solver [line 912 implied]
- M10-EXT-03 Bruneton-Nishita Scattering [line 912 implied]
- M10-EXT-05 Navier-Stokes Wind Field [line 912 implied]
- M5-EXT-52 Holling Type II Cannibalism Feeding Satiator [line 592]

## 3. Dependency order
Execute Tier 1 in the order below because later blocks consume earlier ones.

1. M1-EXT-11 compute-to-indirect-draw barrier [T1-A]
2. M4.5-EXT-26 RVT base system [T1-A] — M9-EXT-22 and M6.5-EXT-13 depend on this
3. M4.5-EXT-27 SSAO [T1-A]
4. M4.5-EXT-28 SSR [T1-A]
5. M4.5-EXT-29 SSGI [T1-A]
6. M4.5-EXT-30 Impostor LOD [T1-A]
7. M4.5-EXT-31 SDF Shadow [T1-A]
8. M4-EXT-08 WFC vertical guard [T1-B]
9. M4-EXT-10 Corridor welder [T1-B]
10. M4-EXT-11 WFC contradiction recovery [T1-B]
11. M4-EXT-09 Furniture solver [T1-B]
12. M4-EXT-23 Whittaker biome classification [T1-B]
13. M4-EXT-24 CA rubble fields [T1-B]
14. M4-EXT-25 Material node-graph compiler [T1-B]
15. M4-EXT-26 Decal atlas/projection [T1-B]
16. M4-EXT-27 Foliage impostor [T1-B]
17. M4-EXT-28 Foliage L-system [T1-B]
18. M2-EXT-57 fixed-point vec3 quantization [T1-C]
19. M2-EXT-68 flood buoyancy and drag bridge [T1-C]
20. M5-EXT-38 XPBD rope/tether [T1-C]
21. M9-EXT-22 RVT skid-mark injector [T1-C]
22. M6.5-EXT-13 blood-spatter RVT projection [T1-D]
23. M6-EXT-12 convolution reverb [T1-D]
24. M8-EXT-10 procedural loot icon generation [T1-D]
25. M12-EXT-03 fragment reassembler [T1-D]
26. M3-EXT-11 fracture-debris broad-phase reuse [T1-D]
27. M8-EXT-33 settlement NavMesh [T1-D]
28. M12-EXT-13 interest-management spatial hash [T1-D]
29. M10-EXT-11 volumetric clouds [T1-D]
30. M5-EXT-53 fear-field diffusion via spatial hash [T1-D]
31. M4-EXT-89 BC7/block compression [T1-D]

After Tier 1, Tier 2 blocks can be executed in dependency order:
- M1-EXT-27 spatial hash before M5-EXT-53 fear diffusion
- M5.4-EXT-10 mission director after M5-EXT-50 reaction-diffusion boundary welder
- M7 save blocks after M8 itemization because save schema depends on item table layout
- M9 chassis/vehicle blocks after M2 physics and M4 material/node-graph blocks

## 4. Acceptance tests
Per batch:
- Code compiles under MSVC 14.44.35207 with Ninja
- Unit test added in tests/unit/ with pass/fail assertion
- If block has reference implementation, compile and run the snippet or its unit test
- If block has no reference implementation, acceptance test is explicit math/spec review with sign-off

Per repo:
- git status --short returns only planned changes for the batch
- 20/20 unit tests still pass
- headless smoke test still exits 0

## 5. ID collision check
Before merging any block, grep spec/ and APPENDICES.md for the exact ID. If a collision is found, treat the ID as provisional and rename before commit.

Verified provisional ceilings at plan time:
M0=12, M1=25, M2=68, M3=11, M4=89, M4.5=31, M5=53, M5.4=10, M6=12, M6.5=13, M7=11, M8=33, M9=22, M10=11, M11=42, M12=13, M13=54

## 6. Execution batches
Batch A: T1-A render infra
Batch B: T1-B world gen and destruction
Batch C: T1-C physics and vehicles
Batch D: T1-D audio VFX and misc
Batch E: Tier 2 design-complete blocks in dependency order
Batch F: Unit tests and acceptance tests for all batches above
