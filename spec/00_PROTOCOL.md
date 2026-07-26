## Changelog — v80 (Hand-crafted Fallout/Skyrim hybrid pivot + Creation Engine editor + expanded SLM)
Expands the spec with a Creation Engine–class editor milestone (M2.6E), seamless interior/exterior streaming, expanded karma/faction reputation, Dragon's Dogma–style companion system with dual-path voice, 20 SLM generators (up from 10), dual-path author/engine voice, Recast/Detour navmesh, and best-in-class engine tech from UE5/Godot/Gothic/KOTOR2/Tyranny/Dragon's Dogma/BG3/Disco Elysium/llama.cpp. Zero removals — every existing milestone, EXT block, and component preserved.


## Changelog — v78.1 (M0/M1 gap closure — 40 gaps, 8 clusters)
Closes 40 architectural gaps across M0 (Vulkan bootstrap) and M1 (GPU-driven ECS) ahead of further M2.6 open-world work, grouped into 8 clusters (Transform/precision, batched raycast, input abstraction, determinism/save integrity, world simulation, resilience/crash safety, asset/content integrity + presentation, telemetry + SLM Director). Every entry follows the standard five-subsection EXT format. IDs assigned to free ranges verified against the live spec: M0-EXT-15..29 and M1-EXT-29..38 (the doc's own Numbering note at the M1-EXT-18 block confirms M1-EXT-13..17 are already claimed by live milestones/, and M1-EXT-18..28 were already defined in-body). No existing EXT ID was overwritten. MiniCPM5-1B config.json was pulled from Hugging Face and confirmed (GQA Llama: hidden_size 1536, 16 query heads, head_dim 128, 2 KV heads).


## Changelog — v77 (real-world value-grounding pass + 2 magnitude bugs)

Continuation of the v76 audit, this time hunting for underspecified/placeholder numeric constants and cross-checking every hardcoded physical/physiological constant against real-world reference data rather than the code structure itself. Two genuine magnitude bugs found and fixed, plus three underspecified constants given researched, cited defaults:

* **[Fixed — magnitude bug]** Appendix B's "Sedan impact at 60 km/h: ~2,900 N·s" was internally inconsistent with its own stated inputs — impulse = mass × Δv, and a sedan (~1,500 kg curb weight) at 60 km/h (16.67 m/s) coming to a full stop implies ≈25,000 N·s, not 2,900 N·s (which back-solves to a ~174 kg object, motorcycle-class). Corrected to ~25,000 N·s.
* **[Fixed — magnitude bug]** `[M10]`'s flood-simulation `infiltration` default was `0.02f` m/s (soil absorption), which is ~72 m/hr — roughly four orders of magnitude past real hydrology figures (real infiltration rates run ~2.8e-8 m/s for clay up to ~5.8e-5 m/s for sandy soil). Corrected to `3.6e-6f` m/s (~1.3 cm/hr, representative loam).
* **[Grounded]** `[M8-EXT-01]` chamber cook-off's `T_fail` was stated as "≈450°C" in prose with no matching `T_melt` value anywhere and no constants in the reference implementation. Real cook-off literature (chambered 7.62mm cartridge testing, nitrocellulose's ~160-170°C autoignition point, commonly-cited 149-204°C practical range) puts real-world onset at roughly 150-200°C — the doc's 450°C overshot that by more than 2x. Set `T_fail = 200°C/473.15K`, `T_melt = 260°C/533.15K`, added as `constexpr` defaults in the reference implementation so this is no longer a bare unbound parameter.
* **[Grounded]** `[M2.9-EXT-07]`'s `CP_BASE` (critical power baseline for the exertion model) had no numeric value anywhere in the doc. Set to `220W`, grounded in exercise-physiology critical-power literature (~150-200W untrained, ~200-250W recreationally active, 300W+ trained endurance athletes) — appropriate for an active-but-not-athlete survivor protagonist.
* **[Grounded]** `[M2.9-EXT-01]`'s `σ_yield_tissue` (soft-tissue cavitation) had no default value. Set to `1.8 MPa` for generic soft tissue, grounded in ballistic-gelatin/soft-tissue failure-stress research (~1-2.5 MPa commonly cited), with prose guidance for lower (~0.3-0.6 MPa organ) and higher (~4-6 MPa muscle-dense limb) per-region values if that tuning is added later.
* **[Verified, no change needed]** Re-checked the physics constants already correct from the v76 pass (g=9.81, R=8.314 J/mol·K, room temp 293.15K, latent heat of vaporization 2,260,000 J/kg, Friis path-loss formula, Griffith-Irwin K_I fracture formula, Henyey-Greenstein phase function, Rayleigh λ⁻⁴ scattering) — all still correct, none touched this pass.
* **[Verified, no change needed]** The Arrhenius corrosion/decay model's `Ea = 5.5e4` J/mol (55 kJ/mol) checked against atmospheric steel corrosion literature (typically cited 40-90 kJ/mol range) — already realistic, not touched.


## Changelog — v76 (full line-by-line code-block audit, no dedupe/ID work this pass)

Went through all 268 code blocks in the doc (236 C++, 32 GLSL) individually against their own Math/How-It-Works sections and, where relevant, real library docs (Jolt `BodyInterface` call signatures, GLSL image/sampler layout-qualifier rules). Most held up — the physics (Erlang-C queueing model, Keplerian orbital solve, Rayleigh/Beer-Lambert/Doppler formulas, XPBD substep, Jolt `AddForce`/`AddImpulse`/`SetShape` usage) checked out against reference derivations and the real API. Three real bugs found and fixed:

* **[Fixed]** `[M6-EXT-07]` Velvet-Noise Late Reverb: `GenerateVelvetNoiseIR` accumulated pulse positions with `t += tAvg * RandomUnit()`. Since `RandomUnit()` is uniform on `[0,1)` with mean 0.5, this gives a mean pulse spacing of `tAvg/2` — double the density the function's own Math section specifies (`t_k = (k + rand[0,1)) · T_avg`, mean spacing `T_avg`). Fixed to `t += tAvg * (0.5f + RandomUnit())`, which restores a mean step of exactly `T_avg` while keeping the irregular jitter. Every room's `(T_avg, τ)` material tuning table was implicitly reverb-tail-dense by 2x versus what was authored.
* **[Fixed]** `[M2.7-EXT-09]` Elastic Canvas Trampoline Boundaries: the Math section defines `F_bounce` as a force, but the reference implementation added it straight to `out_vel_y` with no mass or `dt` term — a force was being treated as a velocity delta, unlike every other spring function in this doc (compare `[M5.2-EXT-10]`'s `UpdateSpringDamper`, which correctly does `accel = F/m; v += accel*dt`). Added `mass`/`dt` parameters and divide-then-integrate, matching the doc's own convention elsewhere. No other call sites reference this function, so the signature change is contained.
* **[Fixed]** `[M4.5-EXT-11]` Multi-Sampled Depth Derivative Silhouette Reconstruction Filter: the GLSL declared `layout(binding = 0, r32f) uniform sampler2D sceneDepthTex;`. `r32f` is a storage-image format qualifier (only legal on `image2D`/etc.) and is not valid on a combined texture-sampler — this line wouldn't compile. A sampler's texel format comes from the bound image view, not a shader-side qualifier. Dropped the format qualifier, keeping just `layout(binding = 0) uniform sampler2D`.
* **[Verified, no bug]** Checked every Jolt Physics call in the doc (`AddForce`, `AddImpulse`, `SetShape`, all with their arg order) against Jolt's real `BodyInterface` signatures — all correct. Checked GLSL subgroup ops (`subgroupBallot`, `subgroupExclusiveAdd`, `subgroupBroadcastFirst`, `subgroupMax`) against the extensions declared — all used consistently with their declared extensions.


## Changelog — v75 (Pro/Flash model split + Appendix K duplicate-bug audit)

* **[Model policy]** `REASONING_MODEL` retargeted from `Gemini 3.5 Flash (high)` to `Gemini 3.1 Pro (thinking_level: high)`; `FAST_MODEL` stays `Gemini 3.5 Flash (thinking_level: low)` for mechanical/boilerplate steps. Reasoning: `Gemini 3.5 Pro` is not yet GA (still limited preview as of this writing) so it isn't a real option; 3.1 Pro is the current shipping Pro-tier model and — unlike 3.5 Flash — is on Google's supported list for Computer Use/browser-desktop control, which removes v74's manual agent-switching workaround for exit criteria that require visually confirming something on screen. See the updated "Running this doc" note below for the full split.
* **[Fixed — real bug, not just duplication]** `[M0-EXT-10]`'s Appendix K copy still had the pre-fix buggy condition (`(range.stageFlags & activeStages) == activeStages`) that the canonical M0 body copy's own inline comment documents as wrong and replaces with `!= 0`. The duplicate had silently regressed back to the bug the main copy already fixed. Appendix K entry is now a cross-reference stub instead of a second copy of the code, so this can't drift again.
* **[Fixed — real bug, not just duplication]** `[M4.5-EXT-12]`'s Appendix K copy dropped the global `BucketCounter` atomic entirely and wrote `subgroupExclusiveAdd(1u)` straight to the image at a per-workgroup-local offset — exactly the overlapping-write bug the canonical M4.5 body copy's `[BUG-FLAG]` note describes and fixes. Reduced to a cross-reference stub.
* **[Fixed — real bug, not just duplication]** `[M4.5-EXT-13]`'s Appendix K copy still used the backwards `nearDepth >= farDepth` check with no reference to the light's shadow bounds — the exact bug the canonical M4.5 body copy's `[BUG-FLAG]` note describes and fixes (the appendix copy's own "verify before wiring this in" caveat was the tell). Reduced to a cross-reference stub.
* **[Deduped, no bug found]** `[M1-EXT-09]`, `[M4.6-EXT-04]`, and `[M5.4-EXT-04]` had Appendix K copies that were byte-identical to their canonical body copies (no behavioral divergence) — still collapsed to cross-reference stubs on the same principle as the three fixes above, so a future edit to the canonical copy can't silently leave a stale duplicate behind the way it just did for the other three.
* **[Root cause note]** All six duplicates trace to Appendix K being built as a "consolidated, deduplicated list" that in practice re-embedded full code for items that already had a canonical body definition elsewhere, instead of only listing items that were new. `[M12-EXT-03]` already followed the correct pattern (appendix stub cross-referencing the canonical copy) — the other six now match it. This closes the actual gap in v72's "zero dangling references" audit, which checked reference/definition pairing but not multiple-definition drift, since at v72 time these six didn't have Appendix K copies yet.
* **[Verified]** Re-ran the full `[Mx-EXT-NN]`/`[K-EXT-NN]`/`[L-EXT-NN]` ID audit after the stub conversion: 294 unique IDs, all with exactly one full definition and zero dangling references.


## Changelog — v74 (retarget to Gemini 3.5 Flash, High)

* **[Model policy]** `REASONING_MODEL` and `FAST_MODEL` both retargeted to `Gemini 3.5 Flash`, split by `thinking_level` (`high` / `low`) instead of by separate model — see the "Running this doc on Gemini 3.5 Flash (High)" note right after the model policy line for the reasoning behind the split, the Computer Use carve-out, and the quota/session-hygiene guidance. No milestone content changed; this is a front-matter-only pass.


## Changelog — v73 (merge of `updateforv72.txt`, triaged)

`updateforv72.txt` (a second AI-generated pass proposing ~20 new subsystems plus several already-covered cross-references) was checked item-by-item against this doc before merging, not merged wholesale. Full breakdown in **Appendix M** below. Summary:

* **7 items kept as real, load-bearing additions** — genuinely solve a stated problem with a technique that's actually used at the scale of one dev: async compute queue separation (`[M0-EXT-14]`, renumbered — collided with the existing `[M0-EXT-11]`), fixed-point rollback netcode (`[M12-EXT-05]`), kinematic ledge-lock rebasing (`[M5.2-EXT-13]`), single-pass Hi-Z downsample (`[M4.6-EXT-06]`), offscreen horde continuum-fluid density field (`[M5.4-EXT-08]`), its structural-fatigue counterpart for hibernated buildings (`[M3-EXT-09]`, renumbered — collided with the existing `[M3-EXT-08]`), and dynamic MSDF glyph rasterizing (`[M1-EXT-12]`).
* **2 items were pure ID collisions with no new content** — `[M0-EXT-11]` and `[M3-EXT-08]` in the update were new definitions reusing IDs already assigned to unrelated existing systems; renumbered above. Every other "colliding" ID in the update (`M1-EXT-06`, `M2.7-EXT-02`, `M2.8-EXT-02/04`, `M3-EXT-06`, `M4.5-EXT-07/08`, `M6-EXT-04/11`, `M8-EXT-07`, `M11-EXT-07`, `M12-EXT-01/02`) was just a citation of an already-existing system inside a "Systems Touched" list, not a redefinition — no action needed.
* **6 items rejected or deferred as scope risk, not technical error** — the visual-polish chain (muscle-volume skinning correction, FFT-driven lip sync, heat-shimmer refraction, split-screen HZB reprojection cache), the from-scratch STUN/TURN NAT traversal gateway, and — the biggest one — wiring the local SLM's output to directly mutate spawn positions/prices/hallucination effects every 30 seconds. None of these are technically "wrong," but each is a multi-week AAA-team-scale undertaking bolted onto a doc already covering M0–M13 solo. Logged with reasoning in Appendix M rather than silently dropped.


## Changelog — v72 (structural/dedupe/gap audit pass)

* **[Fixed — ID collision]** `[M9-EXT-09]` was double-assigned: the original (Volatile Fuel Tank Puncture Bernoulli Drainage) and a v71 Appendix K addition (Anti-Roll Torsional Suspension Stabilizer) were two distinct features sharing one ID. The Appendix K feature is renumbered to `[M9-EXT-20]`; no cross-references needed updating since every other citation of `M9-EXT-09` already pointed to the fuel-tank feature.
* **[Fixed — ID collision]** `[M9-EXT-10]` was double-assigned the same way: the original (Aero-Elastic Body Panel Drag Deformation Scaling) vs. a v71 Appendix K addition (Fluid Hydrodynamic Wading Resistance Modulator). Renumbered the latter to `[M9-EXT-21]`.
* **[Fixed — ID collision]** `[M12-EXT-01]` was double-assigned: the original (Bitstream Delta-Encoded Network Serialization Packet Encoder) vs. a v71 Appendix K addition (Local Network-Relevancy Grid Culling Filter). Renumbered the latter to `[M12-EXT-04]`.
* **[Added]** `[K-EXT-21]` Clustered Froxel Light Culling Pass — closes a real current-gen rendering gap: the doc assumes "thousands of procedural lights get to a tight GPU list for the clustered light pass" in several places (M8.6 electrical traps, vehicle headlights, muzzle flashes) but that culling/clustering system was never specified beyond a §5.8 one-liner, unlike every other rendering subsystem in this doc which already matches current AAA practice (meshlet clustering, SVO GI, Virtual Shadow Maps). Shares its depth-slicing scheme with the existing `[M4.5-EXT-13]` shadow voxelizer rather than adding a second one.
* **[Verified, no change needed]** Full ID audit (every `[Mx-EXT-NN]`/`[K-EXT-NN]`/`[L-EXT-NN]` reference vs. definition): zero dangling references — every ID cited anywhere in the doc has exactly one real definition once the three collisions above are fixed. Verified via grep cross-reference, not asserted.
* **[Verified, no change needed]** Canonical shared structs (`BarrelHeat`, `StructuralFatigue`, `SurfaceFrictionSample`) each have exactly one definition, and the front-matter's item #1–#7 self-audit (ammo cook-off dedup, fouling/jam state sharing, `ResolveFriction()` ordering, `DoorComponent`, RVT overlay system, `BarrelHeat` field lock, `entt::observer` deprecation) are all already implemented in-body, not just recommended — checked each against its claimed landing spot in M2.6/M2.7/M3/M4.5 rather than trusting the front-matter's own say-so.
* **[Verified, no change needed]** Dedupe sweep of Appendix K (the newest, least-reviewed section) against the rest of the doc for content-level overlap (soft-body vehicle deformation vs. `StructuralFatigue`, GPU broadphase hash vs. CPU spatial hash, forensic decal RVT writer vs. skid-mark/blood-spatter RVT writers, wading-resistance drag vs. exhaust-backpressure torque stall): all confirmed complementary, not duplicate — each already carries an explicit "distinct from X because Y" note that checked out against the cited system.
* **[Rejected as already covered]** Considered flagging a "many dynamic lights" gap as a brand-new milestone-scale system; rejected in favor of the minimal fix above (fleshing the existing §5.8 one-liner into `[K-EXT-21]`) since a new milestone would duplicate work the doc already scoped correctly, just never wrote up.
* **[Out of scope, flagged not padded]** §5.8 still has ~20 other un-fleshed one-liners (network clock sync compensator, mud rut tracker, chassis vortex drafting, etc.). Only the light-culling one met the bar of "closes a nameable current-gen gap this pass was asked to check for"; the rest are lower-priority polish items, not audit findings, and are left as-is rather than padded out.

**Model policy — edit only this one line when a better model ships:** `REASONING_MODEL = Gemini 3.1 Pro (thinking_level: high)` · `FAST_MODEL = Gemini 3.5 Flash (thinking_level: low)`. Every instruction in this doc refers to the *role* (`REASONING_MODEL` / `FAST_MODEL`), never a pinned model name — when Google (or a competing lab, since Hermes Agent also natively supports Claude and GPT-OSS as first-class agent options) ships something stronger, swap the name on this line only. Nothing else in the document needs to change.

### Running this doc on Gemini 3.1 Pro / 3.5 Flash — read this before starting M0

`Gemini 3.5 Pro` is not a real option right now — as of this writing it's still in limited enterprise/Hermes Agent preview with no public GA date, so it isn't something you can actually pin this doc to yet. `Gemini 3.1 Pro` is the current shipping Pro-tier model and is what `REASONING_MODEL` points to; `Gemini 3.5 Flash` (GA'd May 19, 2026) covers `FAST_MODEL`. A few sharp edges worth knowing going in:

* **Set `thinking_level` explicitly, every session, for both roles.** For every milestone's architecture decisions (the stuff under "How It Works," struct layouts, the math sections), run `REASONING_MODEL` at `high`. For mechanical work — renaming, formatting, boilerplate, the ~20 unfleshed §5.8 one-liners, simple `[Mx-EXT-NN]` blocks that just wire an existing struct into a new system — use `FAST_MODEL` at `low`; it's meaningfully faster and cheaper for short agentic steps and doesn't need Pro-level reasoning to get right.
* **Computer Use stays on `REASONING_MODEL`.** M0's "verify by running the build, not by reading code" rule, and every milestone's exit criteria that involves visually confirming something on screen (particle effects, UI layout, animation blending), leans on Hermes Agent's browser/desktop-control surface. Gemini 3.1 Pro is on Google's supported list for this; Gemini 3.5 Flash is not, as of this writing. Practical split: let `FAST_MODEL` run the build and any exit criteria that's a `grep`/test-suite/log check (most of them already are, by design), and keep the agent on `REASONING_MODEL` for the handful of milestones where "exit criteria" means "look at the screen and confirm it looks/plays right" — no manual agent-switching needed, since Pro already covers both roles' visual-confirmation needs.
* **Watch your quota on both.** 3.1 Pro at `high` and 3.5 Flash at `high` both burn through weekly usage limits fast; given this doc's scale (13 milestones, hundreds of `[Mx-EXT-NN]` blocks), the `low`-thinking `FAST_MODEL` split above is the main lever — check Hermes Agent's current rate limits for your account before committing to a full M0→M13 run in one week.
* **Context window and output cap vary by role — check both before a long session.** Whatever the current context/output limits are for your pinned `REASONING_MODEL`/`FAST_MODEL` pair, the existing discipline in this doc — one system/one PR per `[Mx-EXT-NN]` block rather than dumping a whole milestone in one shot — still matters regardless of how large the window gets; don't ask the agent to write an entire milestone's code in a single turn.
* **Start a fresh chat per milestone**, not one marathon session for all thirteen, so a bad assumption the agent locks onto early (e.g. misreading a struct field) can't quietly carry forward and bend a later milestone's decisions.

Full rebuild: start over at M0, do not resume any prior partial codebase. Verify every milestone by running the build, not by reading code and assuming it works — this replaces the old separate "M2.5 stabilization gate" milestone, folded into every milestone's exit criteria instead.

This is a custom proprietary C++ engine, not Unity. We are building an original GTA-style open-world procedurally generated zombie survival engine, not a WaW clone. Enforce YAGNI principles, clean modular design, and concise one-liner logic solutions everywhere viable.

---


## Table of Contents

##### Front matter

Engineering Audit · How to Work This Task · Specificity Protocol · Tooling/Iteration Loop · Context & Engineering Pillars · Performance & Resource Budgets

##### Milestones

* M0 — Vulkan 1.4 native bootstrap + capability tiering
* M1 — GPU-driven ECS framework *(incl. Dev-Tool: EnTT Meta Entity Inspector)*
* M2 — Jolt 5.6.0 physics, EventBus, and a real destructible test entity
* M2.6 — Open-world foundations
* M2.7 — Seamless true first/third-person player controller & core combat
* M2.8 — Deterministic co-op architecture
* M2.9 — Traversal, ballistics & survival physiology
* M3 — Macro-destruction & structural graphs
* M4 — Procedural world generation + chunk streaming & culling
* M4.5 — Hybrid ray tracing, GPU-driven pipeline & upscaling
* M4.6 — Memory management
* M5 — NPCs / zombie hordes (base)
* M5.1 — Procedural zombie variation, utility AI & bio-weight simulation
* M5.2 — Procedural animation (physics/IK-driven)
* M5.3 — AI perception system
* M5.4 — AI Director (horde pacing and spawner)
* M6 — Hardware-accelerated audio system with propagation
* M6.5 — GPU particle/VFX system
* M7 — Total persistence
* M8 — Data-driven itemization
* M8.5 — Survivor faction economy
* M8.6 — Settlement construction
* M8.7 — Perk & progression system (PerkPoints)
* M9 — Vehicle system
* M10 — Day/night, weather & atmosphere
* M11 — UI/HUD, input abstraction & consolidated haptics
* M12 — Networked co-op at scale
* M13 — Local Small Language Model (SLM) integration — core system: ten always-on `M13-EXT` generators feed mission text, world lore, journals, and signage across M4/M5.4/M6/M8/M8.5/M8.6/M10/M11; not a bolt-on flavor pass

##### Back matter

Appendices A–J · Not Yet Merged (deferred master-list items) · Appendix K — AAA-Parity Gap-Fill (merged from `update.txt`, v70) · Appendix L — Modding & Extensibility Architecture (v71) · Appendix M — Async/Netcode/Scale Gap-Fill (merged from `updateforv72.txt`, v73)

*Milestones with an "Extended Systems Library" subsection (M0, M1, M2, M2.7, M2.8, M2.9, M3, M4, M4.5, M4.6, M5, M5.1, M5.2, M5.3, M5.4, M6, M6.5, M7, M8, M8.5, M8.6, M9, M11, M12, M13) contain numbered `[Mx-EXT-NN]` feature blocks — search for the ID directly if you know it, e.g. `[M5.2-EXT-05]`.*

---


## Engineering Audit — read this before building

I went through the design sessions content-first (not just by title) looking for things that were actually wrong, not just messy. Six real issues, plus the fix applied for each:

**1. Duplicate ammo cook-off mechanic.** *Thermally Induced Ammo Cook-Off Overdrive* and *Brand-Gated Thermal Ammo Cook-Off* are the same trigger (barrel exceeds ~450°C → chambered round detonates) implemented with two different, incompatible math models (Arrhenius exponential vs. a clamped linear ramp against brand/grime multipliers). Only *Brand-Gated Thermal Ammo Cook-Off* (M8) is kept — it's the one actually wired into the M8 JSON item/brand-tier system the rest of the itemization pipeline depends on.

**2. Two features silently share one state variable.** *Atmospheric Powder-Fouling Friction Accumulation* (M2.7) is the only place carbon fouling `F` is actually produced (`ΔF = κ·(1+γ·Humidity)`); *Tactical Clear-and-Vent Malfunctions* (M2.7) consumes `F` for its jam-probability roll but never defines where `F` comes from. They read as two independent features in the source material but are really one system split across two write-ups — build them together, in the same PR, or the jam mechanic has no fouling input to react to.

**3. Traction/friction has four independent writers with no composition order defined.** *Fluid Momentum Traction Shear* (blood), *Non-Linear Tire Slip Friction Heating* (tire temp), *High-Velocity Wet-Silt Aquaplaning* (standing water), and *Non-Newtonian Mud Silt Advection* (saturated soil) all multiply their own coefficient against `μ_base` independently, and *The Corpse-Pile Flattening Engine* sets `out_traction` directly rather than contributing a multiplier. If all five write straight into `SurfaceFrictionSample` in whatever order the query happens to run, the result is order-dependent and will fight itself (e.g. a flattened corpse in a puddle — does mud win or the corpse's fixed value win?). **Recommendation:** give `SurfaceFrictionSample` a single `ResolveFriction()` step that takes `μ_base` and applies each modifier as an explicit, ordered multiplicative term (surface material → weather/mud → blood → thermal), with corpse-pile treated as a per-cell override that participates in the same chain rather than a hard write. Each of the four/five features below should be re-read as "one term in that chain," not as its own authority.

**4. No base "breakable door" component exists anywhere in M0–M12.** Both *Latch-Shear Kinematic Door Piercing* (M2.7) and *Kinematic Door-Kick Hydraulic Ram Splitting* (M2.7) assume an `EnTT DoorComponent` with a lock-state bitflag and a material shear threshold already exists — but no milestone in the base document ever declares it. Add a minimal `DoorComponent { locked: bool, shear_threshold: float, material_class }` to M2.6 or M3 (it's structurally a small destructible, so M3 fits best) before either of these two systems can compile against something real.

**5. No base Runtime Virtual Texture (RVT) system exists anywhere in M1/M4/M4.5.** Three separate features (*Dynamic Skid-Mark Injection via RVT*, *Capillary Spatter Impact Angle Projection*, *Hydrocarbon Slipstream Flame-Trail Splitting*) all write into "your RVT terrain overlay layer" as if it's already a standing system, but M1's GPU-driven ECS section and M4.5's hybrid-RT section never actually set one up. This needs to be added as a real M1 or M4.5 implementation step (a persistent terrain-chunk overlay texture with a Vulkan storage-image write path) before any of the three dependent features can be built — right now they're all pointing at a system that doesn't exist yet.

**6. `BarrelHeat` is extended by five different features with inconsistent assumptions about its own fields.** *Thermo-Elastic Barrel Distortion*, *Tactical Clear-and-Vent Malfunctions*, *Brand-Gated Thermal Ammo Cook-Off*, and *Atmospheric Powder-Fouling Friction Accumulation* all read or write `BarrelHeat` (kelvin, fouling `F`, brand/grime multipliers) without ever being collected into one canonical struct definition. Recommend locking the actual field list once, in M2.7's base Implementation Steps, e.g.:

```cpp
struct BarrelHeat {
    float kelvin = 293.15f;
    float fouling = 0.0f;       // carbon fouling accumulation (0-1)
    float grimeFactor = 0.0f;   // maintenance neglect scalar
};
```

so every extended system below reads/writes the same fields instead of each silently assuming its own.

**7. `entt::observer` audit (gap-fill pass item 11).** EnTT's `entt::observer` class is deprecated as of the 3.16.0 bump in favor of the reactive mixin/storage pattern. Audited every feature description in this document for the old pattern: **zero occurrences found** — no feature below constructs or references `entt::observer` anywhere in the current doc. Nothing to rewrite. Flagging this as a forward-looking constraint instead: any future feature that would reach for change-detection/reactive queries (e.g. "notify when `BarrelHeat.kelvin` crosses a threshold" style systems) should be written against EnTT 3.16.0's reactive storage mixin (`entt::reactive_mixin` / `on_construct`+`on_update` storage-backed queries) from the start, not the deprecated observer class.

**Forward-dependency flags.** Several systems below are placed at the milestone their *primary* mechanic belongs to, but cite a system from a *later* milestone (e.g. an M2.7 feature that reads M9 surface-friction data, or an M9 feature that needs M10's flood heightmap). These aren't mistakes — the base v37 document already does this on purpose for M9/M10 (`SurfaceFrictionSample` is built as an abstract interface in M9, then *written to* with real data in M10). Each affected system below is now marked **Depends on (not yet built at this point)** so you know to stub a fixed test value rather than expecting real behavior until that later milestone lands.

---


## How to work this task (Hermes Agent — Manager view, mixed model assignment)

**Model assignment (Hermes Agent lets you assign a different model per agent/task — use this, don't run everything on one model):** assign **`REASONING_MODEL`** as the implementation agent for every milestone's actual engineering work — it's the tier with the deep-codebase-reasoning budget this doc's cross-system dependency chains (`BarrelHeat`, `SurfaceFrictionSample`, `StructuralFatigue`) need. Reserve **`FAST_MODEL`** for narrow, low-risk sub-tasks spawned off the main task — formatting a Task List Artifact, generating `STATUS.md`/`CHANGELOG.md` boilerplate, or drafting the Walkthrough Artifact write-up after the reasoning agent has already done the engineering. Never assign `FAST_MODEL` to a task that has to resolve a "Decisions to flag" checkpoint — fast models move quickly through a task list and will guess past a stop condition rather than actually stopping.

**Context persists across sessions in Hermes Agent via its memory system — but per-task working state does not.** Hermes injects its memory file into every new session, so cross-session non-negotiables survive automatically; however, closing and reopening a Task still loses anything you taught it only through in-task review comments. Before M0, write this document's non-negotiables into Hermes's system prompt / project memory (the persistent memory file Hermes loads every session) — at minimum: YAGNI/one-liner-logic-solutions, CRLF line endings, versioned filenames, post-edit `grep` verification, and "every number in this doc is authoritative, not a suggestion." Hermes applies its memory rules automatically to every new Task; re-pasting this whole document at the top of every milestone Task is a fallback, not the primary mechanism.

**Terminal execution policy: start on the "Review-driven" preset (equivalent to Auto, not Turbo).** Turbo auto-runs every terminal command except an explicit Deny list, and real users have reported agents running destructive commands unprompted — not worth it against a repo with a versioned spec document and a real save-format. Populate an **Allow list** with the routine, safe commands this workflow repeats constantly (`git status`, `git diff`, `ctest`, `vcpkg install`, the project's own `--headless` build/test invocation, `grep`) so you're not confirming those every single time, and a **Deny list** covering destructive ones (`rm -rf`, `git push --force`, anything touching `vcpkg.json` version pins without the Implementation Plan calling for it). Loosen toward Turbo later, per-Task, once you trust a given milestone's agent output — not globally, and not for milestones M2.8+ (co-op/networking) or anything touching the save format, where a bad auto-run is expensive to unwind.

Structure this as **one Task per milestone** in the Manager view, run in order — not one giant open-ended session, and not a scheduled/background Task (scheduled runs are designed to operate unattended and will skip the confirmation pause every step below depends on). Hermes Agent supports parallel sub-agents genuinely isolated from each other's file changes — useful *within* a milestone (e.g. one sub-agent drafting the Task List Artifact while the Pro agent starts on Implementation Plan review), but do not run two milestone Tasks in parallel against the same working tree; they'll silently diverge since neither sees the other's edits until you manually route them together. Per-milestone loop, every time, no exceptions:

1. **Implementation Plan Artifact, before any code.** The agent writes an Implementation Plan Artifact: files to add/change, new types/structs (with every field named), system/init order, and which of this milestone's "Decisions to flag" items apply. Do not proceed to code until reviewed and approved in the Manager view.
2. **Task List Artifact.** Break the milestone's Implementation steps into a checklist Artifact.
3. **Small, reviewable diffs.** Commit in small units mapped to Task List items. Enforce YAGNI and one-liner logic solutions where viable.
4. **Build and run after every change — not just compile.** Confirm the fixed-timestep loop is stable. If headless, use `--headless` plus logged frame-time/tick output. Hermes Agent's own sandbox for this Task is a display-less remote Linux environment, so a "run" the agent performs there is a compile+headless-logic check at best — see item 5.
5. **Vulkan validation layers checked after every milestone. [Local-GPU verification required]** The agent's sandbox has no GPU/display, so it cannot produce a real validation log or RenderDoc capture itself. **Hermes Agent's Browser Subagent (Chrome-based click/screenshot verification) does not apply to this project** — this is a native Vulkan binary, not a web app, so there's no page for the browser agent to navigate. Any milestone touching rendering must instead have its validation-layer check and frame-time capture run on your actual RTX 2070 Super machine, with that log/screenshot pasted into the Walkthrough by you — the agent should say so explicitly rather than reporting a sandboxed "pass."
6. **Co-op test paths from M2.8 onward. [Local-GPU verification required]** "Run the binary" means split-screen with 2 local inputs, plus a second instance/machine where practical — this is a local-hardware step for the same display/GPU reason as item 5.
7. **Walkthrough Artifact at the gate.** Before moving to the next milestone, produce a Walkthrough Artifact covering exactly how it was verified, and which parts of that verification were sandboxed vs. confirmed on local GPU hardware. This feeds `STATUS.md`.
8. **Numbers, not vibes.** Check exit criteria against the Performance & Resource Budgets table.
9. **Watch the session clock on long milestones.** Hermes Agent's managed agent sessions have a TTL and don't run forever — for milestones whose Implementation Plan spans multiple sittings (M4, M5.4, M9), have the agent checkpoint progress into `STATUS.md` at natural sub-steps rather than only at the final Walkthrough, so a session expiry mid-milestone doesn't lose unrecorded state.


## Specificity protocol (read this before M0 — this is what stops the agent from guessing)

Fast models move quickly through a task list; enforce all of the following without exception so speed doesn't trade off against correctness, on `REASONING_MODEL` and `FAST_MODEL` alike:

* **Every number in this document is authoritative, not a suggestion.**
* **"Propose, confirm" means stop, not proceed-with-a-default.** Every "Decisions to flag" line is a hard checkpoint.
* **API surfaces get verified, not recalled.** Check the actual signature against installed package headers (vcpkg) inside the sandbox — the sandbox's Linux environment can do this even though it can't run the graphical build.
* **Missing context is a stop condition, not a fill-in-the-blank.** Stop and ask rather than inventing interfaces.
* **When in doubt about scope, under-scope and flag it.**


## Project continuity

`STATUS.md`/`CHANGELOG.md` are the durable, session-independent source of truth. If context is reset (new session, session TTL expiry, or a Rule update), read only `STATUS.md` to know exactly where things stand. Lock dependency versions in `vcpkg.json`.


## Tooling & developer iteration loop

* **Debug-draw is a first-class system.** Build one immediate-mode debug-line/shape renderer in M1.
* **Render graph / pass dependency management (M1).**
* **Two job systems, not four.** Jolt's `JobSystemThreadPool` (M2) stays dedicated to physics. Everything else uses one general-purpose task scheduler (enkiTS) built at M0.
* **Shader toolchain:** HLSL + DXC, targeting SPIR-V.
* **Pipeline cache, persisted to disk (M0).**
* **Profiler of record:** Tracy.
* **RenderDoc-capturable, confirmed at M0.**
* **Unit tests for pure-logic systems (Catch2 or doctest, vendored via vcpkg).**
* **Minimum viable failure-mode handling.** `VK_ERROR_DEVICE_LOST` and out-of-memory conditions log the failure and exit cleanly.


## Context & Engineering pillars

##### Target Hardware

**Recommended (dev reference machine):** RTX 2070 Super, Ryzen 5700G, 16GB RAM — this is what Tier-1/Tier-2 quality (RT shadows, ReSTIR GI) is built and profiled against, at 60 FPS.

**Floor (Tier-0 minimum spec — redefined v63, was implicitly "the 2070S" through v62):** the whole same-generation GPU class rather than one specific card — **RTX 2000-series** (binding case: RTX 2060, 6GB VRAM), **RX 6000-series** (binding case: RX 6600, 8GB VRAM), or **Intel Arc equivalent** (binding case: Arc A580, 8GB VRAM). The tightest common constraint across that set is **6GB VRAM** (from the RTX 2060) — that's the number every Tier-0 memory budget below is built against, not the higher 8GB figures from the other two families. CPU floor: a 6-core/12-thread chip contemporaneous with that GPU generation (e.g. Ryzen 5 3600 or Intel i5-10400) — noticeably weaker than the 8-core 5700G used as the dev reference. **Floor target: 30 FPS (33.3ms/frame)**, not 60 — this doubles the Tier-0 time and (via lower baseline render settings) VRAM headroom versus the old implicit 60 FPS floor, which is the room the M13 SLM budget below draws from.

Features not universally supported gate behind a capability check with a Tier-0 fallback.

1. **Procedural variety over authored content, everywhere it's viable.** 2. **One system, fed from multiple places — not one system per feature.**
2. **Cost scales with what's near/visible, not with total world/population size.**
3. **Best-looking-for-the-cost, not maximal.**
4. **Reuse proven, battle-tested defaults for infrastructure that isn't the game's creative focus.**
5. **Tier your ambition, don't assume it.**


## Day 0 — Architecture risk spikes

* **Spike A — Determinism:** Write a standalone console program with fixed literal seed. XOR particle states into a running 64-bit hash. Run twice, alternate compilation flags. If divergence occurs, M2.8 uses snapshot-sync primary.
* **Spike B — World precision:** Minimal SDL3 window + Vulkan swapchain. Authoritative position as `glm::dvec3`. GPU processes `renderPos = (glm::vec3)(entityPos - cameraPos)`. Track `worldOriginOffset` for rebasing at >500.0 units.
* **Spike C — Tier fallback switch:** Query ray-tracing-pipeline via `vkb::PhysicalDeviceSelector`. Add runtime toggle forcing `supportsRtPipeline = false` to verify pure rasterized fallback.
* **Spike D — Destruction at scale:** Jolt boxes held by `FixedConstraint`. Sever constraints, run BFS graph search. Measure time.


## Performance & resource budgets

Target: stable **30 FPS (33.3ms/frame) on Tier-0 minimum-spec hardware** (redefined v63 — was 60 FPS/16.6ms through v62; Recommended hardware still targets 60 FPS, this line governs the floor). Tier-0 budget below is a straight 2x of the old 16.6ms envelope — matches Tier-0 already running the lightest render path (no RT, static baked lighting per the Tier 0/1/2 ladder), so the extra time is genuine slack, not oversubscription:

* Render: ~14.0 ms
* Physics: ~5.0 ms
* AI: ~5.0 ms
* Animation: ~3.0 ms
* Audio: ~1.0 ms
* Headroom: ~5.3 ms
* Memory (redefined v63 for the 6GB VRAM floor, down from the old 8GB-card assumption): **~4.0GB VRAM engine baseline (Single-viewport)** at Tier-0's no-RT/baked-lighting quality — leaves ~1.5–1.8GB headroom on a 6GB floor card after ~0.3–0.5GB typical OS/driver reserve, which is what M13's SLM budget draws from. **~3.3GB VRAM (Split-screen)**, same ratio as the old single/split split. These are design-time estimates, not profiled numbers — verify against an actual Tier-0-class card once the render path exists, same caveat this doc gives other unverified estimates. 12.0GB System RAM unchanged (both floor CPUs above pair with 16GB system RAM normally).

Numerical method budget:

* Scent Dispersion (M5.3) / Fear Diffusion (M5.4): Coarse 2D grid, updated every 3-6 ticks.
* Regional SEIR Curve (M8): Coarse per-region, updated once per in-game hour.
* Faction Population ODEs (M8.5): Per-faction scalars, updated per in-game hour.
* Settlement Truss Solve (M8.6): Small node count, event-driven.
* Groundwater PDE (M10): Shared 2D flooding heightmap, updated every 6 ticks.

---
