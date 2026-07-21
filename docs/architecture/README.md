# ZombieEngine Architecture

This directory documents the engine architecture by subsystem. Each file is
grounded in the live milestone specs under `spec/` and the confirmed
toolchain: C++20, MSVC 14.44, Ninja, vcpkg, SDL3, Jolt 5.6.0, enkiTS,
Vulkan 1.4, Y-up right-handed.

## Renderer

Renderer architecture is defined primarily in `spec/M0.md` and `spec/M4.5.md`.
The renderer is bindless-first: `M0-EXT-08` allocates descriptor-indexing
handle pages, `M0-EXT-29` configures the HDR swapchain with correct color
space and DPI scale, and `M0-EXT-36` brings up the meshlet/mesh-shader
pipeline. `M4.5` extends this with hybrid ray tracing, upsampling, and
GPU-driven culling. The practical constraint on this hardware is the RTX 2070
SUPER with 8 GB VRAM; all renderer design choices are bounded by that memory
ceiling.

## Physics

Physics is documented in `spec/M2.md` and `spec/M3.md`. The foundation is Jolt
5.6.0 with XPBD joints for ragdoll and constraints. `M2` delivers a real
destructible test entity; `M3` adds macro-destruction and structural graphs.
The IPC barrier sits at the contact layer to prevent penetration between
high-velocity objects. Vehicle physics, traction, and damage deformation are
explicitly scoped to `M9` but the underlying Jolt integration is established
in `M2`.

## ECS / Jobs

The ECS and job architecture is defined in `spec/M1.md`. EnTT provides the
entity/component storage; `M1-EXT-01` brings the SpatialHash uniform grid for
broad-phase queries; `M1-EXT-06` wires the archetype mutation queue; and
`M1-EXT-09` caches concurrent archetype view iteration. enkiTS provides the
persistent task groups. The guiding constraint is no main-thread bottleneck:
all scene mutation happens on job threads, which is also the pattern id Tech 7
uses according to the cached Wikipedia extract.

## Audio

Audio architecture is split across `spec/M6.md` and `spec/M6.5.md`. `M6`
delivers hardware-accelerated audio with propagation; `M6.5` adds the GPU
particle/VFX system. The audio propagation model is sparse voxel with
frequency-band diffraction and portal-aware occlusion, updated at a fixed
10-20 Hz cadence. This maps to Paper 10's geometric audio propagation and
T-10's audio benchmark.

## Persistence

Persistence is defined in `spec/M7.md`. The format is SoA entity tables with
delta compression between ticks and periodic full checkpoint. Schema versioning
with forward/backward migration paths is mandatory. `M7-EXT-18` enforces
atomic write-temp-then-rename for save I/O; `M7-EXT-22` cross-checks
monotonic versus wall-clock to prevent save-state tampering. Mod save-
compatibility is preserved through named save-header offsets.

## Networking

Networking is defined in `spec/M12.md`. The topology is authority server with
CRDT resolution for replicated inventories and placements. Valve
GameNetworkingSockets provides reliable/unreliable message lanes over UDP,
fragmentation/reassembly, NAT traversal, and encryption. Deterministic lockstep
is explicitly not required; client prediction plus server authority is the
chosen model.

## SLM

`spec/M13.md` defines the local small-language-model integration. The runtime
is MiniCPM5-1B running locally through an SPSC queue into the quest/dialogue
system. The model is a content-generation backend, not a network service; all
inference runs on the user's machine with no telemetry exfiltrated.

## How to read this plan

This plan is not the spec. The implementation truth lives in the per-milestone
spec files under `spec/` and the appendix execution map at
`recon/plans/APPENDIX_EXECUTION_MAP.md`. Read those for the canonical block
IDs, acceptance tests, and dependency order. This plan maps research, case
studies, and executable tasks to those blocks.

## Appendix execution order

For implementation order of appendix EXT blocks, see
`recon/plans/APPENDIX_EXECUTION_MAP.md`. It lists the 51 unique provisional
EXT IDs by readiness tier, dependency order, and acceptance tests.

## Research cache

This project uses `scripts/fetch_research.py` to cache research sources
(arxiv abstracts, DOI redirects, GitHub repos, docs pages) under
`~/AppData/Local/hermes/cache/web/`. Use it to resolve `[X]` items in the
professional plan with real HTTP evidence instead of assumptions.

Run `python scripts/fetch_research.py` to refresh all caches. Run
`python scripts/fetch_research.py --vehicle` or `--gsound` to fetch
specific M9/M6 sources.
