# Antigravity Rules — The Endless Quarantine

This file is a **persistent Rule**, not a milestone doc. Antigravity agents do not carry
context between sessions/Tasks — only Rules do (per Antigravity's own docs). Load this
file as a workspace Rule so every session, on every milestone, starts from the same
ground truth instead of re-deriving it from scratch or guessing.

Model policy: `REASONING_MODEL = Gemini 3.1 Pro (thinking_level: high)` for planning,
feature implementation, and anything touching cross-file/cross-system reasoning.
`FAST_MODEL = Gemini 3.5 Flash (thinking_level: low)` for mechanical/boilerplate steps
(renames, formatting, repetitive scaffolding). Don't use Flash for anything that requires
holding milestone-wide context — that's what causes scope drift.

---

## Specificity protocol — non-negotiable, applies to every milestone

* **Every number in the milestone docs is authoritative, not a suggestion.** Don't round,
  don't "improve," don't substitute a library default instead.
* **"Propose, confirm" means stop, not proceed-with-a-default.** Every "Decisions to
  flag" line in a milestone file is a hard checkpoint — surface it and wait.
* **API surfaces get verified, not recalled.** Check the actual signature against
  installed package headers (vcpkg) in the sandbox before calling anything — the sandbox
  can do this even when it can't run the graphical build.
* **Missing context is a stop condition, not a fill-in-the-blank.** If a milestone file
  doesn't specify something needed, stop and ask rather than inventing an interface.
* **When in doubt about scope, under-scope and flag it.** Do not silently expand a task
  to "while I'm in here" adjacent files.

## Repo hygiene — read before touching git

These exist because they've already gone wrong on this repo (`Ltmonkeysmash/Engine`):

* **Never stage or commit `build/` or `vcpkg_installed/`.** Confirm `.gitignore` covers
  both before the first commit of a session. If a commit ever includes either path, stop
  and unstage — do not "fix it in a follow-up commit."
* **Confirm the active branch before writing anything.** State the branch name back at
  the start of a session. Never assume `main`/the last-used branch is still checked out.
* **Touch only files the current milestone's Implementation Steps or EXT items name.**
  If a fix genuinely requires touching an out-of-scope file (e.g. `Application.cpp`,
  `Renderer.h` from an unrelated milestone), stop and flag it as a cross-cutting change
  instead of folding it into the current commit.
* **Never reconstruct file content from another branch's history to "fill in" a gap.**
  If a file's current content is unclear or seems wrong, say so and ask — don't
  fabricate plausible-looking source.
* **`STATUS.md` / `CHANGELOG.md` are the durable, session-independent source of truth**
  for where the project actually stands. On a fresh session (new Task, session TTL
  expiry, or a Rules update), read `STATUS.md` first. Update it before ending a session
  that changed milestone state.
* **Dependency versions are locked in `vcpkg.json`.** Don't bump a version to "latest"
  without it being an explicit milestone item (e.g. the Jolt 5.5.0 / EnTT 3.16.0 bumps
  that are already spec'd).

## Tooling & developer iteration loop

* Debug-draw is a first-class system — the immediate-mode debug-line/shape renderer
  built in M1 stays available and gets used, not bypassed with printf-debugging.
* Render graph / pass dependency management is owned by M1; don't hand-roll a second
  barrier-insertion path in a later milestone.
* **Two job systems, not four.** Jolt's `JobSystemThreadPool` (M2) is dedicated to
  physics only. Everything else uses the one general-purpose task scheduler (enkiTS)
  built at M0. Don't introduce a third scheduler for convenience.
* Shader toolchain: HLSL + DXC, targeting SPIR-V. No GLSL-via-glslang side path.
* Pipeline cache persists to disk (M0) — don't rebuild it from scratch every boot to
  "keep things simple."
* Profiler of record: Tracy.
* Confirm RenderDoc-capturable at M0 and don't regress it later.
* Unit tests for pure-logic systems: Catch2 or doctest, vendored via vcpkg.
* Minimum viable failure-mode handling: `VK_ERROR_DEVICE_LOST` and out-of-memory
  conditions log the failure and exit cleanly — never a silent hang or an unguarded
  crash with no artifact.

## Target hardware — every budget below is against this, not against the dev machine

**Recommended (dev reference machine):** RTX 2070 Super, Ryzen 5700G, 16GB RAM — Tier-1/
Tier-2 quality (RT shadows, ReSTIR GI) is built and profiled against this, at 60 FPS.

**Floor (Tier-0 minimum spec):** the whole same-generation GPU class, not one card —
RTX 2000-series (binding case: RTX 2060, 6GB VRAM), RX 6000-series (binding case: RX
6600, 8GB VRAM), or Intel Arc equivalent (binding case: Arc A580, 8GB VRAM). **6GB VRAM**
(from the RTX 2060) is the tightest common constraint and what every Tier-0 memory
budget is built against — not the higher 8GB figures from the other two families. CPU
floor: a 6-core/12-thread chip contemporaneous with that GPU generation (Ryzen 5 3600 /
Intel i5-10400). **Floor target: 30 FPS (33.3ms/frame)**, not 60.

Features not universally supported gate behind a capability check with a Tier-0
fallback — never a hard dependency on an optional extension.

Design pillars, in priority order:
1. Procedural variety over authored content, everywhere it's viable.
2. One system, fed from multiple places — not one system per feature.
3. Cost scales with what's near/visible, not with total world/population size.
4. Best-looking-for-the-cost, not maximal.
5. Reuse proven, battle-tested defaults for infrastructure that isn't the game's
   creative focus (networking transport, physics solver, etc).
6. Tier your ambition, don't assume it.

## Performance & resource budgets (Tier-0 floor, 33.3ms/frame)

* Render: ~14.0 ms
* Physics: ~5.0 ms
* AI: ~5.0 ms
* Animation: ~3.0 ms
* Audio: ~1.0 ms
* Headroom: ~5.3 ms
* Memory: ~4.0GB VRAM engine baseline (single-viewport) / ~3.3GB (split-screen) on the
  6GB Tier-0 floor. 12.0GB system RAM. These are design-time estimates — verify against
  actual Tier-0-class hardware once the render path exists; don't treat as profiled fact
  until then.

Numerical method budget (don't upgrade these to per-tick/per-entity without a stated
reason — the coarse update rate is intentional, not a placeholder):
* Scent Dispersion (M5.3) / Fear Diffusion (M5.4): coarse 2D grid, updated every 3–6 ticks.
* Regional SEIR Curve (M8): coarse per-region, updated once per in-game hour.
* Faction Population ODEs (M8.5): per-faction scalars, updated per in-game hour.
* Settlement Truss Solve (M8.6): small node count, event-driven.
* Groundwater PDE (M10): shared 2D flooding heightmap, updated every 6 ticks.

## Day 0 — architecture risk spikes (run before/alongside M0 if not already done)

* **Spike A — Determinism:** standalone console program, fixed literal seed, XOR
  particle states into a running 64-bit hash. Run twice, alternate compilation flags. If
  divergence occurs, M2.8 uses snapshot-sync as the primary co-op strategy instead of
  deterministic lockstep.
* **Spike B — World precision:** minimal SDL3 window + Vulkan swapchain. Authoritative
  position as `glm::dvec3`. GPU processes `renderPos = (glm::vec3)(entityPos - cameraPos)`.
  Track `worldOriginOffset` for rebasing at >500.0 units.
* **Spike C — Tier fallback switch:** query ray-tracing-pipeline via
  `vkb::PhysicalDeviceSelector`. Add a runtime toggle forcing `supportsRtPipeline = false`
  to verify the pure rasterized fallback actually works, not just detects.
* **Spike D — Destruction at scale:** Jolt boxes held by `FixedConstraint`. Sever
  constraints, run BFS graph search. Measure time at the target entity count.

## ID scheme

Every extended system is `[Mx-EXT-nn]` (e.g. `[M1-EXT-06]`) — stable, load-bearing IDs.
Reference them by ID, not by description, when talking about a system across files or
sessions. If a milestone file's Extended Systems Library has a gap in the numbering
(e.g. M0 has no `EXT-13`), that's an intentional retired ID from a past renumber, not a
missing system to invent — the milestone file's own "completeness audit" note will say so.

## Milestone file sequencing

One milestone file per Antigravity session/Task. Don't load two milestone files into one
session. Don't proceed past a milestone's Exit Criteria until every item passes by
**actually running the build** — reading the code and reasoning that it should work is
not verification. When Exit Criteria pass, start a fresh session and load the next file
in sequence; update `STATUS.md` first.
