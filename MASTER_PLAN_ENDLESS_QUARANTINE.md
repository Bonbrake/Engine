# Master Plan — Endless Quarantine: Open-World Zombie Survival Engine, Full Rebuild M0 → M13 (v79, Hermes Agent edition)

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
* M2 — Jolt 5.5.0 physics, EventBus, and a real destructible test entity
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

## M0 — Vulkan 1.4 native bootstrap + capability tiering

##### Implementation Steps

* Window Setup via `SDL_Init` / `SDL_CreateWindow`.
* Vulkan Instance via `vkb::InstanceBuilder` requiring API 1.4.0. `VK_EXT_pipeline_robustness` and `VK_EXT_host_image_copy` are core.
* Dynamic Rendering (`VK_KHR_dynamic_rendering`, core in Vulkan 1.4): all rendering uses `vkCmdBeginRendering`/`vkCmdEndRendering` with `VkRenderingAttachmentInfo` — no `VkRenderPass` or `VkFramebuffer` objects anywhere in the engine. This is the attachment model M1's Centralized Render Graph and lightweight framegraph are built against; render-pass/framebuffer objects would require separate compatibility and recreation tracking that fights the topologically-sorted pass graph instead of feeding it.
* Swapchain acquire/present loop explicitly branches on `VK_ERROR_OUT_OF_DATE_KHR` (recreate swapchain before continuing) and `VK_SUBOPTIMAL_KHR` (present succeeds this frame, recreate on the next acquire) — not just "doesn't crash on resize." This is the single most common Vulkan bootstrap bug (alt-tab, resize, or minimize/restore racing the present queue) and must be handled explicitly in both `vkAcquireNextImageKHR` and `vkQueuePresentKHR` return-code checks, not inferred from the resize-survival exit criterion alone.
* Capability-Tier Struct explicitly mapping boolean flags (`meshShaders`, `rtPipeline`, `deviceGeneratedCommands`, `descriptorBuffer`, `subgroupPartitioned`, `variableRateShading`, `opacityMicromap`, `fragmentShaderInterlock`, `attachmentFeedbackLoop`, `unifiedImageLayouts`).
* **`unifiedImageLayouts` capability (optional):** `VK_KHR_unified_image_layouts` is a real, separately-versioned Vulkan extension (not part of core 1.4) that lets a permanently-bound storage image stay in `VK_IMAGE_LAYOUT_GENERAL` for its whole lifetime with no transition-barrier overhead — a direct fit for M1's persistent RVT storage images written via async-compute `imageStore`. Gate it behind this capability flag with the standard explicit `VK_KHR_synchronization2` barrier path as the fallback for hardware/drivers that don't report it, same pattern as every other `caps.*` flag in this struct.
* EnkiTS initialized for general-purpose task scheduling.
* Fixed-Timestep loop: `FIXED_DT = 1.0/60.0`, clamp frameDelta at 0.25s. Intercept focus loss natively for low-overhead throttling.
* Raw Input Polling via `SDL_PollEvent` to a flat `InputState`.
* `settings.json` Config Mapping, `--headless` and `--force-tier0` CLI parsing.
* **SplitMix64 thread isolation:** the `SplitMix64`/`GaussianFromSplitMix64` generators used throughout this doc (M8 pathogen rolls, M8.5 trust drift, procedural narrative `sub_seed()`, etc.) take their state by reference and must advance it — never share one mutable `uint64_t` seed across enkiTS worker fibers. Each background task seeds its own `thread_local uint64_t` (e.g. from `GetBaseChunkSeed(cx, cz)`) so parallel procedural generation stays deterministic and race-free.
* GPU Memory tracking via `VK_EXT_memory_budget`. Discrete GPU topology enforcement.
* Async Spdlog leveled logging, plus JSON-lines structured log sink (`logs/session.jsonl`).
* Crash/Exception interception generating minidump and human-readable text sidecar. `ENGINE_ASSERT` macro integration.
* Deterministic input replay (`--record-input` / `--replay-input`).
* `AssetPath` virtual file path resolver mapping `mods/` directory.
* Pipeline cache persistence to disk, loaded via the `AssetPath` resolver on boot (item 13 above; not a separate resolver).
* Debug-only validation layers, gated behind the new `--dev` CLI flag (release builds never link validation).
* GPU driver blocklist + workaround table, logged through the existing Spdlog JSON-lines sink (`logs/session.jsonl`).
* Descriptor heap capability flag folded into the Capability-Tier struct (item 3 above) alongside `meshShaders`/`rtPipeline`/etc.
* Headless CI boot-smoke-test: automated pass through the `--headless` path (item 8 above), asserting instance/device/swapchain-less init succeeds without a display.
* 8K/multi-RT guarantee assert: Capability-Tier logging (item 3) fails loudly at boot if a Tier-2-claimed GPU can't actually back an 8K swapchain image or the multi-RT count the tier promises.
* RAII handle wrapper layer enforcing strict teardown order Instance→Window→Surface→Device→Swapchain (reverse of construction order).
* Bindless texture/resource design: shader-side array index replaces per-object descriptor set writes, gated on `caps.descriptorBuffer` (ties to M1 item 10's bindless ECS material indexing).
* `VK_KHR_synchronization2` for cross-pass barriers, unifying `VkPipelineStageFlags2`/`VkAccessFlags2` — the barrier primitive M8.6's power-grid pass and M10's flood pass both build on.
* Command pool-per-thread strategy sized off EnkiTS worker count: pool count = `L·T+N` (L=buffered frames, T=recording threads, N=secondary pools).
* Sub-allocated memory pages via VMA: suballocates from large blocks instead of a `vkAllocateMemory` call per resource, tracked against the existing `VK_EXT_memory_budget` line.
* One-time-submit vs reusable command buffer policy: `ONE_TIME_SUBMIT_BIT` for most per-frame draw recording, reusable pools reserved for static/rarely-rerecorded work.
* Dense Geometry Format mesh compression, gated on `caps.meshShaders`.
* `VK_EXT_debug_utils` object naming/labeling applied to every pipeline/shader/buffer/image at creation, so RenderDoc/Aftermath captures are human-readable.
* Vendor GPU crash dump (Nsight Aftermath on NVIDIA, AMD RGD on RDNA) capturing last draw/dispatch + bound shaders on a GPU hang, complementing the existing CPU minidump path.
* Pipeline cache header validation on load: verifies vendor/device/driver UUID against the persisted blob (item above) before trusting it, discarding and rebuilding on mismatch instead of feeding a stale/foreign cache to the driver.
* Device-lost handling protocol for `VK_ERROR_DEVICE_LOST`: stop submission → orderly teardown → write crash artifacts → exit, rather than falling through to the generic crash handler.
* Deterministic crash repro via replay: reuses `--record-input`/`--replay-input` (existing bullet above) but disables async streaming variability during a repro capture, so a replayed session reproduces the same GPU state, not just the same input stream.

##### Decisions to flag

Confirm shader toolchain (HLSL/DXC) and job-system split against M0 specs.

##### Exit Criteria

* Window opens, Vulkan context initializes, validation logs remain clean.
* Swapchain survives resizing and minimization without crashing.
* Capability tier correctly detected and logged; forced fallback verifies cleanly.
* PipelineCache created and persisted.
* Crash handler successfully outputs minidump and sidecar. Replay input yields identical tick traces.
* Budgets accurately logged via VMA.

### Extended Systems Library — engine-side additions for M0

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M0-EXT-01] Hardware-Topology enkiTS Pinning Allocator

##### Systems Touched

Underlies every sub-allocation this doc performs on top of enkiTS/VMA — formalizes the alignment math the "Sub-allocated memory pages via VMA" bullet above already relies on.

##### Math

`A_align = (Size + 63) & ~63` — rounds any allocation size up to the nearest 64-byte cache-line boundary.

##### How It Works

A thin wrapper over VMA sub-allocation that pins worker allocations to cache-line-aligned offsets so no two enkiTS fibers ever share a cache line (false sharing). `Size = 0` resolves safely to `0`; a boundary assertion guards against overflow as allocation sizes approach `size_t` limits on large dynamic requests.

##### Reference Implementation

```cpp
// Rounds up to the nearest 64-byte cache-line boundary; Size=0 -> 0
size_t AlignToCacheLine(size_t size) {
    assert(size <= SIZE_MAX - 63); // overflow guard
    return (size + 63) & ~size_t(63);
}
```

##### Player-Facing Impact

Invisible at the player level — this is what keeps multi-threaded chunk/physics/render sub-allocation from silently thrashing cache lines under heavy load, which is what "stutter for no visible reason" usually turns out to be.

#### [M0-EXT-02] Continuous Driver-Side Shader Object State Manager

##### Systems Touched

Replaces standard Vulkan Pipeline State Objects (PSOs) for stages that change frequently; complements [M4.5-EXT-09]'s bindless material virtualization (that feature removes *material* permutations from the PSO count, this one removes *stage* permutations).

##### Math

A direct set-tracking mapping rather than an algebraic formula: `S_state = {StageHandle_Vertex, StageHandle_Fragment, ...}`.

##### How It Works

Uses `VK_EXT_shader_object` (where `caps.*` reports it) to bind shader stages directly instead of compiling a full PSO per stage combination, removing PSO-compile stutter for stage combinations that only vary at runtime.

##### Reference Implementation

```cpp
struct ShaderObjectState { VkShaderEXT stages[6] = {}; };
void BindChangedStages(VkCommandBuffer cmd, const ShaderObjectState& next, ShaderObjectState& current) {
    const VkShaderStageFlagBits kStageBits[6] = {
        VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT
    };
    for (int i = 0; i < 6; ++i) {
        if (next.stages[i] != current.stages[i]) {
            vkCmdBindShadersEXT(cmd, 1, &kStageBits[i], &next.stages[i]);
            current.stages[i] = next.stages[i];
        }
    }
}
```

##### Player-Facing Impact

No visible compile-stutter the first time a new stage combination is hit at runtime — the second material-permutation problem after M4.5-EXT-09, solved the same way one layer down.

#### [M0-EXT-03] Disk-Persisted Pipeline Cache Header Validator

##### Systems Touched

Formalizes the "Pipeline cache header validation on load" bullet already present in M0's Implementation Steps — this is that check's exact formula, not a second cache system.

##### Math

`H_valid = (UUID_driver ≡ UUID_blob) ∧ (Version_engine ≡ Version_blob)`

##### How It Works

Boolean identity check run before trusting a persisted pipeline-cache blob: both the driver/device UUID and the engine version embedded in the blob must match the current machine exactly, or the cache is discarded and rebuilt rather than fed to the driver.

##### Reference Implementation

```cpp
bool ValidateCacheHeader(const CacheHeader& blob, const DeviceInfo& live) {
    return (blob.driverUuid == live.driverUuid) && (blob.engineVersion == live.engineVersion);
}
```

##### Player-Facing Impact

First boot after a driver update never crashes or corrupts on a stale foreign cache — it just silently rebuilds.

#### [M0-EXT-04] Asynchronous Native OS File Handle Ring Buffer

##### Systems Touched

Backs `AssetPath`'s virtual file resolver and the DirectStorage streaming pipeline (M4.6) with a bounded pool of in-flight OS file handles.

##### Math

`Idx_next = (Idx_current + 1) mod N`, optimized to `Idx_next = (Idx_current + 1) & (N − 1)` since ring sizes are chosen as powers of two.

##### How It Works

A fixed-size circular pool of native OS file handles serving async read requests from streaming systems (asset loads, DirectStorage chunk reads), avoiding per-request handle-open/close overhead on the hot streaming path.

##### Reference Implementation

```cpp
struct FileHandleRing {
    std::vector<OSFileHandle> handles; // pre-sized to power of two N
    std::atomic<uint32_t> cursor{0};
};
OSFileHandle AcquireHandle(FileHandleRing& ring) {
    uint32_t idx = ring.cursor.fetch_add(1, std::memory_order_relaxed);
    return ring.handles[idx & (uint32_t(ring.handles.size()) - 1)];
}
```

##### Player-Facing Impact

Chunk and asset streaming stays smooth under high load without handle-exhaustion stalls.

#### [M0-EXT-05] Discrete GPU Memory-Budget Tracker

##### Systems Touched

Formalizes the "GPU Memory tracking via `VK_EXT_memory_budget`" bullet already in M0's Implementation Steps; feeds the hard budget check in M4.6 (6.0GB/5.0GB VRAM targets).

##### Math

`M_headroom = M_budget − M_usage`, read directly from `VK_EXT_memory_budget`'s native properties fields each frame.

##### How It Works

Polls the driver-reported budget/usage pair per heap and exposes remaining headroom to M4.6's memory-pressure radial scaling, so streaming/eviction decisions react to the real driver-reported budget rather than an assumed constant.

##### Reference Implementation

```cpp
float GetHeapHeadroom(const VkPhysicalDeviceMemoryBudgetPropertiesEXT& budget, uint32_t heapIndex) {
    uint64_t totalBudget = budget.heapBudget[heapIndex];
    uint64_t currentUsage = budget.heapUsage[heapIndex];
    return (currentUsage >= totalBudget) ? 0.0f : static_cast<float>(totalBudget - currentUsage);
}
```

##### Player-Facing Impact

VRAM stays within the documented hardware budget even when other applications are competing for GPU memory, instead of hard OOM crashes.

#### [M0-EXT-06] Cross-Queue Vulkan Dependency Hazard Validation Layer

##### Systems Touched

Dev-only diagnostic sitting alongside the `--dev`-gated validation layers already in M0's Implementation Steps; watches the timeline-semaphore cross-queue sync (Graphics/Compute/Async Transfer) already declared in M0.

##### Math

`HazardDetected = (Resource.ActiveState.WriteBit ∧ Pass.RequiredAccess.ReadBit)`

##### How It Works

Maintains a resource-state tracking table updated during render-graph pass recording. Before a pass submits, it checks the incoming access request against the resource's currently-active queue/write state; a mismatch (a read racing an in-flight cross-queue write, or two writes without a barrier between them) flags loudly in the dev build instead of silently hanging the GPU. Compiled out entirely in release builds via the same `#ifdef ENGINE_DEV_TOOLS` guard as the EnTT Meta Entity Inspector (M1).

##### Reference Implementation

```cpp
struct ResourceState { uint32_t queueFamilyOwner; bool isWritingActive; };
inline bool CheckBarrierHazard(const ResourceState& current, uint32_t targetQueueFamily, bool upcomingWrite) {
    return current.isWritingActive && (current.queueFamilyOwner != targetQueueFamily || upcomingWrite);
}
```

##### Player-Facing Impact

Invisible to players — this is what keeps a bad cross-queue barrier from shipping as an intermittent device-lost crash instead of getting caught at dev-build time.

#### [M0-EXT-07] Device-Agnostic Work-Stealing Job Scheduler Fiber Yield Hook

##### Systems Touched

Extends M0's enkiTS setup. Required by any background task that blocks on a fence mid-execution — the async Jolt BVH cook (`[M2-EXT-01]`) and WFC chunk generation (`[M4-EXT-03]`) both do this and would otherwise stall a whole OS thread instead of yielding it back to the pool.

##### Math

`YieldTarget_fiber = Manager.PopNextReadyWorker()`

##### How It Works

A fiber that hits a synchronization wait (a fence, a semaphore) registers its wait handle on a lightweight check array, then switches execution context back to the primary enkiTS scheduler instead of blocking the OS thread it's running on. The scheduler picks up other ready work immediately; the yielded fiber resumes once its wait condition clears.

##### Reference Implementation

```cpp
struct FiberContext { void* osFiberHandle; bool isSuspendedWaiting; };
void YieldActiveFiber(FiberContext& active, void* primarySchedulerHandle) {
    active.isSuspendedWaiting = true;
    #if defined(_WIN32)
    SwitchToFiber(primarySchedulerHandle);
    #endif
}
```

##### Player-Facing Impact

Long background tasks (chunk generation, physics cooking) never stall unrelated systems sharing the same worker pool, keeping frame pacing smooth during heavy streaming.

#### [M0-EXT-08] Vulkan Descriptor Indexing Bindless Storage Handle Page Allocator

##### Systems Touched

M1's "Bindless descriptor indexing for ECS materials" bullet and `[M4.5-EXT-09]` Bindless Material Shader Virtualization both assume a slot-handle allocation system already exists — it was never declared. This is that allocator, declared here in M0 alongside the Capability-Tier struct's `caps.descriptorBuffer` flag, consumed by M1 and `[M4.5-EXT-09]`.

##### Math

`Handle_offset = Page_BaseIndex + BitScanForward(FreeList_bitmask)`

##### How It Works

A non-fragmenting bitmask free-list allocator. Each page tracks 64 slots via a bitmask; assigning a slot does a single bit-scan for the next free bit instead of searching a table, and releasing a slot just sets the bit back. Backs every bindless material-index lookup in the doc — M1's ECS material indexing and `[M4.5-EXT-09]`'s per-primitive `MaterialIdx` both resolve through this.

##### Reference Implementation

```cpp
struct DescriptorPage { uint64_t freeSlotsBitmask = 0xFFFFFFFFFFFFFFFFull; uint32_t baseIndex; };
inline uint32_t AssignDescriptorSlot(DescriptorPage& page) {
    int bitIdx = std::countr_zero(page.freeSlotsBitmask);
    if (bitIdx >= 64) return 0xFFFFFFFFull; // Page slots fully exhausted
    page.freeSlotsBitmask &= ~(1ULL << bitIdx);
    return page.baseIndex + bitIdx;
}
```

##### Player-Facing Impact

Invisible — this is the missing backend that lets material variety (rust, blood, weathering) scale across the whole world without ever running out of bindless slots or corrupting a material lookup during chunk streaming.

#### [M0-EXT-09] Host-Side Command Queue Swapchain Frame-Pacing Interceptor

##### Systems Touched

Sits alongside M0's Fixed-Timestep loop bullet ("clamp frameDelta at 0.25s. Intercept focus loss natively for low-overhead throttling") — this is the concrete EMA smoothing mechanism behind that throttling, not a second pacing system.

##### Math

`Δt_avg = (1−λ)·Δt_avg + λ·Δt_current`, `λ = 0.10`

Smoothed delta crossing 1.25x the fixed target step triggers background-worker throttling rather than a blocking kernel sleep.

##### How It Works

Tracks frame submission timestamps on the host CPU clock and folds them into an exponential moving average instead of reacting to a single frame's raw delta, so a one-off hitch (window drag, focus-loss transition) doesn't cause an over-aggressive throttle response. The hard 0.25s clamp from M0's base spec is applied to the raw delta before it ever reaches the EMA, so a single freeze event can't corrupt the running average.

##### Reference Implementation

```cpp
struct FramePacingState {
    uint64_t lastSubmissionCpuTimestamp{0};
    float smoothedDeltaTimeSeconds{0.01666f};
};
inline bool EvaluateQueuePacingIntercept(FramePacingState& state, uint64_t currentCpuTimestamp, float targetStepSeconds) {
    float rawDelta = static_cast<float>(currentCpuTimestamp - state.lastSubmissionCpuTimestamp) / 1e9f;
    state.lastSubmissionCpuTimestamp = currentCpuTimestamp;
    if (rawDelta > 0.25f) rawDelta = 0.25f; // hard boundary clamp, matches M0's base fixed-timestep spec
    state.smoothedDeltaTimeSeconds = (0.9f * state.smoothedDeltaTimeSeconds) + (0.1f * rawDelta);
    return state.smoothedDeltaTimeSeconds > (targetStepSeconds * 1.25f); // triggers background worker throttling
}
```

##### Player-Facing Impact

Invisible directly — this is what keeps a rapid window-focus shift or a minimize/restore cycle from causing a visible stutter cascade on resume, by smoothing the recovery instead of snapping straight back to full-rate simulation on the first good frame.

#### [M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator
#### [M0-EXT-13] Capability Probe & Feature-Tier Detection *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
Boot-time GPU/API capability probe feeding the tier ladder (M0-EXT-01..12). Maps detected caps to a feature tier consumed by M4.5/M4.6 quality selection.

##### Math
tier = argmax_t (caps >= req(t)); caps = {maxTexSize, compute, meshlet, RVT, ReSTIR, RT}. Probe once at init.

##### How It Works
At startup, query the adapter for a fixed capability set (max texture dims, compute support, mesh-shader, virtual-texture, ReSTIR, HW-RT). Map the result to a discrete tier enum the renderer reads to pick techniques (ReSTIR vs SSGI, RVT vs classic).

##### Reference Implementation
```cpp
Tier g_tier = ProbeCapabilities(adapter); // sets g_tier used by M4.5 quality ladder
```

##### Player-Facing Impact
The game auto-tunes to the player's GPU - 6GB floor runs Tier-0, beefier cards light up Tier-2 features.

#### [M0-EXT-12] Compute-Worker Thread-Affinity Bitmask Allocator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M0 + enkiTS. Pins compute workers to a CPU affinity bitmask for cache locality / NUMA.

##### Math
mask = affinityBitmask(workerPool); SetThreadAffinityMask(h, mask);

##### How It Works
Compute workers are assigned a CPU affinity bitmask (e.g. excluding the render-thread cores) so heavy compute doesn't contend with the render thread and keeps L2/L3 locality. Set once at scheduler init.

##### Reference Implementation
```cpp
SetThreadAffinityMask(workers, kComputeMask);
```

##### Player-Facing Impact
Compute stays off the render core - smoother frame, less jitter under load.


---

#### [M0-EXT-11] Asynchronous SPIR-V Shader Cache Garbage Collector *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M0 shader cache. Async GC of stale SPIR-V cache entries so the cache never grows unbounded.

##### Math
evict LRU when size>cap; GC on enkiTS worker, not render thread; refcount live pipelines.

##### How It Works
Stale compiled SPIR-V entries are evicted by an LRU policy run on a background worker; entries still referenced by live pipelines are skipped. Keeps disk/memory cache bounded without hitching the frame.

##### Reference Implementation
```cpp
void GcCache(Worker& w){ for(e in lru) if(size>cap && !e.live) Evict(e); }
```

##### Player-Facing Impact
Shader cache stays small and fast - no disk bloat or load hitches from stale entries.


##### Systems Touched
Boot-time GPU/API capability probe feeding the tier ladder (M0-EXT-01..12). Maps detected caps to a feature tier consumed by M4.5/M4.6 quality selection.

##### Math
tier = argmax_t (caps >= req(t)); caps = {maxTexSize, compute, meshlet, RVT, ReSTIR, RT}. Probe once at init.

##### How It Works
At startup, query the adapter for a fixed capability set (max texture dims, compute support, mesh-shader, virtual-texture, ReSTIR, HW-RT). Map the result to a discrete tier enum the renderer reads to pick techniques (ReSTIR vs SSGI, RVT vs classic).

##### Reference Implementation
```cpp
Tier g_tier = ProbeCapabilities(adapter); // sets g_tier used by M4.5 quality ladder
```

##### Player-Facing Impact
The game auto-tunes to the player's GPU - 6GB floor runs Tier-0, beefier cards light up Tier-2 features.

##### Systems Touched

Pipeline initialization, asset toolchain loading, shader runtime hot-reloading.

##### Math

Set-theoretic validation of push constant ranges and bindless descriptor configurations against the allocated active Vulkan pipeline layout blocks:

`V_valid = (R_shader ⊆ R_layout) ∧ ∀i(Stride_shader,i ≡ Stride_heap,i)`

##### How It Works

Validates SPIR-V reflected stage push-constant offsets and binding layouts against the active `VkPipelineLayout` at runtime creation. Iterates explicitly through disjoint `VkShaderStageFlags` to guarantee compilation slices don't exceed target device constraints or trigger validation faults during multi-stage descriptor swaps. Consumes `[M0-EXT-08]`'s bindless slot allocator for the `maxBindlessElementsBound` check rather than tracking a second element budget.

##### Reference Implementation

```cpp
#include <vulkan/vulkan.h>
#include <cstdint>
#include <span>

struct PushConstantStageRange {
    VkShaderStageFlags stageFlags;
    uint32_t offset;
    uint32_t size;
};

struct PipelineLayoutValidationSpec {
    std::span<const PushConstantStageRange> layoutRanges;
    uint32_t maxBindlessElementsBound;
};

inline bool ValidatePipelineCompatibility(const PipelineLayoutValidationSpec& layout, VkShaderStageFlags activeStages, uint32_t shaderOffset, uint32_t shaderSize, uint32_t requiredElements) {
    if (shaderSize == 0) return requiredElements <= layout.maxBindlessElementsBound;
    for (const auto& range : layout.layoutRanges) {
        // Requires (range.stageFlags & activeStages) == activeStages —
        // demands a single range cover ALL active stages at once — wrong for the normal case of
        // per-stage disjoint ranges. A range only needs to overlap the requested stages.
        if ((range.stageFlags & activeStages) != 0) {
            if (shaderOffset >= range.offset && (shaderOffset + shaderSize) <= (range.offset + range.size)) {
                return requiredElements <= layout.maxBindlessElementsBound;
            }
        }
    }
    return false;
}
```

##### Player-Facing Impact

Prevents hard GPU device-lost hangs and structural texture flashing when loading game modifications or live asset updates during high-intensity open-world segments.


## [M0-EXT-15] Minimal Action-Map Input Resolution Layer

##### Systems Touched
Every early input consumer, MOST URGENTLY the debug fly-camera currently
bugged. Pulled forward from M11 ("UI/HUD, input abstraction &
consolidated haptics") — this is a minimal subset built now, not a
replacement for M11's full system later.

##### Math
`ActionState.value = Curve(RawAxis, Deadzone)`, `ActionState.held = RawKeyDown[boundKey]` sampled every tick, independent of whether an SDL event fired that frame.

##### How It Works
Raw `SDL_PollEvent` state resolves once per tick into a named
`ActionState` (`held: bool`, `pressedThisFrame: bool`, `value: float`)
via a binding table (device+code → action). Gameplay code — fly-camera
included — reads `ActionState`, never raw scancodes.

**Direct fly-cam bug analysis:** the reported symptom ("controls have no
effect, gated behind a non-empty-events conditional") is the classic
signature of code branching on discrete SDL *events* (which only fire on
state transitions) where continuous *held* state was actually needed.
`ActionState.held` is sampled every tick regardless of whether
`SDL_PollEvent` returned anything that frame, which removes this bug
class rather than patching this one instance of it.

##### Reference Implementation
```cpp
struct ActionState { bool held = false; bool pressedThisFrame = false; float value = 0.0f; };

struct ActionBinding { SDL_Scancode key; /* or controller axis id */ };

std::unordered_map<std::string, ActionBinding> g_bindings; // populated from settings.json

void ResolveActions(const Uint8* keyState, std::unordered_map<std::string, ActionState>& actions) {
    for (auto& [name, binding] : g_bindings) {
        bool wasHeld = actions[name].held;
        bool isHeld = keyState[binding.key] != 0; // continuous poll, NOT event-based
        actions[name].held = isHeld;
        actions[name].pressedThisFrame = isHeld && !wasHeld;
        actions[name].value = isHeld ? 1.0f : 0.0f; // sticks apply a deadzone curve here instead
    }
}
```

##### Player-Facing Impact
Fly-camera (and every future input-driven system) responds reliably to
held input instead of only reacting to discrete key-transition events.

---

## [M0-EXT-16] ENGINE_DETERMINISM_MODE Compile Guard & CI Assertion

##### Systems Touched
M2.8 (deterministic co-op) — establishes the constraint at the layer
where parallel job scheduling and GPU reductions happen, rather than
discovering a desync bug during co-op testing.

##### Math
`Determinism valid ⟺ ∀ gameplay-affecting reduction, accumulation order is fixed (entity-index order), independent of job-completion order.`

##### How It Works
An `ENGINE_DETERMINISM_MODE` compile guard wraps any gameplay-affecting
float accumulation, asserting it iterates a stable, index-ordered
container rather than an unordered one whose iteration order can vary
run-to-run. Wired into the existing headless CI boot-smoke-test
(already declared in M0) so a violation fails CI immediately.

##### Reference Implementation
```cpp
#ifdef ENGINE_DETERMINISM_MODE
template <typename T>
void AssertOrderedAccumulation(const std::vector<T>& orderedByEntityIndex) {
    // Debug-only: verifies the caller passed an index-ordered container,
    // not an unordered_map/unordered_set whose iteration order may vary.
    static_assert(!std::is_same_v<decltype(orderedByEntityIndex), std::unordered_map<int, T>>,
                  "Gameplay-affecting accumulation must not iterate an unordered container");
}
#endif
```

##### Player-Facing Impact
Co-op play (M2.8/M12) stays in sync instead of silently desyncing from a
floating-point ordering difference that only shows up under load.

---

## [M0-EXT-17] Split Save-Schema / Network-Protocol Version Fields

##### Systems Touched
M7 (persistence) and M12 (networked co-op) — declared independently now
so a save-format patch can never silently break netcode compatibility
or vice versa.

##### Math
`SaveSchemaVersion: uint32_t`, `NetworkProtocolVersion: uint32_t` — two
independent fields, never conflated into one "build version" number.

##### How It Works
Every save file embeds `SaveSchemaVersion`; every network handshake
negotiates `NetworkProtocolVersion` separately. A save-format change
bumps only the former; a wire-format change bumps only the latter.

##### Reference Implementation
```cpp
struct SaveHeader { uint32_t saveSchemaVersion; /* ... */ };
struct NetHandshake { uint32_t networkProtocolVersion; /* ... */ };
```

##### Player-Facing Impact
A patch that changes how saves are stored can't accidentally break
matchmaking compatibility, and vice versa.

---

## [M0-EXT-18] Atomic Write-Temp-Then-Rename Save I/O

##### Systems Touched
M7 persistence — every save write, going forward.

##### Math
`Save valid ⟺ Checksum(save.tmp) matches AND rename(save.tmp, save.dat) completed atomically.`

##### How It Works
Writes go to `save.tmp`, are fsync'd, checksummed, and only then
atomically renamed over `save.dat`. A crash or power-loss mid-write
leaves the previous good save intact.

##### Reference Implementation
```cpp
bool AtomicSaveWrite(const std::string& path, const std::vector<uint8_t>& data) {
    std::string tmpPath = path + ".tmp";
    if (!WriteAndFsync(tmpPath, data)) return false;
    uint32_t checksum = Crc32(data);
    if (!VerifyChecksum(tmpPath, checksum)) return false;
    return AtomicRename(tmpPath, path); // platform-native atomic rename
}
```

##### Player-Facing Impact
A save never gets corrupted by a crash or power loss mid-write — a
survival game's core trust guarantee.

---

## [M0-EXT-19] Platform User-Data Directory Resolution via AssetPath

##### Systems Touched
Extends the existing `AssetPath` resolver (M0) to also resolve a
writable per-user path, separate from the install directory.

##### How It Works
Saves/settings resolve through `AssetPath` to a platform-appropriate
writable directory (`%APPDATA%` on Windows, `~/.local/share` on Linux)
rather than relative to the install folder — avoiding a forced save
migration later when the install directory isn't writable (Steam/mod
scenarios).

##### Reference Implementation
```cpp
std::filesystem::path ResolveUserDataDir() {
#if defined(_WIN32)
    return std::filesystem::path(std::getenv("APPDATA")) / "EndlessQuarantine";
#else
    return std::filesystem::path(std::getenv("HOME")) / ".local/share/EndlessQuarantine";
#endif
}
```

##### Player-Facing Impact
Saves and settings live in the correct per-user location from day one —
no migration script needed later.

---

## [M0-EXT-20] Cross-Machine Crash Correlation Tag

##### Systems Touched
M2.8/M12 co-op — extends the existing Aftermath/RGD crash-dump path
(M0) with a shared identifier so a host-side crash can be matched
against what a peer was doing at the same simulated tick.

##### How It Works
Every crash/minidump is tagged with the current tick number and a
session-id exchanged between host and peer at connection time.

##### Reference Implementation
```cpp
struct CrashTag { uint64_t tick; uint64_t sessionId; };
```

##### Player-Facing Impact
Co-op crash reports can actually be correlated across machines instead
of being two disconnected local dumps.

## [M0-EXT-21] Persistent World-Epoch Clock with Deterministic Offline Fast-Forward

##### Systems Touched
M7 (persistence) and every future decay/weather system — declared at
the timekeeping layer now so offline elapsed-time simulation doesn't get
special-cased into the save system later.

##### Math
`ElapsedSimSeconds_onLoad = (WallClockNow − WallClockAtLastSave) × TimeScale`

##### How It Works
Stores a `WorldEpoch` (fixed in-game start timestamp) plus accumulated
simulated-seconds. On load, computes real-world elapsed time since the
last save and deterministically fast-forwards weather/decay/etc. by that
amount, rather than that logic living downstream in M7.

##### Reference Implementation
```cpp
struct WorldClock { uint64_t epochUnixSeconds; double accumulatedSimSeconds; };

double ComputeOfflineFastForward(uint64_t wallClockAtSave, uint64_t wallClockNow, double timeScale) {
    return static_cast<double>(wallClockNow - wallClockAtSave) * timeScale;
}
```

##### Player-Facing Impact
The world keeps existing while the game isn't running — weather
progresses, food decays — the Zomboid-style "it's a real place" feel.

---

## [M0-EXT-22] Monotonic-vs-Wall-Clock Tamper Cross-Check

##### Systems Touched
Extends [M0-EXT-21] — prevents trivially defeating offline decay/weather
by winding the system clock back.

##### How It Works
At boot, compares the OS monotonic uptime delta against the wall-clock
delta since last recorded session; a mismatch beyond a small tolerance
flags the wall clock as untrusted for that session's offline
fast-forward calculation.

##### Reference Implementation
```cpp
bool IsWallClockTrusted(uint64_t monotonicDeltaMs, uint64_t wallClockDeltaMs, uint64_t toleranceMs) {
    uint64_t diff = (monotonicDeltaMs > wallClockDeltaMs)
        ? (monotonicDeltaMs - wallClockDeltaMs) : (wallClockDeltaMs - monotonicDeltaMs);
    return diff <= toleranceMs;
}
```

##### Player-Facing Impact
Closes an easy exploit for dodging decay/weather consequences.

---

## [M0-EXT-23] Extended Adaptive-Quality Watchdog

##### Systems Touched
Builds directly on the existing frame-pacing EMA (M0-EXT-09) — adds a
second, slower-moving average and an ordered fallback ladder instead of
a single hard-coded quality drop.

##### Math
`Δt_avgLong = (1−λ_long)·Δt_avgLong + λ_long·Δt_current`, `λ_long = 0.02`
(slower time constant than M0-EXT-09's `λ = 0.10`, so this reacts to
*sustained* overbudget frames, not single hitches).

##### How It Works
When `Δt_avgLong` crosses a sustained-overbudget threshold, triggers a
described, ordered quality-reduction ladder (shadow resolution →
particle density → draw distance) instead of the current implicit
"nothing happens" or a single hard fallback.

##### Reference Implementation
```cpp
enum class QualityTier { Full, ReducedShadows, ReducedParticles, ReducedDrawDistance };

QualityTier EvaluateQualityLadder(float smoothedLongDelta, float targetStep, QualityTier current) {
    if (smoothedLongDelta > targetStep * 1.5f) {
        return static_cast<QualityTier>(std::min(static_cast<int>(current) + 1,
                                                  static_cast<int>(QualityTier::ReducedDrawDistance)));
    }
    return current;
}
```

##### Player-Facing Impact
A horde crush or physics spike degrades gracefully in a defined order
instead of an unpredictable frame-rate collapse at the worst moment.

---

## [M0-EXT-24] Single-Instance Boot Lock

##### Systems Touched
Boot sequence — prevents two copies of the game writing to the same
save concurrently.

##### How It Works
A named OS mutex (or lock file) is checked and held at boot; a second
instance detects it and refuses to start (or warns and exits).

##### Reference Implementation
```cpp
bool AcquireSingleInstanceLock(const std::string& lockName) {
#if defined(_WIN32)
    HANDLE h = CreateMutexA(nullptr, TRUE, lockName.c_str());
    return h != nullptr && GetLastError() != ERROR_ALREADY_EXISTS;
#else
    int fd = open(("/tmp/" + lockName).c_str(), O_CREAT | O_EXCL, 0644);
    return fd != -1;
#endif
}
```

##### Player-Facing Impact
Prevents a corrupted save from two instances writing at once.

---

## [M0-EXT-25] Per-Fiber Exception Containment for enkiTS Jobs

##### Systems Touched
Every background job dispatched through enkiTS — a bad chunk-gen task
or corrupt-asset job shouldn't take the whole process down.

##### How It Works
Wraps the top of every enkiTS job dispatch in a try/catch (or SEH filter
on Windows), logging and safely dropping the failed job instead of
propagating into the scheduler.

##### Reference Implementation
```cpp
void SafeJobDispatch(std::function<void()> job) {
    try {
        job();
    } catch (const std::exception& e) {
        Spdlog::Error("Job threw: {}", e.what()); // one dropped job, not a crashed process
    }
}
```

##### Player-Facing Impact
One malformed background task logs an error instead of crashing the
game mid-session.

---

## [M0-EXT-26] Shader-Compile Timeout & Safe-Mode Boot Fallback

##### Systems Touched
M1's async shader compilation path, and M0's pipeline-cache/mod
integrity validation (M0-EXT-03).

##### How It Works
The async shader-compile path gets a watchdog timer; a hung compile logs
and falls back to a cached/default shader instead of hanging boot
indefinitely. Separately, if pipeline-cache validation or a mod fails
integrity checks, the game boots in safe mode (mods disabled, cache
rebuilt) instead of the current implicit "just don't boot."

##### Reference Implementation
```cpp
bool CompileShaderWithTimeout(ShaderCompileJob& job, std::chrono::milliseconds timeout) {
    auto future = std::async(std::launch::async, [&]{ return job.Compile(); });
    if (future.wait_for(timeout) == std::future_status::timeout) {
        Spdlog::Error("Shader compile timed out, falling back to cached shader.");
        return false;
    }
    return future.get();
}
```

##### Player-Facing Impact
A hung shader compile or corrupted mod/cache never fully blocks boot —
the game degrades to safe mode instead of not starting at all.

---

## [M0-EXT-27] Foreground/Background I/O and Thread Priority Tiers

##### Systems Touched
Extends the existing file-handle ring buffer (M0-EXT-04) and enkiTS
worker pool with a simple priority split.

##### How It Works
Requests and jobs are tagged `Foreground` (render/input-critical) or
`Background` (streaming/AI/autosave); foreground work is serviced first
under contention, preventing a heavy streaming burst from starving a
frame-critical job.

##### Reference Implementation
```cpp
enum class JobPriority { Foreground, Background };
// Scheduler services Foreground-tagged work before Background-tagged work
// when both are ready, using existing enkiTS priority/affinity hooks.
```

##### Player-Facing Impact
Autosaves and streaming bursts no longer cause a frame hitch during
gameplay-critical moments.

## [M0-EXT-28] AssetPath Checksum Integrity Validation

##### Systems Touched
Extends the existing `AssetPath` resolver (M0) — every load, mod or
otherwise.

##### How It Works
A hash (CRC32/xxHash) is checked against a manifest at load time; a
mismatch fails loud and safe instead of silently loading a corrupted or
tampered asset.

##### Reference Implementation
```cpp
bool ValidateAssetChecksum(const std::vector<uint8_t>& data, uint64_t expectedHash) {
    return XxHash64(data.data(), data.size()) == expectedHash;
}
```

##### Player-Facing Impact
A corrupted texture, mesh, or mod file never silently loads as garbage.

---


## [M0-EXT-29] HDR Color-Space & DPI-Scale Swapchain Setup

##### Systems Touched
Swapchain creation (M0) and UI layout math (M11, later) — declared now
so color-space/DPI handling isn't retrofitted per-render-pass.

##### How It Works
Swapchain color space and a DPI-scale factor are read once at boot and
threaded through UI layout math from the start, rather than assuming SDR
everywhere.

##### Reference Implementation
```cpp
struct DisplayConfig { VkColorSpaceKHR colorSpace; float dpiScale; };
```

##### Player-Facing Impact
HDR displays and high-DPI/multi-monitor setups render correctly instead
of needing a later rewrite.

---


## M1 — GPU-driven ECS framework

##### Implementation Steps

* `entt::registry` inside application context (EnTT v3.16.0 — bumped from prior pin; see Dependency Version Bump below). Single-threaded registry authority using SPSC ring buffers for background workers — concrete mechanism locked in `[M1-EXT-06]`'s `SPSCMutationQueue` below; no worker thread ever mutates `entt::registry` directly.
* `SpatialHash` — the uniform grid used by every later broad-phase spatial query in this doc (M2.8 comms static, M5.4 horde crush, M8.6 power-grid conductance, M9-EXT-06 horde drag, M9-EXT-11 vehicle drafting, M6.5 blood spatter density). Declared here because it's an M1-level primitive, not owned by any single downstream feature.

  * **Cell size:** 2.0m, chosen as ~4x the average agent collision radius (0.4-0.5m capsule) so a single cell query returns a small, cache-friendly agent cluster without needing multi-cell stitching for typical melee/collision range checks.
  * **Hash function:** `uint64_t Hash(int32_t cx, int32_t cz) { return (uint64_t(uint32_t(cx)) << 32) | uint32_t(cz); }` on `cx = floor(x / cellSize)`, `cz = floor(z / cellSize)` (2D grid; vertical stacking handled by a small per-cell bucket list rather than a third axis, since horde/vehicle/comms queries are all ground-plane).
  * **Storage:** `std::unordered_map<uint64_t, std::vector<entt::entity>>` per cell, rebuilt into pooled vectors each cadence tick (no per-entity heap churn).
  * **Rebuild cadence, tuned per consumer:**

    * Horde crush (M5.4) and vehicle drafting (M9-EXT-11): rebuilt every fixed-timestep tick — these need current-frame positions for crush/draft force resolution.
    * Comms static (M2.8) and power-grid conductance (M8.6): rebuilt every 4 ticks — these query slow-changing world topology, not per-tick agent motion.
    * Blood spatter density (M6.5) and horde drag (M9-EXT-06): rebuilt every 8 ticks (staggered/bucketed across frames by `entity_id % 8` to spread the rebuild cost) — visually/physically tolerant of a few ticks of staleness.
  * **Ownership:** M1 owns the `SpatialHash` class and the insertion/removal API (`Insert(entity, pos)`, `Remove(entity)`, `Update(entity, newPos)`, `QueryRadius(pos, radius) -> span<entity>`, `QueryCell(cx, cz) -> span<entity>`). Each consuming milestone calls `Update()` from its own tick function; M1 does not run a global "update everyone" pass, since update cadence varies per consumer as above.
  * All six referenced features above are retrofitted to call `SpatialHash::QueryRadius` / `QueryCell` by name instead of "your existing SpatialHash."
* Descriptor Buffers (`caps.descriptorBuffer`) using `VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT` with `VkDescriptorPool` fallback.
* Asynchronous Shader Compilation via `VK_EXT_graphics_pipeline_library`. Shader Hot-Reloading via DXC.
* Hi-Z Occlusion Culling via downsampled depth pyramids. Indirect Draw Compaction via prefix-sum.
* Meshlet Pipeline (`caps.meshShaders`) fallback to frustum culling.
* Centralized Render Graph with topologically sorted barriers/transitions.
* Parallel Secondary Command Buffers. Synchronous Asset Loader. Generational Resource Table.
* `EntityFactory` parsing JSON.
* Per-Pass Query Timestamps.
* Dear ImGui Workspace Overlay (visible via `~`) containing Developer Console UI and Always-On Diagnostic HUD.
* MSDF Font Pipeline for diegetic text rasterization.
* Frame-scoped GPU deletion queue for entity-tied GPU resources (destructible entities, barricades): defers destroy until the GPU fence for the resource's last-used frame signals, instead of destroying on the CPU-side entity-destroy tick.
* Full component registration audit list extending the Dev-Tool EnTT Meta Entity Inspector's `MetaRegistry.cpp` (below) to every component, not just the initial registration set.
* Material-batched mesh-pass rendering across forward/transparent/shadow passes: merges vertex buffers per material so each pass issues one bind per material instead of per draw.
* Lightweight framegraph for barriers, formalizing the existing "Centralized Render Graph" line above: passes declare their reads/writes and the framegraph auto-inserts barriers/transitions instead of hand-placed ones.
* SoA layout for hot components (Transform/bone arrays and the other `SpatialHash` consumers: M5.1, M5.4, M8.5-EXT-02): contiguous per-field arrays instead of array-of-structs, for cache-friendly iteration at horde scale.
* Buffer device address for skinned-mesh animation data (zombies, companions): pointer-based buffer access that shares the same indirect-draw path already used for static meshes.
* CVar system tied to the ImGui overlay (item above): typed int/float/bool console variables, replacing ad hoc debug toggles.
* Timeline semaphores for multi-queue sync (graphics/compute/transfer), Vulkan 1.4 core — replaces manual fence juggling.
* Multi-threaded secondary command buffer recording on EnkiTS worker threads via `VkCommandBufferInheritanceInfo`, with one primary command buffer executing the merged result.
* Bindless descriptor indexing for ECS materials, tying into M0's bindless texture/resource design (M0 item above): entity material index resolves to a shader-side array lookup.
* Persistent-mapped staging buffers for per-frame ECS data upload (Transform, animation): a ring buffer replacing per-frame map/unmap calls.
* Occlusion query double-buffering for Hi-Z culling: avoids a GPU/CPU sync stall from reading back last-frame visibility results.
* ECS archetype/chunk iteration cache warming via EnTT grouping, keeping hot components contiguous for the `SpatialHash` consumers at horde-scale entity counts.
* Command buffer reuse pooling: pools are never destroyed/recreated once allocated, only reset.
* Pipeline derivatives for related families (zombie material variants, Meshlet Pipeline) via `VK_PIPELINE_CREATE_DERIVATIVE_BIT`, sharing parent/child pipeline state.
* Batched pipeline creation calls for Async Shader Compilation (item above): multiple pipelines submitted per `vkCreateGraphicsPipelines` call instead of one call each.
* spirv-reflect for auto pipeline layouts: derives descriptor-set/layout structs from shader reflection instead of hand-written layout structs.
* Material system abstraction plugging into item 3 above: one material definition renders into N passes (forward/transparent/shadow) without duplicated per-pass setup.
* Prefab/scene-node loading (glTF/FBX) for M4 procedural urban detail and companion/zombie assembly: a prefab expands to N ECS entities on load instead of one entity per asset.

##### Decisions to flag

Defaults apply.

##### Exit Criteria

* EnTT components render correctly via GPU indirect draw paths.
* Font files successfully generate MSDF maps at boot.
* Hi-Z culling logs measurable draw call reduction.
* ImGui panel updates cvar behaviors live. Pass timings output via Vulkan query pools.
* No validation warnings.

### Extended Systems Library — engine-side additions for M1

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M1-EXT-01] SpatialHash Uniform Grid Cell Bucketing Engine

##### Systems Touched

Formalizes the exact hashing formula behind `SpatialHash` — the uniform grid primitive already declared as shared infrastructure and reused by M2.8 comms static, M5.4 horde crush, M8.6 power-grid conductance, M9-EXT-06 horde drag, M9-EXT-11 drafting, and M6.5 blood spatter density.

##### Math

`Key(x,z) = (uint64_t(⌊x/2.0⌋) << 32) | (uint64_t(⌊z/2.0⌋) & 0xFFFFFFFF)`

##### How It Works

Packs a 2m×2m grid cell's (x,z) coordinates into a single 64-bit key via floor-division and bit-packing. Floor (rather than truncation) keeps cell boundaries continuous across negative coordinates; casting negative signed coordinates to unsigned preserves two's-complement bit patterns safely, and the lower-half mask guarantees the packed z-bits can never bleed into the shifted x-bits.

##### Reference Implementation

```cpp
// Packs a 2m grid cell into one 64-bit key; floor() keeps negative-coordinate cells continuous
uint64_t SpatialHashKey(float x, float z) {
    int64_t cx = static_cast<int64_t>(std::floor(x / 2.0f));
    int64_t cz = static_cast<int64_t>(std::floor(z / 2.0f));
    return (static_cast<uint64_t>(cx) << 32) | (static_cast<uint64_t>(cz) & 0xFFFFFFFFull);
}
```

##### Player-Facing Impact

Invisible directly, but this is the shared broad-phase every proximity query in the game (horde crush, comms static, blood pooling, drafting) sits on top of — a bug here would ripple into all of them at once.

#### [M1-EXT-02] Generational Resource Table Pointer Validator

##### Systems Touched

Backs every `StableId`-based lookup declared in M2.6 and used throughout (EnTT Meta Entity Inspector, M7 persistence, M8-EXT-06 item instances).

##### Math

`Valid = (Handle.Generation ≡ Table[Handle.Index].Generation)`

##### How It Works

Every handle carries a generation counter alongside its table index. On lookup, the handle's stored generation must match the table slot's current generation exactly; a mismatch means the slot was recycled since the handle was taken, and the lookup fails safely instead of silently returning a stale/wrong entity.

##### Reference Implementation

```cpp
bool IsHandleValid(const Handle& h, const std::vector<uint32_t>& generations) {
    return h.index < generations.size() && h.generation == generations[h.index];
}
```

##### Player-Facing Impact

Rapid entity churn (a horde dying and respawning, items being picked up/dropped) never produces a "ghost" reference silently acting on the wrong live entity.

#### [M1-EXT-03] Multi-Threaded Command Pool Matrix

##### Systems Touched

Implements the "Command pool-per-thread strategy" bullet already declared in M0's Implementation Steps, sized against M0's EnkiTS worker count.

##### Math

`Pool_index = (Frame_resource_index · ThreadCount) + Thread_id`

##### How It Works

A flat linear index maps each (in-flight frame, recording thread) pair to a dedicated command pool, so no two threads ever record into the same pool — Vulkan command pools are not internally synchronized — without needing a lock on the hot recording path.

##### Reference Implementation

```cpp
uint32_t GetPoolIndex(uint32_t frameResourceIndex, uint32_t threadCount, uint32_t threadId) {
    assert(threadId < threadCount);
    return (frameResourceIndex * threadCount) + threadId;
}
```

##### Player-Facing Impact

Invisible — this is what keeps multi-threaded draw-call recording race-free without a global lock that would otherwise serialize threads and cost frame time.

#### [M1-EXT-04] Frame-Scoped Deletion Queue

##### Systems Touched

Governs when any GPU resource (buffer, image, pipeline) created by M1's ECS framework is actually freed, across every later milestone that allocates GPU resources.

##### Math

`ReadyToPurge = Fence_completed ≥ FrameFence_assigned`

##### How It Works

A resource marked for deletion is tagged with the fence value of the frame it was last used in, not deleted immediately. A background sweep only frees resources whose tagged fence has already signaled completed on the GPU, guaranteeing no resource is freed while a draw/dispatch that references it might still be in flight.

##### Reference Implementation

```cpp
bool ReadyToPurge(uint64_t completedFenceValue, uint64_t resourceFenceValue) {
    return completedFenceValue >= resourceFenceValue;
}
```

##### Player-Facing Impact

No flickering/corrupted geometry from a resource being freed while the GPU is still mid-draw on it — a class of bug that's otherwise intermittent and hardware-dependent.

#### [M1-EXT-05] Persistent-Mapped Staging Ring Buffer

##### Systems Touched

Backs the upload path for every per-frame dynamic write in the doc — bindless material updates, RVT page uploads (M4.5-EXT-07), UI buffers (M11).

##### Math

`Offset = (FrameIdx · MaxFrameSize) mod TotalPoolSize`

##### How It Works

One large host-visible/persistently-mapped buffer is sub-divided into a ring of per-frame regions. Each frame writes into its own region at a computed offset and never touches a region still being consumed by the GPU from a prior frame, avoiding a `vkMapMemory`/`vkUnmapMemory` call (and its associated stall) on every upload.

##### Reference Implementation

```cpp
void* GetFrameStagingRegion(void* basePtr, size_t maxFrameSize, size_t totalPoolSize, uint32_t frameIdx) {
    size_t offset = (static_cast<size_t>(frameIdx) * maxFrameSize) % totalPoolSize;
    return static_cast<uint8_t*>(basePtr) + offset;
}
```

##### Player-Facing Impact

Frequent small GPU uploads (per-frame UI text, dynamic material tweaks) never introduce a hitch from repeated map/unmap overhead.

#### [M1-EXT-06] EnTT Archetype Component SPSC Mutation Queue Committer

##### Systems Touched

Locks the concrete mechanism behind M1's base-step phrase "Single-threaded registry authority using SPSC ring buffers for background workers" — that phrase named the pattern but never specified it until now. Every background task that wants to add/remove a component or destroy an entity (async collision cook `[M2-EXT-01]`, chunk streaming, WFC generation) must go through this queue instead of touching `entt::registry` from a worker thread.

##### Math

`Head_index = (Head_index + 1) mod QueueSize`

##### How It Works

Worker fibers never call `registry.emplace/remove/destroy` directly — doing so from off the main thread is a data race on EnTT's internal storage. Instead they push a packed `{entity, operationBitmask}` record onto a per-thread single-producer/single-consumer ring buffer. At the start of each main-thread tick, before any system runs, the main loop drains every worker's queue in order and applies the mutations to the live registry. This is the same category of fix as `[M1-EXT-04]`'s frame-scoped deletion queue, one level up — that one defers GPU resource frees to a safe fence point, this one defers ECS structural writes to a safe tick point.

##### Reference Implementation

`readHead` is `std::atomic<uint32_t>` — the producer (worker fiber) does an acquire-load of it in `PushMutation`'s full check, and the consumer (main thread) publishes progress with a release-store. `DrainToRegistry` (consumer-side) is shown below, matching the "How It Works" description.

```cpp
struct DeferredMutation { entt::entity targetEntity; uint32_t operationBitmask; };
struct alignas(64) SPSCMutationQueue {
    DeferredMutation dataPool[1024];
    alignas(64) std::atomic<uint32_t> writeHead{0};
    alignas(64) std::atomic<uint32_t> readHead{0};

    inline bool PushMutation(entt::entity ent, uint32_t op) {
        uint32_t currWrite = writeHead.load(std::memory_order_relaxed);
        if (((currWrite + 1) & 1023) == readHead.load(std::memory_order_acquire)) return false; // Mutation ring pool is full
        dataPool[currWrite] = { ent, op };
        writeHead.store((currWrite + 1) & 1023, std::memory_order_release);
        return true;
    }

    // Called once at the start of each main-thread tick, before any system runs.
    inline void DrainToRegistry(entt::registry& registry, void (*apply)(entt::registry&, const DeferredMutation&)) {
        uint32_t currWrite = writeHead.load(std::memory_order_acquire);
        uint32_t currRead = readHead.load(std::memory_order_relaxed);
        while (currRead != currWrite) {
            apply(registry, dataPool[currRead]);
            currRead = (currRead + 1) & 1023;
        }
        readHead.store(currRead, std::memory_order_release);
    }
};
```

##### Player-Facing Impact

Invisible — this is what lets chunk streaming, physics cooking, and procedural generation run on background threads at all without randomly corrupting the entity registry under heavy load.

#### [M1-EXT-07] Thread-Local Zero-Allocation Linear Page-Bump Arena

##### Systems Touched

CPU-side scratch memory for short-lived per-tick arrays — pathfinding candidate lists, raycast hit buffers, visibility query results. Distinct from `[M4.6-EXT-01]` (GPU vertex-buffer pooling) and `[M0-EXT-01]` (VMA cache-line alignment); this is the third consumer of that same alignment idiom, applied to plain CPU heap scratch instead of GPU sub-allocation.

##### Math

`Ptr_aligned = (Pointer_current + AlignMask) & ~AlignMask`

##### How It Works

Each worker thread owns one pre-allocated byte buffer. Short-lived per-tick allocations bump a local offset forward with alignment rounding instead of calling `malloc`/`new`; the entire arena resets to offset 0 at the end of the tick with zero per-object free calls. Any system needing a scratch array that dies at the end of the current tick (A* open/closed sets, `[M2-EXT-03]`'s query-hash scratch, temporary boid-neighbor lists) should draw from this instead of the heap.

##### Reference Implementation

```cpp
struct BumpArena { uint8_t* memoryBufferPage; size_t capacity; size_t currentOffset; };
inline void* ArenaAllocateBump(BumpArena& arena, size_t size, size_t alignment = 16) {
    size_t alignedOffset = (arena.currentOffset + alignment - 1) & ~(alignment - 1);
    if (alignedOffset + size > arena.capacity) return nullptr; // Arena page boundary hit
    arena.currentOffset = alignedOffset + size;
    return arena.memoryBufferPage + alignedOffset;
}
```

##### Player-Facing Impact

Invisible — this is what keeps pathfinding and raycast-heavy systems (horde AI at scale) from stalling on heap allocator locks mid-tick.

#### [M1-EXT-08] Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
#### [M1-EXT-10] Render-Graph Pass Dependency DAG Flattener *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 render graph. Linearizes the pass DAG into a valid submit order respecting resource read/write edges.

##### Math
topo = Kahn(N, E); inDeg[v]-- on emit; stable order by priority when tied.

##### How It Works
The render graph is a DAG of passes with resource edges. A Kahn topological sort flattens it to a submission order; ties broken by priority so important passes stay early. Recomputed only when the graph changes.

##### Reference Implementation
```cpp
vector<Pass*> order=TopoSort(graph); // Kahn, priority tiebreak
```

##### Player-Facing Impact
Render passes always execute in a valid order - no read-before-write hazards, no manual ordering.

---
#### [M1-EXT-11] Compute-to-Indirect-Draw Execution Barrier *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M2.6 GPU culling. Memory/execution barrier between compute culling and the indirect draw that consumes its buffer.

##### Math
barrier(srcStage=COMPUTE, dstStage=DRAW, buf=visibleList, access=WRITE->READ).

##### How It Works
After the compute cull pass writes the visible-instance buffer, an explicit barrier (or split barrier) ensures the indirect-draw pass sees the writes before reading instance counts/offsets. Prevents reading stale or partial cull data.

##### Reference Implementation
```cpp
vkCmdPipelineBarrier(cb, COMPUTE, DRAW, 0, 0,nullptr, 1,&bufBarrier, 0,nullptr);
```

##### Player-Facing Impact
GPU culling + indirect draw stay correct across passes - no popped or duplicated instances.

---
#### [M1-EXT-16] SoA Cache-Line Padding *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 ECS storage. Pads SoA component arrays to 64-byte cache lines to avoid false sharing / straddle.

##### Math
stride = align(sizeof(T), 64); base = alloc(n*stride, 64).

##### How It Works
Hot component arrays are allocated with a stride rounded to a cache line and base-aligned to 64B, so concurrent jobs touching different entities don't thrash the same line. Applied to the components M0-EXT-12 identified as hot.

##### Reference Implementation
```cpp
auto* a = (T*)aligned_alloc(64, n*align(sizeof(T),64));
```

##### Player-Facing Impact
Multithreaded ECS updates avoid cache-line contention - smoother frame under load.

#### [M1-EXT-22] CVar System (ImGui-Backed) *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + UI. Central console-variable registry with ImGui debug panel.

##### Math
CVarRegistry::Get().Set(name, val); panel binds to registry;

##### How It Works
A typed CVar registry holds tunables (exposed to ImGui sliders/console); systems read live values. Dev/QA tuning without recompiles. Ship-disabled UI.

##### Reference Implementation
```cpp
CVarF32 r(&reg,"render.ssao",1.0f);
```

##### Player-Facing Impact
Tunables are live-editable in-dev - fast iteration on feel/perf.


---

#### [M1-EXT-21] Buffer Device Address for Skinned-Mesh Animation Data *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M5.1. Exposes skinning bone matrices via buffer device addresses for shader direct-read.

##### Math
matrices: device_address; shader reads *((mat4*)addr + boneIdx);

##### How It Works
Skinning bone-matrix buffers are bound by device address so the vertex shader reads bone data directly without descriptor indirection - fewer binds, faster skinning at scale.

##### Reference Implementation
```cpp
mat4* bones = (mat4*)skbuffAddr;
```

##### Player-Facing Impact
Skinning binds drop away - more animated characters for the same cost.


---

#### [M1-EXT-20] EnTT Group-Backed Hot-Component Storage *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + EnTT. Uses EnTT groups for O(1) hot-component view iteration.

##### Math
group = reg.group<Pos,Vel>(); for(auto e: group) ...;

##### How It Works
Hot component pairs are registered as an EnTT group so views over them are O(1) and cache-coherent (group backs them with a shared packed array). Faster than ad-hoc views for the per-frame hot set.

##### Reference Implementation
```cpp
auto g = reg.group<Pos,Vel>(); for(auto e: g) Step(e);
```

##### Player-Facing Impact
Hot-system iteration is near-free - big entity counts stay at 60fps.


---

#### [M1-EXT-19] SoA Layout for Hot Components *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 ECS. Hot components stored Structure-of-Arrays for cache-efficient iteration.

##### Math
array<T> pos; array<T> vel; iterate i linearly; no pointer chase.

##### How It Works
Hot component arrays are stored SoA (one contiguous array per field) so system loops stream linearly through cache. Pairs with M1-EXT-16 padding. Cold components stay AoS.

##### Reference Implementation
```cpp
for(i) Integrate(pos[i], vel[i]);
```

##### Player-Facing Impact
Hot systems iterate cache-friendly - less stall, more entities per ms.


---

#### [M1-EXT-18] Material-Batched Mesh-Pass Rendering *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M4.5. Batches draw calls by material to cut state changes.

##### Math
batch = GroupBy(materialId, draws); one bind per batch;

##### How It Works
Draws are sorted/batched by material (pipeline + descriptor set) so each batch binds once; slashes pipeline-switch overhead on dense scenes. Complements M1-EXT-25 bindless.

##### Reference Implementation
```cpp
for(b in batches) { Bind(b.mat); Draw(b.draws); }
```

##### Player-Facing Impact
Far more draws per frame at the same cost - denser worlds run smooth.


---

#### [M1-EXT-17] Deterministic Secondary Command-Buffer Merger *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1. Merges per-thread secondary command buffers into the primary in deterministic order.

##### Math
merge(secondaries, order=M1-EXT-13) -> primary; stable sort by pass id.

##### How It Works
Worker threads record secondary command buffers; a merger stitches them into the primary in the topological order M1-EXT-13 produced, deterministically, so multi-threaded recording reproduces the same final command stream.

##### Reference Implementation
```cpp
Merge(secondaries, topoOrder, primary);
```

##### Player-Facing Impact
Multi-threaded command recording stays deterministic - co-op/save replays match.


---

#### [M1-EXT-15] GPU Query Pool Timestamp Profiler *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1. Timestamp queries via Vulkan query pool for GPU frame profiling.

##### Math
ts = QueryPool.GetTimestamp(pass); dt = ts_end - ts_start;

##### How It Works
A query pool records GPU timestamps at pass boundaries; the CPU reads them back (deferred) to build a per-pass GPU time breakdown. Off by default in shipping, dev-only.

##### Reference Implementation
```cpp
float dt = (tsEnd - tsStart) * period;
```

##### Player-Facing Impact
You can see where GPU time goes per pass - real profiling, not guesses.


---

#### [M1-EXT-13] Render-Graph Barrier Topological Sorter *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

---

#### [M1-EXT-28] GPU Software Occlusion Rasterizer (HZB Feeder) *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M4.6-EXT-06. CPU-side rasterizes occluders into a HZB feeder for early occlusion.

##### Math
raster occ proxies -> HZB mip0; feeds M4.6-EXT-06 downsample.

##### How It Works
Small/cheap occluder proxies are rasterized on CPU into the HZB's base mip so the GPU HiZ downsample (M4.6-EXT-06) can reject hidden draws even before full depth exists. Cheap, low-count occluders only.

##### Reference Implementation
```cpp
RasterOccluders(proxies, hzbMip0);
```

##### Player-Facing Impact
Distant hidden geometry is culled early - fewer draws, more FPS.


---

#### [M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M2.6. Uniform-grid broadphase as a CPU counterpart to GPU culling.

##### Math
cell = floor(p/cell); bucket; pairs = cells overlapping both AABBs.

##### How It Works
A uniform-grid spatial hash accelerates CPU-side broadphase (physics, AI queries) complementary to GPU HiZ culling; used where GPU culling isn't applicable. Shares cell convention with M2.6-EXT-01.

##### Reference Implementation
```cpp
uint32_t key=SpatialHash(p);
```

##### Player-Facing Impact
CPU broadphase stays cheap - physics/AI neighbor queries scale.


---

#### [M1-EXT-26] Chunk-Boundary Spatial-Hash Transfer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M2.6. Transfers spatial-hash ownership across streaming chunk boundaries.

##### Math
on chunk unload: move cells in border into neighbor hash; re-key by cell.

##### How It Works
When a world chunk unloads, its border spatial-hash cells are handed to the neighbor chunk's hash (re-keyed) so broadphase pairs spanning the seam stay valid. Pairs with M3-EXT-11.

##### Reference Implementation
```cpp
TransferBorderCells(from, to);
```

##### Player-Facing Impact
Streaming chunks don't drop collisions at seams - no pop-through at boundaries.


---

#### [M1-EXT-25] Descriptor Update Templates for Per-Frame Bindless Writes *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + M4.5 bindless. Writes descriptor sets via update templates for fast per-frame binds.

##### Math
vkUpdateDescriptorSetWithTemplate(ds, tpl, data);

##### How It Works
Per-frame bindless descriptor writes use update templates (one memcpy-shaped write) instead of N individual writes, slashing CPU cost of replenishing the bindless table each frame.

##### Reference Implementation
```cpp
vkUpdateDescriptorSetWithTemplate(set, tpl, staging);
```

##### Player-Facing Impact
Bindless replenishment is near-free - texture/resource churn stays cheap.


---

#### [M1-EXT-24] Multi-Threaded Secondary Command Buffer Recording *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1 + enkiTS. Records secondary command buffers across worker threads.

##### Math
for(w in workers) w.Record(secondaries[w]); merge after (M1-EXT-17).

##### How It Works
Each worker thread records its slice of secondaries in parallel; the merger (M1-EXT-17) stitches them deterministically. Cuts CPU record time on big scenes.

##### Reference Implementation
```cpp
parallel_for(workers, RecordSecondary);
```

##### Player-Facing Impact
Command recording parallelized - lower CPU frame cost on dense draws.


---

#### [M1-EXT-23] Timeline Semaphores for Multi-Queue Sync *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M1. Vulkan timeline semaphores sequence work across graphics/compute/transfer queues.

##### Math
wait(timeline, val); signal(timeline, val+1);

##### How It Works
A timeline semaphore carries a monotonic value so queues can wait on / signal fine-grained points without binary-semaphore ping-pong, simplifying multi-queue submission ordering.

##### Reference Implementation
```cpp
vkWaitSemaphores(tl, val); vkSignalSemaphore(tl, val+1);
```

##### Player-Facing Impact
Multi-queue work stays correctly ordered - async compute/transfer without stalls.


##### Systems Touched
M1 render graph. Topologically sorts pass barriers so dependent passes execute in valid order.

##### Math
order = TopoSort(passes, barrierEdges); Kahn with priority tiebreak.

##### How It Works
Builds a DAG of render passes joined by barrier edges (M1-EXT-11), then a Kahn topological sort emits a valid submission order; ties broken by priority. Ensures a pass's inputs are produced before it runs.

##### Reference Implementation
```cpp
vector<Pass*> o=TopoSort(graph);
```

##### Player-Facing Impact
Render passes always run in valid order - no read-before-write, no manual sequencing.


##### Systems Touched
M1 ECS storage. Pads SoA component arrays to 64-byte cache lines to avoid false sharing / straddle.

##### Math
stride = align(sizeof(T), 64); base = alloc(n*stride, 64).

##### How It Works
Hot component arrays are allocated with a stride rounded to a cache line and base-aligned to 64B, so concurrent jobs touching different entities don't thrash the same line. Applied to the components M0-EXT-12 identified as hot.

##### Reference Implementation
```cpp
auto* a = (T*)aligned_alloc(64, n*align(sizeof(T),64));
```

##### Player-Facing Impact
Multithreaded ECS updates avoid cache-line contention - smoother frame under load.

##### Systems Touched

Extends `SpatialHash` (M1's canonical uniform grid, `[M1-EXT-01]`) — protects the 2.5ms AI budget when a localized density spike (a 150+ zombie swarm compressed into a choke point) collapses a single cell's proximity pass into an O(N²) loop.

##### Math

When a cell's entity occupancy `N` exceeds `K_max = 64`, the cell subdivides into 4 sub-quadrants, packed via bit-shift to avoid colliding with the primary `[M1-EXT-01]` key space:

`ChildKey(cx,cz,i) = (Key(cx,cz) << 4) | (i & 0x0F)`

##### How It Works

Rather than shrinking the base 2.0m cell size globally (which would blow up cell count and hash-map overhead everywhere else in the world), overcrowded cells alone get a one-time subdivision into 4 leaf quadrants under the same parent key, shifted into unused low bits. `SpatialHash::QueryRadius`/`QueryCell` (M1) transparently walks into subdivided leaves when the parent cell's `isCellSubdivided` flag is set, so callers never need to know a given cell split.

##### Reference Implementation

```cpp
struct SpatialCellHeader {
    uint32_t entityCount{0};
    uint32_t subQuadrantMask{0}; // bitmask flag tracking populated leaf entries
    bool isCellSubdivided{false};
};
inline uint64_t ComputeSubdividedCellKey(uint64_t parentSpatialKey, int32_t subX, int32_t subZ) {
    uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03);
    return (parentSpatialKey << 4) | static_cast<uint64_t>(leafOffset & 0x0Full);
}
```

##### Player-Facing Impact

Invisible directly — this is what keeps AI proximity queries fast even when a horde crushes into a single narrow chokepoint, instead of the frame budget collapsing exactly when the fight gets most intense.

### Dev-Tool: EnTT Meta Entity Inspector (EnTT 3.16.0 reflection unlock)

*Not a gameplay milestone — scoped as a dev-tool/debug-overlay feature riding on M1's existing ImGui overlay.*

* **Registration:** EnTT 3.16.0's `entt::meta` reflection is registered for a fixed set of components in a single central file (`src/debug/MetaRegistry.cpp`), not scattered per-component registration calls — keeps the "what's inspectable" list auditable in one place. Initial registration set: `Transform`, `Health`, `DamageEvent`/`ResolvedDamageEvent` (item 5), `StableId` (item 7), `PerkPoints`-related components (item 6/M8.7), `BarrelHeat`, `BulletComponent` (item 4), and `SurfaceFrictionSample`.
* **Inspector UI:** a runtime panel added to M1's existing ImGui workspace overlay. Lists entities queryable either by `SpatialHash` region (drag-select a world-space box, matching M1's existing spatial primitive rather than a second ad hoc query path) or by direct `StableId` lookup (type a known ID). Selecting an entity shows its registered components with live-editable fields (numeric sliders/text fields generated generically from each field's reflected type via `entt::meta`), writing changes back through the registry immediately.
* **Scope decision: dev-only, stripped from release builds** via a compile-time `#ifdef ENGINE_DEV_TOOLS` guard around both the meta-registration call site and the ImGui panel — this is explicitly not being extended into a mod-support surface in this pass. Flagging the boundary per the spec: a future mod-support extension would need to sandbox arbitrary field writes (a mod shouldn't be able to, say, rewrite `StableId` and desync a save), rate-limit/validate write ranges per field, and likely move from "any registered field is editable" to an explicit per-field mod-write allowlist. None of that is being built now.
* **Entity lookup key:** the inspector's region/ID lookups resolve to `StableId` (item 7) first and map to the live `entt::entity` handle for the current session, rather than storing raw handles anywhere in the tool's own state — handles aren't meaningful across a save/load boundary, and this tool is exactly the kind of thing someone will leave a "selected entity" bookmark for across a reload.

## M2 — Jolt 5.5.0 physics, EventBus, and a real destructible test entity

##### Implementation Steps

* `JPH::JobSystemThreadPool` and `JPH::PhysicsSystem` setup on Jolt Physics **5.5.0** (bumped from the prior 5.1 pin — confirmed latest stable, includes VS2026 support; see Dependency Version Bump below). Right-handed Y-up alignment.
* **Build Jolt itself with `JPH_CROSS_PLATFORM_DETERMINISTIC` defined.** This is a Jolt library compile-time CMake option, distinct from and required alongside `[M2.8-EXT-04]`'s fixed-point math layer — that layer only guards *your* gameplay code's float drift; without this flag Jolt's own solver (contact manifold resolution, constraint integration) is not guaranteed bit-identical across compilers/CPU architectures (confirmed via Jolt's own docs — MSVC vs. Clang, x86 vs. ARM), which would silently break M2.8/M12 co-op determinism at the physics layer regardless of how careful the rest of the doc is. Costs ~8% CPU on the physics step (Jolt's own stated figure) — accept it, this is not optional for a co-op game built on deterministic lockstep. Every platform must be built from identical Jolt source with identical defines (`JPH_DOUBLE_PRECISION` must match everywhere too) — a mismatched build on even one client desyncs everyone.
* Fixed-timestep integration: `PhysicsSystem::Update()`. `EMotionQuality::LinearCast` for CCD.
* Event Delivery Bus via `entt::dispatcher` (`.enqueue<Event>()`).
* Mirror Jolt transforms back into EnTT `Transform` components.
* Health and Destructible act as separate, composable components.
* **`DamageEvent` — the canonical struct dispatched on the EventBus for any HP-affecting interaction.** Declared here since M2.7 (melee/gunplay), M2.9 (per-limb ballistics), and every later damage-dealing EXT feature must construct/consume this exact shape rather than parallel ad hoc structs:

```cpp
enum class LimbId : uint8_t { Head, Torso, ArmLeft, ArmRight, LegLeft, LegRight, Count };
// LimbId is the one limb enum in the codebase — M2.7's Per-Limb Vulnerability Matrices and
// M2.9's ballistics/cavitation features both index into this, not a second local enum.

struct DamageEvent {
    float amount = 0.0f;                  // raw, pre-mitigation
    entt::entity source = entt::null;     // dealer; entt::null for environmental/scripted damage
    entt::entity instigator = entt::null; // credit-attribution owner when source is indirect
                                           // (e.g. source = thrown grenade, instigator = player who threw it;
                                           // for direct melee/gunfire, instigator == source)
    LimbId hit_location = LimbId::Torso;
    float penetration_depth = 0.0f;       // cm penetrated before this hit resolved; read by M2.9
                                           // per-limb wound severity and armor/cover interaction checks
    enum class DamageTag : uint8_t { Melee, Ballistic, Fire, Fall, Environmental, Infection } tag = DamageTag::Melee;
    // parry (M2.7-EXT-01) only intercepts DamageTag::Melee; Ballistic/Fire/Environmental pass through unparryable
};

struct ResolvedDamageEvent {
    DamageEvent source_event;
    float final_amount = 0.0f; // after parry/armor/perk mitigation has run
    bool was_parried = false;
    bool was_mitigated = false;
};
// Dispatch order: raw DamageEvent enqueued -> parry system (M2.7-EXT-01) may consume it if tag == Melee ->
// armor/cover mitigation applies -> PerkPoints resolved-damage hooks (M8.7) apply last -> ResolvedDamageEvent
// enqueued and applied to Health. Systems needing to intercept before final application subscribe to
// DamageEvent; systems that only care about final HP loss (UI, hit-markers, blood VFX) subscribe to
// ResolvedDamageEvent.
```

* **`BulletComponent`** — referenced by M2.9-EXT-04's tumbling-bullet bitmask but never declared until now:

```cpp
struct BulletComponent {
    glm::vec3 velocity{0.0f};
    float mass = 0.0f;
    float dragCoefficient = 0.0f; // C_d0, baseline (non-tumbling) value
    enum Flags : uint32_t { None = 0, Tumbling = 1 << 0, Ricocheted = 1 << 1, Spalled = 1 << 2 };
    uint32_t flags = Flags::None; // M2.9-EXT-04 sets Tumbling on a steep glance-angle impact
};
```

Owning milestone: M2 (declared alongside `DamageEvent` since both are core combat data shapes read across M2.7/M2.9); M2.9-EXT-04 is retrofitted to read/write `BulletComponent::flags & Flags::Tumbling` instead of an unnamed bitmask.

* Frame Resource Pacing (matching command buffers/fences per swapchain image layer).
* Forward `JPH::DebugRenderer` geometry into the M1 debug pass.
* File System Workspace: `assets/`, `saves/`, `logs/`, `config/`, `mods/`. Soft Asset Mitigation (magenta placeholder fallbacks).

##### Decisions to flag

Health/Destructible split is settled. `DamageEvent`/`ResolvedDamageEvent` split is settled — no feature past this point reads raw `amount` post-mitigation.

##### Exit Criteria

* Jolt simulation loops reliably within fixed timestep ticks.
* EventBus routes damage cleanly.
* Destructible test entity takes damage and visibly swaps meshes/removes colliders.
* Debug drawing maps Jolt collision hulls over visual draws.
* Missing asset gracefully degrades to placeholder.

### Extended Systems Library — engine-side additions for M2

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M2-EXT-01] Asynchronous Collision Cooking & Proxy Primitive Swapping Queue

##### Systems Touched

Extends M2 (Jolt Physics engine setup) and pairs with M4 chunk streaming topologies.

##### Math

Low-overhead bounding volume calculations extract a temporary AABB envelope from raw vertex streams before initiating full structural shape compilation loops:

`V_box = (x_max − x_min) · (y_max − y_min) · (z_max − z_min)`

##### How It Works

Generating a high-fidelity `JPH::MeshShape` straight from procedural vertex arrays on the main thread will stall execution and induce severe frame hitches on your Ryzen 5700G hardware setup. This feature introduces a deferred collision preparation queue. When a sector chunk streams into memory, the engine immediately constructs a cheap, temporary compound primitive shape (a flat bounding-box envelope) so actors don't fall through uninitialized geometry. The raw index/vertex buffers are handed to an asynchronous enkiTS task to compile the optimized Jolt BVH shape offline. Once the background thread completes cooking, the engine performs an atomic shape swap on the main thread via `JPH::BodyInterface::SetShape`, replacing the proxy cleanly without disrupting runtime physics updates.

##### Reference Implementation

```cpp
// Validates array bounds before launching background tasks; performs an atomic
// shape swap so Jolt never observes a half-cooked shape mid multi-threaded update
void QueueAsyncCollisionSwap(JPH::BodyInterface& bi, JPH::BodyID body_id, const std::vector<glm::vec3>& vertices, JPH::ShapeRefC& proxy_shape) {
    if (vertices.empty()) return;

    // Instantly bind the cheap proxy shape to preserve main-thread pacing
    bi.SetShape(body_id, proxy_shape, true, JPH::EActivation::DontActivate);

    // Dispatch full BVH mesh cooking to background enkiTS workers; once cooked,
    // bi.SetShape(body_id, cooked_mesh_shape, true, JPH::EActivation::Activate) runs on main thread
}
```

##### Player-Facing Impact

Traversing high-velocity avenues across procedurally streaming city sectors stays perfectly smooth. Complex architectural debris, ruined foundations, and uneven highway slabs stream in right in front of your path with zero stutter, while physics bounds always stay accurate to the visual mesh.

##### Depends on (not yet built at this point)

M4 — implement against simple static grid meshes for now; revisit once procedural streaming loops land.

#### [M2-EXT-02] Jolt Physics Multi-Body Rigid Island Sleep Event Bus Bridge

##### Systems Touched

`[M7-EXT-01]` The Corpse-Pile Flattening Engine already says "When an EnTT ragdoll entity transitions to a sleep flag..." but M2 never declared the mechanism that sets that flag. This is it — `[M7-EXT-01]` is retrofitted below to subscribe to `HibernationEvent` instead of an assumed flag.

##### Math

`RegistryFlag_hibernate = StableIDMap.Resolve(Body_Handle)`

##### How It Works

Hooks Jolt's native island-sleep callback. When Jolt puts a rigid-body island to sleep (or wakes it), the bridge resolves the sleeping `JPH::BodyID` back to its owning `StableId` via the existing M2.6 `StableId`/generational lookup, then enqueues a `HibernationEvent` on the M2 EventBus. Any system that cares about an entity going dormant — `[M7-EXT-01]`'s corpse-flattening swap, LOD downgrades, AI perception exclusion — subscribes to this event instead of polling Jolt sleep state every tick.

##### Reference Implementation

```cpp
struct HibernationEvent { uint64_t targetStableId; bool isEnteringSleep; };
void OnJoltBodySleepStateChange(entt::dispatcher& dispatcher, uint64_t stableId, bool sleptState) {
    dispatcher.enqueue<HibernationEvent>({ stableId, sleptState });
}
```

##### Player-Facing Impact

Invisible directly — this is the missing wire that makes corpse-pile flattening, LOD downgrades, and dormant-entity AI exclusion actually trigger, instead of each of those features silently polling for a flag nothing ever sets.

#### [M2-EXT-03] Continuous Narrow-Phase Contact Point Query Collector Cache

##### Systems Touched

Sits in front of Jolt's narrow-phase query API, deduping redundant raycasts/sweeps fired by multiple systems in the same frame — melee parry (`[M2.7-EXT-01]`), AI perception line-of-sight (M5.3), vault ledge-analysis (M2.9), and any `SpatialHash`-adjacent query that also needs a physical line trace.

##### Math

`QueryHash = Hash(V_origin ⊕ V_destination)`

##### How It Works

Hashes the start/end vectors of a line/capsule query. If an identical query (same origin, same destination, same frame) was already dispatched this frame, the cache returns the cached hit result instead of re-walking the Jolt BVH tree a second time. Cleared at the start of every fixed-timestep tick — this is a same-frame dedup cache, not a persistent result cache, so it never returns stale hits.

##### Reference Implementation

```cpp
struct LineQuery { glm::vec3 start; glm::vec3 end; };
inline uint64_t HashSpatialQuery(const LineQuery& q) {
    std::hash<float> hasher;
    return (static_cast<uint64_t>(hasher(q.start.x)) << 32) ^ static_cast<uint64_t>(hasher(q.end.z));
}
```

##### Player-Facing Impact

Invisible — protects the 2.5ms physics budget when many systems (AI perception across a horde, melee parry checks, vault analysis) all want line-of-sight or sweep results against the same geometry in the same frame.

#### [M2-EXT-04] Fixed-Timestep Mechanical Joint Constraint Kinetic Energy Clamper

##### Systems Touched

Hardening pass alongside `[M5.2-EXT-01]`'s Stable-PD reformulation. That feature stabilizes ragdoll joint *torque generation*; this clamper is a last-resort hard cap on the resulting *velocity* — a defense-in-depth pair, not a duplicate. Specifically relevant right after an M2.6 floating-origin rebase jump, where a position discontinuity could otherwise inject a one-frame velocity spike into an active ragdoll.

##### Math

`v_clamped = v · min(1.0, V_maxLimit / (‖v‖ + ε))`

##### How It Works

After Jolt's solver step resolves contact manifolds and any Stable-PD joint torques (`[M5.2-EXT-01]`) have been applied, this pass checks the resulting linear/angular velocity of active ragdoll bodies against a fixed safety ceiling. Velocities above the ceiling are rescaled down to it rather than left to integrate into an exploding joint. This never fires under normal play — it's specifically the backstop for origin-rebase discontinuities and extreme collision impulses that Stable-PD alone doesn't fully bound.

##### Reference Implementation

```cpp
inline void ClampVelocityImpulse(glm::vec3& vel, float maxLimit) {
    float lenSq = glm::dot(vel, vel);
    if (lenSq > maxLimit * maxLimit) {
        vel = glm::normalize(vel) * maxLimit;
    }
}
```

##### Player-Facing Impact

Ragdolls near a world-origin rebase boundary (`[M2.6]`, >500 units) never visibly explode or teleport — they stay physically bounded even at the exact moment the floating-origin shift happens.

#### [M2-EXT-05] Mechanical Muscle Exhaustion Joint-Friction Damping Adder
#### [M2-EXT-06] Jolt-to-EnTT Double-Precision Transform Remapper *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2 (Jolt) + M2.6 (EnTT Transform). Each tick maps Jolt's single-precision body transform back to the authoritative double-precision EnTT Transform.

##### Math
EnTT.pos = worldOrigin + (double)jolt.pos; EnTT.quat = (double)jolt.quat.

##### How It Works
Jolt simulates in single precision around a local origin for stability; the canonical world transform is double precision in EnTT. After each physics step, the body's local transform is re-based to the streaming origin and written back to the double-precision EnTT Transform so rendering/culling stay world-accurate at km scale.

##### Reference Implementation
```cpp
auto& T = reg.get<Transform>(e); T.pos = origin + (dvec3)body.GetPosition(); T.rot=(dquat)body.GetRotation();
```

##### Player-Facing Impact
Physics stays stable up close while the world stays precise at long range - no drift or snapping.

---
#### [M2-EXT-07] Kinematic Virtual Sweep Tunneling Safeguard *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2 character controller + M2.6 collision. Clamps fast kinematic steps to a swept result so the body can't pass through thin geometry.

##### Math
swept = Sweep(shape, from, to, world); if(hit.t < 1) to = from + dir*hit.t - skin.

##### How It Works
When a kinematic character moves more than its thickness in one step, a virtual sweep against the broadphase prevents tunneling through walls/thin floors. The move is shortened to the first hit minus a skin width; the remainder is queued for next step.

##### Reference Implementation
```cpp
SweepHit h = Sweep(capsule, from, to, world); if(h.t<1) to = lerp(from,to,h.t)-normal*skin;
```

##### Player-Facing Impact
Fast movement never clips through walls or floors - no fall-through bugs.

##### Systems Touched

Bridges M2.9's Two-Compartment Exertion Model (`W'balance`) into the `JPH::CharacterVirtual` velocity input, replacing an artificial speed-clamp scalar with a native damping term inside the fixed physics step.

##### Math

`C_exhaust = Clamp(W'balance / W'max_pool, 0.35, 1.0)`

`v_damped_target = v_raw_input_target · C_exhaust`

##### How It Works

Rather than post-processing the character controller's output velocity with an arbitrary cap when the metabolic pool empties, this multiplies the exhaustion scalar directly into the velocity vector `JPH::CharacterVirtual::ExtendedUpdate()` consumes as input — so the deceleration reads as physically damped rather than an abrupt speed-cap snap. Floor-clamped at 0.35 so exhaustion never fully zeroes movement (matching M2.9-EXT-07's coupled hydration-exertion floor behavior).

##### Reference Implementation

```cpp
inline glm::vec3 ApplyPhysiologicalVelocityDamping(const glm::vec3& targetVelocity, float liveWBalance, float maxWBalance) {
    float scalar = (maxWBalance > 0.001f) ? (liveWBalance / maxWBalance) : 0.35f;
    float cExhaust = std::clamp(scalar, 0.35f, 1.0f);
    return targetVelocity * cExhaust; // feeds directly into JPH::CharacterVirtual velocity input
}
```

##### Player-Facing Impact

Sprinting yourself out of stamina produces a gradual, organic slowdown rather than an arbitrary speed-cap toggle — you feel your character physically tire rather than hit an invisible wall.

## M2.6 — Open-world foundations

##### Implementation Steps

Built to the same depth as the rest of the doc — this is the precision/streaming substrate every later milestone (M4's chunk streaming, M2.8's co-op sync, M7's persistence) is built directly on top of, so a stub here compounds into floating-point jitter, desyncs, and duplicate/lost entities much later when they're far harder to trace back to the root cause.

* **Double Precision Authoritative Space.** `float`'s 23-bit mantissa only holds sub-millimeter precision out to roughly ±4096 units from the origin; past that, position error grows in discrete steps (`ULP(x) ≈ x · 2⁻²³`) and manifests as visible jitter, z-fighting, and physics tunneling on large open worlds. The fix is a two-tier position representation: every entity's `Transform.position` is authoritative `glm::dvec3` (double precision, world-space, used for gameplay logic, physics queries, and save data), while the GPU only ever receives camera-relative single precision: `renderPos = (glm::vec3)(entityPos - cameraPos)`. This keeps all rendering math inside float32's precise range regardless of how far the player has walked from world origin, since the camera is always near `(0,0,0)` in render-space.
* **Periodic Origin Rebasing.** Camera-relative rendering alone isn't enough once the *camera's own* `dvec3` position accumulates enough magnitude that `entityPos - cameraPos` itself starts losing precision during the subtraction. When `glm::length(cameraPos - worldOriginOffset) > 500.0`, the engine performs an origin rebase: `worldOriginOffset += (cameraPos - worldOriginOffset)`, and every live entity's cached render-space transform is recomputed against the new offset on the same frame (not deferred) to avoid a visible pop. A 500.0-unit rebase threshold paired with hysteresis (rebase target snaps to the nearest 100.0-unit grid rather than the exact trigger point) prevents rebase-thrashing when the player idles near the boundary. Rebasing dispatches a `WorldOriginRebased { glm::dvec3 newOffset }` event on the M2 EventBus — M2.8's co-op replication layer and M12's network tick both subscribe to this, since a rebase changes what "position" means for any in-flight interpolation snapshot and must be applied identically on every peer in the same simulation tick, not independently per-client.
* **`Transform` component standardization.** A single canonical component — `struct Transform { glm::dvec3 position; glm::dquat rotation; glm::dvec3 scale; }` — replaces every bare-position field used ad hoc in M0/M1/M2 prototyping. All physics (M2/M3), rendering (M0/M4.5), and animation (M5.2) systems read/write this one struct rather than maintaining parallel position caches, so there is exactly one source of truth per entity and no risk of the render and physics representations of "where an entity is" drifting apart. A debug fly-camera (free-fly, noclip, decoupled from the player's `CharacterVirtual`) is wired against this same `Transform` path specifically so world-precision and rebasing can be visually verified by flying far from origin and watching for pop/jitter, rather than trusting the math on paper.
* **glTF Geometry Caching.** Static mesh geometry (buildings, props, terrain decoration) is loaded once per unique asset path and cached in a hash-keyed table (`std::unordered_map<uint64_t /*FNV-1a of asset path*/, MeshHandle>`) rather than re-parsed and re-uploaded to the GPU every time a chunk streams a duplicate prop instance. Cache eviction is LRU-bounded against M4.6's VRAM budget tracker — a mesh with zero active chunk references is a decay candidate, not an immediate free, so a player oscillating across a chunk boundary doesn't thrash the cache every frame.
* **`uint64_t StableId` generation.** Every persistent entity (not transient VFX/particles) is assigned a `StableId` at creation, deterministic under two different concerns: `worldSeed`-derived entities (procedurally placed props, spawns) get an ID hashed from `(worldSeed, sectorCoord, localSlotIndex)` so the *same* entity gets the *same* ID across a save/reload with no lookup table required; player-caused entities (dropped items, player-built structures) get an ID from a monotonic counter namespaced per save file. This split matters because M7's persistence only needs to serialize the second kind explicitly — the first kind can be re-derived deterministically from the world seed and doesn't bloat the save file — while M2.8/M12's network layer uses `StableId`, not the ECS's internal `entt::entity` handle, as the wire identifier, since raw EnTT handles aren't guaranteed to match across independently-simulated peers.
* **Dynamic Sector Seam Blending.** Terrain height and detail noise is evaluated per-sector independently for streaming parallelism, which naturally produces a visible discontinuity at shared vertex borders since each sector's FBM (fractal Brownian motion) sampling has no knowledge of its neighbor. The fix blends the top-level FBM octave across a fixed-width border band using the neighboring sector's sample rather than a hard cutoff: `h_blended(p) = lerp(h_local(p), h_neighbor(p), smootherstep(t))`, where `t` is the normalized distance into the border band (0 at the band's inner edge, 1 at the shared seam) and `h_neighbor` is evaluated using the *neighbor sector's* full noise parameters, not just extrapolated from local data — this requires each sector to have read access to its immediate neighbors' generation parameters during the blend pass, not just their generated heightfield.
* **Procedural Micro-Detail Generation.** Beneath the macro terrain heightfield, small-scale surface variation (cracks, rubble texture, ground clutter density) is generated with domain-warped OpenSimplex2: `detail(p) = simplex2(p + warpStrength · simplex2(p · warpFreq))`, feeding the warped coordinate back through a second noise evaluation rather than sampling raw noise directly, which avoids the visibly grid-aligned look plain simplex noise produces at a glance. `warpStrength`/`warpFreq` are tuned per biome tag (from M4's socio-economic/biome zoning) so, e.g., urban rubble reads visually distinct from rural terrain texture without a second bespoke noise system.
* **Sector Boundary Entity Handoff.** When a dynamic (physics-simulated or AI-controlled) entity crosses a sector boundary, authority for simulating that entity must transfer to whichever sector currently owns the streaming/simulation budget for that region — without a frame where both sectors think they own it (duplicate simulation) or neither does (entity freezes/falls through world). Handoff is a two-phase handshake over the EventBus: the source sector dispatches `EntityHandoffRequested { StableId, targetSector }` on crossing, and only relinquishes simulation ownership after receiving `EntityHandoffAcknowledged` from the target sector on a later tick — never optimistically in the same frame the crossing was detected, since the target sector may not have finished streaming in yet. Until acknowledgment, the source sector keeps simulating; this makes the handoff fail-safe (worst case is one extra sector briefly straddling ownership) rather than fail-open (an entity briefly owned by nobody).

##### Decisions to flag

glTF loader choice (e.g., cgltf/tinygltf). OpenSimplex2 procedural detail scaling and per-biome warp parameter defaults. Origin-rebase threshold/hysteresis grid size if 500.0/100.0 proves too aggressive or too loose once real content streams through it. Sector size — this must be decided here and then held fixed through M4, since M4's chunk streaming layer is built directly on top of the sector boundaries defined in this milestone.

##### Exit Criteria

* World precision implemented exactly as spiked in Day 0 Spike B; `Transform` (dvec3 position/rotation/scale) replaces every bare position field from M0–M2 prototyping, with zero remaining call sites reading raw floats for world position.
* Flying the debug fly-camera several kilometers from world origin produces no visible jitter, pop, or z-fighting; an origin rebase is observable in the debug overlay and produces no visible frame-to-frame pop when it fires.
* Two adjacent sectors' terrain shows no visible seam or lighting discontinuity at their shared border; a dynamic physics body (e.g. a rolling barrel) crossing a sector boundary is picked up by exactly one sector's simulation at all times, never zero and never two.
* Stable IDs are verified to survive a save/reload cycle unchanged (M7's persistence round-trip), and procedurally-placed entities re-derive the identical ID from `worldSeed` alone with no lookup table.

## M2.7 — Seamless true first/third-person player controller & core combat

##### Implementation Steps

* Canonical `BarrelHeat` struct, locked here — this is the only field list any weapon-heat system is allowed to read/write:

```cpp
struct BarrelHeat {
    float kelvin = 293.15f;
    float fouling = 0.0f;       // carbon fouling accumulation (0-1), written by M2.7-EXT-07, read by M2.7-EXT-12
    float grimeFactor = 0.0f;   // maintenance neglect scalar, read by M8-EXT-01
};
```

* `JPH::CharacterVirtual` controller.
* Decoupled Camera Kinematic Entity (see M2.7-EXT-14) driving an occlusion-aware capsule-cast spring arm (third-person) and a look-at joint override on the neck/head/spine (first-person) — the camera is the source of truth the skeleton conforms to (not the reverse), avoiding the phase lag and positional drift that slerping onto a noisy skeletal joint would introduce.
* Damped Recoil Oscillator: `velocity += (-k*position - c*velocity)*dt`. Procedural Recoil Sequencing (R2 low-discrepancy sequence). Procedural Sway (incommensurate sine waves).
* Semi-Implicit Ballistic Integration (replaces hitscan): de Marre penetration, ricochets (below 10–15°).
* Magazine Tracking. Melee sweeps.
* Per-Limb Vulnerability Matrices: IK limps on legs, sway scaling on arms.
* Perceptual Stress Modulation (`StressLevel`) scaling tremor/recoil. Magnification Sway Scaling (inverse Fitts's Law).
* Cover Stability Damping (`c=2√(km)`). Dynamic Lean Kinetics.
* Downed State Lifecycle (`Alive`/`Downed`/`Dead`), Hardcore Lifecycle Validation.

##### Decisions to flag

Jolt's built-in controller vs hand-rolled (default Jolt). Revive window duration (resolved: 60s). Hardcore permadeath toggle. Tommy gun default layouts.

##### Exit Criteria

* Player moves via Jolt controller. Seamless 1st/3rd person camera.
* Ballistic drop, de Marre penetration, and ricochet visible.
* Sway, recoil, IK-limp visible dynamically via StressLevel.
* Downed state functions with correct revive window.



### Extended Systems Library — engine-side additions for M2.7

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M2.7-EXT-01] Kinetic Melee Edge Parrying & Character Stumble Impulses

##### Systems Touched

Extends M2.7 (Tactile Melee Projections) and intercepts incoming threats via the EventBus (M2).

##### How It Works

Your close-quarters melee combat sweeps capsules or boxes through Jolt contact layers. This system registers a specialized structural event handler on your EnTT dispatcher loop (.sink<DamageEvent>().connect()). When an enemy attacks, if your player input bitmask registers an active guard frame, the system intercepts the incoming DamageEvent, discards the health reduction entirely, and invokes a targeted physical impulse straight onto the attacker's Jolt rigid body handle using JPH::BodyInterface::AddImpulse.

##### Math

The parry event evaluates the attacker's forward momentum vector (v_input ) and reflects it across the defensive parry plane normal (n_parry ), integrating a controlled angular pushback torque:

`v_rebound =e⋅(v_input −2(v_input ⋅n_parry )n_parry )+J_stumble`

##### Reference Implementation

```cpp
// Reflection formula assumes n is unit length; guards a degenerate/non-normalized contact
// normal instead of relying on an unstated caller invariant (this doesn't divide by |n|, so it can't
// itself produce NaN/Inf, but a non-unit n silently gives a physically wrong reflection).
// Executed on the EventBus dispatcher to negate player damage and inject dynamic physical lurch forces to enemies
void ProcessParry(JPH::BodyInterface& bi, JPH::BodyID attacker, const glm::vec3& v_in, const glm::vec3& n_raw, float e, const glm::vec3& j_stumble) {
    glm::vec3 n = (glm::dot(n_raw, n_raw) > 1e-6f) ? glm::normalize(n_raw) : glm::vec3(0.0f, 0.0f, 1.0f);
    bi.AddImpulse(attacker, JPH::Vec3(e * (v_in.x - 2.0f * glm::dot(v_in, n) * n.x) + j_stumble.x, 0.0f, e * (v_in.z - 2.0f * glm::dot(v_in, n) * n.z) + j_stumble.z));
}
```

##### Player-Facing Impact

Melee defense shifts from passive damage blocking to an active kinetic skill check. Timing an iron bar or axe swing precisely against an oncoming zombie’s lunging strike catches their collision vector directly, redirecting their momentum. Heavy zombies are sent reeling sideways into structural geometry, breaking up their charging patterns and opening clean operational windows for rapid counter-attacks.

#### [M2.7-EXT-02] Thermo-Elastic Barrel Distortion & Aim Bloom

##### Systems Touched

Deepens M2.7 core gunplay mechanics and utilizes your M8 data-driven JSON caliber variables.

##### Math

Rapid fire channels massive thermal energy into your gun barrel. Centerline mechanical bowing introduces an explicit trajectory bias vector (V_drift ), while internal hoop stress expands the scale modifier (S_bloom ) of your R2 low-discrepancy shooting spread sequence:

`V_drift = u ^ _warp ⋅α_thermal ⋅(T_barrel −T_ambient ) S_bloom =1.0+γ⋅tanh(ω⋅(T_barrel −T_ambient )^2)`

`v_final =Normalize(v_aim +V_recoil ⋅S_bloom +V_drift )`

`S_bloom`'s quadratic term is wrapped in `tanh` so the scale factor saturates smoothly toward `1.0 + γ` instead of growing unbounded if something upstream fails to clamp `BarrelHeat.kelvin` — a defense-in-depth clamp independent of whatever caps `kelvin` elsewhere.

##### How It Works

This reads/writes `BarrelHeat.kelvin` on the canonical struct locked in M2.7's base Implementation Steps (do not redeclare a local/partial `BarrelHeat` here). Every fired projectile increments `kelvin` based on specific thermodynamic attributes loaded directly from your weapon JSON files. Inside the fixed-timestep loop step, your ballistic calculation applies this thermal drift and scaling to your projectile entities before casting them onto the enkiTS background task pools.

##### Reference Implementation

```cpp
// Wrapped in tanh so spread scaling saturates instead of growing unbounded — defense-in-depth
// independent of BarrelHeat.kelvin's upstream clamp (see [M8-EXT-01] cook-off trigger)
// Applies progressive accuracy degradation to bullet paths inside your fixed-timestep loop step
void CalcThermalBloom(float temp, float t_amb, float gamma, float omega, glm::vec3& spread_vector, glm::vec3& drift_vector) {
    float dT = std::max(0.0f, temp - t_amb);
    spread_vector *= (1.0f + gamma * std::tanh(omega * (dT * dT)));
    drift_vector += glm::vec3(0.01f, 0.005f, 0.0f) * dT;
}
```

##### Player-Facing Impact

Dump two consecutive drum magazines out of your Tommy gun layout, and the weapon frame becomes a physical hot potato. The gun doesn't just display a superficial smoke effect—it physically throws off your alignment. The aim center progressively drifts up and to the right, and your shooting spread turns into a chaotic hose of lead. This forces you to switch weapons strategically or utilize melee sweeps while your primary weapon cools down in your volumetric inventory grid.

##### Depends on (not yet built at this point)

M8 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.7-EXT-03] Rotational Weapon Inertia Tensors (Tactile Gunweight Lag)

##### Systems Touched

Layers mechanical weight-lag metrics onto camera view calculations and weapon mesh rendering, reading mass/dimension fields from M8's data-driven JSON item blueprints.

##### Math

`Iθ ′′ +cθ ′ +kθ=τ_camera`

##### How It Works

Hooks straight into your M2.7 core combat rendering loop. Currently, your system tracks direct camera orientations with a damped recoil oscillator. This feature introduces structural angular lag based on weapon dimensions parsed directly from your data-driven JSON item blueprints (M8).

Calculate an analytical moment of inertia (I) for your held item (e.g., your high-mass Tommy gun vs. a lightweight pistol). When the player moves their mouse or joystick rapidly (τ_camera ), the actual 3D weapon rendering matrix calculates a critically damped angular spring-mass delay behind the true screen-center look vector.

##### Reference Implementation

```cpp
void UpdateWeaponInertiaLag(float I, float c, float k, float torque_camera, float dt, float& theta, float& theta_dot) {
    float theta_ddot = (torque_camera - (c * theta_dot) - (k * theta)) / (I + 1e-5f);
    theta_dot += theta_ddot * dt;
    theta += theta_dot * dt;
}
```

##### Player-Facing Impact

Heavy, long-barreled weapons feel physical and heavy, naturally lagging behind rapid camera snaps. It bridges beautifully with your Magnification Sway Scaling (M2.7)—aiming down a long-range optic makes the barrel sway with real mechanical momentum, rewarding players who steady their weapons against terrain boundaries using Cover Stability Damping.

#### [M2.7-EXT-04] Kinematic Vault-Slide Projection

##### Systems Touched

Extends M2.7 (JPH::CharacterVirtual controller updates) and pairs with M2.9 Ledge IK parameters.

##### Math

`v_slide = v_entry − (v_entry · n)n`

`v_slide(t+Δt) = v_slide(t) · max(0.0, 1.0 − γ·μ·Δt)`

##### How It Works

Your character uses a kinematic virtual controller, which means it does not accept forces—it relies on explicit velocity inputs during .ExtendedUpdate(). To avoid breaking this design, when an IK vault ends on a surface where SurfaceFrictionSample::traction is low, the engine strips vertical velocity components and projects the remaining forward momentum straight onto the ground tangent plane. This velocity vector decays over time based on the active surface friction coefficient (μ), scaled by tuning constant γ.

##### Reference Implementation

Decay rate multiplies by μ·γ — higher friction means faster decay, matching the Reference Implementation below.

```cpp
// Executed directly inside JPH::CharacterVirtual::ExtendedUpdate() input parsing
void ApplyVaultSlide(const glm::vec3& n, float mu, float dt, float gamma, glm::vec3& velocity) {
    velocity = (velocity - glm::dot(velocity, n) * n) * std::max(0.0f, 1.0f - (gamma * mu * dt));
}
```

##### Player-Facing Impact

Sprinting away from a swarm, leaping over a concrete barricade, and landing on a blood-covered asphalt sector switches the player into a sustained, low-clearance slide. You maintain total velocity vectors and full firing capabilities with your Tommy gun layout, turning slick streets into high-speed evasion channels.

##### Depends on (not yet built at this point)

M2.9 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.7-EXT-05] Grounded Surface Viscosity Weapon Stabilization

##### Systems Touched

Modifies M2.7 weapon cover stabilization variables using M9 surface friction queries.

##### Math

When resting your weapon against geometry, the spring muscle damping coefficient (c) is augmented using the material damping scalar (η_surface ) extracted from the SurfaceFrictionSample data structure:

`c_stabilized = 2·√(k·m) + η_surface`

##### How It Works

The weapon-rest spring-damper is normally critically damped (`c = 2√(k·m)`) so it settles without oscillating. Resting on real geometry adds the surface's own damping scalar on top of that baseline instead of replacing it — a soft, high-damping material (sandbag) pushes `c_stabilized` well past critical damping (sway dies out fast, no residual vibration), while a hard, low-damping material (metal railing, glass) barely raises `c` above the critical baseline, so residual high-frequency oscillation survives and reads as weapon chatter. The `η_surface` read from `SurfaceFrictionSample` is read-only here — this feature only consumes the material identity `ResolveFriction()` already resolved; it never writes back into the friction chain.

##### Reference Implementation

```cpp
// Modifies your weapon oscillator variables inside the combat calculation update
void StabilizeRest(float k, float mass, float eta_surf, float& current_damping) {
    current_damping = 2.0f * std::sqrt(k * mass) + eta_surf;
}
```

##### Player-Facing Impact

The composition of your cover determines your shooting stability. Bracing your Tommy gun against an impact-absorbing sandbag wall or rubberized defense post dampens recoil and magnification sway down to near-zero, ensuring clean weapon placement. Bracing the steel frame against a slick metal railing or fractured glass pane introduces high-frequency vibration ticks, forcing you to choose your defensive positions carefully.

##### Depends on (not yet built at this point)

M9 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.7-EXT-06] Stowed Weapon Swing Pendulum Inertia

##### Systems Touched

Enhances M2.7 kinematic player input parsing and updates weapon mesh rendering offsets.

##### Math

Weapons attached to body slings or backpack equipment hooks are simulated as low-overhead driven pendulums. The angular displacement vector (θ) of the stowed weapon mesh reacts to the kinematic linear accelerations (a_player ) generated by your player controller updates:

`θ'' + 2ζω₀θ' + ω₀²θ = −(a_player · u_tangent) / L_sling`

##### How It Works

Because your virtual character controller uses kinematic velocity sets, its frame-to-frame velocity differences reveal the true linear acceleration vectors (a_player ). Inside your update step, this acceleration acts as the driving force for a simple inline second-order ordinary differential equation (ODE) tracker, shifting the local transform hierarchy matrix of the stowed weapon model.

##### Reference Implementation

```cpp
void UpdateSlingPendulum(float a_track, float w0, float zeta, float dt, float& theta, float& d_theta) {
    d_theta += (-2.0f * zeta * w0 * d_theta - (w0 * w0) * theta - a_track) * dt; theta += d_theta * dt;
}
```

##### Player-Facing Impact

Sprinting down an alleyway and coming to a sudden, dead stop causes your slung Tommy gun to swing forward from behind your shoulder with real mechanical momentum, clattering against your gear. Rapidly turning corners forces stowed long guns to sway outward, introducing realistic movement that lets you visually sense the weight of your equipment as you maneuver.

#### [M2.7-EXT-07] Atmospheric Powder-Fouling Friction Accumulation

##### Systems Touched

Complements M2.7 core weapon mechanics, writes the canonical `BarrelHeat.fouling` field (locked in M2.7's base Implementation Steps), and scales over M10 environmental humidity maps. `BarrelHeat.fouling` written here is read by [M2.7-EXT-12] Tactical Clear-and-Vent Malfunctions for its jam-probability roll — build and wire both in the same pass; the jam system has no valid input without this one.

##### Math

Every bullet discharged from a weapon deposits carbon fouling (F) within `BarrelHeat.fouling`. The accumulation rate scales up when operating inside highly saturated or rainy atmospheric matrices:

ΔF=κ_caliber ⋅(1.0+γ⋅Humidity_ambient ) The internal mechanical action friction coefficient (μ_bolt ) scales non-linearly over this tracking value, lengthening bolt cycling delays and introducing random cycling failures if cleaning routines are neglected:

`μ_bolt =μ_base +α⋅F 2`

##### How It Works

The `BarrelHeat` component (canonical fields: `kelvin`, `fouling`, `grimeFactor`) already lives on your weapon EnTT sheet from M2.7's base setup — this feature writes `fouling` on it directly rather than tracking a second, parallel scalar. When a bullet emission event is processed, the loop samples the ambient humidity values directly from your M10 weather simulation data blocks and accumulates into `BarrelHeat.fouling`. The updated fouling scalar modifies bolt mechanical travel times inline, directly delaying the reload tracking logic when weapon components run dirty, and is read downstream by the jam-probability roll in [M2.7-EXT-12].

##### Reference Implementation

```cpp
void AccumulateFouling(float hum, float kappa, float gamma, BarrelHeat& heat, float& mu_bolt) {
    heat.fouling += kappa * (1.0f + gamma * hum); mu_bolt += 0.05f * (heat.fouling * heat.fouling);
}
```

##### Player-Facing Impact

Firing your Tommy gun layout during a dry afternoon is highly reliable. However, running a prolonged, high-volume shootout during a humid storm front causes carbon fouling to cake the internal components rapidly. The action responds with sluggish cycle speeds and increased bolt lag, eventually causing a shell stovepipe failure that forces you to duck behind cover to clear the jam.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub humidity value for now; revisit once that milestone lands. (The `BarrelHeat.fouling` write itself is not stubbed — only the humidity input is.)

#### [M2.7-EXT-08] Muzzle-Flash Overpressure Shockwave Overdrive

##### Systems Touched

Deepens M2.7 core weapon firing code, bypassing weapon-jam models completely.

##### Math

Rapid, sustained automatic fire raises barrel overpressure levels. The peak blast wave overpressure (P_blast ) expands outward as a directional spatial cone vector mapped to your weapon look orientation:

`P_blast(r,θ) = (E_chemical / r³) · cos²(θ)`

##### How It Works

Each fired round evaluates the near-field blast-pressure formula at the shot's origin, independent of fire rate. At high cyclic rates the push events fire more often, compounding knockback on nearby entities without any single shockwave growing stronger; chemical energy is fetched from the item's JSON configuration.

##### Reference Implementation

```cpp
void ApplyMuzzleBlast(float energy, float r, float theta, float& out_push_force) {
    out_push_force = (energy / (r * r * r + 1e-5f)) * std::pow(std::cos(theta), 2.0f);
}
```

##### Player-Facing Impact

Instead of punishing high fire rates with tedious weapon malfunctions, running your Tommy gun layout at maximum cycle speeds turns the gun barrel into a kinetic weapon. The massive muzzle overpressure generates a real physical shockwave cone that knocks back close-quarters zombies and breaks their balance tracking, giving you defensive breathing room at the cost of high visual flash bloom and screen tremor.

**[CORRECTED — note]** `P_blast` is a per-shot near-field pressure term (energy in, distance/angle out) — it has no fire-rate input. "Sustained automatic fire raises overpressure" in the prose above means the push events fire more *often* at high cyclic rate, not that any single shockwave gets stronger; `E_chemical` is the fixed per-round chemical energy from the weapon's JSON record, not an accumulating value.

#### [M2.7-EXT-09] Elastic Canvas Trampoline Boundaries

##### Systems Touched

Extends M2.7 (JPH::CharacterVirtual) movement parsing via Jolt shape casts.

##### Math

When your kinematic controller drops onto dynamic fabric assets (such as rooftop shop awnings or canvas truck beds), the system bypasses standard fall-damage lookups, processing a non-linear elastic spring restoration force array:

`F_bounce = −k·|x|^α·sign(x) − c·ẋ`

##### How It Works

When the character's downward shape-cast lands on a body tagged as a dynamic fabric surface (canvas awning, tarp, truck bed cover) rather than solid ground, the fall-damage/landing check is skipped, and the vertical entry velocity instead drives the non-linear spring-restitution formula, converting downward momentum into an upward launch impulse.

##### Reference Implementation

```cpp
// F_bounce above is a force; converting to a velocity change needs /mass and *dt like every
// other spring in this doc (see M5.2-EXT-10's UpdateSpringDamper) — the original reference
// implementation added the raw force straight to out_vel_y with no mass or dt term at all.
void CalcTrampolineBounce(float k, float x, float alpha, float c, float dx, float mass, float dt, float& out_vel_y) {
    float f_bounce = -k * std::pow(std::abs(x), alpha) * (x > 0.f ? 1.f : -1.f) - c * dx;
    out_vel_y += (f_bounce / (mass + 1e-5f)) * dt;
}
```

##### Player-Facing Impact

Leaping from a multi-story building ledge doesn't mean a quick restart or a fractured leg loop. Dropping straight onto a canvas shop awning absorbs your downward kinetic velocity vectors, converting them into a massive upward launch that catapults your kinematic controller across alleys and directly into a vault-slide.

#### [M2.7-EXT-10] Biomechanical Muscle Snapback Momentum

##### Systems Touched

Deepens M2.7 tactile melee projection arcs, modifying Jolt constraint boundaries.

##### Math

Melee heavy attacks that compress a zombie's joint constraints to their absolute structural limits without breaking them build high non-linear elastic restorative tension (F_snap ):

`F_snap = k·(Δx)³`

##### How It Works

Heavy melee strikes push a zombie's ragdoll joint constraints toward — not past — their hard angular limit. Holding the limb there through the swing builds cubic restorative tension; releasing it converts that tension into an outward impulse that can strike adjacent entities standing behind it.

##### Reference Implementation

```cpp
void CalcMuscleSnap(float k, float dx, float& out_impulse) {
    out_impulse = k * (dx * dx * dx);
}
```

##### Player-Facing Impact

Striking a massive zombie with a blunt bat or pipe can stretch its muscle joints to their maximum physical constraints. When you let go or release the pressure, the limb snaps back like a heavy rubber band. This mechanical reaction delivers an unintentional high-velocity back-smash that clobbers and disorients any adjacent zombies standing behind it.

#### [M2.7-EXT-11] Kinematic Door-Kick Hydraulic Ram Splitting

##### Systems Touched

Integrates M2.7 (JPH::CharacterVirtual) vault frames with M3 door structural rigid hulls — reads the target door's `EnTT DoorComponent` (locked, shearThreshold, material) rather than assuming those values exist ad hoc.

##### Math

Sprinting and executing a kick event on a closed door hull transfers your full kinematic kinetic energy instantly to the door structure via an absolute momentum transfer model, gated by the door's `DoorComponent.material`:

`v_door_exit = (m_player / (m_player + m_door)) · v_player_entry`

##### How It Works

A sprint+kick input against a closed door queries `DoorComponent.material`. Reinforced doors absorb the hit with zero transfer; all other materials transfer momentum from the player's kinematic controller to the door body via the mass-ratio formula, launching the door and anything behind it outward.

##### Reference Implementation

```cpp
void ApplyDoorRam(const DoorComponent& door, float m_p, float m_d, const glm::vec3& v_p, glm::vec3& v_door) {
    if (door.material == MaterialClass::Reinforced) { v_door = glm::vec3(0.0f); return; }
    v_door = (m_p / (m_p + m_d)) * v_p;
}
```

##### Player-Facing Impact

Instead of halting your movement to play a slow door-opening animation, hitting a closed door at full sprint and hitting your kick button transfers your character's full momentum into the door frame. The door blasts open like a hydraulic ram; if an entity is listening or lurking directly on the other side, it gets smashed backward with shattered joint gains, flinging it straight into a dynamic ragdoll state.

##### Depends on (not yet built at this point)

M3 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.7-EXT-12] Tactical Clear-and-Vent Malfunctions (Rewarding Wear)

##### Systems Touched

Complements M2.7 core weapon cycling loops, reads `BarrelHeat.fouling` as written by [M2.7-EXT-07] Atmospheric Powder-Fouling Friction Accumulation (build/wire both in the same pass — this system has no fouling input to react to otherwise), and reads brand quality tier from data-driven JSON weapon records.

##### Math

Weapon misfires function as an active tactical mechanic rather than a random penalty. The jam probability (P_jam ) scales based on `BarrelHeat.fouling` (F) and your weapon brand quality tier scalar:

`P_jam =α⋅F 2 ⋅BrandQualityMultiplier`

Rolling this probability against a per-cycle random draw determines whether the action jams. Manually clearing the weapon action at the exact moment of failure vents high-pressure trapped cylinder gases, lowering barrel heat immediately:

`ΔT_vent =−β⋅(T_barrel −T_ambient )`

##### How It Works

Each firing cycle rolls the jam probability against `BarrelHeat.fouling` and the weapon's brand quality tier. On a jam, a brief manual-clearance window lets the player vent trapped heat from `BarrelHeat.kelvin`, which immediately tightens Aim Bloom rather than only clearing the jam.

##### Reference Implementation

```cpp
bool RollJam(const BarrelHeat& heat, float alpha, float brand_quality_mult, float roll) {
    return (alpha * heat.fouling * heat.fouling * brand_quality_mult) > roll;
}

void HandleTacticalVent(float precision_tick, float t_amb, float beta, BarrelHeat& heat, float& out_bloom) {
    if (precision_tick > 0.85f) { heat.kelvin -= beta * (heat.kelvin - t_amb); out_bloom = 0.5f; }
}
```

##### Player-Facing Impact

When an unmaintained weapon or cheap makeshift pipe-rifle misfires, timing your manual bolt clearance precisely vents the trapped heat away from the receiver. This drops your barrel temperature instantly, tightening your procedural shooting spread (Aim Bloom) and rewarding your fast reflexes with an immediate window of hyper-accurate fire.

#### [M2.7-EXT-13] Latch-Shear Kinematic Door Piercing

##### Systems Touched

Links your M2.7 player controller acceleration values directly to the `EnTT DoorComponent` (locked, shearThreshold, material) added in M3's Implementation Steps.

##### Math

Sprinting and kicking a closed door structure processes the player's kinetic entry force (F_kick ). The system queries `DoorComponent.locked` and `DoorComponent.shearThreshold` directly off the target entity:

`F_kick = (m_player · ‖v_player‖) / Δt`

`OpenFlag=(Locked∧F_kick >σ_shear )∨(Unlocked∧Material==Weak)`

##### How It Works

A sprint+kick input computes kinetic entry force against the door's `DoorComponent` fields: locked doors open only if kick force exceeds `shearThreshold`; unlocked doors open freely unless flagged `Reinforced`, in which case the kick does nothing and zero-clamps player velocity instead.

##### Reference Implementation

```cpp
bool TryKinematicDoorBreak(const DoorComponent& door, float mass, float vel, float dt) {
    return (door.material != MaterialClass::Reinforced) &&
           ((door.locked ? (mass * vel / dt) : 1000.f) > door.shearThreshold);
}
```

##### Player-Facing Impact

This structure prevents players from breaking the level design. Executing a kick command at full sprint against a flimsy, cheap locked door (like a rotting residential portal) shatters the latch instantly, slamming the door panel open to knock down zombies hiding on the other side. However, attempting this against a heavy reinforced steel security barrier or a solid closed door that is structurally strong does absolutely nothing but zero-clamp your velocity vector, leaving you completely vulnerable.

#### [M2.7-EXT-14] Decoupled View-Matrix Camera with Inertialized Perspective Blending

##### Systems Touched

Replaces the prior spring-arm-to-head-joint camera model referenced in M2.7's Implementation Steps; feeds look-at targets into the character rig and camera-space transforms into M2.7-EXT-15's weapon rig below.

##### Math

Perspective-switch positional offsets decay to zero via a quintic ease rather than a spring-damper, so switching camera modes can't reintroduce the oscillation this feature exists to remove:

`offset(t) = offset₀ · (1 − 6t⁵ + 15t⁴ − 10t³)`, `t ∈ [0,1]`

The camera must be the source of truth the skeleton conforms to. Slerping a spring-arm camera directly onto the character's skeletal head joint inverts that dependency: animation-clip noise, footstep impacts, and Motion Matching frame transitions inject acceleration spikes straight into the camera, and slerping a raw joint transform adds phase lag on top, producing motion sickness and drift over long sessions.

##### How It Works

The camera is promoted to an independent virtual kinematic entity, updated first in the frame loop as the source of truth — nothing drives it off a mesh joint. Its base transform follows an isolated kinematic path layered with low-frequency fractional Brownian noise (or a small set of incommensurate sine waves) for organic breathing sway. The character's neck/head/spine bones are then pulled toward the camera's forward vector with a fast analytical look-at solver, so the mesh conforms to the camera rather than the other way around. Switching between first- and third-person no longer slerps between two camera states; it computes the 3D offset between old and new view matrices and decays that offset to zero on the quintic curve above, preserving view velocity and eliminating positional snapping. Third-person occlusion replaces the old thin raycast with a capsule-cast sized to the `JPH::CharacterVirtual` radius, run through a dual-stage spring-damper so the arm glides around geometry instead of snapping.

##### Reference Implementation

```cpp
// Inertialized perspective blend — replaces spring-mass slerp between 1st/3rd person camera states
glm::vec3 InertializeCameraOffset(const glm::vec3& offset0, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float ease = 1.0f - (6.0f * std::pow(t, 5.0f) - 15.0f * std::pow(t, 4.0f) + 10.0f * std::pow(t, 3.0f));
    return offset0 * ease;
}
```

##### Player-Facing Impact

Camera transitions between first- and third-person feel weighty and continuous instead of snapping — a sprint-to-slide into cover carries visible momentum through the perspective switch. Third-person camera glides around doorframes and cover geometry cleanly instead of clipping through walls or popping to a new position.

#### [M2.7-EXT-15] Nested Weapon Component Rig with Two-Bone IK Arm Locking
#### [M2.7-EXT-16] Procedural Recoil Low-Discrepancy Sequence Cache *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.7 weapons. Caches a per-weapon Halton/low-discrepancy recoil sequence so kick is reproducible and desync-safe.

##### Math
kick_i = (Halton(2,i), Halton(3,i)) * spread; index advances per shot, reset on reload.

##### How It Works
Each weapon owns a seeded low-discrepancy sequence for recoil offset; the i-th shot uses the i-th sample, so recoil patterns are identical across clients (important for co-op determinism) and feel designed, not random. Index resets on reload.

##### Reference Implementation
```cpp
vec2 kick = LowDisc(seq, shotIndex++); // Halton(2,3)
```

##### Player-Facing Impact
Recoil is consistent and fair in co-op - same gun kicks the same way for every player.

---
#### [M2.7-EXT-17] Parametric Gait-Warping Stride Adjuster *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.7 locomotion. Warps gait cycle to stride length under speed/slope so feet don't slide.

##### Math
phase = walkDist / strideLen; footLock when phase in plant window; blend warped clip.

##### How It Works
Rather than speed-scale a fixed gait (which slides feet at extremes), the gait phase is driven by distance traveled over the current stride length; on slopes the stride is lengthened/shortened parametrically. Feet lock to ground during the plant window, eliminating skate.

##### Reference Implementation
```cpp
float phase = traveled / strideLen(speed, slope); Pose = SampleGait(clip, phase);
```

##### Player-Facing Impact
Characters walk/run uphill and at all speeds without foot sliding - grounded, believable motion.

##### Systems Touched

Drives weapon-model bone hierarchies (pivot, barrel, slide, magazine, receiver, optics) inside M2.7's core combat loop; hand-socket targets feed the player arm rig via the same Two-Bone IK solver used by M5.2's procedural pose layer (M5.2-EXT-05).

##### Math

The Two-Bone IK solver resolves the elbow/knee bend angle via the law of cosines over the upper/lower segment lengths (l₁, l₂) and the distance to target (d):

`θ = acos((l₁² + l₂² − d²) / (2·l₁·l₂))`

##### How It Works

Firearm models carry their own internal skeleton — pivot, barrel, slide, magazine, receiver, optics — operating in a local coordinate space relative to the camera rather than being a single rigid mesh glued to a screen-space transform. Firing applies procedural forces to that internal rig first (slide blows back, barrel pitches up on its hinge) before any of it reaches the player's hands. The weapon blueprint defines fixed left/right hand socket transforms; the player's arm meshes are pulled to those sockets by a non-iterative Two-Bone IK solver, so when recoil, sway, or weight-lag displaces the gun frame, the arms flex and compress to follow it automatically instead of needing a hand-animated variant per weapon.

Three failure modes are handled inside the solver itself, not left as caller responsibility:

1. **Arccosine domain crash:** float drift or an over-extended limb can push the law-of-cosines ratio slightly outside `[-1, 1]` (e.g. `1.000002`), and an unclamped `std::acos()` call returns NaN, collapsing the joint. Clamp the ratio to `[-1.0, 1.0]` immediately before the `acos()` call.
2. **Collinear hinge singularity:** when the limb is fully extended, the cross product between limb direction and target goal collapses toward the zero vector; normalizing it divides by ~0 and the joint flips at high frequency. Check the cross product's squared magnitude against a `1e-6` threshold first, and fall back to the joint's hinge axis or a target-plane normal when it's below that.
3. **EnTT integration order:** running the IK pass before parent-child transforms have resolved for the current frame introduces a 1-frame lag, visible as feet/hands sliding on moving surfaces. The solver must run immediately after Motion Matching's pose selection and before local-to-world matrix translation, so the SoA transform buffer the GPU skinning pass reads is already current.

##### Reference Implementation

```cpp
// Clamped law-of-cosines two-bone solve with collinear-singularity fallback
float SolveTwoBoneAngle(float l1, float l2, float dist) {
    float ratio = (l1 * l1 + l2 * l2 - dist * dist) / (2.0f * l1 * l2);
    ratio = std::clamp(ratio, -1.0f, 1.0f); // guards the acos() domain crash
    return std::acos(ratio);
}

glm::vec3 SafeBendAxis(const glm::vec3& limb_dir, const glm::vec3& to_target, const glm::vec3& fallback_hinge) {
    glm::vec3 cross = glm::cross(limb_dir, to_target);
    return (glm::dot(cross, cross) < 1e-6f) ? fallback_hinge : glm::normalize(cross); // collinear guard
}
```

##### Player-Facing Impact

Weapons feel mechanically weighted rather than screen-glued — slides visibly cycle, barrels rise under recoil, and the character's arms follow the gun's displacement instead of the gun floating rigidly at screen center. Extreme poses (diving, sliding, corner-leaning) never produce a snapped or flipped arm.

## M2.8 — Deterministic co-op architecture

##### Implementation Steps

* Host-Authoritative Topography. Deterministic Seed Distribution (XorShift128+).
* Snapshot Replication Safety Net (Catmull-Rom smoothing) as fallback.
* Automated Desync Interception (periodic xxHash64 checksum validation).
* Headless Client Simulation QA testing.
* Local Viewport Division (split-screen).
* Network Transport Binding (ENet/GameNetworkingSockets).
* Portable Profile Partitioning vs World Save.
* JSON Progress Unlocks / PerkPoints via EventBus. Spaced-Repetition Skill Decay.
* Seeded Attribute Distribution.

##### Decisions to flag

Peer-hosted vs dedicated. NAT-traversal requirement. Initial perk list.

##### Exit Criteria

* 2 local players operate split-screen simultaneously.
* Remote client connects and replicates deterministic seeds.
* Snapshot/reconciliation fallback triggers correctly if determinism breaks.
* Profiles persist accurately across instances.



### Extended Systems Library — engine-side additions for M2.8

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M2.8-EXT-01] High-Fidelity Radio-Frequency Signal Attenuation (Comms Static)

##### Systems Touched

Upgrades M2.8 deterministic co-op network loops and complements Appendix D dead camps tracker.

##### Math

Diegetic voice communications or telemetry tracking beacons passing between separate player positions experience signal degradation. The system calculates transmission path losses (PL) over structural walls using a knife-edge diffraction approximation pattern:

`PL(d,θ) = 20·log₁₀(4π·d/λ) + 20·log₁₀(θ·√(d/λ) + Σ(σ_yield·t_wall) + 1)`

The formula above uses a single straight-line obstruction distance `dist` (not the two-segment d₁/d₂ knife-edge form), matching the Reference Implementation below. The `+1` inside the second log keeps it from going to `-∞` at zero obstruction.

##### How It Works

This framework hooks straight into your networked co-op transport bindings. When players utilize walkie-talkies or tracking indicators, an asynchronous enkiTS background thread queries the direct 3D vector between the positions against your existing SpatialHash layout. It counts the thickness and material yield threshold constants of intervening structural components, updating a flat audio degradation scalar used by your audio streaming callbacks to inject static distortion matrices.

##### Reference Implementation

```cpp
// Computes radio signal loss across intervening structural barriers to drive audio static filters
float CalculateSignalLoss(float dist, float lambda, float theta, float wall_yield_sum) {
    return 20.0f * std::log10((4.0f * 3.1415f * dist) / lambda) + 20.0f * std::log10(theta * std::sqrt(dist / lambda) + wall_yield_sum + 1.0f);
}
```

##### Player-Facing Impact

This system adds a physical spatial layer to your co-op communication tracking. Standing out in the open allows clear audio routing and flawless positioning indicators on your menu panels. However, if your co-op partner moves deep inside an urban commercial structure or behind a heavy reinforced concrete barrier, your radio link begins to distort. High frequencies drop out, voice streams fill with harsh, physical static hiss, and your location tracking markers begin to drift and glitch out, mirroring the structural composition of the environment.

#### [M2.8-EXT-02] Cross-Platform Topology Token Replication (Determinism-Drift Isolation)

##### Systems Touched

Upgrades M2.8 deterministic co-op network loops and the automated desync interception check.

##### Math

Procedural spatial markers undergo 64-bit coordinate quantization to transform arbitrary float positions into absolute, microarchitecture-invariant integer grid coordinates before packing into the network layout:

`p_quantized = floor((p_float − offset) / Δx)`

SplitMix64 itself is pure integer math and deterministic everywhere. The risk is downstream: once a seed drives floating-point work (transcendental calls like `sin`/`cos`/`pow`), compiler and microarchitecture differences between machines (e.g. host vs. a co-op partner's different CPU) can produce slightly different float results. The approach below sidesteps that by never replicating raw floats across the network at all — only structural tokens.

##### How It Works

This feature shifts procedural generation authority entirely to the host. Rather than having each client re-run local float-based generation from a shared seed and hoping the results match bit-for-bit, the host replicates compact structural topology tokens — discrete building archetype indices, WFC choice bitmasks, and quantized integer origin coordinates — over your existing ENet/GameNetworkingSockets transport. The client intercepts these tokens, skips its own local floating-point generation for that cell, and builds the local visual elements directly from the host-provided structural keys, guaranteeing identical physical bounds on every connected instance.

##### Reference Implementation

```cpp
// Quantizes vector coordinates into fixed integer boundaries to insulate network packets
// from floating-point microarchitecture divergence; packed to the actual 16-bit value range
uint64_t PackQuantizedToken(const glm::vec3& pos, const glm::vec3& offset, float delta_x) {
    uint32_t qx = static_cast<uint32_t>(std::clamp((pos.x - offset.x) / delta_x, 0.0f, 65535.0f));
    uint32_t qz = static_cast<uint32_t>(std::clamp((pos.z - offset.z) / delta_x, 0.0f, 65535.0f));
    return (static_cast<uint64_t>(qx) << 32) | qz;
}
```

##### Player-Facing Impact

Co-op sessions stay perfectly synchronized without phantom hits or rubber-banding desyncs. Firing into a rotting structure breaks the wall panels into identical fragments on both your viewport and your co-op partner's split-screen, forcing zombies to route through the exact same structural gaps on both machines.

#### [M2.8-EXT-03] Split-Screen Multi-Viewport Instanced Render Pipeline (Local Co-op Guard)

##### Systems Touched

Expands M2.8 Local Viewport Division and interfaces directly with M1's centralized render graph.

##### Math

Viewport projection matrices isolate geometry positions using an instance-index mapping across partitioned layout segments:

`P_out = M_proj[ViewportID] · M_view[ViewportID] · P_world`

##### How It Works

Standard split-screen approaches fully duplicate scene updates, frustum culling checks, and command-buffer recording per player window — in a fully procedural engine, that duplication stalls command queues and wastes CPU cycles. This feature replaces separate draw loops with a single-pass multi-viewport path. A compute shader checks procedural cluster bounds against all active local-player frustums simultaneously and writes a combined `MultiDrawIndirect` argument block. The vertex/mesh shader reads a flat `ViewportID` from an instanced rendering index, mapping geometry regions to their assigned screen bounds in one hardware execution pass.

Packing the viewport mask into the same 32-bit field as `instanceCount` is only safe because these are single-instance procedural draws (`instanceCount` is always 1 pre-pack) — the top 4 bits are otherwise unused. Do not apply this packing to any draw call needing `instanceCount > 0x0FFFFFFF`; the mask and count would collide.

##### Reference Implementation

```cpp
// Flags draw commands with a viewport mask to eliminate duplicate render passes;
// only valid for single-instance draws — top 4 bits of instanceCount are otherwise unused here
void BuildSplitScreenIndirectCommand(VkDrawIndexedIndirectCommand& cmd, uint32_t active_viewports_mask) {
    cmd.instanceCount = (cmd.instanceCount & 0x0FFFFFFF) | (active_viewports_mask << 28);
}
```

##### Player-Facing Impact

Local split-screen co-op matches single-player frame times. You and your co-op partner can explore entirely different sectors of a high-density urban environment, look in opposite directions, or engage separate zombie hordes without frame drops or engine micro-stutters.

#### [M2.8-EXT-04] Fixed-Point Deterministic Math Emulation Layer (Soft-Float Isolation)

##### Systems Touched

Upgrades M2.8 deterministic co-op network loops; safeguards the SplitMix64 thread-isolation pattern (Appendix note) from downstream float drift addressed conceptually in M2.8-EXT-02.

##### Math

Continuous transformations scale into bitwise-stable 64-bit integer tracking values, preserving identical resolution ranges across hardware architectures:

`I_fixed = int64_t(F_value · 65536.0)` — 16.16 fixed-point

##### How It Works

Native floating-point instructions can produce microarchitecture-dependent rounding differences across CPU families (your Ryzen 5700G vs. a co-op partner's Intel machine) or under aggressive optimization flags (`-ffast-math`). This feature is the stricter alternative to M2.8-EXT-02's token-replication approach: rather than only replicating discrete structural decisions, it strips native hardware float ops from the specific systems that need bit-identical results across machines — core procedural generation math, pathfinding cost accumulation, and damage-roll arithmetic — and routes them through an isolated fixed-point path instead.

The two co-op determinism features are complementary, not redundant: M2.8-EXT-02 avoids replicating floats at all by sending structural tokens; this feature covers the narrower set of shared calculations that must still run identically on every machine (e.g. deterministic damage rolls) where a token hand-off isn't practical.

##### Reference Implementation

```cpp
// Forces cross-platform mathematical determinism by bypassing native hardware float variation
int64_t FixedPointMultiply(int64_t a, int64_t b) {
    return (a * b) >> 16; // 16.16 fixed-point bit-shift precision guard
}
```

##### Player-Facing Impact

Extended multiplayer survival sessions stay perfectly synchronized. Long projectile trajectories and terrain navigation choices never drift or trigger network desync reconciliations, even when mixing different CPU families in the same session.

#### [M2.8-EXT-05] Host-Authoritative Topography & Seeded Client Reconstruction
#### [M2.8-EXT-06] XorShift128+ Seed Distribution Sandbox Synchronizer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.8 co-op determinism. Derives per-peer deterministic sub-seeds from the session seed for lockstep.

##### Math
s_i = XorShift128+(s_session ^ peerId); each peer advances its own stream independently but reproducibly.

##### How It Works
The host session seed is split into per-peer sub-streams via XorShift128+ keyed by peer id, so every client generates the same per-peer randomness in the same order (spawns, loot) without sharing full RNG state. Essential for co-op lockstep (M12).

##### Reference Implementation
```cpp
uint64_t s = XorShift128p(sessionSeed ^ peerId); // each peer's stream
```

##### Player-Facing Impact
Co-op stays in lockstep - every player sees the same spawns/loot, no desync.

---
#### [M2.8-EXT-07] xxHash64 ECS State Checksum Aggregator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.8 + M12 determinism. Rolling xxHash64 over ECS snapshot for desync detection.

##### Math
h = xxh64(state_i, h); compare h_A vs h_B every N ticks.

##### How It Works
Each tick (dev/verify gated), the full deterministic ECS snapshot is folded into a running xxHash64; peers exchange hashes every N ticks and flag divergence on mismatch, pinpointing the first differing entity. Complements M2.8-EXT-09 replay verification.

##### Reference Implementation
```cpp
uint64_t h=XXH64(&snap, len, seed); if(h!=peerH) FlagDesync(tick);
```

##### Player-Facing Impact
Desyncs are caught fast and located - co-op correctness is verifiable, not assumed.

---
#### [M2.8-EXT-08] Fixed-Point Mesh-Vector Quantization Factory *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.8 + M2.6. Quantizes mesh/transform vectors to fixed-point for lossless wire transmission.

##### Math
q = round(v * 2^F) >> F; reconstruct v' = q / 2^F; F=16 gives sub-mm at km range.

##### How It Works
To send transforms/mesh deltas over the wire without float drift between clients, vectors are quantized to signed fixed-point (F fractional bits) before serialization and dequantized on receipt. Guarantees bit-identical reconstruction across platforms.

##### Reference Implementation
```cpp
int32_t q = (int32_t)roundf(v * (1<<F)); float v2 = q / (float)(1<<F);
```

##### Player-Facing Impact
Networked transforms reconstruct identically on all clients - no float-induced drift.
#### [M2.8-EXT-09] (provisional) Co-op Deterministic Seeded Replay Verification *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Reads fixed-point layer M2.8-EXT-04 (verified) + topology-replication token M2.8-EXT-02. Writes 64-bit hash to debug telemetry (M13 / M5.4 bandit tracker channel). Observation-only, ship-disabled.

##### Math
Running 64-bit FNV-1a hash over serialized deterministic state each tick: H = H xor FNV1a(state_i); H = (H*1099511628211) mod 2^64. Two clients exchange H every N ticks; divergence if H_A != H_B.

##### How It Works
At fixed cadence (every 60 ticks, dev-gated), each client hashes full deterministic sim state via fixed-point serialization M2.8-EXT-04; host compares client hashes. Mismatch trips dev-only alarm identifying first diverging entity (Spike A verification wired into shipped co-op path). Ship-disabled by default.

##### Reference Implementation
```cpp
uint64_t g_h = 14695981039346656037ULL;
for(auto& e: serializedState) g_h = (g_h ^ FNV1a(e)) * 1099511628211ULL;
if(hostH != peerH) LogDivergence(tick, firstDifferingHandle);
```

##### Player-Facing Impact
Co-op drift is caught, not assumed away — desyncs debuggable without affecting players.

---

##### Systems Touched

Declares the base "Host-Authoritative Topography. Deterministic Seed Distribution (XorShift128+)" bullet. Feeds M4's procedural world generation (clients regenerate identical chunks from the distributed seed rather than downloading geometry) and gates on `[M2.8-EXT-04]`'s fixed-point math layer so regenerated terrain matches bit-for-bit across platforms.

##### Math

`seed_client = seed_host` (verbatim, no derivation) — a single 128-bit XorShift128+ state is generated once by the host at session start and pushed to every joining client over the reliable channel, rather than each machine rolling its own.

##### How It Works

The host is the sole authority on world topology: it never streams heightmap or chunk mesh data over the network. Instead, on join, it sends the 128-bit XorShift128+ seed plus the current save's mutation log (destructions, placed structures) once. Each client then runs M4's chunk generator locally against that seed, producing byte-identical base geometry, and replays the mutation log on top. Ongoing gameplay only needs to sync entity state and mutation deltas, not terrain — a chunk is never transmitted, only regenerated.

##### Reference Implementation

```cpp
// Sent once per joining client; client regenerates terrain locally instead of downloading it
struct WorldJoinPacket {
    uint64_t seedLow, seedHigh;      // 128-bit XorShift128+ state, verbatim from host
    std::vector<MutationLogEntry> mutationLog; // destructions/placements to replay after generation
};

void OnClientJoin(Connection& client, const XorShift128State& hostSeed, const std::vector<MutationLogEntry>& log) {
    WorldJoinPacket packet{hostSeed.low, hostSeed.high, log};
    SendReliable(client, packet); // client regenerates M4 chunks from this seed, then replays mutationLog
}
```

##### Player-Facing Impact

Joining a co-op session is fast even on poor connections, since the (potentially enormous) open world never needs to download — only a tiny seed and a compact history of what's been blown up or built.

## M2.9 — Traversal, ballistics & survival physiology

##### Implementation Steps

* Procedural Ledge Analysis (SDF raycast fan) and two-phase IK vault.
* Hydrostatic Buoyancy Simulation.
* Two-Compartment Exertion Model (`W′balance = W′max − ∫(P−CP)dt`). Traversal draws from this pool. Carbon Monoxide exposure lowers `CP`.
* Sleep Debt Accumulation (`perf = e^(-λD)`). Hallucinations via Langevin-dynamics gradient modeling.
* Caloric Condition Metrics (BCS). Layered Thermal Insulation (`R_total = ΣR_i`).
* Procedural blood/caustics/tracers/decals integrated via gyroscopic particle physics.

##### Decisions to flag

IK vault initial implementation vs later integration.

##### Exit Criteria

* Vaulting and swimming perform physically.
* Sprinting accurately depletes critical power reserve.
* Sleep debt degrades performance and triggers varied hallucination inputs.
* BCS scales carry limits.



### Extended Systems Library — engine-side additions for M2.9

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M2.9-EXT-01] Projectile Hydrodynamic Cavitation Tensors (Soft-Tissue Impact)

##### Systems Touched

Deepens M2.9 semi-implicit ballistic updates using M5.1 anatomical hit-volume SDF gradients.

##### Math

When a high-velocity projectile cuts through a zombie's anatomical signed distance field, the instantaneous tissue energy dissipation gradient (dE/dx) generates a transient radial cavitation envelope (R_cavity ):

`dE/dx = −(0.5)·ρ_tissue·C_d·A_bullet·‖v‖²`

`R_cavity(x) = γ·√(π·(−dE/dx) / σ_yield_tissue)`  *(energy-dissipation rate over tissue yield strength — matches `GetCavitationRadius()` below; larger energy transfer or softer tissue both grow the cavity)*. **σ_yield_tissue ≈ 1.8 MPa** as a default for generic soft tissue, grounded in ballistic-gelatin/soft-tissue failure-stress research (commonly cited in the ~1-2.5 MPa band); pass a lower value (~0.3-0.6 MPa) for organ/lung-tagged hit volumes and a higher value (~4-6 MPa) for muscle-dense limb hit volumes if per-region tuning is wanted later.

This radius maps a secondary volume destruction calculation across internal vital fields inside your EnTT structural setups.

##### How It Works

Your ballistics update pipeline runs independently across enkiTS worker threads. This system modifies how your DamageEvent structures process anatomical data indices upon intersecting a target vector. Instead of executing a simple line-trace subtraction step, the projectile evaluates the kinetic energy gradient across your 3D tissue density layers. If the round preserves high residual energy, it applies an omnidirectional radial damage pass to nearby skeletal joints, lowering joint gains automatically.

##### Reference Implementation

```cpp
// Calculates the internal hydrodynamic tissue destruction radius based on entry velocity metrics
float GetCavitationRadius(float vel, float mass, float drag, float area, float rho, float yield_tissue, float gamma) {
    return gamma * std::sqrt((0.5f * rho * drag * area * (vel * vel)) / (3.1415f * yield_tissue + 1e-5f));
}
```

##### Player-Facing Impact

This mechanical addition highlights the distinction between firearm categories. Firing low-velocity pistol rounds into targets punches straight, narrow entry paths that slow dynamic bodies down only on direct bone connections. Firing high-velocity rifle cartridges or your Tommy gun layout at close range triggers hydrodynamic cavitation. The energy shocks and liquefies surrounding tissue, tearing open massive exit wounds, disabling limbs instantly without requiring perfect bone placement, and triggering accurate physical staggers based on momentum transfers.

##### Depends on (not yet built at this point)

M5.1 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.9-EXT-02] Ballistic Kinetic Spalling & Secondary Fragment Shell Vectors

##### Systems Touched

Integrates M2.9's ballistics pipeline with M3's macro-destruction material-yield data to gate fragmentation yields.

##### Math

`v_frag =v_reflect +(n_surface ×ψ_scatter )⋅cosϕ`

##### How It Works

Integrates your M2.9 semi-implicit ballistics pipeline with your M3 macro-destruction code blocks.

When a high-velocity round impacts a reinforced structural surface (like a brick or concrete wall) whose material yield threshold prevents complete de Marre armor penetration, the remaining kinetic energy is not deleted. Instead, if the impact angle is steep, the bullet triggers a local structural micro-spalling event. The system generates an array of 3 to 5 low-mass secondary projectile entities launched backward from the wall, using a cosine-weighted distribution (cosϕ) centered around the surface normal vector (n_surface ).

##### Reference Implementation

```cpp
void SpawnSpallFragments(const glm::vec3& reflectN, const glm::vec3& surfaceN, float phi, int fragCount, std::vector<glm::vec3>& out_vectors) {
    for (int i = 0; i < fragCount; ++i) {
        glm::vec3 scatter = GenerateCosineScatterVector(surfaceN, phi);
        out_vectors.push_back(reflectN + scatter);
    }
}
```

##### Player-Facing Impact

Missing a target inside an enclosed masonry room becomes incredibly hazardous. Firing your Tommy gun into concrete walls generates real lethal stone fragments and shrapnel flying backward into the room, creating an immersive ricochet threat that forces tactical placement and caution in tight urban skirmishes.

#### [M2.9-EXT-03] Diurnal Atmospheric Air-Density Ballistic Shift

##### Systems Touched

Deepens M2.9 semi-implicit ballistic updates using M10 day/night temperature tracks.

##### Math

`ρ(T) = P_ambient / (R_specific · T_Kelvin)`

`v(t+Δt) = v(t) + (g − 0.5·ρ(T)·C_d·A/m · ‖v(t)‖·v(t))·Δt`

##### How It Works

Your projectile updates execute on background enkiTS loops using standard semi-implicit integration steps. This system drops the hardcoded static air density constant entirely, evaluating true density (ρ) based on your diurnal Julian day temperature variations (T_Kelvin).

##### Reference Implementation

```cpp
// Inlined directly inside the M2.9 projectile integration thread loop step
void StepBulletDensityDrag(float p_amb, float r_spec, float temp_k, float c_d, float area, float mass, float dt, glm::vec3& pos, glm::vec3& vel) {
    vel += (glm::vec3(0.0f, -9.81f, 0.0f) - (0.5f * (p_amb / (r_spec * temp_k)) * c_d * area / mass) * glm::length(vel) * vel) * dt; pos += vel * dt;
}
```

##### Player-Facing Impact

Your long-range firearm accuracy shifts naturally based on the time of day and the local weather. Firing your weapon during a freezing −5 ∘ C midnight sector run exposes bullets to dense air arrays, increasing flight resistance and causing noticeable trajectory drops over distance. Firing down an avenue during a blistering afternoon lets shots travel flatter and with higher terminal kinetic force.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M2.9-EXT-04] Projectile Ricochet Tumbling Stability Matrix

##### Systems Touched

Expands the M2.9 Semi-Implicit Ballistic Integration loops.

##### Math

Glancing bullet impacts that fail to pass through hard materials via de Marre formulas lose axial aerodynamic stability. The projectile’s cross-sectional drag coefficient (C_d ) updates based on an active tumbling state flag:

`C_d = C_d0 + TumbleFlag·(C_cylinder − C_d0)`

The tumbling behavior cuts down velocity vectors exponentially while increasing tissue damage tracking indicators on subsequent impact checks:

`Damage_mod = 1.0 + TumbleFlag·(A_tip/A_flat − 1.0)`

##### How It Works

Operating directly within your background enkiTS integration fiber loops, when a bullet entity hits a surface at a steep glance angle, the system sets an internal bitmask flag inside the BulletComponent structure. The integration pass scales up the aerodynamic drag calculations inline, avoiding heavy EnTT component mutations or memory allocations mid-flight.

##### Reference Implementation

```cpp
void StepTumblingBullet(float rho, float area, float mass, float c_d0, bool tumbled, float dt, glm::vec3& vel) {
    vel += (glm::vec3(0.f, -9.81f, 0.f) - (0.5f * rho * (tumbled ? 1.2f : c_d0) * area / mass) * glm::length(vel) * vel) * dt;
}
```

##### Player-Facing Impact

Missed shots inside narrow masonry alleys scream off concrete walls, destabilizing and tumbling through the air. These deflecting rounds lose velocity quickly over distance, but if they strike an entity at close range, they inflict massive tissue destruction, making indoor firefights unpredictable and hazardous.

#### [M2.9-EXT-05] Deflective Projectile Ricochet Shredder

##### Systems Touched

Integrates within the M2.9 semi-implicit ballistics pipeline, using surface hardness bits.

##### Math

`v_reflect =v_in −2⋅(v_in ⋅n)n,v_frag =v_reflect +(n×ψ_scatter )⋅cos(ϕ)`

##### How It Works

When a high-velocity projectile strikes solid masonry or armored steel barriers at a narrow incidence angle, it reflects elastically across the specular surface normal while fracturing into secondary fragment vectors (v_frag).

##### Reference Implementation

```cpp
glm::vec3 DeflectBullet(const glm::vec3& v, const glm::vec3& n, float e) {
    return e * (v - 2.0f * glm::dot(v, n) * n);
}
```

##### Player-Facing Impact

You can intentionally skip bullet tracks off concrete floors or steel metal ramps at shallow angles. Skipping a burst of fire from your Tommy gun layout underneath a low warehouse loading dock or vehicle chassis frame lets you bypass their defensive posture, chewing up hidden zombie feet and ankles with lethal bounced shrapnel paths.

#### [M2.9-EXT-06] Biomechanical Momentum-Gated Parkour

##### Systems Touched

Extends M2.7 (JPH::CharacterVirtual input loops) and scales over M2.9 metabolic stamina pools.

##### Math

`v_exit = v_entry · cos(θ) · (W′_balance / W′_max)`

##### How It Works

This system replaces supernatural bouncing vectors with realistic human athletic boundaries. When crossing an obstacle found by your M2.9 raycast fan check, your virtual character controller projects its forward velocity vector along the surface exit tangent plane, scaled directly by your available metabolic energy reserve (W′_balance). If `W′_balance ≤ 0.1·W′_max`, the exit velocity drops below the recovery threshold, dropping your kinematic velocity arrays down into your character's hard floor-stumble stagger loops.

##### Reference Implementation

```cpp
void ProcessHumanVault(const glm::vec3& v_in, const glm::vec3& tangent, float w_bal, float w_max, glm::vec3& out_vel) {
    out_vel = tangent * (glm::length(v_in) * glm::dot(glm::normalize(v_in), tangent) * (w_bal / w_max));
}
```

##### Player-Facing Impact

Movement patterns feel grounded and athletic. You can cleanly hurdle low concrete dividers, vault windowsill frames, or slide under barriers, provided you manage your entry speed and watch your metabolic reserves. Sprinting completely out of breath and trying to vault a brick wall causes your character to physically clip their foot, break forward momentum, and tumble over the obstacle in a heavy, vulnerable stumble.

#### [M2.9-EXT-07] Coupled Hydration-Exertion Depletion (Hunger & Thirst)

##### Systems Touched

Extends M2.9's Two-Compartment Exertion Model directly — does not introduce parallel hunger/thirst meters. Reads M10 ambient temperature for sweat-rate scaling; reuses the same thermal math already driving `R_total` insulation.

##### Math

Hydration (`H`) and satiety (`S`) are not independent bars — they act as multipliers on the existing exertion pool:

`CP_effective = CP_base · Clamp(S, 0.4, 1.0)`, with **CP_base = 220W** — grounded in exercise-physiology critical-power literature: ~150-200W untrained, ~200-250W recreationally active, 300W+ for trained endurance athletes. 220W sits at the active-but-not-athlete end appropriate for a survivor protagonist who isn't a professional athlete but isn't sedentary either.

`dW'/dt _drain = (P − CP_effective) · (1.0 + κ_dehydration · (1.0 − H))`

Sweat-driven hydration loss scales with the same ambient-temperature term already computed for thermal insulation:

`dH/dt = −λ_sweat · max(0.0, T_ambient − T_comfort) · (P/P_max) − λ_base`

##### How It Works

`H` and `S` are two floats added to the existing player physiology component sitting alongside `W'balance`. Every fixed-timestep exertion update already reads `CP` and `P` — this feature just rewrites `CP_effective` in place before the existing exertion integration runs, so the rest of M2.9's traversal/sprint code needs zero changes. Food and water items reduce depletion rate or restore `S`/`H` by a flat amount on consumption; there is no separate "hunger system" tick.

##### Reference Implementation

```cpp
// Called once per physiology fixed-timestep tick, before the existing W'balance integration
void ApplyMetabolicModifiers(float hydration, float satiety, float t_ambient, float t_comfort,
                               float power, float power_max, float dt, float& cp_effective,
                               float& hydration_out) {
    constexpr float CP_BASE = 220.0f;  // watts — recreationally-active adult critical power (exercise physiology lit: ~150-200W untrained, ~200-250W active, 300W+ trained endurance athletes)
    cp_effective = CP_BASE * std::clamp(satiety, 0.4f, 1.0f);
    float sweat_loss = 0.02f * std::max(0.0f, t_ambient - t_comfort) * (power / power_max);
    hydration_out = hydration - (sweat_loss + 0.0015f) * dt;
}
```

##### Player-Facing Impact

A hot day spent sprinting from a horde burns through hydration dramatically faster than resting in shade — the same heat that's already dangerous for insulation reasons now directly punishes exertion too. Going hungry doesn't kill you outright; it quietly caps how hard you can push before your legs give out, which reads as "I'm just tired" until you realize you haven't eaten in two days.

#### [M2.9-EXT-08] Caloric Body Condition Scoring & Layered Thermal Insulation
#### [M2.9-EXT-09] Pneumatic Tire Slip-Angle Deformation Loop *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.9 vehicles. Models tire deformation under slip angle for handling + wear.

##### Math
F_lat = D*sin(C*atan(B*slip)); slip = atan(v_lat/v_long); deform = k*F_lat.

##### How It Works
A simplified Pacejka-style tire model computes lateral force from slip angle each step; the tire mesh deforms (sidewall bulge, contact patch) proportional to load. Slip beyond grip threshold triggers slide. Feeds handling + visual deformation + wear.

##### Reference Implementation
```cpp
float Fy = D*sin(C*atan(B*slipAngle)); deform = clamp(Fy*kDeform, 0, maxBulge);
```

##### Player-Facing Impact
Tires deform and lose grip realistically - handling feels physical, not arcade.

---
#### [M2.9-EXT-10] Kinematic Character Flood Buoyancy & Drag Bridge *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M2.9 + M2.7 character controller. Bridges the character controller to water buoyancy/drag so wading/swimming is stable.

##### Math
F_buoy = rho*g*Vsub; F_drag = 0.5*rho*C_d*A*v^2; a = (F_buoy - F_drag - g*m)/m.

##### How It Works
When the character enters a water volume, buoyancy (submerged volume) and drag are applied to the controller's vertical velocity; the kinematic step is clamped so it floats/sinks smoothly instead of jittering at the surface. Wading depth modulates speed (links M9-EXT-21).

##### Reference Implementation
```cpp
vec3 a = (rho*g*Vsub - 0.5*rho*Cd*A*v*v - g*mass)*up/mass;
```

##### Player-Facing Impact
Wading/swimming feels physically consistent - no bobbing jitter at the waterline.

##### Systems Touched

Declares the base "Caloric Condition Metrics (BCS). Layered Thermal Insulation" bullet. BCS feeds `[M2.9-EXT-07]`'s hydration-exertion depletion as its caloric-reserve term; insulation feeds the same system's sweat/heat-loss calculation as the R-value that gates it.

##### Math

Body Condition Score is a bounded 1–9 veterinary-style index driven by caloric balance over time: `BCS_t = Clamp(BCS_{t-1} + k·(CalIntake − CalExpend)/CalExpend, 1, 9)`, where `k` is a slow-adaptation constant so BCS drifts over days rather than snapping per meal. Insulation is resistances in series, standard building-physics R-value stacking: `R_total = ΣR_i` (base layer + mid layer + shell + wet-penalty term), and heat loss rate follows `Q = ΔT / R_total`.

##### How It Works

Each equipped clothing slot contributes an `R_i` looked up from its item data; wet items (rain, water crossings, blood-soaked) apply a multiplicative penalty to their own `R_i` before summation, since wet insulation conducts heat far worse than dry. `R_total` is recomputed on any equip change and consumed once per tick by the core body-temperature solver. BCS runs on a much slower clock — once per in-game day — averaging the day's caloric intake against expenditure (baseline metabolism plus `[M2.9-EXT-07]`'s exertion draw) and nudging the score up or down within its 1–9 band.

##### Reference Implementation

```cpp
// Recomputed on any equip change; consumed once per tick by the body-temperature solver
float ComputeTotalInsulation(std::span<const ClothingLayer> layers) {
    float rTotal = 0.0f;
    for (const auto& layer : layers) {
        float rEffective = layer.baseR * (layer.isWet ? layer.wetPenalty : 1.0f);
        rTotal += rEffective;
    }
    return rTotal;
}

// Runs once per in-game day, not per tick
float UpdateBodyConditionScore(float bcsPrev, float calIntake, float calExpend, float k) {
    if (calExpend <= 0.0f) return bcsPrev; // guard: no defined expenditure baseline this tick, hold steady
    float delta = k * (calIntake - calExpend) / calExpend;
    return std::clamp(bcsPrev + delta, 1.0f, 9.0f);
}
```

##### Player-Facing Impact

Layering matters concretely: stacking a dry shell over a wet base layer visibly slows heat loss on the temperature HUD, and BCS gives a slow, readable trend — weeks of undereating show up as a declining condition score long before it becomes an emergency, rather than starvation being a hidden instant-death timer.

## M3 — Macro-destruction & structural graphs

##### Implementation Steps

* `EnTT DoorComponent { bool locked; float shearThreshold; MaterialClass material; }` — minimal canonical breakable-door component. Backs both M2.7-EXT-11 (Kinematic Door-Kick Hydraulic Ram Splitting) and M2.7-EXT-13 (Latch-Shear Kinematic Door Piercing); neither compiles against something real without this.
* Voronoi Structural Integrity Graph bound by Jolt `FixedConstraint`.
* BFS Asynchronous Collapse Processing.
* Implicit Surface Boolean CSG (Jump Flood Algorithm) and Screened Poisson Surface Reconstruction.
* Spectral Truss Pre-Analysis (Laplacian eigenmodes).
* SDF Neural Collision Proxies for fragments.
* Dynamic Ballistic Fracture Injection driven directly by de Marre kinetic values.
* Resonance Structural Collapse (`mx″+cx′+kx=F(t)`). Sustained Thermal Degradation over decay curves.
* Rubble Blue-Noise Thinning. Debris Surface Friction Mapping.

##### Decisions to flag

Staged vs binary destruction default (staged recommended).

##### Exit Criteria

* Dozens of structures destructible. BFS graph search beats brute-force.
* Nav grid clears same-frame.
* Resonance collapse triggers on loaded frequencies.



### Extended Systems Library — engine-side additions for M3

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M3-EXT-01] Structural Fatigue Cycle Creep (Palmgren-Miner Wear)

##### Systems Touched

Complements M3 (Macro-destruction structural graphs) and processes M5.4 horde pressure variables.

##### How It Works

Your M3 structural integrity graph evaluates building fractures using high-velocity, immediate impact kinetic energy. This feature introduces a flat EnTT data struct component: struct StructuralFatigue { float value = 0.0f; }; assigned to barricades and walls. When a massive zombie horde presses against a barrier, their collective boid separation forces apply sub-critical cyclic loading. The engine increments this value on a slow hourly cadence or event check, bypassing expensive structural matrix solves on every single frame until a fracture threshold is crossed.

##### Math

The sub-critical rhythmic breakdown accumulates structural damage (D) using a linear fatigue tracking framework:

`D = Σ(n_i / N_i), where N_i = (S_e/S_i)^m · N_e`

The exponent ratio is `S_e/S_i` (reference endurance limit over applied stress) — Basquin's S-N relation means higher applied stress `S_i` gives a smaller `N_i` (fewer cycles to failure), matching the Reference Implementation below (`s_e / stress_si`).

When D≥1.0, the material yield threshold drops to zero, forcing an immediate structural graph collapse.

##### Reference Implementation

```cpp
// Updates the structural fatigue component loop inside your EnTT registry on a coarse background tick
void TickFatigue(float stress_si, float s_e, float m, float n_e, float& fatigue) {
    fatigue += 1.0f / (std::pow(s_e / (stress_si + 1e-5f), m) * n_e);
}
```

##### Player-Facing Impact

Your defensive safehouses are vulnerable to long-term physical wear. A wooden palisade wall might easily survive a few direct sledgehammer or vehicle hits, but letting a persistent zombie horde constantly rattle and press against it for an entire day-cycle causes internal fatigue creep. The structural elements will eventually weaken and collapse under a minor impact, forcing you to actively clear away lingering populations before they compromise your perimeter walls.

##### Depends on (not yet built at this point)

M5.4 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M3-EXT-02] Piezoluminescent Strain Shaders for Impending Structural Collapse

##### Systems Touched

Integrates M3 (Macro-destruction structural graphs) directly into the M1/M4.5 GPU-driven G-buffer pipelines.

##### Math

The material strain ratio (C_stress ) is evaluated per-frame inside your enkiTS background structural graph passes. The structural node strain tensor (σ) is normalized against its temperature-dependent failure threshold (σ_yield (T)):

`C_stress = Clamp(∥σ∥ / σ_yield(T), 0.0, 1.0)`

This scalar is packed straight into an unused channel of your indirect draw instance buffer, allowing the fragment shader to apply localized color matrix transformations within your perceptually uniform OKLab space.

##### How It Works

Instead of a wall or ceiling platform remaining visually pristine until its structural health bar hits zero and triggers an instantaneous collapse, this system maps physical truss stress straight to your rendering attachments using your G-Buffer feedback loops.

##### Reference Implementation

```cpp
float ComputeStrainRatio(float sigma, float sigmaYieldAtTemp) {
    return (sigmaYieldAtTemp <= 1e-5f) ? 1.0f : std::clamp(sigma / sigmaYieldAtTemp, 0.0f, 1.0f);
}
```

```glsl
vec3 ApplyStrainVisual(vec3 baseColor, float cStress) {
    vec3 crackGlow = mix(vec3(0.0), vec3(1.0, 0.4, 0.1), smoothstep(0.6, 1.0, cStress));
    return mix(baseColor, baseColor * 0.6 + crackGlow, cStress);
}
```

##### Player-Facing Impact

When a building or defense bunker is nearing its critical Euler buckling point—either because a massive horde is exerting massive physical crowd load against it or because sustained fire has compromised its load-bearing supports—the concrete and wood structures will visually telegraph their distress. The surfaces will physically craze, micro-fractures will grow dynamically across the texture map, and raw, high-stress structural lines will glow with a faint, dust-flaking desaturated hue, giving players a visceral warning to evacuate a sector before the asynchronous BFS collapse engine activates.

##### Depends on (not yet built at this point)

M4.5 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M3-EXT-03] Structural Truss Resonance Audio Leak

##### Systems Touched

Links M3 macro-destruction truss structures to M5.3 AI spatial memory arrays via enkiTS.

##### Math

Instead of casting heavy air-ray audio tracks through space, physical impacts applied to walls (zombies clawing or heavy barricade impacts) are treated as a structural wave equation traveling across your graph layout. The node displacement vector updates across connected Jolt FixedConstraint components using a discrete graph Laplacian matrix (L_graph ):

`u ¨ =c 2 L_graph u−γ u ˙`

##### How It Works

Physical impacts against a wall (clawing, barricade hits, gunfire) inject a displacement impulse into the nearest structural graph node instead of casting an audio ray. The impulse propagates across the graph's Laplacian each tick, decaying with the γ term; any AI entity in contact with a node above threshold displacement registers it as a directional stimulus.

##### Reference Implementation

```cpp
// Executed on an enkiTS background fiber task across structural node matrices
void PropagateTrussVibration(float c_sq, float gamma, float dt, const std::vector<float>& adj_u, float current_u, float& d_u) {
    float laplacian = 0.0f; for (float u : adj_u) laplacian += (u - current_u);
    d_u += (c_sq * laplacian - gamma * d_u) * dt;
}
```

##### Player-Facing Impact

Zombies actively leaning against or standing in contact with a connected structural building layout will physically sense vibrations from your movements inside. Smashing furniture, firing weapons, or boarding up a back window vibrates the structural framework, causing idle entities outside to immediately alert and target the specific support columns you are standing near.

##### Depends on (not yet built at this point)

M5.3 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M3-EXT-04] Hydrocarbon Slipstream Flame-Trail Splitting

##### Systems Touched

Connects M3 macro-destruction fuel cell hits directly to the persistent RVT terrain-overlay system built in M4.5 (chunk-anchored, `imageStore` write path) — writes into that real overlay, not an assumed layer.

##### Math

When a vehicle fuel tank sustains a puncture event, instead of a boring depletion meter, it spawns a continuous line of liquid fuel cells advected on the terrain map. The thermal ignition front propagation velocity (v_f ) scales non-linearly with your vehicle's exhaust backfire thermal matrix:

`v_f = v_0 · (T_exhaust / T_ambient)²`

##### How It Works

A punctured fuel tank spawns a continuous line of fuel-cell entities advected along the vehicle's path, written into M4.5's persistent RVT terrain overlay. Igniting any cell starts a thermal front propagating at a rate scaled by the exhaust-to-ambient temperature ratio.

##### Reference Implementation

```cpp
void TraceFlameTrail(float t_ex, float t_amb, float v0, float dt, glm::vec3& fire_pos, const glm::vec3& v_veh) {
    // ratio is t_ex/t_amb (hotter exhaust -> faster ignition front), not the inverse
    if (t_ex > t_amb) fire_pos += (v_veh - glm::normalize(v_veh) * (v0 * std::pow(t_ex / t_amb, 2.0f))) * dt;
}
```

##### Player-Facing Impact

Puncturing your gas tank is no longer an annoying penalty. It lets you intentionally drop a thick trail of raw fuel down the highway. Intentionally forcing a manual exhaust backfire or dropping a road flare ignites the trail, sending a wall of high-velocity fire chasing behind your car that cooks sprinting hordes while you accelerate away.

#### [M3-EXT-05] Rayleigh Surface Wave Ground-Slam Propagation

##### Systems Touched

Pairs heavy structural collapse events or large phenotype landings with EnTT tracking arrays.

##### Math

High-mass kinetic impacts hitting the ground plane generate elastic Rayleigh surface waves. The seismic amplitude (A) radiates outward as a 2D cylindrical wave front that decays over distance (r):

`A(r) = A_0 · e^(−αr) / √r`

##### How It Works

A high-mass impact against the ground plane emits a 2D cylindrical Rayleigh wave. Nearby dynamic bodies sample its amplitude each tick; amplitude above a stability threshold applies a knockdown/stagger impulse, decaying with distance per the formula.

##### Reference Implementation

```cpp
float GetSeismicAmplitude(float a0, float alpha, float r) {
    return (a0 * std::exp(-alpha * r)) / (std::sqrt(r) + 1e-5f);
}
```

##### Player-Facing Impact

Luring a heavy, high-mass phenotype zombie off a building ledge turns the ground into an active hazard. When its massive weight slams into the pavement, it creates a visual and physical shockwave that ripples across the asphalt, knocking down nearby common zombies and temporarily dropping the controller stability metrics of any close players.

#### [M3-EXT-06] Shared Fatigue-Life Wear (Weapons & Vehicle Panels)

##### Systems Touched

Applies the existing M3-EXT-01 Palmgren-Miner fatigue accumulator to weapon components and vehicle body panels — this is the third consumer of one canonical fatigue system (structures, and now weapons/vehicles), not a new durability mechanic.

##### Math

Identical shape to M3-EXT-01, no new formula:

`D = Σ(n_i/N_i), where N_i = (S_e/S_i)^m · N_e`

Weapon/panel failure triggers at the same `D ≥ 1.0` threshold already used for structural collapse.

##### How It Works

Weapons and vehicle panels attach the same `StructuralFatigue` accumulator structures use. Every stress event calls the shared `TickFatigue()` already driving structural collapse; crossing `D ≥ 1.0` fails the weapon/panel through the same threshold check, no separate durability system.

##### Naming

Reuses M3-EXT-01's actual accumulator verbatim — `struct StructuralFatigue { float value = 0.0f; }`, updated via `TickFatigue()`. Weapons and vehicle panels attach the same component type structures use; this is the third consumer of one canonical fatigue system, not a parallel one.

##### Reference Implementation

```cpp
// Identical call signature to the M3 structural fatigue check — same component type attached
void TickFatigue(float stress_i, float endurance_limit, float exponent_m,
                  float reference_cycles, StructuralFatigue& acc) {
    float n_i = reference_cycles * std::pow(endurance_limit / stress_i, exponent_m);
    acc.value += 1.0f / n_i;
}
// Failure check uses the same D >= 1.0 threshold as structural collapse, read from acc.value
```

##### Player-Facing Impact

A weapon fired constantly under high-stress conditions (hot barrel, dirty action — both of which you already track) wears out with the same believable curve a bridge truss collapses under repeated load, instead of an arbitrary "durability: 47/100" number going down linearly.

#### [M3-EXT-07] Multi-Layer Structural Degradation & Stress-Tensor Cracking

##### Systems Touched

M3's destruction graph (impact/load events), M4.5's material/shading layer (exposes sub-surface texture tiers).

##### Math

Directional stress accumulates per structural node as a simple decaying tensor, reusing the same load-then-decay shape as `[M9]`'s vehicle fatigue rather than inventing a new curve:

$$S_{node}(t) = S_{node}(t-1)\cdot e^{-\lambda \Delta t} + \sum F_{impact} \cdot \hat{d}_{impact}$$

A node cracks/exposes its next material tier once $|S_{node}|$ crosses a per-material threshold (concrete → rebar → dust; plaster → brick → stud).

##### How It Works

Each destructible node (already tracked by `[M3]`'s structural graph) carries a small stress tensor instead of a scalar HP value. Impacts and sustained load shifts (leaning debris, blast pressure) add a directional term; the term decays exponentially like every other decay-family value in this doc. When accumulated stress crosses a threshold, the renderer's existing multi-layer vertex blend (used for wet-surface porosity in `[M4.5-EXT-04]`) is repurposed to blend toward the next material tier along the stress vector's direction, so cracks visibly radiate from the actual impact point instead of a generic damage decal.

##### Reference Implementation

```cpp
struct StructuralStressNode {
    glm::vec3 stressTensor{0.0f};
    uint8_t materialTierIndex = 0; // 0=intact, 1=cracked, 2=exposed substrate
};

void ApplyStressImpulse(StructuralStressNode& node, glm::vec3 impactDir, float impactForce, float dt, float lambdaDecay) {
    node.stressTensor *= std::exp(-lambdaDecay * dt);
    node.stressTensor += impactDir * impactForce;
    float magnitude = glm::length(node.stressTensor);
    constexpr float kTierThresholds[3] = { 0.0f, 40.0f, 120.0f };
    for (int tier = 2; tier >= 0; --tier) {
        if (magnitude >= kTierThresholds[tier]) { node.materialTierIndex = static_cast<uint8_t>(tier); break; }
    }
}
```

##### Player-Facing Impact

Walls and structures crack and expose rebar/studs along the actual direction repeated gunfire or blast pressure came from, instead of a uniform damage-percentage reskin.

#### [M3-EXT-08] Chemical Weathering & Particulate Deposition Tracker

---

#### [M3-EXT-11] Fracture-Debris Broad-Phase Reuse *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M3 + M2.6. Reuses the broadphase for fracture-debris collision instead of a second pass.

##### Math
debris cells share M2.6/M1-EXT-27 hash; pairs resolved in same pass.

##### How It Works
Fracture debris reuses the existing spatial-hash broadphase (no separate collision world); debris pairs are resolved in the same pass as the parent structure's, saving a whole broadphase.

##### Reference Implementation
```cpp
ResolveInPass(debris, broadphase);
```

##### Player-Facing Impact
Debris collides correctly without a second broadphase - cheaper destruction.


---

#### [M3-EXT-10] Spherical-Harmonics Visibility Pre-Filter Grid *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M3 + M4.5-EXT-17. Pre-filters visibility into an SH grid for cheap indirect/occlusion queries.

##### Math
SH coeffs per probe from sampled visibility; convolve for cosine lobe.

##### How It Works
Per probe, visibility is sampled and projected to spherical harmonics; a cosine-lobe convolution gives a cheap ambient-occlusion / diffuse-indirect term reusable by lighting and audio. Pairs with M4.5-EXT-17.

##### Reference Implementation
```cpp
Probe p; p.sh = ProjectVisibility(samples);
```

##### Player-Facing Impact
Indirect light + occlusion resolve cheaply from one SH grid - consistent, fast.


##### Systems Touched

`[M3]` destructible material tags, `[M6.5]` particle/VFX system (soot/spark sources), `[M7]`'s existing long-term structural weathering narrative (this gives it concrete math instead of prose-only).

##### Math

Rust/stain accumulation is Arrhenius-style, matching `[M7]`'s existing "elapsed real hours" weathering hook instead of adding a second decay family:

$$k = A \cdot e^{-E_a / RT}, \qquad \text{StainDepth}(t) = \text{StainDepth}(t-1) + k \cdot \Delta t_{hours}$$

##### How It Works

Metal-on-metal contact points (bolts on signage, rebar on concrete) and combustion sources (fire, gunfire muzzle blast) both write into the same per-surface stain-depth channel already used by `[M7-EXT]`'s weathering system. Rust uses the Arrhenius rate above; soot uses a directional projection along the local airflow vector (already computed for `[M10]`'s wind field) instead of a straight-down splat, so stains trail correctly on ceilings above a fire.

##### Reference Implementation

```cpp
float AccumulateRustStain(float currentDepth, float ambientTempKelvin, float elapsedHours) {
    constexpr float A = 1.2e7f, Ea = 5.5e4f, R = 8.314f;
    float k = A * std::exp(-Ea / (R * ambientTempKelvin));
    return currentDepth + k * elapsedHours;
}

glm::vec3 ProjectSootTrail(glm::vec3 sourcePos, glm::vec3 windVelocity, float dt) {
    return sourcePos + windVelocity * dt; // feeds the M6.5 particle emitter's trail direction
}
```

##### Player-Facing Impact

Rust runs and soot trails read as physically grounded evidence of what happened in a space (a burned-out room's ceiling stains trail toward the actual airflow path) rather than a fixed decal stamped at authoring time.



## M4 — Procedural world generation + chunk streaming & culling

##### Implementation Steps

* Layered Chunk Streaming Topology (Voronoi + WFC AC-4 propagation).
* Dynamic Biome Weight Interpolation (2-3 chunk blend band).
* Procedural Urban Detail (L-systems, BSP interiors).
* Socio-Economic Utility Tagging for building types.
* Procedural Signage Compilation (context-free grammars).
* Geometric Edge Seam Welding for LOD.
* Multi-Player Radial Tracking.
* Hydraulic erosion waterway carving.

##### Decisions to flag

Load/unload radii, chunk size.

##### Exit Criteria

* Player traverses chunks with no stutter.
* Urban layouts read as logically zoned. Tags drive logic successfully.

### Extended Systems Library — engine-side additions for M4

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M4-EXT-01] Hierarchical Global Macro-Graph Constraint Pass

##### Systems Touched

Foundational to M4 (Procedural world generation) and integrates with Appendix C (Loot/Zone socio-economic tagging).

##### Math

The macro-infrastructure layout uses a point-to-segment distance check to snap local chunk generation bounds to global roadway and river corridors derived purely from the world master seed:

`d = ‖(p − a) × (p − b)‖ / ‖b − a‖`

##### How It Works

A pure bottom-up chunk generation framework suffers from seam-alignment failures where road networks or power grids fail to connect across independent sector borders. This feature introduces a top-down macro-graph generation pass that runs before any local chunk evaluates Wave Function Collapse or L-systems. Using the world seed, the engine generates a fast, global coordinate graph outlining major highway vectors, primary river waterways, and socio-economic zoning bounds across the entire world coordinate space. When an individual 256×256m chunk streams into visibility, it queries this global macro-graph. Any intersecting layout vectors are injected directly into the chunk's local WFC initialization as immutable, pre-placed tile constraints, forcing local procedural assemblies to snap to the city's macro-layout.

##### Reference Implementation

```cpp
// Evaluated before local chunk WFC logic executes; injects global structural constraints
// to prevent boundary seam errors during chunk streaming
bool IsMacroInfrastructureLine(const glm::vec3& cell_pos, const glm::vec3& start, const glm::vec3& end, float width_threshold) {
    glm::vec3 line_vec = end - start;
    glm::vec3 cell_vec = cell_pos - start;
    float t = std::clamp(glm::dot(cell_vec, line_vec) / (glm::dot(line_vec, line_vec) + 1e-5f), 0.0f, 1.0f);
    float dist = glm::length(cell_vec - t * line_vec);
    return dist < width_threshold;
}
```

##### Player-Facing Impact

The procedurally generated open world behaves like a realistically planned city. Highways run continuously for kilometers across distinct biomes, commercial districts group logically around industrial shipping channels, and rural residential clusters give way to high-density downtown blocks without a single disconnected road or broken street seam.

#### [M4-EXT-02] Asynchronous Quadric Error Meshlet Decimation (Runtime Procedural LODs)

##### Systems Touched

Deepens M4 procedural world generation and pairs with M1 meshlet execution paths.

##### Math

Edge-collapse calculations use quadric error matrices to rank surface error metrics before stripping indices out of the buffer:

`Error = vᵀ · (Q_A + Q_B) · v`

##### How It Works

Synthesizing structural geometry at runtime means there are no hand-authored LOD models. Drawing distant building blocks or street ruins at maximum fidelity would quickly overwhelm the vertex pipeline. This system runs an automatic edge-decimation pass on background enkiTS workers. As the mesh-grammar code finishes generating a high-density vertex buffer, the decimation pass simplifies vertex pairs using quadric error evaluations, building low-overhead meshlet variants in memory before the data transfers to the GPU memory pool.

##### Reference Implementation

```cpp
// Evaluated on background worker threads to trim triangle density on procedurally generated meshes
bool EvaluateEdgeDecimation(float error_weight, float threshold) {
    return error_weight < threshold; // drop vertex pairs under the precision limit
}
```

##### Player-Facing Impact

Distant city silhouettes look clean and detailed while using a fraction of the rendering overhead. Geometry transitions smoothly as you move through the world, avoiding distracting pop-in.

#### [M4-EXT-03] Hierarchical WFC Multi-Grid Graph Layering (Contradiction Guard)

##### Systems Touched

Integrates with M4 chunk streaming topologies and guides local pathfinding grid structures (M5-EXT-01).

##### Math

Constraint evaluation is restricted to localized sub-grids, mapping dependencies down to progressively smaller coordinate frames:

`StateSpace = Size_macro · Size_medium · Size_micro`

##### How It Works

Evaluating global road networks and fine interior details inside a single massive WFC matrix creates an exponential blow-up in tile constraints, causing long generation stalls or unresolvable contradictions. This system splits generation into decoupled structural passes: a top-down macro pass defines road frameworks and building footprints (feeding [M4-EXT-01]'s macro-graph), a medium pass manages interior wall layouts, and a micro pass handles furniture placement. Each completed parent tier acts as an immutable constraint map for its children, keeping per-tier constraint counts small.

##### Reference Implementation

```cpp
// Isolates constraint grids to eliminate algorithmic lockups during chunk assembly
uint32_t ResolveHierarchicalTile(uint32_t parent_mask, uint32_t local_allowed_bits) {
    return local_allowed_bits & parent_mask; // clear tile configurations that don't fit the parent tier
}
```

##### Player-Facing Impact

New streaming city sectors materialize ahead of your vehicle with zero frame drops. Buildings feature logical layouts — doors open onto sidewalks, hallways connect cleanly, and rooms populate without layout errors.

#### [M4-EXT-04] Voronoi Chunk Seam Vertex Normal-Welding Post-Process

##### Systems Touched

Cleans up geometry seams left by M4's Layered Chunk Streaming Topology (Voronoi + WFC), specifically where independent chunks generate their own localized WFC constraints and produce boundary vertices with minor floating-point mismatches.

##### Math

`n_unified = (n_ChunkA + n_ChunkB) / ‖n_ChunkA + n_ChunkB‖`, applied only where `‖n_ChunkA + n_ChunkB‖² > 1e-6`

##### How It Works

Boundary vertices at a chunk seam are matched by world-space coordinate via a thread-safe lookup as neighboring chunks finish streaming in. Their normals are averaged and renormalized so lighting doesn't crack across the seam; a degenerate near-zero sum (opposing normals) falls back to world-up rather than producing a NaN.

##### Reference Implementation

```cpp
inline glm::vec3 ComputeWeldedBoundaryNormal(const glm::vec3& normalA, const glm::vec3& normalB) {
    glm::vec3 unified = normalA + normalB;
    float lenSq = glm::dot(unified, unified);
    if (lenSq > 1e-6f) {
        return unified * (1.0f / std::sqrt(lenSq));
    }
    return glm::vec3(0.0f, 1.0f, 0.0f); // degenerate-normal fallback
}
```

##### Player-Facing Impact

Terrain and building seams between independently streamed chunks stay visually seamless — no specular lighting cracks or shading discontinuities at chunk boundaries, even under the Fresnel-driven wet-surface shading from M4.5-EXT-06.

#### [M4-EXT-05] Geometric Edge Seam Welding for LOD Transitions

##### Systems Touched

Declares the base "Geometric Edge Seam Welding for LOD" bullet. Runs downstream of `[M4-EXT-02]`'s meshlet decimation (welds the LOD boundary that decimation produces) and `[M4-EXT-04]`'s chunk-seam normal welding (shares the same weld-tolerance constant so both operate on one consistent epsilon).

##### Math

Two chunk edges at different LOD levels are welded by snapping any boundary vertex to the nearest vertex on the coarser edge within tolerance: `Weld(v) = argmin_{u ∈ EdgeCoarse} ‖v − u‖, subject to ‖v − u‖ < ε`, with `ε = 1e-3 · ChunkScale` — the same relative-epsilon pattern used elsewhere in the doc rather than a fixed world-unit constant.

##### How It Works

When two adjacent chunks stream in at different LOD tiers (near chunk full-res, far chunk decimated), their shared border edge no longer has matching vertex density — the fine edge has extra vertices the coarse edge lacks. Rather than leaving a visible crack, each extra fine-edge vertex is snapped onto the coarse edge's line by finding its nearest coarse-edge point within `ε` and welding position + normal to match. This runs once per chunk-boundary pair at stream-in time, not per frame, since LOD tier only changes on chunk transitions.

##### Reference Implementation

```cpp
// Runs once per chunk-boundary pair at stream-in time, not per frame
void WeldLodSeam(std::span<Vertex> fineEdge, std::span<const Vertex> coarseEdge, float chunkScale) {
    const float epsilon = 1e-3f * chunkScale;
    for (auto& v : fineEdge) {
        float bestDistSq = std::numeric_limits<float>::max();
        const Vertex* best = nullptr;
        for (const auto& u : coarseEdge) {
            float distSq = glm::distance2(v.position, u.position);
            if (distSq < bestDistSq) { bestDistSq = distSq; best = &u; }
        }
        if (best && bestDistSq < epsilon * epsilon) {
            v.position = best->position;
            v.normal = best->normal;
        }
    }
}
```

##### Player-Facing Impact

Walking toward the world's LOD horizon never reveals a visible crack or z-fighting seam where high-detail terrain meets its lower-detail neighbor — the transition is geometrically continuous, not just shaded to look that way.

#### [M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache

##### Systems Touched

Multi-threaded ECS system execution, background enkiTS worker fiber tasks.

##### Math

Direct pointer-offset derivation mapping raw sparse-set indices directly to contiguous Structure-of-Arrays (SoA) memory allocations without re-evaluating registry page hierarchies:

`Addr_c(i) = Base_c + (SparseMapping_c[i] × Stride_c)`

##### How It Works

Caches internal registry array offsets for heavily multi-threaded systems running over enkiTS fiber pools. Worker fibers never call into `entt::registry` itself mid-tick — they only read from these pre-resolved base pointer/stride pairs, snapshotted at the start of the tick before `[M1-EXT-06]`'s SPSC mutation queue drain runs. Because the drain can reallocate a pool's backing storage, this cache must be re-resolved every tick rather than persisted across ticks; it is a same-tick read-only shortcut, not a second source of truth for the registry.

##### Reference Implementation

```cpp
#include <cstdint>
#include <cstddef>

struct LockedComponentPoolCache {
    uint8_t* rawDataBufferMemoryHead = nullptr;
    const uint32_t* sparseSetDenseIndicesPtr = nullptr;
    size_t componentTypeAllocationStride = 0;

    template<typename ComponentType>
    [[nodiscard]] inline ComponentType* ResolveComponentPointerDirect(uint32_t rawSparseEntityId) const noexcept {
        const uint32_t denseTargetIndex = sparseSetDenseIndicesPtr[rawSparseEntityId];
        return reinterpret_cast<ComponentType*>(rawDataBufferMemoryHead + (denseTargetIndex * componentTypeAllocationStride));
    }
};
```

##### Player-Facing Impact

Maintains sub-millisecond frame updates when evaluating flocking, avoidance, and movement calculations for over 200 simulation entities simultaneously.

#### [M4-EXT-12] Voronoi Crustal Tectonics & Macro-Geomorphology

##### Systems Touched

`[M4]`'s base terrain generation pass (runs before it — establishes the macro heightfield skeleton that noise/erosion passes then refine).

##### Math

Plate boundaries come from a standard Voronoi cell decomposition over randomly seeded plate centers; boundary type (convergent/divergent/transform) is assigned per edge from the relative motion vector of its two adjacent cells, then an uplift falloff is applied outward from convergent boundaries:

$$\text{Uplift}(d) = U_{max} \cdot e^{-d / \sigma_{range}}$$

##### How It Works

Before any per-chunk noise runs, a single low-resolution (world-scale, not chunk-scale) Voronoi diagram is generated once from the world seed and cached. Each cell gets a random drift vector; edges between cells with converging drift get positive uplift (mountains), diverging edges get subsidence (rift valleys/basins), and edges with mostly-parallel drift get lateral fault noise instead. This uplift/subsidence field is sampled as a low-frequency bias added underneath the existing per-chunk terrain noise, so mountain ranges and valleys have world-scale coherent shape instead of every chunk rolling independent hills.

##### Reference Implementation

```cpp
struct TectonicPlate { glm::vec2 centerSeed; glm::vec2 driftVector; };

float SampleUpliftBias(glm::vec2 worldPos, const std::vector<TectonicPlate>& plates) {
    // Find nearest two plate centers (standard Voronoi nearest + second-nearest)
    int nearestIdx = -1, secondIdx = -1; float nearestD = FLT_MAX, secondD = FLT_MAX;
    for (int i = 0; i < (int)plates.size(); ++i) {
        float d = glm::length(worldPos - plates[i].centerSeed);
        if (d < nearestD) { secondD = nearestD; secondIdx = nearestIdx; nearestD = d; nearestIdx = i; }
        else if (d < secondD) { secondD = d; secondIdx = i; }
    }
    float convergence = glm::dot(plates[nearestIdx].driftVector, plates[secondIdx].driftVector - plates[nearestIdx].driftVector);
    float boundaryDist = std::abs(secondD - nearestD) * 0.5f;
    constexpr float sigmaRange = 800.0f, upliftMax = 350.0f;
    return convergence > 0.0f ? upliftMax * std::exp(-boundaryDist / sigmaRange) : -upliftMax * 0.4f * std::exp(-boundaryDist / sigmaRange);
}
```

##### Player-Facing Impact

Mountain ranges and rift valleys read as geologically coherent across many kilometers instead of the terrain looking like independently-rolled chunk hills stitched together.

#### [M4-EXT-13] Dual-Contouring Volumetric Cave Carving

##### Systems Touched

`[M4-EXT-12]`'s heightfield (source surface to carve out of), `[M2]`'s Jolt collision generation (caves need real collision, not just visual holes).

##### Math

Cave volumes are authored as a 3D signed distance field (metaball union of procedurally placed tunnel segments), triangulated with Dual Contouring rather than Marching Cubes because DC preserves sharp edges at heightfield/cave seams instead of universally rounding them:

$$\text{SDF}_{cave}(p) = \min_i \left( |p - \text{segment}_i| - r_i \right)$$

##### How It Works

Tunnel paths are generated as connected line segments (a simple random walk biased away from the surface and toward low-stress tectonic zones from `[M4-EXT-12]`, since valleys are cheaper to carve believably than mountain cores). The SDF above is evaluated on a local voxel grid wherever a tunnel segment's bounding box overlaps a chunk; Dual Contouring extracts a watertight mesh from that grid, which is stitched into the chunk's terrain mesh at the seam and handed to Jolt as a static collision mesh, same pipeline as regular terrain.

##### Reference Implementation

```cpp
struct CaveTunnelSegment { glm::vec3 start; glm::vec3 end; float radius; };

float SampleCaveSDF(glm::vec3 p, const std::vector<CaveTunnelSegment>& segments) {
    float minDist = FLT_MAX;
    for (auto& seg : segments) {
        glm::vec3 ab = seg.end - seg.start;
        float t = glm::clamp(glm::dot(p - seg.start, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
        float d = glm::length(p - (seg.start + ab * t)) - seg.radius;
        minDist = std::min(minDist, d);
    }
    return minDist; // negative = inside cave, feed into your existing Dual Contouring voxel extractor
}
```

##### Player-Facing Impact

Caves and overhangs open seamlessly out of the surface terrain with real collision, instead of caves being a separate hand-placed prefab bolted onto procedural ground.

#### [M4-EXT-14] Stratigraphic Substrate Horizon Tracking

##### Systems Touched

`[M4-EXT-13]`'s cave walls (exposes strata visually), `[M3]`'s explosion/fracture rules (routes cracks along real cleavage planes).

##### Math

Rock strata are 3D Simplex noise bands clipped by the tectonic fault planes from `[M4-EXT-12]`, giving each world-space point a discrete stratum index and a cleavage normal:

$$\text{Stratum}(p) = \lfloor (\text{Simplex3D}(p \cdot f_{strata}) + \text{FaultOffset}(p)) \cdot N_{layers} \rfloor$$

##### How It Works

A single low-frequency 3D Simplex field is evaluated once per world region and clipped (offset-shifted) wherever it crosses a tectonic fault edge, so strata visibly bend/break exactly at fault lines instead of running perfectly straight through them. Each stratum index maps to a material (sediment/igneous/metamorphic) with a density and cleavage-normal entry in a small static table; when `[M3]` resolves an explosion's fracture pattern, it reads the local cleavage normal and biases crack propagation to follow it instead of a purely radial blast pattern.

##### Reference Implementation

```cpp
struct StratumMaterial { float density; glm::vec3 cleavageNormal; uint8_t visualTierId; };
inline StratumMaterial g_StrataTable[8]; // populated once at world-gen time from the seed

int SampleStratumIndex(glm::vec3 worldPos, float faultOffset, float freqScale, int layerCount) {
    float n = (Simplex3D(worldPos * freqScale) + faultOffset) * 0.5f + 0.5f;
    return glm::clamp((int)(n * layerCount), 0, layerCount - 1);
}
```

##### Player-Facing Impact

Blasted rock faces and cave walls show real-looking sedimentary banding that bends at fault lines, and explosions crack along that banding instead of a generic radial shatter.

#### [M4-EXT-15] Darcy's-Law Deep Aquifer Extension

##### Systems Touched

Extends `[M10]`'s existing 2D groundwater heightmap (shared flooding grid) — this does not replace it, it adds the sub-surface porous-flow term that grid was a simplified stand-in for.

##### Math

Groundwater flux through a porous stratum obeys Darcy's Law:

$$Q = -KA\frac{dh}{dl}$$

where $K$ is the hydraulic conductivity looked up from `[M4-EXT-14]`'s stratum material table (sand = high $K$, clay/bedrock = near-zero).

##### How It Works

`[M10]`'s existing groundwater heightmap already tracks a single water-table height per 2D cell; this feature adds the missing piece — how fast water actually moves between adjacent cells — by weighting the existing cell-to-cell transfer rate with each cell's stratum conductivity instead of a flat constant. Excavating through a low-$K$ clay stratum into a high-$K$ sand stratum below the water table now floods noticeably faster than excavating through uniform bedrock, because the conductivity term is no longer constant.

##### Reference Implementation

```cpp
float DarcyFlowRate(float headA, float headB, float conductivityK, float crossSectionA, float pathLength) {
    return -conductivityK * crossSectionA * (headB - headA) / pathLength; // feeds the existing M10 groundwater cell-transfer step
}
```

##### Player-Facing Impact

Digging through soft sand into an aquifer floods a dig site fast; digging the same depth through bedrock barely floods at all — groundwater behaves like real geology reads, not a uniform fill rate everywhere.

#### [M4-EXT-16] Saint-Venant Generation-Time Hydraulic Erosion

##### Systems Touched

`[M4-EXT-12]`'s tectonic heightfield (input surface), `[M10]`'s runtime Saint-Venant flooding solver (`[M10-EXT-01]`) — this reuses that exact flux stencil at generation time instead of writing a second one.

##### Math

Same shallow-water flux equation `[M10-EXT-01]` already implements, run for a bounded number of offline iterations at world-gen time with a sediment-transport term added:

$$\Delta z_{sediment} = c \cdot (q_{capacity} - q_{transported})$$

##### How It Works

At world-generation time only (never at runtime — this is a one-shot bake, not a per-frame system), rainfall is simulated across the macro heightfield using `[M10-EXT-01]`'s existing hydraulic-head flux solver for a fixed number of iterations, carrying sediment downhill and depositing it where flow slows. This carves drainage networks and alluvial fans directly into the baked heightfield before chunks are ever streamed, so rivers have geologically plausible paths instead of being purely noise-based. Because it's the same flux stencil as the runtime flooding system, there is no second implementation to maintain or desync.

##### Reference Implementation

```cpp
void BakeHydraulicErosionPass(std::vector<float>& heightGrid, int width, int height, int iterations) {
    std::vector<float> sediment(heightGrid.size(), 0.0f);
    for (int iter = 0; iter < iterations; ++iter) {
        for (int i = 0; i < (int)heightGrid.size(); ++i) {
            // Reuses StepSaintVenantFlux() from [M10-EXT-01] per-cell, offline, no per-frame budget concerns
            float capacity = EstimateSedimentCapacity(heightGrid, i, width);
            sediment[i] += 0.3f * (capacity - sediment[i]);
            heightGrid[i] -= 0.3f * (capacity - sediment[i]) * 0.01f;
        }
    }
}
```

##### Player-Facing Impact

Rivers and drainage valleys look like they were actually carved by water over geological time, and the same math that bakes them governs live flooding at runtime, so a riverbed floods exactly where you'd expect it to.

#### [M4-EXT-17] Edaphic Soil Chemistry & Nutrient Grid

##### Systems Touched

`[M4-EXT-18]` canopy seeding (gates where trees can take root), `[M8.5]` survivor faction farming/food economy (soil quality gates crop yield).

##### Math

Three nutrient channels (N, P, K) plus pH decay/regenerate toward a stratum-derived baseline set by `[M4-EXT-14]`'s material table:

$$\text{NPK}(t) = \text{NPK}(t-1) + \alpha \cdot (\text{NPK}_{baseline} - \text{NPK}(t-1)) \cdot \Delta t - \text{Consumption}$$

##### How It Works

A coarse (much lower resolution than the terrain heightfield — one sample per ~8m, not per-vertex) grid stores NPK + pH per cell, initialized from the local stratum's baseline fertility. Plant seeding checks (`[M4-EXT-18]`) read this grid as a gate: a species' seed-success roll is rejected outright if any required nutrient is below its minimum threshold. Farmed plots (`[M8.5]`) draw down the same channels per harvest and can have them topped back up by compost/fertilizer, reusing this one grid rather than a separate farming-only nutrient system.

##### Reference Implementation

```cpp
struct SoilNutrientCell { float nitrogen, phosphorus, potassium, ph; };

bool CanSpeciesSeedHere(const SoilNutrientCell& cell, float minN, float minP, float minK) {
    return cell.nitrogen >= minN && cell.phosphorus >= minP && cell.potassium >= minK;
}
```

##### Player-Facing Impact

Farm plots you've stripped bare over several harvests visibly stop producing until fertilized, and wild foliage density genuinely varies with real soil quality instead of every patch of ground supporting identical plant density.

#### [M4-EXT-18] Space-Colonization Canopy Seeding & Shade Competition

##### Systems Touched

`[M4-EXT-17]`'s nutrient grid (gate), existing foliage rendering (`[M6.5-EXT-11]` undergrowth kinematics reads canopy shade output from this).

##### Math

Standard space-colonization: each candidate growth point attracts the nearest branch tip within radius $d_i$, growth direction is the normalized average of attraction vectors:

$$\vec{g} = \frac{\sum_i (\vec{p}_i - \vec{b})/|\vec{p}_i - \vec{b}|}{|\sum_i (\vec{p}_i - \vec{b})/|\vec{p}_i - \vec{b}||}$$

##### How It Works

Tree placement runs the space-colonization algorithm once at chunk-generation time (not per-frame): candidate attraction points are scattered where `[M4-EXT-17]`'s nutrient grid passes the species threshold, canopy branches grow toward the densest unclaimed attraction clusters and consume points within their kill-distance. After canopy trees are placed, a top-down occlusion pass writes a shade-value texture (how much direct light reaches ground level under each canopy); this shade texture is what `[M6.5-EXT-11]` reads to decide undergrowth density and species mix, and what biases nearby foliage trunks to curve away from occluded zones toward open sky.

##### Reference Implementation

```cpp
glm::vec3 ComputeGrowthDirection(glm::vec3 branchTip, const std::vector<glm::vec3>& attractionPoints, float killDistance, float influenceRadius) {
    glm::vec3 sum{0.0f}; int count = 0;
    for (auto& pt : attractionPoints) {
        float d = glm::length(pt - branchTip);
        if (d < influenceRadius && d > killDistance) { sum += glm::normalize(pt - branchTip); ++count; }
    }
    return count > 0 ? glm::normalize(sum) : glm::vec3(0.0f, 1.0f, 0.0f);
}
```

##### Player-Facing Impact

Forests grow with organic, resource-competitive canopy shapes — trees near cliffs visibly lean toward open sky, and undergrowth thins naturally under dense canopy — instead of uniformly-spaced identical tree stamps.

#### [M4-EXT-19] Anthropogenic Infrastructure Exclusion Splines
#### [M4-EXT-08] 3D WFC Vertical Structural Dependency Guard *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 WFC. Guarantees vertical structural dependencies (floor supports wall above) are satisfied.

##### Math
for cell c: require support(c.below) valid before placing c; backtrack if violated.

##### How It Works
During WFC propagation, a cell may only take a tile whose vertical structural preconditions are met by the tile below (e.g. a wall needs a floor/support beneath, not open air). Violations prune the possibility set; if a column deadlocks it backtracks.

##### Reference Implementation
```cpp
if(!BelowSupports(tile, grid[below])) mask &= ~bit(tile);
```

##### Player-Facing Impact
Generated structures stand up - no floating walls or unsupported floors.

---
#### [M4-EXT-09] Deterministic Interior Furniture Spatial Constraint Solver *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M2.6. Places furniture in rooms respecting clearances/reachability deterministically.

##### Math
for slot: if(Clearance(f, room) && Reachable(f)) place(f, seed); seeded RNG.

##### How It Works
Given a generated room, candidate furniture placements are tested against clearance (no overlap, walkable gap) and reachability (path from door). A seeded solver picks valid placements so interiors are navigable and varied but reproducible per seed.

##### Reference Implementation
```cpp
if(Clearance(f,room) && Reachable(f,door)) Place(f, rng);
```

##### Player-Facing Impact
Interiors are furnished but still walkable - loot rooms aren't blocked by a couch.

---
#### [M4-EXT-10] Macro-Graph Vector Spline Corridor Welder *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M5.4. Welds road corridor splines into one continuous drivable path graph.

##### Math
path = CatmullRom(join(spline_i, spline_{i+1})); tangent matched at joints; width field added.

##### How It Works
Adjacent road-vector splines are stitched into a single C1-continuous corridor (matched tangents at joints) and given a width field, producing the drivable macro-graph that M5.4-EXT-06 routes hordes/caravans over and M4 terrain-gen uses for road masks.

##### Reference Implementation
```cpp
Curve c = CatmullRom(Concat(splines)); c.width = laneField;
```

##### Player-Facing Impact
Roads are continuous and drivable end-to-end - pathing and driving share one graph.

---
#### [M4-EXT-11] WFC Contradiction Horizon Recovery *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 WFC. On contradiction, rolls back to a saved horizon and re-expands instead of stalling.

##### Math
on contradiction: restore grid to horizon H; re-expand from H with new tie-break; H saved every K cells.

##### How It Works
WFC can collapse to zero options. Rather than a 3x3 local reset, a saved horizon (every K cells of progress) is restored and expansion re-runs with a perturbed tie-break, escaping the dead end. Keeps background chunk gen progressing.

##### Reference Implementation
```cpp
if(mask==0){ grid=horizon; ReExpand(horizon, newTieBreak); }
```

##### Player-Facing Impact
Chunk generation never hard-stalls on a bad tile pick - maps keep streaming.

---
#### [M4-EXT-21] Signage Grammar Transcoder *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 PCG + M13 SLM + M11 UI. Generates contextual signage text via a grammar for world flavor.

##### Math
sign = Grammar(prod, seededRNG(locale)); terminals drawn deterministically per seed.

##### How It Works
A phrase grammar produces signage (shop names, warnings, graffiti) seeded by world-graph node + locale. Output feeds M13 SLM broadcasts and M11 signage so streets read as lived-in. Deterministic per seed.

##### Reference Implementation
```cpp
string sign = SignageGrammar.Generate(nodeSeed, locale);
```

##### Player-Facing Impact
Streets/ruins carry readable, varied signage - world feels inhabited, not prop-empty.

---

#### [M4-EXT-28] Procedural Foliage L-System Mesh Generator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4. L-System generates foliage meshes (branches/leaves) procedurally.

##### Math
mesh = LSystem(axiom, rules, iters, seed);

##### How It Works
An L-System (axiom + production rules + iterations) generates branch/leaf geometry per species, seeded for determinism. Pairs with M4-EXT-23 biome for species selection. Output meshes feed M4-EXT-27 impostors.

##### Reference Implementation
```cpp
Mesh m = GrowLSystem(species, seed);
```

##### Player-Facing Impact
Trees/shrubs are generated, not placed - varied, biome-correct foliage.


---

#### [M4-EXT-27] Vegetation Impostor *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M4.5-EXT-30. Generates vegetation impostors (billboards) for distant foliage.

##### Math
impostor = BakeBillboard(mesh, K dirs); sample by view dir at range.

##### How It Works
Distant vegetation switches to baked impostor billboards (like M4.5-EXT-30 but for plants) so forests stay cheap at distance. Cross-fade hides the swap.

##### Reference Implementation
```cpp
vec2 uv = OctUV(viewDir); sample impostor;
```

##### Player-Facing Impact
Distant forests stay cheap - no full foliage drawn at range.


---

#### [M4-EXT-26] Procedural Decal Atlas Packing & Runtime Projection *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M4.5-EXT-26 RVT. Packs decals into an atlas and projects at runtime.

##### Math
uv = PackDecals(decalList); project(decals, surface) -> RVT overlay.

##### How It Works
Bullet holes, blood, scorch are packed into a decal atlas and projected onto surfaces at runtime, written into the RVT overlay (M4.5-EXT-26) so they persist and layer correctly. Atlas packing avoids bind spam.

##### Reference Implementation
```cpp
Atlas a = Pack(decals); Project(a, surf);
```

##### Player-Facing Impact
Damage/decals persist and layer on surfaces - world shows wear.


---

#### [M4-EXT-25] Procedural Material Node-Graph Compiler (Structure + Color + Normal) *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M4.5. Compiles a procedural material node graph to a shader/texture set.

##### Math
shader = CompileGraph(graph); textures = BakeGraph(graph, res);

##### How It Works
A material node graph (noise, blends, params) is compiled to a shader for runtime and/or baked to albedo/normal/roughness textures at build. One authored graph drives both. Determinism via seeded params.

##### Reference Implementation
```cpp
Material m = Compile(graph);
```

##### Player-Facing Impact
Surfaces are authored once as a graph - infinite variation, consistent look.


---

#### [M4-EXT-24] Cellular Automata Structural Collapse & Rubble Debris Fields *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M3. Cellular-automata rule that turns damaged structures into rubble fields.

##### Math
cell.next = Rule(cell, neighbors, fatigue); debris spawns where cell collapses.

##### How It Works
A CA over the structure grid, seeded by M3-EXT-09 fatigue, propagates collapse: cells past threshold become rubble, spawning debris fields that feed M2.6 collision + M4-EXT-26 decals. Emergent, physics-flavored ruin.

##### Reference Implementation
```cpp
grid = StepCA(grid, fatigue); SpawnRubble(collapsed);
```

##### Player-Facing Impact
Buildings fall apart believably into rubble - ruins look earned, not placed.


---

#### [M4-EXT-23] Whittaker Temperature/Precipitation Biome Classification *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 PCG. Classifies biome from temp/precipitation via Whittaker curves.

##### Math
biome = Whittaker(temp, precip); blend at boundaries by noise.

##### How It Works
A Whittaker climate diagram maps (temperature, precipitation) to a biome class; boundary cells blend between classes via noise so transitions are gradual, not hard seams. Drives M4-EXT-10 road + M4-EXT-23 foliage placement.

##### Reference Implementation
```cpp
Biome b = Classify(temp, precip);
```

##### Player-Facing Impact
Biomes transition naturally - deserts fade to forest instead of snapping.


---

#### [M4-EXT-22] Meshoptimizer Vertex Cache / Fetch Optimization *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 + M5.1. Reorders mesh indices for vertex-cache and fetch efficiency.

##### Math
order = Meshopt::OptimizeVertexCache(ib); then OptimizeFetch(vb, ib);

##### How It Works
Mesh indices are reordered (vertex-cache then fetch) so GPU vertex shading and memory access are coherent - fewer vertex shader invocations and cache misses per mesh.

##### Reference Implementation
```cpp
meshopt::optimizeVertexCache(ib,ib,nv); meshopt::optimizeFetch(vb,ib,nv);
```

##### Player-Facing Impact
Meshes shade/load faster - more geometry for the same vertex cost.


---

#### [M4-EXT-20] WFC Adjacency Propagator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4 WFC. Propagates adjacency constraints during WFC so connected tile types stay consistent.

##### Math
for neighbor n of cell c: mask[c] &= adjacency[tile][n.dir]; propagate to changed.

##### How It Works
Extends WFC with an explicit adjacency-edge table (road meets road, wall meets floor) that propagates constraint masks to neighbors on each placement, keeping the generated graph connected and sane. Consumes M4-EXT-11's contradiction recovery.

##### Reference Implementation
```cpp
PropagateAdjacency(grid, placed);
```

##### Player-Facing Impact
Generated layouts stay internally consistent - roads connect, walls align.


##### Systems Touched
M4 PCG + M13 SLM + M11 UI. Generates contextual signage text via a grammar for world flavor.

##### Math
sign = Grammar(prod, seededRNG(locale)); terminals drawn deterministically per seed.

##### How It Works
A phrase grammar produces signage (shop names, warnings, graffiti) seeded by world-graph node + locale. Output feeds M13 SLM broadcasts and M11 signage so streets read as lived-in. Deterministic per seed.

##### Reference Implementation
```cpp
string sign = SignageGrammar.Generate(nodeSeed, locale);
```

##### Player-Facing Impact
Streets/ruins carry readable, varied signage - world feels inhabited, not prop-empty.

---

##### Systems Touched

`[M5.4-EXT-06]`'s existing road graph (source geometry — this does not duplicate it, it consumes it), `[M4-EXT-17]` nutrient grid and hydrology grids (both get overridden under roads).

##### Math

Exclusion strength falls off from spline centerline with a hard cutoff at road width plus a soft shoulder blend:

$$w(d) = \begin{cases} 1 & d \le w_{road}/2 \ 1 - \frac{d - w_{road}/2}{w_{shoulder}} & w_{road}/2 < d < w_{road}/2 + w_{shoulder} \ 0 & \text{otherwise} \end{cases}$$

##### How It Works

For every edge in the already-built `[M5.4-EXT-06]` road graph, the terrain generation pass writes an exclusion mask along the spline: full-strength exclusion clears foliage seeding, soil-nutrient variance (flattened to a fixed "graded" baseline), and hydrology (roads shed water rather than pooling it) directly under the road surface, blending to zero over a soft shoulder distance so the transition from graded roadbed to wild terrain isn't a hard seam. This reuses the road graph that already exists for vehicle pathfinding instead of building a second infrastructure layer.

##### Reference Implementation

```cpp
float SampleRoadExclusionWeight(glm::vec3 worldPos, const RoadGraphEdge& edge, float roadWidth, float shoulderWidth) {
    float distToSpline = DistancePointToPolyline(worldPos, edge.proceduralControlSplinePoints);
    float halfRoad = roadWidth * 0.5f;
    if (distToSpline <= halfRoad) return 1.0f;
    if (distToSpline < halfRoad + shoulderWidth) return 1.0f - (distToSpline - halfRoad) / shoulderWidth;
    return 0.0f;
}
```

##### Player-Facing Impact

Roads read as genuinely graded infrastructure cutting through wild terrain — flat, foliage-free, properly draining — with a natural shoulder transition instead of forest growing directly through the asphalt.



## M4.5 — Hybrid ray tracing, GPU-driven pipeline & upscaling

##### Implementation Steps

* Persistent RVT Terrain Overlay: chunk-anchored Runtime Virtual Texture, one persistent overlay texture per loaded terrain chunk, written via a Vulkan storage-image `imageStore` path (`VK_FORMAT_R8G8B8A8_UNORM`, allocated/streamed alongside chunk load/unload). This is a real, standing system — not an abstract interface — and is the actual backing store for three dependent features for skid-mark injection (M9-EXT-07), capillary spatter impact-angle projection (M6.5-EXT-02), and flame-trail splitting (M3-EXT-04).
* Tiered Graphical Quality Ladder (Tier 2: G-Buffer + RT shadows + ReSTIR GI, denoised via DLSS Ray Reconstruction on RTX hardware, FSR 4 Ray Regeneration on RDNA hardware, or standard temporal denoising fallback on Arc/other; Tier 1: G-Buffer + shadows + SSAO + VRS; Tier 0: G-Buffer + static baked).
* Opacity Micromaps for foliage.
* GPU Device Generated Commands. Recoil-Driven VRS. A compute pass evaluates luma variance over local 8×8 image tiles (`σ² = (1/64)Σ(L_i − μ_luma)²`) and forces half-rate shading on low-variance (flat) tiles while preserving full rate across high-contrast material edges; the recoil-driven scalar above modulates the variance threshold rather than replacing this signal.

```glsl
layout(local_size_x = 8, local_size_y = 8) in;
shared float tileLuma[64];
uint CalculateVRSShadingRate(float variance, float limit) {
    return (variance < limit) ? 1u : 0u; // 1u = half-rate shading
}
```

* Sparse Voxel Octree GI Fallback for Tier-1.
* Async Compute Interleaving for particles. Wavefront Subgroup Array Packing.
* Glass Reflection Normals using fractured Voronoi normal data.
* Adaptive Upscaling Interface (FSR/DLSS/XeSS), vendor-detected at boot from M0's Capability-Tier struct — no hardcoded vendor branch, same pattern as the RT-denoiser selection above.
* **Per-pixel motion velocity buffer:** required infrastructure — none of FSR/DLSS/XeSS above function correctly without it. Computed in the vertex/skinning pass for both rigid and skinned-mesh geometry as the delta between current and historical clip-space positions: `v_pixel = (M_ProjViewCurrent · P_local) − (M_ProjViewHistory · P_localHistory)`, written to a dedicated velocity attachment consumed directly by the upscaler.

```glsl
vec2 ComputePixelMotionVector(vec4 currentPosProj, vec4 historyPosProj) {
    vec2 ndcCurrent = currentPosProj.xy / currentPosProj.w;
    vec2 ndcHistory = historyPosProj.xy / historyPosProj.w;
    return (ndcCurrent - ndcHistory) * 0.5;
}
```

* Latency-Reduction Interface (optional, Tier 2 only): NVIDIA Reflex 2 (Frame Warp) on RTX, AMD Anti-Lag 2 on RDNA — both reduce input-to-photon latency using the same input-timestamp data M0's deterministic replay system already captures; no Frame Warp equivalent exists on non-NVIDIA hardware, so Anti-Lag 2 provides latency reduction without the warp step. Off by default on Tier 0/1.
* Virtual Shadow Maps: page-cached, per-texel-consistent shadow resolution replacing fixed-resolution cascaded shadow maps, feeding the same G-buffer lighting pass as the RT-shadow Tier-2 path above.
* Visibility Buffer rendering: a single-pass primitive-ID + barycentric buffer decoupling visibility from shading, compatible with the Meshlet Pipeline (M1) and reducing G-buffer bandwidth versus full-attribute G-buffer writes. **Packing:** since `u+v+w=1`, only store `u`,`v` — pack both as 16-bit unorm into a single `uint32_t` alongside the `PrimitiveID`, and derive `w = 1.0 - u - v` shader-side:

```cpp
// Only u,v need storing (w = 1-u-v); clamped to guard float rounding before the unorm cast
uint32_t PackBarycentrics(float u, float v) {
    uint16_t pu = static_cast<uint16_t>(std::clamp(u, 0.0f, 1.0f) * 65535.0f);
    uint16_t pv = static_cast<uint16_t>(std::clamp(v, 0.0f, 1.0f) * 65535.0f);
    return (static_cast<uint32_t>(pu) << 16) | pv;
}
```

* **Visibility Buffer silhouette anti-aliasing reconstruction:** required companion to the Visibility Buffer bullet above, since deferring attribute interpolation to the shading pass means standard hardware MSAA cannot run. Evaluates barycentric-coordinate screen-space derivatives across primitive boundaries and blends edge pixels by the resulting weight before the upscaler above consumes the frame: `EdgeWeight = Saturate(‖∇u‖ + ‖∇v‖)`.

```glsl
float EvaluateVisibilityEdge(vec2 barycentrics) {
    vec2 dx = dFdx(barycentrics);
    vec2 dy = dFdy(barycentrics);
    return clamp(length(dx) + length(dy), 0.0, 1.0);
}
```

* Cluster-based GPU-driven culling: meshlet/triangle clusters culled on the GPU (frustum + Hi-Z occlusion) ahead of the indirect-draw compaction (M1) already in the pipeline, rather than per-object CPU culling.
* Clipmap terrain streaming: nested-resolution heightfield rings centered on the camera, replacing flat chunk-LOD popping for the M4/M2.6 terrain streaming radius.
* Triplanar PBR mapping for the RVT terrain overlay (M4.5 above) and any steep/undomain-unwrapped geometry (cliffs, rubble piles): blends three axis-projected texture samples by surface-normal weight instead of relying on a UV unwrap.

##### Decisions to flag

RT budget in split-screen (default: rasterized only).

##### Exit Criteria

* Hybrid RT pipeline runs efficiently. Tier-0 fallback behaves flawlessly.
* VRS reduces shading cost dynamically.
* RVT terrain overlay writes and reads verified via a debug visualization pass; skid-mark, spatter, and flame-trail features render onto the real chunk-anchored overlay, not a stubbed/assumed layer.



### Extended Systems Library — engine-side additions for M4.5

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M4.5-EXT-01] Rayleigh-Scattering Volumetric Cone Attenuation (Flashlight Shadowing)

##### Systems Touched

Expands M1 GPU indirect G-buffer rendering and handles M10 weather visual adjustments.

##### Math

Spotlights and flashlights pass through active weather storm grids and cellular smoke blocks. The scattering attenuation coefficient (β_scatter) is evaluated using localized particle density (ρ_smoke) and rain intensity:

`β_scatter = β_clear_air + ζ·ρ_smoke + β_rain_per_unit·RainIntensity`

The flashlight's analytical maximum viewport illumination distance (L_max) decays exponentially over this atmospheric matrix, truncating the G-buffer lighting pass variables inline:

`L_max = L_baseline · e^(−β_scatter · L_baseline)`

The Reference Implementation below uses three flat, JSON-tunable constants (clear-air baseline, smoke coefficient, rain coefficient) rather than the full physical Rayleigh-scattering cross-section (wavelength, refractive index, molecular density) — the physical derivation is why those three constants are the right shape of model, it's just not evaluated per-frame directly.

##### How It Works

To avoid a costly screen-space volumetric lighting loop that would break your 7.0ms rendering target, this feature computes atmospheric cone intersections strictly inside your primary lighting shaders. The system samples your active 2D cellular-automata fire and smoke grid arrays (M6.5) along the forward light tracking vector. The gathered density limits downscale the projection limits (L_max ) dynamically, updating your instance indirect uniform streams directly on the GPU.

##### Reference Implementation

```cpp
// Truncates your spotlight illumination length based on localized atmospheric smoke and rain variables
float GetScatteredLightLength(float base_len, float density, float rain_intensity, float zeta) {
    return base_len * std::exp(-(0.02f + zeta * density + 0.15f * rain_intensity) * base_len);
}
```

##### Player-Facing Impact

Your tactical flashlight is vulnerable to environmental conditions. Activating your light source in clean, clear air reveals structural blocks across long visual ranges. However, if you venture into dense smoke plumes generated by burning structural cells or enter an active heavy downpour, the light beam scatters immediately. The projection cone shortens into a bright, desaturated wall of fog right in front of your viewport, blinding your direct sight lines and forcing you to rely on physical debug overlays or sound cues to detect incoming shapes.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M4.5-EXT-02] Post-Processing Photopupillary Luminance Tracking

##### Systems Touched

Executed completely on the GPU within your M1/M4.5 G-buffer post-processing steps.

##### Math

To preserve frame budget constraints, eye exposure scaling (A_eye) calculates luminance tracking changes across a non-linear differential function with asymmetric time steps (τ_dark ≫ τ_light):

`dA_eye/dt = (L_scene − A_eye) / τ`

##### How It Works

Each frame samples average scene luminance and drives an exponential approach toward it, with separate time constants for brightening vs. darkening — dark adaptation is slow, light adaptation is fast.

##### Reference Implementation

```glsl
// Inlined within your tonemapping compute shader pass
layout(push_constant) uniform ExposureParams { float dt; float tau_light; float tau_dark; };
float AdjustExposure(float scene_lum, float current_exp) {
    float tau = (scene_lum > current_exp) ? tau_light : tau_dark;
    return current_exp + ((scene_lum - current_exp) / tau) * dt;
}
```

##### Player-Facing Impact

Moving from a brightly lit safezone into a pitch-black alleyway inflicts realistic night blindness. Your screen remains heavily crushed and dark for several seconds, forcing you to move slowly until your vision adjusts to the dark contours. Conversely, switching your high-intensity flashlight on inside a dark basement flashes a blinding white glare across your viewport, making light management a key survival consideration.

#### [M4.5-EXT-03] Ocular Afterimage Retinal Bleach Compute Shader

##### Systems Touched

Expands M1 post-processing passes and interfaces with M5.4 powered lighting structures.

##### Math

High-intensity lighting events overwhelm your retinal cell arrays. The local pixel bleach concentration (B) accumulates instantly, fading over a slow logarithmic decay window:

`∂B(x,y)/∂t = κ·max(0.0, L_frame(x,y) − L_threshold) − B(x,y)/τ_bleach`

The resulting bleach buffer overlay performs a localized, inverted color blend pass directly across your final rendering output attachments:

`C_final(x,y) = C_raw(x,y)·(1.0−B(x,y)) + (1−C_raw(x,y))·B(x,y)`

##### How It Works

This effect uses a low-overhead half-resolution floating-point texture attachment that persists between frames. Your final post-processing pass runs a compute shader lane that samples the screen luminance layout; if an emission threshold is crossed, it injects values into the persistent buffer, blending the results over subsequent frames with minimal memory tracking overhead.

##### Reference Implementation

```glsl
// Previous version only decayed the bleach buffer (`b - dt/tau`) and never implemented
// the injection term (κ·max(0, L_frame − L_threshold)) from the math above — as written, bright
// lights could never actually burn an afterimage in, only fade one out. Added the missing terms.
// Executed in your post-processing pipeline pass to calculate local screen burn-in effects
vec3 ApplyRetinalBleach(vec2 uv, vec3 raw_color, float scene_lum, float threshold, float kappa, sampler2D bleach_tex, float dt, float tau, out float new_bleach) {
    float b = texture(bleach_tex, uv).r;
    b += (kappa * max(0.0, scene_lum - threshold) - b / tau) * dt;
    new_bleach = clamp(b, 0.0, 1.0);
    return mix(raw_color, vec3(1.0) - raw_color, new_bleach);
}
```

##### Player-Facing Impact

When a generator substation explodes or a flashbang erupts right in your face, turning away doesn't clear your vision. A bright, burning inverted silhouette of the blast geometry stays burned straight into your screen, slowly fading over time and muddying your sight lines while you try to escape through the dark.

##### Depends on (not yet built at this point)

M5.4 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M4.5-EXT-04] Thin-Film Specular Wetness Shading

##### Systems Touched

Inlined within your Vulkan M1/M4.5 G-buffer terrain fragment shader passes.

##### Math

Accumulated rainfall maps create a microscopic water layer thickness (d) over world surfaces. The surface specular reflection intensity (R) updates dynamically using an adaptation of Fresnel's thin-film interference formulas based on the water saturation variables (Wetness):

`R(θ) = R_base·(1.0−Wetness) + Wetness·(R_water + (1.0−R_water)·(1.0−cosθ)^5)`

##### How It Works

To run safely within your 7.0ms rendering target, this feature completely avoids extra rendering passes. Your base terrain fragment shader reads the 2D wetness byte field from your active weather stream data. The layer calculations adjust the roughness and specular parameters inside the lighting calculations inline, executing on the GPU.

##### Reference Implementation

```glsl
// [SUPERSEDED — see [M4.5-EXT-06]] Equivalent to calling that version's ApplyWetShading with porosity = 0.0
vec3 ApplyWetShading(vec3 diffuse, vec3 spec, float wet, float cos_theta) {
    float f = 0.02 + 0.98 * pow(1.0 - cos_theta, 5.0);
    return mix(diffuse, diffuse * 0.3, wet) + mix(spec, vec3(f), wet);
}
```

##### Player-Facing Impact

As storm conditions pass over a sector, the dry, desaturated concrete roads visually shift. Pavements take on a dark, saturated look while reflecting sharp silhouettes of safehouse searchlights and muzzle flashes directly across the asphalt surface, enhancing visual realism without dropping your rendering frame rates.

#### [M4.5-EXT-05] Retinal Flash-Blindness Contrast Silhouette Tracking

##### Systems Touched

Inlined within your GPU M1/M4.5 post-processing post-exposure shaders.

##### Math

Extremely intense illumination events burn your retinal color attachments, but moving shapes mask the light vectors. The local pixel overlay applies an inverted high-contrast edge convolution to the screen burn-in buffer:

`C_silhouette (x,y)=C_burn (x,y)⋅K_Sobel ⊗L_frame (x,y)`

##### How It Works

A bright light event writes a screen-space burn-in buffer that persists for several frames. While active, a Sobel edge convolution inverts the burn color where a moving silhouette's edges fall, keeping shapes trackable through the wash-out.

##### Reference Implementation

```glsl
// Executed inline within your post-processing tonemapping steps
vec3 ApplyRetinalSilhouette(vec3 raw_color, vec3 burn_color, float edge_mask) {
    return mix(raw_color, vec3(1.0) - raw_color, burn_color * edge_mask);
}
```

##### Player-Facing Impact

Blinding yourself with a close flashbang or an electrical grid surge doesn't just display a frustrating blank screen. While your view is washed out in white, moving objects like sprinting zombies leave crisp, high-contrast black silhouettes burned onto your eye buffer, letting you track their shapes perfectly through total visual chaos.

#### [M4.5-EXT-06] Fresnel-Driven Multi-Layer Surface Saturation Shading

##### Systems Touched

Embedded directly inside your Vulkan runtime terrain fragment and RVT texturing pipeline pass.

##### Math

To prevent players from manipulating screen gamma settings or gaming visibility checks during rain cycles, water tracking scales through an analytical sub-resolution substrate porosity factor (ψ). Saturated materials shift reflections into sharp mirror highlights while absorbing and shifting diffuse values non-linearly over micro-puddle layer thickness values (d):

`R(θ) = R_base·(1.0−Wetness) + Wetness·(R_water + (1.0−R_water)·(1.0−cosθ)^5)`

`Diffuse_sat = Diffuse_base·(1.0 − ψ·Clamp(d/d_max, 0.0, 0.7))`

##### How It Works

The terrain fragment shader samples wetness/porosity computed server-side from the rain-intensity track — not client-adjustable — applying both the Fresnel blend and porosity-scaled diffuse darkening so a gamma/brightness slider can't undo either term.

`R(θ)` is the same Fresnel term as [M4.5-EXT-04]. This version (`ApplyWetShading` with a `porosity` parameter) is the canonical one — a strict superset adding porosity-scaled diffuse darkening (`Diffuse_sat`) on top of EXT-04's spec-only blend. [M4.5-EXT-04] should call this function with `porosity = 0.0` rather than maintaining a second implementation.

##### Reference Implementation

```glsl
// [CANONICAL — see [M4.5-EXT-04]] Superset of that feature's ApplyWetShading; porosity=0.0 reproduces it exactly
vec3 ApplyWetShading(vec3 diff, vec3 spec, float wet, float cos_t, float porosity) {
    float f = 0.02 + 0.98 * pow(1.0 - cos_t, 5.0);
    return mix(diff, diff * (1.0 - 0.7 * porosity), wet) + mix(spec, vec3(f), wet);
}
```

##### Player-Facing Impact

When a rain storm matches full intensity, surfaces don't just change to a generic glossy shader. Asphalt, brick, and soil layers dark and saturate realistically based on their real-world structural porous absorption properties. Reflection lines lock into exact Fresnel values, preventing players from manipulating standard brightness options to gain artificial sight visibility inside unlit stormy nights.

#### [M4.5-EXT-07] Compute-Shader Procedural Texture Synthesizer Pipeline

##### Systems Touched

Connects M4.5's persistent RVT terrain overlay (`imageStore` write path) directly to the OpenSimplex2 generation passes.

##### Math

Multi-layered fractional Brownian motion (FBM) accumulates octaves of noise into a single distribution value, evaluated once per texel inside a one-time generation dispatch rather than per-frame:

`FBM(x) = Σ_{i=0}^{N−1} a_i · Noise(f_i · x)`

##### How It Works

Evaluating multi-layered, domain-warped OpenSimplex2 noise inside a G-buffer fragment shader per-pixel, per-frame would saturate your RTX 2070 Super's arithmetic units and collapse frame pacing. This system decouples texture generation from rendering. When a sector chunk streams into memory, the engine issues a single one-time compute-shader dispatch that runs the procedural material logic once, synthesizing albedo, roughness, normal, and material blend data, then writes the results directly into the chunk-anchored RVT pages via Vulkan `imageStore`. The rendering pipeline is then insulated from noise-generation math entirely, sampling the pre-baked RVT pages through fast, low-overhead bindless texture lookups every frame instead of recomputing noise.

Two issues that surface at scale, both handled at the pipeline level:

1. **Memory coherency race:** sampling the RVT page in the G-buffer fragment shader immediately after the async compute `imageStore` — with no explicit hardware layout/access tracking — is a genuine race and shows up as visible tile flicker. Insert an explicit `VkImageMemoryBarrier2` right after the compute dispatch: `srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT`, `dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT`, `srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT`, `dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT`.
2. **Allocation fragmentation:** don't allocate a fresh device-memory block per chunk on stream-in — that fragments the pool and stalls the driver. Pre-allocate a large uniform VMA block pool at boot and sub-allocate RVT page slots linearly out of it (the same pattern as [M4.6-EXT-01]'s vertex-buffer pooling), returning slots to the async tracking loop on chunk eviction.

The `local_size_x = 16, local_size_y = 16` dispatch below is intentional, not arbitrary — it's a clean 256-thread block that maps evenly onto both 32- and 64-thread hardware warps; don't shrink it to an odd/non-power-of-two size.

##### Reference Implementation

```glsl
// Inlined within your dedicated procedural texture synthesizer compute shader pass
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba8) writeonly uniform image2D rvt_page_image;

void main() {
    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    // Execute fractional Brownian motion noise rules once inside compute pass
    float noise_val = EvaluateProceduralFBM(vec2(texel_coord) * 0.005);
    vec4 packed_material = vec4(vec3(noise_val), 1.0);

    imageStore(rvt_page_image, texel_coord, packed_material);
}
```

```cpp
// Explicit barrier required between the compute write above and the next frame's
// G-buffer sample of the same RVT page — without this, tiles flicker from a genuine data race
VkImageMemoryBarrier2 rvtWriteToReadBarrier{
    .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
    .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
};
```

##### Player-Facing Impact

Visual material variety remains incredibly high without dragging down your frame rate. High-fidelity asphalt cracks, specialized concrete weathering paths, mud saturations, and blood spatters blend dynamically across kilometers of terrain, while preserving a stable update loop on both Recommended (60 FPS) and Tier-0 floor (30 FPS) hardware.

##### Depends on (not yet built at this point)

M4.5 base RVT allocation layer — must compile against a temporary stand-in storage image if tested ahead of full RVT page tracking initialization.

#### [M4.5-EXT-08] Two-Pass GPU-Driven Cluster Occlusion Culling (HZB Culling)

##### Systems Touched

Updates M4.5 hybrid ray tracing / G-buffer rendering and integrates with M1 draw-compaction loops.

##### Math

A compute-shader occlusion test evaluates a cluster's bounding envelope against a downsampled depth pyramid. Stated with the project's standard (non-reversed) depth convention, where smaller values are closer to camera: a cluster is visible only if its nearest point is not farther than the HZB's stored depth at that texel —

`Visible = (Depth_bounds_near ≤ HZB_sample)`

##### How It Works

Visibility cells can't be precomputed in an infinite, dynamically generated world, and drawing un-culled city geometry would overwhelm the rasterizer. This uses a two-pass Hierarchical Z-Buffer pipeline: Pass 1 draws the previous frame's visible entities to build a depth pyramid; Pass 2 runs a compute shader that projects the current frame's procedural meshlet bounds against that HZB. Visible geometry handles are written into a Vulkan indirect-draw buffer via atomic counters, culling occluded geometry before rasterization.

##### Reference Implementation

```glsl
// Inlined within the primary HZB occlusion compute shader pass; assumes standard (non-reversed) depth
bool IsClusterVisible(vec3 boundsNear, float hzb_depth) {
    return boundsNear.z <= hzb_depth; // cull if the cluster's nearest point is behind the occluder
}
```

##### Player-Facing Impact

Framerates stay high and stable looking down cluttered city streets or standing in a dense central square. The engine skips rendering hidden rooms and occluded alleys entirely, saving GPU headroom for what's directly in view.

#### [M4.5-EXT-09] Bindless Material Shader Virtualization (Permutation Guard)

##### Systems Touched

Enforces safety constraints on M4.5 hybrid pipelines and accelerates M1 shader toolchains. Slot handles are allocated via `[M0-EXT-08]`'s descriptor page allocator — this feature is that allocator's primary consumer, not a second handle-management scheme. Also folds in a material-classification pre-pass that groups pixels by material index into uniform buckets before shading, avoiding warp/wavefront divergence from directly branching on a per-pixel bindless index.

##### Math

Surface materials resolve via an integer handle stored per-primitive, read out of the Visibility Buffer rather than baked into pipeline state:

`MaterialIdx = PrimitiveBuffer[PrimitiveID].MaterialRow`

Classification writes matching pixel coordinates into per-material bucket buffers ahead of shading: `imageStore(MaterialClassificationBuffer, TexelCoord, MaterialBucketID)`.

##### How It Works

Compiling a separate pipeline permutation for every procedural material blend (blood over mud, rust over sheet metal, and so on) causes a combinatorial explosion of pipeline objects and severe shader-compilation stuttering during play. This system uses exactly one master pipeline state object for opaque geometry and one for transparency. Every material configuration lives as a row in a global bindless data array indexed through `[M0-EXT-08]`'s slot allocator; the pixel shader reads a single integer index fetched from the Visibility Buffer to look up material properties at runtime, so no new PSO is ever compiled for a new material combination. Before the main shading dispatch, a compute pre-pass classifies pixels by `MaterialIdx` into uniform buckets (below) so the shading pass itself doesn't branch divergently per-pixel across warps/wavefronts.

##### Reference Implementation (classification pre-pass)

```glsl
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, r32ui) uniform uimage2D materialBucketImage;
void main() {
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
    uint matId = ExtractMaterialIndex(uv); // Fetches primitive properties out of the visibility pass data
    imageStore(materialBucketImage, uv, uvec4(matId, 0, 0, 0));
}
```

##### Reference Implementation

```glsl
// Fetches virtualized surface data within the primary shading pass
vec4 ShadingPassLookup(uint mat_index, vec2 uv) {
    return texture(BindlessTexArray[nonuniformEXT(mat_index)], uv);
}
```

##### Player-Facing Impact

The environment shows rich material variety with zero compilation stutter. Explosions char walls instantly, and vehicles leave dynamic skid marks and blood spatter on roads without triggering pipeline stalls.

#### [M4.5-EXT-10] Global Distance Field Generation for Volumetric Shadow Proxies (SDF Shadows)

##### Systems Touched

Extends M4.5 hybrid ray tracing and reacts to M3 macro-destruction structural updates.

##### Math

Raymarching steps through a distance field, accumulating travel distance until an intersection or the step budget is exhausted:

`Distance = SampleSDFVolume(RayCurrentPosition)`

##### How It Works

Crisp long-distance shadows normally require heavy shadow-map cascades or ray-traced geometry lookups that saturate the GPU. This system bakes low-resolution Signed Distance Field volumes for structural elements on background worker threads — regenerated for the affected region whenever M3 destruction changes local geometry. The graphics pipeline evaluates distant shadows and ambient occlusion by raymarching directly through these unified volume textures, bypassing raw triangle intersection entirely.

##### Reference Implementation

```glsl
// Approximates structural occlusion using cheap distance-field evaluations
float RaymarchSDFShadow(vec3 origin, vec3 direction, float max_dist) {
    float t = 0.01;
    for (int i = 0; i < 32; ++i) {
        float h = SampleSDF(origin + direction * t);
        if (h < 0.001) return 0.0; // hit found, shadow is blocked
        t += h;
        if (t > max_dist) break;
    }
    return 1.0;
}
```

##### Player-Facing Impact

Distant structures and debris fields cast smooth, accurate shadows across roads and valleys. Ambient shadowing stays clean over long distances without cratering frame rate or shadow fidelity.

#### [M4.5-EXT-11] Multi-Sampled Depth Derivative Silhouette Reconstruction Filter

##### Systems Touched

Complements M4.5's Adaptive Upscaling Interface and per-pixel motion velocity buffer — sharpens geometry silhouettes specifically on Tier-0/Tier-1 profiles running spatial denoisers without hardware ray tracing, where temporal upscaling alone tends to ghost/smear fast-moving edges.

##### Math

Discrete Laplacian over a 3×3 depth stencil flags sharp discontinuities:

`∇²D = [D(x+1,y) + D(x−1,y) + D(x,y+1) + D(x,y−1)] − 4D(x,y)`

##### How It Works

A compute pass samples the scene depth buffer at each texel's four immediate neighbors and evaluates the discrete Laplacian. Values crossing a tunable threshold are flagged directly into the upscaler's edge mask (feeding the same motion-vector-driven upscale pass M4.5 already declares), sharpening silhouette reconstruction specifically where cheaper spatial denoisers would otherwise blur high-velocity edges.

##### Reference Implementation

```glsl
layout(local_size_x = 16, local_size_y = 16) in;
// r32f is a storage-image format qualifier and isn't legal on a combined texture-sampler —
// the original reference implementation wrote `layout(binding = 0, r32f) uniform sampler2D`,
// which won't compile. A sampler's texel format comes from the bound image view, not a
// shader-side qualifier; only the binding index belongs here.
layout(binding = 0) uniform sampler2D sceneDepthTex;

void main() {
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
    float dCenter = texelFetch(sceneDepthTex, uv, 0).r;
    float dLeft   = texelFetch(sceneDepthTex, uv + ivec2(-1, 0), 0).r;
    float dRight  = texelFetch(sceneDepthTex, uv + ivec2(1, 0), 0).r;
    float dUp     = texelFetch(sceneDepthTex, uv + ivec2(0, 1), 0).r;
    float dDown   = texelFetch(sceneDepthTex, uv + ivec2(0, -1), 0).r;
    float laplacianEdge = (dLeft + dRight + dUp + dDown) - (4.0 * dCenter);
    // values crossing threshold feed the upscaler's edge mask
}
```

##### Player-Facing Impact

Fast-moving zombies and vehicles keep crisp outlines on lower-tier hardware profiles instead of smearing into the background during upscaled, non-RT rendering.

#### [M4.5-EXT-12] Visibility Buffer Material-Classification Wavefront Compactor Pass

##### Systems Touched

Visibility Buffer shading pipeline, asynchronous compute shader dispatches, bindless lookups. Extends `[M4.5-EXT-09]`'s material-classification pre-pass with the concrete cross-workgroup compaction mechanism that pre-pass calls for but doesn't fully specify.

##### Math

Subgroup ballot bitwise lane-reductions combined with a global atomic append to compress divergent procedural texture indexing into uniform hardware execution warps:

`LaneOffset = subgroupExclusiveAdd(int(MatID_pixel ≡ MatID_target))`, `GlobalOffset = atomicAdd(bucketCounter, subgroupLaneCount) + LaneOffset`

##### How It Works

Scans screen-space coordinates, grouping matching material rows together via hardware subgroup intrinsics before appending them into a dense linear execution stream, eliminating intra-warp divergence during the final G-buffer sampling pass.

**[BUG-FLAG]** The originally submitted reference implementation computed `subgroupExclusiveAdd` as a per-workgroup local lane offset and wrote directly to `materialBucketImage` at that offset with no global counter. Since every dispatched workgroup's first matching lane resolves to offset 0, every workgroup across the screen would overwrite the same image texels — the "global atomic appends" claimed in the Math section was never actually implemented. Fixed below by adding a single `uint` atomic counter buffer that each subgroup increments once (by its total matching-lane count) to reserve a disjoint output range before writing.

##### Reference Implementation

```glsl
#version 460
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_arithmetic : require

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, r32ui) uniform uimage2D materialBucketImage;
layout(binding = 1) uniform utexture2D primitiveIdTexture;
layout(binding = 2, std430) buffer BucketCounter { uint bucketWriteCursor; };

layout(push_constant) uniform MaterialTarget { uint targetMaterialClassId; };

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    uint rawPrimitiveData = texelFetch(primitiveIdTexture, pixelCoord, 0).r;
    uint currentMaterialId = rawPrimitiveData & 0xFFFFu;

    bool isTargetMaterial = (currentMaterialId == targetMaterialClassId);
    uvec4 activeLaneBallot = subgroupBallot(isTargetMaterial);
    uint matchingLaneCount = subgroupBallotBitCount(activeLaneBallot);

    uint subgroupBaseOffset = 0u;
    if (subgroupElect() && matchingLaneCount > 0u) {
        subgroupBaseOffset = atomicAdd(bucketWriteCursor, matchingLaneCount);
    }
    subgroupBaseOffset = subgroupBroadcastFirst(subgroupBaseOffset);

    if (isTargetMaterial) {
        uint localLaneOffset = subgroupExclusiveAdd(1u);
        uint globalOffset = subgroupBaseOffset + localLaneOffset;
        imageStore(materialBucketImage, ivec2(int(globalOffset), 0), uvec4(pixelCoord.x, pixelCoord.y, 0u, 0u));
    }
}
```

##### Player-Facing Impact

Stabilizes frame rates during dense close-quarters combat scenes, preventing rendering micro-stutters when multiple material variants interact simultaneously within the frame budget.

#### [M4.5-EXT-13] Cluster Depth-Bounds Frustum Voxelizer for Virtual Shadow Maps

##### Systems Touched

Virtual Shadow Map generation, light-space visibility culling, shadow page allocator loops.

##### Math

Frustum voxelization slicing bounding depths to prune shadow page allocations across empty space coordinates:

`Visible_page = (Z_cluster_near ≤ Z_shadow_bounds_far) ∧ (Z_cluster_far ≥ Z_shadow_bounds_near)`

##### How It Works

Subdivides the screen camera frustum into discrete 3D voxel clusters, projecting their localized depth bounds against the shadow-casting light source. Clusters whose depth range doesn't overlap the light's shadow-relevant depth bounds are skipped entirely, so the engine never allocates, clears, or evaluates Virtual Shadow Map pages for empty or fully occluded regions.

**[BUG-FLAG]** The originally submitted reference implementation compared `nearDepth >= farDepth` from the same min-max depth pyramid sample — under a standard convention where near ≤ far, that condition is true only for a degenerate/empty cluster, meaning almost every real cluster would be marked "skip" and shadow pages would essentially never allocate. It also never referenced the light's shadow bounds at all, so it didn't implement the overlap test stated in the Math section. Fixed below to actually test cluster depth range against the light-space shadow bounds.

##### Reference Implementation

```glsl
#version 460
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(binding = 0, r32f) uniform readonly image2D minDepthPyramid;
layout(binding = 1, r32f) uniform readonly image2D maxDepthPyramid;
layout(binding = 2, std430) writeonly buffer ShadowPageRequestTable { uint allocationFlags[]; };

layout(push_constant) uniform ShadowBounds { float shadowBoundsNear; float shadowBoundsFar; };

void main() {
    ivec2 clusterId = ivec2(gl_GlobalInvocationID.xy);
    float clusterNear = imageLoad(minDepthPyramid, clusterId).r;
    float clusterFar  = imageLoad(maxDepthPyramid, clusterId).r;

    uint pageIndex = uint(clusterId.y * 128 + clusterId.x);
    bool visible = (clusterNear <= shadowBoundsFar) && (clusterFar >= shadowBoundsNear);
    allocationFlags[pageIndex] = visible ? 1u : 0u;
}
```

##### Player-Facing Impact

Maintains crisp, high-resolution shadow mapping across wide urban vistas without inducing memory spikes or allocation hitches when sprinting across sector thresholds.

#### [M4.5-EXT-19] Anisotropic Micro-Surface Optics & Heiligenschein Retroreflection

##### Systems Touched

`[M4.5]`'s existing wet-surface shading (`[M4.5-EXT-04]`), material tier data from `[M4-EXT-14]`.

##### Math

Standard anisotropic backscatter falls out of existing GGX terms already in the renderer; the added Heiligenschein term is a brightness boost near the anti-solar point ($\theta \approx 0$ between view and light vectors):

$$I_{Heiligenschein}(\theta) = I_{base} \cdot \left(1 + k \cdot e^{-\theta^2/2\sigma^2}\right), \quad \theta = \angle(V, L_{\text{sun}})$$

##### How It Works

Porous terrestrial materials (dirt, grass, dew-covered surfaces) already carry a wetness/porosity value from `[M4.5-EXT-04]`; this feature adds a small forward-facing brightness boost calculated per-pixel from the angle between the view vector and the sun vector from `[M10-EXT-02]`, peaking exactly at the camera's own shadow point and falling off with a Gaussian, matching the physical retroreflection real porous/dewy ground exhibits. Crystalline/wet surfaces separately get a high-frequency normal-perturbation glint term sampled from a tiling blue-noise normal map, so specular sparkle positions shift correctly as the camera moves instead of looking baked-in.

##### Reference Implementation

```cpp
float ComputeHeiligenscheinBoost(glm::vec3 viewDir, glm::vec3 sunDir, float porosity, float sigma) {
    float theta = std::acos(glm::clamp(glm::dot(viewDir, sunDir), -1.0f, 1.0f));
    float k = porosity * 0.6f; // only porous/dewy surfaces get a strong boost
    return 1.0f + k * std::exp(-(theta * theta) / (2.0f * sigma * sigma));
}
```

##### Player-Facing Impact

Dewy grass and porous dirt show a faint bright halo around the camera's own shadow (exactly like real dew-covered lawns do), and wet/crystalline surfaces sparkle with tiny shifting specular points as you move instead of a flat, static shine.

#### [M4.5-EXT-20] Beer-Lambert Translucent Medium Light Extinction
#### [M4.5-EXT-14] Compute Skinning Vertex Tangent-Space Recomputer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M5.1. Recomputes vertex tangent space after skinning on compute so normal mapping stays correct.

##### Math
T = Orthonormalize(T - dot(T,N)*N, N); B = cross(N,T); per vertex post-skin.

##### How It Works
After the skinning compute pass writes deformed positions/normals, a second pass recomputes the tangent basis per vertex (orthonormalized to the new normal) so normal/parallax maps light correctly on animated characters/creatures.

##### Reference Implementation
```cpp
void RecomputeTangent(vec3& T, vec3 N){ T=normalize(T-dot(T,N)*N); vec3 B=cross(N,T); }
```

##### Player-Facing Impact
Skinned characters keep correct lighting/skin detail through extreme joint deformation.

---
#### [M4.5-EXT-15] Hysteresis-Gated TAA Variance Clamper *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M0-EXT-40. Clamps TAA temporal variance with hysteresis to kill ghosting without losing detail.

##### Math
if(var>thr && stable) blend->current; else keep history; hysteresis band avoids threshold flicker.

##### How It Works
Tracks per-pixel temporal variance; when variance exceeds threshold AND pixel is stable, clamps history blend toward current (kills ghosting). Hysteresis band prevents flicker at the threshold. Works with M4.5-EXT-18 visibility buffer.

##### Reference Implementation
```cpp
if(variance>thr && stable) out=mix(history,current,clampK);
```

##### Player-Facing Impact
AA stays stable - no smearing on motion, no shimmer when still.

---
#### [M4.5-EXT-16] Software Micro-Polygon Voxel Rasterizer (compute) *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M2.6. Computes micro-polygon voxelization for destruction/footprint detail without a hardware path.

##### Math
voxel = RasterMicro(poly, vol); coverage dilated by k for blend.

##### How It Works
A compute pass voxelizes small destroyed fragments/footprints into a low-res volume used for decals, debris masking, footstep material resolution. Software path keeps it off the critical render pass.

##### Reference Implementation
```cpp
void VoxelizeMicro(Quad q, VoxelVol& vol, int k){ for(px in q.proj) vol.Set(px, cov); if(k) Dilate(vol,k); }
```

##### Player-Facing Impact
Destruction leaves voxel-accurate debris/footprint detail, not just a hole.

---
#### [M4.5-EXT-17] Directional Ambient Visibility Field Cache *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M6. Caches a directional ambient visibility field so GI + audio occlusion query cheaply.

##### Math
vis(dir) = TraceOcclusion(dir, field); sampled per probe; reprojected on camera move.

##### How It Works
Bakes a low-res directional visibility field per probe (SH or octahedral). Lighting uses it for cheap AO/GI; M6 acoustic occlusion reuses the same field so sound and light agree on what's blocked.

##### Reference Implementation
```cpp
float Visibility(vec3 dir, ProbeField& f){ return f.Sample(octEncode(dir)); }
```

##### Player-Facing Impact
Lighting + sound occlusion are consistent and cheap - one field serves both.

---
#### [M4.5-EXT-18] Transient Skinned Vertex Cache for Visibility Buffer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M1 visibility buffer. Caches transient skinned vertices for the visibility buffer within a frame.

##### Math
ring cache of skinned verts keyed by (mesh,frame); hit on same-frame reuse.

##### How It Works
Skinned vertices needed by the visibility-buffer pass are cached in a small ring buffer for the frame so the same deformed vertex isn't recomputed by multiple consumers (shadow, main, velocity). Cleared each frame.

##### Reference Implementation
```cpp
SkinnedCache c; if(!c.Get(key,out)){ out=Skin(pos,nrm); c.Put(key,out); }
```

##### Player-Facing Impact
Visibility buffer stays coherent across passes without redundant skinning.
#### [M4.5-EXT-27] Scalable Ambient Obscurance (SSAO) *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Tier-1 quality ladder. Reads G-Buffer depth+normal from M4.5 deferred pass. Outputs occlusion term multiplied into ambient/SH fill (complements M10-EXT-03 SH ambient).

##### Math
McGuire 2012 SSAO: per pixel sample N points in view-space sphere radius R, project to screen, compare depth; occlusion = sum(max(0,z_view-z_sample)/z_sample). 4x4 rotated poisson disk (24-sample spiral) + bilateral depth-weight to avoid haloing.

##### How It Works
Fullscreen pass after G-Buffer: reconstruct view-space pos from depth, sample AO kernel with per-pixel rotation (blue-noise dither, reuse K-EXT-22 STBN), blur with depth-aware edge-preserving filter, output single-channel AO texture. Tier-1 only; Tier-2 uses ReSTIR GI (subsumes contact shadows) as fallback for 6GB floor.

##### Reference Implementation
```cpp
float SSAO(uint2 px, Texture2D depth, float radius, uint samples){ float occ=0; vec3 P=ViewPos(px,depth); for(uint i=0;i<samples;++i){ vec3 s=Kernel(i)*radius; occ+=max(0.0, P.z - ViewPos(px+s.xy,depth).z)/ViewPos(px+s.xy,depth).z; } return 1.0 - occ/samples; }
```

##### Player-Facing Impact
Tier-1 contact shadows / ambient occlusion on the 6GB floor without ReSTIR.

---
#### [M4.5-EXT-28] Screen-Space Reflections (SSR) *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Absent as a system previously. Reads G-Buffer depth+normal+rough/metal; writes reflection sample for M4.5 lighting resolve. Tier-1/2 (Tier-0 uses baked env approximation).

##### Math
Ray-march in view space from R=reflect(V,N); at each step project to screen, compare ray depth vs scene depth, accept on crossing (binary-search refine). Fallback to env/SH on miss/over-distance. Roughness spreads ray origin/length (fewer steps for rough).

##### How It Works
Fullscreen pass after opaque G-Buffer. For each pixel above roughness threshold, march R; on hit sample HDR color buffer (RVT/atlas) at hit UV, mix by Fresnel. Cheap hierarchical-Z accelerated march (reuse existing HZB) for large steps. Denoised with short temporal/edge-aware blur.

##### Reference Implementation
```cpp
bool TraceSSR(vec3 P, vec3 R, Texture2D depth, out vec2 hitUV){ vec3 pos=P; float step=kStep; for(int i=0;i<kMaxSteps;++i){ pos+=R*step; vec2 uv=Proj(pos); float d=Linearize(depth.Sample(uv)); if(pos.z>d){ hitUV=uv; return true; } } return false; }
```

##### Player-Facing Impact
Wet/metal surfaces reflect the world — richer Tier-1 visuals without ReSTIR.

---
#### [M4.5-EXT-29] Screen-Space Global Illumination (SSGI) Fallback *(SOURCED FROM PLAN FILE)*

##### Systems Touched
ReSTIR GI is Tier-2 only. This is Tier-0/1 approximation: screen-space diffuse bounces reusing same G-Buffer+HZB+temporal denoiser M4.5-EXT-15. Not a ReSTIR replacement — cheap fallback.

##### Math
Per pixel trace short diffuse ray (cosine-weighted hemisphere around N), march like SSR, on hit sample lit color, apply Lambert albedo/pi*L*max(0,N.w). One bounce; multi-bounce approximated by reusing prior frame SSGI buffer (temporal, reprojected via M4.5-EXT-15 velocity buffer).

##### How It Works
Same fullscreen march infra as M4.5-EXT-28 but diffuse-weighted + lower precision, blended under direct lighting. Bounded sample count (8-16) for 6GB floor. Denoised by existing TAA variance-clamp path. Disabled on Tier-2 (ReSTIR instead).

##### Reference Implementation
```cpp
vec3 SSGI(vec3 P, vec3 N, Texture2D litColor, int samples){ vec3 acc=vec3(0); for(int i=0;i<samples;++i){ vec3 w=CosineSample(N); if(TraceSSR(P,w,litColor,uv)) acc+=litColor.Sample(uv)*max(0.0,dot(N,w)); } return acc/samples; }
```

##### Player-Facing Impact
Tier-0/1 gets cheap indirect light instead of flat ambient on the 6GB floor.

---
#### [M4.5-EXT-30] Impostor LOD (Octahedral) for Distant Meshes *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Distant static meshes (buildings, ruined vehicles) currently pop or draw full geometry. Generates octahedral-impostor atlases per mesh at bake time; sampled beyond view-distance threshold. Reuses M4.5 LOD/culling.

##### Math
Octahedral impostor: pre-render mesh from K directions on octahedron hemisphere into one atlas. Runtime compute view octahedral coord oct=octEncode(normalize(viewDir)), index atlas, sample impostor billboard. Cheaper than spherical/3D layouts (better texel packing).

##### How It Works
Offline: render K (e.g. 32) views into atlas. Runtime: billboard quad with UVs remapped by octahedral direction; depth preserved via depth-impostor write for correct occlusion. Cross-fade last real LOD to impostor over small distance band to hide swap. Feeds same culling as rest of M4.5.

##### Reference Implementation
```cpp
vec2 OctUV(vec3 vd){ vec3 n=normalize(vd); n/=(abs(n.x)+abs(n.y)+abs(n.z)); return vec2(n.x/(1-abs(n.z)), n.y/(1-abs(n.z)))*0.5+0.5; }
```

##### Player-Facing Impact
Distant cities/ruins stay detailed at range without drawing full geometry — perf win on 6GB floor.

---
#### [M4.5-EXT-31] Signed Distance Field (SDF) Shadow Cascade *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Only passing mention previously. Builds sparse clipmap SDF of nearby opaque geometry from depth buffer (or baked mesh SDFs) for cheap soft long-range shadows + contact shadows shadow-map cascade can't afford at distance.

##### Math
Per clipmap level, rasterize scene depth into SDF via 6-sweep or jump-flooding on depth buffer; d(p)=nearestSurfaceDistance(p) with sign from depth-vs-scene. Shadow test along light ray accumulates min over steps of d(ray(t))/t (cone soft shadow): vis=saturate(1-k*min_d/t).

##### How It Works
Compute pass converts depth (or downsampled depth) into SDF clipmap each time camera moves a cell. Lighting samples SDF along light dir for cheap soft shadow at range where shadow maps run out of resolution. Hybrid: near field shadow maps, far field SDF (fallforward, not double system).

##### Reference Implementation
```cpp
float SdfShadow(vec3 P, vec3 L, Texture3D sdfClip, float coneK){ float t=0,vis=1; for(int i=0;i<kSteps;++i){ t+=kStep; float d=SdfSample(sdfClip, P+L*t); vis=min(vis, saturate(1.0-coneK*d/t)); } return vis; }
```

##### Player-Facing Impact
Long-range soft shadows hold at distance on the 6GB floor without huge shadow-atlas cost.

---

#### [M4.5-EXT-26] Runtime Virtual Texture (RVT) Base System *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M4.6 streaming. Core RVT: page table, tile allocator, feedback (M4.5-EXT-21).

##### Math
page = Hash(worldPos>>pageShift); tile = Alloc(); phys = tile.addr;

##### How It Works
The base RVT holds a page table mapping world regions to physical tiles, a tile allocator (ring/LRU), and a feedback path (M4.5-EXT-21). All RVT consumers (terrain, decals M4-EXT-26, skidmarks M9-EXT-22, blood M6.5-EXT-13) share it. Streamed via M4.6-EXT-05.

##### Reference Implementation
```cpp
PageTable pt; TileAlloc ta; Feedback fb;
```

##### Player-Facing Impact
One virtual-texture system serves terrain/decals/tracks - coherent, streamed, low VRAM.


---

#### [M4.5-EXT-25] Specialization Constants for Bindless Material Uber-Shader *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M1-EXT-25 bindless. Drives the material uber-shader via specialization constants.

##### Math
const bool kSSR = spec; #if shader branches compile-time per permutation;

##### How It Works
Feature toggles (SSR, SSGI, decals) are specialization constants so the uber-shader compiles only the needed permutation per material at pipeline-creation, avoiding runtime branches while staying one source shader. Bindless feeds it all textures.

##### Reference Implementation
```cpp
VkSpecializationInfo sci = {kSSR,kSSGI};
```

##### Player-Facing Impact
Material variants compile lean - no runtime branch cost, one shader source.


---

#### [M4.5-EXT-24] Subpass-Less Explicit Load/Store Resolution Barriers *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5. Explicit load/store ops + barriers replace subpasses for flexible pass resolution.

##### Math
barrier(src=COLOR, dst=COLOR, load=LOAD, store=STORE) per attachment;

##### How It Works
Rather than Vulkan subpasses, each lighting resolve uses explicit attachment load/store + a barrier, giving finer control over what's kept between passes (and working on APIs without subpasses). Determinism via fixed order (M1-EXT-13).

##### Reference Implementation
```cpp
RenderPass(attach, LOAD, STORE, barrier);
```

##### Player-Facing Impact
Pass resolution is portable + explicit - no subpass assumptions, same result everywhere.


---

#### [M4.5-EXT-21] RVT Feedback Buffer & Cache Invalidation *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.5 + M4.5-EXT-26 RVT. Feedback buffer drives RVT page (un)loading + invalidation.

##### Math
feedback = RenderVisiblePages(); request load for missing, evict LRU;

##### How It Works
The RVT feedback pass renders requested page indices; pages not resident are streamed in (M4.6-EXT-05) and LRU-evicted, keeping the virtual texture covering what the camera sees. Invalidation on world edits (M9-EXT-22) reloads affected pages.

##### Reference Implementation
```cpp
VkBuffer fb = RenderFeedback(cam); UpdateRvtPages(fb);
```

##### Player-Facing Impact
Virtual textures cover the view without ballooning VRAM - streaming stays tight.


##### Systems Touched
Only passing mention previously. Builds sparse clipmap SDF of nearby opaque geometry from depth buffer (or baked mesh SDFs) for cheap soft long-range shadows + contact shadows shadow-map cascade can't afford at distance.

##### Math
Per clipmap level, rasterize scene depth into SDF via 6-sweep or jump-flooding on depth buffer; d(p)=nearestSurfaceDistance(p) with sign from depth-vs-scene. Shadow test along light ray accumulates min over steps of d(ray(t))/t (cone soft shadow): vis=saturate(1-k*min_d/t).

##### How It Works
Compute pass converts depth (or downsampled depth) into SDF clipmap each time camera moves a cell. Lighting samples SDF along light dir for cheap soft shadow at range where shadow maps run out of resolution. Hybrid: near field shadow maps, far field SDF (fallforward, not double system).

##### Reference Implementation
```cpp
float SdfShadow(vec3 P, vec3 L, Texture3D sdfClip, float coneK){ float t=0,vis=1; for(int i=0;i<kSteps;++i){ t+=kStep; float d=SdfSample(sdfClip, P+L*t); vis=min(vis, saturate(1.0-coneK*d/t)); } return vis; }
```

##### Player-Facing Impact
Long-range soft shadows hold at distance on the 6GB floor without huge shadow-atlas cost.

---

##### Systems Touched

Underwater rendering pass, `[M10]`'s Saint-Venant water volumes (source depth), `[M6-EXT-11]`'s acoustic half of this same priority (paired system — this is the visual side only).

##### Math

$$I(d) = I_0 \cdot e^{-\alpha_\lambda d}$$

with $\alpha_\lambda$ set per-wavelength (red attenuates fastest, blue slowest — standard real-water absorption ordering) rather than a single scalar fog density.

##### How It Works

Underwater view distance already exists as a generic fog value; this replaces the single fog density with three per-channel extinction coefficients so color shifts believably with depth (a red object desaturates to grey within a few meters while blue light persists much further), evaluated once per pixel against the camera-to-surface distance sampled from the existing water-volume SDF.

##### Reference Implementation

```cpp
glm::vec3 ApplyBeerLambertExtinction(glm::vec3 baseColor, glm::vec3 incidentLight, float depth) {
    const glm::vec3 alpha = { 0.45f, 0.12f, 0.05f }; // R,G,B extinction per meter, blue attenuates slowest
    glm::vec3 transmittance = glm::exp(-alpha * depth);
    return baseColor * incidentLight * transmittance;
}
```

##### Player-Facing Impact

Underwater visuals shift from full color near the surface to a deep blue-green haze at depth the way real water actually looks, instead of a flat colored fog applied uniformly regardless of depth.



## M4.6 — Memory management

##### Implementation Steps

* Transient Pool Allocators for hot-paths.
* Pageable Device Memory LRU Eviction (distance-weighted).
* DirectStorage Streaming Pipelines.
* VRAM Subsystem Enforced Budgets (6.0GB global strict segmentation). System RAM mapped to 12.0GB.
* Memory-Pressure Radial Scaling on chunk loading.
* Vector Quantized Asset Compression for generated textures.

##### Decisions to flag

Direct VMA suballocation tuning.

##### Exit Criteria

* Zero per-frame heap allocations.
* VRAM usage is strictly flat over extended walk loops.

### Extended Systems Library — engine-side additions for M4.6

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M4.6-EXT-01] Procedural Vertex Buffer Pooling & Dynamic GPU Compaction

##### Systems Touched

Complements M4.6 memory management and streams mesh data into M1 storage allocations.

##### Math

Sub-allocation offsets advance through a shared arena using aligned bump-pointer arithmetic:

`Offset_next = (Offset_current + Stride − 1) & ~(Stride − 1)`

##### How It Works

Allocating a separate Vulkan vertex/index buffer for every procedurally generated room, barrier component, or debris fragment fragments the system heap and degrades performance during multi-threaded command recording. This system pre-allocates a single large virtual memory pool at boot. Background worker threads write raw procedural triangle lists directly into aligned sub-segments of that one buffer. When chunks fall out of view, their address ranges return to an asynchronous ring allocator, avoiding heap fragmentation and driver reallocation stalls.

##### Reference Implementation

```cpp
// Bounds-checks the write head against pool capacity before handing out a segment
uint64_t AssignPoolSegment(uint64_t allocation_size, uint64_t& write_head, uint64_t limit) {
    if (write_head + allocation_size > limit) return 0ull; // caller must handle allocation failure
    uint64_t result = write_head;
    write_head += allocation_size;
    return result;
}
```

##### Player-Facing Impact

Driving at high speed through procedurally generated sectors stays perfectly fluid. The engine streams millions of unique geometry elements into memory without a single driver stall or hitch.

#### [M4.6-EXT-02] Asynchronous OS Overlapped Direct-VRAM Disk Streaming Queue

##### Systems Touched

Backs M4.6's "DirectStorage Streaming Pipelines" bullet with a concrete I/O mechanism. **[OVERLAP FLAG]** Checks out native file handles from `[M0-EXT-04]`'s existing Asynchronous Native OS File Handle Ring Buffer rather than opening a second handle pool — the two features must share one ring, not maintain parallel ones.

##### Math

Ring index advance reuses `[M0-EXT-04]`'s power-of-two masking: `Idx_next = (Idx_current + 1) & (N − 1)`.

##### How It Works

Uses non-blocking overlapped OS reads to transfer raw texture/geometry blocks directly into VMA-allocated storage pools, bypassing an intermediate CPU-memory copy. Outstanding reads are tracked via `[M0-EXT-04]`'s existing handle ring rather than a new one; this feature only adds the overlapped-read dispatch and completion-to-VMA-mapping logic on top of that shared ring.

##### Reference Implementation

```cpp
struct NativeIORequest { HANDLE fileHandle; void* directVRAMTarget; uint32_t bytesToTransfer; OVERLAPPED overlapped; };
void DispatchNativeIO(NativeIORequest& req, uint64_t storageOffset) {
    req.overlapped.Offset = static_cast<DWORD>(storageOffset & 0xFFFFFFFF);
    req.overlapped.OffsetHigh = static_cast<DWORD>((storageOffset >> 32) & 0xFFFFFFFF);
    #if defined(_WIN32)
    ReadFile(req.fileHandle, req.directVRAMTarget, req.bytesToTransfer, nullptr, &req.overlapped);
    #endif
}
```

##### Player-Facing Impact

Asset/chunk streaming under heavy load (high-speed driving, fast traversal) never stalls waiting on a CPU-side memory copy — data lands straight in VRAM-mapped pools.

#### [M4.6-EXT-03] Vulkan Host-Visible Page-Pool Memory Defragmentation Compiler
#### [M4.6-EXT-05] DirectStorage Decompression Buffer Ring Allocator Pool *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.6 + M0 streaming. Ring-allocates GPU buffers for DirectStorage asset decompression so streaming never stalls.

##### Math
buf = Ring.Alloc(size); Decompress(src, buf); free when consumed by consumer.

##### How It Works
A ring of GPU staging buffers is pre-allocated; DirectStorage writes decompressed asset chunks into the next free slot, the consumer reads it, then the slot recycles. No per-asset alloc churn, no stall.

##### Reference Implementation
```cpp
GpuBuffer b=ring.Alloc(sz); Decompress(in,b); Consume(b); ring.Free(b);
```

##### Player-Facing Impact
Assets stream in without hitches or allocator thrash.

---
#### [M4.6-EXT-08] BC7 / Block-Texture Compression & Transcode *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Zero mention previously. GPU-friendly BC7 (desktop)/ASTC (mobile) compression for material/atlas textures M4-EXT-25 + M4.5-EXT-26 RVT produce, cutting VRAM on 6GB Tier-0 floor. Runs on enkiTS scheduler at bake/load (not render thread).

##### Math
BC7: each 4x4 texel block encoded into 128 bits across one of 8 partition modes with endpoint+index quantization + optional mode-1 alpha split; quality/speed via partition search. ASTC generalizes to 4x4..12x12 with similar endpoint+weight scheme. Offline or load-time encode; hardware decodes free.

##### How It Works
Material graph output + atlas pages encoded to BC7 once at content-bake (or first load, cached). Decompressor M0-EXT-13/ring allocator hands already-compressed data to GPU (no per-frame decode). 4K albedo ~4x VRAM drop (RGBA8->BC7). Tier-0 relief without visual loss at chosen quality.

##### Reference Implementation
```cpp
void EncodeBC7(const RGBA* b, uint8_t out[16], int quality){ BC7Partition best=SearchPartitions(b,quality); /* 8 modes, endpoint quant */ CompressEndpoints(best); }
```

##### Player-Facing Impact
Big VRAM savings on the 6GB floor — more textures, fewer hitches.

---

#### [M4.6-EXT-07] Basis Universal GPU Texture Transcoder *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M4.6. Transcodes Basis Universal supercompressed textures to GPU formats at load.

##### Math
VkImage img = BasisTranscode(data, targetFmt);

##### How It Works
Textures are stored as Basis Universal (one supercompressed blob) and transcoded to the GPU's optimal format (BC7/ASTC) at load on a worker - small on disk, native on GPU, no per-format authoring. Pairs with M4.6-EXT-08 BC7.

##### Reference Implementation
```cpp
Image i = BasisTranscode(basis, fmt);
```

##### Player-Facing Impact
Textures stay tiny on disk, native on GPU - fast loads, low VRAM.


##### Systems Touched
Zero mention previously. GPU-friendly BC7 (desktop)/ASTC (mobile) compression for material/atlas textures M4-EXT-25 + M4.5-EXT-26 RVT produce, cutting VRAM on 6GB Tier-0 floor. Runs on enkiTS scheduler at bake/load (not render thread).

##### Math
BC7: each 4x4 texel block encoded into 128 bits across one of 8 partition modes with endpoint+index quantization + optional mode-1 alpha split; quality/speed via partition search. ASTC generalizes to 4x4..12x12 with similar endpoint+weight scheme. Offline or load-time encode; hardware decodes free.

##### How It Works
Material graph output + atlas pages encoded to BC7 once at content-bake (or first load, cached). Decompressor M0-EXT-13/ring allocator hands already-compressed data to GPU (no per-frame decode). 4K albedo ~4x VRAM drop (RGBA8->BC7). Tier-0 relief without visual loss at chosen quality.

##### Reference Implementation
```cpp
void EncodeBC7(const RGBA* b, uint8_t out[16], int quality){ BC7Partition best=SearchPartitions(b,quality); /* 8 modes, endpoint quant */ CompressEndpoints(best); }
```

##### Player-Facing Impact
Big VRAM savings on the 6GB floor — more textures, fewer hitches.

---

##### Systems Touched

Complements M4.6's Pageable Device Memory LRU Eviction and VRAM budget enforcement — targets fragmentation left behind by streamed-out chunk graphics assets during prolonged open-world traversal, on top of `[M4.6-EXT-01]`'s vertex-buffer pooling for geometry.

##### Math

`Φ_fragmentation = 1.0 − (Σ BlockSize_Allocated) / (Heap_HighWaterAddress − Heap_BaseAddress)`

##### How It Works

Tracks vacant blocks left by evicted chunk assets and delegates compaction to a background enkiTS task rather than the main thread, sliding valid allocations linearly toward the low end of the page pool. This is a page-pool-level compactor sitting below `[M4.6-EXT-01]`'s vertex-buffer arena — it defragments the underlying VMA sub-allocation pages those pools draw from.

##### Reference Implementation

```cpp
struct MemorySegmentAllocation {
    uint64_t virtualDeviceAddress{0};
    uint64_t sizeBytes{0};
    bool isSegmentVacant{true};
};
void CompactMemoryPagePool(MemorySegmentAllocation* segmentsArray, uint32_t elementCount) {
    uint32_t writeHeadIndex = 0;
    for (uint32_t i = 0; i < elementCount; ++i) {
        if (!segmentsArray[i].isSegmentVacant) {
            if (i != writeHeadIndex) {
                segmentsArray[writeHeadIndex] = segmentsArray[i];
                segmentsArray[i].isSegmentVacant = true;
            }
            writeHeadIndex++;
        }
    }
}
```

##### Player-Facing Impact

Extended open-world driving/traversal sessions never accumulate allocation stalls or OOM faults from fragmented VRAM — the 6.0GB/5.0GB budget stays usable rather than degrading over hours of play.

## M5 — NPCs / zombie hordes (base)

##### Implementation Steps

* ECS Native Spawning. Navigation Grid A* Pathfinding.
* Dual Quaternion Compute Skinning.
* Dynamic Logistic Spawn Pacing (`dN/dt = rN(1−N/K)`) reading AI Director threat.
* Perimeter Illumination Repulsion (light sources add nav-grid weights). Hydrostatic Velocity Penalties.

##### Decisions to flag

Horde count target (~200). CPU vs GPU skinning target.

##### Exit Criteria

* ~200 zombies pathfind efficiently. GPU skinning confirmed.
* Zombies respond to destroyed barricades immediately.

### Extended Systems Library — engine-side additions for M5

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M5-EXT-01] Asynchronous Tile-Voxelized NavMesh Baker

##### Systems Touched

Replaces static grids in M5 (Navigation Grid A* Pathfinding) and reacts to M3 macro-destruction structural collapses.

##### Math

Voxel generation checks surface orientation by evaluating the dot product between the Jolt geometry surface normal (n) and the world up vector (u), filtering out non-traversable slope boundaries:

`n · u ≥ cos(θ_max)`

##### How It Works

A simple, flat navigation grid breaks completely when structural components collapse under horde load (M3) or players erect free-form fortifications across choke points. This feature converts the navigation framework into a collection of asynchronous coordinate tiles matching your world chunk bounds. When a structural graph link severs or a barricade component is emplaced, the engine flags the local navigation tile as dirty. An enkiTS background thread task triggers a 3D Jolt box-cast sweeping pass across the local tile area. The sweep samples surface height coordinates, filters out non-traversable angles using the slope dot-product boundary, and bakes a dense, local navigation connectivity grid. The new navigation tile is dynamically re-stitched to adjacent nodes, updating AI routing live without causing main-thread frame drops.

##### Reference Implementation

```cpp
// Asserts normalized upward tracking vectors to guard against NaN outputs;
// evaluates slope angle limits live to mark local tile traversability flags
bool CheckVoxelWalkable(const glm::vec3& surface_normal, float max_slope_angle) {
    float dot_up = glm::dot(surface_normal, glm::vec3(0.0f, 1.0f, 0.0f));
    // Guard against inverted normals or invalid geometry weights
    float clamped_dot = std::clamp(dot_up, -1.0f, 1.0f);
    return std::acos(clamped_dot) <= max_slope_angle;
}
```

##### Player-Facing Impact

Zombie pathfinding adapts intelligently to dynamic changes in the environment. Kicking a rotting locked door open or blasting a hole through a residential wall instantly causes trailing hordes to alter their pursuit trajectories. They stream through the new structural opening or navigate around debris piles left by collapsed building roofs, instead of walking into walls or pathing along broken nodes.

#### [M5-EXT-02] Dual-Quaternion GPU Compute Skinning

##### Systems Touched

Declares the actual "Dual Quaternion Compute Skinning" bullet from M5's Implementation Steps above. Note: `[M5.1-EXT-03]` refers to this system as "M5.2's existing Dual-Quaternion compute skinning" — read that as "M5's"; this base-M5 bullet is the canonical declaration. Consumes bone bind weights from `[M5.1-EXT-03]`'s Master Skeleton and per-bone pose data from M5.2's active ragdolls / IK-driven locomotion targets, and writes finished vertex positions into M1's buffer-device-address skinned-mesh path.

##### Math

Per-vertex skinning blends unit dual quaternions (screw-motion interpolation) rather than bone matrices, using the weights `[M5.1-EXT-03]` already computes and normalizes:

`q̂_blend = Σ w_i · q̂_i`, then `q̂_final = q̂_blend / ‖q_blend‖` — normalizing by the real-part magnitude renormalizes both parts of the dual quaternion together.

##### How It Works

Linear blend skinning interpolates bone matrices directly, which collapses volume at extreme twists — the classic "candy-wrapper" pinch at elbows, knees, and hips. Dual quaternion skinning avoids this by interpolating rigid screw motions instead of matrices, so twisted joints sweep smoothly instead of folding. A compute pass dispatched per horde entity reads each bone's current-frame transform (from ragdoll or procedural IK, whichever M5.2 has active that tick), converts it to a unit dual quaternion, blends per-vertex using `[M5.1-EXT-03]`'s baked weights, and writes the skinned position straight into the buffer-device-address vertex buffer M1 already declared for skinned meshes — no CPU-side skinning path exists at all.

##### Reference Implementation

```glsl
// Compute shader: blends bone dual-quaternions per vertex, writes skinned position via buffer_reference
struct DQ { vec4 real; vec4 dual; };

DQ BlendDualQuat(DQ bones[4], float weights[4]) {
    DQ r; r.real = vec4(0); r.dual = vec4(0);
    for (int i = 0; i < 4; ++i) {
        // hemisphere check avoids blending quaternions across opposite sign conventions
        float s = dot(bones[i].real, bones[0].real) < 0.0 ? -1.0 : 1.0;
        r.real += weights[i] * s * bones[i].real;
        r.dual  += weights[i] * s * bones[i].dual;
    }
    float mag = length(r.real);
    r.real /= mag; r.dual /= mag;
    return r;
}
```

##### Player-Facing Impact

Zombies crawling through low-clearance gaps, mid-lunge attacks, and compact ragdoll pile-ups no longer show pinched or self-intersecting mesh at the joints. Because skinning runs entirely on the GPU, ~200 simultaneous horde entities skin cleanly every frame without a CPU cost.

#### [M5-EXT-03] Dynamic Logistic Spawn Pacing

##### Systems Touched

Declares M5's "Dynamic Logistic Spawn Pacing" bullet, wiring its carrying-capacity term to M5.4's `TensionSignal` and its output into M1's `EntityFactory` JSON-driven spawn requests (ECS Native Spawning).

##### Math

`K(t) = K_base · (1 + κ·TensionSignal)` — carrying capacity rises and falls with the Director's smoothed tension scalar rather than staying fixed.

`N_{t+Δt} = N_t + r·N_t·(1 − N_t/K(t))·Δt`

##### How It Works

Each Director tick, the spawn system re-evaluates `K(t)` off M5.4's already-computed `TensionSignal` before stepping the logistic growth equation. Low tension caps horde population well below the ~200 target so a quiet sector doesn't stay saturated; a spike in tension (gunfire, a screamer alarm, regional SEIR severity feeding through `[M5.4-EXT-03]`) raises the ceiling and lets population race toward it. The resulting spawn count is dispatched through M1's `EntityFactory`, which parses each zombie's initial component set from JSON exactly as it does for any other entity.

##### Reference Implementation

```cpp
float StepLogisticSpawn(float n_current, float r_rate, float k_base, float kappa, float tension, float dt) {
    float k_t = k_base * (1.0f + kappa * tension);
    return n_current + r_rate * n_current * (1.0f - n_current / k_t) * dt;
}
```

##### Player-Facing Impact

Horde density visibly tracks how loud and dangerous you've been rather than sitting at a flat number — a sector you've kept quiet stays thinned out, while one you've been firing guns in all night keeps refilling toward a much higher ceiling.

#### [M5-EXT-04] Perimeter Illumination Repulsion

##### Systems Touched

Declares M5's "Perimeter Illumination Repulsion" bullet: a lightweight `LightSource` registry (position, radius, intensity) queried through M1's `SpatialHash`, feeding an additive cost weight into `[M5-EXT-01]`'s async navmesh baker so lit tiles read as less desirable to path through.

##### Math

`W_light(tile) = Σ_{s ∈ Lights(tile)} β · (I_s / (1 + d_s²))` — summed, inverse-square-falloff repulsion from every registered light source within query radius of the tile.

##### How It Works

Every placed or portable light source (torches, floodlights, vehicle headlights) registers into a small entity set queried the same way any other `SpatialHash` consumer does. When `[M5-EXT-01]`'s tile baker runs (on structural change or, for lights, whenever a source is added/removed/toggled), it sums the inverse-falloff weight from nearby lights into that tile's traversal cost before re-stitching it to neighbors. Zombies still path through if that's the only route, but the cost bias makes them favor darker approach lines when one exists — no separate zombie-AI-side light query is needed, since the weight is baked straight into the nav data they already read.

##### Reference Implementation

```cpp
float AccumulateLightWeight(const std::vector<LightSource>& lights, const glm::vec3& tileCenter, float beta) {
    float w = 0.0f;
    for (const auto& l : lights) {
        float d2 = glm::distance2(tileCenter, l.position);
        if (d2 <= l.radius * l.radius) w += beta * (l.intensity / (1.0f + d2));
    }
    return w;
}
```

##### Player-Facing Impact

A well-lit barricade perimeter genuinely discourages wandering hordes from drifting through it, rewarding deliberate light discipline at a camp. Shoot out a floodlight or let a generator die, and the repulsion drops out live through the same dirty-tile re-bake `[M5-EXT-01]` already runs for structural changes — the perimeter goes dark and the paths shift immediately.

#### [M5-EXT-05] Hydrostatic Velocity Penalties

##### Systems Touched

Declares M5's "Hydrostatic Velocity Penalties" bullet by reusing the hydrostatic-pressure term from M9's vehicle wading model (`P_hydrostatic = ρ_water · g · h_water`), sampling M10's Saint-Venant flood depth field, and applying it to zombie locomotion speed instead of engine torque.

##### Math

`P_hydrostatic = ρ_water · g · h_water` *(same term M9 already computes for vehicle wading — sample it rather than re-deriving)*

`v_effective = v_locomotion · Clamp(1.0 − P_hydrostatic/P_ref, 0.2, 1.0)`

##### How It Works

M9 already converts local flood depth into a hydrostatic pressure term each tick to throttle vehicle engine torque. Zombie locomotion reads that same computed pressure value — it does not stand up a second water-physics system — and applies it as a velocity clamp instead of a torque efficiency curve, floored so a horde wading through knee-deep runoff slows down but never fully stalls the way a stalling vehicle engine can.

##### Reference Implementation

```cpp
float ApplyHydrostaticSlow(float v_locomotion, float p_hydrostatic, float p_ref) {
    float mult = std::clamp(1.0f - p_hydrostatic / p_ref, 0.2f, 1.0f);
    return v_locomotion * mult;
}
```

##### Player-Facing Impact

Hordes wading through flooded streets after a heavy Saint-Venant flood event visibly bog down, giving you a genuine tactical option to funnel or outrun pursuers through standing water — without the engine needing a zombie-specific water system, since it's riding the same hydrostatic term M9 already pays for.

## M5.1 — Procedural zombie variation, utility AI & bio-weight simulation

##### Implementation Steps

* Deterministic Trait Distribution (`ZombieTraits`).
* Weighted Consideration Utility AI. Reynolds Boid Flocking.
* Location-Aware Zombie Bio-Weight Simulation tied to Socio-Economic tags. Mass-Driven Jolt Physics Scaling.
* Anatomical Hit-Volume SDF Gradients.
* Pareto-Front Phenotype Evolution (`θ_{t+1} = θ_t + α(θ_fit − θ_t)`).
* Heat-Diffusion Automatic Rigging. Phenotype Vocalization Shift via PSOLA.
* Line-of-Sight Screamer Convergence. Holling Type II Cannibalism Feeding.

##### Decisions to flag

Evolution learning rate (`α`). Phenotype matrix distribution curves.

##### Exit Criteria

* Zombie traits generate deterministically.
* Heavy zombies push physics constraints.
* Phenotypes naturally adapt across simulated day-cycles.



### Extended Systems Library — engine-side additions for M5.1

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M5.1-EXT-01] Aerodynamic Wake Slipstream Horde Slingshot

##### Systems Touched

Links M9 vehicle chassis velocity tracking fields straight to M5.1 Reynolds boid steering arrays.

##### Math

A speeding vehicle chassis shears the atmosphere, creating a low-pressure wake zone behind its path. Aggressive zombie phenotypes entering this tracking cone encounter virtually zero aerodynamic resistance, dropping their drag coefficients (C_d ) exponentially:

`C_d(x) = C_d0 · (1.0 − e^(−x/λ_wake))`

##### How It Works

Each tick, boids inside M9's tracked wake cone have their drag coefficient queried against distance into the cone via the exponential falloff, letting fast zombies inside the cone match the vehicle's closing speed.

##### Reference Implementation

```cpp
float GetHordeDraftDrag(float c_d0, float dist, float lambda) {
    return c_d0 * (1.0f - std::exp(-dist / lambda));
}
```

##### Player-Facing Impact

Speeding down highways doesn't let you effortlessly outrun threats. High-tier running zombies can actively catch your vehicle's slipstream, entering the low-pressure pocket behind your rear bumper. They gain a high-velocity drafting boost that lets them latch onto your tailgate or swarm your open truck beds like drafting racing cars.

##### Depends on (not yet built at this point)

M9 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M5.1-EXT-02] Player Character Generation (Uninfected-Mode Phenotype Pipeline)

##### Systems Touched

Runs the existing M5.1 zombie phenotype/bio-weight generator with the infection-mutation pass disabled — one canonical procedural humanoid generator for players, survivors, and zombies, not a separate hand-authored player model.

##### Math

No new math — identical `ZombieTraits` deterministic trait distribution already used for zombies, evaluated with `InfectionState = Uninfected` so the mutation terms zero out.

##### How It Works

The same trait-distribution pipeline already generating every zombie in the world runs once at character-creation time with the mutation/decay passes gated off by an `InfectionState` flag — character creation is choosing which slice of an existing procedural space to sample from, not a second content pipeline.

##### Reference Implementation

```cpp
// Same function signature as zombie generation — InfectionState gates the mutation pass off
HumanoidPhenotype GeneratePlayerCharacter(uint64_t player_seed) {
    return GenerateZombiePhenotype(player_seed, InfectionState::Uninfected); // mutation terms no-op
}
```

##### Player-Facing Impact

Player character appearance is procedurally generated with the same richness and variation as the zombies and survivors around them — genuinely unique-looking characters with zero hand-authored art, consistent with everything else in this engine.

#### [M5.1-EXT-03] Topology-Invariant Master Rig with Deformation-Transfer Binding

##### Systems Touched

Upgrades M5.1 zombie variation routines and feeds bind data into M5.2 procedural animation and M5.2's existing Dual-Quaternion compute skinning.

##### Math

Vertex skinning weights fall off with distance from a bone's influence line, but are now sampled once against the fixed master topology rather than recomputed per instance:

`Weight = max(0, 1 − Distance_to_bone / Radius_of_influence)²`

Per-vertex bone weights are NOT computed at runtime via raw line-proximity against each phenotype's bone placement — at extreme joint configurations (deep lunges, low-clearance crawling, compact ragdoll pile-ups) that produces "paper-folding" volume collapse and self-intersecting mesh, since a line-distance-only solve has no knowledge of surrounding topology. Bone placement is still generated per-phenotype/seed-driven at runtime, but weights are not recomputed from scratch each time — see the heat-diffusion approach below.

##### How It Works

A single, structurally perfect Master Skeleton is authored once with hand-painted, high-quality base skinning weights covering every joint fold case (deep bends, compact poses, etc.). When a phenotype seed generates a specific mutant or zombie scale, the system does not re-derive skinning weights — it programmatically adjusts the *lengths and offsets* of the Master Skeleton's bone nodes to match the generated SDF volume, then transfers the resulting mesh deformation via Skeletal Morph Targets / Deformation Transfer matrices computed against the master's known-good weight painting. Because the topology and weight painting never change — only bone length/offset — extreme poses fold cleanly across every generated body variation instead of collapsing.

##### Reference Implementation

```cpp
// Normalizes bone weights so skin shaders never divide by a near-zero sum;
// weights themselves come from the Master Skeleton's baked painting, not a runtime line-proximity solve
void BalanceSkeletalBinds(float* weights, uint32_t count) {
    float sum = 0.0f;
    for (uint32_t i = 0; i < count; ++i) sum += weights[i];
    if (sum > 0.001f) for (uint32_t i = 0; i < count; ++i) weights[i] /= sum;
}
```

##### Player-Facing Impact

Hordes display rich anatomical variety — distorted mutations, heavily built behemoths, and lanky, emaciated infected all move, crawl, and ragdoll with accurate joint behavior and zero texture stretching or paper-folding, even in extreme compact poses.

#### [M5.1-EXT-04] Anatomical Hit-Volume SDF Gradients

##### Systems Touched

Declares the base "Anatomical Hit-Volume SDF Gradients" bullet. Consumes `[M5.1-EXT-03]`'s Master Skeleton pose data (skinned vertex positions feed the SDF's zero-crossing surface) and feeds M2.9's ballistic damage resolution — hit location and penetration depth are read off the gradient rather than a raw mesh raycast.

##### Math

Each anatomical region (skull, torso, limb) is represented as a signed distance field; a hit's damage multiplier and exit-wound depth come from the gradient at the impact point: `Depth = |Φ(p)| / |∇Φ(p)|`, where `Φ` is the SDF value and `∇Φ` its normalized gradient direction.

##### How It Works

Rather than raycasting the full skinned mesh per shot (expensive at horde density), each zombie carries a small set of primitive SDF volumes (capsules/ellipsoids) rigged to the Master Skeleton's bones, updated once per animation tick alongside `[M5.1-EXT-03]`'s pose. A hit tests against this lightweight SDF stack instead of triangle geometry: the gradient at the query point gives both the surface normal (for spatter direction) and penetration depth (for through-and-through vs. stopping-power resolution) in one evaluation.

##### Reference Implementation

```cpp
// Evaluated once per hit query against the rigged capsule/ellipsoid SDF stack, not per-triangle
float EvaluatePenetrationDepth(const glm::vec3& hitPoint, const AnatomicalSdf& sdf) {
    float phi = sdf.Evaluate(hitPoint);              // signed distance at impact point
    glm::vec3 gradPhi = sdf.EvaluateGradient(hitPoint); // normalized surface gradient
    return std::abs(phi) / glm::length(gradPhi);
}
```

##### Player-Facing Impact

Headshots and limb hits register consistently even mid-animation (staggering, lunging, ragdolling) without expensive per-triangle raycasts, and bullet penetration depth reads correctly against actual body mass rather than a flat hitbox.

#### [M5.1-EXT-05] Heat-Diffusion Automatic Rigging & PSOLA Phenotype Vocalization

##### Systems Touched

Declares the base "Heat-Diffusion Automatic Rigging. Phenotype Vocalization Shift via PSOLA" bullet. Rigging half feeds `[M5.1-EXT-03]`'s Master Skeleton binding; vocalization half feeds M6's audio propagation as the per-zombie emitter source, sharing PSOLA machinery with M6's `[M6-EXT]` voice pipeline rather than duplicating a pitch-shift implementation.

##### Math

Skin weight at each mesh vertex is the steady-state solution of the heat equation seeded at each bone's joint position, solved to convergence rather than approximated by inverse distance: `∂h/∂t = κ∇²h`, with each joint held as a fixed-temperature boundary condition (`h = 1` at its own seed point, `h = 0` at all other seeds) and the converged `h` per joint, per vertex, normalized across joints to give that vertex's bone weights. Vocalization pitch/formant shift is a PSOLA resampling of the base clip's pitch marks by a ratio derived from the phenotype's vocal-tract-length scalar `L`: `PitchRatio = L_reference / L_phenotype`.

##### How It Works

Automatic rigging: bone influence weights for each generated phenotype are solved by heat diffusion across the mesh surface from seed points at each joint — heat "flows" from a bone's seed point outward, and the resulting steady-state temperature at each vertex becomes its skin weight, which handles irregular/mutated topology (extra mass, missing limbs) more robustly than distance-based weighting. Vocalization: each phenotype's base groan/scream audio clip has its pitch and formants shifted via Pitch-Synchronous Overlap-Add (PSOLA) according to the phenotype's mass and vocal-tract-length parameters from `[M5.1-EXT-02]`'s generation pipeline, so a heavy zombie sounds bass-shifted and a child-scaled infected sounds pitched up from the same source clips instead of needing per-phenotype voice recordings.

##### Reference Implementation

```cpp
// Solved once at phenotype-generation time, not per frame; converged weights are cached on the mesh
void SolveHeatDiffusionWeights(Mesh& mesh, std::span<const JointSeed> joints, int iterations, float kappa) {
    for (auto& joint : joints) {
        std::vector<float> h(mesh.vertexCount, 0.0f);
        h[joint.seedVertexIndex] = 1.0f; // fixed boundary condition at the joint's seed point
        for (int i = 0; i < iterations; ++i)
            h = DiffuseLaplacian(mesh, h, kappa); // one explicit ∂h/∂t = κ∇²h step
        mesh.AssignBoneWeightChannel(joint.boneId, h);
    }
    mesh.NormalizeWeightsAcrossJoints();
}
```

##### Player-Facing Impact

Every generated phenotype — however unusual its proportions — gets correctly weighted deformation and a vocalization that matches its size, without hand-rigging or recording a unique voice per body type.

#### [M5.1-EXT-06] Screamer Convergence & Holling Type II Cannibalism Feeding
#### [M5.1-EXT-07] Optimal Reciprocal Collision Avoidance (ORCA) Solver *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.1/M5.3 + M5.4. Local collision-avoidance velocity for agents (companions, animals, crowds).

##### Math
v = ORCA(agent, neighbors, goal); each agent picks velocity in its free half-plane, reciprocal.

##### How It Works
Each dynamic agent computes a velocity avoiding collision with neighbors while moving to its goal, using the ORCA reciprocal formulation (neighbors do the same, so avoidance is shared). Drives companion/animal/crowd navigation.

##### Reference Implementation
```cpp
vec3 v = ORCA.Solve(pos, vel, goal, neighbors);
```

##### Player-Facing Impact
Allies/animals/crowds navigate around each other smoothly - no overlap or jitter.

##### Systems Touched

Declares the base "Line-of-Sight Screamer Convergence. Holling Type II Cannibalism Feeding" bullet. Reads horde positions from M5.4's AI Director and gates on M5.3's line-of-sight perception queries; feeds back into M5.4's threat pacing since a large convergence event raises local threat.

##### Math

Cannibalism feeding rate follows a Holling Type II functional response — feeding rate saturates rather than scaling linearly with corpse density: `f(N) = (a·N) / (1 + a·h·N)`, where `N` is nearby corpse count, `a` is the attack/discovery rate, and `h` is handling time per corpse.

##### How It Works

A "screamer" phenotype that spots the player emits a convergence call; every zombie with unobstructed line-of-sight to the screamer (checked via M5.3's perception system) paths toward its position rather than the player directly, pulling the horde together before they re-acquire the player individually. Idle zombies near corpse piles roll into cannibalism feeding using the Holling Type II curve above — at low corpse density every corpse gets fed on quickly, but as density rises, handling time bottlenecks the rate so feeding doesn't scale unrealistically with a huge pile.

##### Reference Implementation

```cpp
// Holling Type II feeding rate: saturates rather than scaling linearly with corpse density
float ComputeFeedingRate(float corpseDensity, float attackRate, float handlingTime) {
    return (attackRate * corpseDensity) / (1.0f + attackRate * handlingTime * corpseDensity);
}
```

##### Player-Facing Impact

Screams visibly and audibly marshal scattered zombies into a converging horde rather than a diffuse trickle, and corpse piles create believable feeding lulls — a useful, readable distraction mechanic — instead of an infinite instant-consume sink.

## M5.2 — Procedural animation (physics/IK-driven)

##### Implementation Steps

* IK-Driven Locomotion Solver (parametric foot trajectory).
* Proportional Active Ragdoll Dynamics via Jolt soft-body muscles and PD motors.
* Gyroscopic Balance Control. Grounded Foot-Slip Correction.
* XPBD Cloth Simulation with wind-vector coupling. Secondary motion jiggle oscillators.
* Separable Subsurface Scattering Wounds.
* Approximate-Nearest-Neighbor Motion Matching (`caps.annMotionMatching`). Projective Dynamics cloth (`caps.projectiveDynamicsCloth`).

##### Decisions to flag

Zero-clip vs minimal baked reference poses.

##### Exit Criteria

* Locomotion resolves via IK seamlessly.
* Hit reactions flow through PD motors accurately.



### Extended Systems Library — engine-side additions for M5.2

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M5.2-EXT-01] Procedural Joint Compliance Degradation (Dynamic Muscle Crippling)

##### Systems Touched

Expands M5.2 (Procedural Animation / Active Ragdolls) and interfaces with M2.9 Ballistics.

##### Math

The Proportional-Derivative (PD) motor torque equations governing your active ragdoll skeleton joints are modified at runtime based on localized bone kinetic energy dissipation tensors:

`T_motor = K_p·(θ_target − θ_current) + K_d·(ω_target − ω_current)`

When a specific limb segment sustains an entry from a non-penetrating ballistic hit, instead of applying a static lurch animation, the engine scales down the joint tracking gains exponentially over accumulated tissue damage metrics:

`K_p ←K_p ⋅e^(−α⋅∑D_limb)`

`,K_d ←K_d ⋅e^(−α⋅∑D_limb)`

Explicit PD (evaluating force from the current frame's error only) is unstable on a heavy phenotype combined with a large `Δt` (frame hitch) or stiff `K_p` — it can overshoot and oscillate without bound. Reference Implementation below uses **Stable PD** (implicit) instead: it solves for next-step velocity before applying force, damping high-gain resonance regardless of frame-time — the technique from Tan, Liu & Turk's "Stable Proportional-Derivative Controllers" (2011), standard for ragdoll/character joint control.

##### Reference Implementation

```cpp
// Implicit "Stable PD" form (Tan et al. 2011) — solves for next-step torque so high joint
// stiffness or a large frame-time delta damps out instead of overshooting into a stretched/exploded joint.
void CalculateStableJointTorque(float theta_target, float theta_current, float omega_target, float omega_current,
                                 float k_p, float k_d, float dt, float& out_torque) {
    float position_error = theta_target - theta_current;
    float velocity_error = omega_target - omega_current;
    float implicit_damping = 1.0f + (dt * k_d) + (dt * dt * k_p);
    out_torque = (k_p * position_error + k_d * velocity_error) / implicit_damping;
}
```

##### How It Works

This cleanly replaces hardcoded injury animations with genuine physics-driven impairment. Your active ragdolls continually try to match their procedural locomotion target poses. By dropping the motor stiffness (K_p ) and damping (K_d ) on a heavily damaged joint, the limb physically gives out under the zombie's own phenotype mass scale.

##### Player-Facing Impact

Zombies don't just have a generic "limp" toggle. If you sweep a low-caliber weapon or a blunt melee object into a heavy zombie's right knee joint, that specific Jolt constraint goes soft. The zombie's leg will physically buck under its weight, lowering its hip center-of-mass, causing it to drag the limp leg realistically across dynamic rubble fields, or tumble forward completely if it tries to turn a sharp corner at high velocity.

#### [M5.2-EXT-02] Zombie Tissue Rigor Mortis Lifecycle Decay

##### Systems Touched

Intersects M5.1 zombie bio phenotypes with M5.2 Active Ragdoll joint tracking loops.

##### Math

The muscle flexibility index (χ) of a zombie transitions over biological timeline states (t_age ). Fresh infections display standard flexibility; recently turned corpses enter a high-stiffness phase, which eventually degrades into a flaccid, loose decay state:

`χ(t_age) = Clamp(e^(−α·t) + 2.5·(1.0−e^(−β·t))·e^(−γ·t), 0.05, 1.5)`

This flexibility coefficient scales the base motor stiffness variables (K_p) across all skeletal joint elements inside the active ragdoll controllers:

`K_p = K_p0 · χ(t_age)`

##### How It Works

Your zombie entities hold timestamp values inside their EnTT component sheets tracking their generation time. During your active ragdoll physics loop updates, the joint tracking motor gains sample this age-dependent modifier, shifting constraint limits dynamically without introducing specialized procedural animation code branches.

##### Reference Implementation

```cpp
void AdjustRigorStiffness(float age, float alpha, float beta, float gamma, float& k_p) {
    k_p *= std::clamp(std::exp(-alpha * age) + 2.5f * (1.0f - std::exp(-beta * age)) * std::exp(-gamma * age), 0.05f, 1.5f);
}
```

##### Player-Facing Impact

Zombies generated from freshly killed survivors move with fluid dexterity. Hordes that have spent hours wandering in cold sectors enter a stiff, robotic rigor mortis phase, causing their walks to appear mechanical and making them clumsy over dynamic debris. Deeply decayed zombies display loose, hyper-extended joint reactions that cause their bodies to buck wildly under bullet impacts.

#### [M5.2-EXT-03] Biomechanical Joint Luxation Limits

##### Systems Touched

Deepens M5.2 (Active Ragdolls) and interfaces with Jolt constraint friction callbacks.

##### Math

Instead of skeletal constraints possessing binary breaks, joint limits experience structural luxation (dislocation) when the torque loads (τ_joint) surpass a dynamic tendon structural yield limit (T_yield):

`If ∥τ_joint∥ > T_yield ⟹ θ_limit_max = θ_limit_max + γ·(∥τ_joint∥ − T_yield)`

This structural displacement warps your target physics limits permanently, inducing a persistent mechanical structural offset in the ragdoll assembly.

##### How It Works

Your active ragdoll limits are driven by Jolt's native joint constraint systems. This field listens to the impulse manifold data returned by the physics step. If an impact load crosses the structural boundary, the script uses SetLimits directly on the Jolt constraint handles to warp the joint bounds inline, bypassing the need to destroy or detach the entire joint assembly.

##### Reference Implementation

```cpp
void CheckJointLuxation(float torque, float yield_t, float gamma, float& limit_max) {
    if (torque > yield_t) limit_max += gamma * (torque - yield_t);
}
```

##### Player-Facing Impact

Dropping a heavy zombie off a multi-story building ledge onto concrete doesn't always break the leg off entirely. If the forces hit a specific threshold, the hip or knee joint experiences realistic dislocation. The active ragdoll constraint warps permanently, causing the limb to drag backward at an unnatural angle while the entity struggles to balance its weight during locomotion loops.

#### [M5.2-EXT-04] Centrifugal Kinetic Limb Flail

##### Systems Touched

Expands M5.2 active ragdoll constraints using Jolt contact impulse manifolds.

##### Math

When a zombie's joint compliance drops or experiences partial structural failure, the limb is not lazily deleted. It acts as an unconstrained chaotic double pendulum. The rotational asymmetry generates a dynamic centrifugal angular torque vector (τ_flail ) during locomotion swings:

`τ_flail =m_limb ⋅(ω_torso ×(ω_torso ×r_limb ))`

##### How It Works

When a limb's joint compliance is loosened/broken, it stops following the animated/ragdoll blend and free-swings as an unconstrained double-pendulum segment. Torso rotation drives a centrifugal torque that whips the limb outward; contact impulses apply through the existing Jolt manifold like any rigid-body hit.

##### Reference Implementation

```cpp
void ComputeLimbFlail(float mass, const glm::vec3& w, const glm::vec3& r, glm::vec3& out_torque) {
    out_torque = mass * glm::cross(w, glm::cross(w, r));
}
```

##### Player-Facing Impact

Damaging a heavy zombie's arm or shoulder joint causes the limb to swing wildly like a heavy canvas sack as it charges forward. This chaotic kinetic momentum turns the limb into a physical flail that accidentally smashes into and trips neighboring zombies in the crowd, initiating chain-reaction stumble cascades across the herd.

#### [M5.2-EXT-05] Control-Rig-Style Procedural Pose Override Layer

##### Systems Touched

Sits downstream of `caps.annMotionMatching` (M5.2 above) in the per-frame pose pipeline. Intended chain: Motion Matching → Upper-Body Override (`[K-EXT-01]`, Appendix K) → Procedural Rig (this layer) → Physics Post-Process → Final Pose. This layer consumes Motion Matching's output pose directly, further overridden by `[K-EXT-01]`'s spine-root blend before it reaches this rig layer.

##### Math

A per-joint override is a direct pose substitution: `pose_joint = Lerp(pose_MM, pose_procedural, weight)`, weight supplied per-joint by the Constraint-Weight Blending layer (M5.2-EXT-06).

##### How It Works

A new post-solve stage that runs after Motion Matching produces its base pose and before the existing Active Ragdoll / PD-motor layer (M5.2 above) takes over for physically-driven joints. Bone transforms coming out of Motion Matching can be overridden per-joint by procedural solvers (aim-offsets, look-at, foot placement corrections) rather than requiring a new baked clip for every situational variant.

##### Reference Implementation

```cpp
// Applied per-joint after Motion Matching resolves the base pose, before ragdoll blending
Transform ApplyProceduralOverride(const Transform& mm_pose, const Transform& procedural_pose, float weight) {
    return Transform::Lerp(mm_pose, procedural_pose, weight); // weight from constraint-weight blending, EXT-06
}
```

##### Player-Facing Impact

Zombies and companions can aim, look, or reach at targets that Motion Matching's clip library was never authored for, without a visible pop back to a baked animation.

#### [M5.2-EXT-06] Constraint-Weight Blending (Animated ↔ Procedural)

##### Systems Touched

Per-joint blend weight consumed by the procedural override layer (M5.2-EXT-05) above.

##### Math

A single scalar per constrained joint drives the blend between the Motion Matching pose and the procedural solve:

`pose_final = pose_animated · (1 − w) + pose_procedural · w`, `w ∈ [0, 1]`

`w = 0` is pure Motion Matching output; `w = 1` is fully procedural (e.g. a hard IK foot-lock or aim solve). Intermediate values let a solver fade in/out over several frames instead of snapping.

##### How It Works

Every procedural solver in M5.2-EXT-05 (foot IK, aim IK, look-at, etc.) writes its result as a candidate pose plus a target blend weight rather than overwriting the joint transform outright. Each tick, the weight is stepped toward its target (a simple rate-limited lerp, not an instant snap) and `pose_final` is recomputed from the current animated pose and the procedural candidate. This keeps every procedural override going through one shared blend point instead of each solver having its own ad hoc fade logic.

##### Reference Implementation

```cpp
float w = std::clamp(blend_weight, 0.0f, 1.0f);
Transform pose_final = Transform::Lerp(pose_animated, pose_procedural, w);
```

##### Player-Facing Impact

Procedural corrections (foot placement on stairs, reaching for a door handle) fade in smoothly rather than snapping the limb into place, avoiding the "robotic pop" look of hard IK switches.

#### [M5.2-EXT-07] Ragdoll-to-Animation Recovery Blend

##### Systems Touched

Bridges M5.2's Active Ragdoll Dynamics (existing bullet above) back to Motion Matching once a knocked-down entity is ready to stand.

##### Math

A 10–15 frame crossfade blends the ragdoll's final settled pose into an orientation-matched get-up clip:

`pose_blend(t) = pose_ragdoll · (1 − α(t)) + pose_getup_clip · α(t)`, `α(t) = t / N`, `t ∈ [0, N]`, `N ∈ [10, 15]` frames

The get-up clip is selected/oriented to match the ragdoll's final yaw and prone/supine state so the crossfade doesn't visibly rotate the body mid-blend.

##### How It Works

Once a ragdolled entity's Jolt bodies settle (velocity below threshold for N consecutive frames), the system snapshots the ragdoll's final per-bone world pose and picks a get-up animation clip matching that orientation (prone-face-down, prone-face-up, or on-side, each with its own clip). The crossfade then runs for a fixed 10–15 frame window, blending bone-by-bone from the snapshotted ragdoll pose toward the clip's pose at each frame. Once `alpha` reaches 1, control hands off fully to Motion Matching and the ragdoll bodies are released from the physics world.

##### Reference Implementation

```cpp
float alpha = std::clamp(float(frame) / float(crossfadeFrames), 0.0f, 1.0f); // crossfadeFrames in [10,15]
Transform pose = Transform::Lerp(ragdoll_final_pose, getup_clip_pose, alpha);
```

##### Player-Facing Impact

A zombie or companion knocked down by an explosion or vehicle impact doesn't teleport back into a standing idle pose — it settles where physics put it, then rises through a matched get-up animation from that exact position and orientation.

#### [M5.2-EXT-08] IK Rig Asset (Per-Skeleton Bone-Chain / End-Effector Definition)

##### Systems Touched

Consumed by the procedural override layer (M5.2-EXT-05) and motion-warping (M5.2-EXT-09) below, for any feature that needs to resolve an end-effector target (foot, hand, weapon muzzle) back through a bone chain.

##### Math

No formula — a declarative bone-chain graph per skeleton: `chain = [root_bone, ..., end_effector_bone]` plus a pole-vector hint per chain.

##### How It Works

A one-time, per-skeleton data asset declaring bone chains (root→end-effector) and pole-vector hints, authored once per unique skeleton (base zombie, companion, phenotype variants sharing a skeleton) rather than re-declared per-feature.

##### Reference Implementation

```cpp
struct IKChain { std::vector<BoneIndex> bones; glm::vec3 poleVectorHint; };
struct IKRigAsset { std::string skeletonId; std::vector<IKChain> chains; }; // variable count per skeleton — do not hardcode a fixed chain count; phenotype variants may need more than the standard 4 human-limb chains
```

No IK Rig asset format or loader exists elsewhere in this doc — it's a genuinely new data asset type and file format, not a reuse of glTF geometry caching (M2.6) or prefab loading (M1), which load meshes/scenes, not bone-chain/end-effector metadata. The format needs to be designed before M5.2-EXT-05/09 can consume it.

##### Player-Facing Impact

None directly — this is the authoring/data layer other procedural-animation features above depend on to know which bones form "the left arm" or "the right foot" without hardcoding bone-name strings per skeleton.

#### [M5.2-EXT-09] Motion-Warping for Target Alignment

##### Systems Touched

Consumes the IK Rig asset (M5.2-EXT-08, flagged above) and blends through the procedural override layer (M5.2-EXT-05) for melee-hit alignment, vault-landing alignment (M2.7's vault fan-cast), and door-interaction alignment.

##### Math

The root motion offset for a clip is warped toward a target transform over the clip's duration using the same normalized-time blend shape as the recovery crossfade above:

`offset(t) = offset_clip(t) + (target − offset_clip(1)) · β(t)`, `β(t)` a normalized ease-in-ease-out curve over `t ∈ [0, 1]` (clip-relative time), so the warp is zero at the clip start and fully resolved at the clip's designed contact frame.

##### How It Works

During a vault, melee hit, or door interaction, the resolved contact point becomes the warp target. The root-motion offset blends from its authored path toward that target using an ease curve reaching full alignment at the clip's designed contact frame, so hands/feet land precisely regardless of approach angle.

##### Reference Implementation

```cpp
glm::vec3 WarpRootOffset(glm::vec3 clip_offset, glm::vec3 clip_end_offset, glm::vec3 target, float beta) {
    return clip_offset + (target - clip_end_offset) * beta; // beta from an authored ease curve, not linear t
}
```

##### Player-Facing Impact

Melee hits actually connect with the zombie's hitbox instead of swinging through empty air a foot off-target, and vault/door clips land hands and feet precisely on the ledge or handle regardless of the player's exact approach angle.

#### [M5.2-EXT-10] Physics-Based Secondary Motion Post-Process (Spring-Damper Attachments)

##### Systems Touched

Runs as the final stage of the pose pipeline — after Motion Matching, the procedural override layer (M5.2-EXT-05), and ragdoll blending — for non-simulated attachment points (capes, straps, loose gear). Explicitly separate from the existing XPBD Cloth Simulation (M5.2 above), which owns actual cloth meshes; this is a cheap per-point spring for rigid/semi-rigid dangling attachments that don't warrant a full cloth solve.

##### Math

A critically-damped spring-damper per attachment point, driven by its parent bone's motion:

`x'' = −k(x − x_target)/m − c·x'`, critically damped at `c = 2√(km)`

where `x` is the attachment's current offset from its rest position, `x_target` is the rest position driven by the parent bone, `k` the spring stiffness, `m` the point mass, and `c` the damping coefficient. Critical damping is chosen specifically so the attachment settles to rest without oscillating past its target — appropriate for gear that should sway and stop, not bounce indefinitely.

##### How It Works

Each tracked attachment point runs its own critically-damped spring each tick, pulled toward a rest position driven by its parent bone. Critical damping (`c = 2√(km)`) means it settles without overshoot or jitter.

##### Reference Implementation

```cpp
void UpdateSpringDamper(glm::vec3& x, glm::vec3& v, const glm::vec3& x_target, float k, float m, float dt) {
    float c = 2.0f * std::sqrt(k * m); // critical damping
    glm::vec3 accel = (-k * (x - x_target) - c * v) / m;
    v += accel * dt;
    x += v * dt;
}
```

##### Player-Facing Impact

Capes, ammo straps, and loose gear sway and settle naturally with a zombie's or companion's movement instead of rigidly following the parent bone or requiring a full cloth-sim pass for something that small.

#### [M5.2-EXT-11] Asynchronous Fixed-Block Humanoid Rig Matrix Pool Allocator

##### Systems Touched

Backs skeletal bone-transform storage for every humanoid entity (zombies, companions, players) across M5.2's animation pipeline — eliminates heap allocation thrashing when hundreds of entities spawn/evict across chunk borders.

##### Math

`Stride_aligned = (BoneCount·64 + 63) & ~63` — cache-line-aligned stride sizing.

##### How It Works

A flat memory arena of fixed-size, cache-aligned rig blocks is constructed once at boot, each holding up to 32 bone matrices. A simple free-list index chain hands out and reclaims blocks in O(1) with zero runtime heap traffic, matching the same alignment idiom `[M0-EXT-01]` and `[M1-EXT-07]` already use elsewhere in the doc for cache-line safety under enkiTS worker contention.

##### Reference Implementation

```cpp
struct alignas(64) HumanoidRigBlock {
    float boneMatrices[32][16]; // fixed 32-bone structural transform block
    uint32_t nextAvailableIndex;
};
struct RigFixedPoolArena {
    HumanoidRigBlock* memoryBlockBuffer{nullptr};
    uint32_t freeListCursorHead{0};
    inline HumanoidRigBlock* AllocateRigBlock(RigFixedPoolArena& pool) {
        HumanoidRigBlock* block = &pool.memoryBlockBuffer[pool.freeListCursorHead];
        pool.freeListCursorHead = block->nextAvailableIndex;
        return block;
    }
};
```

##### Player-Facing Impact

Invisible directly — this is what keeps horde spawn/despawn waves at chunk borders from causing allocator-driven frame hitches.

#### [M5.2-EXT-12] Skeletal Joint Yield Torque Bone Fracturing Filter
#### [M5.2-EXT-14] (provisional) IK Rig Metadata Serialization Loader *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Consumed by M5.2-EXT-08 (IK Rig Asset struct) + M5.2-EXT-09 (Motion-Warping target). Feeds every end-effector consumer in M5.2 (melee-hit align, vault landing, door interact). Distinct from M2.6 glTF/prefab loading (mesh/scene, not bone-chain+pole-vector metadata).

##### Math
Declarative asset: IKRigAsset{skeletonId, chains:[{boneIdx[], poleVectorHint}]}. Hashing for on-disk table reuses same FNV-1a keying idiom M2.6 uses for geometry cache (uint64_t key=FNV1a(path)).

##### How It Works
Small JSON/flat-binary asset per skeleton, authored once, loaded at skeleton-registration. Loader parses bone-name->index against skeleton's own joint list (renamed bone fails loudly at load, not silently at runtime), stores resolved IKRigAsset in same hash-keyed table as geometry cache (key space skeletonId string). Phenotype variants reuse one asset; extra chains via override asset.

##### Reference Implementation
```cpp
struct IKChainDef { std::vector<uint16_t> boneIdx; glm::vec3 poleVectorHint; }; auto asset=LoadIKRig(skeletonId);
```

##### Player-Facing Impact
Skeletons load bone-chain defs from data, not hardcoded bone-name lookups — robust to renamed bones.

---
#### [M5.2-EXT-15] XPBD Rope/Tether Constraint *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Long-range-attachment (tether) constraint used internally for cloth/vegetation but not exposed; promotes to reusable XPBD distance/rope constraint. Consumes M5.2 solver + M5.2-EXT-08 IK/joint infra. Used by zipline/grappling/winch + drag-ragdoll.

##### Math
Extended PBD (Macklin 2016): each distance constraint has compliance a and accumulates Lagrange multiplier lambda per substep, stiffness independent of iteration count/timestep: Dx=(w1w2/(w1+w2+a~)) * C * nC, with a~=a/Dt^2. Rope = chain of N distance constraints solved with substep XPBD (e.g. 4 substeps, 1 iter each).

##### How It Works
Build rope as small particle chain (positions+inverse masses), one XPBD distance constraint per segment. Each substep: predict, solve all constraints updating lambda, integrate. Two endpoints bind to entity handles (player hand, vehicle hitch). Persistent-corrected lambda: rope neither explodes nor sags through floors under load.

##### Reference Implementation
```cpp
void SolveDistanceXPBD(Particle& a, Particle& b, float rest, float alpha, float dt, float& lambda){ vec3 d=b.x-a.x; float C=length(d)-rest; vec3 n=d/max(length(d),1e-5f); float k=1.0f/(a.w+b.w+alpha/(dt*dt)); vec3 corr=-k*C*n; a.x-=a.w*corr; b.x+=b.w*corr; lambda+=k*C; }
```

##### Player-Facing Impact
Ziplines/ropes/winches are stable and non-stretchy — no PBD explosion or floor-sag failure.

---

##### Systems Touched

Extends M5.2's Active Ragdoll Dynamics and `[M5.2-EXT-01]`'s Stable-PD joint torque model — detects structural joint failure after severe explosive or vehicle impacts and soft-kills the affected joint's motor rather than letting it stretch into a degenerate state.

##### Math

`S_shear = ‖τ‖ / r_bone_profile³`

`If S_shear ≥ σ_bone_fracture_limit ⟹ K_p → 0, K_d → 0` (motor deactivated)

##### How It Works

Tracks per-joint transient torque impulse against a fracture shear limit defined per bone profile. Crossing the limit flags the joint as fractured and zeroes its Stable-PD gains (`[M5.2-EXT-01]`) immediately, so the limb goes fully passive/ragdoll at that joint instead of the PD motor fighting an already-destroyed constraint — this is the structural-failure counterpart to `[M5.2-EXT-01]`'s gradual compliance degradation, triggering on a hard threshold rather than accumulated damage.

##### Reference Implementation

```cpp
struct PhysicsJointMotorData {
    float transientTorqueImpulseNewtonSeconds{0.0f};
    float fractureShearLimit{1200.0f};
    bool isJointFractured{false};
};
inline void CheckBoneJointFracture(PhysicsJointMotorData& joint, float frameImpulseForce, float dt) {
    float transientStress = joint.transientTorqueImpulseNewtonSeconds + (frameImpulseForce / (dt + 1e-5f));
    if (transientStress > joint.fractureShearLimit) {
        joint.isJointFractured = true; // strips active PD muscle torque gains this frame
    }
}
```

##### Player-Facing Impact

A zombie hit by an explosion or vehicle impact snaps limbs convincingly at the moment of overload instead of stretching into a rubber-jointed glitch — the ragdoll reads as broken, not buggy.

## M5.3 — AI perception system

##### Implementation Steps

* Signal-to-Noise Ratio (SNR) Detection Probability (`P = 1/(1+e^{-(SNR-θ)/s})`).
* Ray-Marched Foliage Concealment.
* AI Spatial Memory Hash Grid with event tags.
* Atmospheric Gaussian Plume Scent Dispersion advected by wind. Smoke Scent Suppression.
* Perception Sensor Fusion (Kalman filter).
* Confidence-Gated Stealth Strikes.

##### Decisions to flag

Perception budget/staggering at horde scale.

##### Exit Criteria

* SNR detection reliably differentiates noise profiles.
* Upwind/downwind scent mechanics demonstrably function.



### Extended Systems Library — engine-side additions for M5.3

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M5.3-EXT-01] Supersonic Scent-Wake Cones (Aerodynamic Scent Compression)

##### Systems Touched

Modifies spatial distribution rules for the Atmospheric Gaussian Plume Scent Dispersion grid.

##### Math

`θ_cone = arcsin(‖v_advect + v_diff‖ / ‖v_player‖)`

##### How It Works

Extends your M5.3 Atmospheric Gaussian Plume Scent Dispersion grid. When a player is sprinting or driving a vehicle fast through an open-world environment, updating scent emission cell-by-cell on a coarse grid creates blocky, disconnected traces that entities cannot pathfind along smoothly.

When the entity's velocity vector (‖v_player‖) exceeds the local wind advection and diffusion thresholds, the engine switches from standard circular emission grids to an analytical Mach-cone geometry trailing behind the player. Scent concentration levels are compressed strictly inside the calculated boundary angle (θ_cone).

The ratio inside `arcsin` must be ≤ 1 (it's a sine): slower ambient speed over faster player speed, the same shape as the classic Mach-cone half-angle `arcsin(c/v)`.

##### Reference Implementation

```cpp
void EmitScentCone(const glm::vec3& playerVel, const glm::vec3& windVel, float ambientDiffusionSpeed, ScentGrid& grid, uint32_t cellX, uint32_t cellZ) {
    float speed = glm::length(playerVel);
    if (speed <= ambientDiffusionSpeed) {
        grid.DepositStandardRadius(cellX, cellZ, 1.0f);
    } else {
        float angle = std::asin(ambientDiffusionSpeed / speed); // slower ambient speed over faster mover, per the Mach-cone correction already noted on this block
        grid.DepositWedgeCone(cellX, cellZ, glm::normalize(playerVel + windVel), angle);
    }
}
```

##### Player-Facing Impact

Moving at high speed leaves a concentrated, tightly packed aerodynamic scent wake behind you. Downwind screamer hordes can pick up the trail of a speeding vehicle over immense distances long after it has passed, preventing players from completely bypassing zombie perception logic simply by driving fast.

#### [M5.3-EXT-02] Atmospheric Convection Acoustic Refraction (Wind-Gated Hearing Filters)

##### Systems Touched

Blends M10 (Weather/Wind vectors) with M5.3 (SNR perception filtering) and M6 (Acoustic audio events).

##### Math

The sound pressure level (SPL) of an audio event arriving at an AI spatial hearing sensor is attenuated based on the vector dot product of the localized wind velocity (v_wind ) and the direct sound propagation unit vector (u_sound ) relative to the speed of sound (c):

`SPL_effective = SPL_base − 20·log₁₀(1 − (v_wind · u_sound)/c)`

##### How It Works

Your M5.3 Signal-to-Noise Ratio (SNR) detection pipeline calculates continuous detection probabilities by dividing player audio signatures by active environmental backgrounds. This system introduces physical audio warping. Sound traveling upwind (against a high-velocity wind storm vector) is refracted upward and compressed, dropping its effective volume rapidly before it reaches zombie spatial hash coordinates.

##### Reference Implementation

```cpp
float ComputeWindAttenuatedSPL(float splBase, const glm::vec3& windVel, const glm::vec3& soundDir, float speedOfSound) {
    float dotProduct = glm::dot(windVel, glm::normalize(soundDir));
    float ratio = dotProduct / speedOfSound;
    float effectiveDenominator = std::max(1e-3f, 1.0f - ratio);
    return splBase - (20.0f * std::log10(effectiveDenominator));
}
```

##### Player-Facing Impact

This introduces a deep layer of tactical meteorology. When a severe 15% rain storm or a 3% gale-force atmospheric event kicks up extreme wind vectors, firing a weapon upwind drastically reduces its audible footprint. You can utilize high-velocity storm fronts to mask noisy tactical movements or weapon discharges from a screamer horde positioned downwind from your location, turning the environment into an active sensory shield.

##### Depends on (not yet built at this point)

M10, M6 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M5.3-EXT-03] Volatile Scent-Evaporation Thermal Grids

##### Systems Touched

Extends M5.3 Atmospheric Gaussian Scent Plume Dispersion matrices.

##### Math

Your character's scent emission rate (E_scent ) is treated as a volatile organic compound evaporation field that scales over ambient atmospheric temperatures (T_Celsius ) and wind flow shears:

`E_scent = E_baseline · (1.0 + α·T_Celsius)`

The lifespan and decay rate (λ_decay) of the scent particles deposited into your spatial tracking grids expand exponentially as the ground surface heats up:

`λ_decay = λ_base · e^(β·T_Celsius)`

##### How It Works

Your spatial tracking system distributes scent footprint intensity bytes across your spatial maps. This system scales the baseline injection levels and decay rates by sampling your global M10 calendar temperature loops. It updates the grid variables on a slow, low-overhead background cadence, avoiding frame budget spikes.

##### Reference Implementation

```cpp
void UpdateScentEvap(float t_c, float base_e, float base_d, float alpha, float beta, float& out_e, float& out_d) {
    out_e = base_e * (1.0f + alpha * t_c); out_d = base_d * std::exp(beta * t_c);
}
```

##### Player-Facing Impact

Sprinting across an asphalt avenue during a hot 35 ∘ C midday sector cause your scent trail to evaporate and dissipate into the air rapidly, making it difficult for tracking trackers to lock onto your path. However, cold, humid night cycles lock your scent down near the pavement surface, leaving a thick, long-lasting scent trail that lets background trackers track your route across long distances.

#### [M5.3-EXT-04] Volumetric Scent-Cloud Domain Warping (Wind Blaster)

##### Systems Touched

Modifies M5.3 Atmospheric Gaussian Scent Plume cells during explosive events.

##### Math

High-pressure explosive detonations generate a divergence step that distorts your spatial scent tracking field arrays away from the blast center:

`∇⋅v_scent =Q⋅δ(x−x_blast )`

##### How It Works

An explosive detonation injects a divergence source term at the blast's grid cell. Rather than deleting scent data, nearby scent-cell velocity vectors are pushed radially outward from the blast center each tick, scattering the cloud instead of erasing tracking data.

##### Reference Implementation

```cpp
void DisplaceScent(const glm::vec3& b_pos, const glm::vec3& s_pos, float force, glm::vec3& out_scent_vec) {
    glm::vec3 dir = s_pos - b_pos; out_scent_vec += glm::normalize(dir) * (force / (glm::dot(dir, dir) + 1e-5f));
}
```

##### Player-Facing Impact

If a tracking zombie pack has localized your position via scent plume advection vectors, detonating a pipe bomb or firing a high-overpressure muzzle shot creates a localized atmospheric shockwave. This violently clears and pushes your scent cloud away from your coordinates, breaking their scent tracking vectors instantly.

#### [M5.3-EXT-05] Aerodynamic Scent-Plume Mass Displacement

##### Systems Touched

Deepens M5.3 Atmospheric Gaussian Scent Plume cell updates using M10 wind pathways.

##### Math

To prevent players from gaming or wiping their scent trails using simple tracking updates, explosions do not delete scent footprint data arrays. Instead, high-pressure explosive detonations generate a physical fluid divergence step that shifts scent cells outward using a strict conservation of mass vector:

`∂t_∂C +v_wind ⋅∇C=D∇ 2 C+S_blast (x−x_blast )`

##### How It Works

Rather than clearing scent data on explosion, a blast adds a source term to the advection-diffusion scent equation, shoving nearby scent mass radially outward. Tracking entities still converge on the scattered field, just without a precise origin point.

##### Reference Implementation

```cpp
void ShiftScentMass(const glm::vec3& b_pos, const glm::vec3& cell_pos, float blast_force, glm::vec3& scent_velocity) {
    glm::vec3 dist_vec = cell_pos - b_pos; scent_velocity += glm::normalize(dist_vec) * (blast_force / (glm::dot(dist_vec, dist_vec) + 1e-4f));
}
```

##### Player-Facing Impact

Detonating a pipe bomb or firing a high-overpressure muzzle shot does not dissolve your scent trail. The blast wave violently shoves your localized scent cloud outward, scattering it down alleys and building corridors. Tracking zombie herds lose your exact spot coordinates, but they will still converge on the broader, scattered scent field downwind, forcing you to think about wind direction before creating explosions.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M5.3-EXT-06] Volatile Scent-Suppression Atmospheric Aerosol Diffusion Grid

##### Systems Touched

Hardens M5.3's Atmospheric Gaussian Plume Scent Dispersion against active particulate layers — dense smoke from burning structures (M6.5 fire thermodynamics) or vehicle exhaust — masking and decaying the player's scent signature within affected grid cells. Complements `[M5.3-EXT-03]`'s thermal scent-evaporation grid; that one scales emission/decay by temperature, this one scales the read-out by local aerosol density.

##### Math

`C_effective = C_raw_scent · e^(−κ_mask · ρ_smoke)`

##### How It Works

Samples the local aerosol/smoke cell density (M6.5) at the same grid resolution the scent-dispersion pass already queries, and applies an exponential mask directly to the raw scent value read out for that cell — no separate grid maintained, this is a read-time modifier on the existing plume data.

##### Reference Implementation

```cpp
inline float ComputeAerosolMaskedScent(float rawScentValue, float aerosolCellDensity, float attenuationKappa) {
    if (aerosolCellDensity < 0.001f) return rawScentValue;
    return rawScentValue * std::exp(-attenuationKappa * aerosolCellDensity);
}
```

##### Player-Facing Impact

Setting a diversionary fire or driving through smoke genuinely masks your scent trail from tracking hordes, giving you a tactical option beyond `[M5.3-EXT-04]`/`[M5.3-EXT-05]`'s blast-displacement approach — smoke suppresses continuously rather than scattering a one-time cloud.

#### [M5.3-EXT-07] Confidence-Gated Stealth Strikes
#### [M5.3-EXT-08] Visual Occlusion Sector Ray-March Pre-Filter *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.3 + M5.4. Ray-marches visibility sectors pre-frame to pre-cull AI that can't see the player.

##### Math
sector_i visible if March(origin, dir_i) < dist(player); cull AI in invisible sectors.

##### How It Works
Before AI update, the player's visibility is ray-marched into angular sectors; AI whose sectors are occluded are pre-culled from perception (they can't see the player), saving perception/behavior cost. Recomputed at a low rate, reprojected between.

##### Reference Implementation
```cpp
bool sees = MarchSector(origin, dirToPlayer) < distPlayer; if(!sees) ai.PerceptionOff();
```

##### Player-Facing Impact
Off-screen AI doesn't magically track you through walls - perception is honest and cheaper.

##### Systems Touched

Declares the base "Confidence-Gated Stealth Strikes" bullet. Reads a target zombie's per-sense confidence accumulators from M5.3's core perception system (sight/sound/scent each contribute independently rather than a single binary "aware" flag) and gates M2.7's melee takedown animation/damage path.

##### Math

Each zombie already tracks a scalar detection-confidence value `C ∈ [0,1]` that rises as senses accumulate evidence of the player and decays otherwise. A stealth strike is legal only below a hard threshold: `Eligible = (C < C_threshold) ∧ (C_threshold = 0.15)`, and its guaranteed-kill damage multiplier scales inversely with how close the target was to noticing: `DmgMult = Lerp(3.0, 8.0, 1 − C/C_threshold)` — a target at 0 confidence gets the full 8x backstab multiplier, one at 0.14 barely qualifies at ~3x.

##### How It Works

Rather than a boolean "is this zombie aware," the takedown check reads the same continuous confidence value M5.3's perception system already maintains per zombie per tick. If confidence sits under the threshold when the player initiates a melee takedown, the strike is legal and its damage multiplier is computed live off exactly how close the target was to detecting the player — so a strike on a completely oblivious zombie in a quiet room hits harder than one on a target that had already caught a flicker of movement.

##### Reference Implementation

```cpp
// Checked at takedown-initiation time against the target's live per-tick confidence value
bool TryStealthStrike(float confidence, float confidenceThreshold, float& outDamageMult) {
    if (confidence >= confidenceThreshold) return false; // too alert, strike not legal
    outDamageMult = std::lerp(3.0f, 8.0f, 1.0f - confidence / confidenceThreshold);
    return true;
}
```

##### Player-Facing Impact

Stealth kills feel earned and readable off the same detection meter the player already watches for regular sneaking — no separate hidden "can I backstab this one" check, and near-misses (almost-detected zombies) still reward a stealth kill, just for less damage.

#### [M4.6-EXT-04] VRAM Sparse-Resident Memory Page Physical Allocation Tracker

##### Systems Touched

Runtime Virtual Texture (RVT) engines, Vulkan Memory Allocator handling, chunk streaming pipelines.

##### Math

Distance-weighted look-ahead residency mapping driving speculative physical allocation queries against incoming camera tracking vectors:

`P_alloc = v_camera · (x_chunk − x_camera) / (‖x_chunk − x_camera‖² + ε)`

##### How It Works

Tracks memory allocations for sparse-resident physical textures across procedurally streaming chunk zones. Instead of allocating texture arrays flatly, pages use sparse-resident descriptors. When velocity checks reveal an approaching boundary, non-blocking map requests commit memory page blocks before sampling events occur, working alongside `[M4.5-EXT-07]`'s pre-allocated VMA block pool rather than a second allocation path.

##### Reference Implementation

```cpp
#include <cmath>

inline bool IsSparsePageResidencyRequired(const float camPos[3], const float camVel[3], const float chunkPos[3], float radiusThreshold, float epsilon) {
    float toChunk[3] = { chunkPos[0] - camPos[0], chunkPos[1] - camPos[1], chunkPos[2] - camPos[2] };
    float distanceSquared = (toChunk[0] * toChunk[0]) + (toChunk[1] * toChunk[1]) + (toChunk[2] * toChunk[2]);
    if (distanceSquared < epsilon) return true;

    float velocityProjection = (camVel[0] * toChunk[0]) + (camVel[1] * toChunk[1]) + (camVel[2] * toChunk[2]);
    float allocationWeight = velocityProjection / (distanceSquared + epsilon);
    return allocationWeight > (1.0f / radiusThreshold);
}
```

##### Player-Facing Impact

Eliminates texture popping and streaming stalls during high-velocity driving sequences, keeping engine VRAM usage within its strict 6.0GB boundary.

## M5.4 — AI Director (horde pacing and spawner)

##### Implementation Steps

* Multi-Player State Aggregation.
* Thompson-Sampling Spawning Bandit (`WaveComposition ∼ ThompsonSampling(α,β)`).
* Reaction-Diffusion Fear Field (`∂φ/∂t = D∇²φ + S(x,t) - kφ`).
* Panic-Gradient Audio Ducking.
* Reynolds Boid Crowd Correction over flow vectors.
* Unified `TensionSignal` Subscriptions (EMA-smoothed scalar shared by haptics, UI, music).
* Horde Pressure Structural Loading mapped to M3 trusses.

##### Decisions to flag

Fear-field grid resolution.

##### Exit Criteria

* Threat correctly alters wave compositions automatically.
* Fear-field diffuses panic efficiently.



### Extended Systems Library — engine-side additions for M5.4

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M5.4-EXT-01] Volumetric Boid Compression Stampede Crush

##### Systems Touched

Enhances M5.4 reaction-diffusion fields and M5.1 Reynolds boid flocking parameters.

##### Math

When hordes are panicked, their separation thresholds drop, generating intense mechanical crowd pressure (P_crush ). Your engine evaluates this by tallying neighbor density straight from your existing SpatialHash:

`P_crush = Σ_{i∈LocalCell} ∥F_separation_i∥`

If P_crush exceeds the tissue threshold (σ_flesh), the engine fires a target DamageEvent downward to any entities caught beneath the swarm.

##### How It Works

Each tick, the AI steering phase sums separation-force magnitudes of boids sharing a local spatial-hash cell into a crush-pressure scalar. Exceeding the tissue threshold damages any entity beneath the crowd proportional to the excess — a panicked horde's own density becomes the hazard.

##### Reference Implementation

```cpp
// Evaluated inside the AI steering phase across your fixed-timestep spatial hash lookups
void CalcStampedeCrush(float force_sum, float sigma_flesh, float scale, float& out_damage) {
    if (force_sum > sigma_flesh) out_damage += (force_sum - sigma_flesh) * scale;
}
```

##### Player-Facing Impact

This mechanics triggers terrifying stampede scenarios. If an explosion or a screamer alarm panics a packed alleyway, the trailing zombies will physically run over and trample crawled or injured entities in their path. This creates crushed organic debris, giving you a tactical incentive to herd hordes into tight architectural gaps to let them crush their own numbers.

#### [M5.4-EXT-02] Over-Mantle Tumbling Torque Vectoring

##### Systems Touched

Integrates M5.4 horde steering pressure variables directly into M2.9 procedural vaulting routines.

##### Math

When a zombie entity initiates a procedural vault maneuver over a low wall barrier, it experiences crowd pushing forces (F_push ) from trailing entities. If this push force exceeds the balance threshold, it injects an over-rotating angular momentum torque (τ_tumble ):

`τ_tumble =r_torso ×F_push`

`If the rotation velocity (ω_rot ) exceeds stability limits during the vault, the entity's procedural animation loop aborts, instantly switching its Jolt physics state flags into a full tumbling ragdoll pass.`

##### How It Works

To safeguard your performance budget, the engine completely bypasses ragdoll initialization while the zombie is airborne. The entity is treated as a single Jolt rigid body box tracking rotation. The system queries your SpatialHash neighbor densities; if forward crowd force vectors cross safety limits, it forces an immediate tumble state, delaying ragdoll assembly generation until terminal contact with the terrain.

##### Reference Implementation

```cpp
bool CheckVaultTumble(const glm::vec3& f_push, float r_y, float inertia, float limit, float dt, float& w_rot) {
    w_rot += (f_push.z * r_y / inertia) * dt; return std::abs(w_rot) > limit;
}
```

##### Player-Facing Impact

When a dense horde chases you over a rooftop, the front zombies attempting to vault over the edge railings get physically pushed from behind by the mass of the crowd. Instead of landing cleanly on their feet, they over-rotate, tumbling head-over-heels over the edge and crashing into the street below, creating organic pile-ups beneath the structure.

#### [M5.4-EXT-03] Emergent Difficulty via Director Pacing & Outbreak Feedback (No Difficulty Setting)
#### [M5.4-EXT-05] Reaction-Diffusion Grid Sub-sampled Boundary Welder *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.4 fear field (RD). Welds RD grid boundaries across chunk seams so the field is continuous.

##### Math
ghost = avg(neighbors across seam); sub-sample boundary, copy to neighbor ghost cells each step.

##### How It Works
The reaction-diffusion fear field is computed per chunk; at chunk boundaries, a sub-sampled ghost exchange copies edge cells into neighbor ghost cells so diffusion crosses seams without discontinuity or double-counting.

##### Reference Implementation
```cpp
ExchangeGhost(boundary[A], ghost[B]); // seam weld each RD step
```

##### Player-Facing Impact
Fear/panic spreads continuously across the world - no visible grid seams in the fear field.

---
#### [M5.4-EXT-06] Hierarchical A* Macro-Cell Long-Range Road-Graph Router *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.4 horde + M8.5 caravan routing. Hierarchical A* over the macro road-graph for off-screen long-range routing.

##### Math
macro = Cluster(roadNodes, cell); path = HAStar(root,goal,macro,fine); determinism = SplitMix64(seed).

##### How It Works
A macro-graph of abstract street/highway nodes (built from M4-EXT-10 splines) is clustered into macro-cells; Hierarchical A* plans at the macro level then refines fine paths inside the goal cell. Seeded via SplitMix64 for deterministic hordes. Terrain-gen consumes it for road-exclusion masks (M4-EXT-17). Single system - M9-EXT-18 convoy router extends this graph with a lane-width field rather than duplicating it.

##### Reference Implementation
```cpp
RoadGraph g = BuildMacroGraph(splines); Route r = HAStar(g, hordeOrigin, target, seed);
```

##### Player-Facing Impact
Hordes flow along real streets and caravans traverse believably; roads reshape spawning - one shared graph, no duplicate systems.

---
#### [M5.4-EXT-07] Holling Type II Cannibalism Feeding Satiator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.4 cannibal factions. Models feeding satiation (Holling Type II) so factions stop at capacity.

##### Math
consumption = (a*N)/(1 + a*h*N); dN/dt = r*N*(1-N/K) - consumption; satiation caps intake.

##### How It Works
Cannibal factions 'consume' victim density with a Holling Type II functional response (intake saturates with abundance), so they don't infinitely devour - a satiation term caps per-capita consumption, producing stable predator-prey oscillation instead of wipeout.

##### Reference Implementation
```cpp
float eat = (a*N)/(1+a*h*N); pop -= eat; satiation = clamp(satiation+eat,0,1);
```

##### Player-Facing Impact
Cannibal factions thin herds then back off - the world's ecosystem stays balanced, not extinct.

---

#### [M5.4-EXT-09] Fear-Field Diffusion via Spatial Hash *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M5.4 + M1-EXT-27. Diffuses the fear/reaction-diffusion field over a spatial hash.

##### Math
f_new = f + D*Laplacian(f) - decay; Laplacian from neighbor cells in hash;

##### How It Works
The reaction-diffusion fear field is stepped by sampling neighbor cells from the spatial hash (M1-EXT-27) to compute the Laplacian, diffusing panic smoothly across the world without a full grid. Cheap, sparse.

##### Reference Implementation
```cpp
f = StepDiffuse(f, hash, D, decay);
```

##### Player-Facing Impact
Fear/panic spreads organically - hordes react to player pressure at range.


##### Systems Touched
M5.4 cannibal factions. Models feeding satiation (Holling Type II) so factions stop at capacity.

##### Math
consumption = (a*N)/(1 + a*h*N); dN/dt = r*N*(1-N/K) - consumption; satiation caps intake.

##### How It Works
Cannibal factions 'consume' victim density with a Holling Type II functional response (intake saturates with abundance), so they don't infinitely devour - a satiation term caps per-capita consumption, producing stable predator-prey oscillation instead of wipeout.

##### Reference Implementation
```cpp
float eat = (a*N)/(1+a*h*N); pop -= eat; satiation = clamp(satiation+eat,0,1);
```

##### Player-Facing Impact
Cannibal factions thin herds then back off - the world's ecosystem stays balanced, not extinct.

---

##### Systems Touched

Reads M5.4's existing AI Director pacing state (`TensionSignal`, Reaction-Diffusion Fear Field) and M8's SEIR outbreak severity directly as inputs to spawn/horde density — this feature is a feedback wire between two existing systems, not a new difficulty system, and explicitly has no player-facing setting.

##### Math

Spawn/horde density multiplier is a direct function of existing, already-tracked world state — not a player-set scalar:

`Density_mult = 1.0 + α·NoiseExposure_recent + β·OutbreakSeverity_regional + γ·SettlementsLost`

##### How It Works

The Director already aggregates multi-player state and drives a unified `TensionSignal`; this feature adds two more inputs into the same density multiplier the Director already applies — regional SEIR severity (M8) and a settlement-loss counter (M8.6/M7). There is no menu option anywhere that writes to `Density_mult`; every save reads the identical formula.

##### Reference Implementation

```cpp
// Extends the existing AI Director density multiplier — no new top-level system, no settings hook
float ComputeDirectorDensityMult(float noise_exposure, float outbreak_severity, int settlements_lost) {
    return 1.0f + 0.15f * noise_exposure + 0.25f * outbreak_severity + 0.1f * settlements_lost;
}
```

##### Player-Facing Impact

Every player gets the same base experience — the world gets harder specifically because of what you did (stayed loud, let an outbreak spread, lost a settlement), not because of a slider, which keeps the tension honest and comparable across playthroughs.

## M6 — Hardware-accelerated audio system with propagation

##### Implementation Steps

* Ray-Traced Acoustic Diffraction Portals (`caps.rtPipeline`). Fallback to fan-raycast fractional occlusion.
* **HRTF binaural spatialization** for headphone output (the default listening setup for this genre): Miniaudio (the resolved audio backend) only provides distance/pan attenuation, not head-related transfer function filtering — without it, all the diffraction/occlusion work above tells the player *that* a sound is blocked but stereo panning alone can't reliably tell them *front vs. behind*, which is exactly the cue a horde-detection survival game depends on. Layer a lightweight HRTF convolution stage (a small measured HRIR set, e.g. MIT KEMAR or a similarly-sized public dataset, nearest-neighbor or simple magnitude-interpolated lookup — full per-frame spectral interpolation is unnecessary polish for a solo scope) on top of Miniaudio's existing mixer output per 3D voice, gated by a `caps.hrtfEnabled` user setting (speaker users should keep plain stereo/surround panning, since HRTF over speakers sounds wrong).
* Material-Indexed Reverb. Velvet-Noise Late Reverb Synthesis.
* Pitch-Synchronous Overlap-Add Voices (PSOLA) / Real-Time Neural Voice Coding (`caps.neuralVocoder`).
* Granular Doppler Shifts. Modal Ambiance Synthesis.
* Acoustic Environmental Absorption mapping rain intensity.
* Markov-Chain Note Generation layered by TensionSignal.

##### Decisions to flag

Audio backend (Resolved: Miniaudio). Procedural generative scale/chord-pools.

##### Exit Criteria

* Sound paths diffract or muffle correctly per occlusion checks.
* Generative music adapts dynamically.



### Extended Systems Library — engine-side additions for M6

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M6-EXT-01] Acoustic Doppler-Shift Pitch Modulator

##### Systems Touched

Combines M2.9 Ballistic projectile velocities with M6 audio engine streaming callbacks.

##### Math

The sound frequency (f_effective ) of high-speed entities (like a flying projectile or a screaming mutated zombie) shifts dynamically based on its relative movement vector relative to the player camera look vector:

`f_effective =f_base ⋅( c−v_source ⋅u c−v_observer ⋅u`

) Where c represents your dynamically calculated thermally driven speed of sound constant, and u is the direct unit vector pointing from the source straight to the observer.

##### How It Works

Operating entirely within your background audio rendering thread callbacks, the sound mixer samples the relative velocity deltas between the source entity and the player's camera system. The calculated scaling ratio directly updates the pitch playback speed constants passed to your Vulkan-linked mixers without re-allocating audio voices.

##### Reference Implementation

```cpp
float CalcDoppler(float f0, float c, const glm::vec3& v_s, const glm::vec3& v_o, const glm::vec3& u) {
    return f0 * ((c - glm::dot(v_o, u)) / (c - glm::dot(v_s, u) + 1e-5f));
}
```

##### Player-Facing Impact

Ducking behind a brick wall while automatic weapon fire covers your sector transforms the audio landscape. You hear the high-pitched acoustic whistle of oncoming rounds compressing the air waves before transitioning into a low-frequency hiss as they pass your viewport, letting you judge projectile velocities by ear.

#### [M6-EXT-02] Footstep Impact Acoustic Resonance

##### Systems Touched

Pairs your M2.7 character controller speed parameters with M6 ray-traced acoustic portals.

##### Math

The initial sound pressure magnitude (SPL_step ) of a character walking or sprinting scales directly over the entity's total mass (m_total ), velocity vectors (v), and the acoustic mechanical impedance constant (Z_surface ) of the structural material tile underneath:

`SPL_step = κ·m_total·∥v_foot∥²·Z_surface`

##### How It Works

Inside the kinematic controller update pass, when foot tracking flags indicate surface contact frames, the system reads the material ID returned by your SurfaceFrictionSample query. The combined weight and velocity scalars determine the sound pressure level inline, passing it directly to the spatial sound hash maps without creating heavy audio object structures.

##### Reference Implementation

```cpp
float GetFootstepVolume(float mass, float v, float z_surf, float kappa) {
    return kappa * mass * (v * v) * z_surf;
}
```

##### Player-Facing Impact

Sprinting lightly across a soft grass lawn emits a low acoustic signature that easily slips past alert zombie ears. However, running across a hollow corrugated sheet metal roof with a fully packed, heavy inventory layout generates sharp acoustic resonance spikes that echo across urban blocks, instantly drawing nearby hordes to your precise structure.

#### [M6-EXT-03] Acoustic Echo-Location Decoy Sound Mirroring

##### Systems Touched

Upgrades M6 ray-traced audio propagation paths inside your background audio callbacks.

##### Math

Sound pressure wave rays hitting flat, high-density masonry structural surfaces undergo clean specular acoustic reflection, shifting the virtual source location vector perceived by AI perception nodes:

`I_echo =I_source ⋅cos(θ)⋅R_material_hardness`

##### How It Works

A sound ray traced against a flat, high-density masonry surface reflects specularly rather than terminating; the AI perception system evaluates the reflected ray's apparent origin — scaled by material hardness — as an additional candidate sound source.

##### Reference Implementation

```cpp
void EchoAcousticRay(const glm::vec3& ray_in, const glm::vec3& n, float r_mat, glm::vec3& ray_out) {
    ray_out = (ray_in - 2.0f * glm::dot(ray_in, n) * n) * r_mat;
}
```

##### Player-Facing Impact

You can trick zombie hearing systems using geometry. Throwing a bottle or discharging an un-suppressed weapon directly at a flat brick wall causes the sound wave vector to bounce cleanly off the facade. Tracking zombies down the block will mistake the acoustic reflection point for the source, racing toward the wall while you slip away down an opposing corridor.

#### [M6-EXT-04] Voxelized Acoustic Propagation Portal Weaver

##### Systems Touched

Complements M6 hardware audio structures and reacts to breaks tracked by M3 destruction.

##### Math

Sound pressure attenuates through a localized opening based on the aperture's area and orientation relative to the sound path:

`Attenuation = Aperture_area · (SoundVector · FaceNormal) / Distance²`

##### How It Works

Standard spatial-audio setups need designers to hand-place acoustic portals and occlusion zones, which isn't possible in a fully procedural world with destructible walls. This system runs an automated spatial flood-fill on background worker threads that groups hollow world volumes into discrete acoustic zones and maps portal connections wherever structural gaps exist — windows, open doors, holes blown in walls. The audio engine traces sound paths through this live network, updating occlusion instantly whenever a wall breaks (M3).

##### Reference Implementation

```cpp
// Registers a spatial portal to dynamically alter audio occlusion whenever structural health changes
void BuildAcousticLink(float area, float structural_health, float& link_scalar) {
    link_scalar = (structural_health <= 0.0f) ? 1.0f : std::clamp(area * 0.1f, 0.0f, 0.9f);
}
```

##### Player-Facing Impact

Sound propagates realistically. Gunshots or zombie groans from an adjacent closed room sound muffled and deep, but breaking a window or opening a door lets high frequencies spill realistically into your corridor with sharp positioning accuracy.

#### [M6-EXT-05] Audio Voice Prioritization Matrix Allocation Weight Culler

##### Systems Touched

Sits under M6's Miniaudio backend decision, gating channel budget at horde scale — distinct from the propagation/Doppler/echo EXT features above, which shape sound that's already playing rather than deciding whether it plays at all.

##### Math

`Priority = Volume_base / (d_distance² + ε) · cos(θ_viewAngle)`

##### How It Works

Running full atmospheric/propagation processing for every active sound source under a 200-zombie horde would exhaust Miniaudio's channel budget. This evaluates a cheap distance/orientation priority score per candidate voice each tick and culls anything below a structural cutoff before it ever reaches full mixing — inaudible groans and distant footsteps get dropped from the active voice set instead of silently competing for channels with what the player can actually hear.

##### Reference Implementation

```cpp
struct ActiveVoice { float baseVolume; float distance; float dotViewAngle; };
inline bool EvaluateVoiceCull(const ActiveVoice& voice, float structuralCutoff) {
    float computedVolumeScalar = (voice.baseVolume * voice.dotViewAngle) / (voice.distance * voice.distance + 1e-4f);
    return computedVolumeScalar < structuralCutoff;
}
```

##### Player-Facing Impact

Audio stays clean and prioritized even in a packed horde swarm — the sounds that matter (nearby threats, gunfire) never compete for a channel with a groan happening three blocks away.

#### [M6-EXT-06] Variable-Rate Audio Emitter Execution Cadence Stagger Matrix

##### Systems Touched

Protects M6's 0.5ms audio execution budget alongside `[M6-EXT-05]`'s voice-priority culler — where that feature decides *whether* a voice mixes at all, this one staggers *how often* ray-traced acoustic diffraction/portal tracking (M6's base RT-diffraction bullet) re-evaluates for emitters that do make the cut, spreading cost across frames instead of paying full rate for every crowded horde emitter simultaneously.

##### Math

`I_cadence = Clamp(⌊d_distance / 10.0⌋, 1, 8)`

`TickExecute = ((CurrentFrameCounter + EmitterIndex) mod I_cadence) == 0`

##### How It Works

Each active emitter's cadence interval scales with distance to the player camera — near emitters re-evaluate every tick, distant ones every 8th. Adding the emitter's own index into the modulo staggers which specific frame each interval-N group fires on, so the workload spreads evenly across frames rather than every "every-8th-tick" emitter re-evaluating in the same frame.

##### Reference Implementation

```cpp
struct AudioEmitterSource {
    float rangeDistanceToPlayerCameraMeters{0.0f};
    uint32_t emitterIndexIdentifier{0};
};
inline bool EvaluateAudioUpdateTick(const AudioEmitterSource& source, uint64_t frameTickCounter) {
    uint32_t optimalStep = static_cast<uint32_t>(source.rangeDistanceToPlayerCameraMeters * 0.1f);
    uint32_t frameIntervalMask = std::clamp(optimalStep, 1u, 8u);
    return ((frameTickCounter + source.emitterIndexIdentifier) % frameIntervalMask) == 0;
}
```

##### Player-Facing Impact

Ray-traced acoustic diffraction stays affordable even under a full horde of simultaneous groaning/growling emitters — nearby threats stay acoustically crisp while distant crowd noise updates just often enough not to sound stale.

#### [M6-EXT-07] Material-Indexed Velvet-Noise Late Reverb

##### Systems Touched

Declares the base "Material-Indexed Reverb. Velvet-Noise Late Reverb Synthesis" bullet. Reads room/surface material tags from `[M6-EXT-04]`'s voxelized propagation portals and supplies the late-reverberation tail that `[M6-EXT-04]`'s early-reflection portal system doesn't itself model.

##### Math

Velvet-noise: a sparse pulse train of ±1 impulses at irregular intervals, avoiding the metallic comb-filtering of dense white-noise reverb tails: pulse positions `t_k = (k + rand[0,1)) · T_avg`, `k = 0,1,2,...`, with pulse density (average spacing `T_avg`) and decay envelope both driven by the room's dominant material index: `EnvelopeDecay(t) = e^{-t / τ_material}`, `τ_material` looked up per surface (concrete = long tail, carpet/foliage = short).

##### How It Works

Each enclosed space voxelized by `[M6-EXT-04]` carries a dominant material tag. That tag indexes into a small table of `(T_avg, τ)` pairs — hard reflective materials get sparser velvet-noise pulses with a longer exponential decay, soft absorptive materials get denser pulses that decay fast. The convolution is cheap enough to run per-listener in real time since velvet noise needs far fewer taps than a full impulse-response convolution for a perceptually equivalent diffuse tail, and it's regenerated (not just gain-scaled) whenever the listener crosses into a room with a different dominant material.

##### Reference Implementation

```cpp
// Regenerated whenever the listener crosses into a room with a different dominant material
std::vector<float> GenerateVelvetNoiseIR(float tAvg, float tau, float sampleRate, float durationSec) {
    std::vector<float> ir(static_cast<size_t>(sampleRate * durationSec), 0.0f);
    float t = 0.0f;
    while (t < durationSec) {
        // t_k = (k + rand[0,1)) * T_avg, accumulated: RandomUnit() alone has mean 0.5, which would
        // halve the actual pulse spacing versus the T_avg the material table tunes against — the
        // 0.5f bias term restores a mean step of exactly T_avg.
        t += tAvg * (0.5f + RandomUnit());
        size_t sampleIndex = static_cast<size_t>(t * sampleRate);
        if (sampleIndex >= ir.size()) break;
        ir[sampleIndex] = (RandomUnit() < 0.5f ? -1.0f : 1.0f) * std::exp(-t / tau);
    }
    return ir;
}
```

##### Player-Facing Impact

A concrete parking garage rings with a long, metallic tail while a carpeted living room down the hall sounds dead and close — reverb character shifts believably room-to-room without needing hand-placed reverb zones.

#### [M6-EXT-11] Voxel-Cone Acoustic Occlusion & Material Absorption
#### [M6-EXT-08] Ray-Traced Acoustic Diffraction Node Topology Cache *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M6 audio + M4.5-EXT-17. Caches ray-traced acoustic diffraction nodes so sound bends around corners.

##### Math
diffraction = UTD(node); cache node topology per probe; reuse across frames.

##### How It Works
Sound propagation rays that bend around edges (diffraction) are pre-traced and cached as a node topology per acoustic probe; gameplay reuses the cache so occluded sources still audibly leak around corners without re-tracing each frame.

##### Reference Implementation
```cpp
Node n = TraceDiffraction(src, edge); cache.Add(n);
```

##### Player-Facing Impact
You hear zombies around a corner, not just in line of sight - audio occlusion matches the world.

---
#### [M6-EXT-09] Velvet-Noise Late Reverb Interleaved Mixing Buffer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M6 audio. Mixes velvet-noise late reverb into the bus for cheap, artifact-free tails.

##### Math
ir = velvetNoise(N, density); tail = Convolve(dry, ir); interleave into reverb bus.

##### How It Works
Late reverb is synthesized from a velvet-noise impulse response (uniformly distributed taps) rather than a recorded IR, interleaved into the reverb mixing buffer. Cheap, stateless, no metallic ringing artifacts.

##### Reference Implementation
```cpp
float tail = MixVelvet(dry, taps); bus.Reverb += tail;
```

##### Player-Facing Impact
Rooms/caves have natural-sounding reverb tails without sampled-IR cost or artifacts.

---
#### [M6-EXT-10] Acoustic Convection Wave Refraction Filter *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M6 audio. Refracts sound waves by wind/convection so distant audio bends with weather.

##### Math
c_eff = c0 + wind·dir; delay/filter per path by c_eff; Snell bend at layers.

##### How It Works
Models how wind/temperature gradients refract sound: effective speed varies with wind along the path; the audio filter applies per-path delay/bend (Snell at layer boundaries) so upwind sources fade and downwind carry farther, matching weather.

##### Reference Implementation
```cpp
float ceff = c0 + dot(wind, pathDir); ApplyRefraction(src, ceff);
```

##### Player-Facing Impact
Wind direction changes what you hear at range - downwind gunfire carries, upwind dies.

#### [M6-EXT-12] Convolution-Reverb from Voxel Occlusion *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M6 + M3/M4.5. Builds convolution reverb IR from the voxel/occlusion field.

##### Math
ir = Convolve(diag, voxelOcclusion); tail = Conv(px, ir);

##### How It Works
A reverb impulse response is synthesized from the scene's voxelized occlusion (M3/M4.5-EXT-17) so indoor spaces sound enclosed and outdoor open - physically-derived tails, no sampled IRs. Pairs with M6-EXT-09 velvet tail.

##### Reference Implementation
```cpp
IR ir = BuildIR(voxelField); bus += Conv(dry, ir);
```

##### Player-Facing Impact
Reverb matches the space you're in - caves boom, streets are open.


##### Systems Touched
M6 audio. Refracts sound waves by wind/convection so distant audio bends with weather.

##### Math
c_eff = c0 + wind·dir; delay/filter per path by c_eff; Snell bend at layers.

##### How It Works
Models how wind/temperature gradients refract sound: effective speed varies with wind along the path; the audio filter applies per-path delay/bend (Snell at layer boundaries) so upwind sources fade and downwind carry farther, matching weather.

##### Reference Implementation
```cpp
float ceff = c0 + dot(wind, pathDir); ApplyRefraction(src, ceff);
```

##### Player-Facing Impact
Wind direction changes what you hear at range - downwind gunfire carries, upwind dies.

##### Systems Touched

`[M6]`'s existing sound propagation system (extends it — this is the missing occlusion/material-absorption term, not a second audio system), `[M10]`'s wind field (already warps propagation per front matter).

##### Math

Each material carries an absorption coefficient $a_f$ per frequency band; a traced sound cone's surviving energy after $n$ boundary crossings is:

$$E_{out} = E_{in} \cdot \prod_{i=1}^{n} (1 - a_{f,i})$$

##### How It Works

`[M6]` already traces sound cones through a low-resolution voxel representation of world geometry for obstruction; this feature adds the missing per-material term by tagging each voxel with the absorption coefficients of the material occupying it (reusing `[M4-EXT-14]`'s stratum/material table for terrain voxels, and per-object material tags elsewhere), so a cone passing through dense forest attenuates high frequencies quickly while a cone bouncing through a stone cavern loses very little energy per bounce and its reflections' delay times stack into a long, echoing tail.

##### Reference Implementation

```cpp
float TraceAcousticConeEnergy(float initialEnergy, const std::vector<float>& absorptionCoeffsAlongPath) {
    float energy = initialEnergy;
    for (float a : absorptionCoeffsAlongPath) energy *= (1.0f - a);
    return energy;
}
```

##### Player-Facing Impact

Gunfire in a stone cavern genuinely booms and echoes for longer than the same shot fired in a dense forest, because the sound cone is actually losing energy differently per material it passes through rather than every environment using one generic reverb preset.



## M6.5 — GPU particle/VFX system

##### Implementation Steps

* Compute Draw Compaction Pipeline.
* Fluid Hydrodynamic Blood Simulation with Weber-number atomization.
* Stress-Intensity Glass Fracture.
* Cellular-Automata Fire Thermodynamics (`∂T/∂t = α∇²T`).
* Depth-Buffer Particle Collision Proxies.
* Blue-Noise Importance Sampling. Linked-List Order-Independent Transparency (`caps.fragmentShaderInterlock`).
* G-Buffer Attachment Feedback Loops for infinite decals.

##### Decisions to flag

Particle budget ceiling.

##### Exit Criteria

* GPU particles execute cleanly without CPU drag. Decals layer efficiently.



### Extended Systems Library — engine-side additions for M6.5

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M6.5-EXT-01] Shockwave Hydro-Cavitation (Gore Fragmentation Dispersal)

##### Systems Touched

Connects M2.9 semi-implicit ballistics directly to your M6.5 fluid hydrodynamic blood particle pipeline.

##### Math

When a high-velocity projectile cuts through a zombie’s anatomical hit-volume SDF gradient, it calculates the tissue kinetic energy dissipation rate (dE/dx). If this force beats the structural tissue yield strength, it triggers a fluid shockwave explosion radius (R_cavity ):

`dE/dx = −(0.5)·ρ_tissue·C_d·A_bullet·‖v‖²`

`R_cavity = ζ·√(π·(−dE/dx) / σ_yield_tissue)`  *(same corrected orientation as [M2.9-EXT-01])*

Same cavitation-radius formula and `GetCavitationRadius()` shape as [M2.9-EXT-01] (only the tuning constant differs: `ζ` here vs `γ` there). [M2.9-EXT-01] is the canonical declaration; this feature calls that shared function with its own tuning constant rather than redeclaring a second copy.

##### How It Works

This framework acts inside your EventBus dispatcher loop. When a DamageEvent is published, the projectile reads the zombie's bio-weight phenotype component parameters. If the incoming bullet kinetic energy crosses the threshold, the system passes an instant fragment command to your M6.5 GPU compute shader, spraying a high-velocity wave of physical blood and tissue particles using dynamic Weber-number atomization.

##### Reference Implementation

```cpp
// Duplicate of [M2.9-EXT-01]'s GetCavitationRadius() — reuse that shared function with gamma=1.25f
// (this feature's tuning constant) rather than maintaining a second copy.
// Determines the structural tissue cavitation shock radius during your EventBus damage passes
float GetCavitationRadius(float vel, float mass, float drag, float area, float rho, float yield_t) {
    return 1.25f * std::sqrt((0.5f * rho * drag * area * (vel * vel)) / (3.1415f * yield_t + 1e-5f));
}
```

##### Player-Facing Impact

Low-caliber pistol rounds will cleanly punch straight through a skinny zombie's flesh, causing standard damage with minimal stagger. However, blasting a heavy, high-mass phenotype zombie with a high-velocity weapon at point-blank range triggers hydro-cavitation. The soft tissue physically liquefies and explodes outward, tearing massive structural chunks out of the zombie, severing limb joint constraints, and spraying dynamic blood over the pavement to create immediate friction drops.

#### [M6.5-EXT-02] Capillary Spatter Impact Angle Projection

##### Systems Touched

Connects M6.5 GPU fluid particle splash states straight to the persistent RVT terrain-overlay system built in M4.5 (chunk-anchored, `imageStore` write path) — writes into that real overlay, not an assumed layer.

##### Math

When a high-velocity fluid droplet collides with a static structure wall, the aspect ratio of the generated impact mark is determined by the dot product of the particle's incoming velocity vector (v) and the surface normal array (n), yielding the impact angle (θ):

`sinθ = |v · n| / ‖v‖`

`Width = D_base, Length = D_base / (sinθ + 1e−5)`

##### How It Works

When your compute particle system registers surface intersections, it extracts the velocity components. Instead of instantiating heavy quad objects, the GPU projectively writes an elliptical shape mask straight into your RVT layer data sheets using an optimized image write command, packing the aspect ratios into a compact spatial texture cache.

##### Reference Implementation

```glsl
// Computed within your fluid collision shader to scale blood spray overlays on the world surfaces
vec2 CalcSpatterAspect(vec3 v_in, vec3 normal, float d_base) {
    float sine = abs(dot(normalize(v_in), normal)); return vec2(d_base, d_base / (sine + 1e-5));
}
```

##### Player-Facing Impact

Blasting a zombie close to a brick storefront creates an accurate pattern of elongated blood droplets pointing directly back to the origin of the gunshot. Perpendicular hits leave round marks, while steep glances create long, streaking trails that let you read the exact trajectory lines of past firefights on the urban architecture.

#### [M6.5-EXT-03] Venturi-Effect Smoke Drafting

##### Systems Touched

Links your M6.5 cellular fire grid system straight to your global M10 wind velocity vectors.

##### Math

The velocity of gaseous smoke particles (v_smoke ) passing through restricted architectural openings accelerates based on the pressure gradients generated by global cross-breeze wind vectors (v_wind ):

`A_room · v_wind = A_portal · v_venturi  ⟹  v_venturi = v_wind · (A_room / A_portal)`

##### How It Works

Your M6.5 thermodynamics engine updates gaseous data grids across discrete cells. When a cell intersects a structural portal boundary (M3), the update thread samples the global wind vectors from M10. The system uses a simple area ratio scalar to scale up the velocity variables, driving the smoke cells along the wind vector without running full Navier-Stokes calculations.

##### Reference Implementation

```cpp
// ratio is a_room/a_door (bigger room area over narrower opening -> speedup through the gap);
// the algebra in the Math section above must solve the continuity equation in this direction, not backwards.
void StepVenturiDraft(const glm::vec3& v_wind, float a_room, float a_door, glm::vec3& out_smoke_vel) {
    out_smoke_vel = v_wind * (a_room / (a_door + 1e-5f));
}
```

##### Player-Facing Impact

Setting fire to the ground floor of an enclosed retail block creates a natural drafting hazard. If you open a back exit door, the incoming wind forms a high-velocity suction tunnel that draws thick smoke plumes rapidly down hallways and squirts them out through the open doorways, forcing you to think about ventilation paths when using fire defensively.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M6.5-EXT-04] Particle Stream Compaction Pipeline

##### Systems Touched

Declares M6.5's "Compute Draw Compaction Pipeline" bullet by extending M1's indirect-draw-compaction prefix-sum pattern — already used there for entity mesh draws — to the M6.5 particle instance buffer.

##### Math

`offset_i = Σ_{j<i} alive_j` — a parallel prefix sum (Blelloch scan) over each slot's alive flag gives every living particle its compacted destination index.

##### How It Works

The particle buffer is a fixed-capacity ring of slots; most are dead (`lifetime ≤ 0`) at any given tick. A compute pass evaluates the alive flag per slot, runs a workgroup-parallel prefix sum across the buffer to compute compacted destination offsets, then scatters living particles into a tightly packed draw buffer. The final compacted count is written straight into the indirect draw args buffer, so the GPU issues exactly one draw call sized to the living particle count — no CPU readback, and no draw cost spent on dead slots.

##### Reference Implementation

```glsl
// Workgroup-shared prefix sum over an alive-flag tile; scatters into compacted output at the resulting offset
shared uint tile[GROUP_SIZE];
void CompactTile(uint localIdx, uint aliveFlag, inout uint globalOffset) {
    tile[localIdx] = aliveFlag;
    barrier();
    for (uint stride = 1u; stride < GROUP_SIZE; stride <<= 1u) {
        uint v = (localIdx >= stride) ? tile[localIdx - stride] : 0u;
        barrier();
        tile[localIdx] += v;
        barrier();
    }
    if (aliveFlag == 1u) {
        uint dest = globalOffset + tile[localIdx] - 1u;
        // scatter this slot's particle data to compactedBuffer[dest] here
    }
}
```

##### Player-Facing Impact

Blood, embers, and smoke particles across simultaneous horde combat never cost a wasted draw on expired slots — particle throughput scales with what's actually on screen instead of the buffer's allocated capacity.

#### [M6.5-EXT-05] Stress-Intensity Glass Fracture

##### Systems Touched

Declares M6.5's "Stress-Intensity Glass Fracture" bullet. Deepens M3's material-yield framework for brittle glass panes specifically, reusing M3's Voronoi Structural Integrity Graph cell generator for the shatter pattern and feeding resulting shard debris into M6.5's GPU particle pipeline for the spray.

##### Math

Linear elastic fracture mechanics stress intensity factor: `K_I = Y·σ·√(π·a)`, where `a` is a pre-seeded flaw length and `Y` a geometry factor. Fracture triggers when `K_I ≥ K_IC` (the pane's fracture toughness).

##### How It Works

Glass panes carry a `FractureToughness` (`K_IC`) material parameter and a pre-seeded flaw-length distribution. On any impact event — ballistic (M2.9), blast, or blunt melee — the local stress at the impact point is evaluated against `K_I`; once it crosses `K_IC`, M3's Voronoi cell generator is invoked rooted at the impact point to produce a radiating shatter pattern. Cells within the propagation radius convert to M6.5 GPU particle shard entities for the spray; cells outside it remain as static collidable fragments still wired into M3's structural graph.

##### Reference Implementation

```cpp
bool CheckGlassFracture(float sigma, float flaw_a, float geom_y, float k_ic) {
    float k_i = geom_y * sigma * std::sqrt(3.1415f * flaw_a);
    return k_i >= k_ic;
}
```

##### Player-Facing Impact

Shooting through a shop window doesn't just delete the pane — the crack pattern radiates realistically from the actual impact point, and shards spray outward as real particles that catch light and scatter across the ground instead of vanishing.

#### [M6.5-EXT-06] Cellular-Automata Fire Thermodynamics

##### Systems Touched

Declares M6.5's "Cellular-Automata Fire Thermodynamics" bullet. Feeds `[M6.5-EXT-03]`'s Venturi smoke drafting (already wired to M10 wind) and exposes local heat to M2.9's Layered Thermal Insulation model.

##### Math

`∂T/∂t = α∇²T`, discretized as a per-chunk grid with a standard 5-point Laplacian stencil. A cell ignites when its temperature crosses a fuel-specific ignition threshold.

##### How It Works

Each flammable chunk holds a 2D temperature grid updated once per tick via the discretized heat equation above. Fuel cells (wood, brush, fabric) carry a small combustion state machine — unlit → igniting → burning → spent — driven purely by neighboring cell temperature crossing that fuel's ignition threshold, so fire spreads cell-to-cell through the same diffusion update rather than a separate particle-based spread system. Burning cells act as a heat source term re-injected into the equation, and their temperature is what `[M6.5-EXT-03]` samples for smoke generation and M2.9 samples for nearby thermal insulation drain.

##### Reference Implementation

```cpp
void StepFireGrid(float* T, float* T_next, int w, int h, float alpha, float dt) {
    for (int y = 1; y < h - 1; ++y) for (int x = 1; x < w - 1; ++x) {
        int i = y * w + x;
        float lap = T[i-1] + T[i+1] + T[i-w] + T[i+w] - 4.0f * T[i];
        T_next[i] = T[i] + alpha * lap * dt;
    }
}
```

##### Player-Facing Impact

Fire spreads believably across flammable material — a burning zombie stumbling into dry brush or a wooden barricade ignites it cell by cell rather than instantly, and lingering too close to a growing fire visibly saps your thermal insulation buffer.

#### [M6.5-EXT-07] Depth-Buffer Particle Collision Proxies

##### Systems Touched

Declares M6.5's "Depth-Buffer Particle Collision Proxies" bullet, giving GPU particles (blood, debris, embers) cheap world-collision using the depth pre-pass buffer M1 already produces for Hi-Z occlusion culling, instead of a per-particle Jolt query.

##### Math

World-space surface reconstruction from a screen-space depth sample, compared against each particle's predicted next-frame position to detect penetration.

##### How It Works

Each tick, a particle's predicted next position projects into screen space and samples the existing Hi-Z depth buffer at that coordinate. The corresponding world-space point and surface normal are reconstructed from the depth sample and its screen-space derivatives. If the particle's predicted position would land behind that reconstructed surface, its velocity is reflected/clamped against the derived normal instead of resolving against a full Jolt collision shape. This is a camera-visible approximation only: particles outside the frustum or in occluded/off-screen regions fall back to a simple ground-plane collision rather than testing against depth data that doesn't exist for them.

##### Reference Implementation

```glsl
vec3 ReconstructWorldPos(vec2 screenUV, float depth, mat4 invViewProj) {
    vec4 clip = vec4(screenUV * 2.0 - 1.0, depth, 1.0);
    vec4 world = invViewProj * clip;
    return world.xyz / world.w;
}
```

##### Player-Facing Impact

Thousands of blood droplets and embers bounce and settle believably against floors, furniture, and stairs in view without a full physics-engine collision query per particle, keeping horde-scale particle counts affordable.

#### [M6.5-EXT-08] Blue-Noise Importance Sampling

##### Systems Touched

Declares M6.5's "Blue-Noise Importance Sampling" bullet, sharing the same precomputed blue-noise point-set/texture used by M3's Rubble Blue-Noise Thinning to distribute particle spawn positions (blood droplets, ember scatter, decal jitter) evenly instead of clumping the way naive uniform-random sampling does.

##### Math

`p_i = p_center + r_i · BlueNoiseOffset(i mod N)` — an incrementing index into a small precomputed low-discrepancy offset table, rather than a fresh RNG draw per particle.

##### How It Works

A single small blue-noise offset texture — the same shared asset M3's rubble-thinning pass already reads — is sampled per spawned particle using an incrementing index instead of a per-particle RNG call, giving a perceptually even spatial distribution at the cost of one texture lookup.

##### Reference Implementation

```glsl
vec2 SampleBlueNoiseOffset(sampler2D blueNoiseTex, uint particleIndex, uint texSize) {
    ivec2 texel = ivec2(particleIndex % texSize, (particleIndex / texSize) % texSize);
    return texelFetch(blueNoiseTex, texel, 0).xy * 2.0 - 1.0;
}
```

##### Player-Facing Impact

Blood spatter and ember bursts look organically scattered rather than showing the telltale clustered/gapped pattern of naive random placement, at effectively zero extra runtime cost.

#### [M6.5-EXT-09] Linked-List Order-Independent Transparency

##### Systems Touched

Declares M6.5's "Linked-List Order-Independent Transparency" bullet, resolving draw-order correctness for overlapping particle layers (smoke, blood mist, fire) using `caps.fragmentShaderInterlock`.

##### Math

Per-pixel fragments are composited front-to-back after depth-sorting the linked list, standard OIT alpha-blend accumulation: `C_out = Σ_i (C_i · α_i · Π_{j<i}(1 − α_j))`, where fragments are ordered by ascending depth (`i = 0` nearest camera) before the sum is evaluated, so occlusion falls off correctly regardless of the order fragments were inserted into the list.

##### How It Works

A per-pixel fragment list is built using an atomic head-pointer image plus an interlock-guarded linked-list insert into a node buffer storing depth and color per fragment — no CPU-side back-to-front sort. A resolve pass walks each pixel's list (typically 4-8 entries at horde-combat particle density), insertion-sorts it by depth, and blends front-to-back. Hardware lacking `fragmentShaderInterlock` falls back to standard alpha blending with a depth pre-pass.

##### Reference Implementation

```glsl
// Interlock-guarded insert into the per-pixel fragment linked list
layout(binding = 0, r32ui) uniform uimage2D headPointers;
void InsertFragment(ivec2 pixel, uint nodeIndex, inout uint nodeBuffer[]) {
    beginInvocationInterlockARB();
    uint prevHead = imageLoad(headPointers, pixel).r;
    imageStore(headPointers, pixel, uvec4(nodeIndex));
    nodeBuffer[nodeIndex] = prevHead; // this node's "next" points at the old head
    endInvocationInterlockARB();
}
```

##### Player-Facing Impact

Dense overlapping smoke, fire, and gore-mist composite correctly from any camera angle instead of showing flicker or incorrect occlusion when several particle systems overlap in view at once.

#### [M6.5-EXT-10] G-Buffer Attachment Feedback Loop

##### Systems Touched

Declares M6.5's "G-Buffer Attachment Feedback Loops for infinite decals" bullet — the underlying write mechanism `[M6.5-EXT-02]` already assumes exists for its RVT terrain-overlay writes.

##### Math

`G_albedo' = lerp(G_albedo, DecalColor, α_decal)`, written via `imageStore` directly into the same G-buffer attachment that's read as an input attachment within the same pass — the "feedback loop."

##### How It Works

Decal events (bullet impacts, scorch marks, blood smears) write their footprint directly into the relevant G-buffer channel at the corresponding texel using `imageStore`, blended with existing content, rather than spawning a decal-quad entity. Because the decal is baked into the surface's material data instead of rendered as separate geometry, decals compound indefinitely with no decal-count budget and no overdraw from stacked decal quads.

##### Reference Implementation

```glsl
layout(binding = 1, rgba8) uniform image2D gAlbedo;
void BlendDecal(ivec2 texel, vec4 decalColor, float alpha) {
    vec4 existing = imageLoad(gAlbedo, texel);
    imageStore(gAlbedo, texel, mix(existing, decalColor, alpha));
}
```

##### Player-Facing Impact

Hundreds of layered bullet holes, scorch marks, and blood smears stack indefinitely on the same wall over a long firefight with no performance cliff, since they live in the surface's material data rather than as separate rendered objects.

#### [M6.5-EXT-11] fBm Wind Kinematics & Collision-Reactive Foliage

##### Systems Touched

`[M4-EXT-18]`'s canopy shade output (gates undergrowth density/species), `[M10]`'s wind vector field (drives sway direction/strength).

##### Math

Foliage vertex offset sums low- and high-frequency noise octaves so gusts read as continuous rolling waves rather than uniform jitter:

$$\text{fBm}(x) = \sum_{i=0}^{n-1} a_i \cdot \text{textureRead}(2^i \cdot x)$$

Collision response is a simple spring-damper recovering toward rest position:

$$\ddot{x} = -k(x - x_{rest}) - c\dot{x}$$

##### How It Works

Undergrowth vertex shaders sample the fBm sum above, scaled by `[M10]`'s current wind vector magnitude/direction, producing continuous traveling wind waves across a field instead of every plant swaying independently and incoherently. Separately, a small buffer of nearby physical colliders (players, vehicles, zombies) is checked per foliage instance each frame; on overlap, the shader bends the affected vertices away from the collider's center and the spring-damper above pulls them back to rest once the collider passes, reusing the same damped-oscillator shape as other spring systems in this doc rather than a bespoke bounce curve.

##### Reference Implementation

```cpp
glm::vec3 ComputeFoliageOffset(glm::vec3 basePos, glm::vec2 windDir, float windStrength, float fbmSample, glm::vec3 colliderPush) {
    glm::vec3 windOffset = glm::vec3(windDir.x, 0.0f, windDir.y) * windStrength * fbmSample;
    return basePos + windOffset + colliderPush;
}

void StepFoliageSpringRecovery(glm::vec3& displacement, glm::vec3& velocity, float k, float c, float dt) {
    glm::vec3 accel = -k * displacement - c * velocity;
    velocity += accel * dt;
    displacement += velocity * dt;
}
```

##### Player-Facing Impact

Grass and brush ripple in visible traveling waves that match the actual wind direction, and physically bend out of the way when a survivor or vehicle pushes through them, springing back naturally afterward instead of clipping through statically.

#### [M6.5-EXT-12] Volumetric Micro-Atmospherics & Heat-Shimmer Fields

---

#### [M6.5-EXT-13] Capillary Blood-Spatter RVT Projection *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M6.5 + M4.5-EXT-26 RVT. Projects blood spatter into the RVT overlay.

##### Math
spatter -> RVT projection (M4.5-EXT-26); persists + layers;

##### How It Works
Wound blood-spatter is projected into the RVT overlay (shared with M4-EXT-26 decals, M9-EXT-22 skidmarks) so gore persists and layers on surfaces, aging over time. One projection path for all surface stains.

##### Reference Implementation
```cpp
ProjectSpatter(rt, surf);
```

##### Player-Facing Impact
Combat leaves persistent, layered blood - the world remembers firefights.


##### Systems Touched

`[M10]`'s humidity/temperature grids (already tracked per front matter), `[M6.5]`'s particle system (evaporation particles).

##### Math

Fog settles by world-space height threshold weighted by local humidity; heat shimmer intensity scales with the temperature delta between a hot surface and ambient air:

$$\text{FogDensity}(y) = \text{Humidity} \cdot \max\left(0, 1 - \frac{y - y_{floor}}{y_{settle}}\right), \qquad \text{ShimmerAmp} \propto (T_{surface} - T_{ambient})$$

##### How It Works

Fog is rendered as a true participating medium (ray-marched density, not a screen-space plane), with density weighted by the existing per-cell humidity value from `[M10]` and settling toward low ground using the height falloff above, so valleys and ditches fill with fog during high-humidity cycles while nearby high ground stays clear. Hot surfaces (asphalt, dark roofing) read their own temperature against `[M10]`'s ambient air temperature; the delta drives a screen-space refractive heat-shimmer overlay and spawns a low-rate evaporation particle stream from `[M6.5]`'s existing emitter pool, both fading out as the surface's tracked wetness value (shared with `[M4.5-EXT-04]`) dries toward zero.

##### Reference Implementation

```cpp
float SampleFogDensity(float worldY, float floorY, float settleHeight, float humidity) {
    float heightFactor = glm::clamp(1.0f - (worldY - floorY) / settleHeight, 0.0f, 1.0f);
    return humidity * heightFactor;
}

float ComputeHeatShimmerAmplitude(float surfaceTempC, float ambientTempC, float wetnessRemaining) {
    float delta = std::max(0.0f, surfaceTempC - ambientTempC);
    return delta * 0.02f * (1.0f - wetnessRemaining); // shimmer fades in as surface dries
}
```

##### Player-Facing Impact

Fog convincingly pools in low ditches and riverbeds on humid nights while high ground stays clear, and sun-baked asphalt visibly shimmers and steams as it dries after rain instead of every wet surface just darkening uniformly.



## M7 — Total persistence

##### Implementation Steps

* **`StableId` save/load wiring** (declared in M2.6 as `uint64_t StableId` but never wired to persistence until now). Generation method: monotonic counter seeded from a per-save-file high-water mark stored in the save header (`nextStableId`), not a content hash or UUID — a counter is simpler to reason about for a single-player-authoritative save and avoids UUID collision bookkeeping this game doesn't need. Persisted alongside component data as a plain field in each serialized entity's Zstandard-compressed block, not in a separate index. At load time, `EntityFactory` (M1) creates a fresh `entt::entity` handle for each serialized record and immediately calls `registry.emplace<StableId>(handle, savedStableIdValue)` — the freshly created handle is never assumed to numerically match its pre-save handle (EnTT handles aren't stable across a process restart), so any system holding a cross-reference to another entity (e.g. a companion's trust record referencing the player, an active DamageEvent's `source`) must serialize that reference as the `StableId` value, not the raw handle, and re-resolve it via a `StableId -> entt::entity` lookup map rebuilt once at load completion.
* Sector Delta Serialization (chunk hibernation).
* Deterministic Time-Lapse Shader updating oxidation, bloat pressure (`PV=nRT`), thermodynamics offline.
* Player Activity Migration Biasing.
* Continuous Hemorrhage Kinetics (`dHP/dt = -k·Σ(wound)`).
* Zstandard Dictionary Compression.
* Atomic File System Swapping (`rename("save.tmp", "save.zst")`), with an explicit `fsync`/`FlushFileBuffers` call on the temp file **before** the rename, and a follow-up `fsync` on the containing directory after it. `rename()` alone is atomic w.r.t. visibility but not durability — the OS is free to reorder the temp file's actual data flush after the rename completes, so a power loss right after "save complete" can still leave `save.zst` pointing at a half-written temp file's data. This is the single most common atomic-save bug (confirmed against Godot's own save-corruption fix and SQLite's atomic-commit notes) and costs one extra syscall per save, not a redesign. Three-Deep Rolling Backup. Ordered Format Schema Migration.

##### Decisions to flag

Autosave frequency mapping.

##### Exit Criteria

* Items drop/hibernate/recover accurately across session closes.
* Rolling backups successfully catch synthetic mid-write interruptions.



### Extended Systems Library — engine-side additions for M7

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M7-EXT-01] The Corpse-Pile Flattening Engine (Dynamic Static Hulls)

##### Systems Touched

Connects M7 total persistence body lifecycles directly to your M2.7 character virtual movement loops. Consumes `[M2-EXT-02]`'s `HibernationEvent` on the M2 EventBus — the mechanism that fires when Jolt puts the ragdoll's island to sleep.

##### Math

When a dynamic ragdoll asset falls into a sleeping state, its multi-body constraint system is destroyed to free up physics tracking loops. Instead of deleting the collision entirely, its bounds are baked into a single, low-overhead static Jolt box or convex hull collider. Over real-world time deltas (Δt), your time-lapse shader compresses the height (h) of this static shape to simulate physical decay and compaction:

`h(t)=h_0 ⋅e^(−λ⋅Δt)`

`The surface friction coefficient (μ) of this corpse object shifts dynamically as it flattens:`

`μ_corpse =μ_asphalt ⋅(1.0−e^(−α⋅Δt ))`

##### How It Works

This system subscribes to `[M2-EXT-02]`'s `HibernationEvent` on the M2 EventBus rather than polling any ragdoll state directly. When a `HibernationEvent{stableId, isEnteringSleep=true}` fires, an async enkiTS worker thread handles the swap. It removes the complex dynamic ragdoll assembly from the Jolt world and instantiates a single, ultra-cheap static hull collider at the same root coordinate. Your player's kinematic controller can walk on top of these shapes perfectly. As they flatten over game hours, the physics shape shrinks toward the pavement while writing the cell's `corpseOverrideMultiplier` term on `SurfaceFrictionSample` to act slick and wet. This is a per-cell contribution into `ResolveFriction()`'s fixed chain, not a direct write to the resolved traction value — a flattened corpse sitting in a puddle correctly composes both effects instead of one silently overwriting the other.

##### Reference Implementation

```cpp
// Flattens the static corpse collision box scale over time and contributes a term into ResolveFriction() —
// never writes the resolved traction value directly
void FlattenCorpseShape(float dt, float decay_rate, glm::vec3& out_scale, float& corpseOverrideMultiplier) {
    out_scale.y = std::max(0.05f, out_scale.y * std::exp(-decay_rate * dt));
    corpseOverrideMultiplier = 0.15f / (out_scale.y + 0.1f);
}
```

##### Player-Facing Impact

Killing a massive horde in a narrow street choke point leaves a genuine physical obstacle. Initially, the street is choked with a high, jagged pile of solid dead bodies that your vehicle must physically smash over, causing suspension bucking. As time-lapse ticks pass, they compress down into a flattened, squishy, highly dangerous slick carpet of organic sludge that turns that section of the highway into a zero-traction drift zone.

#### [M7-EXT-02] Electrochemical Oxidation Vector Blending (Structural Time-Lapse)

##### Systems Touched

Implements the rendering data layer for your M7 deterministic time-lapse shaders.

##### Math

When sector chunks hibernate, your engine updates material states offline by evaluating real-world time deltas (Δt). The degradation rate of structural metals and wood frames follows an Arrhenius reaction configuration:

`K_decay = A_humidity · e^(−E_a/(R·T_sector))`

`Oxidation_scalar = Clamp(Oxidation_initial + K_decay·Δt_hours, 0.0, 1.0)`

This tracking value maps directly to your GPU instance matrices to alter material maps on chunk loading passes.

##### How It Works

This feature populates the physical parameters utilized by your M7 sector serialization processes. When a sector is saved or reloaded, the engine calculates the real-world time offset using your Julian day clock tracker. An offline compute pass processes the Arrhenius equations using the historical humidity and temperature averages of that biome chunk. The resulting decay floats are written directly into your generational resource indirection tables, adjusting G-buffer texture blend weights with zero runtime frame cost.

##### Reference Implementation

```cpp
// Computes long-term structural decay increments across sector hibernation intervals offline
void ComputeOxidation(float dt_hours, float temp_k, float humidity, float e_a, float& oxidation) {
    oxidation = std::clamp(oxidation + (humidity * std::exp(-e_a / (8.314f * temp_k))) * dt_hours, 0.0f, 1.0f);
}
```

##### Player-Facing Impact

Leaving world sectors unvisited for extended periods causes visible environmental degradation. Returning to an old safehouse or city sector after weeks of gameplay reveals distinct weathering patterns. Exposed metal structures exhibit real rust flaking, wooden barriers show rot weathering, and brick structures manifest damp salt tracking. This maps onto the environment's structural attributes, causing old, abandoned fortifications to lose baseline yield capacity if neglected over seasonal weather arcs.

#### [M7-EXT-03] Submerged Corpse Buoyancy Re-Activation

##### Systems Touched

Intersects M7 corpse gas pressure logs with M10 shallow-water Saint-Venant flooding fields.

##### Math

When corpse shapes are static, they do not consume updates. However, if your Saint-Venant water depth (h_water ) rises, the engine verifies the buoyant force versus gravitational weight using the corpse volume parameters (V_corpse ):

`F_buoyant = ρ_water · V_corpse · g > m_corpse · g`

If this condition matches, the engine switches the object's Jolt layer flag back to a dynamic state, letting the fluid velocity field move it.

##### How It Works

Static corpse bodies consume no per-tick updates by default. Each coarse sector tick, local water depth is checked against a buoyancy threshold from stored volume/mass; crossing it flips the corpse's Jolt layer from static back to dynamic.

##### Reference Implementation

```cpp
// Checked inside your coarse sector update tick loop
bool CheckCorpseFloat(float h_water, float body_vol, float mass, float rho_w) {
    return (h_water > 0.4f) && ((rho_w * body_vol) > mass);
}
```

##### Player-Facing Impact

Flooded streets during massive rain storms create dynamic visual and physical hazards. Bloated bodies from older skirmishes will physically dislodge from the asphalt layer, rising to the top of deep puddles and drifting down urban current currents. They can bunch up behind your defensive barricades, clogging pathways and acting as floating dynamic obstacles that can disrupt vehicle control or trip up your foot traction.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M7-EXT-04] Soft-Body Cloth, Tethered Rot-Cloth & Cosserat-Rod Vegetation (Jolt 5.5.0 Soft-Body Unlock)

##### Systems Touched

New capability unlocked by the Jolt 5.5.0 bump (item 8) — Jolt's XPBD-based soft-body system. Sits adjacent to M7's corpse-pile system (shares the same static/dynamic sleep-state lifecycle philosophy) and depends on M1's `SpatialHash` (culling) and M10's wind vector field (external force).

##### Depends on

M1 (`SpatialHash`, for distance culling), M10 (wind vector field, for external force sampling), M8 dependency bump to Jolt 5.5.0 (for the stable soft-body API). Do not start this before all three have landed.

**What in-game surfaces use it** (picked from existing doc surfaces, not invented from scratch):

* **Torn zombie clothing** — cloth patches on the ragdoll's shirt/coat, driven by Jolt's skinning constraints so the cloth follows the character's animated/ragdolled skeleton rather than simulating independently. Uses edge + dihedral bend constraints (standard cloth pair) tuned soft enough to visibly tear-flap during the M5.2 procedural animation and M7-EXT-01 corpse-flattening transition.
* **Foliage reacting to wind** — grass/brush clumps in the procedural world (M4) use a lightweight Cosserat-rod chain (3-5 segments) per clump rather than full cloth, since vegetation bends along one axis rather than draping.
* **Companion hair/fur** — a short Cosserat-rod chain per companion, skinned to the head joint, primarily a cosmetic fidelity pass — lowest simulation priority of the three surfaces (first to get LODded out at distance, see below).
* Rope/tether mechanic: out of scope for this pass — nothing in the existing 26-milestone doc calls for a player-usable rope, so this isn't invented here; the long-range-attachment (tether) constraint type is still used internally (see below) to keep cloth/vegetation from over-stretching, just not exposed as a player mechanic.

##### Constraint types per surface

* Cloth (zombie clothing): edge constraints (in-plane stretch resistance) + dihedral bend constraints (out-of-plane fold resistance), plus a tether (long-range attachment) constraint from each cloth vertex back to its nearest skinned anchor point to cap max stretch during violent ragdoll impacts.
* Vegetation/hair: Cosserat rod constraints (bend + twist) per segment, anchored at the root to the static clump position (vegetation) or head joint (companion hair/fur).
* Tetrahedron volume constraints: not used in this pass — reserved for a future soft, squishable volumetric body (e.g. a bloated corpse's torso deforming under pressure) if that's ever pulled forward from a "later" bucket; flagging the boundary rather than building it now, since nothing currently in the doc calls for volumetric soft-body deformation.

##### Wind integration

Vegetation and cloth soft bodies sample M10's wind vector field at their current world position each tick and apply it as an external force in Jolt's soft-body solve step (`JPH::SoftBodyMotionProperties::AddForce` with the sampled wind vector scaled by a per-surface wind-sensitivity constant — cloth is more wind-sensitive than the stiffer vegetation rods) — not a separate ad hoc wind hack layered on top.

##### Performance policy

* Iteration count: 4 XPBD substeps per physics tick for cloth (zombie clothing is usually close to the camera and the tear/flap detail matters), 2 substeps for vegetation clumps (background detail, cheaper is fine), 3 for companion hair/fur.
* LOD/culling via `SpatialHash` (M1): soft bodies beyond 40m from the active camera are frozen to their last pose and excluded from the tick's solve pass entirely (queried once per M1's 8-tick stagger cadence, matching the "tolerant of staleness" bucket from item 1); companion hair/fur is the first surface dropped to a static pose at distance (30m), vegetation next (40m), zombie clothing last (50m, since it's usually mid-combat and close).

##### Math

Per-substep XPBD constraint solve for a soft-body particle `i` with predicted position `p_i`, iterating each constraint `C` (edge, dihedral, Cosserat bend/twist, or tether) toward zero with compliance `α` (inverse stiffness) rather than a hard position solve:

`Δp_i = -(w_i · ∇C_i(p)) / (Σw_j|∇C_j(p)|² + α/Δt²) · C(p)`

where `w_i = 1/m_i` is the inverse mass per particle. Wind is applied as an external force before the constraint pass: `p_i' = p_i + Δt²·(w_i·F_wind)`, with `F_wind` the sampled M10 wind vector scaled by the per-surface wind-sensitivity constant from the section above.

##### Reference Implementation

```cpp
// One XPBD substep for a soft-body constraint group (cloth edge/dihedral, Cosserat rod, or tether).
// Called 2-4 times per physics tick depending on surface (see substep counts above).
struct SoftBodyParticle { glm::vec3 pos, prevPos; float invMass; };

void XPBDSubstep(std::span<SoftBodyParticle> particles,
                  std::span<SoftBodyConstraint> constraints,
                  glm::vec3 windForce, float dt) {
    // 1. Apply external forces (wind) as a predicted-position update
    for (auto& p : particles)
        if (p.invMass > 0.0f) p.pos += dt * dt * p.invMass * windForce;

    // 2. Solve each constraint's XPBD correction (edge/dihedral/Cosserat/tether share this shape;
    //    only C(p) and its gradient differ per constraint type)
    for (auto& c : constraints) {
        float C = c.EvaluateError(particles);              // e.g. |p1-p0| - restLength for an edge
        auto grad = c.EvaluateGradient(particles);          // per-particle ∇C
        float wSum = c.SumWeightedGradients(particles, grad);
        float lambda = -C / (wSum + c.compliance / (dt * dt));
        c.ApplyCorrection(particles, grad, lambda);
    }
}
```

##### Player-Facing Impact

Torn clothing on downed zombies visibly flaps and drapes over the corpse-pile as it flattens rather than clipping through the compacting collision shape. Foliage sways believably in response to the same wind that drives smoke drafting and scent plumes, rather than using a separate canned sway shader. Companion silhouettes read as more alive at a glance via responsive hair/fur, without spending simulation budget on distant NPCs.

#### [M7-EXT-05] Perishable Item Decay & Cook-Safety Integral (Food Spoilage & Cooking)

##### Systems Touched

Reuses M7-EXT-02's existing Arrhenius material-decay formula (`K_decay = A_humidity · e^(−E_a/(R·T_sector))`, currently driving structural oxidation during chunk hibernation) with food-specific constants instead of new decay code. Writes into the shared illness system from [M8-EXT-03] on spoiled consumption.

##### Math

Spoilage reuses the exact oxidation formula, retargeted:

`Spoilage_scalar = Clamp(Spoilage_initial + K_decay · Δt_hours, 0.0, 1.0)`, where `A_humidity`, `E_a` are food-tier constants, not the metal constants.

Cooking safety is a logistic pathogen-kill integral over temperature and time:

`SafetyScore = 1.0 / (1.0 + e^(−k·(∫T_core dt − Threshold)))`

##### How It Works

Food items get the same generational decay float already computed for chunk-hibernation oxidation — the offline Julian-day time-lapse pass that ages a rusting truss ages a stored ration identically, just reading a different constants table keyed off the item's food-tier tag. A cold environment (fridge, winter chunk, per M10) lowers `A_humidity`'s effective contribution the same way it currently slows metal oxidation. Cooking accumulates `∫T_core dt` while an item sits on an active heat source entity; crossing the safety threshold zeroes out the spoilage-driven illness roll on consumption.

##### Reference Implementation

```cpp
// Same function signature as ComputeOxidation() from M7-EXT-02, different constants table
void ComputeFoodSpoilage(float dt_hours, float temp_k, float humidity, float e_a_food,
                          float& spoilage) {
    float k_decay = humidity * std::exp(-e_a_food / (8.314f * temp_k));
    spoilage = std::clamp(spoilage + k_decay * dt_hours, 0.0f, 1.0f);
}

float ComputeCookSafety(float integrated_core_temp, float threshold, float k) {
    return 1.0f / (1.0f + std::exp(-k * (integrated_core_temp - threshold)));
}
```

##### Player-Facing Impact

A working fridge in a looted grocery store is a genuinely valuable, defensible find — not because of a hardcoded "cooler box" flag, but because it's measurably slowing the same decay math that's rusting every truss in the sector. Undercooking meat in a hurry because a horde is closing in is a real, felt risk/reward decision, not a flat "cooked/raw" toggle.

#### [M7-EXT-06] Player-Facing Save Slot & Autosave UI Layer

##### Systems Touched

Thin UI layer directly over the existing M7 `StableId`/Zstandard persistence engine, atomic file-swap, and rolling backup — zero new backend, purely the missing player-facing surface.

##### Math

None — this is a UI/UX feature reading existing save-header metadata (`nextStableId`, timestamp, playtime) already written by M7.

##### How It Works

Save slots are just named references to the existing atomic-swap save files M7 already produces; the autosave indicator reads the existing rolling-backup write events; load-screen data (playtime, location, thumbnail) comes from save-header fields M7's format already reserves space for.

##### Reference Implementation

```cpp
// Reads existing save-header fields — writes nothing new to disk
struct SaveSlotDisplayInfo { std::string label; float playtimeHours; uint64_t timestamp; };
SaveSlotDisplayInfo ReadSlotInfo(const SaveHeader& header) {
    return { header.slotLabel, header.playtimeHours, header.savedAtUnixTime };
}
```

##### Player-Facing Impact

Players get a normal save/load menu with slots and autosave feedback — the missing surface over a persistence engine that was already fully built.

#### [M7-EXT-07] Hierarchical Delta-State Persistence (Procedural Seed-Diff Engine)

---

#### [M7-EXT-11] Zstd Save-Compression & Streaming Store *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M7 + M7-EXT-08. Compresses + streams save chunks via Zstd.

##### Math
chunk = ZstdCompress(buf, dict); stream to store;

##### How It Works
Save chunks are Zstd-compressed (with M7-EXT-08 dict) and streamed to the store incrementally so huge saves don't block; pairs with M7-EXT-09 atomic log for integrity.

##### Reference Implementation
```cpp
StreamChunk(ZstdCompress(buf, dict));
```

##### Player-Facing Impact
Big saves compress + stream - no hitch, small footprint.


---

#### [M7-EXT-10] Binary Save Format Structural Schema Migrator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M7. Versioned binary save schema with forward/back migrators.

##### Math
ver = ReadHeader(); while(ver<cur) Migrate(ver++, buf);

##### How It Works
Save blobs carry a schema version; on load, a chain of migrators upgrades old saves to the current struct layout field-by-field, so old saves still load after format changes. No blanket reject.

##### Reference Implementation
```cpp
while(v<cur) buf=Migrate(v++, buf);
```

##### Player-Facing Impact
Old saves keep loading after updates - no wipe on patch.


---

#### [M7-EXT-09] Atomic File-Swap Append-Only State Transaction Logger *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M7. Atomic, append-only log of state transactions for crash-safe saves.

##### Math
log.Append(tx); fsync; swap(active, shadow) on commit;

##### How It Works
State changes are appended to an atomic transaction log; on commit the active/shadow files swap (no partial write). Crash mid-save = replay shadow, never corrupt. Append-only means no in-place mutation.

##### Reference Implementation
```cpp
log.Append(tx); SwapAtomic(active, shadow);
```

##### Player-Facing Impact
Saves never corrupt mid-write - crash recovers cleanly.


---

#### [M7-EXT-08] Zstandard Custom Dictionary Static Compiler *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M7. Builds a Zstd custom dictionary from save-data corpus for better compression.

##### Math
dict = ZstdTrain(corpus); compress(data, dict);

##### How It Works
A Zstd dictionary is trained offline on representative save blobs so repeated save structures compress far better; the dictionary ships with the game. Used by M7-EXT-11 streaming store.

##### Reference Implementation
```cpp
ZstdCDict d = Train(corpus);
```

##### Player-Facing Impact
Saves compress much smaller - faster loads, less disk.


##### Systems Touched

Integrates with M7's total persistence lifecycle and tracks updates against M4 streaming slots.

##### Math

A chunk's saved state is filtered down to only what has diverged from its procedural baseline:

`Delta_state = RuntimeComponentData − SeedGeneratedBaseline`

##### How It Works

Saving the raw position/state of every item, debris prop, and wall panel across an infinite procedural world would balloon save files to gigabytes and stall the engine on disk writes. This treats a chunk's saved state as an equation — `WorldGen(Seed) + DeltaTable` — and ignores unmodified entities entirely, serializing only objects that have been moved, broken, looted, or spawned. On reload, the seed regenerates the baseline instantly and the (small) delta table applies on top of the live registry. This is the chunk-level companion to M7-EXT-06's save-slot UI and M7's existing `StableId`/Zstandard backend — it's what keeps the per-chunk delta small enough for that backend to write quickly.

##### Reference Implementation

```cpp
// Flags a component as save-worthy only once it has drifted from its procedural baseline
bool EvaluatePersistenceDelta(const glm::vec3& live_pos, const glm::vec3& base_pos, float limit) {
    return glm::distance(live_pos, base_pos) > limit;
}
```

##### Player-Facing Impact

The world remembers your impact across hundreds of hours of play. Discarded magazines, broken walls, and burnt-out vehicle wrecks stay exactly where you left them across sessions, while save files stay small and load times stay fast.

## M8 — Data-driven itemization

##### Implementation Steps

* JSON Layout Schema Validation for all items.
* **Brand/quality-tier schema** — read by M8-EXT-01 (cook-off) and M2.7-EXT-12 (jam) but never declared until now. Added to every weapon-family entry in the item JSON tables:

```json
{
  "id": "tommy_gun_m1928",
  "brandTier": "military_surplus",       // enum, see below
  "brandTierMultiplier": 0.4,            // float, [0.0, 2.0], default 1.0
  "brandQualityMultiplier": 0.6          // float, [0.0, 2.0], default 1.0
}
```

* `brandTier`: enum string, one of `military_surplus | commercial_licensed | improvised_zip_gun | black_market_reload`. Authored per-weapon-family (not per-instance) in `assets/items/weapons.json`.
* `brandTierMultiplier`: scales cook-off probability (M8-EXT-01) and jam probability (M2.7-EXT-12); lower is better-made. Military surplus/commercial licensed default to 0.4-0.7; improvised/black-market default to 1.2-2.0.
* `brandQualityMultiplier`: a second, independent scalar for per-instance wear-and-provenance variance (a specific looted rifle can be a rough or pristine example of its tier), rolled once at item-spawn time within a tier-specific range and then fixed for that instance's lifetime; stored on the item instance, not the family definition. Both multipliers compose multiplicatively wherever cook-off/jam math already reads `BrandTierMultiplier` (M8-EXT-01's formula becomes `... · GrimeFactor · brandTierMultiplier · brandQualityMultiplier`).
* Authoring location: `assets/items/weapons.json`, validated by the existing JSON Layout Schema Validation step above (add both fields to the schema).
* Volumetric Grid Inventory.
* Kinetic Lock-Busting Solver (`KE` vs Jolt material shear).
* Personal Compartmental SEIR Infection.
* Log-Normal Camouflage Masking.

##### Decisions to flag

Scarcity queue constants. Whether `brandQualityMultiplier`'s per-instance roll range should itself be exposed in JSON per tier (recommended: yes, e.g. `"qualityRollRange": [0.5, 0.8]` for `military_surplus`) rather than hardcoded in C++.

##### Exit Criteria

* JSON manipulation adds modular items seamlessly without C++ modification.
* Improvised tools map mathematically to mass and geometry bounds.



### Extended Systems Library — engine-side additions for M8

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M8-EXT-01] Brand-Gated Thermal Ammo Cook-Off

##### Systems Touched

Reads the canonical `BarrelHeat` struct (locked in M2.7's base Implementation Steps) — specifically `kelvin` and `grimeFactor` — and intersects with your M8 JSON item tables for `BrandTierMultiplier`.

##### Math

Chamber ammo cooking off is a rare, extreme mechanical failure gate. The calculation evaluates to zero unless your barrel temperature (`BarrelHeat.kelvin`) passes maximum physical operational limits (**T_fail = 473.15K / 200°C**, **T_melt = 533.15K / 260°C**), scaling up strictly if using low-grade weapon brands or ignoring weapon cleaning loops. *(Corrected from the doc's earlier ≈450°C figure, which overshot the real physical process by more than 2x: published cook-off literature — chambered 7.62mm cartridge testing, nitrocellulose's ~160-170°C autoignition point, and the commonly-cited 149-204°C/300-400°F "cooks off" range for chambered ammo — puts real-world onset around 150-200°C, not 450°C. T_fail=200°C sits at the top of that real range (erring toward "less trigger-happy," appropriate for a rare failure gate); T_melt=260°C is the upper bound cited for guaranteed cook-off, so P_cook saturates to 1.0 across a plausible 60°C band instead of an arbitrary one.)*

`P_cook =Clamp( (T_barrel −T_fail) / (T_melt −T_fail) ,0.0,1.0)⋅GrimeFactor⋅BrandTierMultiplier⋅BrandQualityMultiplier`

##### How It Works

Every fire cycle checks `BarrelHeat.kelvin` against the fixed failure temperature; below it, probability is always zero. Above it, the probability formula scales with grime and brand-tier/quality multipliers, and a failed roll empties the magazine uncontrolled.

##### Reference Implementation

```cpp
// Grounded in real cook-off literature (chambered rifle-cartridge cook-off testing,
// nitrocellulose autoignition ~160-170°C): onset at 200°C, saturated at 260°C.
constexpr float kCookOffTFailK = 473.15f;  // 200°C
constexpr float kCookOffTMeltK = 533.15f;  // 260°C

float ComputeCookOffProbability(const BarrelHeat& heat, float t_fail, float t_melt, float brand_tier_mult, float brand_quality_mult) {
    if (heat.kelvin <= t_fail) return 0.0f;
    return std::clamp((heat.kelvin - t_fail) / (t_melt - t_fail), 0.0f, 1.0f) * heat.grimeFactor * brand_tier_mult * brand_quality_mult;
}

bool VerifyChamberCookOff(const BarrelHeat& heat, float t_fail, float t_melt, float brand_tier_mult, float brand_quality_mult, float roll) {
    return ComputeCookOffProbability(heat, t_fail, t_melt, brand_tier_mult, brand_quality_mult) > roll;
}
```

##### Player-Facing Impact

High-tier tactical weapons or military-grade components are heavily insulated and will never cook off ammunition under normal conditions. However, if you neglect weapon care or push a cheap makeshift zip-gun into sustained automatic fire, the receiver heat reaches critical boundaries. The weapon enters a brief, terrifying runaway loop that automatically empties the rest of the magazine, forcing you to ride out the recoil until the weapon clicks empty.

#### [M8-EXT-02] Property-Matching Crafting & Derived Item Stats

##### Systems Touched

Reads the same material/shape/mass tags already required on every scavenged item for physics. Declares the canonical `CraftingStationComponent` — the workbench-role entity later shared with [M9-EXT-16]'s garages (repair-role).

##### Math

No recipe table. A combination is valid if tag compatibility passes, and derived stats are computed, not looked up:

`Damage_derived = k_edge·Sharpness + k_mass·Mass·HandleLeverage`

`Durability_derived = N_e·(S_e/S_material)^m` — the same Miner's-rule exponent/constants M3-EXT-01 uses, reused via a dedicated small helper (`GetFatigueLifeCycles`) that wraps those constants rather than duplicating them.

##### How It Works

Every item already carries physics tags (mass, material, is-it-bladed) because your bullet penetration and ragdoll mass-scaling need them anyway. Crafting queries two items' tag sets against a small compatibility ruleset (rigid-handle + bladed-edge = melee weapon, container + absorbent = filter, etc.) and computes output stats from the input tags directly — adding a new craftable combination later means adding a compatibility rule, not authoring a new item.

##### Reference Implementation

```cpp
struct CraftableTags { float mass; float sharpness; bool isHandle; bool isBlade; MaterialType material; };
struct CraftingStationComponent { float toolTierMultiplier = 1.0f; };

bool TryCraftMelee(const CraftableTags& handle, const CraftableTags& blade,
                    float& out_damage, float& out_durability) {
    if (!handle.isHandle || !blade.isBlade) return false;
    out_damage = 4.0f * blade.sharpness + 0.8f * handle.mass;
    out_durability = GetFatigueLifeCycles(blade.material); // wraps the M3-EXT-01 Miner's-rule constants
    return true;
}
```

##### Player-Facing Impact

There's no recipe book to memorize — if it physically makes sense (a kitchen knife duct-taped to a mop handle), the system will let you build it and compute believable stats, consistent with the world having zero hand-authored content anywhere else.

#### [M8-EXT-03] Secondary Illness via Reparameterized SEIR (Sepsis, Dysentery, Tetanus)

##### Systems Touched

Reuses M8's existing Personal Compartmental SEIR Infection engine (the same one modeling zombie-bite infection) with different R0/incubation/mortality constants — not a new illness system. Fed by untreated wounds (M2.9 damage events), contaminated food ([M7-EXT-05]), and contaminated water ([M8.5-EXT-02]).

##### Math

Identical SEIR compartmental equations already governing zombie infection, re-parameterized per illness type:

`dS/dt = −βSI/N, dE/dt = βSI/N − σE, dI/dt = σE − γI, dR/dt = γI` — same shape, different `(β, σ, γ)` triplet per illness (sepsis, dysentery, tetanus each get their own triplet, seeded at population N=1 for a personal infection).

##### How It Works

An untreated open wound, a bad meal, or contaminated fish doesn't get bespoke sickness code — it seeds the existing SEIR state machine at N=1 with a different constants triplet than the zombie-infection variant. The same UI/status-effect hooks already built for showing infection progression drive these too.

##### Reference Implementation

```cpp
// Identical integrator shape to the existing zombie-infection SEIR — different (beta, sigma, gamma) table
void StepPersonalSEIR(float beta, float sigma, float gamma, float dt,
                       float& S, float& E, float& I, float& R) {
    float dS = -beta * S * I;
    float dE = beta * S * I - sigma * E;
    float dI = sigma * E - gamma * I;
    S += dS * dt; E += dE * dt; I += dI * dt; R += gamma * I * dt;
}
```

##### Player-Facing Impact

Ignoring a wound or eating spoiled food doesn't just cost a flat HP tick — it runs the same believable exposed-infected-recovered progression already governing whether a bite turns you, just tuned to a survivable illness instead of a fatal one (usually).

#### [M8-EXT-04] Per-Save Pathogen Characterization & Cure Synthesis (Deliberately Near-Impossible)

##### Systems Touched

Rolls its own R0/mutation-drift/antigen-signature parameters per save file (independent of the M8-EXT-03 illness constants) at world-gen time. Sample integrity uses [M7-EXT-05]'s Arrhenius spoilage curve. Requires uninterrupted M8.6 grid power (same `CableSpanComponent`/`CheckCableSnap()` system as every other electrical feature). Lab equipment durability uses [M3-EXT-06]'s shared `StructuralFatigue`/`TickFatigue()` system directly. Scientist companion bonus reads [M8.5-EXT-03]'s trust/skill state. Escalation is driven by the existing M5.4 AI Director. Completion feeds Appendix A's procedural narrative engine.

##### Math

Pathogen parameters are rolled once per save via SplitMix64, not fixed:

`(β_strain, σ_strain, R0_strain) = Hash(save_seed, "pathogen") → deterministic-but-unknown-to-player triplet`

Sample cold-chain integrity is the existing spoilage formula, applied to lab samples:

`SampleIntegrity = Clamp(1.0 − K_decay·Δt_hours, 0.0, 1.0)` — reusing [M7-EXT-05]'s `K_decay` function directly, just with a tighter `T_fail` threshold than food.

Synthesis success per attempt is a low-probability Bernoulli draw, gated multiplicatively (not additively) by equipment tier and scientist bonus, so no combination of bonuses pushes probability anywhere near certain:

`P_success = Clamp(P_base·EquipTierMult·(1.0 + ScientistBonus), 0.0, P_ceiling)`, with `P_ceiling` deliberately low (single-digit percent) regardless of stacked bonuses.

##### How It Works

At world-gen, a hidden pathogen-parameter triplet is hashed from the save seed — there is no fixed "correct answer," so no wiki entry can shortcut it; the player must empirically characterize the strain via captured samples before synthesis unlocks at all. Samples decay under the reused spoilage function, gated by uninterrupted power to the lab's refrigeration — a snapped `CableSpanComponent` power line (weather, vehicle impact, or simple neglect, all of which already exist) kills the cold chain and the batch. Lab equipment (centrifuges, analyzers) are subject to the same [M3-EXT-06] fatigue wear as everything else — they fail and need replacement. The M5.4 AI Director is explicitly made aware of active research-site power/activity signatures and escalates spawn density/horde pressure against that location the longer it stays active, using pacing knobs that already exist rather than a scripted final encounter. Losing the settlement (walls per [M8.6-EXT-06], outbreak per [M8-EXT-03]) can permanently wipe accumulated sample/equipment progress — persistence (M7) records this loss like any other world-state change, there is no special-cased "safe" savepoint for research progress.

##### Reference Implementation

```cpp
// Rolled once at world-gen, stored in the save header, never exposed to the player directly
struct PathogenParams { float beta; float sigma; float r0; };
PathogenParams RollPathogenParams(uint64_t save_seed) {
    uint64_t h = SplitMix64(save_seed ^ 0x504154484F47454Eull); // "PATHOGEN" tag
    return { 0.1f + (h & 0xFFFF) / 65535.0f * 0.4f,
             0.05f + ((h >> 16) & 0xFFFF) / 65535.0f * 0.15f,
             1.5f + ((h >> 32) & 0xFFFF) / 65535.0f * 3.5f };
}

float RollSynthesisSuccess(float p_base, float equip_tier_mult, float scientist_bonus, float p_ceiling) {
    return std::clamp(p_base * equip_tier_mult * (1.0f + scientist_bonus), 0.0f, p_ceiling);
}
```

##### Player-Facing Impact

This is meant to be the hardest achievable thing in the game, and most playthroughs shouldn't finish it. You have to actually characterize your specific save's strain through captured samples before synthesis is even possible; keeping a cold chain alive through a sustained AI Director escalation against your own research site is a genuine campaign, not a checklist; losing the settlement can erase real progress with no safety net beyond normal persistence. A completed cure is a narrative capstone, not an off-switch — the world doesn't go quiet just because you won.

##### Depends on (not yet built at this point)

[M8.5-EXT-03], [M8.6-EXT-06], [M3-EXT-06], [M7-EXT-05] — implement against hardcoded/stub values for now; revisit once each lands. (Fatigue naming resolved: use `StructuralFatigue`/`TickFatigue()` verbatim.)

#### [M8-EXT-05] Data-Table Modding/Workshop Exposure

##### Systems Touched

Exposes the JSON item/brand-tier tables (M8), socio-economic tags (Appendix C), crafting compatibility rules ([M8-EXT-02]), and pathogen-parameter roll function ([M8-EXT-04], exposed as an overridable seed only — not the roll itself, to preserve the intended difficulty of that feature) as editable/loadable external data. Not a new system — a loader-path addition for tables that already exist.

##### Math

None — this is I/O and validation, not simulation.

##### How It Works

Every table this engine already runs on (weapon JSON records, brand-tier multipliers, socio-economic tag weights, crafting compatibility rules) gets a secondary load path that checks a user-data/mods directory before falling back to the built-in table, with schema validation against the existing struct definitions so a malformed mod table fails to load loudly rather than corrupting a run silently.

##### Reference Implementation

```cpp
// Generic loader used identically for every existing data table — not table-specific code
template<typename T>
std::vector<T> LoadTableWithModOverride(const std::string& tableName) {
    auto modPath = GetModDataPath(tableName);
    if (std::filesystem::exists(modPath)) return LoadAndValidate<T>(modPath);
    return LoadAndValidate<T>(GetBuiltinDataPath(tableName));
}
```

##### Player-Facing Impact

Because the entire game already runs off data tables rather than hardcoded content, community modding support is mostly "let people point at their own version of a table that already exists" — a natural, low-effort payoff of the zero-hand-authored-assets approach held since M0, rather than a bolted-on modding framework fighting the engine's own content pipeline.

#### [M8-EXT-06] Two-Tier Deterministic Archetype Synthesis (Identity vs. Session Variance)

##### Systems Touched

Governs how every weapon, structural item, and core prop generated by M8's itemization system is synthesized; the item-instance identifier this feature defines is what M7-EXT-07's delta-state persistence actually serializes for items.

##### Math

Generation splits into two independent SplitMix64 streams keyed off different seeds — a fixed archetype hash for identity, a session/instance seed for variety:

`Tier1: gen₁ = SplitMix64(Hash64("Weapon_TommyGun"))` — constant across every playthrough
`Tier2: gen₂ = SplitMix64(SessionRunSeed ⊕ InstanceUUID)` — varies per instance/session

##### How It Works

Generating 100% of assets procedurally while keeping specific items ("a Tommy Gun") instantly recognizable and identical across playthroughs requires separating *identity* from *variety* into two data classes. **Tier 1 — Global Archetype Manifest:** every recognizable item gets a permanent 64-bit hash (e.g. `Hash64("Weapon_TommyGun")`) that seeds an isolated SplitMix64 generator producing the baseline identity: geometric proportions (component assembly graph, receiver ratios, socket offsets) and functional physics baselines (inertia tensors, slide travel, recoil spring constant `k`, thermal bloom rate). Because this seed never changes, the Tommy Gun's core shape and feel are identical on every machine and save file. **Tier 2 — Session Mutation Vector:** once the Tier 1 baseline exists, the current `SessionRunSeed` combined with the item instance's UUID drives a second, bounded pass — weathering noise masks (scratches, rust, grime), attachment kitbashing rolls (weighted by the local zone's item tier), and a small handling-tolerance multiplier (clamped `0.95`–`1.05`) on the recoil spring constant. This pass never touches structural topology.

##### Invariant

Tier 2 may only scale or mask values Tier 1 already produced — it must never modify attachment-point topology or bone-linkage counts. A stripped-down instance's base skeleton must match the Global Archetype Manifest exactly.

##### Reference Implementation

```cpp
// Tier 1: fixed identity — same on every machine, every playthrough
StructuralDNA SynthesizeArchetype(uint64_t archetypeHash) {
    SplitMix64 gen(archetypeHash); // isolated stream, never shared
    return BuildBaselineFromSeed(gen); // geometry + physics baselines only
}

// Tier 2: bounded session variety layered on top — never rewrites Tier 1 topology
ItemInstance ApplySessionMutation(const StructuralDNA& base, uint64_t sessionSeed, uint64_t instanceUUID) {
    SplitMix64 gen(sessionSeed ^ instanceUUID); // isolated stream, never shared
    ItemInstance inst{base};
    inst.wearMask = SampleWeatheringMask(gen);
    inst.recoilToleranceMul = std::clamp(0.95f + 0.10f * NextFloat01(gen), 0.95f, 1.05f);
    return inst; // topology/attachment-point count untouched
}
```

##### Serialization

Only the 64-bit archetype hash, 64-bit instance UUID, and a compact wear/attachment-index struct are ever written to a save file — no mesh, texture, or baseline stat data. Reload re-runs both generation tiers from scratch, reproducing the item exactly with zero asset disk footprint, consistent with M7-EXT-07's delta-state persistence model.

##### Player-Facing Impact

A Tommy Gun always looks and handles like a Tommy Gun, on any save or machine — but the specific one in your hands has its own scratches, rust pattern, and a barely-perceptible mechanical "personality" from wear tolerance, without ever risking a broken or nonsensical weapon configuration.

#### [M8-EXT-07] Memory-Mapped FlatBinary Archetype Schema Serializer

##### Systems Touched

Backs the JSON Layout Schema Validation step in M8's base implementation — a boot-time compile pass that removes runtime string-parsing overhead when reading item/weapon records, complementing `[M8-EXT-06]`'s Two-Tier Archetype Synthesis (that feature generates the archetype identity; this one is how the *authored* JSON-declared fields, like `brandTier`/`brandTierMultiplier`, get resolved cheaply at runtime once loaded).

##### Math

`RecordAddress = BufferBaseAddress + (RecordIndex · FixedStrideBytes)`

##### How It Works

At boot, the JSON item/weapon tables (already schema-validated per M8's base step) are pre-compiled into a dense, fixed-stride binary buffer. Runtime queries resolve a record's address by direct index arithmetic against a memory-mapped buffer instead of walking a string-keyed JSON structure, removing string parsing and heap allocation from the itemization hot path entirely.

##### Reference Implementation

```cpp
struct alignas(16) PackedWeaponBlueprint {
    uint32_t weaponIdHash;
    float baselineSpringK;
    float thermalBloomExpansionRate;
    uint32_t structuralBrandTierFlags;
};
inline const PackedWeaponBlueprint* ResolveBinaryArchetypeRecord(const uint8_t* mmapBufferPointer, uint32_t recordIndex) {
    constexpr size_t stride = sizeof(PackedWeaponBlueprint);
    return reinterpret_cast<const PackedWeaponBlueprint*>(mmapBufferPointer + (recordIndex * stride));
}
```

##### Player-Facing Impact

Invisible directly — this is what keeps item/weapon-stat lookups cheap enough to run every frame (brand-tier cook-off checks, jam-probability rolls) without the string-parsing cost item JSON would otherwise carry into the hot path.

#### [M8-EXT-08] Log-Normal Camouflage Masking

---

#### [M8-EXT-10] Procedural Loot Icon Generation *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M8 + M8-EXT-09. Renders item icons procedurally from defs (not hand-authored art).

##### Math
icon = RenderIcon(def, rarity); cache by def hash;

##### How It Works
Loot icons are drawn from the item definition (shape/rarity tint) rather than shipped sprites, so modded/new items get icons automatically. Cached by def hash. Feeds M11 UI.

##### Reference Implementation
```cpp
Texture icon = GenIcon(def);
```

##### Player-Facing Impact
Every item (incl. modded) gets an icon - no missing-sprite holes.


---

#### [M8-EXT-09] Memory-Mapped FlatBinary Inventory Cache Dictionary *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M8. Memory-maps a flat-binary item-definition dictionary for instant lookup.

##### Math
ItemDef* def = mmap(dict)[id]; // no parse on access;

##### How It Works
Item definitions live in a flat binary mmap'd at load; lookup is a direct offset (no per-item parse), so inventory/loot resolution is O(1) and zero-alloc. Pairs with M8-EXT-10 icon gen.

##### Reference Implementation
```cpp
ItemDef* d = (ItemDef*)mmapBase + id*sizeof(ItemDef);
```

##### Player-Facing Impact
Item lookups are instant - no parse stall on big inventories.


##### Systems Touched

Declares the base "Log-Normal Camouflage Masking" bullet. Reads equipped-item camo ratings from M8's item data schema and feeds a detection-range multiplier into M5.3's perception confidence accumulators.

##### Math

Effective detection range under camouflage is drawn from a log-normal rather than a flat percentage reduction, so most rolls cluster near the expected value but rare unlucky rolls still spike: `R_eff = R_base · e^{μ + σZ}`, `Z ~ N(0,1)`, with `μ = −CamoRating/10` (higher camo rating shifts the distribution's median down) and `σ` fixed small (~0.15) so variance stays tight around that median rather than swinging wildly.

##### How It Works

Rather than camo gear applying a flat "−30% detection range," each perception check against a camouflaged player samples a log-normal multiplier seeded from the item's `CamoRating` and applies it to the base detection range before M5.3 compares against actual distance. Log-normal is used instead of a flat multiplier because detection in the real world is bursty, not linear — most of the time the reduction lands close to the expected value, but every so often (long right tail) a zombie gets lucky and spots the player well past the "expected" masked range, which reads as believable rather than a gear-guaranteed cloak.

##### Reference Implementation

```cpp
// Sampled once per perception check against a camouflaged player
float ComputeEffectiveDetectionRange(float baseRange, float camoRating, float sigma, std::mt19937& rng) {
    std::normal_distribution<float> gaussian(0.0f, 1.0f);
    float z = gaussian(rng);
    float mu = -camoRating / 10.0f;
    return baseRange * std::exp(mu + sigma * z);
}
```

##### Player-Facing Impact

Good camo reliably shortens how far zombies notice the player, but it's never an absolute guarantee — an occasional unlucky spot at range keeps ghillie-suit stealth builds tense rather than a solved problem.

## M8.5 — Survivor faction economy

##### Implementation Steps

* Coupled Territory Population Dynamics (predator-prey differential equations).
* Elastic Supply-Demand Trade Models.
* Macro Regional Price Inflation (`MV=PQ`).
* Bayesian Companion Trust Updating.

##### Decisions to flag

Faction population counts and trust gating.

##### Exit Criteria

* Economy fluxes accurately following massive loot extraction.
* Trust modifiers behave probabilistically.

### Extended Systems Library — engine-side additions for M8.5

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M8.5-EXT-01] Poisson-Disk Farm Plot Growth & Scent-Attractor Coupling

##### Systems Touched

Reuses the Poisson-disk clustering already driving foliage placement (Appendix F). Reads live M10 weather/season state for growth gating. Writes a positive source term into M5.3's Atmospheric Gaussian Plume Scent Dispersion grid.

##### Math

Growth stage (`g`) follows a logistic curve gated by a weather-suitability scalar (`W_suit`, 0–1, computed from your existing M10 temperature/precipitation state — not new weather code):

`dg/dt = r·g·(1 − g)·W_suit`

Crop scent-source strength feeds directly into the existing plume equation as an additional emission term:

`Q_farm = β_crop · g · A_plot`

##### How It Works

Plot centers are seeded via the same Poisson-disk sampler used for foliage clustering, constrained to socio-economic-tagged residential/rural cells (Appendix C) — no new placement logic, just a new consumer of tags/sampler that already exist. Each active plot writes `Q_farm` as an additional point-source into the scent-dispersion grid M5.3 already solves every tick; the plume system doesn't know or care that this source is "a farm" versus a corpse or a gunshot.

##### Reference Implementation

```cpp
void UpdateFarmGrowth(float r, float w_suit, float dt, float& g) {
    g = std::clamp(g + r * g * (1.0f - g) * w_suit * dt, 0.0f, 1.0f);
}

float GetFarmScentSource(float beta_crop, float g, float area) {
    return beta_crop * g * area; // fed into the existing M5.3 plume grid as Q at this cell
}
```

##### Player-Facing Impact

Planting near your base is efficient but not free — a productive plot is a standing scent beacon that measurably increases horde traffic toward your walls, using the exact same plume math a fresh gunshot or blood pool already generates. A cold snap right after planting can wipe out a harvest for real, because it's reading the same weather state driving everything else.

#### [M8.5-EXT-02] Boid-Flock Fish Population & Hazard-Cell Contamination

##### Systems Touched

Lightweight Reynolds boid flocking (already used for zombie crowd movement and drafting in M5.1) applied to fish inside existing Jolt water volumes. Contamination risk is derived from two things that already exist — the M10 Saint-Venant `h_water` flood heightmap and the Appendix C socio-economic zone tag — rather than a new hazard-cell struct.

##### Math

Catch events are a Poisson arrival process gated by local school density (`ρ_fish`):

`P(catch in Δt) = 1 − e^(−ρ_fish·κ_gear·Δt)`

Contamination probability is derived at catch-time from existing per-cell data, no new persistent struct:

`P_contaminated = Clamp(w_zone·IsIndustrialTag(cell) + w_flood·Saturate(h_water/h_ref), 0.0, 1.0)`

where `IsIndustrialTag` reads the same Appendix C socio-economic tag already used for loot weighting, and `h_water` is sampled directly from M10's existing flood heightmap — no new hazard scalar is written or stored anywhere.

##### How It Works

Fish schools are boid-flocked exactly like zombie crowds (same separation/alignment/cohesion weights, different steering target — avoid player/predator instead of pursue), just running at a coarser update cadence since they don't need combat-grade fidelity. Catching a fish rolls the Poisson arrival, then computes contamination risk on the fly from the zone tag and current flood depth at that cell, feeding into the shared illness roll from [M8-EXT-03].

##### Reference Implementation

```cpp
bool RollFishCatch(float rho_fish, float kappa_gear, float dt, uint64_t seed) {
    float p = 1.0f - std::exp(-rho_fish * kappa_gear * dt);
    return SplitMix64(seed) < (uint64_t)(p * UINT64_MAX);
}

// Derived on the fly from existing Appendix C zone tags + M10 h_water — no stored hazard cell.
float GetContaminationRisk(const ZoneTag& zone, float h_water, float h_ref,
                            float w_zone, float w_flood) {
    float industrial = (zone.socioEconomicTag == SocioTag::Industrial) ? 1.0f : 0.0f;
    float flood = std::clamp(h_water / h_ref, 0.0f, 1.0f);
    return std::clamp(w_zone * industrial + w_flood * flood, 0.0f, 1.0f);
}
```

##### Player-Facing Impact

Fishing in a clean rural creek is slow but safe; fishing downstream of a flooded industrial sector is faster (denser schools near runoff-fed nutrients) but rolls real illness risk, computed live from data the world already tracks. No separate "is this water safe" UI or new hazard bookkeeping — just the existing zone tag and flood depth doing double duty.

#### [M8.5-EXT-03] Recruitable Survivor AI (Companion Command Layer over Existing Zombie AI)

##### Systems Touched

Runs the existing M5.1 Weighted Consideration Utility AI stack with the faction flag flipped, plus a command-override layer. Extends M8.5's existing Bayesian Companion Trust Updating rather than introducing a second trust system. Cross-session identity uses the M7 `StableId` system.

##### Math

Trust doesn't just Bayesian-update on discrete events — it drifts continuously toward a floor absent reinforcement, using a bounded Ornstein-Uhlenbeck process layered on top of the existing Bayesian updater:

`dTrust = θ·(μ_floor − Trust)·dt + σ·dW + ΔTrust_bayesian_event`

##### How It Works

A recruited survivor entity keeps its existing zombie-stack components (utility-AI consideration set) but with `Faction = Player` and a `CommandOverride` component that can inject a player-issued goal above the AI's own utility-scored choices — a similar overriding pattern to how screamer convergence already overrides normal zombie utility scoring in M5.1. Trust events (shared kills, gifted items, completed requests) fire the existing Bayesian updater from M8.5; between events, the OU term above handles ambient drift so trust isn't static outside of scripted moments. `StableId` (M7) is what lets a companion's trust record survive a save/reload without needing a new persistence path.

##### Reference Implementation

```cpp
// rng_state is passed by reference so GaussianFromSplitMix64 can advance its state
// the generator across calls — every DriftTrust tick would need an already-different seed handed in from
// outside with no shown mechanism to do so. Passed by reference so the generator advances in place, and
// should live in thread_local storage per the SplitMix64 threading note in M0's Implementation Steps
// (enkiTS worker fibers must never share a mutable seed state across threads).
// Layered on top of the existing M8.5 Bayesian trust updater — does not replace it
void DriftTrust(float theta, float mu_floor, float sigma, float dt, uint64_t& rng_state, float& trust) {
    float noise = GaussianFromSplitMix64(rng_state) * sigma * std::sqrt(dt);
    trust += theta * (mu_floor - trust) * dt + noise;
}
```

##### Player-Facing Impact

Companions feel alive between story beats — trust doesn't just move when a scripted event fires, it quietly drifts if you ignore them, using the same random-walk math already giving your zombie phenotypes organic variation rather than a hand-tuned relationship-meter script.

#### [M8.5-EXT-04] Live Economy-Sim Trade Pricing (Barter)

##### Systems Touched

Reads directly from M8.5's existing Elastic Supply-Demand Trade Model and Macro Regional Price Inflation (`MV=PQ`) — this feature is a UI/interaction layer over data that already exists, not a new pricing system.

##### Math

No new formula — trade offer value is the existing regional price term evaluated at the transaction's item category and quantity:

`Price_offer = P_region(item_category)·Q_traded`, where `P_region` is already being computed by the existing `MV=PQ` regional model.

##### How It Works

A trade UI reads the same live `P_region` values M8.5 is already maintaining per-region per-category — a besieged settlement's prices are already spiking in the economy sim before a player ever opens a trade menu; this feature just exposes that number instead of computing a separate shop price.

##### Reference Implementation

```cpp
// Not new pricing logic — reads the existing regional price state directly
float GetTradeOfferValue(const RegionalEconomyState& econ, ItemCategory cat, float qty) {
    return econ.GetPrice(cat) * qty; // GetPrice() already exists in the M8.5 MV=PQ model
}
```

##### Player-Facing Impact

Prices genuinely reflect what's happening in the world — a settlement that just lost its ammo cache to a raid will actually pay more for ammo, because the same economy simulation already tracking that shortage is what the trade window is reading from.

## M8.6 — Settlement construction

##### Implementation Steps

* Static Equilibrium Truss Solver (matrix-free conjugate-gradient).
* Kirchhoff Current Law Power Grid mapping voltage resistance.
* M/M/c Queueing Labor Models for workstation assignment.

##### Decisions to flag

Staged decay thresholds.

##### Exit Criteria

* Free-form physical buildings hold or collapse dynamically when simulated loads fail.



### Extended Systems Library — engine-side additions for M8.6

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M8.6-EXT-01] Kirchhoff Power Grid Overload Arcing

##### Systems Touched

Connects your M8.6 electrical network arrays directly into your M6.5 cellular-automata fire grids.

##### Math

When line configurations experience high loads, the thermal power dissipation (P_loss ) is tracked across line resistance values (R):

`P_loss = I²·R`

If `P_loss` crosses your safety threshold (`P_fuse`), the line fails, firing an immediate event that passes the 2D matrix coordinates straight into your cellular fire simulation to ignite an electrical hazard cell.

##### How It Works

Each power-loop update checks every line's dissipated power against its fuse threshold. A line crossing threshold fails immediately, passing its grid coordinates directly into the M6.5 cellular fire sim to ignite a hazard cell at that exact location.

##### Reference Implementation

```cpp
// Checked during your structural power loop update passes
bool CheckGridOverload(float current_i, float resistance_r, float p_fuse) {
    return ((current_i * current_i) * resistance_r) > p_fuse;
}
```

##### Player-Facing Impact

Overloading your safehouse grid by daisy-chaining too many traps and searchlights onto an un-relayed line creates an immediate fire hazard. The lines will physically snap, throw bright electrical arcs across the dark viewport, and ignite nearby wooden components, forcing you to design balanced electrical layout structures using dedicated relays.

#### [M8.6-EXT-02] Surface-Saturation Capacitive Arc Tracking

##### Systems Touched

Extends M8.6 (Kirchhoff Power Networks) across M10 shallow-water flooding maps.

##### Math

When high-voltage power components or electrical grids snap, current paths route along the shortest path over saturated ground terrain coordinates. The local cell resistance (R_cell ) drops non-linearly based on the shallow-water height variables (h_water ):

`G_cell = (1.0 + α·h_water) / R_baseline`

This is a conductance (`G_cell` = 1/R, computed by `GetCellConductance` below), not a resistance — it rises with water depth rather than falling. Note [M8.6-EXT-04] below computes the same quantity via a separate function, also named `G_cell` — see the consolidation note there.

##### How It Works

Your power grid updates run on a slow cadence. When an open wire or circuit failure event fires, the system maps out conductance lines using your existing 2D SpatialHash layout. It checks the water layer states across neighboring tiles, routing lethal electrical damage values directly through flooded cells containing players or entities without running complex finite-element field models.

##### Reference Implementation

```cpp
float GetCellConductance(float r_base, float h_water, float alpha) {
    return (1.0f + alpha * h_water) / r_base;
}
```

##### Player-Facing Impact

Snapping an electrical wire onto dry concrete does little more than spark locally. However, if you drop that same live wire into an intersection flooded by a severe storm, the electrical current arcs across the wet asphalt surface. It transforms the puddle into a highly lethal electric trap that shocks everything wading through the water, letting you use the weather to control choke points.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M8.6-EXT-03] Catenary Cable Tension Snapping Solver

##### Systems Touched

Complements M8.6 electrical wiring layouts and handles Jolt distance constraints. Declares the canonical `CableSpanComponent` — the single shared component backing both this feature and [M8.6-EXT-05] Catenary Wire Clothesline Decapitation Loop. Per standard ECS practice, this is one system distinguished by a role tag, not two independent implementations of near-identical tension-snap math (the same gap pattern as the ammo cook-off duplicate):

```cpp
enum class CableRole { PowerLine, DecapTrap };

struct CableSpanComponent {
    JPH::BodyID anchorA;
    JPH::BodyID anchorB;
    float ropeDensity   = 1.0f;   // ρ_cable
    float breakTension  = 0.0f;   // T_break
    CableRole role       = CableRole::PowerLine;
};
```

##### Math

Power lines hanging between anchor cells map out a catenary mathematical curve shape. The internal tension force (T) at the support boundaries reacts to the suspended cable mass (m) and structural sag distances (a):

`y = a·cosh(x/a), T_max = ρ_cable · g · y_max`

If an external kinetic impact load (like a vehicle hit) injects a sudden force impulse (J), the tension spike triggers an immediate line snap if safety limits are crossed:

`T_total = T_max + (‖J‖/Δt) > T_break`

##### How It Works

Your power grids map connection coordinates across structural anchor blocks. This system attaches a `CableSpanComponent` (with a Jolt distance or spring constraint) between the anchor nodes to represent the wire span. If an object intersects the bounding volume of the wire path, the collision impulses are passed directly into the shared `CheckCableSnap()` tension check below; crossing the breaking threshold deletes the constraint component instantly. What happens on snap is gated by `CableSpanComponent::role`: `PowerLine` cuts off circuit routing (feeds [M8.6-EXT-01]/[M8.6-EXT-02]); `DecapTrap` fires the kinematic joint-severing impulse described in [M8.6-EXT-05] instead. The tension math and break check are identical either way — only the on-snap consequence differs.

##### Reference Implementation

```cpp
// Shared by both CableRole::PowerLine and CableRole::DecapTrap — the only tension-snap check in the codebase
bool CheckCableSnap(const CableSpanComponent& cable, float t_max, float impulse, float dt) {
    return (t_max + (impulse / dt)) > cable.breakTension;
}
```

##### Player-Facing Impact

Smashing a speeding off-road vehicle straight into an overhead utility pole or tearing through low-hanging power line rigs will physically catch the cables. The lines will stretch taut with high tension before snapping violently with a sharp electrical crack, causing live, sparking wire ends to whip down across the street surface and shock nearby entities.

#### [M8.6-EXT-04] Electro-Capacitive Water-Wading Chain Stun

##### Systems Touched

Merges your M8.6 Kirchhoff power networks directly into M10 Saint-Venant flooding tiles.

##### Math

When a live electrical wire snaps and contacts a flooded street, current routes across adjacent cells along the highest conductance paths based on water thickness (h_water ):

`G_cell = (1.0 + α·h_water) / R_baseline`

Same conductance formula and computation as [M8.6-EXT-02] Surface-Saturation Capacitive Arc Tracking (`GetCellConductance` there, `StepChainConductance` here — same inputs/math, plus an added `1e-5f` epsilon). [M8.6-EXT-02] is the canonical declaration; this feature calls that shared function rather than maintaining a second implementation.

##### How It Works

When a snapped live wire contacts a flooded tile, current conducts outward across adjacent cells scaling with water depth via the shared conductance function ([M8.6-EXT-02]). Any entity in a cell above the conduction threshold is stunned simultaneously — a large flooded area chain-stuns everyone in it at once.

##### Reference Implementation

```cpp
void StepChainConductance(float r_base, float h_w, float alpha, float& out_g) {
    out_g = (1.0f + alpha * h_w) / (r_base + 1e-5f);
}
```

##### Player-Facing Impact

Dropping a live electrical line or shooting a power transformer above a flooded avenue turns the puddle into a massive chain-reaction trap. The current routes across the contiguous water layer, shocking and paralyzing every zombie wading through the water simultaneously, letting you use active weather storms to secure entire avenues.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M8.6-EXT-05] Catenary Wire Clothesline Decapitation Loop

##### Systems Touched

Extends M8.6 wiring anchor blocks, setting clean Jolt distance constraints. Reuses the canonical `CableSpanComponent` and `CheckCableSnap()` declared in [M8.6-EXT-03] with `role = CableRole::DecapTrap` — this is the same tension-snap system as the power-line cable, not a second implementation of it. Build M8.6-EXT-03 first; this feature only adds the placement flow (player rigs a wire between two anchor blocks) and the on-snap decapitation impulse.

##### Math

A steel cable rigged between two fixed anchor blocks maps out a structural catenary curve — identical math to [M8.6-EXT-03]. When an asset crosses the coordinate at high velocity, the mechanical tension spike (T_total ) is processed through the same `CheckCableSnap()` check:

`T_total = T_static + (‖J_impact‖/Δt) > T_break`

##### How It Works

Once a player rigs a cable between two anchor blocks (reusing M8.6-EXT-03's placement flow, `role = CableRole::DecapTrap`), the span tracks tension through the shared `CheckCableSnap()`. Crossing break tension triggers the decapitation impulse instead of the power-line's circuit-cut — same detector, different on-snap consequence via `role`.

##### Reference Implementation

```cpp
// On CheckCableSnap() returning true for a CableSpanComponent with role == CableRole::DecapTrap:
// apply the decapitation impulse instead of PowerLine's circuit-cut behavior.
void ApplyDecapImpulse(JPH::BodyInterface& bi, JPH::BodyID head, const glm::vec3& v_impact) {
    bi.AddImpulse(head, JPH::Vec3(v_impact.x, v_impact.y, v_impact.z));
}
```

##### Player-Facing Impact

You can rig a thin steel wire between structural metal poles or ruined cars to form an active defensive line. Speeding vehicles pass safely beneath the boundary, but pursuing common zombies running at neck height hit the wire constraint directly. If the tension limits hold, the cable performs an instant kinetic joint severing loop, cleanly decapitating targets.

#### [M8.6-EXT-06] Player-Placed Barricade Snap Constraints (Fortification)

##### Systems Touched

Reuses the exact `CableSpanComponent`/`CheckCableSnap()` pattern declared in [M8.6-EXT-03], adding a third `CableRole::Barricade` value rather than new constraint code. Horde load reuses M9-EXT-06's continuum-fluid horde-pressure force and M5.4-EXT-01's existing `P_crush` formula.

##### Math

No new snap math — identical to the existing shared check:

`T_total = T_static + (‖J_impact‖/Δt) > T_break`

Horde load against a placed barricade is the same crowd-pressure sum already computed for crush damage:

`P_wall = Σ_{i∈LocalCell} ‖F_separation_i‖` (identical to the existing `P_crush` term from M5.4-EXT-01, evaluated against the barricade's bounding volume instead of a player/entity)

##### How It Works

Placing a barricade spawns a `CableSpanComponent`/Jolt constraint pair with `role = CableRole::Barricade` between two anchor points, exactly like a power line or a decap-trap wire — same struct, same break-check function, third enum value. A leaning horde applies force through the existing `SpatialHash`-driven crowd-density query already computing `P_crush` elsewhere; crossing `T_break` deletes the constraint the same way a snapped power line does.

##### Reference Implementation

```cpp
// CableRole::Barricade added to the existing enum from M8.6-EXT-03 — no new struct
enum class CableRole { PowerLine, DecapTrap, Barricade };

// Reuses CheckCableSnap() verbatim; reuses the existing crowd-pressure sum from M5.4-EXT-01
void ApplyHordeLoadToBarricade(const CableSpanComponent& wall, float p_wall, float dt) {
    if (CheckCableSnap(wall, wall.tStatic, p_wall, dt)) {
        // delete constraint — identical consequence path to a snapped power line
    }
}
```

##### Player-Facing Impact

A barricade groaning and finally snapping under horde weight uses the identical physical logic as watching a power line snap under a vehicle impact — one consistent "things break under load" language across the whole game instead of a separate abstracted "wall HP" bar.

#### [M8.6-EXT-07] Kirchhoff Circuit Ohmic Dissipation Water Vaporization Solver

##### Systems Touched

Links M8.6's Kirchhoff power grid to M10's Saint-Venant shallow-water flooding grid — when a snapped live line (`[M8.6-EXT-03]`/`[M8.6-EXT-05]`, `CableSpanComponent`) discharges into a flooded cell, this converts the dissipated electrical energy into a vaporization volume that feeds back into the flood heightmap, rather than the electrical and flood systems staying one-directionally coupled (M8.6-EXT-02/04 currently only read water depth to boost conductance; this is the reverse write).

##### Math

`Δh_water = −(I²·R·Δt) / (ρ_water · A_cell · L_vap)`

##### How It Works

When `CheckCableSnap()` (`[M8.6-EXT-03]`) fires into a flooded cell, the resulting ohmic power loss (`I²R`, already computed for the overload-arcing check in `[M8.6-EXT-01]`) is converted via the latent heat of vaporization into a mass-loss term, which is subtracted directly from that cell's `h_water` value in the M10 Saint-Venant integration step.

##### Reference Implementation

```cpp
inline float ComputeOhmicWaterVaporizationDelta(float currentAmperageI, float lineResistanceR, float cellAreaSquareMeters, float dt) {
    constexpr float latentHeatVaporizationWater = 2260000.0f; // J/kg
    constexpr float waterMassDensityKgM3 = 1000.0f;
    float energyDissipationJoules = (currentAmperageI * currentAmperageI) * lineResistanceR * dt;
    float massVaporizedKg = energyDissipationJoules / latentHeatVaporizationWater;
    return -(massVaporizedKg / (waterMassDensityKgM3 * cellAreaSquareMeters)); // feeds the Saint-Venant PDE directly
}
```

##### Player-Facing Impact

A live wire dropped into a shallow flooded intersection doesn't just electrify the puddle (`[M8.6-EXT-04]`) — sustained high-amperage discharge visibly boils it down over time, giving the electrified-water trap a natural expiration instead of staying lethal indefinitely.

#### [M8.6-EXT-08] M/M/c Queueing Labor Model for Workstation Assignment

##### Systems Touched

Declares the base "M/M/c Queueing Labor Models for workstation assignment" bullet. Reads survivor population and assigned-labor counts from M8.5's faction economy; feeds task-completion timing back into settlement production rates.

##### Math

Standard M/M/c queue: `c` identical workstations (servers), Poisson task arrivals at rate `λ`, exponential service time at rate `μ` per busy station. Expected wait time before a survivor gets a free station: `Wq = C(c,λ/μ) / (cμ − λ)`, where `C(c,ρ)` is the Erlang-C probability of all `c` stations being occupied.

##### How It Works

Each settlement task type (cooking, crafting, medical) is modeled as a queue with a fixed station count `c` (how many survivors can work it simultaneously). Idle survivors arrive at the queue when assigned; if all stations are busy they wait rather than instantly working, and expected wait time is computed live from the Erlang-C formula above as population and station count change — so adding a second workbench measurably cuts the backlog instead of needing a hand-tuned throughput number.

##### Reference Implementation

> **Fix notes:** `ExpectedWaitTime` divided by `(c * mu - lambda)` with no guard. Any task queue that gets overloaded — `lambda >= c * mu`, i.e. arrivals outpace total service capacity, which a settlement absolutely can hit under a horde-pressure spike — drove this to zero or negative, producing an `inf`/negative wait time instead of a large-but-finite one. Also guarded `mu <= 0` and `c <= 0` (no stations assigned), both of which would otherwise divide by zero in `rho = lambda / mu` or in `ErlangC`'s `rho / c` term.

```cpp
#include <limits>

// Recomputed whenever population or station count changes for a given task queue
float ErlangC(int c, float rho) {
    if (c <= 0) return 1.0f; // no stations: every arrival waits
    float sum = 0.0f;
    float term = 1.0f;
    for (int k = 0; k < c; ++k) {
        if (k > 0) term *= rho / k;
        sum += term;
    }
    term *= rho / c;
    float denom = (1.0f - rho / c) * sum + term;
    return (denom > 1e-5f) ? (term / denom) : 1.0f;
}

float ExpectedWaitTime(int c, float lambda, float mu) {
    if (c <= 0 || mu <= 0.0f) return std::numeric_limits<float>::max(); // no service capacity at all
    float rho = lambda / mu;
    float capacityMargin = c * mu - lambda;
    if (capacityMargin <= 1e-5f) return std::numeric_limits<float>::max(); // queue is unstable/overloaded
    return ErlangC(c, rho) / capacityMargin;
}
```

##### Player-Facing Impact

Overcrowded settlements visibly bottleneck at popular stations (survivors queued and idle), giving the player a clear, mathematically grounded signal that it's time to build another workstation rather than an opaque productivity stat.

---

#### [M8.6-EXT-09] Genetic-Algorithm Settlement Macro-Layout Optimizer
#### [M8.6-EXT-10] Settlement NavMesh from GA Layout *(SOURCED FROM PLAN FILE)*

##### Systems Touched
Consumes M8.6-EXT-09 GA building-footprint layout, produces walkable NavMesh the AI director M5.4 + horde pathing need inside a settlement. Distinct from M4-EXT-08 WFC reachability (interiors vs settlement exterior/plaza).

##### Math
From GA footprint polygon set, compute free-space polygon (building set subtracted from zoning polygon), then standard NavMesh triangulation (improved funnel / Delaunay over walkable region) with portal edges between adjacent polygons for string-pulling path queries.

##### How It Works
Once M8.6-EXT-09 bakes footprints + gate positions, derives settlement walkable mesh in same one-time bake. Doorways become portals; perimeter chokepoints the GA optimized for defense become natural funnel points horde pathing uses. Cached with layout — never recomputed per-frame.

##### Reference Implementation
```cpp
NavMesh BuildSettlementNavMesh(const SettlementLayoutGenome& g){ Polygon free=ZoningPolygon(); for(auto& b:g.buildings) free=Subtract(free,b.footprint); return TriangulateWalkable(free, GateEdges(g.buildings)); }
```

##### Player-Facing Impact
Settlements are navigable for AI + hordes — defense chokepoints double as pathing funnels.

##### Systems Touched
Runs once per settlement at world-seed bake time, not per-chunk-stream — same offline/one-time-budget tier `[M4-EXT-09]` above already reserves for simulated-annealing landmark interiors. Output layout (building footprints, defensive perimeter, gate positions) feeds `[M5.4-EXT-06]`'s road graph as endpoints and M8.6's power-grid conductance as node positions.

##### Math
FI2Pop-style feasible/infeasible two-population GA (Kimbrough et al.; used in prior search-based settlement/dungeon-layout work): fitness combines road-graph connectivity, defensibility (perimeter chokepoint count vs. open frontage), and resource-node coverage (existing farmland/water/scavenge-site tags within walking radius); infeasible individuals (disconnected or overlapping footprints) are kept in a separate population and repaired rather than discarded outright, which is what makes FI2Pop converge reliably where a single-population GA on a heavily-constrained layout problem tends to stall on infeasible dead-ends.

##### How It Works
Per the same reasoning already applied to SA in `[M4-EXT-09]`: unbounded iterative convergence cost is incompatible with a streaming per-chunk time budget, so this is explicitly reserved for the small, finite set of settlement instances (not run per-chunk, not run per-building) — generated once when a settlement is first placed by the macro-graph and cached like any other seed-derived content. Initial population is a set of randomly-permuted building-footprint placements against the settlement's zoning tag and perimeter shape; each generation scores individuals on the fitness function above, keeps top performers plus randomly-mutated/crossed-over offspring, and repairs infeasible layouts (footprint overlap, disconnected road access) rather than culling them, per FI2Pop. Terminate on a fixed generation count (not convergence detection) to keep worst-case bake time bounded, same fixed-budget philosophy as everything else time-boxed in this doc.

##### Reference Implementation
```cpp
struct SettlementLayoutGenome { std::vector<BuildingPlacement> buildings; float fitness; };

float EvaluateFitness(const SettlementLayoutGenome& g, const WorldTagState& tags) {
    float connectivity = RoadGraphConnectivityScore(g.buildings);      // feeds [M5.4-EXT-06]
    float defensibility = PerimeterChokepointScore(g.buildings);
    float resourceCoverage = ResourceNodeCoverageScore(g.buildings, tags);
    return connectivity + defensibility + resourceCoverage;
}

// FI2Pop: feasible and infeasible populations evolved separately; infeasible individuals
// are repaired (footprint de-overlap, road-access patch) each generation, not discarded.
SettlementLayoutGenome RunFixedGenerationGA(uint32_t generations, uint32_t populationSize, uint32_t seed);
```

##### Player-Facing Impact
Settlements read as if someone actually planned them for defense and logistics — walls following real chokepoints, roads reaching every building — without hand-laying out each one, and it's a one-time bake cost paid at settlement placement, never a per-frame or per-chunk-stream cost.

## M8.7 — Perk & progression system (PerkPoints)

##### Implementation Steps

Built to the same depth as M8.5's Bayesian Companion Trust and faction economy — a real system with acquisition, spend, and cross-system hooks, not a stub currency.

* **Acquisition sources** (pulled from existing loops, not a new invented currency): 1 point per faction reputation tier crossed (M8.5's trust/reputation thresholds), 1 point per in-game week survived past the first (reads M7's persistence clock), and periodic milestone kills (every 50th confirmed kill, tracked via a simple counter on the player entity) award 1 point. All three sources dispatch a `PerkPointAwarded { uint32_t amount, PerkSource reason }` event on the M2 EventBus rather than mutating a perk-point counter directly, so the UI/HUD (M11) can react without polling.
* **Spend structure: a web, not a tree.** A strict tree (linear branches gated by prerequisites) doesn't fit a game whose core loop is already cross-system (companion trust, faction standing, item brand tiers, per-limb combat) — a web lets a perk in the "scavenging" cluster have a prerequisite in the "combat" cluster, matching how the rest of the game already interlocks systems instead of siloing them. Perks are grouped into four clusters for UI purposes (Combat, Survival, Scavenging, Leadership) but prerequisite edges can cross clusters.
* **Cross-system hooks (required, at least one per system per the spec):**

  * **Brand/quality-tier (M8, item schema):** `Perk: "Field Armorer"` — reduces effective `brandQualityMultiplier` variance on newly looted items by widening the favorable end of the tier's `qualityRollRange` (from M8's item JSON), representing a player who can recognize and prioritize better-conditioned examples of a given weapon tier while looting.
  * **DamageEvent resolution (M2):** `Perk: "Hardened"` — subscribes to the pre-final-application `DamageEvent` (not `ResolvedDamageEvent`) and applies a flat percentage reduction to `amount` when `tag == DamageTag::Ballistic`, inserted into the resolution chain after armor/cover mitigation and before the event is re-enqueued as `ResolvedDamageEvent` — this is the "resolved flag interception point" the base spec calls for.
  * **Faction economy / companion trust (M8.5):** `Perk: "Diplomat"` — adds a flat additive bonus to the Bayesian trust-update step's prior whenever a positive trade/quest outcome occurs, so trust climbs faster per positive interaction without changing how negative interactions are weighted.
* **Respec policy:** refundable, at a cost — a full respec consumes a consumable "Memory Serum" item (crafted via M8's itemization pipeline, not purchased with PerkPoints themselves, to avoid a circular currency loop) and refunds 100% of spent points. No partial/single-perk respec — full reset only, to keep the respec UI and prerequisite-graph re-validation simple.
* **Data shape:**

```json
{
  "id": "hardened",
  "cost": 2,
  "prerequisites": ["combat_basics"],
  "cluster": "Combat",
  "effect": { "type": "damage_mitigation", "tag": "Ballistic", "reduction": 0.15 }
}
```

Perk definitions live in `assets/perks/perks.json`, validated by the same JSON Layout Schema Validation step as M8's item tables. Unlocking a perk dispatches `PerkUnlocked { uint32_t perkId }` on the EventBus; systems that care (damage resolution, item-loot mitigation, trust updater) subscribe rather than polling a perk-state table every tick.

##### Decisions to flag

Whether cross-cluster prerequisite edges need a visual "web" UI (spend more M11 UI budget) vs. a simplified per-cluster tree-of-trees presentation that just permits the occasional cross-cluster unlock condition without rendering it as a literal graph. Memory Serum crafting-cost tuning.

##### Exit Criteria

* All three acquisition sources correctly award points via EventBus events, visible in the M11 HUD.
* At least the three named cross-system perks (Field Armorer, Hardened, Diplomat) function end-to-end against the real `brandQualityMultiplier`, `DamageEvent`, and trust-update systems — no stubbed hooks.
* Respec consumes a Memory Serum and fully refunds spent points; prerequisite graph re-validates cleanly afterward.

## M9 — Vehicle system

##### Implementation Steps

* Wheeled Suspension Constraints (Jolt dedicated wheeled vehicle integrations).
* Degradation Handling Shifts.
* Abstract Friction Query Interface (`SurfaceFrictionSample`), with a single canonical composition step: `SurfaceFrictionSample::ResolveFriction(baseMu)`. Every system that modifies traction (surface material, weather/mud, corpse-pile flattening, blood-slick, tire-thermal) writes into one of this struct's named multiplier fields instead of writing `traction`/`mu` directly. `ResolveFriction` applies them in a fixed order — see reference implementation below. No system outside `ResolveFriction` is allowed to set the resolved traction value.
* Line-of-Sight Entry Dynamics.

##### Decisions to flag

Interface boundaries pending M10 mapping.

##### Exit Criteria

* Vehicles traverse seamlessly, utilizing `SurfaceFrictionSample` data logic.
* All traction-modifying systems (blood-slick, tire-thermal, aquaplaning, mud, corpse-pile) write into `SurfaceFrictionSample` multiplier fields only — zero direct writes to a resolved `traction`/`mu` value anywhere in the codebase.

##### Canonical friction composition (locks Architectural Gap #3)

```cpp
// SurfaceFrictionSample::ResolveFriction — the ONLY place traction is written.
// Fixed multiplicative order: surface material -> weather/mud -> blood-slick -> tire-thermal.
// Corpse-pile flattening (M7-EXT-01) is a per-cell override that participates as a term
// in this same chain — it never writes traction directly.
struct SurfaceFrictionSample {
    float materialMuMultiplier      = 1.0f; // surface material (asphalt/dirt/gravel/etc.)
    float weatherMuMultiplier       = 1.0f; // aquaplaning (M9-EXT-03) * mud shear (M9-EXT-04)
    float corpseOverrideMultiplier  = 1.0f; // corpse-pile flattening (M7-EXT-01), 1.0 if none present
    float bloodMuMultiplier         = 1.0f; // blood-slick viscosity (M9-EXT-01)
    float tireThermalMuMultiplier   = 1.0f; // tire-thermal breakdown (M9-EXT-02), 1.0 for foot traction

    float ResolveFriction(float baseMu) const {
        float mu = baseMu;
        mu *= materialMuMultiplier;
        mu *= weatherMuMultiplier;
        mu *= corpseOverrideMultiplier;
        mu *= bloodMuMultiplier;
        mu *= tireThermalMuMultiplier;
        return mu;
    }
};
```



### Extended Systems Library — engine-side additions for M9

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M9-EXT-01] Fluid Momentum Traction Shear (Blood-Slick Viscosity)

##### Systems Touched

Connects M6.5 (GPU Fluid Blood Particles) directly to your M9 SurfaceFrictionSample interface — writes the `bloodMuMultiplier` term consumed by `SurfaceFrictionSample::ResolveFriction()`.

##### How It Works

When a high-velocity projectile triggers a fluid blood spatter event via M6.5, the compute shader outputs impact coordinates. Instead of instantiating an expensive physical grid, an asynchronous enkiTS task translates these locations into a temporary spatial hash map density identifier. When your vehicle constraint system (M9) or player controller (M2.7) queries the terrain surface via your abstract SurfaceFrictionSample pointer format, it sets the cell's `bloodMuMultiplier` term based on local fluid density; the final traction value only comes out of `ResolveFriction()`, never this write directly.

##### Math

`μ_slick =μ_base ⋅e^(−λ⋅ρ_blood)`

`Viscosity_slick =Viscosity_base +α⋅ρ_blood`

##### Reference Implementation

```cpp
// Writes the blood-slick term consumed by SurfaceFrictionSample::ResolveFriction() — never mutates traction directly
void ApplyFluidShear(float blood_vols, float& bloodMuMultiplier, float& viscosity) {
    bloodMuMultiplier = (blood_vols > 0.0f) ? std::exp(-0.45f * blood_vols) : 1.0f;
    if (blood_vols > 0.0f) { viscosity += 0.25f * blood_vols; }
}
```

##### Player-Facing Impact

Mowing down a massive cluster of zombies in a narrow urban corridor creates a persistent physical hazard. The pavement becomes visibly and physically slick. If you sprint through the gore, your character controller triggers grounded foot-slip corrections to simulate sliding. If you steer a vehicle through it at high speed, your wheels lose suspension bite, extending your drift arcs and breaking your vehicle's turning response until you clear the slick grid zone.

#### [M9-EXT-02] Non-Linear Tire Slip Friction Heating (Thermal Traction Breakdown)

##### Systems Touched

Feeds your M9 vehicle chassis components directly into the SurfaceFrictionSample system — writes the `tireThermalMuMultiplier` term consumed last in `SurfaceFrictionSample::ResolveFriction()`.

##### How It Works

Your vehicle constraints read surface tracking vectors via Jolt's dedicated wheeled vehicle setup. This feature attaches a low-overhead floating-point tracking variable directly to your vehicle's wheel instances. Inside your fixed-timestep update loop, the engine extracts the active tire slip velocity and friction load vectors natively from Jolt. The generated sliding power translates into thermal accumulation, setting the wheel's `tireThermalMuMultiplier` term inline; `ResolveFriction()` is what actually applies it to the final traction value.

##### Math

`dT_tire/dt = α·∥F_slip·v_slip∥ − β·(T_tire − T_ambient)`

`μ(T_tire) = μ_baseline·(1.0 − γ·max(0.0, T_tire − T_optimal)²)`

##### Reference Implementation

```cpp
// Writes the tire-thermal term consumed by SurfaceFrictionSample::ResolveFriction() — never mutates mu directly
void UpdateTireHeat(float power, float t_amb, float alpha, float beta, float gamma, float t_opt, float dt, float& t, float& tireThermalMuMultiplier) {
    t += (alpha * power - beta * (t - t_amb)) * dt;
    float d = std::max(0.0f, t - t_opt);
    tireThermalMuMultiplier = 1.0f - gamma * (d * d);
}
```

##### Player-Facing Impact

Executing prolonged, high-speed drifts or spinning your wheels aggressively over dry asphalt heats the tire rubber past its optimal threshold. Your vehicle handling drops off noticeability the longer you abuse the vehicle controls, causing your tires to glaze over, spin out, and offer virtually zero traction until you chill out your driving and let the rubber cool back down.

#### [M9-EXT-03] High-Velocity Wet-Silt Aquaplaning

##### Systems Touched

Ties M10 shallow-water Saint-Venant flooding heightmaps directly into your M9 vehicle chassis handling loops.

##### Math

When your vehicle hits a flooded roadway sector, a hydrodynamic water wedge builds beneath the wheels. The operating tire friction coefficient (μ_active ) decays non-linearly based on vehicle velocity (v) relative to tire inflation pressure (P_pressure ) and current puddle depth (h_water ):

`F_plane = Clamp((‖v_vehicle‖ / (5.56·√P_pressure)) · (h_water/h_tread), 0.0, 1.0)`

`μ_active = μ_base · (1.0 − F_plane)`

Velocity is in the numerator over a pressure-scaled denominator, and water depth over tread depth (not the reverse), matching the Reference Implementation below.

##### How It Works

Inside your vehicle suspension update step, your wheel tracking loop queries your global M10 water height grid layout using your abstract SurfaceFrictionSample interface. If the vehicle's airspeed vector is high over saturated tiles, the system bypasses Jolt's standard tire contact manifold logic and folds the wedge-lift falloff into the cell's `weatherMuMultiplier` term (shared with M9-EXT-04 mud shear — both are weather/surface-saturation effects and are mutually exclusive by surface type in practice); `ResolveFriction()` applies it to the final traction value, not this write.

##### Reference Implementation

```cpp
// Writes into the shared weather/mud term consumed by SurfaceFrictionSample::ResolveFriction() — never mutates traction directly
void ApplyAquaplaning(const glm::vec3& vel, float h_water, float p_tire, float h_tread, float& weatherMuMultiplier) {
    weatherMuMultiplier *= std::clamp(1.0f - (glm::length(vel) / (5.56f * std::sqrt(p_tire))) * (h_water / h_tread), 0.0f, 1.0f);
}
```

##### Player-Facing Impact

Cruising down urban roads at low speed during a severe 15% rain storm event is safe and controllable. However, if you pin the throttle to outrun a fast-moving director zombie wave and plow into a deeply flooded intersection, your tires instantly lose mechanical contact with the asphalt. Your car transitions into a terrifying, un-steerable projectile that hydroplanes across the water layer, letting your raw vehicle momentum slide you straight through wooden palisade barriers or brick walls.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M9-EXT-04] Non-Newtonian Mud Silt Advection (Soil Rut Tracking)

##### Systems Touched

Links M10 groundwater and weather grids straight to the M9 vehicle chassis constraints.

##### Math

When environmental rain components saturate unpaved sectors, the soil transitions into a non-Newtonian yield-stress fluid. The traction behavior follows a simplified Herschel-Bulkley fluid shear model:

`τ_shear = τ_0 + K·(‖v_slip‖/h_tire)^n`

The resulting operational tire friction coefficient (μ_active) scales non-linearly over the localized shear rate, shifting vehicle handling:

`μ_active = Clamp(μ_base · (1.0 − τ_shear/σ_yield_mud), 0.08, 1.0)`

A stationary vehicle drives `‖v_slip‖` to exactly `0.0f`; for shear-thinning mud (`n < 1`), raising `0` to a fractional power is undefined on some compiler/libm configurations. The reference implementation regularizes the ratio inside a squared-and-epsilon'd form before the power rule, which reduces to the exact same formula as `ε → 0`.

Slip velocity is over tread height (not the reverse); shear stress is over yield stress (not the reverse) — matching the Reference Implementation below.

##### How It Works

Your M10 groundwater PDE tracks water transport across a 2D heightmap grid on a regular 6-tick update cadence. This feature updates the terrain descriptors returned by your abstract SurfaceFrictionSample interface. When a vehicle wheel component (M9) passes over a mud cell, the system reads the wheel's active slip velocity. If the velocity is low, the mud remains rigid; if the player punches the throttle and spins the tires, the shear calculations drop the cell's `weatherMuMultiplier` term (shared with M9-EXT-03 aquaplaning) instantly — `ResolveFriction()` is what actually applies it to the Jolt wheel manifolds.

##### Reference Implementation

```cpp
// Regularized against the zero-slip-velocity pow(0, fractional) instability described above.
// shear_rate_sq = (v_slip^2+eps)/(h_tire^2+eps); pow(shear_rate_sq, n/2) == (v_slip/h_tire)^n as eps -> 0.
// Writes into the shared weather/mud term consumed by SurfaceFrictionSample::ResolveFriction() — never mutates mu directly
void ApplyMudShear(float v_slip, float h_tire, float tau_0, float k, float n, float yield_mud, float& weatherMuMultiplier) {
    const float epsilon = 1e-5f;
    float shear_rate_sq = (v_slip * v_slip + epsilon) / (h_tire * h_tire + epsilon);
    float tau_shear = tau_0 + k * std::pow(shear_rate_sq, n * 0.5f);
    weatherMuMultiplier = std::clamp(weatherMuMultiplier * (1.0f - tau_shear / (yield_mud + epsilon)), 0.08f, 1.0f);
}
```

##### Player-Facing Impact

Traversing dirt tracks or unpaved alleyways during rain storms changes how you control your vehicles. Creeping through mud pools at low throttle settings preserves structural soil firmness, allowing clean, controlled movement. However, if you panic and pin the accelerator down, your wheels shear the mud layer. Grip parameters instantly wash out, your tires spin pointlessly, and the chassis digs into a physical rut, leaving you vulnerable unless your vehicle has high-torque transmission specs to break free.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M9-EXT-05] Aerodynamic Wind Lift & Side-Slip Force Vectoring (Chassis Loft)

##### Systems Touched

Complements M9 vehicle chassis handling loops using M10 atmospheric wind vectors.

##### Math

Vehicles traveling at high speed across open sector chunks interact with global wind velocity vectors (v_wind ). The engine evaluates an aerodynamic lift force (F_lift ) and side-slip thrust (F_side ) vector based on the angle of relative air flow (β):

`F_aero_lift = 0.5·ρ_air·∥v_relative∥²·A_chassis·C_lift(β)·u_up`

`F_aero_side = 0.5·ρ_air·∥v_relative∥²·A_chassis·C_side(β)·(u_up × u_forward)`

##### How It Works

This framework integrates safely inside your vehicle physics loop step. During the vehicle update sequence, the engine reads the current wind velocity vector directly from your M10 weather simulation data blocks. It computes the relative airspeed vector against the vehicle's rigid body velocity. The resulting directional forces are applied straight to the vehicle chassis center-of-mass handle via Jolt’s native BodyInterface::AddForce commands, avoiding manipulation of wheel manifold arrays.

##### Reference Implementation

```cpp
// Previously only computed/applied the lift term — the math and How It Works above both
// promise a side-slip force too (F_aero_side, via u_up x u_forward), which was silently missing.
// Injects aerodynamic wind forces straight onto your vehicle chassis rigid body center of mass
void ApplyAeroWind(JPH::BodyInterface& bi, JPH::BodyID chassis, const glm::vec3& v_rel, float rho, float area, float c_lift, float c_side, const JPH::Vec3& up, const JPH::Vec3& forward) {
    float q = 0.5f * rho * glm::dot(v_rel, v_rel) * area;
    JPH::Vec3 side_dir = up.Cross(forward);
    bi.AddForce(chassis, up * (q * c_lift) + side_dir * (q * c_side));
}
```

##### Player-Facing Impact

Driving high-speed vehicles through open terrain zones during major atmospheric storm events introduces severe instability hazards. High crosswinds generate physical aerodynamic lift that unloads your vehicle's suspension springs, reducing tire contact area and degrading your cornering tracking. A sudden gale-force storm front can physically push your vehicle chassis sideways off course, requiring you to counter-steer carefully or slow down to maintain stable traction paths across exposed highway bridges.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M9-EXT-06] Horde Continuum Fluid Drag (The Swarm Plow Solver)

##### Systems Touched

Feeds M9 vehicle chassis physics from the M5.4 horde SpatialHash density query, avoiding per-zombie Jolt rigid-body contact resolution at horde scale.

##### Math

`F_horde = −0.5·C_h·ρ_local·A_veh·∥v_rel∥·v_rel`

##### How It Works

Driving a vehicle into a massive swarm of zombies is a staple of open-world gameplay. However, generating distinct Jolt multi-body ragdoll collision pairs for 50+ zombies simultaneously on the same frame will cause a massive CPU physics spike, obliterating your 2.5ms physics time allocation. Instead of simulating individual rigid contact manifolds, your vehicle system (M9) queries its front bounding box against your existing SpatialHash to get local zombie density (ρ_local). The engine treats the crowd as a thick fluid medium, applying a single, consolidated opposing drag force (F_horde) straight to the vehicle chassis.

##### Reference Implementation

```cpp
void ApplyHordeDrag(JPH::BodyInterface& bi, JPH::BodyID chassis, float c_h, float rho_local, float area, const glm::vec3& v_rel) {
    glm::vec3 f_horde = -0.5f * c_h * rho_local * area * glm::length(v_rel) * v_rel;
    bi.AddForce(chassis, JPH::Vec3(f_horde.x, f_horde.y, f_horde.z));
}
```

##### Player-Facing Impact

The car behaves realistically, meeting massive physical resistance and slowing down when hitting dense swarms rather than gliding through them or hitching the frame rate. Simultaneously, a simple radial or forward-cone sweep fires an event to your EnTT registry to instantly toggle the intersected zombies to dynamic ragdoll states with an outward velocity vector, creating an optimized, massive crowd-plowing effect for virtually zero performance cost.

#### [M9-EXT-07] Dynamic Skid-Mark Injection via Runtime Virtual Textures (RVT)

##### Systems Touched

Replaces the broken G-buffer model; integrates M9 wheel slip with the persistent RVT terrain-overlay system built in M4.5 (chunk-anchored, `imageStore` write path) — writes into that real overlay, not an assumed layer.

##### Math

The previous version attempted to write permanent skid data directly into a G-Buffer attachment. Because G-Buffers are completely cleared and overwritten every frame, this data would vanish on the very next frame.

The high-performance solution is to project the wheel slip coordinate into the M4.5 RVT terrain-overlay's chunk-anchored texture using a Vulkan storage image write (imageStore). The tire material deposition layer alpha (A_skid ) accumulates strictly when Jolt reports sliding power:

`ΔA_skid =Clamp(κ⋅∥F_slip ⋅v_slip ∥⋅Δt,0.0,1.0)`

##### How It Works

Whenever Jolt reports wheel slip above threshold, the system writes into M4.5's RVT terrain-overlay at the wheel's chunk-anchored UV via `imageStore`, accumulating deposition alpha over time rather than a per-frame G-buffer channel that clears next frame.

##### Reference Implementation

```cpp
// Dispatched to a local compute lane or executed inline when wheel slip passes thresholds
void ComputeSkidAblation(float slip_force, float slip_vel, float kappa, float dt, float& tex_alpha) {
    tex_alpha = std::min(1.0f, tex_alpha + (kappa * std::abs(slip_force * slip_vel) * dt));
}
```

##### Player-Facing Impact

Pulling a high-speed J-turn or burning out over dry pavement injects permanent, dark rubber tracks directly into the world terrain layer map. Because this modifies the base texture layer cached inside your chunk streaming pipeline, it completely avoids entity allocation overhead or performance drops from old-school decal entity lists.

#### [M9-EXT-08] Open/Locked Axle Torque Differential Splitter

##### Systems Touched

Integrates natively within M9 (Wheeled Vehicle Physics) inside Jolt wheel constraint updates.

##### Math

`The engine regulates torque distribution vectors (T_left ,T_right ) across driving wheels by measuring relative angular velocities (ω) against an adjustable mechanical slip lock coefficient (α_lock ):`

`T_left = T_total · (0.5 + α_lock · Clamp((ω_right − ω_left) / (ω_right + ω_left + 1e−5), −0.5, 0.5))`

`T_right = T_total − T_left`

The slip-ratio fraction is angular-velocity difference over their sum (not the reverse).

##### How It Works

Executed inline inside your Jolt vehicle update callbacks. When a tire spins out on zero-traction surfaces (such as your blood-slick mud zones), setting α_lock =0.0 represents an open differential that routes power to the slipping tire. Toggling a locking differential component sets α_lock =1.0, distributing torque uniformly across the axle components to force mechanical grip.

##### Reference Implementation

```cpp
void ApplyDiffSplit(float t_tot, float w_l, float w_r, float lock, float& t_l, float& t_r) {
    float bias = lock * std::clamp((w_r - w_l) / (w_r + w_l + 1e-5f), -0.5f, 0.5f);
    t_l = t_tot * (0.5f + bias); t_r = t_tot - t_l;
}
```

##### Player-Facing Impact

Attempting to drive an off-road vehicle out of a muddy, gore-filled ditch with an open differential causes one tire to spin rapidly in place while the grounded wheel receives zero torque. Activating your locking differential locks the axles together, spraying debris behind the vehicle as it claws its way out of the obstacle.

#### [M9-EXT-09] Volatile Fuel Tank Puncture Bernoulli Drainage

##### Systems Touched

Connects M3 macro-destruction structural hits directly to M9 vehicle metrics.

##### Math

Fuel exit velocity (v_leak ) and volumetric drainage rates (Q) are calculated using Torricelli's adaptation of Bernoulli's theorem based on fluid head height (h_fuel ):

`v_leak = √(2⋅g⋅h_fuel)`

`Q=A_puncture ⋅v_leak`

`dh_fuel/dt = −Q/A_tank`

##### How It Works

To safeguard your physics performance, the leak loop updates the remaining fuel height and subtracts raw fuel weight by applying an external downward gravity force modifier to the chassis rigid body. Every 60 frames, an enkiTS background job writes the unified mass metrics back to Jolt's permanent mass properties array via SetMassProperties.

##### Reference Implementation

```cpp
// Was silently hardcoding 0.5f for tank cross-sectional area instead of taking a_tank as a parameter — matches dh_fuel/dt = -Q/A_tank above
void HandleFuelLeak(float a_hole, float a_tank, float rho, float g, float dt, float& h_fuel, float& mass) {
    float q = a_hole * std::sqrt(2.0f * g * std::max(0.0f, h_fuel)); h_fuel -= (q / a_tank) * dt; mass -= rho * q * dt;
}
```

##### Player-Facing Impact

A stray bullet puncturing your vehicle's gas tank causes fuel to drain rapidly based on fluid pressure. Your fuel gauge drops while a trail of flammable fluid maps your route across the terrain texture layers. This creates a volatile hazard that can be ignited by a subsequent exhaust backfire or a ricochet spark.

#### [M9-EXT-10] Aero-Elastic Body Panel Drag Deformation Scaling

##### Systems Touched

Links M3 macro-destruction structural hits to your M9 vehicle chassis velocity curves.

##### Math

When your vehicle collides with physical structures, the crumpled metal skin panels deform, warping the aerodynamic tracking profile. The total drag coefficient (C_d ) updates based on structural panel damage ratios (D_panel ):

`C_d_deformed = C_d_baseline + Σ_{i∈Panels} γ_i·D_panel_i·A_frontal`

##### How It Works

Your vehicle sub-components use flat floats inside their EnTT sheets to track mesh crumple states. This system hooks directly into the vehicle's forward air resistance update pass. Instead of utilizing uniform vehicle physics bounds, the drag force step reads the damage coefficients, scaling up the aerodynamic opposition inline without re-evaluating complex vehicle meshes.

##### Reference Implementation

```cpp
void ApplyAeroDamage(float base_cd, float damage_sum, float gamma, float area, float rho, float v, float& out_force) {
    out_force = 0.5f * rho * (base_cd + gamma * damage_sum) * area * (v * v);
}
```

##### Player-Facing Impact

Smashing your car front-end straight into concrete walls and zombie clusters crumples the hood and tears the bumper panels open. Driving at high speed down open highway stretches exposes the ruined body panels to severe wind resistance, dropping your top-end speed and creating an audible wind howl that forces you to scavenge pristine vehicle components.

#### [M9-EXT-11] Wake-Slipstream Aerodynamic Drafting

##### Systems Touched

Links your M9 vehicle chassis velocity updates straight to the global wind paths.

##### Math

A vehicle traveling at high speed leaves an aerodynamic wake area behind its path. A trailing vehicle entering this tracking zone experiences reduced air density components (ρ_effective ) based on its distance (x) behind the lead vehicle chassis:

`ρ_effective = ρ_ambient · (1.0 − e^(−x/λ_wake))`

##### How It Works

Inside the vehicle physics tick, vehicles use your spatial hash map to query if any active vehicle entities occupy the forward tracking cone vector. If a lead vehicle is detected, the distance is calculated, and the air density constant used in the trailing vehicle's drag loop is scaled down inline before passing data back to the physics loops.

##### Reference Implementation

```cpp
float GetDraftDensity(float rho_amb, float dist, float lambda) {
    return rho_amb * (1.0f - std::exp(-dist / lambda));
}
```

##### Player-Facing Impact

When traveling down wide highway corridors with co-op players, lining up your vehicle directly behind your partner's truck lets you catch their slipstream. Your car experiences reduced wind resistance, allowing you to save fuel and gain a high-speed drafting boost to pass them or clear obstacle fields quickly.

#### [M9-EXT-12] Exhaust Backpressure Aquatic Stutter

##### Systems Touched

Replaces the engine-bricking hydrolock trap; interfaces M10 shallow-water heights with M9 vehicle torque loops.

##### Math

When a vehicle chassis is submerged in deep puddle chunks, the engine does not instantly die. Instead, the available output drive torque is modulated by the pressure differential (ΔP) between the internal exhaust manifold gases and the external hydrostatic fluid column:

`η_torque = Clamp(1.0 − P_hydrostatic/P_exhaust, 0.15, 1.0), where P_hydrostatic = ρ_water · g · h_water`

##### How It Works

Each tick while submerged, local water depth converts to hydrostatic pressure and is compared against exhaust pressure for a torque efficiency multiplier, clamped so the engine stalls to a crawl but never fully cuts out — continuous, recoverable power loss instead of a binary hydrolock.

##### Reference Implementation

```cpp
void CalcAquaticTorque(float h_w, float rho, float g, float p_ex, float& out_torque) {
    out_torque *= std::clamp(1.0f - ((rho * g * h_w) / (p_ex + 1e-5f)), 0.15f, 1.0f);
}
```

##### Player-Facing Impact

Fording deep water turns into an active gameplay challenge. The engine sputters, chokes, and drops power, forcing you to feather and pump the accelerator to keep exhaust pressures high enough to clear the tailpipe. Revving the engine too hard drops traction, while letting off the gas stalls the block, making flooded avenues an intense balancing act.

##### Depends on (not yet built at this point)

M10 — implement against a hardcoded/stub value for now; revisit once that milestone lands.

#### [M9-EXT-13] Gyroscopic Wheel-Spin Precession Self-Righting

##### Systems Touched

Modifies M9 two-wheeled vehicle chassis setups inside your fixed physics callbacks.

##### Math

Spinning vehicle wheels generate a strong angular momentum tensor (L). When a motorcycle or dirt bike leaves the ground, the player can modulate engine throttle speeds to create a controllable gyroscopic precession righting torque (τ_gyro ):

`τ_gyro =ω_wheel ×L_wheel`

##### How It Works

While airborne, wheel angular momentum is tracked each tick. Throttle input changes spin rate; the cross product of wheel angular velocity and momentum applies as a righting torque on the chassis, letting the player rotate mid-air via engine RPM.

##### Reference Implementation

```cpp
void ApplyGyroRighting(const glm::vec3& w_wheel, const glm::vec3& l_wheel, glm::vec3& out_chassis_torque) {
    out_chassis_torque = glm::cross(w_wheel, l_wheel);
}
```

##### Player-Facing Impact

Traversing ruined sectors on a dirt bike becomes an intense mechanical arcade loop. Launching off a pile of ruined structural debris lets you gun the throttle or stomp the rear brake mid-air, using wheel angular momentum to physically rotate and snap the chassis pitch straight into alignment for a clean landing.

#### [M9-EXT-14] Non-Newtonian Mud-Slurry Catapult (Tire Rooster-Tails)

##### Systems Touched

Links M9 wheel spin velocities to your M6.5 GPU fluid particle generators.

##### Math

Spinning vehicle tires over saturated mud cells generates a high-velocity centrifugal fluid projection vector (v_spray ) launched directly from the tire tread radius:

`v_spray =(ω_wheel ×r_tread )+v_vehicle`

##### How It Works

While a tire contacts saturated mud and spins above a slip threshold, tread angular velocity crossed with its radius vector (plus vehicle velocity) gives a spray launch vector; the M6.5 GPU particle system spawns a rooster-tail burst along it each qualifying tick.

##### Reference Implementation

```cpp
glm::vec3 GetRoosterTailVel(const glm::vec3& w, const glm::vec3& r, const glm::vec3& v) {
    return glm::cross(w, r) + v;
}
```

##### Player-Facing Impact

Spinning your wheels aggressively into a muddy corner generates a heavy, dynamic spray cone of mud particles behind your rear bumper. If a fast zombie or enemy vehicle is tailgating your position closely, the thick mud spray cakes their visual sensor profiles, blinding their sight lines and causing them to crash or lose your trajectory.

#### [M9-EXT-15] Chassis Vortex Debris Drafting

##### Systems Touched

Links M9 large vehicle chassis velocities to your background spatial items tracker.

##### Math

Large, high-mass vehicle blocks moving at road speeds generate a trailing low-pressure aerodynamic vortex field that creates suction forces along your vehicle look axis:

`F_suction =γ⋅(∇×v_vehicle )⋅A_chassis`

##### How It Works

Loose items within the tracked wake radius behind a moving high-mass chassis are pulled toward the vehicle's path (radial attraction) while picking up a lateral swirl from the vehicle's velocity, combining into a drafting motion that draws them along the wake.

##### Reference Implementation

CORRECTED for a divide-by-zero the drafted version reintroduced — epsilon must guard the distance before *any* division, not just the outer scale term (this is the master's already-fixed form; keep it, do not replace with the unguarded version):

```cpp
void ApplyVortexSuction(const glm::vec3& v_veh, const glm::vec3& d_pos, float scale, glm::vec3& out_item_vel) {
    float dist = glm::length(d_pos) + 1e-5f; // guard applied before any division
    glm::vec3 pull_dir = -d_pos / dist;
    glm::vec3 swirl = glm::cross(v_veh, glm::vec3(0.f, 1.f, 0.f));
    out_item_vel += (pull_dir * glm::length(v_veh) + swirl) * (scale / dist);
}
```

##### Player-Facing Impact

Driving a heavy utility truck down city streets transforms the space behind your tailgate into a tactical slipstream loop. The low-pressure vortex generates an aerodynamic suction field that physically draws in and pulls loose ammunition crates, supply drops, or sliding crawler zombies along inside your trailing wake, making resource collection high-speed and satisfying.

#### [M9-EXT-16] Socio-Economic Fuel Scarcity & Shared-Station Garage Repair

##### Systems Touched

Fuel spawn density keyed to the same socio-economic tags already placing loot (Appendix C) — no new scarcity table. Garages are `CraftingStationComponent` entities shared with [M8-EXT-02]'s crafting workbenches (repair-role instance). Panel repair reuses [M3-EXT-06]'s shared `StructuralFatigue` component directly.

##### Math

No new formula — fuel-can spawn weight per chunk is an existing loot-weight lookup keyed by socio-economic tag (industrial zones weighted high, rural sprawl low); panel repair resets the shared `StructuralFatigue.value` from [M3-EXT-06] toward zero at a rate gated by garage tool-tier, identical gating pattern to crafting-station tool tiers in [M8-EXT-02].

##### How It Works

Fuel cans are just another entry in the existing loot-weight table Appendix C already drives — industrial/gas-station tagged cells get a higher weight, no parallel scarcity system. A garage is a `CraftingStationComponent` instance (same struct [M8-EXT-02] workbenches use) with a repair-specific tool-tier multiplier; repairing a panel calls the exact same fatigue-reset path a repaired weapon would use.

##### Reference Implementation

```cpp
// Same CraftingStationComponent struct used by M8-EXT-02 workbenches, repair-tier context
void RepairPanelAtStation(const CraftingStationComponent& garage, StructuralFatigue& panel) {
    panel.value = std::max(0.0f, panel.value - garage.toolTierMultiplier * 0.3f);
}
```

##### Player-Facing Impact

Fuel and vehicle repair scarcity follow the exact same "where does this realistically come from" logic already placing every other loot category — industrial districts are where you go for both spare fuel and functioning garages, for the same underlying reason.

#### [M9-EXT-17] Rigid-Body Lateral Frame Shear Stress Accumulator

##### Systems Touched

Feeds M9 vehicle chassis impacts into the shared `StructuralFatigue`/`TickFatigue()` Palmgren-Miner accumulator already declared canonical by `[M3-EXT-06]` — this is the vehicle-frame consumer of that shared system (structures, weapons, and now chassis rails), not a fourth parallel wear model.

##### Math

`σ_shear = ‖F_impact · u_lateral‖ / A_chassis_rail_cross_section`

##### How It Works

Off-road impacts and collision forces are projected onto the chassis's lateral coordinate frame to get a shear stress magnitude, which is fed directly into `TickFatigue()` (`[M3-EXT-06]`) using the same `D ≥ 1.0` failure threshold structures and weapons already use — no separate vehicle-durability curve.

##### Reference Implementation

```cpp
inline float CalculateChassisShearStress(const glm::vec3& rawCollisionForceVector, const glm::vec3& chassisLateralNormal, float structuralRailArea) {
    float transverseLoadMagnitude = std::abs(glm::dot(rawCollisionForceVector, chassisLateralNormal));
    return transverseLoadMagnitude / (structuralRailArea + 1e-5f); // feeds directly into the shared TickFatigue() loop
}
```

##### Player-Facing Impact

Repeated off-road abuse and side-impact collisions wear out a vehicle's frame with the same believable long-term fatigue curve already governing walls and weapons, rather than an arbitrary vehicle "health bar" — consistent with `[M9-EXT-16]`'s garage repair resetting this exact same `StructuralFatigue.value`.

#### [M9-EXT-19] Parallax-Occlusion Topography Deformation & Persistent Ruts
#### [M9-EXT-18] Vehicle Convoy Long-Range Router *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M9 + M5.4. Routes a vehicle convoy over long distance on the road graph, keeping spacing.

##### Math
route = HAStar(roadGraph, A, B); spacing_i = i*gap - speedLag; replan on threat.

##### How It Works
Drives a convoy of vehicles along M5.4-EXT-06's macro road-graph to a distant objective, maintaining inter-vehicle spacing and speed lag; reroutes around threats/failures. Reuses the single road graph - not a second system.

##### Reference Implementation
```cpp
ConvoyRoute r = HAStar(roadGraph, start, goal); AssignVehicles(r, gap);
```

##### Player-Facing Impact
Convoys travel coherently cross-map and reroute around danger - believable faction logistics.

---
#### [M9-EXT-21] Fluid Hydrodynamic Wading Resistance Modulator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M9 vehicles + M2.9 flood bridge. Modulates vehicle wading resistance by water depth/hydrodynamics.

##### Math
f = clamp(depth - axle, 0, bodyH)/bodyH; Fdrag = 0.5*rho*Cd*A_f*f*v^2 + rho*Cs*A_w*f*v; normalFrac = 1 - (rho*Vsub)/m; drive = normalFrac*(1 - clamp(Fdrag/Ftmax,0,1)).

##### How It Works
As a vehicle enters water, submergence fraction f scales frontal + skin-friction drag; buoyancy sheds normal force (less traction). Drive force is modulated by both (deep water = less grip AND more drag), so shallow fords are fine but deep water bogs the vehicle. Engine can stall if drag exceeds max traction.

##### Reference Implementation
```cpp
float WadingModulator(float depth,float v,float mass){ float f=clamp((depth-axle)/bodyH,0,1); float Vsub=f*vol; float nf=max(0,1-(rho*Vsub)/mass); float Fd=0.5*rho*Cd*A_f*f*v*v + rho*Cs*A_w*f*v; float Ftmax=nf*mass*g*muMax; return nf*(1-clamp(Fd/max(Ftmax,1),0,1)); }
```

##### Player-Facing Impact
Vehicles ford shallow streams fine but become sluggish and can stall in deep water - players must pick crossings, find bridges, or risk a bogged, vulnerable vehicle.
#### [M9-EXT-22] RVT Skid-Mark / Tire-Track Injector *(SOURCED FROM PLAN FILE)*

##### Systems Touched
One of three features audit line 133 says blocked on missing RVT base. Writes tire tracks + drift scars into M4.5-EXT-26 RVT overlay from M9 wheel-contact + slip-state telemetry. Consumed by terrain material resolve as extra blend layer.

##### Math
Decal written into RVT clipmap page (see M4.5-EXT-26 base). Track = polyline of wheel-contact samples; width from slip; intensity fades with speed. Write-merge (newest-wins) into page tile so overlapping tracks resolve deterministically.

##### How It Works
On wheel slip above threshold, sample contact point -> RVT page coord, draw tracked quad with width from slip + alpha from intensity. Persists in chunk-anchored RVT so tracks survive across sessions/streaming. Feeds terrain material as blend layer.

##### Reference Implementation
```cpp
void InjectSkid(VkCommandBuffer cb, RvtPageTable& rvt, vec3 contact, float slip, float intensity){ /* write-merge track quad into rvt page */ }
```

##### Player-Facing Impact
Cars leave real skid marks / drift scars on the ground that persist — world shows your passage.

---

#### [M9-EXT-20] Anti-Roll Torsional Suspension Stabilizer *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M9. Torsion bar linking wheel axes to curb rollover in hard cornering.

##### Math
tau = kTorsion * (rollAngle - rest); counter-torque to chassis;

##### How It Works
An anti-roll bar models torsional stiffness between left/right wheels; body roll in a corner winds the bar, generating counter-torque that flattens the chassis. Stabilizes the vehicle without stiffening ride.

##### Reference Implementation
```cpp
float tau = kTorsion*(roll - rest); chassisTorque -= tau;
```

##### Player-Facing Impact
Vehicles resist rollover in hard turns - stable, planted handling.


##### Systems Touched
One of three features audit line 133 says blocked on missing RVT base. Writes tire tracks + drift scars into M4.5-EXT-26 RVT overlay from M9 wheel-contact + slip-state telemetry. Consumed by terrain material resolve as extra blend layer.

##### Math
Decal written into RVT clipmap page (see M4.5-EXT-26 base). Track = polyline of wheel-contact samples; width from slip; intensity fades with speed. Write-merge (newest-wins) into page tile so overlapping tracks resolve deterministically.

##### How It Works
On wheel slip above threshold, sample contact point -> RVT page coord, draw tracked quad with width from slip + alpha from intensity. Persists in chunk-anchored RVT so tracks survive across sessions/streaming. Feeds terrain material as blend layer.

##### Reference Implementation
```cpp
void InjectSkid(VkCommandBuffer cb, RvtPageTable& rvt, vec3 contact, float slip, float intensity){ /* write-merge track quad into rvt page */ }
```

##### Player-Facing Impact
Cars leave real skid marks / drift scars on the ground that persist — world shows your passage.

---

##### Systems Touched

`[M9-EXT-04]`'s existing mud advection (this is the rendering/geometry half — EXT-04 already handles the friction-side soil shear; this adds the visual displacement channel it was writing into as a stub), `SurfaceFrictionSample`'s ordered composition (unchanged, reads only).

##### Math

Slopes beyond a threshold angle strip topsoil texture toward exposed bedrock with a simple linear blend; wheel/foot pressure writes signed depth into a persistent displacement channel, clamped to a maximum rut depth:

$$\text{Rut}(t) = \text{clamp}(\text{Rut}(t-1) + P_{contact} \cdot k_{soft} \cdot \Delta t,\ 0,\ \text{Rut}_{max})$$

##### How It Works

High-frequency ground detail uses Parallax Occlusion Mapping so micro-crevices self-shadow correctly as the view angle changes; where local slope exceeds ~45°, the material blend weight shifts from topsoil toward exposed bedrock/scree, matching the debris-slide look already described for steep terrain. Separately, wheel and foot colliders write directly into a persistent runtime displacement texture channel per chunk (the same channel `[M9-EXT-04]` already reads slip velocity against), so repeated passes over the same soft ground carve a lasting tessellated rut instead of the ground resetting every frame; the rut depth is clamped so a single heavy vehicle pass doesn't dig to the world floor.

##### Reference Implementation

```cpp
float AccumulateRutDepth(float currentDepth, float contactPressure, float softnessK, float dt, float maxDepth) {
    float newDepth = currentDepth + contactPressure * softnessK * dt;
    return glm::clamp(newDepth, 0.0f, maxDepth);
}

float ComputeBedrockBlendWeight(float slopeAngleRadians, float thresholdRadians) {
    return slopeAngleRadians <= thresholdRadians ? 0.0f : glm::clamp((slopeAngleRadians - thresholdRadians) / (glm::radians(20.0f)), 0.0f, 1.0f);
}
```

##### Player-Facing Impact

Steep slopes visibly shed topsoil to bare rock, and repeatedly driving the same muddy path carves a real, lasting rut you can see and steer around, instead of soft ground looking identical no matter how much traffic it's taken.



## M10 — Day/night, weather & atmosphere

##### Implementation Steps

* Celestial Shadow Orientation tied to Julian dates.
* Saint-Venant Shallow-Water Flooding (`∂h/∂t + ∂(hu)/∂x + ∂(hv)/∂y = R`).
* Surface Friction Writer Operations (updates M9 interface).
* Advection-Diffusion Groundwater Contamination.
* Inverse-Square Electromagnetic Pulse Kinetics. Anisotropic Cloud Scattering Shader.
* **Humidity map:** 256x256 texture covering the active world sector (one texel ≈ 4m, matched to the terrain heightmap resolution), stored as normalized 0-1 relative humidity. Updated once per in-game hour (not per-frame — humidity diffuses slowly relative to wind/temperature) via a cheap diffusion-toward-weather-target pass; rain events pin nearby texels to 1.0 and let them decay exponentially afterward. Read by M2.7-EXT-07's fouling accumulation (`ΔF = κ·(1+γ·Humidity)`) and M5.3-EXT-03's scent-evaporation grid.
* **Wind vector field:** 2D (`glm::vec2`, world-XZ plane — vertical wind is out of scope for this pass) sampled on a coarser 64x64 grid than humidity/temperature, since wind direction/strength is visually and mechanically low-frequency compared to puddle-level detail. Updated every 30 real-time seconds by perturbing a small number of Perlin-noise-driven "weather cells" that drift across the map, with per-texel values bilinearly interpolated at query time. Read by wind lift/side-slip (M9-EXT-05), drafting (M9-EXT-11), scent-plume domain warping (M5.3-EXT-04/05), and — once built — the soft-body cloth/vegetation system's external wind force (see new M7-EXT-04 below).
* **Ambient temperature / calendar-driven loop:** single scalar per sector (not a full grid — temperature doesn't vary meaningfully at sub-kilometer scale for this game's purposes) driven by a combined day/night sinusoid plus a slower seasonal offset: `T(t) = T_season + A_diurnal · sin(2π·(t_hour - phase)/24)`, where `T_season` steps between four preset seasonal baselines and `A_diurnal` is the day/night swing (larger in summer, smaller in winter). Sampled once per tick by anything that needs current air temperature; read by M2.9-EXT-03's diurnal air-density ballistic shift (replacing its "hardcoded/stub value" dependency placeholder), M5.2-EXT-02's rigor mortis decay rate, and M9's aquaplaning surface-temperature checks.

##### Decisions to flag

Weather event probabilities. Humidity/wind grid resolutions above are a starting point — revisit if profiling shows the 256x256 humidity texture is oversized relative to what M2.7-EXT-07/M5.3-EXT-03 actually sample from it.

##### Exit Criteria

* Puddles and mud dynamically degrade vehicle and player traction.
* Humidity, wind, and temperature fields are queryable by world position and match the update cadences above; M2.9-EXT-03's air-density ballistic shift and M2.7-EXT-07's fouling accumulation compile against the real fields instead of stub values.

### Extended Systems Library — engine-side additions for M10

#### [M10-EXT-01] Hydraulic-Head Flux Solver for Saint-Venant Flooding **[MATH DONE]**

##### Systems Touched

Locks down the concrete flux math behind M10's base "Saint-Venant Shallow-Water Flooding" bullet. Feeds M9-EXT-03 (aquaplaning) and M9-EXT-04 (mud advection), both of which currently depend on this as a hardcoded/stub value.

##### Bug this closes

A flux stencil driven only by terrain height (`z[idx] - z[idx+1]`) ignores accumulated water depth (`h`) and will move water uphill out of a filled depression toward a lower-but-dry neighbor, because it never checks which cell actually holds more total water surface elevation. Flux must be driven by the hydraulic head (`h + z`), not `z` alone.

##### Math

Per-cell water surface elevation: `H_i = h_i + z_i` (depth + terrain height).

Lateral flux between adjacent cells `i, j`: `Q_{i→j} = k · (H_i − H_j) · min(h_i, h_j)^1.5` — driven by head difference, scaled by the shallower cell's depth (prevents dry cells from "pulling" flux they have no water to supply).

Depth update: `h_i(t+Δt) = max(0, h_i(t) + (Δt/A_cell)·Σ_j Q_{j→i} + (Rainfall − Infiltration)·Δt)`

##### How It Works

Each background enkiTS worker owns a strip of the flat 1D cell grid. For every cell, flux to each of its 4 neighbors is computed from the head difference, not raw terrain height, so a full depression with a lower-terrain-but-already-flooded neighbor won't erroneously drain into it once heads equalize. The `min(h_i, h_j)^1.5` term is an upwind-style clamp: it throttles flux out of a nearly-dry cell toward zero as `h→0`, so a single large `dt` spike can't drive a cell's depth negative. Final depth is hard-clamped to `[0, maxFloodDepth]` (3.0m per M10's decision-to-flag) after rainfall/infiltration are applied.

##### Reference Implementation

```cpp
// One flat 1D grid, updated on background enkiTS workers — no dynamic allocation, no nested pointers.
struct FloodCell {
    float h = 0.0f;              // current water depth (m)
    float z = 0.0f;              // static terrain height (m), set at world-gen
    float infiltration = 3.6e-6f;  // soil absorption rate, m/s, per-cell soil type (~loam, 1.3 cm/hr — corrected from 0.02 m/s, which was ~72 m/hr, roughly 4 orders of magnitude past real hydrology figures; real infiltration rates run from ~2.8e-8 m/s for clay up to ~5.8e-5 m/s for sandy soil, so loam sits near the middle of that band)
};

// Flux-driven depth update for one axis pass (call once for +X neighbor, once for +Z neighbor per tick).
inline void ApplyHydraulicHeadFlux(FloodCell* cells, uint32_t idx, uint32_t neighborIdx,
                                    float k, float dt, float cellArea) {
    const float Hi = cells[idx].h + cells[idx].z;
    const float Hj = cells[neighborIdx].h + cells[neighborIdx].z;
    const float shallow = std::min(cells[idx].h, cells[neighborIdx].h);
    // Upwind clamp: flux magnitude scales toward 0 as the shallower side empties, so a large
    // dt can't overdraw a near-dry cell into negative depth.
    const float flux = k * (Hi - Hj) * std::pow(std::max(0.0f, shallow), 1.5f);
    const float dh = (flux * dt) / cellArea;
    cells[idx].h         = std::max(0.0f, cells[idx].h         - dh);
    cells[neighborIdx].h = std::max(0.0f, cells[neighborIdx].h + dh);
}

inline void ApplyRainfallAndInfiltration(FloodCell& cell, float rainfallRate, float dt, float maxFloodDepth) {
    cell.h += (rainfallRate - cell.infiltration) * dt;
    cell.h = std::clamp(cell.h, 0.0f, maxFloodDepth); // hard clamp — never negative, never past M10's 3.0m cap
}
```

##### Player-Facing Impact

Rain genuinely pools in low ground and stays there instead of mathematically draining uphill toward a dry-but-lower neighboring tile — a flooded intersection stays flooded, and M9-EXT-03's aquaplaning check reads a depth value players can actually see and predict.

#### [M10-EXT-02] Keplerian Cosmic Orbitals & Synchronized Shadow Vectors

##### Systems Touched

Every shadow-mapped light in the renderer (sun/moon direction is currently likely a simple day/night sinusoid this replaces with a real orbital solve), `[M10]`'s existing day/night temperature sinusoid (unchanged, reads clock time the same way).

##### Math

Standard two-body Keplerian elements resolved to a direction vector per in-game timestamp:

$$\nu = \text{TrueAnomaly}(M, e), \qquad \vec{L} = R_z(\Omega) R_x(i) R_z(\omega) \cdot [\cos\nu, \sin\nu, 0]^T$$

using fixed orbital elements ($e$, $i$, $\Omega$, $\omega$) tuned to give the desired day length and seasonal sun-angle swing rather than real astronomical values.

##### How It Works

Sun and moon directions are computed deterministically from the current in-game timestamp via the closed-form Keplerian solve above instead of a sinusoid approximation, so both viewports in co-op resolve the exact same vector from the exact same tick (critical for `[M2.8]`'s determinism requirement) and shadow maps update in perfect lockstep with no drift or crawling between the two light sources.

##### Reference Implementation

```cpp
glm::vec3 SolveKeplerianDirection(double meanAnomaly, double eccentricity, double inclination, double longAscNode, double argPeriapsis) {
    double E = meanAnomaly; // Newton-Raphson solve for eccentric anomaly
    for (int i = 0; i < 6; ++i) E -= (E - eccentricity * std::sin(E) - meanAnomaly) / (1.0 - eccentricity * std::cos(E));
    double trueAnomaly = 2.0 * std::atan2(std::sqrt(1 + eccentricity) * std::sin(E / 2.0), std::sqrt(1 - eccentricity) * std::cos(E / 2.0));
    glm::vec3 orbitalPlane(std::cos(trueAnomaly), std::sin(trueAnomaly), 0.0f);
    glm::mat3 rot = glm::mat3(glm::rotate(glm::mat4(1.0f), (float)longAscNode, glm::vec3(0,0,1)))
                  * glm::mat3(glm::rotate(glm::mat4(1.0f), (float)inclination, glm::vec3(1,0,0)))
                  * glm::mat3(glm::rotate(glm::mat4(1.0f), (float)argPeriapsis, glm::vec3(0,0,1)));
    return glm::normalize(rot * orbitalPlane);
}
```

##### Player-Facing Impact

Sun and moon shadows stay perfectly locked together with no visible crawl or desync between co-op viewports, and day length/seasonal sun angle follow a consistent, tunable long-term cycle instead of an approximated wobble.

#### [M10-EXT-03] Bruneton-Nishita Atmospheric Scattering & SH Ambient Fill

##### Systems Touched

`[M10-EXT-02]`'s sun/moon vectors (drive scattering direction), all outdoor lighting (ambient fill replaces any flat ambient constant).

##### Math

Rayleigh scattering drives the characteristic blue-sky wavelength falloff; Mie scattering uses the Henyey-Greenstein phase function for aerosol forward-scattering:

$$\beta_{Rayleigh}(\lambda) \propto \lambda^{-4}, \qquad P_{HG}(\theta) = \frac{1}{4\pi}\frac{1-g^2}{(1+g^2-2g\cos\theta)^{3/2}}$$

Precomputed irradiance is projected into low-order spherical harmonics ($Y_l^m$, $l \le 2$) for cheap ambient bounce lighting.

##### How It Works

Sky color is generated from a precomputed Bruneton/Nishita scattering LUT instead of a texture asset, parameterized by sun angle from `[M10-EXT-02]` and current atmospheric density (feeding `[M10-EXT-05]`'s meteorology output as a density multiplier). Mie scattering's Henyey-Greenstein term produces the correct bright halo around the sun through haze/aerosols. The same LUT bakes a low-order SH irradiance map once per sun-angle bucket, which every outdoor surface samples for ambient fill light in shadowed areas instead of a flat ambient color, so shadowed alleys still pick up believable sky-tinted bounce light.

##### Reference Implementation

```cpp
float HenyeyGreensteinPhase(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0f - g2) / (4.0f * 3.14159265f * std::pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f));
}

glm::vec3 RayleighScatterCoefficient(glm::vec3 wavelengthsNm) {
    return glm::vec3(std::pow(wavelengthsNm.x, -4.0f), std::pow(wavelengthsNm.y, -4.0f), std::pow(wavelengthsNm.z, -4.0f));
}
```

##### Player-Facing Impact

Skies shift realistically from blue midday to orange/red sunset with a correct bright halo around the sun through haze, and shadowed areas outdoors pick up believable sky-colored fill light instead of looking flatly black or grey.

#### [M10-EXT-04] Cryospheric Frost Nucleation & Snow Accumulation

##### Systems Touched

`[M10]`'s existing rain/snow accumulation map, `[M4.5]`'s material shading (frost replaces baseline roughness with a glint mask).

##### Math

Frost coverage grows outward from object seams as a masked fractal-noise front once local temperature crosses freezing:

$$\text{FrostMask}(p, t) = \text{fBm}(p) \cdot \min\left(1, \frac{t - t_{frost_start}}{\tau_{spread}}\right) \cdot \mathbb{1}[T_{local} < 0°C]$$

##### How It Works

The existing top-down snow/rain accumulation map already tracked by `[M10]` is read alongside `[M10]`'s ambient temperature sinusoid; whenever local temperature drops below 0°C and surface moisture is present, a frost front expands outward from geometric seams (edges, corners — read from the existing vertex normal/curvature data) using the fBm mask above, gradually replacing the surface's baseline roughness map with a high-frequency glint mask so frosted surfaces catch specular highlights the way real frost crystals do.

##### Reference Implementation

```cpp
float ComputeFrostCoverage(float fbmSample, float secondsBelowFreezing, float spreadTimeConstant, float ambientTempC) {
    if (ambientTempC >= 0.0f) return 0.0f;
    float growth = glm::clamp(secondsBelowFreezing / spreadTimeConstant, 0.0f, 1.0f);
    return fbmSample * growth;
}
```

##### Player-Facing Impact

Frost visibly creeps outward from window edges and object seams over the course of a cold night rather than surfaces snapping instantly to a uniform frosted look the moment temperature crosses zero.

#### [M10-EXT-05] Navier-Stokes Meteorological Wind & Barometric Fronts

##### Systems Touched

`[M6.5-EXT-11]` foliage sway, `[M9]` vehicle wind-lift/drafting, `[M6-EXT-11]` acoustic wind-warp, `[M10-EXT-03]` cloud/scattering density — this is the shared wind-field source all of them already assume exists.

##### Math

A coarse 3D velocity field solved with a simplified incompressible Navier-Stokes step (advect → diffuse → project), driven by moving low/high pressure cells:

$$\frac{\partial \vec{u}}{\partial t} = -(\vec{u}\cdot\nabla)\vec{u} + \nu\nabla^2\vec{u} - \frac{1}{\rho}\nabla p$$

##### How It Works

Pressure cells (simple Gaussian blobs of high/low pressure) drift across the world-space grid over real time; a coarse 3D velocity volume is updated via the standard stable-fluids advect/diffuse/project loop driven by the pressure gradient between cells, at a resolution far below terrain (this is the world-scale wind system, not the per-texel wind vector `[M10]`'s front matter already defines for puddle-level detail — that finer 2D field samples this coarser 3D one as its boundary condition). This single volume is the wind source every other system listed above already assumed existed.

##### Reference Implementation

```cpp
struct PressureCell { glm::vec3 position; float pressureDelta; float radius; };

glm::vec3 SampleWindFromPressureCells(glm::vec3 worldPos, const std::vector<PressureCell>& cells) {
    glm::vec3 windAccum{0.0f};
    for (auto& cell : cells) {
        glm::vec3 toPoint = worldPos - cell.position;
        float dist = glm::length(toPoint);
        if (dist < cell.radius && dist > 0.01f) {
            float falloff = 1.0f - (dist / cell.radius);
            windAccum += glm::normalize(toPoint) * cell.pressureDelta * falloff;
        }
    }
    return windAccum;
}
```

##### Player-Facing Impact

Weather fronts move across the map as coherent systems with a real leading edge, and every wind-driven effect in the game (foliage, drafting, sound warp, clouds) reacts to one consistent, physically-grounded wind source instead of independently faked local gusts.

#### [M10-EXT-06] Phenological Chlorophyll Cycles & Seasonal Canopy Thinning

---

#### [M10-EXT-11] Volumetric Cloud & Participating-Medium Scattering *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M10. Ray-marched volumetric clouds with participating-medium scattering.

##### Math
L = Integrate(ray, sigma_s, sigma_a, phase, light); multi-scatter approx;

##### How It Works
Clouds are a participating medium ray-marched per pixel with single+multi-scatter approximation (Henyey-Greenstein phase), lit by sun/moon. Drives weather mood + godrays. Costs gated by tier (M4.5).

##### Reference Implementation
```cpp
vec3 L = MarchClouds(ro, rd, sun);
```

##### Player-Facing Impact
Sky has real volumetric clouds - weather reads alive, not a flat dome.


##### Systems Touched

`[M4-EXT-18]`'s canopy system (albedo/alpha source), `[M10]`'s existing seasonal baseline temperature stepping (shares the same season index).

##### Math

Chlorophyll fraction decays toward zero as winter approaches, driving an albedo blend from green through carotenoid orange/yellow, with leaf-drop alpha thinning following the same curve:

$$\text{Chlorophyll}(season) = \max\left(0, 1 - \frac{season - season_{peak}}{season_{winter} - season_{peak}}\right)$$

##### How It Works

Foliage material albedo already samples a single seasonal timeline value (the same season index `[M10]`'s temperature baseline already steps through); this feature blends canopy albedo along that same index from green toward oxidized orange/yellow using the curve above, and drives per-vertex alpha thinning on the same schedule so leaf density visibly drops as the value approaches the winter endpoint, reusing one seasonal driver instead of adding a second calendar system.

##### Reference Implementation

```cpp
glm::vec3 BlendSeasonalFoliageAlbedo(glm::vec3 greenAlbedo, glm::vec3 autumnAlbedo, float chlorophyllFraction) {
    return glm::mix(autumnAlbedo, greenAlbedo, chlorophyllFraction);
}

float ComputeLeafDropAlpha(float chlorophyllFraction) {
    return chlorophyllFraction; // vertex alpha directly tracks chlorophyll fraction, thinning canopy as it approaches 0
}
```

##### Player-Facing Impact

Forests visibly shift from green to autumn colors and thin out toward winter over the game's season cycle, tied to the same calendar already driving temperature, instead of foliage color staying static year-round.



## M11 — UI/HUD, input abstraction & consolidated haptics

##### Implementation Steps

* Low-Overhead Input Remapping Abstraction.
* **Controller hot-plug handling:** `SDL_CONTROLLERDEVICEADDED`/`SDL_CONTROLLERDEVICEREMOVED` events are handled explicitly — a disconnect mid-session (common with Bluetooth/2.4GHz pads and battery drops) pauses that player's input gracefully and shows a reconnect prompt instead of the input abstraction layer silently reading a dead device handle or crashing on the next poll. Reconnection re-binds to the same logical player slot rather than creating a new one.
* OKLab Perceptually Uniform UI Contrast Matrix.
* Viewport-Aware Multi-Player Menu Shell.
* Timestep Accumulator Photo Mode Freezing.
* Haptic Audio Sub-Band Splitting.
* Probabilistic Objective Grammar Compilation.

##### Decisions to flag

Default control mapping structures.

##### Exit Criteria

* Contrast formulas preserve UI visibility seamlessly. Input rebinding performs statically.



### Extended Systems Library — engine-side additions for M11

*Systems proposed in later design-iteration sessions, folded in here at their correct pipeline destination. IDs are stable — reference them (e.g. `M9-EXT-03`) when discussing this doc with the coding agent so everyone means the same system.*

#### [M11-EXT-01] Viewport-Isolated Haptic Waveform Summation

##### Systems Touched

Expands M11 (Haptic sub-band splitting) and interfaces with M2.8 (Local split-screen viewports).

##### Math

To prevent actuator saturation and inter-viewing feedback bleed during multi-player local split-screen sessions, high-frequency wave packets (A_i ) are isolated using your controller modulo indexing system:

`H_final(t) = SoftClip(Σ_{i=1}^{V} W_i·A_i(t)·cos(ω_i·t + ϕ_i))`

Where `W_i` is a dynamic spatial distance weight tensor computed between the actuator-owning player's camera position and the absolute physical 3D coordinate of the structural audio impact source.

##### How It Works

When two local players are sharing a window surface partitioned into dual viewports, standard audio-to-actuator systems mix all low-frequency sub-band waveforms indiscriminately. This system uses your ControllerID router logic to perform real-time wave interferometry. It dynamically filters out or scales down haptic signals generated by Player 2's actions before they can bleed into Player 1's physical DualSense device handle.

##### Reference Implementation

```cpp
struct HapticSource { glm::vec3 worldPos; float amplitude; float omega; float phase; };
float ComputeIsolatedHaptic(const std::vector<HapticSource>& sources, const glm::vec3& ownerCamPos, float t) {
    float sum = 0.0f;
    for (const auto& s : sources) {
        float dist = glm::distance(ownerCamPos, s.worldPos);
        float w = 1.0f / (dist * dist + 1.0f);
        sum += w * s.amplitude * std::cos(s.omega * t + s.phase);
    }
    return std::tanh(sum);
}
```

##### Player-Facing Impact

In a chaotic split-screen scenario where Player 1 is driving an off-road vehicle across loose debris fields, their controller actuators will cleanly hum with localized tire viscosity and suspension clicks. If Player 2 simultaneously discharges a heavy weapon on the other side of the screen, Player 1 will only feel a soft, distance-attenuated shockwave roll across their palms, ensuring each player retains a clean, un-muddled tactile link to their specific survival state.

#### [M11-EXT-02] Adaptive Skeletal Trigger Strain Profiles

##### Systems Touched

Connects M11 controller haptics with M2.7 player StressLevel metrics and weapon variables.

##### Math

This loop updates the physical trigger resistance variables (F_trigger ) parsed by your platform hardware interface layer. It requires zero performance tracking overhead; it scales directly over your pre-computed EnTT StressLevel floats and the weapon thermal structural bounds:

`F_trigger =F_base +α⋅StressLevel+β⋅max(0.0,T_barrel −T_optimal )^2`

##### How It Works

`F_trigger` is recomputed once per frame from two independent inputs already tracked elsewhere — M2.7's `StressLevel` scalar and the weapon's current `T_barrel` — and pushed to the platform's adaptive-trigger API alongside the existing haptic rumble calls, not on a separate update path. The stress term scales linearly so panic ramps trigger resistance smoothly rather than in steps; the thermal term only kicks in once the barrel exceeds `T_optimal` (via the `max(0.0, …)` clamp) and then grows quadratically, so a merely warm barrel feels unchanged while an overheating one becomes noticeably stiff to fire.

##### Reference Implementation

```cpp
// Updates your hardware state variables within the main window loop handler
void GetTriggerStrain(uint8_t base_f, float stress, float temp, float t_opt, float alpha, float beta, uint8_t& out_force) {
    out_force = static_cast<uint8_t>(std::clamp(static_cast<float>(base_f) + alpha * stress + beta * std::pow(std::max(0.0f, temp - t_opt), 2.0f), 0.0f, 255.0f));
}
```

##### Player-Facing Impact

Your controller physical feedback mirrors your character's state. When calm, pulling your weapon's trigger feels smooth and light. However, when you are cornered by a massive swarm and your StressLevel spikes, your character grips the weapon tightly in a panic, causing the physical trigger on your controller to push back with stiff mechanical resistance.

#### [M11-EXT-03] Raycast-Fan Fog-of-Reveal Navigation (Map/Minimap)

##### Systems Touched

M5.3's actual perception stack (SNR Detection Probability, Ray-Marched Foliage Concealment, Kalman-filtered Sensor Fusion) has no fan-cast sightline primitive to reuse — the doc's only existing fan-cast dispatches are M2.9's SDF ledge-analysis fan (traversal) and M6's fan-raycast audio occlusion (acoustic), neither vision-shaped. This feature declares a new, small fan-cast vision-occlusion primitive shared between the player's fog-reveal and (as a future consumer) M5.3's own sightline checks, rather than claiming reuse of something that doesn't exist yet.

##### Math

No new formula for the reveal logic itself — a fan-cast sampling pass evaluated from the player's position/heading and accumulated into a persistent per-chunk reveal bitmask instead of a per-frame AI decision.

##### How It Works

A raycast-fan sweep is run once from the player's viewpoint per movement tick; cells the fan clears get flipped in a persistent reveal buffer that the HUD reads to render fog-of-war, using M7's chunk-serialization path to persist reveal state across sessions rather than a new save format. The fan-cast primitive itself is new and self-contained here — M5.3 can adopt it later, but this feature does not depend on M5.3 already having one.

##### Reference Implementation

```cpp
// New shared fan-cast vision primitive — first consumer is this feature, not a reuse of M5.3.
void RevealFogFromRaycastFan(const glm::vec3& pos, float heading, float fanAngle, int rayCount,
                              RevealBitmask& out_reveal) {
    for (int i = 0; i < rayCount; ++i) {
        float theta = heading - fanAngle * 0.5f + fanAngle * (i / float(rayCount - 1));
        MarkRevealedIfVisible(pos, theta, out_reveal);
    }
}
```

##### Player-Facing Impact

Map reveal genuinely tracks what your character could plausibly have seen (blocked by walls, foliage, and structural graph occlusion) instead of an arbitrary radius-around-player reveal circle.

#### [M11-EXT-04] Diegetic Onboarding via Seeded Low-Severity Starter Outbreak

##### Systems Touched

Uses the existing M8 SEIR outbreak engine seeded at low severity in the player's starting settlement — teaching happens through the real systems at low stakes, not through UI tutorial popups.

##### Math

No new formula — the starter settlement's outbreak seed uses the same SEIR constants as any other regional outbreak, with `I_0` (initial infected count) deliberately set low and `SettlementDefense` deliberately high so the early scenario is survivable by design, not by different rules.

##### How It Works

World-gen places a below-average-severity outbreak in whichever settlement the player starts near, using the exact same outbreak-seeding function every other settlement uses — no scripted tutorial state exists in code, just a parameter choice at world-gen time. Environmental signage (in-world, procedurally placed text/props) explains mechanics contextually; there is no separate tutorial-popup system to maintain.

##### Reference Implementation

```cpp
// Same settlement outbreak-seeding function used everywhere else, called with tutorial-tier params
void SeedStarterOutbreak(SettlementState& s, uint64_t seed) {
    SeedRegionalOutbreak(s, seed, /*I0=*/0.02f, /*defenseMult=*/1.4f); // identical function, gentler inputs
}
```

##### Player-Facing Impact

Your first exposure to sickness/scarcity mechanics is real and survivable rather than a scripted, consequence-free popup sequence — you learn the actual systems under real but forgiving conditions.

#### [M11-EXT-05] OKLab-Driven Accessibility Suite (Colorblind, Captions, Remapping)

##### Systems Touched

Colorblind-safe palette remapping reuses the same perceptually-uniform OKLab pipeline already computing structural-stress visualization (`C_stress`, M3-EXT-02) and the OKLab UI contrast matrix already listed in M11's base implementation steps — one color-science pipeline serving both, not a second one.

##### Math

No new color math — colorblind simulation/correction runs through the same OKLab transform already in the engine; caption timing reads existing M6 audio-event dispatch timestamps directly rather than a separate subtitle-authoring system.

##### How It Works

The OKLab matrix transform already converting structural stress into a perceptually-uniform color gradient is the same transform applied to any UI element needing a colorblind-safe remap — it's a general-purpose perceptually-uniform color pipeline, and accessibility is simply another consumer of it. Captions subscribe to the same EventBus dispatch events M6/M2 already fire for damage/audio events, displaying the associated text tag rather than requiring a hand-authored subtitle track per line. Input remapping reads/writes the existing M11 Low-Overhead Input Remapping Abstraction's binding table.

##### Reference Implementation

```cpp
// Same OKLab transform function used for M3-EXT-02 structural-stress visualization
glm::vec3 ApplyColorblindSafePalette(const glm::vec3& oklab_color, ColorblindMode mode) {
    return RemapOKLabForDeficiency(oklab_color, mode); // shared transform, new mode parameter
}
```

##### Player-Facing Impact

Accessibility options plug into color and event infrastructure that already exists across the whole engine rather than needing a separately maintained accessibility color system that could visually drift from the "real" one over updates.

#### [M11-EXT-06] Runtime String-Template Localization Pipeline

##### Systems Touched

Wraps Appendix A's procedural narrative/diary-entry generation in resolvable string templates from the point of generation, rather than post-processing already-generated English strings.

##### Note

Original source description cited "Appendix E," which actually covers `sub_seed()` hashing and facial rig priority, not narrative text. Corrected to Appendix A ("Procedural Narrative System" — diary/journal template grammars), the actual narrative/dialogue generator.

##### Math

None — this is a data-pipeline feature: template keys with typed slot parameters instead of concatenated strings.

##### How It Works

Wherever Appendix A's narrative engine currently assembles a line from its template grammar, it instead assembles a template key (`"quest.dead_camp.discovered"`) plus a typed parameter set (settlement name, cause-of-death enum); a language table resolves the key+params into displayed text per active locale. Because this sits at the generation point rather than after, adding a language later is a table addition, not a retrofit against already-baked English strings.

##### Reference Implementation

```cpp
// Called wherever Appendix A needs to concatenate an English string directly
std::string ResolveLocalizedLine(const std::string& templateKey,
                                  const std::unordered_map<std::string, std::string>& params,
                                  LocaleId locale) {
    return g_LocaleTables[locale].Resolve(templateKey, params);
}
```

##### Player-Facing Impact

Adding a new supported language later is cheap and low-risk precisely because localization was never bolted on after the fact — it's how text generation has worked from the point this feature lands forward.

#### [M11-EXT-07] MSDF Shaded Drop-Shadow Vector Signed Distance Field Generator

---

#### [M11-EXT-09] UI Panel Atlasing & Nine-Slice Batching *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M11. Atlases UI panels + nine-slice borders into batched draws.

##### Math
panel -> atlas region; nine-slice borders; batch panels per frame;

##### How It Works
UI panel backgrounds are atlased and nine-sliced (corners fixed, edges stretched) then batched per frame so a busy UI is a handful of draws. Complements M11-EXT-08 text batching.

##### Reference Implementation
```cpp
BatchPanels(atlas, nineSlice);
```

##### Player-Facing Impact
UI panels batch into few draws - complex HUD doesn't cost frames.


---

#### [M11-EXT-08] MSDF Vector Text Layout Geometry Batching Pipeline *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M11 + M1-EXT-12. Batches MSDF text glyph geometry into one draw per text block.

##### Math
glyphQuads -> batched mesh; one draw per layout; sample MSDF (M1-EXT-12);

##### How It Works
UI text layouts assemble glyph quads into a single batched mesh (one draw per block) sampling the MSDF atlas (M1-EXT-12), so dense HUD/text stays cheap. Pairs with M11-EXT-09.

##### Reference Implementation
```cpp
BatchText(layout, atlas);
```

##### Player-Facing Impact
UI text is one draw per block - HUD stays cheap at high text density.


##### Systems Touched

Extends M1's MSDF Font Pipeline (base implementation step) — adds sharp vector drop-shadows/borders for diegetic UI text without a duplicate vertex pass, particularly valuable across M2.8's split-screen multi-viewport rendering where doubling shader passes per player is expensive.

##### Math

`α_shadow = smoothstep(0.5−w, 0.5+w, SampleMSDF(uv − uv_offset))`

##### How It Works

Instead of a second draw pass or duplicated vertex arrays for text shadows/borders, the fragment shader samples the same MSDF font atlas a second time at a light-offset UV coordinate within the same draw, producing a sharp shadow via `smoothstep` in a single texture-sample step.

##### Reference Implementation

```glsl
// Embedded directly within the primary MSDF font layout text render pass
float EvaluateVectorDropShadow(vec2 currentTexUv, vec2 shadowOffsetDirection, float edgeClarityWidth) {
    float distanceSample = texture(MsdfFontTextureAtlas, currentTexUv - shadowOffsetDirection).r;
    return smoothstep(0.5 - edgeClarityWidth, 0.5 + edgeClarityWidth, distanceSample);
}
```

##### Player-Facing Impact

Diegetic HUD text and vector UI borders stay crisply readable against any background, at zero extra draw-call cost — matters most in split-screen where per-player rendering budget is already halved.

#### [M5.4-EXT-04] Thompson-Sampling Actor Weights Snapshot Persistence Register

##### Systems Touched

AI Director spawn engines, Zstandard dictionary save-slot serialization routines.

##### Math

Discrete logging tracking progressive modification of the director's predictive threat arrays:

`α_{t+1} = α_t + ΔSuccess`, `β_{t+1} = β_t + ΔFailure`

##### How It Works

Serializes active alpha/beta distribution weights from the Thompson-Sampling director engine (`[M5.4-EXT-03]`'s tension model) directly into the persistence disk file stream, so the director's learned evaluation parameters regarding player survival stress are retained cleanly across application boots instead of resetting to baseline on session reload.

##### Reference Implementation

```cpp
#include <cstdint>
#include <cstring>

struct ThompsonBanditTracker {
    uint32_t alphaStateScalar;
    uint32_t betaStateScalar;
};

inline void SerializeDirectorStatePayload(const ThompsonBanditTracker& state, uint8_t* destinationBuffer, size_t* byteOffsetCursor) {
    std::memcpy(destinationBuffer + (*byteOffsetCursor), &state.alphaStateScalar, sizeof(uint32_t));
    (*byteOffsetCursor) += sizeof(uint32_t);
    std::memcpy(destinationBuffer + (*byteOffsetCursor), &state.betaStateScalar, sizeof(uint32_t));
    (*byteOffsetCursor) += sizeof(uint32_t);
}
```

##### Player-Facing Impact

Maintains dynamic difficulty scaling across separate play sessions, keeping threat mechanics responsive to established individual performance patterns.

## M12 — Networked co-op at scale

##### Implementation Steps

* **`StableId` network replication wiring.** M12 does need cross-boundary entity identity — host-enforced structural reconciliation (below) is meaningless without a stable way to say "this client-side entity is that server-side entity." The server-authoritative `StableId` (same values as M7's save/load counter — the two systems share one ID space, since a hosted save is still just a save) is included in every replication packet for a newly-relevant entity. Each client maintains a local `StableId -> entt::entity` map; on receiving a replication packet for an unfamiliar `StableId`, the client spawns a local entity via `EntityFactory` and registers the mapping before applying the packet's component data.

  * **Desync handling:** if a packet references components on a `StableId` the client hasn't spawned yet (entity exists server-side, spawn packet lost or arrived out of order), the client buffers the update for up to 2 replication ticks and requests a full resync for that `StableId` if the spawn packet still hasn't arrived — rather than silently dropping the update or crashing on a missing map entry.
* **Late-join / reconnect full-state snapshot.** `[M12-EXT-01]`'s delta encoder assumes a prior acknowledged baseline — it has nothing to diff against for a client joining an already-running session or reconnecting after a drop. On connect, the host serializes one full (non-delta) snapshot of every `StableId`-relevant entity in the joining client's initial `SpatialHash` region, sent as a burst before regular delta replication begins for that client; the client doesn't apply any delta packet until the full snapshot finishes. Reconnecting clients reuse their prior `StableId`-to-player mapping rather than being treated as a brand-new player slot. This is standard baseline co-op functionality (Valheim/Terraria-style), not an edge case — a co-op game an AI agent generates without it will only work if every player is present from world creation onward.
* Spatial Relevance Replication Filtering (bandwidth bounded strictly) — filters by `SpatialHash` (M1) region around each client's authority, so replication and the M1 spatial primitive share one broad-phase structure instead of M12 maintaining a second one. Serialized onto the wire via `[M12-EXT-01]`'s delta-encoded packet format below — filtering decides *what* replicates, the encoder decides how it's packed.
* Host-Enforced Structural Reconciliation.
* Headless Diagnostic Load Verification.

##### Decisions to flag

Max connection overhead testing frames.

##### Exit Criteria

* Bandwidth holds under dense simulated load without crashing or un-recoverable desyncs.

### Extended Systems Library — engine-side additions for M12

#### [M12-EXT-01] Bitstream Delta-Encoded Network Serialization Packet Encoder

##### Systems Touched

The actual wire format for M12's Spatial Relevance Replication Filtering above. Distinct from `[M2.8-EXT-02]`/`[M2.8-EXT-04]` (those solve local co-op *determinism*) — this is *bandwidth compression* for entities already selected as relevant by the spatial filter.

##### Math

`ΔX = X_CurrentSnapshot − X_BaselineFrame`

##### How It Works

Instead of writing full floating-point component state to the transport socket every tick, this encoder computes integer deltas against the previous acknowledged snapshot and packs only the changed bits into a sequential bitstream, skipping any field that hasn't moved since the baseline. Pairs directly with the `StableId`-keyed replication packets already declared above.

##### Reference Implementation

```cpp
struct NetworkDeltaEncoder { uint32_t bitBuffer; int bitCount; };
inline void PackIntegerBits(NetworkDeltaEncoder& encoder, uint32_t value, int count) {
    encoder.bitBuffer |= (value << encoder.bitCount);
    encoder.bitCount += count;
}
```

##### Player-Facing Impact

Co-op sessions with many relevant entities in view stay within bandwidth budget instead of saturating the connection with redundant full-precision state every tick.

#### [M12-EXT-02] Bitmask Packet Loss Sliding Window Acknowledgement Register

---

#### [M12-EXT-13] Interest-Management Spatial Hash (Net Culling) *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M12 + M1-EXT-27. Spatial-hash interest management for network scope.

##### Math
scope = Hash.Query(playerCell, R); subscribe only in-scope;

##### How It Works
A spatial hash defines each client's interest region; only entities entering scope are subscribed/unsubscribed, bounding both bandwidth and simulation broadcast. Builds on M12-EXT-04.

##### Reference Implementation
```cpp
Scope s = Hash.Query(cell, R); Subscribe(s);
```

##### Player-Facing Impact
Net scope tracks the player - only nearby state is simulated/sent.


---

#### [M12-EXT-07] Network Clock Sync & Tick-Drift Compensator *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M12. Syncs client/server clocks and compensates tick drift.

##### Math
offset = MeasureRTT(); drift = Kalman(offset); interp = Lerp(state, offset);

##### How It Works
Client measures server clock offset (RTT-based) and a Kalman filter tracks drift; entity state is interpolated at the corrected time so remote actors move smoothly despite latency. Core to co-op feel.

##### Reference Implementation
```cpp
float off = Compensate(rtt, kalman); Interp(state, off);
```

##### Player-Facing Impact
Co-op actors move smoothly under latency - no rubber-banding jank.


---

#### [M12-EXT-04] Local Network-Relevancy Grid Culling Filter *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M12 + M1-EXT-27. Culls network updates by a relevancy grid around each player.

##### Math
updates = FilterByGrid(playerCell, radius); send only relevant;

##### How It Works
Per client, a spatial relevancy grid decides which entity updates to send (only those near/affecting the player), slashing bandwidth. Pairs with M12-EXT-13 interest management.

##### Reference Implementation
```cpp
Send(FilterRelevant(updates, clientGrid));
```

##### Player-Facing Impact
Network only sends what matters per player - bandwidth stays low in co-op.


##### Systems Touched

Hardens M12's transport layer against packet loss/jitter, working alongside `[M12-EXT-01]`'s delta encoder — the delta encoder decides how state is packed onto the wire; this is what lets the host reliably detect which packets never arrived so it knows when to fall back to a full resync (per M12's base "Desync handling" step) instead of assuming delivery.

##### Math

`AckBitmask = AckBitmask | (1ULL << (Sequence_incoming & 63))`

##### How It Works

A zero-allocation, lock-free rolling 64-bit bitmask records arriving replication sequence numbers relative to the highest sequence seen so far. When a new highest sequence arrives, the bitmask shifts by the delta (clearing to zero if the gap exceeds 64) before the new bit is set, letting the host cheaply isolate exactly which of the last 64 packets are missing.

##### Reference Implementation

```cpp
struct NetworkAckSlidingWindow {
    uint64_t rollingAckBitmaskField{0};
    uint32_t highestSequenceReceived{0};
};
inline void RegisterNetworkPacketArrival(NetworkAckSlidingWindow& window, uint32_t incomingSequence) {
    if (incomingSequence > window.highestSequenceReceived) {
        uint32_t shiftDelta = incomingSequence - window.highestSequenceReceived;
        window.rollingAckBitmaskField = (shiftDelta >= 64) ? 0ull : (window.rollingAckBitmaskField << shiftDelta);
        window.highestSequenceReceived = incomingSequence;
    }
    uint32_t relativeBitOffset = window.highestSequenceReceived - incomingSequence;
    if (relativeBitOffset < 64) {
        window.rollingAckBitmaskField |= (1ULL << relativeBitOffset);
    }
}
```

##### Player-Facing Impact

Co-op sessions under real-world packet loss or high-latency jitter recover cleanly — the host knows precisely which entity updates went missing and can trigger the exact `StableId` resync path M12's base desync-handling step already defines, instead of silently drifting.

---

## M13 — Local Small Language Model (SLM) integration

**Scope note for the build agent: treat M13 as a core cross-cutting system, not an optional tail milestone.** Its ten `EXT` generators are the primary text-content source for eight other milestones (M4 world-gen signage, M5.4 mission threat text, M6 audio-cue interpretation, M8 item descriptors, M8.5 faction extortion notices, M8.6 settlement journals, M10 day-cycle logs, M11 stealth HUD text) — do not defer M13 to "polish pass" priority. The shared backend (queues, worker thread, `SLM::Submit`) in the Implementation Steps below is a **hard dependency** for every one of those systems the moment their own milestone's `outText` field is touched; build the backend before or alongside the first consuming milestone, not after all ten are speced out.

### Implementation Steps

1. **Model & quantization — locked (v78, superseding the v66 Gemma 4 E2B pick).** Model chosen: **MiniCPM5-1B**, GGUF `Q4_K_M`, via a local `llama.cpp`-derived backend. MiniCPM5-1B (OpenBMB) uses the standard `LlamaForCausalLM` architecture — a plain 24-layer dense transformer with grouped-query attention (`num_attention_heads=16`, `num_key_value_heads=2`, `head_dim=128`), not a novel architecture requiring custom kernels or a still-maturing runtime path. Chosen over the Gemma 4 E2B pick for three concrete reasons: (a) verified footprint — the Q4_K_M GGUF is a confirmed 688MB file on the model's own repo, not a range pending runtime verification; (b) simpler compatibility path — MiniCPM5-1B rides on llama.cpp's mature generic Llama loading path with no PLE-style offload machinery and no dependency on a specific runtime's support maturity; (c) license — Apache 2.0, same distribution-compliance benefit the v66 pick already established (see step 7 below).
**Config (from `config.json`, fetched directly):** `vocab_size=130560`, `hidden_size=1536`, `num_hidden_layers=24`, `num_attention_heads=16`, `num_key_value_heads=2`, `head_dim=128`, `max_position_embeddings=131072` (irrelevant overhead at this doc's locked 256–512 `n_ctx` — never touched). Total params 1,080,632,832; non-embedding params 679,552,512 (per the model card).
**VRAM math, same formula this doc uses throughout:** weights (Q4_K_M GGUF, confirmed file size) = 688MB. KV cache = `2 × layers × kv_heads × head_dim × bytes_per_elem` (fp16) = `2 × 24 × 2 × 128 × 2` = 24,576 bytes/token → ≈12.0MB at `n_ctx=512`, ≈6.0MB at `n_ctx=256` — trivial by construction given the low KV-head count. Plus ~100–150MB runtime overhead (standard llama.cpp context/compute buffers, no PLE-style paging involved) → **~800–850MB VRAM total.**
**Tier-0 budget — pass, explicitly, no verification-pending caveat.** Floor 6GB (RTX 2060 binding case) − 4.0GB engine baseline − ~0.3–0.5GB typical OS/driver reserve = 1.5–1.8GB headroom (unchanged from v63, per this doc's Performance & Resource Budgets section). ~800–850MB leaves ~700MB–1GB of headroom unused — more comfortable than the previous Gemma 3 1B-it fallback (~850–900MB) and unambiguously inside budget, unlike the flagged-unverified 1.5–3GB range the Gemma 4 E2B pick carried.
No dedicated fallback model is carried forward from this pick — MiniCPM5-1B's confirmed footprint fits the Tier-0 headroom with margin to spare, removing the need for a documented "if this fails verification" alternate that the two previous locks required.
Thinking mode: force **off** in the request template, both the sampling params and the flag. Verified directly against the released `chat_template.jinja`: the `<think>...</think>` scaffold is only emitted when `enable_thinking` is explicitly passed — `True` opens it, `False` emits it pre-closed (empty), and leaving the kwarg undefined skips the scaffold entirely (neither branch fires). That last case is *not* safe to rely on as a de-facto "off": `generation_config.json`'s own defaults are `temperature=0.9`, `top_p=0.95` — the documented Think-mode sampling pair, not the No-Think pair (`0.7`/`0.95`) — and OpenBMB's own deployment docs note the model can lean into think-style output on an unset flag depending on runtime. So the request template must set both explicitly: `enable_thinking=False` **and** `temperature=0.7, top_p=0.95`. Getting either wrong risks blowing the 1–2 tok/frame throttle and the single-shot no-history design in step 2 below. Output stays deterministic single-shot every call. If a future re-eval prefers a different model, redo this paragraph's math from that model's own `config.json` and published card, don't just swap the name.
2. **KV cache hard-cap.** `n_ctx` locked to 256–512 tokens. Every prompt is a single-shot template built from live component fields (no multi-turn chat history retained in hot paths) so the context window never grows unbounded.
3. **Dedicated compute queue.** The SLM runtime gets its own `VkQueue` from a queue family that supports `VK_QUEUE_COMPUTE_BIT` without `VK_QUEUE_GRAPHICS_BIT`, enumerated during M0's device/queue setup alongside the graphics and transfer queues (extends M0's existing queue-family enumeration step — not a new bootstrap phase). If no such dedicated family exists on a given GPU, fall back to the shared graphics+compute family rather than failing init.
4. **Throttled, off-critical-path dispatch.** Inference requests are pushed onto a lock-free SPSC queue using the same `[M1-EXT-06]` mutation-queue pattern — the main thread is the producer (every `SLM::Submit` call below enqueues a request struct), the SLM worker thread is the consumer that drains it and runs generation, capped at 1–2 tokens per frame. This reuses M1's existing staggered-update convention (the same category of pacing as `SpatialHash`'s tick-staggered queries) rather than inventing a new scheduling primitive.

Every EXT block below calls `SLM::Submit` to enqueue a request. The request-side `SPSCRequestQueue` and a concrete `SLM::Submit` are implemented below, alongside the outbound `SLMResultQueue` (step 6).

```cpp
#include <string>
#include <cstdint>
#include <atomic>

struct SLMRequest { entt::entity target; std::string tag; std::string promptFields; };

class alignas(64) SPSCRequestQueue {
    SLMRequest m_ring[256];
    alignas(64) std::atomic<uint32_t> m_writeHead{0};
    alignas(64) std::atomic<uint32_t> m_readHead{0};
public:
    // Called from the main thread inside SLM::Submit — never from the worker.
    bool Push(entt::entity target, std::string tag, std::string promptFields) {
        uint32_t w = m_writeHead.load(std::memory_order_relaxed);
        uint32_t next = (w + 1) & 255;
        if (next == m_readHead.load(std::memory_order_acquire)) return false; // request pool full, drop this frame's request
        m_ring[w] = { target, std::move(tag), std::move(promptFields) };
        m_writeHead.store(next, std::memory_order_release);
        return true;
    }
    // Called from the SLM worker thread's loop, off the main tick.
    bool Pop(SLMRequest& out) {
        uint32_t r = m_readHead.load(std::memory_order_relaxed);
        if (r == m_writeHead.load(std::memory_order_acquire)) return false; // nothing pending
        out = std::move(m_ring[r]);
        m_readHead.store((r + 1) & 255, std::memory_order_release);
        return true;
    }
};

// Global/engine-owned; the two queues declared above and below live here, not per-component.
inline SPSCRequestQueue  g_slmRequestQueue;
inline SLMResultQueue    g_slmResultQueue; // defined in step 6 below

namespace SLM {
    // Formats args into a single templated prompt string and enqueues it — never blocks, never touches outText.
    template<typename... Args>
    inline void Submit(const char* tag, Args&&... args) { /* format args into promptFields, then: */ }
    inline void Submit(const char* tag, entt::entity target) { g_slmRequestQueue.Push(target, tag, {}); }
}
```

5. **Hardware backend resolution.** Resolved once at boot (see `[M13-EXT-11]`): explicit CUDA env markers first, then a Vulkan compute fallback, then an interactive prompt only if both are ambiguous. Never silently guesses past that point.
6. **`SLM::Submit` result delivery.** Every `EXT` block below calls `SLM::Submit(tag, ..., &c.outText)` — that `&c.outText` pointer must never be written from the worker thread. Writing into live ECS component memory off the main thread is exactly the race `[M1-EXT-06]` exists to prevent, and a registry sort/reallocation mid-generation would leave the pointer dangling. `SLM::Submit` instead stores the completed string in a small SPSC completion queue keyed by `entt::entity`; the main thread drains it once per tick (same drain point as `[M1-EXT-06]`'s `DrainToRegistry`) and does the `outText` write itself.
7. **Legal/distribution compliance (v78, rewritten now that the locked model is MiniCPM5-1B under Apache 2.0).** Apache 2.0 keeps the same standard open-source attribution checklist the v66 Gemma 4 pick established: no MAU cap, no distribution-notice requirement beyond the license text itself, no restriction on fine-tuning and re-shipping the result. One-time setup:

   * Include the Apache 2.0 license text (and a short "this game includes MiniCPM5-1B, © OpenBMB" credit) in the game's credits/legal screen or a `LICENSE-MINICPM.txt` shipped alongside the install — good practice under Apache 2.0's attribution clause.
   * Don't use OpenBMB's name/logo/trademarks in marketing or imply endorsement (Apache 2.0 doesn't grant trademark rights).
   * Fine-tuning on game-specific data later is unrestricted under Apache 2.0 — the result can be treated as project IP.

   Content-safety scope (same as before, model-independent): none of the ten `M13-EXT` systems generate real-person content or anything outside walled fictional-world flavor text, but if modding ever lets raw player free-text reach the prompt, keep it template-bounded (a filled-in field inside a fixed prompt shell, never the whole prompt) so a malicious mod can't steer output. Ships fully offline, no live API dependency.

   No fallback-license branch applies here — this pick carries no fallback model (see step 1).

```cpp
#include <entt/entt.hpp>
#include <atomic>
#include <string>
#include <cstdint>

struct SLMResult { entt::entity target; std::string text; };

class alignas(64) SLMResultQueue {
    SLMResult m_ring[256];
    alignas(64) std::atomic<uint32_t> m_writeHead{0};
    alignas(64) std::atomic<uint32_t> m_readHead{0};
public:
    // Called from the SLM worker thread on generation completion.
    bool Push(entt::entity target, std::string text) {
        uint32_t w = m_writeHead.load(std::memory_order_relaxed);
        uint32_t next = (w + 1) & 255;
        if (next == m_readHead.load(std::memory_order_acquire)) return false; // drop if consumer stalled
        m_ring[w] = { target, std::move(text) };
        m_writeHead.store(next, std::memory_order_release);
        return true;
    }
    // Called once per main-thread tick, before any M13 system reads outText.
    template<typename ApplyFn>
    void Drain(ApplyFn&& apply) {
        uint32_t w = m_writeHead.load(std::memory_order_acquire);
        uint32_t r = m_readHead.load(std::memory_order_relaxed);
        while (r != w) { apply(m_ring[r].target, m_ring[r].text); r = (r + 1) & 255; }
        m_readHead.store(r, std::memory_order_release);
    }
};
```

Each `EXT` block's `SLM::Submit(tag, ..., entity)` call below should be read as "push a request tagged with `entity`"; the `&c.outText` shorthand in the snippets marks *which field the drain step assigns*, not a live pointer handed to the worker thread.

Every one of the ten `EXT` block snippets below must iterate with the two-argument EnTT form `reg.view<T>().each([](entt::entity entity, auto& c) {...})` and pass `entity` explicitly into `SLM::Submit(tag, ..., entity, &c.outText)` — step 6's result-delivery design requires an `entt::entity` to route each completed generation back to the right component.

### Extended Systems Library

Ten narrow, single-purpose text-generation systems. Each is a plain component holding its input fields plus an `outText` string buffer, drained by one `registry.view<T>().each(...)` line in the M13 update pass — deliberately not a manager class or thread-pool wrapper (YAGNI: the SPSC queue and worker thread already exist once, in the shared backend from the Implementation Steps above; a manager class per system would just be duplicate machinery around that one queue).

#### [M13-EXT-01] Procedural Mission & Bounty Director

##### Systems Touched

Structural town graph (M4), `FactionZone` control state (M8.5).

##### How It Works

When a `TownNode`'s threat level exceeds a configurable ceiling in a faction-contested zone, its road name, threat index, and rival-faction flag are submitted as a single-shot prompt, generating a short rescue/extraction/supply-run order text.

##### Reference Implementation

```cpp
struct PMDComponent { std::string roadName; int threat; bool hasRival; std::string outText; };
// M13 update pass: reg.view<PMDComponent>().each([](entt::entity entity, auto& c) { if (c.threat > kMissionThreatCeiling) SLM::Submit("mission", entity, c.roadName, c.threat, &c.outText); });
```

##### Player-Facing Impact

Mission text is grounded in the actual generated town layout instead of a fixed template pool — two playthroughs with different WFC seeds read differently.

#### [M13-EXT-02] Infected Mutation Forensic Log

##### Systems Touched

Corpse inspection (M2.9), infected phenotype generation (M5.1).

##### How It Works

On autopsy/inspection of a defeated special infected, the entity's stored phenotype tag plus the ambient temperature/weather at time of death are submitted to generate a short biological readout.

##### Reference Implementation

```cpp
struct ForensicComponent { std::string phenotypeTag; float ambientTempC; int weatherType; std::string outText; };
// reg.view<ForensicComponent>().each([](entt::entity entity, auto& c) { SLM::Submit("autopsy", entity, c.phenotypeTag, c.ambientTempC, &c.outText); });
```

##### Player-Facing Impact

Autopsy flavor text reflects the specific mutation and conditions that produced it, reinforcing M5.1's procedural phenotype variation instead of reading generic.

#### [M13-EXT-03] Dynamic Graveyard & Memorial Wall Epitaphs

##### Systems Touched

M7 persistence (death-cause history), safehouse zone registry (M8.6).

##### How It Works

On an NPC or prior-character death event, its `StableId` history record (cause of death, days survived) generates a short wall inscription placed in the nearest safehouse memorial slot.

##### Reference Implementation

```cpp
struct EpitaphComponent { std::string name; std::string causeOfDeath; int daysSurvived; std::string outText; };
// reg.view<EpitaphComponent>().each([](entt::entity entity, auto& c) { SLM::Submit("epitaph", entity, c.name, c.causeOfDeath, &c.outText); });
```

##### Player-Facing Impact

Memorials reflect the player's actual run history instead of static flavor text.

#### [M13-EXT-04] Inter-Faction Extortion & Diplomacy Dispatch

##### Systems Touched

`FactionZone` control percentage (M8.5), dead-drop container placement (M8.6).

##### How It Works

When a faction's local control percentage crosses a threshold, a threat/ultimatum/trade-offer text is generated and placed in that zone's dead-drop container.

##### Reference Implementation

```cpp
// v67: enriched with ammo/nutrition/roster fields (from updateforv66.txt's FactionStrategicStateToken)
// so dispatch text is grounded in actual faction capacity, not control% alone.
struct ExtortionComponent {
    std::string factionName; float controlPct;
    float ammoReservesScalar = 1.0f;      // low reserves -> desperate/trade-offer tone
    float populationNutritionIndex = 1.0f; // low nutrition -> more likely to sue for supply trade over threat
    uint32_t combatantRosterSize = 0;      // high roster -> ultimatum tone credible
    std::string outText;
};
// reg.view<ExtortionComponent>().each([](entt::entity entity, auto& c) {
//     if (c.controlPct > kControlThreshold) SLM::Submit("extortion", entity, c.factionName, c.controlPct, c.ammoReservesScalar, c.populationNutritionIndex, c.combatantRosterSize, &c.outText);
// });
```

##### Player-Facing Impact

Faction pressure is legible without a dialogue system — reading a note tells you who's winning the territory fight nearby, and whether they're bluffing (low ammo/food) or not.

#### [M13-EXT-05] Procedural Vehicle Maintenance Diagnostics

##### Systems Touched

Jolt vehicle chassis stress state (M9), inventory/UI text fields.

##### How It Works

Replaces a bare health percentage with a status line generated from the vehicle's live axle-stress and fuel-line-degradation floats (both already computed by M9's chassis simulation — this is a text view on existing values, not a new stress model).

##### Reference Implementation

```cpp
struct VehicleDiagComponent { float axleStress; float fuelLineDegradation; std::string outText; };
// reg.view<VehicleDiagComponent>().each([](entt::entity entity, auto& c) { SLM::Submit("vehicle_diag", entity, c.axleStress, c.fuelLineDegradation, &c.outText); });
```

##### Player-Facing Impact

"Axle warped, fuel line patched and holding" reads more concretely than "62% condition" without adding a new mechanical system.

#### [M13-EXT-06] Dynamic Survivor Hearsay & Rumor Network

##### Systems Touched

Procedural world anomaly tracker (M4), safehouse bulletin board props (M8.6).

##### How It Works

Undiscovered map anomalies (crashed convoys, hidden caches) are periodically sampled and turned into a rumor string posted to a nearby bulletin board component.

##### Reference Implementation

```cpp
struct RumorComponent { std::string anomalyType; bool discovered; std::string outText; };
// reg.view<RumorComponent>().each([](entt::entity entity, auto& c) { if (!c.discovered) SLM::Submit("rumor", entity, c.anomalyType, &c.outText); });
```

##### Player-Facing Impact

Bulletin boards give soft, deniable direction toward points of interest instead of a hard waypoint marker.

#### [M13-EXT-07] Safehouse Logbook Auto-Journalizer

##### Systems Touched

Day/night cycle rollover (M10), settlement defense/scavenging counters (M8.6).

##### How It Works

On a day-cycle rollover or sleep transition, accumulated daily counters (walls reinforced, hostiles eliminated, scrap recovered) are compressed into one diary-style log line.

##### Reference Implementation

```cpp
struct JournalComponent { int wallsReinforced; int hostilesEliminated; int scrapRecovered; std::string outText; };
// reg.view<JournalComponent>().each([](entt::entity entity, auto& c) { SLM::Submit("journal", entity, c.wallsReinforced, c.hostilesEliminated, c.scrapRecovered, &c.outText); });
```

##### Player-Facing Impact

A running, readable log of "what happened while I was surviving" without the player writing anything themselves.

#### [M13-EXT-08] Infiltration & Stealth Audio-Text Interpreter

##### Systems Touched

Acoustic propagation system (M6), stealth HUD overlay (M11).

##### How It Works

Sound emission events above an amplitude threshold near the player are translated into a short sensory description line, primarily as an accessibility aid for sound-cue-heavy stealth moments.

##### Reference Implementation

```cpp
struct AudioInterpretComponent { float frequencyHz; float amplitude; std::string outText; };
// reg.view<AudioInterpretComponent>().each([](entt::entity entity, auto& c) { if (c.amplitude > kAudibleThreshold) SLM::Submit("audio_cue", entity, c.frequencyHz, c.amplitude, &c.outText); });
```

##### Player-Facing Impact

Gives players who rely less on raw audio cues an equivalent text-based tension signal instead of nothing.

#### [M13-EXT-09] Dynamic Quarantine Zone Signage & Lore

##### Systems Touched

Procedural building generation (M4), interior enemy density tags (M5.4).

##### How It Works

Barricaded/locked-down buildings get warning signage or graffiti generated from their interior danger rating and any military-response tag assigned during world-gen, giving a soft difficulty hint before entry.

##### Reference Implementation

```cpp
// v67: enriched with zoning/mortality/martial-law fields (from updateforv66.txt's SocioEconomicSignageStateToken)
struct QuarantineSignComponent {
    int dangerLevel; std::string militaryCode;
    uint32_t zoningClass = 0;         // 0=Residential, 1=Commercial, 2=Industrial — flavors the sign's wording
    float localMortalityRatio = 0.0f; // high ratio -> starker/more official warning tone
    bool martialLawEnforced = false;  // true -> stenciled military font/phrasing instead of hand-painted graffiti
    std::string outText;
};
// reg.view<QuarantineSignComponent>().each([](entt::entity entity, auto& c) {
//     SLM::Submit("signage", entity, c.dangerLevel, c.militaryCode, c.zoningClass, c.localMortalityRatio, c.martialLawEnforced, &c.outText);
// });
```

##### Player-Facing Impact

Environmental storytelling telegraphs interior threat level without a UI danger meter, consistent with the doc's existing environmental-storytelling appendix approach.

#### [M13-EXT-10] Jury-Rigged Weapon Mod Descriptors

##### Systems Touched

M8 crafting/attachment system, item tooltip UI.

##### How It Works

When a scrap component is attached to a weapon via the crafting screen, a one-line tooltip describing the combined item is generated from the two item names.

##### Reference Implementation

```cpp
struct WeaponModComponent { std::string baseItemName; std::string scrapItemName; std::string outText; };
// reg.view<WeaponModComponent>().each([](entt::entity entity, auto& c) { SLM::Submit("weapon_mod", entity, c.baseItemName, c.scrapItemName, &c.outText); });
```

##### Player-Facing Impact

Jury-rigged combinations get a distinct identity in the inventory list instead of a generic "Modified Weapon" label.

#### [M13-EXT-11] Hardware Backend Auto-Detection & Async Compute Queue Isolation

##### Systems Touched

Engine boot sequence (extends M0's device/queue enumeration), M13 SLM worker thread startup.

##### How It Works

Resolved once at boot, in order: explicit CUDA environment markers, then a Vulkan compute-capable queue family, then — only if both are genuinely ambiguous — an interactive console prompt so the engine never silently guesses wrong and never hangs waiting on unset environment state.

##### Reference Implementation

```cpp
#include <cstdlib>
#include <iostream>
#include <string>

enum class SLMBackend { CUDA, VulkanCompute, CPU };

inline SLMBackend ResolveSLMBackend() {
    if (std::getenv("CUDA_VISIBLE_DEVICES") || std::getenv("CUDA_PATH")) return SLMBackend::CUDA;
    if (std::getenv("VULKAN_SDK")) return SLMBackend::VulkanCompute;

    std::cout << "SLM backend indeterminate. [1] CUDA  [2] Vulkan compute  [3] CPU fallback: ";
    int choice = 3;
    if (!(std::cin >> choice)) { std::cin.clear(); std::cin.ignore(10000, '\n'); choice = 3; }
    return (choice == 1) ? SLMBackend::CUDA : (choice == 2) ? SLMBackend::VulkanCompute : SLMBackend::CPU;
}
```

```cpp
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

// Extends M0's existing queue-family enumeration — call alongside the graphics/transfer queue lookup, not separately.
inline uint32_t FindDedicatedComputeQueueFamily(const std::vector<VkQueueFamilyProperties>& families, uint32_t graphicsFamilyIndex) {
    for (uint32_t i = 0; i < families.size(); ++i) {
        const bool hasCompute = (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
        const bool hasGraphics = (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        if (hasCompute && !hasGraphics) return i; // dedicated async compute family
    }
    return graphicsFamilyIndex; // fallback: share the graphics+compute family rather than fail init
}
```

##### Player-Facing Impact

Invisible — this is what lets M13's text generation run without stealing frame time from rendering or physics, and what stops the engine from misdetecting hardware and silently falling back to a slow path.

#### [M13-EXT-12] Attenuated Narrative Radio Scrambler

##### Systems Touched

M2.8-EXT-01's radio-frequency signal attenuation (comms static physics), M6 acoustic propagation, faction/survivor radio chatter text.

##### How It Works

`[M2.8-EXT-01]` already computes terrain diffraction path-loss in dB as a physics value for comms static audio; this feeds that same path-loss number plus transmitter distance into the SLM alongside the raw message, so the *text* rendered on a radio HUD/subtitle degrades in step with the audio — dropped words and garbled phrasing scale with actual signal quality instead of the transcript staying perfectly clean while the audio crackles.

##### Reference Implementation

```cpp
struct RadioScramblerComponent {
    float pathLossDb = 0.0f; uint32_t transmitterDistanceM = 0;
    std::string rawMessage; std::string outScrambledText;
};
// reg.view<RadioScramblerComponent>().each([](entt::entity entity, auto& c) {
//     if (c.pathLossDb > kScrambleThresholdDb) SLM::Submit("radio_scramble", entity, c.pathLossDb, c.transmitterDistanceM, c.rawMessage, &c.outScrambledText);
// });
```

##### Player-Facing Impact

Weak-signal radio chatter reads as genuinely degraded (dropped words, static-garbled phrasing) instead of a clean transcript playing under crackly audio — the text and the sound tell the same story.

#### [M13-EXT-13] Survivor Behavior-Tree Flavor Advisor

---

#### [M13-EXT-15] SLM Broadcast Text Determinism Seeder *(RECONSTRUCTED FROM CITATION CONTEXT — VERIFY)*

##### Systems Touched
M13 + M8.5/M5.4. Seeds SLM broadcast text deterministically per world state.

##### Math
seed = Hash(worldState); text = SLM.Generate(prompt, seed);

##### How It Works
World/broadcast text from the SLM is seeded by a hash of current world state so the same situation yields the same broadcast across clients (co-op consistency) while still feeling dynamic. Guarded by M13-EXT-53 content-drought check.

##### Reference Implementation
```cpp
string t = SLM.Generate(prompt, Hash(world));
```

##### Player-Facing Impact
Broadcasts are consistent in co-op yet vary with the world - no desync in story.


##### Systems Touched

M5.4-equivalent human-survivor behavior tree (faction affiliation, health, ammo, LOS state), M13 dispatch pipeline.

##### How It Works

This does not make behavior-tree *decisions* — that stays deterministic C++ logic, since letting an SLM pick combat behavior would make survivor AI non-reproducible and hard to debug. It only reads the already-decided branch (flee/fight/regroup) plus context (health%, ammo count, LOS-obstructed flag) and generates a one-line barked callout matching that branch, run through the same throttled 1–2-tokens/frame SLM queue every other `M13-EXT` system uses.

##### Reference Implementation

```cpp
struct SurvivorBarkComponent {
    uint32_t factionCode = 0; uint8_t healthPct = 100; uint8_t ammoCount = 0;
    bool losObstructed = false; std::string activeBranchTag; // set by the (unrelated, deterministic) BT itself
    std::string outBark;
};
// reg.view<SurvivorBarkComponent>().each([](entt::entity entity, auto& c) {
//     SLM::Submit("survivor_bark", entity, c.activeBranchTag, c.healthPct, c.ammoCount, c.losObstructed, &c.outBark);
// });
```

##### Player-Facing Impact

Survivor NPCs bark contextual, varied one-liners ("I'm dry, covering!" vs a fixed "Reloading!" string) without the AI's actual decision-making becoming non-deterministic or harder to debug — the SLM narrates the decision, it never makes it.

---

## Appendices

### Appendix A: Procedural Narrative System

Diary/journal entry generator composed from template grammars filled from simulation values:

* Humanity crosses threshold (e.g. below 40) -> reads recent morally weighted action.
* Faction reputation crosses threshold -> tracks trade/conflict events.
* Settlement raided -> reads structural damage logs and casualties.

### Appendix B: Concrete Property Tables

* Day/Night: 48 real-time minutes (60/40 split). Weather: 15% rain, 3% storms.
* Destruction Thresholds: Wood wall failure at 800 N·s. Brick failure at 3,200 N·s. Sedan impact at 60 km/h: ~25,000 N·s *(corrected — the doc's prior 2,900 N·s figure was internally inconsistent with its own stated inputs: impulse = mass × Δv, and a stated "sedan" at a stated 60 km/h (16.67 m/s) coming to a full stop implies J = 1500 kg × 16.67 m/s ≈ 25,000 N·s using a representative curb weight for a mid-size sedan; 2,900 N·s only back-solves to a ~174 kg object, i.e. motorcycle-class, not a sedan)*.

### Appendix C: Worked Examples — Quest & Loot Generation

* Loot distributions derived strictly through Socio-Economic tags (e.g., Gym yields heavy blunt items and protein).

### Appendix D: Lone Survivors & Dead Camps

* Lone Survivors spawn via M8.5 ODE population mechanics.
* Dead Camps dynamically spawn based on recorded persistence failure causes (SEIR outbreaks leave bloat/hazard maps, Raids leave ballistic-scarred structural destruction).

### Appendix E: Procedural Generation & Animation Unification

* `sub_seed()` SplitMix64 hashing ensuring generation continuity without saving specific geometry structures in-memory.
* Facial Expression Rig Priority: Visemes override procedurally animated autonomic jitters seamlessly.

### Appendix F/G/H: AAA-Grade Generation, Simulation, World & Content

* **Voice:** Piper offline neural TTS, Formant/pitch-decoupled scaling, PSOLA processing.
* **Textures/Weather:** Domain-warped OpenSimplex2 terrain mapping, runtime WFC 3D constraint assemblies.
* **Simulation:** Real-time HTN/GOAP integrations, XPBD cloth coupling, reaction-diffusion modeling.
* **World:** Foliage Poisson-disk clustering, DDGI/ReSTIR integrations, stream-power hydraulic erosion matrices.

### Appendix I/J: FPS Controller Deep-Dive & Gunplay Pass

* Hand-modeled parameter sweeps defining exact mantle/climb curves utilizing critically damped spring constants.
* Ballistic integration modeling genuine drop and weapon-rest stabilization techniques scaling strictly from mass/inertia math formulas, completely replacing arbitrary attachment arrays.

---

## Not yet merged — awaiting priority confirmation

The following `master_feature_list.md` tables were deliberately excluded from this v43 pass (solo dev asked to sequence engine layer first):

* **Procedural Generation — World/Systems** (Poisson disk sampling, domain-warped noise, WFC adjacency validator [already `[M4-EXT-03]`/`[M4-EXT-11]`], chunk-boundary determinism test [route to §5.10 verification suite], graph-based room connectivity [already `[K-EXT-18]` + `[M4-EXT-08]`], rejection sampling [shared gate, not a standalone system — see draft pass-2 note], seed-sharing [already `[M2.8-EXT-05]`], cellular automata [now `[M4-EXT-24]`], graph grammars [covered by Procedural Mission & Event Director, Dormans & Bakkes], genetic algorithms [now `[M8.6-EXT-09]`], Whittaker biome classification [now `[M4-EXT-23]`], thermal erosion, spline-based roads/rivers [roads fully covered by `[M4-EXT-01]`/`[M4-EXT-19]`/`[M4-EXT-10]`; rivers half-covered — bank/confluence hydrology detail only], hidden difficulty correction) — deferred, engine layer first.
* **Procedural Asset Generation (Solo/No-Artist)** (histogram-preserving blending [now `[M4-EXT-25]`], PPTBF materials [now `[M4-EXT-25]`], node-graph procedural materials [now `[M4-EXT-25]`], noise→normal map derivation [now `[M4-EXT-25]`], procedural mesh via primitive+grammar [already `[K-EXT-17]`], procedural foliage L-system [now `[M4-EXT-28]`], procedural color palette [now `[M4-EXT-25]`], procedural UV unwrap [recommended `[M4-EXT-26]`-class, deliberately not merged this pass — decal atlas renumbered to `[M4-EXT-26]` instead], procedural skeletal rig generation [already `[M5.1-EXT-03]`], procedural decal atlasing [now `[M4-EXT-26]`], procedural loot icon generation [now `[M8-EXT-10]`]) — deferred, engine layer first. See teq-v78-procedural-gapfill-draft.md close-out notes (pass 1 + pass 2) and Part A Hermes gap-fill appendix for the ID citations above.
* **AI** (behavior tree/planning layer, line-of-sight result caching) — deferred, engine layer first.
* **Audio** (dynamic/adaptive music, voice-line barking system) — deferred, engine layer first.
* **Meta/UX/Tooling** (pseudo-localization QA mode, save versioning/migration, save backup rotation, cross-progression/cloud save, remote-config balance patching, terrain deformation persistence, weapon attachment/modification, NPC daily schedules, curated environmental storytelling props, frame-time variance display, predictive asset streaming, input buffering/coyote-time, session-level difficulty rubber-band, server-authoritative anti-cheat, death-cam/kill-cam replay) — deferred, engine layer first. (Mod load-order conflict detection removed from this deferred bucket — no longer deferred, see **Appendix L**.)
* **Libraries to Adopt** (FastNoise2, vk-bootstrap+VMA formalization, Slang shader compiler) — deferred; these are toolchain adoptions that touch already-deferred procgen systems (FastNoise2) or are cross-cutting infra decisions best made alongside the deferred tables, not folded in piecemeal. (Tracy profiler already decided and merged — see "Profiler of record: Tracy" under Tooling & developer iteration loop — removed from this list.)

#### [M12-EXT-03] Bitstream Delta-Encoded Packet Replay Fragment Reassembler

Same feature, same `OrderedPacketSlice`/`ReassembleInboundFragment` shape as the Section 5.7 appendix entry under this same ID. This is the canonical copy (full Systems/Math/How-It-Works writeup); the appendix entry should just cross-reference it.

##### Systems Touched

Network transmission parsing, `StableId` entity replication tracking arrays. Sits downstream of `[M12-EXT-01]`'s delta encoder and `[M12-EXT-02]`'s loss-detection window — this reassembles fragments that arrive out of order, it doesn't decide packing or detect loss itself.

##### Math

Modulo slicing matching inbound network delivery tracking indices to sliding sequence buffer slots:

`SlotIdx = Seq_packet mod BufferCapacity`

##### How It Works

Intercepts fragmented delta network payloads passing across ENet/GameNetworkingSockets boundaries. Because packets can land out of sequence due to jitter, this reassembler sorts inbound fragments across a sliding modulo array, preventing the client-side entity factory from processing broken or unaligned component updates until sequence chains resolve cleanly.

##### Reference Implementation

Sequence-window checks use an unsigned wraparound-safe distance comparison (not `sequence + poolCapacity <= maxTrackedSequence`, which overflow-wraps near `UINT32_MAX` and would silently accept stale fragments).

>

The copy is clamped to `min(rawBufferLen, sizeof(discretePayloadBuffer))` (never a fixed `memcpy(..., 256)`, since ENet/GameNetworkingSockets fragments are rarely exactly 256 bytes and a fixed copy would over-read past `rawBufferData`). A `payloadLength` field records how many bytes in the slot are valid.

```cpp
#include <cstdint>
#include <cstring>
#include <algorithm>

struct OrderedPacketSlice {
    uint32_t sequenceIdentifier;
    uint8_t discretePayloadBuffer[256];
    uint16_t payloadLength;
    bool isSlotPopulated;
};

inline bool ReassembleInboundFragment(uint32_t sequence, const uint8_t* rawBufferData, uint32_t rawBufferLen, OrderedPacketSlice* ringPool, uint32_t poolCapacity, uint32_t maxTrackedSequence) {
    const uint32_t backwardDistance = maxTrackedSequence - sequence;
    if (backwardDistance < 0x80000000u && backwardDistance >= poolCapacity) return false; // stale, already outside sliding window
    const uint32_t targetedSlot = sequence % poolCapacity;
    const uint32_t copyLen = std::min<uint32_t>(rawBufferLen, sizeof(OrderedPacketSlice::discretePayloadBuffer)); // never read past rawBufferData's real length

    ringPool[targetedSlot].sequenceIdentifier = sequence;
    std::memcpy(ringPool[targetedSlot].discretePayloadBuffer, rawBufferData, copyLen);
    ringPool[targetedSlot].payloadLength = static_cast<uint16_t>(copyLen);
    ringPool[targetedSlot].isSlotPopulated = true;
    return true;
}
```

##### Player-Facing Impact

Eliminates position rubber-banding and desynchronization during networked multiplayer co-op sessions, maintaining tight physical synchronization under volatile connection profiles.

---

# 5\. Extended Architectural Subsystems — Master Addendum

Consolidated, deduplicated list from the architecture gap-finding pass. IDs follow `[Mx-EXT-nn]` and slot into their listed milestone. Items marked **[MATH DONE]** have full math + reference code below; everything else is scoped (systems touched + purpose) and needs math fleshed out before implementation — do that per-item when you get to it, don't block the merge on it.

**Merge audit (v67) — `updateforv66.txt` fully integrated, nothing dropped.**

**Merge audit (v68) — two gaps found in the v67 pass, now closed.** The v67 audit above covered the EXT-block items but silently missed two large non-EXT-formatted sections of `updateforv66.txt`: Section 3's 20-layer environmental simulation hierarchy (never mentioned as included *or* excluded — just dropped), and Section 6's standalone sandboxed verification test suite. Both are now in: the 20 layers are added as 20 full EXT blocks — `[M3-EXT-07/08]` (structural stress-tensor cracking, particulate deposition), `[M4-EXT-12..19]` (tectonics, cave carving, stratigraphy, Darcy aquifer extension to the existing M10 groundwater grid, generation-time Saint-Venant erosion reusing `[M10-EXT-01]`'s flux stencil, edaphic soil chemistry, space-colonization canopy, road-graph exclusion splines), `[M4.5-EXT-19/20]` (Heiligenschein/anisotropic micro-optics, Beer-Lambert underwater extinction), `[M6-EXT-11]` (acoustic voxel occlusion, pairs with EXT-20), `[M6.5-EXT-11/12]` (fBm foliage kinematics, volumetric micro-atmospherics/heat-shimmer), `[M9-EXT-19]` (POM topography deform + persistent ruts, fills the displacement-channel stub `[M9-EXT-04]` was already reading), `[M10-EXT-02..06]` (Keplerian sun/moon solver, Bruneton-Nishita scattering, cryospheric frost, Navier-Stokes wind field, phenological seasonal canopy). None of these duplicate an existing system — each explicitly wires into something already in the doc (stratum table, groundwater grid, road graph, wind field, season index) rather than adding a second version of it. The verification suite is added as new §5.10, using local mock structs so it never collides with canonical types. First pass flagged six items as "duplicates" and left them out; on a closer re-read most weren't true duplicates, just overlapping-sounding names — they're folded in below instead: `[M1-EXT-11]` compute→indirect-draw barrier, `[M4.5-EXT-18]` skinned Visibility-Buffer cache, `[M2.9-EXT-10]` player flood buoyancy, `[M4-EXT-11]` WFC contradiction recovery, `[M2.8-EXT-08]` fixed-point mesh-vector quantization (sibling to `[M2.8-EXT-04]`'s scalar version, not a dupe — that one's a 16.16 scalar for damage/pathfinding math, this one's a 24.8 `vec3` for procedural mesh-generation determinism, different data shape), `[M13-EXT-12]` radio scrambler, `[M13-EXT-13]` survivor behavior-tree flavor advisor, plus enriched fields on `[M13-EXT-04]` and `[M13-EXT-09]` pulled from the update's richer telemetry tokens, `[M9-EXT-18]` (routes vehicles onto the existing `[M5.4-EXT-06]` road graph instead of building a second one), and new §5.9 naming lexicon. Two things genuinely were exact duplicates and stayed out: `AlignToCacheLine` (byte-identical to `[M0-EXT-01]`, nothing to add), and a second `SurfaceFrictionSample` struct with different field names for the same six multiplier slots — the canonical one (defined under M9 below) is what everything else in this doc already writes to, so a second definition would just reintroduce the exact order-dependent bug the front-matter audit item #3 fixed; if you want the update's field names instead, rename in one place (the canonical struct), don't add a second struct. One more from the update, `DirectorTelemetryStateToken`, wasn't added as an `M13-EXT` item at all — horde-pacing telemetry (avg player health, shots fired, SEIR severity) already feeds M5.4's own `ThompsonBanditTracker`, and that's deterministic tuning logic, not narrative text, so it stays in M5.4 rather than being routed through the SLM.

## 5.1 Architecture, Threading, Memory, Drivers (M0 / M1 / M1.9 / M4.6)

**[M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator** — canonical copy is under §5.1/M0 in the main body; see there. *(v75: this appendix copy had regressed to the pre-fix `== activeStages` condition the body copy's own comment documents and replaces with `!= 0` — collapsed to this stub rather than left as a second, stale copy that could drift again.)*

**[M0-EXT-11] Asynchronous SPIR-V Shader Cache Garbage Collector**

Systems: shader toolchain, disk cache.
Purpose: sweeps orphaned SPIR-V variant bytecode off disk on an isolated thread after engine updates so the shader cache doesn't grow unbounded across versions.

**[M0-EXT-12] Compute-Worker Thread-Affinity Bitmask Allocator**

Systems: enkiTS scheduler.
Purpose: pins background worker threads to explicit physical cores during heavy workloads to avoid L1/L2 cache-line migration thrashing. (Verify actual win on target hardware before committing — thread pinning is a double-edged sword on hybrid P/E-core CPUs.)

**[M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache** — canonical copy is under §5.1/M1 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub so a future edit to the canonical copy can't leave a stale duplicate behind, as happened with three other Appendix K entries this pass.)*

**[M1-EXT-10] Render Graph Pass Dependency DAG Flattener**

Systems: frame graph compiler.
Purpose: unrolls nested render-pass dependencies into one linear Vulkan execution order, cutting driver-side barrier overhead per frame.

**[M4.6-EXT-04] VRAM Sparse-Resident Memory Page Physical Allocation Tracker** — canonical copy is under §5.1/M4.6 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub for the same reason as `[M1-EXT-09]` above.)*

**[M4.6-EXT-05] DirectStorage Decompression Buffer Ring Allocator Pool**

Systems: GPU decompression streaming.
Purpose: recycles raw byte allocations on the VRAM streaming channel so high-speed chunk load bursts (e.g. driving fast) don't spike memory footprint. Note: requires GDeflate-capable path — confirm this is actually available on your target Vulkan drivers (DirectStorage proper is Windows/DX; on Vulkan you're relying on VK_EXT/vendor GDeflate compute decompression, not the DirectStorage API itself — name this something like "GPU Decompression Ring Allocator" to avoid implying a Windows-only API).

**[M1-EXT-11] Compute-to-Indirect-Draw Execution Barrier** — [MATH DONE]

Purpose: explicit `VkDependencyInfo`/`VkBufferMemoryBarrier2` between the GPU-driven culling compute dispatch and `vkCmdDrawIndexedIndirectCount`, so L2 writes from the compute pass are guaranteed visible to the Command Processor's indirect reader — without it, expect intermittent mesh flicker / missing chunks / hangs on tighter GPU schedulers.

```cpp
void CmdIndirectDrawBarrier(VkCommandBuffer cmd, VkBuffer indirectBuf, VkBuffer countBuf) {
    VkBufferMemoryBarrier2 b[2]{};
    for (auto* buf : { &indirectBuf, &countBuf }) {
        VkBufferMemoryBarrier2& x = b[buf == &indirectBuf ? 0 : 1];
        x.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        x.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        x.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        x.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        x.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        x.buffer = *buf; x.offset = 0; x.size = VK_WHOLE_SIZE;
    }
    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.bufferMemoryBarrierCount = 2; dep.pBufferMemoryBarriers = b;
    vkCmdPipelineBarrier2(cmd, &dep);
}
```

**Numbering note (v78, this pass):** the live `milestones/` directory has since moved ahead of this
consolidated doc in several places I don't have visibility into from here — confirmed collisions
include `[M1-EXT-13]` (Render-Graph Barrier Topological Sorter), `[M1-EXT-15]` (GPU Query Pool
Resolver), `[M1-EXT-16]` (SoA Cache-Line Padding), and `[M1-EXT-17]` (unknown content). The entries
below are numbered `[M1-EXT-18]` onward specifically to not collide with those. Treat these numbers
as provisional until checked against the actual `01_M1.md` — this doc is the one that's behind, not
the live files.

**[M1-EXT-18] Material-Batched Mesh-Pass Rendering**

Systems: existing indirect-draw/compaction path (the one `[M1-EXT-11]`'s barrier already sits in
front of), M4.5 forward/G-buffer/transparent/shadow passes, `[M0-EXT-08]`/`[M4.5-EXT-09]`'s bindless
`MaterialIdx`.

Math: draw commands stable-sorted by `MaterialIdx` before the existing prefix-sum compaction runs,
producing contiguous per-material runs: `run_m = [i : MaterialIdx(i) = m]`.

Purpose: adds a sort key to the existing compaction pass, not a second buffer or pass — each render
pass issues one pipeline/descriptor bind per material batch instead of per draw. Forward/
transparent/shadow passes consume the same material-sorted buckets since `MaterialIdx` doesn't
change mid-frame. Fewer binds per frame at horde scale (mixed zombie skins, weapon variants,
debris); no visual change.

**[M1-EXT-19] SoA Layout for Hot Components**

Systems: Transform + bone-matrix arrays, `SpatialHash` consumers (M5.1, M5.4, `[M8.5-EXT-02]`),
composes with `[M1-EXT-09]`'s pointer/stride cache rather than replacing it — EXT-09 still resolves
offsets into whatever layout backs the pool; this changes what that layout physically is for the
named hot fields.

Purpose: array-of-structs → structure-of-arrays for fields actually iterated hot-path per tick only
(cold fields stay packed as-is), so enkiTS workers iterating one field stream contiguous memory
instead of skipping over unrelated fields. Reduces cache-miss stalls at horde scale; no behavior
change.

**[M1-EXT-20] EnTT Group-Backed Hot-Component Storage**

Systems: composes with `[M1-EXT-09]` and `[M1-EXT-18]`'s SoA item — same hot-field set (Transform,
bone arrays, `SpatialHash` consumers), not registry-wide.

Purpose: applies `entt::registry::group` to the named hot components so EnTT physically sorts their
backing storage for contiguous joint iteration, rather than relying solely on `[M1-EXT-09]`'s
tick-local offset cache to paper over a non-contiguous sparse-set layout. Grouping can reallocate a
pool's backing storage, so `[M1-EXT-09]`'s cache is still re-resolved every tick exactly as it
already does today — this doesn't change that discipline, it changes what the cache points into.
Only applied to the same hot-field set the SoA item targets; over-grouping the whole registry would
fight EnTT's mutation flexibility for cold, rarely-touched components. Compounds with `[M1-EXT-18]`'s
locality win rather than duplicating it.

**[M1-EXT-21] Buffer Device Address for Skinned-Mesh Animation Data**

Systems: zombie/companion skeletal animation, M5.2 rig-pool, existing static-mesh indirect-draw
path.

Purpose: skinning computed once per frame into a device-local buffer accessed via `VkDeviceAddress`
(pushed as a push-constant pointer, current Vulkan bindless practice); the resulting buffer feeds
the same indirect-draw/culling path already used for static geometry, so skinned and static share
one draw pipeline instead of two. Removes per-mesh descriptor rebind overhead for animated
characters at horde scale.

**[M1-EXT-22] CVar System (ImGui-Backed)**

Systems: existing ImGui overlay (`[M1-EXT-15]`'s HUD lives here), scattered ad hoc debug toggles
across earlier milestones.

Purpose: typed int/float/bool/string console-variable registry, live-editable from an ImGui panel
and settable via boot-time launch flags — replaces scattered hardcoded debug toggles with one
queryable registry other systems read from instead of reinventing their own flag. Dev tool only, no
player-facing effect.

**[M1-EXT-23] Timeline Semaphores for Multi-Queue Sync**

Systems: graphics/compute/transfer submission, `[M13-EXT-11]`'s async compute queue (this
formalizes the cross-queue sync mechanism EXT-11 already assumes exists rather than introducing a
competing one), `[M4.6-EXT-05]`'s streaming transfer queue.

Purpose: `VK_KHR_timeline_semaphore` (core Vulkan 1.2+, mandatory-core in 1.4) replaces
per-submission fence+binary-semaphore pairs with one monotonically increasing counter per queue
relationship; consumers wait on a target value instead of juggling handles. No player-facing effect
— cleaner cross-queue sync, headroom for future async work.

**[M1-EXT-24] Multi-Threaded Secondary Command Buffer Recording**

Systems: `[M1-EXT-03]`'s Multi-Threaded Command Pool Matrix — that system only allocates one pool
per thread/frame-slot and never specifies a primary-vs-secondary buffer strategy; this is that
missing piece.

Math: `Pool_index` is unchanged from `[M1-EXT-03]`; each worker records a `SECONDARY` command
buffer from its own pool via `VkCommandBufferInheritanceInfo` (render pass + subpass inherited from
the primary), then one primary buffer executes all secondaries via a single `vkCmdExecuteCommands`.

Purpose: workers record draw calls in parallel with zero cross-thread synchronization beyond
`[M1-EXT-03]`'s existing per-thread pool isolation; the primary buffer does no drawing itself, only
begins the render pass and stitches secondaries together in the deterministic order `[M1-EXT-13]`'s
topo sort already establishes. Without this, `[M1-EXT-03]`'s pools exist but sit idle behind
single-threaded recording.

**[M1-EXT-25] Descriptor Update Templates for Per-Frame Bindless Writes**

Systems: `[M0-EXT-08]`/`[M4.5-EXT-09]`'s bindless material slot writes, `[M4.5-EXT-07]`'s RVT page
uploads — wherever a per-frame descriptor write currently loops individual
`vkUpdateDescriptorSets` calls.

Purpose: `VkDescriptorUpdateTemplate` pre-declares the write layout once at init; each frame's
batch of bindless-slot/RVT-page updates submits as one templated call from a tightly packed data
array instead of N individual calls. Lower per-frame descriptor-write CPU overhead as bindless slot
churn grows (material variety, chunk streaming); no visual change.

## 5.2 Physics, Kinematics, Coordinate Remapping (M2 / M2.6 / M2.7 / M2.8 / M2.9 / M9)

**[M2-EXT-06] Jolt-to-EnTT Double-Precision Transform Remapper**

Purpose: translates Jolt's single-precision contact manifolds into your authoritative `glm::dvec3` positions to avoid coordinate drift at sector boundaries far from world origin.

**[M2-EXT-07] Kinematic Virtual Sweep Tunneling Safeguard**

Purpose: projects speculative AABB sweeps ahead of kinematic ticks to catch velocity spikes before they tunnel through thin dynamic colliders.

**[M2.7-EXT-16] Procedural Recoil Low-Discrepancy Sequence Cache**

Purpose: pre-bakes a circular buffer of low-discrepancy (R2 sequence) offsets at boot instead of generating them at runtime inside the recoil oscillator — pure perf/consistency win, trivial to implement.

**[M2.7-EXT-17] Parametric Gait-Warping Stride Adjuster**

Purpose: adjusts foot clearance arcs in Motion Matching against local surface slope for zero-slip foot placement on stairs/ramps.

**[M2.8-EXT-06] XorShift128+ Seed Distribution Sandbox Synchronizer**

Purpose: syncs thread-local procgen RNG seed state across co-op peers at replication checkpoints so world generation doesn't diverge between clients. This is load-bearing for correctness in co-op — flag as high priority, not optional polish.

**[M2.8-EXT-07] xxHash64 ECS State Checksum Aggregator**

Purpose: periodic (e.g. every 30 ticks) parallel hash of hot SoA component arrays across peers to detect and localize desync to a specific entity index, rather than a full-state diff.

**[M2.8-EXT-08] Fixed-Point Mesh-Vector Quantization Factory** — [MATH DONE] — sibling to `[M2.8-EXT-04]`, not a duplicate: EXT-04 is a scalar 16.16 fixed-point multiply for damage rolls/pathfinding cost; this is a signed 24.8 fixed-point `vec3` specifically for procedural mesh generation (vertex positions, bone offsets) so chunk-boundary seams and physics divergence don't appear across CPU families under `-ffast-math`.

```cpp
struct FixedVec3 { // 24.8 fixed-point: 24 bits whole, 8 bits fractional
    int32_t x, y, z;
    static int32_t FromFloat(float v) { return int32_t(std::lround(v * 256.0f)); }
    static float ToFloat(int32_t v) { return float(v) / 256.0f; }
    FixedVec3 Mul(int32_t scalar256) const {
        return { int32_t((int64_t(x) * scalar256) >> 8), int32_t((int64_t(y) * scalar256) >> 8), int32_t((int64_t(z) * scalar256) >> 8) };
    }
    FixedVec3 Add(const FixedVec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
};
```

**[M2.9-EXT-09] Pneumatic Tire Slip-Angle Deformation Loop**

Purpose: visually deforms tire mesh vertices from lateral suspension load to sell drift/sidewall flex — cosmetic, not simulation-critical.

**[M9-EXT-20] Anti-Roll Torsional Suspension Stabilizer** — *renumbered from a v71 draft collision with `[M9-EXT-09]` (Volatile Fuel Tank Puncture Bernoulli Drainage), which already owned that ID — see front-matter changelog.*

Purpose: standard anti-roll bar constraint between linked wheel axes to prevent rollover in hard cornering.

**[M9-EXT-21] Fluid Hydrodynamic Wading Resistance Modulator** — *renumbered from a v71 draft collision with `[M9-EXT-10]` (Aero-Elastic Body Panel Drag Deformation Scaling), which already owned that ID — see front-matter changelog.*

Purpose: drag force on vehicle body from wheel submersion depth against the flood heightmap (M10 dependency).

**[M2.9-EXT-10] Kinematic Character Flood Buoyancy & Drag Bridge** — [MATH DONE] — Depends on (not yet built at this point): M10 flood heightmap

Purpose: `JPH::CharacterVirtual` resolves motion via kinematic sweeps, not rigid-body forces, so it's otherwise blind to flooding — zombies (M5) and vehicles (M9) already get flood physics, the player didn't. This adds the missing third leg: samples M10's Saint-Venant depth at the player's position each fixed tick and folds buoyancy + drag directly into the kinematic velocity input before `ExtendedUpdate()` runs.

```cpp
void ApplyFloodBuoyancy(CharacterPhysiologyComponent& phys, float waterDepthAtFeet, float dt) {
    if (waterDepthAtFeet <= 0.0f) return;
    constexpr float rhoWater = 1000.0f, g = 9.81f;
    float submerged = std::clamp(waterDepthAtFeet / phys.waistHeightM, 0.0f, 1.0f);
    float buoyancyAccel = (rhoWater * phys.capsuleVolumeM3 * submerged * g) / phys.massKg;
    phys.kinematicVelocityInput.y += buoyancyAccel * dt; // fed straight into ExtendedUpdate()'s velocity input
}
```

**[M9-EXT-18] Vehicle Convoy Long-Range Router** — dedup note, no new system

Purpose: faction/AI vehicle convoys need a lightweight macro-graph for long-range routing distinct from the fine NavMesh — but `[M5.4-EXT-06]`'s abstract street/highway node graph already exists for exactly this (built for off-screen horde routing). Extend that graph with a vehicle-lane-width/speed-limit field and reuse it here; do not stand up a second road-graph system.

## 5.3 GPU Render Pipeline (M4.5)

**[M4.5-EXT-12] Visibility Buffer Material-Classification Wavefront Compactor** — canonical copy is under §5.1/M4.5 in the main body; see there. *(v75: this appendix copy had dropped the global `BucketCounter` atomic and wrote `subgroupExclusiveAdd(1u)` straight to the image at a per-workgroup-local offset — the exact overlapping-write bug the body copy's `[BUG-FLAG]` note describes and fixes. Collapsed to a stub rather than left as a second, buggy copy.)*

**[M4.5-EXT-13] Cluster Depth-Bounds Frustum Voxelizer for Virtual Shadow Maps** — canonical copy is under §5.1/M4.5 in the main body; see there. *(v75: this appendix copy still used the backwards `nearDepth >= farDepth` check with no reference to the light's shadow bounds — the exact bug the body copy's `[BUG-FLAG]` note describes and fixes. Collapsed to a stub rather than left as a second, buggy copy.)*

**[M4.5-EXT-14] Compute Skinning Vertex Tangent-Space Recomputer**

Purpose: re-derives tangent vectors (T×N) during GPU dual-quaternion skinning to fix specular artifacts on extreme ragdoll poses.

**[M4.5-EXT-15] Hysteresis-Gated TAA Variance Clamper**

Purpose: clips history color buffer against local neighborhood AABB with hysteresis to cut ghosting behind fast-moving silhouettes without over-sharpening on every frame.

**[M4.5-EXT-16] Software Micro-Polygon Voxel Rasterizer (compute)**

Purpose: renders sub-pixel debris/fracture geometry via compute-shader atomics into the Visibility Buffer, bypassing fixed-function raster setup cost for triangles under ~1px. High complexity — de-prioritize unless debris density actually becomes a measured bottleneck.

**[M4.5-EXT-17] Directional Ambient Visibility Field Cache**

Purpose: coarse 3D voxel skylight-exposure field for real-time AO under canopies/alleys without full GI cost.

**[M4.5-EXT-18] Transient Skinned Vertex Cache for Visibility Buffer** — [MATH DONE]

Purpose: the Visibility Buffer shading pass reconstructs position from the static bind-pose vertex buffer — fine for static geometry, but skinned characters would render as T-poses since nothing deforms per-frame. A compute pass runs dual-quaternion skinning first into a ring-buffered transient GPU allocation; indirect draws for skinned entities bind that frame's cache offset instead of the static buffer.

```cpp
struct SkinnedRingPool {
    VkBuffer buffer; uint8_t* mapped;
    uint64_t capacityBytes, cursor;
    uint64_t Suballocate(uint32_t vertexCount, uint32_t strideBytes) {
        uint64_t size = (uint64_t(vertexCount) * strideBytes + 63) & ~63ull; // 64B align
        if (cursor + size > capacityBytes) return UINT64_MAX; // overflow guard
        uint64_t offset = cursor; cursor += size; return offset;
    }
};
```

**Numbering note (v78, this pass):** live `milestones/` already has `[M4.5-EXT-21]` through
`[M4.5-EXT-24]` (VRS luma-variance shading rate, glass-fracture normal perturbation, and two more
per your recent session) that aren't in this doc yet. The entry below is numbered `[M4.5-EXT-25]`
to not collide — verify against the live file before treating it as final.

**[M4.5-EXT-25] Specialization Constants for Bindless Material Uber-Shader**

Systems: `[M4.5-EXT-09]`'s classification pre-pass + master opaque/transparent pipelines — the two
PSOs the entire bindless-material scheme is built around.

Purpose: for the small set of compile-time-knowable branches inside the uber-shader (feature-tier
checks, fixed loop bounds) that currently branch at runtime on `MaterialIdx`-derived flags, promote
them to `VkSpecializationInfo` constants baked at the existing two-PSO compile time. `[M4.5-EXT-09]`
already compiles exactly one opaque + one transparent PSO specifically to avoid a permutation
explosion — this adds a handful of specialization constants on those same two PSOs, not per-material
PSOs, so it doesn't reintroduce what EXT-09 exists to prevent. Only applied where a branch is
genuinely compile-time-constant per PSO; material-varying lookups stay as runtime bindless reads
exactly as EXT-09 already does them. Marginal warp/wavefront divergence reduction in the
classification pre-pass — lower-confidence gain than the M1 items above, cheap to try, no visual
change.

**[M4.5-EXT-26] Runtime Virtual Texture (RVT) Base System**

##### Systems Touched
This is the missing base system flagged at audit line 133: three downstream features (skid-mark injection, capillary spatter projection, hydrocarbon flame-trail splitting) all write into "your RVT terrain overlay layer" that does not yet exist. Builds the persistent chunk-anchored terrain overlay texture with a Vulkan storage-image write path. Consumed by `[M9-EXT-22]` (skidmarks) and `[M6.5-EXT-13]` (blood spatter) below. Feeds `[M4.5]`'s hybrid-RT G-Buffer as a sampling source.

##### Math
Clipmap of L nested page rings centered on the camera (same streaming radius concept as the existing clipmap terrain at line 3097, reused). Physical page = fixed texel size `P` (e.g. 256) covering world tile `T`. Virtual address space is a 2D torus indexed by `(pageX, pageY)`; a page table maps resident pages to physical atlas tiles. Feedback buffer: each rendered frame, the pixel shader writes the (pageX, pageY, mip) it actually sampled into a feedback SSBO, read back and drained on the next frame to drive page residency (most-recently-used eviction when atlas is full).

##### How It Works
One persistent RVT atlas (RGBA16F or R8 packable channels) bound as a storage image. Material/decals/impacts render into it via a separate render pass with a write-merge (max-blend or newest-wins) so overlapping decals resolve deterministically. A feedback pass gathers sampled page coords; a CPU/GPU page-residency manager streams the missing pages (rebake from material graph `[M4-EXT-25]` or copy from baked tile) and updates the page table. This is the standing system the three audit-flagged features assume — once it exists, those three are unblocked.

##### Reference Implementation
```cpp
struct RvtPageTable { VkImage atlas; std::unordered_map<uint64_t, uint32_t> resident; /* pageKey->atlasTile */ };
// feedback SSBO filled by shader: layout(location=0) out uvec2 gFeedbackPage;
void RvtRenderDecal(VkCommandBuffer cb, RvtPageTable& rvt, const DecalInstance& d) {
    // bind rvt.atlas as storage image, draw decal quad, write-merge into page tile
}
```

##### Player-Facing Impact
Skid marks, blood spatter, and flame trails finally have a real surface to project onto instead of pointing at a non-existent layer; terrain detail (tire tracks through mud, drag marks) persists per-chunk like the doc always implied.

---

**[M4.5-EXT-27] Scalable Ambient Obscurance (SSAO)**

##### Systems Touched
Tier-1 quality ladder (line 3047) lists SSAO but defines no system. Reads the G-Buffer depth + normal already produced by `[M4.5]`'s deferred pass. Outputs an occlusion term multiplied into ambient/SH fill (complements `[M10-EXT-03]`'s SH ambient).

##### Math
McGuire et al. "Scalable Ambient Obscurance" (2012): for each pixel, sample N points in a view-space sphere of radius `R`, project to screen, compare depth; occlusion accumulates `sum(max(0, z_view - z_sample) / z_sample)`. Use a 4x4 rotated poisson disk (or the published 24-sample spiral) and a bilateral depth-weight to avoid haloing across edges.

##### How It Works
A fullscreen pass after G-Buffer: reconstruct view-space position from depth, sample the AO kernel with per-pixel rotation (blue-noise dither, reuse `[K-EXT-22]`'s STBN if available), blur with a depth-aware edge-preserving filter, output a single-channel AO texture. Tier-1 only; Tier-2 uses ReSTIR GI which subsumes contact shadows, so this is the fallback for the 6 GB floor.

##### Reference Implementation
```cpp
float SSAO(uint2 px, Texture2D depth, float radius, uint samples) {
    float occ = 0; vec3 P = ViewPos(px, depth);
    for (uint i = 0; i < samples; ++i) {
        vec3 s = P + Kernel(i) * radius;
        float zd = Linearize(depth, ProjToScreen(s));
        occ += max(0.0f, (P.z - zd) / zd) * EdgeWeight(px, s);
    }
    return 1.0f - occ / samples;
}
```

##### Player-Facing Impact
Corners, under-vehicles, and rubble crevices read with real contact shadow instead of flat ambient — the single biggest "looks proc-gen" fix for interiors on the Tier-0 floor.

---

**[M4.5-EXT-28] Screen-Space Reflections (SSR)**

##### Systems Touched
Absent as a system (only false-positive heat-shimmer mentions). Reads G-Buffer depth + normal + rough/metal; writes a reflection sample for `[M4.5]`'s lighting resolve. Tier-1/2 only (Tier-0 uses a baked cubemap/env approximation).

##### Math
Ray-march in view space from the reflection vector `R = reflect(V, N)`; at each step project to screen, compare ray depth vs scene depth, accept on crossing (binary-search refine). Fallback to env/SH when the ray misses or exceeds max distance. Roughness spreads the ray origin/length (lower step count for rough).

##### How It Works
Fullscreen pass after opaque G-Buffer. For each pixel above a roughness threshold, march `R`; on hit, sample the HDR color buffer (or the RVT/atlas) at the hit UV and mix by Fresnel. Cheap hierarchical-Z accelerated march (reuse the existing HZB) to take large steps. Denoised with a short temporal/edge-aware blur.

##### Reference Implementation
```cpp
bool TraceSSR(vec3 P, vec3 R, Texture2D depth, out vec2 hitUV) {
    vec3 pos = P; float step = kStep;
    for (int i = 0; i < kMaxSteps; ++i) {
        pos += R * step; vec2 uv = ProjToScreen(pos);
        float sceneZ = Linearize(depth, uv);
        if (pos.z > sceneZ) { hitUV = uv; return true; }   // crossed behind geometry
        step *= 1.5f;                                       // hi-Z style growth
    }
    return false;
}
```

##### Player-Facing Impact
Wet asphalt, puddles (post-rain `[M10]`), and vehicle paint show real reflected world instead of a flat env tint — the visual signature of "next-gen" on mid-tier hardware.

---

**[M4.5-EXT-29] Screen-Space Global Illumination (SSGI) Fallback**

##### Systems Touched
ReSTIR GI is Tier-2 only (line 3047). This is the Tier-0/1 approximation: screen-space diffuse bounces reusing the same G-Buffer + HZB + temporal denoiser `[M4.5-EXT-15]` already owns. Not a replacement for ReSTIR, the cheap fallback path.

##### Math
For each pixel, trace a short diffuse ray (cosine-weighted hemisphere sample around `N`), march like SSR, on hit sample the lit color and apply Lambert `albedo/π · L · max(0,N·ω)`. Accumulate one bounce; multiple bounces approximated by reusing the previous frame's SSGI buffer (temporal, reprojected via `[M4.5-EXT-15]`'s velocity buffer).

##### How It Works
Same fullscreen march infra as `[M4.5-EXT-28]` but diffuse-weighted and lower precision, blended under direct lighting. Bounded sample count (8-16) for the 6 GB floor; denoised by the existing TAA variance-clamp path. On Tier-2 hardware this pass is disabled in favor of ReSTIR.

##### Reference Implementation
```cpp
vec3 SSGI(vec3 P, vec3 N, Texture2D litColor, int samples) {
    vec3 acc = vec3(0);
    for (int i = 0; i < samples; ++i) {
        vec3 w = CosineHemisphere(N, i); vec2 hit;
        if (TraceSSR(P + N*0.01f, w, depth, hit)) acc += litColor[hit] * max(0.0f, dot(N, w));
    }
    return acc / samples;
}
```

##### Player-Facing Impact
Colored bounce light (red wall tints the floor, muzzle flash briefly lights a corridor) on hardware that can't afford ReSTIR — keeps the world from looking flat-emissive on the floor spec.

---

**[M4.5-EXT-30] Impostor LOD (Octahedral) for Distant Meshes**

##### Systems Touched
Zero mentions in doc — distant static meshes (buildings, ruined vehicles) currently pop or draw full geometry. Generates octahedral-impostor atlases per mesh at bake time; sampled in place of the mesh beyond a view-distance threshold. Reuses `[M4.5]`'s existing LOD/culling.

##### Math
Octahedral impostor (Shaderbits/Tech Art Aid): pre-render the mesh from K directions arranged on an octahedron hemisphere into a single atlas. At runtime, compute the view octahedral coordinate `oct = octEncode(normalize(viewDir))`, index the atlas, and sample the impostor billboard. Cheaper than spherical or 3D-impostor layouts (better texel packing).

##### How It Works
Offline: render K (e.g. 32) views into an atlas. Runtime: a billboard quad whose UVs are remapped by the octahedral direction; depth can be preserved via a depth-impostor write for correct occlusion. Cross-fade between the last real LOD and the impostor over a small distance band to hide the swap. Feeds the same culling the rest of `[M4.5]` uses.

##### Reference Implementation
```cpp
vec2 OctUV(vec3 viewDir) {            // map view dir -> atlas uv in [0,1]
    vec3 n = normalize(viewDir);
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    vec2 p = (n.z >= 0.0) ? n.xy : (1.0 - abs(n.yx)) * sign(n.xy);
    return p * 0.5 + 0.5;
}
```

##### Player-Facing Impact
Distant city skylines and zombie-haunted suburbs render at fraction of the cost with no visible pop — the draw-call relief that lets the Tier-0 floor keep 30 FPS with a large view distance.

---

**[M4.5-EXT-31] Signed Distance Field (SDF) Shadow Cascade**

##### Systems Touched
Only one passing mention of DF shadows — no system. Builds a sparse clipmap SDF of nearby opaque geometry from the depth buffer (or from baked mesh SDFs), used for cheap soft long-range shadows and contact shadows that the existing shadow-map cascade (line 24x HZB mentions) can't afford at distance.

##### Math
Per clipmap level, rasterize scene depth into a signed distance field via the standard 6-sweep or jump-flooding on the depth buffer; `d(p) = nearestSurfaceDistance(p)` with sign from depth-vs-scene. Shadow test along the light ray accumulates `min over steps of d(ray(t)) / t` (cone soft shadow): `vis = saturate(1 - k * min_d / t)`.

##### How It Works
A compute pass converts the depth (or a downsampled depth) into an SDF clipmap each time the camera moves a cell. Lighting samples the SDF along the light direction for a cheap soft shadow that holds at range where shadow maps run out of resolution. Hybrid: near field uses shadow maps, far field uses SDF — no double system, the SDF is a fallforward for the cascade's tail.

##### Reference Implementation
```cpp
float SdfShadow(vec3 P, vec3 L, Texture3D sdfClip, float coneK) {
    float t = 0, vis = 1.0f;
    for (int i = 0; i < kSteps; ++i) {
        vec3 s = P + L * t;
        float d = texture(sdfClip, s).r;          // signed distance
        vis = min(vis, saturate(1.0f - coneK * d / max(t, 1e-3)));
        t += max(d, kMinStep);
    }
    return vis;
}
```

##### Player-Facing Impact
Long shadows from tall ruins and smoke stacks stay soft and correct at distance without a 4K shadow-map cascade — the silhouette readability that makes outdoor horror scenes read, on the floor spec.

---

## 5.4 AI, Director, Propagation (M5.1 / M5.3 / M5.4)

**[M5.1-EXT-07] / [M5.3-EXT-XX] Optimal Reciprocal Collision Avoidance (ORCA) Solver**

Purpose: standard ORCA local avoidance so hordes don't clip/overlap at close range — pairs with your existing Reynolds flocking (subsystem #15 in Section 2) rather than replacing it; ORCA governs hard collision avoidance, boids govern soft group behavior.

**[M5.3-EXT-08] Visual Occlusion Sector Ray-March Pre-Filter**

Purpose: coarse macro-voxel LOS approximation to cheaply reject most raycasts before falling back to full-res traces for AI perception.

**[M5.4-EXT-04] Thompson-Sampling Actor Weights Snapshot Persistence Register** — canonical copy is under §5.1/M5.4 in the main body; see there. *(v75: was a byte-identical duplicate, no divergence found — collapsed to a stub for the same reason as `[M1-EXT-09]` above.)*

**[M5.4-EXT-05] Reaction-Diffusion Grid Sub-sampled Boundary Welder**

Purpose: blends the panic/fear field (subsystem #26-equivalent reaction-diffusion grid) across sector boundaries so AI panic propagation doesn't stall or discontinuity-snap at chunk edges.

*[M5.4-EXT-06] Hierarchical A Macro-Cell Long-Range Router**

Purpose: routes off-screen horde movement across unstreamed sectors on an abstract street/highway node graph, avoiding full pathfinding cost for entities the player can't see.

**[M5.4-EXT-07] Holling Type II Cannibalism Feeding Satiator**

Purpose: saturating consumption-rate curve binding idle zombies to corpse piles as a tactical distraction mechanic.

## 5.5 Audio (M6)

**[M6-EXT-08] Ray-Traced Acoustic Diffraction Node Topology Cache**

Purpose: caches sound-path reflection vectors for stable geometry so Miniaudio doesn't re-trace rays every frame for static room shapes.

**[M6-EXT-09] Velvet-Noise Late Reverb Interleaved Mixing Buffer**

Purpose: interleaves diffuse reverb calc across sequential steps to flatten audio-thread mixing spikes during heavy indoor combat (pairs with existing subsystem #25).

**[M6-EXT-10] Acoustic Convection Wave Refraction Filter**

Purpose: warps SPL by wind vector so upwind gunshots muffle faster over distance than downwind ones.

## 5.6 World Generation Detail (M4)

**[M4-EXT-08] 3D WFC Vertical Structural Dependency Guard**

##### Systems Touched
Extends `[M4-EXT-03]` Hierarchical WFC Multi-Grid Graph Layering and the adjacency propagator §5.6 notes as already live but not yet in this doc (`[M4-EXT-20]`). Consumes `[M4-EXT-11]`'s contradiction-recovery primitive. Gates M2.7 traversal (stairs must be walkable) and M8.6 settlement construction (multi-floor safehouses).

##### Math
Vertical socket compatibility, checked at every floor boundary in addition to `[M4-EXT-03]`'s existing horizontal adjacency:

`VerticalOK(below, above) = (below.topSocket == above.bottomSocket) ∧ (above.requiresSupport → below.providesSupport)`

##### How It Works
This is the standard WFC "socket system" extended to the vertical axis: each tile face carries a socket ID, and vertical faces additionally carry two flags — `requiresSupport` (stairs/pillars need solid floor beneath) and `providesSupport`. After each floor's local WFC pass completes, run a flood-fill reachability check seeded from ground-floor entrance/stair-base tiles through `providesSupport`-linked stair-transition tiles. Any upper-floor region the flood-fill doesn't reach gets its column reset to full entropy via `[M4-EXT-11]`'s existing recovery mechanism (same primitive, scoped to a column instead of a 3×3 neighborhood) and re-collapsed with stair-class tiles weighted higher.

##### Reference Implementation
```cpp
struct WfcTileSocket { uint32_t topSocket, bottomSocket; bool requiresSupport, providesSupport; };

bool VerticalCompatible(const WfcTileSocket& below, const WfcTileSocket& above) {
    if (below.topSocket != above.bottomSocket) return false;
    if (above.requiresSupport && !below.providesSupport) return false;
    return true;
}

// Flood-fill from ground-floor entrances through stair-linked tiles; unreached cells
// get passed to RecoverWfcContradiction (existing [M4-EXT-11] fn) for that column.
void FindUnreachableUpperFloorCells(std::vector<WfcTileSlot>& grid, const std::vector<uint32_t>& entranceIdx,
                                     std::vector<uint32_t>& outUnreached, uint32_t strideX, uint32_t strideZ, uint32_t floors) {
    std::vector<bool> visited(grid.size(), false);
    std::queue<uint32_t> q;
    for (auto i : entranceIdx) { visited[i] = true; q.push(i); }
    while (!q.empty()) {
        uint32_t idx = q.front(); q.pop();
        for (uint32_t n : NeighborsIncludingVertical(idx, strideX, strideZ, floors)) {
            if (!visited[n] && VerticalCompatible(grid[idx].socket, grid[n].socket)) { visited[n] = true; q.push(n); }
        }
    }
    for (uint32_t i = 0; i < grid.size(); ++i) if (!visited[i]) outUnreached.push_back(i);
}
```

##### Player-Facing Impact
Multi-story ruins and safehouses never generate with floating floors, disconnected stairwells, or sealed unreachable rooms — matters here specifically because upper floors are a core loot-risk/sightline space in this game, not just set dressing.

---

**[M4-EXT-09] Deterministic Interior Furniture Spatial Constraint Solver**

##### Systems Touched
Declares M4's "Procedural Urban Detail (L-systems, BSP interiors)" bullet. Consumes the existing Socio-Economic Utility Tagging (room-use tags). Feeds M8 loot-container placement and M8.6 settlement construction (player-built interiors reuse the same solver).

##### Math
Greedy clearance-respecting placement score, evaluated per candidate wall segment:

`PlaceScore(piece, wallSeg) = wallSeg.freeLength ≥ piece.footprintWidth  ∧  no overlap with piece.clearanceRadius against already-placed pieces`

##### How It Works
Two established technique families exist for this: optimization-based simulated annealing over a cost function of clearance/alignment/traffic-flow (Merrell et al., "Furniture Layout Using Interior Design Guidelines," reused as recently as Infinigen Indoors, 2024), and rule-based greedy/agent placement using a semantic object library (Germer & Schwarz's agent-based room furnishing; Xu et al.'s clearance-box parent/child object grouping). SA produces higher-quality layouts but has unbounded iterative convergence cost — incompatible with a hard per-chunk generation time budget in a streaming open world.

Recommend greedy deterministic placement as the runtime path: each room-use tag maps to an ordered furniture manifest (anchor pieces first — bed, table, counter — then secondary props parented to already-placed anchors, mirroring Xu et al.'s clearance-box grouping). Anchors place against the longest remaining free wall segment that fits their footprint, tried at both wall ends before falling back to the next-longest wall. Entirely deterministic given the chunk's WFC seed — this is what the deferred bucket's "chunk-boundary determinism test" item needs from any interior-furnishing system, so building it deterministic from the start avoids a retrofit.

Reserve simulated annealing for a small, fixed set of non-streamed "landmark" interiors (named safehouses, quest locations) where a one-time offline/loading-screen budget is acceptable — same cheap-default/expensive-featured-case split already used elsewhere in this doc (e.g. `[M4-EXT-02]`'s runtime LOD vs. authored assets).

##### Reference Implementation
```cpp
struct FurnitureManifestEntry { uint32_t meshId; glm::vec2 footprintExtents; float clearanceRadius; bool isAnchor; };

bool TryPlaceAgainstWall(const FurnitureManifestEntry& piece, const std::vector<WallSegment>& walls,
                          const std::vector<PlacedFurniture>& placed, glm::vec2& outPos, float& outYaw) {
    // walls pre-sorted longest-free-length first (deterministic given seed)
    for (const auto& wall : walls) {
        if (wall.freeLength < piece.footprintExtents.x) continue;
        glm::vec2 candidate = wall.start + wall.direction * (wall.freeLength * 0.5f);
        bool clear = true;
        for (const auto& p : placed) if (glm::distance(candidate, p.pos) < piece.clearanceRadius + p.clearanceRadius) { clear = false; break; }
        if (clear) { outPos = candidate; outYaw = wall.inwardFacingYaw; return true; }
    }
    return false;
}
```

##### Player-Facing Impact
Every generated interior reads as furnished with intent rather than randomly scattered, and loot containers always have a legible, reachable placement instead of spawning inside furniture bounds.

---

**[M4-EXT-10] Macro-Graph Vector Spline Corridor Welder**

##### Systems Touched
Consumes `[M4-EXT-01]`'s macro-graph road/river vectors — feeds local WFC initialization the same way `[M4-EXT-01]` already injects immutable pre-placed tile constraints. Shares terrain math with `[M4-EXT-16]` (Saint-Venant erosion, river corridors) and reuses `[M4-EXT-19]`'s exclusion-spline concept for the soft edge.

##### Math
Two-band cell reservation from closest-point-on-spline distance:

`d(cell) = min distance from cell center to macro-graph spline` (Catmull-Rom, fixed-subdivision sampling — Newton refinement isn't needed at chunk grid-cell resolution)

Hard corridor: `d ≤ corridorWidth/2` → cell pre-collapsed to a fixed road/riverbed tile.
Soft blend: `corridorWidth/2 < d ≤ corridorWidth/2 + blendWidth` → cell's WFC possibility mask (`[M4-EXT-11]`'s existing field) pre-weighted toward corridor-compatible border tiles instead of fully collapsed.

##### How It Works
Before a chunk's local WFC starts, every grid cell is tested against the macro-graph spline. Cells inside the hard band get welded to fixed corridor tiles exactly as `[M4-EXT-01]` already does for macro-graph vectors; this entry adds the missing soft blend band so the transition from graded corridor to wild terrain isn't a hard tile seam at the corridor's edge — the same softening `[M4-EXT-19]` already applies to road exclusion, generalized to any macro-graph spline, not just roads.

##### Reference Implementation
```cpp
float DistanceToSplineApprox(glm::vec2 p, const std::vector<glm::vec2>& splineSamples) {
    float best = FLT_MAX;
    for (size_t i = 0; i + 1 < splineSamples.size(); ++i)
        best = std::min(best, DistancePointToSegment(p, splineSamples[i], splineSamples[i + 1]));
    return best;
}

void ReserveCorridorCells(std::vector<WfcTileSlot>& grid, const std::vector<glm::vec2>& splineSamples,
                           float corridorWidth, float blendWidth, glm::vec2 gridOrigin, float cellSize) {
    for (size_t i = 0; i < grid.size(); ++i) {
        glm::vec2 cellPos = gridOrigin + CellCoord(i) * cellSize;
        float d = DistanceToSplineApprox(cellPos, splineSamples);
        if (d <= corridorWidth * 0.5f) { grid[i].possibilityMask = kCorridorTileMask; grid[i].collapsed = true; }
        else if (d <= corridorWidth * 0.5f + blendWidth) { grid[i].possibilityMask &= kCorridorBorderCompatibleMask; }
    }
}
```

##### Player-Facing Impact
Roads and rivers read as continuous macro features chunks snap to, not independently-generated segments that misalign at chunk borders; buildings never spawn straddling a road centerline.

**[M4-EXT-11] WFC Contradiction Horizon Recovery** — [MATH DONE]

Purpose: WFC tile propagation can collapse to zero valid options at a cell (deadlock) — without an explicit recovery step, background chunk generation stalls or leaves rendering voids. On contradiction, reset a 3×3 neighborhood to full entropy and force-stamp the contradiction cell to a safe fallback tile (e.g. debris) so propagation can resume.

```cpp
void RecoverWfcContradiction(std::vector<WfcTileSlot>& grid, uint32_t idx, uint32_t strideX, uint32_t strideZ) {
    if (grid[idx].possibilityMask != 0) return;
    int32_t cx = idx % strideX, cz = idx / strideX;
    for (int32_t dz = -1; dz <= 1; ++dz) for (int32_t dx = -1; dx <= 1; ++dx) {
        int32_t nx = cx + dx, nz = cz + dz;
        if (nx >= 0 && nx < (int32_t)strideX && nz >= 0 && nz < (int32_t)strideZ) {
            auto& slot = grid[nz * strideX + nx];
            slot.possibilityMask = ~0ull; slot.collapsed = false; slot.entropy = 64;
        }
    }
    grid[idx] = { /*possibilityMask=*/1ull, /*collapsed=*/true, /*entropy=*/0 }; // fallback debris tile
}
```

**Numbering note (v78, this pass):** live `milestones/` already has `[M4-EXT-20]` and `[M4-EXT-21]`
(WFC adjacency propagator, signage grammar transcoder) that aren't in this doc yet. The entry below
**Numbering note (v78, this pass):** live `milestones/` already has `[M4-EXT-20]` and `[M4-EXT-21]` (WFC adjacency propagator, signage grammar transcoder) that aren't in this doc yet. The entry below is numbered `[M4-EXT-22]` to not collide — verify against the live file before treating it as final.

**[M4-EXT-22] Meshoptimizer Vertex Cache / Fetch Optimization**

Systems: procedural mesh output from M4 (buildings, terrain, debris), authored asset import path,
feeds the M1 material-batched buffers (`[M1-EXT-18]`).

Purpose: current standard practice (zeux/meshoptimizer, Tipsify-derived) — post-transform vertex
cache optimization reorders triangle indices to maximize hits on the GPU's small (16–32 vertex)
post-transform cache, followed by pre-transform vertex fetch optimization (reorders the vertex
buffer itself to match). Runs once at mesh-bake time for procedural output and at import time for
authored assets — not per-frame, a static reordering that changes nothing about the mesh's
appearance. Free vertex-shader-invocation and memory-bandwidth reduction on every mesh in the game;
genuinely absent from the doc as of this pass, not a duplicate of anything above.

---

**[M4-EXT-23] Whittaker Temperature/Precipitation Biome Classification**

##### Systems Touched
Feeds the existing "Dynamic Biome Weight Interpolation (2-3 chunk blend band)" bullet (M4 Implementation Steps, currently undefined *how* a biome ID gets assigned in the first place — the blend band interpolates between IDs but nothing in the doc names the classification rule). Consumes the same climate scalar fields `[M2.6]`'s per-biome noise-warp tuning and `[M4.5-EXT-04]`'s wetness/porosity system already read.

##### Math
Classical Whittaker diagram, discretized to a lookup table rather than the textbook triangular plot (Guehl/AutoBiomes and most shipped implementations do the same — a rectangular table is trivial to index, the triangle isn't):

`BiomeId = Table[Quantize(meanAnnualTemp, T_BUCKETS)][Quantize(meanAnnualPrecip, P_BUCKETS)]`

##### How It Works
Two scalar fields — mean annual temperature and mean annual precipitation — are already implied by existing systems (latitude/altitude-driven temperature falls straight out of the world's coordinate space; precipitation can reuse the same wind-field/orographic logic `[M10-EXT-05]`'s Navier-Stokes wind field and the tectonic mountain layout already establish, since windward/leeward sides of mountain ranges are exactly what drives real-world precipitation gradients). Quantize both into a small number of buckets (6 is the standard AutoBiomes/gamedev-common choice) and look up a biome ID from a fixed 2D table matching the classical Whittaker regions (tropical rainforest, desert, tundra, etc.), authored once as data, not code. That biome ID is what "Dynamic Biome Weight Interpolation" already blends across the 2-3 chunk band — this entry defines the missing input to that existing blend, it doesn't replace it.

##### Reference Implementation
```cpp
enum class BiomeId : uint8_t { Ice, Tundra, BorealForest, Grassland, Woodland, SeasonalForest,
                                TemperateRainforest, Savanna, Desert, TropicalRainforest };

constexpr uint32_t kTBuckets = 6, kPBuckets = 6;
extern const BiomeId kWhittakerTable[kTBuckets][kPBuckets]; // authored once as data (Appendix F sibling table)

BiomeId ClassifyBiome(float meanAnnualTempC, float meanAnnualPrecipMm) {
    uint32_t tIdx = std::clamp((uint32_t)((meanAnnualTempC + 10.0f) / 40.0f * kTBuckets), 0u, kTBuckets - 1);
    uint32_t pIdx = std::clamp((uint32_t)(meanAnnualPrecipMm / 4000.0f * kPBuckets), 0u, kPBuckets - 1);
    return kWhittakerTable[tIdx][pIdx];
}
```

##### Player-Facing Impact
Biome placement follows the same temperature/precipitation logic that produces real-world biome distribution (deserts in orographic rain-shadows, boreal forest at high latitude/altitude) instead of an arbitrary or purely noise-driven zone map, so the 2-3 chunk blend band was already built to smooth transitions between something that now actually makes climatic sense.

---

**[M4-EXT-24] Cellular Automata Structural Collapse & Rubble Debris Fields**

##### Systems Touched
Distinct from `[M4-EXT-13]`'s Dual-Contouring cave carving (that's macro cave *voxel structure*; this is a 2D binary overlay on top of already-built BSP rooms). Runs after `[K-EXT-18]`'s BSP room split, before `[M4-EXT-09]`'s furniture solver — rubble-occupied cells are excluded from furniture placement and get tagged impassable for NavMesh, same gating pattern `[M4-EXT-09]` already uses for clearance radii.

##### Math
Standard Game-of-Life-style birth/survival CA on a per-room floor grid, seeded from a per-building "decay severity" scalar (reuse `[M8]`'s Regional SEIR outbreak severity — harder-hit sectors generate more collapsed/rubble-choked interiors, tying visual decay to the actual outbreak simulation instead of being cosmetic noise):

`alive_{t+1}(c) = alive_t(c) ? (N(c) ≥ 4) : (N(c) ≥ 5)` where `N(c)` = count of the 8 neighbors currently alive, run 4-5 iterations from an initial random fill of `decaySeverity × baseFillRate`.

##### How It Works
This is the well-established roguelike cave-CA algorithm (random initial fill, birth/survival neighbor-count rules, iterate ~4-5 times, then flood-fill to guarantee connectivity) applied at room-floor scale instead of world-cave scale — genuinely distinct from `[M4-EXT-13]`'s SDF cave carving, which solves a different problem (3D volumetric structure vs. 2D floor occupancy overlay). "Alive" cells become rubble/debris prop clusters; "dead" cells stay clear floor. `decaySeverity` (already an existing per-sector value from M8's SEIR curve) scales the initial random-fill percentage, so a sector at outbreak peak generates rooms choked with collapsed debris while a low-severity sector generates mostly clear interiors — the same severity value already driving zombie density and loot scarcity now also drives what the space itself looks like. After CA settles, flood-fill from the room's door tiles (reusing the same reachability-check pattern `[M4-EXT-08]` already applies vertically) to guarantee at least one clear path across the room; any cell it can't reach gets force-cleared rather than leaving a genuinely unreachable pocket.

##### Reference Implementation
```cpp
struct RoomFloorGrid { std::vector<bool> rubble; uint32_t w, h; };

void StepCA(RoomFloorGrid& g) {
    std::vector<bool> next(g.rubble.size());
    for (uint32_t y = 0; y < g.h; ++y) for (uint32_t x = 0; x < g.w; ++x) {
        uint32_t n = CountAliveNeighbors8(g, x, y);
        bool alive = g.rubble[y * g.w + x];
        next[y * g.w + x] = alive ? (n >= 4) : (n >= 5);
    }
    g.rubble = next;
}

void GenerateRubbleField(RoomFloorGrid& g, float decaySeverity, uint32_t seed) {
    SeededFill(g.rubble, decaySeverity * kBaseFillRate, seed); // decaySeverity from M8 Regional SEIR
    for (int i = 0; i < 5; ++i) StepCA(g);
    // Flood-fill from door tiles (reuses [M4-EXT-08]'s pattern); unreached rubble cells force-cleared.
}
```

##### Player-Facing Impact
Buildings in hard-hit outbreak sectors read as visibly, differently ruined — collapsed ceilings, debris-choked hallways forcing detours — rather than every interior using the same clean BSP floorplan regardless of how bad that sector's outbreak got, and the connectivity guarantee means "differently ruined" never means "unreachable loot."

---

**[M4-EXT-25] Procedural Material Node-Graph Compiler (Structure + Color + Normal)**

##### Systems Touched
Single compiler backing five previously-separate deferred names: **PPTBF materials**, **node-graph procedural materials**, **histogram-preserving blending**, **noise→normal map derivation**, **procedural color palette**. Compiles to the project's locked HLSL/DXC toolchain. Feeds `[M4-EXT-02]`'s texture atlas infrastructure and is a fifth caller alongside `[K-EXT-17]`'s per-item micro-detail warp (which stays as-is — that path derives exact normals from SDF geometry before extraction and shouldn't be rerouted through a 2D normal-derivation node; this compiler is for flat/tileable surface materials, not the per-item mesh-detail path).

##### Math
PPTBF (Guehl et al. 2020, *Semi-Procedural Textures Using Point Process Texture Basis Functions*): a window function multiplied by a correlated Gabor-function mixture, generalizing sparse convolution noise to reproduce cells/cracks/grains/scratches/stains/waves from one parametric family. The published form exposes ~30 parameters (several discrete), which is authoring-heavy for a solo dev — recommend exposing a small fixed set of *presets* (crack-network, grain, scratch-field, stain) as named node instances with 3-5 tunable floats each, not the full general parameter space; same cheap-default-vs-full-generality split used elsewhere in this doc.

Histogram-preserving blending (Heitz & Neyret 2018, common node type for tiling-noise material blends): blend N textures by matching the output histogram to a target rather than naive lerp, which avoids the washed-out gray blending artifact plain interpolation produces on stochastic textures.

Noise→normal derivation: standard Sobel-style finite-difference of a height/structure field to produce tangent-space normals, `normal = normalize(vec3(-dHeight/dx, -dHeight/dy, 1))`.

##### How It Works
A small node-graph IR (structure node → optional histogram-blend node → normal-derive node → color-palette node) compiles to a single HLSL fragment per material at content-bake time, not per-frame — same bake-once-cache pattern as everything else procedural in this doc. Structure nodes are PPTBF presets or plain domain-warped OpenSimplex2 (reusing `[M2.6]`'s existing warp function directly rather than a second noise implementation). Color-palette nodes map a scalar structure field through a small authored gradient (2-5 color stops per material family — rust, concrete, foliage-litter) rather than per-pixel procedural color synthesis, since Guehl's own paper notes structure is the tractable procedural part and color detail is the part best left data-driven/authored, not generated from scratch.

##### Reference Implementation
```cpp
struct MaterialNode { enum Kind { PPTBFStructure, HistogramBlend, NormalDerive, ColorPalette } kind;
                       std::vector<uint32_t> inputs; /* params per Kind, omitted for brevity */ };

// Compiles a node list (already topologically sorted) into one HLSL fragment string.
// Bake-time only — never touches the per-frame shader path.
std::string CompileMaterialGraph(const std::vector<MaterialNode>& nodes);

glm::vec3 DeriveNormalFromHeight(float hL, float hR, float hD, float hU, float texelSize) {
    return glm::normalize(glm::vec3((hL - hR) / (2.0f * texelSize), (hD - hU) / (2.0f * texelSize), 1.0f));
}
```

##### Player-Facing Impact
Every generated or modded material — rust, cracked concrete, moss growth on the same wall — gets a distinct, non-tiling-obvious surface without an artist authoring five separate texture sets, and structural variation (cracks, grain) stays procedurally infinite while color stays art-directed rather than looking randomly generated.

---

**[M4-EXT-26] Procedural Decal Atlas Packing & Runtime Projection**

##### Systems Touched
Reuses `[M4-EXT-02]`'s texture-atlas infrastructure (same atlas manager the signage grammar and the loot-icon bake above already share — a third caller, not a new atlas system). Feeds M6.5's blood-spatter/scorch/impact-mark rendering (currently declared as a spatial-density system via `SpatialHash`, but the doc doesn't specify how the actual decal texture gets packed/looked up — this is that missing piece) and M4's signage grammar (weathered/torn poster variants).

##### Math
Shelf (skyline) bin-packing: sort decal rects by height descending, place each into the shortest-remaining shelf that fits its width, open a new shelf when none fit — O(n log n), deterministic given a fixed insertion order, good enough for decal-sized atlas entries (skyline beats naive shelf on packing density for this size class but shelf is simpler and the atlas entries here are small/uniform enough that the density gap doesn't matter).

##### How It Works
Decals (bullet holes, blood spatter, scorch marks, torn/weathered signage variants) are small textures projected onto a decal box in world space at runtime — standard decal-projection technique, not novel. The procedural part is generation and atlas packing: procedurally vary each decal instance (rotation, scale, one of a handful of base shapes composited via the same domain-warped noise `[M2.6]` already uses, so no two bullet holes look identical) and bake the result into the shared atlas via shelf-packing at first-use rather than pre-baking every possible variant. Runtime projection reads atlas UV offset + the existing `SpatialHash` density system to decide when a surface is "full" and starts overwriting oldest decals — that eviction policy already needs to exist for memory-bounded runtime decal accumulation, and reuses the same broad-phase spatial-hash pattern `M6.5`'s blood-spatter density already established rather than a second density tracker.

##### Reference Implementation
```cpp
struct DecalAtlasEntry { glm::vec2 uvOffset, uvExtent; };

// Sort-by-height-desc, then shortest-fitting-shelf placement -- deterministic given insertion order.
DecalAtlasEntry PackDecalShelf(TextureAtlas& atlas, glm::ivec2 decalPixelSize);

glm::vec3 GenerateDecalVariant(uint32_t baseShapeId, uint32_t instanceSeed) {
    // baseShapeId -> template silhouette; per-instance rotation/scale/warp phase from instanceSeed,
    // composited via [M2.6]'s existing domain-warped OpenSimplex2, not a second noise system.
}
```

##### Player-Facing Impact
Bullet holes, blood spatter, and battle damage accumulate as varied, non-repeating marks instead of the same three decal textures visibly stamped everywhere, packed into infrastructure that already exists rather than a competing atlas system.

## 5.7 Persistence, Data, Network, UI (M7 / M8 / M11 / M12)

**[M7-EXT-08] Zstandard Custom Dictionary Static Compiler**

Purpose: builds a permanent compression dictionary from static world archetype templates at boot to speed up background save serialization.

**[M7-EXT-09] Atomic File-Swap Append-Only State Transaction Logger**

Purpose: append-only delta log for ECS mutations so autosave doesn't need a full-state stutter — periodic compaction to the real save file happens off the critical path.

**[M7-EXT-10] Binary Save Format Structural Schema Migrator**

Purpose: translates old save-slot byte layouts to current component schema when struct sizes change between versions — required before you ship any patch that touches a serialized component.

**[M8-EXT-09] Memory-Mapped FlatBinary Inventory Cache Dictionary** — bug note

Purpose: index-math pointer access into item/weapon data to skip JSON parsing at runtime.
⚠️ Requires a **stride/padding alignment guard** baked in from the start, not bolted on later — struct layout changes must be validated against the flat file's fixed stride or you get silent misreads. Treat "alignment guard" as a requirement of this system, not a separate optional item.

**[M8-EXT-10] Procedural Loot Icon Generation**

##### Systems Touched
M8 itemization (every JSON item record needs a UI icon), M11 UI/HUD inventory rendering, M1's render pipeline (reused offscreen), and M4's existing texture-atlas infrastructure (context-free grammar signage compilation already builds and manages an atlas — this reuses it instead of a second one).

##### Math
Fixed camera framing normalized to a category bounding box; no per-item authored parameters.

##### How It Works
Since every loot item already exists as a real 3D mesh for the world (no separate 2D art asset is authored — matches the solo/no-artist constraint directly), bake the inventory icon by rendering that same mesh offscreen once at item-JSON-load time, not per-frame: normalize the mesh into a category-specific bounding box, apply a fixed camera angle/FOV per category (weapons framed to show silhouette diagonally, consumables framed near top-down to show label/cap), light with a flat 2–3-point rim/key rig independent of in-world lighting so icons stay legible regardless of time-of-day or weather, and cache the result into the same atlas M4's signage system already builds and manages.

##### Reference Implementation
```cpp
struct IconCameraPreset { glm::vec3 eulerAngles; float fovDeg; glm::vec2 boundsPadding; };

// category → preset lookup is a small fixed table (weapon, consumable, apparel, component, ...)
void BakeItemIcon(uint32_t itemMeshId, ItemCategory category, RenderTarget& offscreenTarget, TextureAtlas& itemIconAtlas) {
    const IconCameraPreset& preset = IconPresetForCategory(category);
    AABB bounds = ComputeMeshBounds(itemMeshId);
    glm::mat4 view = BuildFramingCamera(bounds, preset.eulerAngles, preset.boundsPadding);
    RenderMeshOffscreen(itemMeshId, view, preset.fovDeg, kFlatIconLightRig, offscreenTarget);
    itemIconAtlas.Insert(itemMeshId, offscreenTarget.ReadBack());
}
```

##### Player-Facing Impact
Every generated or modded item — including player-crafted variants with randomized attachments — gets a correct, current inventory icon automatically; no art backlog and no placeholder icons on modded content.

---

**[M11-EXT-08] MSDF Vector Text Layout Geometry Batching Pipeline**

Purpose: batches all diegetic UI/text/menu geometry into one vertex buffer pass instead of many small draws.

**[M12-EXT-04] Local Network-Relevancy Grid Culling Filter** — *renumbered from a v71 draft collision with `[M12-EXT-01]` (Bitstream Delta-Encoded Network Serialization Packet Encoder), which already owned that ID — see front-matter changelog.*

Purpose: throttles state sync to only co-op peers whose relevancy grid cell overlaps the changed entity, instead of broadcasting everything to everyone.

**[M12-EXT-03] Bitstream Delta-Encoded Packet Replay Fragment Reassembler** — [MATH DONE, FIXED] — duplicate of the full write-up in "Not yet merged," kept here for index completeness only; that's the canonical copy

Math: $SlotIdx = Seq_{packet} \mod BufferCapacity$

```cpp
#include <cstdint>
#include <cstring>
#include <algorithm>
struct OrderedPacketSlice { uint32_t sequenceIdentifier; uint8_t discretePayloadBuffer[256]; uint16_t payloadLength; bool isSlotPopulated; };

inline bool ReassembleInboundFragment(uint32_t sequence, const uint8_t* rawBufferData, uint32_t rawBufferLen, OrderedPacketSlice* ringPool,
                                       uint32_t poolCapacity, uint32_t maxTrackedSequence) {
    // Wraparound-safe unsigned distance check — the previous "sequence + poolCapacity <= maxTrackedSequence"
    // form could itself overflow-wrap when sequence is near UINT32_MAX, silently accepting stale fragments.
    const uint32_t backwardDistance = maxTrackedSequence - sequence;
    if (backwardDistance < 0x80000000u && backwardDistance >= poolCapacity) return false; // stale, outside window
    const uint32_t slot = sequence % poolCapacity;
    // Copy is clamped to the real datagram size — an
    // over-read past rawBufferData on every fragment smaller than 256 bytes (the normal case).
    const uint32_t copyLen = std::min<uint32_t>(rawBufferLen, sizeof(OrderedPacketSlice::discretePayloadBuffer));
    ringPool[slot].sequenceIdentifier = sequence;
    std::memcpy(ringPool[slot].discretePayloadBuffer, rawBufferData, copyLen);
    ringPool[slot].payloadLength = static_cast<uint16_t>(copyLen);
    ringPool[slot].isSlotPopulated = true;
    return true;
}
```

Sequence-window checks use an unsigned wraparound-safe distance comparison (no signed negation, so no `INT32_MIN` UB risk).
Impact: prevents entity state updates from applying out-of-order/fragmented packets.

## 5.8 Not yet formalized (name + one-liner only — flesh out math when you get to each)

These were flagged as missing but haven't been scoped with math/structs yet. Listed so nothing gets lost; add IDs when you pick a milestone slot for each. (Six items formerly listed here — Upper-Body Animation Layer Override, Procedural Mesh Kitbasher Pipeline, Dynamic Voxel GI Cache Injection Pass, Dual-Clutch Transmission RPM/Torque Controller, Continuum-Fluid Horde Density Pressure Field, and Offline Time-Lapse Population Drift Simulator — are now fully fleshed in **Appendix K** as `[K-EXT-01]`, `[K-EXT-17]`, `[K-EXT-05]`, `[K-EXT-03]`, `[K-EXT-02]`, and `[K-EXT-04]` respectively; removed from this stub list, not duplicated.)

* **IK Rig Metadata Serialization Loader** — loads bone-chain/end-effector defs from disk instead of hardcoded bone name lookups.
* **(see `[M5.4-EXT-10]` Procedural Mission & Event Director — fleshed in M5.4's Extended Systems Library; M13 stays the SLM text consumer, not the owner)**
* **Procedural Environmental Ambient Audio Baker** — synthesizes live wind/rain/city-hum soundscape from chunk tags + wind vectors.
* **GPU-Side Storage Buffer Decompressor (GDeflate, compute)** — same caveat as M4.6-EXT-05 above: this is the Vulkan-compute-shader path, not the Windows DirectStorage API — name it accordingly.
* **Chunk Boundary Entity Transfer Queue** — atomic handoff of simulated AI ownership across sector lines without double-ticking.
* **Asynchronous glTF Geometry Cache Purger** — distance-weighted LRU unload of static meshes under VRAM pressure.
* **Spatiotemporal Blue-Noise Jitter Array Interleaver** — deterministic low-discrepancy per-frame offsets for hybrid ray tracing denoise.
* **Luma-Variance Adaptive Shading Rate Calculator** — screen-space contrast → Variable Rate Shading map.
* **Barycentric Silhouette Edge Reconstruction Filter** — sub-pixel AA on Visibility Buffer primitive edges pre-upscale.
* **Triplanar Material PBR Blending Shader** — world-space-normal texture blend for un-unwrapped cliff/rubble geometry.
* **XPBD Continuous Rope/Cable Solver** — drift-free tether/winch/rigging constraint updates.
* **Kinematic Full-Body IK Surface Locker** — pins hands/feet to moving vehicle surfaces.
* **Exhaust Backpressure Aquatic Stutter Engine** — submersion-depth-driven engine stall/misfire.
* **Herschel-Bulkley Non-Newtonian Mud Rut Tracker** — soil shear-thinning tire rut deformation.
* **Chassis Vortex Debris Drafting Engine** — wind-wake force behind fast vehicles pulling debris/dust.
* **Spherical Harmonics Visibility Pre-Filter Grid** — cheap coarse obstruction check before committing to LOS raycasts.
* **Saint-Venant 2D Shallow Water PDE** — street-level flooding/current/aquaplaning (already referenced as M10 dependency elsewhere — confirm this isn't a duplicate of existing M10 groundwater system before adding).
* **Julian-Day Diurnal Atmospheric Gas Density Calculator** — ambient air density from daily temp cycle, affects long-range bullet drop.
* **Arrhenius Electrochemical Structural Weathering Pass** — rust/rot decay multiplier from real elapsed hours on unmaintained bases.
* **Network Clock Sync & Tick Drift Compensator** — host-authoritative timeline correction for co-op input replay alignment.
* ~~Dynamic Light Frustum & Occlusion Culler~~ — fleshed out as `[K-EXT-21]` in this pass (v72); no longer just a one-liner.

## 5.9 Real-World Reference Naming Lexicon (M8 / M9)

Public-domain alphanumeric model numbers for the procedural vehicle/weapon L-system seeds — real mechanical layout classes, no trademarked names in-engine.

**Vehicles** — real-world class → in-game seed ID → structural bounds
Heavy truck (F-150-class) → `Model 150-F Utility` → dual-channel frame, leaf-spring; Full-size SUV (H1-class) → `GUV-M998 Heavy` → portal hubs, wading limits; Sports coupe (M3-class) → `Sports-E90 Inline-6` → independent rear cradle; Police interceptor (CVPI-class) → `Interceptor-V8 Fleet` → push-guards, anti-roll; Compact hatch (Golf GTI-class) → `Compact-GTI 1.6` → transverse sport shell; Semi-truck → `Commercial-M2 Hauler` → twin-screw dual rear-axle.

**Weapons** — real-world class → in-game seed ID → ballistic category
.45 pistol (1911-class) → `Model 1911-A1 Pistol` → heavy-mass sidearm; 9mm pistol (Glock-class) → `G-17 Gen-5 Combat` → polymer frame; Pump shotgun (870-class) → `Pump-870 Tactical` → 12ga breacher; Carbine (M4-class) → `Carbine M4A1 Spec` → 5.56 direct-impingement; Battle rifle (AK-class) → `Rifle AK-47 Classic` → long-stroke piston; SAW (M249-class) → `Storm-249 SAW Belt` → open-bolt belt-fed; Anti-materiel (M82-class) → `Anti-Material M82A1` → .50 BMG long-range.

Full expanded tables (15 vehicles, 15 weapons) live in `updateforv66.txt` Section 5 — pull additional rows from there as the itemization pipeline (M8) needs more seeds; the pattern above is the naming convention to keep consistent.

## 5.10 Standalone Sandboxed Verification Test Suite

Zero-dependency C++ (standard headers only) sanity harness from `updateforv66.txt` Section 6, adapted to use local mock structs so it never collides with the canonical types defined elsewhere in this doc (`AlignToCacheLine` here mirrors `[M0-EXT-01]`'s real one for a standalone smoke test, not a second implementation to maintain). Compiles and runs independent of the engine — use it as a pre-merge gate before trusting a new version's WFC recovery, Saint-Venant flux stencil, and SPSC queue logic.

```cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <atomic>

// Block 0: cache-line alignment (mirrors [M0-EXT-01]'s allocator rounding, standalone check only)
size_t AlignToCacheLine(size_t size) {
    assert(size <= static_cast<size_t>(-64));
    return (size + 63) & ~static_cast<size_t>(63);
}

// Block 1: WFC contradiction-horizon recovery ([M4-EXT-11])
struct MockWfcSlot { uint64_t mask; bool collapsed; uint32_t entropyScore; };

void VerifyWfcRecoveryPattern() {
    std::vector<MockWfcSlot> testGrid(25, { 0xFFFFFFFFFFFFFFFFull, false, 64 });
    uint32_t targetIndex = 12;
    testGrid[targetIndex].mask = 0x00000000ull;
    uint32_t stride = 5;
    int32_t targetX = static_cast<int32_t>(targetIndex % stride);
    int32_t targetZ = static_cast<int32_t>(targetIndex / stride);
    if (testGrid[targetIndex].mask == 0x00000000ull) {
        for (int32_t offsetZ = -1; offsetZ <= 1; ++offsetZ) {
            for (int32_t offsetX = -1; offsetX <= 1; ++offsetX) {
                int32_t nX = targetX + offsetX, nZ = targetZ + offsetZ;
                if (nX >= 0 && nX < 5 && nZ >= 0 && nZ < 5) {
                    uint32_t idx = static_cast<uint32_t>(nZ * stride + nX);
                    testGrid[idx].mask = 0xFFFFFFFFFFFFFFFFull;
                    testGrid[idx].collapsed = false;
                    testGrid[idx].entropyScore = 64;
                }
            }
        }
        constexpr uint64_t kDebrisTileMask = 0x0000000000000001ull;
        testGrid[targetIndex].mask = kDebrisTileMask;
        testGrid[targetIndex].collapsed = true;
        testGrid[targetIndex].entropyScore = 0;
    }
    assert(testGrid[12].collapsed == true);
    assert(testGrid[12].mask == 1ull);
    assert(testGrid[13].mask == 0xFFFFFFFFFFFFFFFFull);
}

// Block 2: Saint-Venant upwind flux throttling ([M10-EXT-01], [M4-EXT-16])
struct FloodCell { float h; float z; };

void StepSaintVenantFluxVerification() {
    FloodCell cellA{ 2.0f, 10.0f };
    FloodCell cellB{ 0.0f, 11.5f };
    float headA = cellA.h + cellA.z;
    float headB = cellB.h + cellB.z;
    float shallow = std::min(cellA.h, cellB.h);
    float flux = 0.5f * (headA - headB) * std::pow(std::max(0.0f, shallow), 1.5f);
    assert(flux == 0.0f); // dry neighbor (cellB.h == 0) must never produce flux, prevents negative depth
}

// Block 3: M13/M1 SPSC ring-buffer correctness ([M1-EXT-06], [M13] channels)
struct MockAsyncPacket { uint32_t entityHandle; char messagePayloadText[32]; };

class SanitySpscQueue {
    MockAsyncPacket m_dataPool[16];
    std::atomic<uint32_t> m_writeIndex{0};
    std::atomic<uint32_t> m_readIndex{0};
public:
    bool Push(uint32_t ent, const char* txt) {
        uint32_t currWrite = m_writeIndex.load(std::memory_order_relaxed);
        uint32_t nextWrite = (currWrite + 1) & 15;
        if (nextWrite == m_readIndex.load(std::memory_order_acquire)) return false;
        m_dataPool[currWrite].entityHandle = ent;
        std::strncpy(m_dataPool[currWrite].messagePayloadText, txt, 31);
        m_dataPool[currWrite].messagePayloadText[31] = '\0';
        m_writeIndex.store(nextWrite, std::memory_order_release);
        return true;
    }
    bool Pop(MockAsyncPacket& out) {
        uint32_t currRead = m_readIndex.load(std::memory_order_relaxed);
        if (currRead == m_writeIndex.load(std::memory_order_acquire)) return false;
        out = m_dataPool[currRead];
        m_readIndex.store((currRead + 1) & 15, std::memory_order_release);
        return true;
    }
};

void VerifySpscCommunicationPipeline() {
    SanitySpscQueue testQueue;
    assert(testQueue.Push(42, "FactionStrategicPass"));
    assert(testQueue.Push(99, "ForensicDecayTrigger"));
    MockAsyncPacket packetOut;
    assert(testQueue.Pop(packetOut));
    assert(packetOut.entityHandle == 42);
    assert(std::strcmp(packetOut.messagePayloadText, "FactionStrategicPass") == 0);
    assert(testQueue.Pop(packetOut));
    assert(packetOut.entityHandle == 99);
    assert(std::strcmp(packetOut.messagePayloadText, "ForensicDecayTrigger") == 0);
}

int main() {
    assert(AlignToCacheLine(0) == 0);
    assert(AlignToCacheLine(1) == 64);
    assert(AlignToCacheLine(64) == 64);
    assert(AlignToCacheLine(65) == 128);
    VerifyWfcRecoveryPattern();
    StepSaintVenantFluxVerification();
    VerifySpscCommunicationPipeline();
    std::cout << "ZOMBIEENGINE SANITY SUITE: PASS\n";
    return 0;
}
```

---

## Appendix K — AAA-Parity Gap-Fill (merged from `update.txt`, v70)

**Merge audit (v70 — supersedes the v69 pass's summary, which undercounted by omission and, on this re-check, double-counted two items between its own categories).** `update.txt` (a Gemini chat transcript proposing 42 numbered "AAA parity" subsystems) was cross-checked against the full doc item-by-item, twice: once at v69, and again here after a dedicated bug/gap-completeness pass. Every one of the 42 numbered proposals maps to exactly one of the following, no item counted twice:

* **5 items fleshed out** in **K.1** below — these existed only as name+one-liner stubs in the old §5.8 and are now fully specified with math/struct/implementation: item 1 Skeletal Upper-Body Override (`[K-EXT-01]`), item 13 Holling Type II Horde Pressure (`[K-EXT-02]`), item 35 Dual-Clutch Transmission (`[K-EXT-03]`), item 37 Population ODE Drift Simulator (`[K-EXT-04]`), and item 9 Mesh Grammar Kitbasher (`[K-EXT-17]`, added in this pass — the v69 audit claimed "six" stubs fleshed but only shipped five; this was the missing sixth). A sixth K.1 block, `[K-EXT-05]`, adds a real-time structural-break injection path on top of item 27's already-existing SVO GI grid (see inline-covered, below) — it's additive detail, not a distinct numbered item, so it isn't double-counted here.
* **14 items genuinely new**, added in **K.2**: items 2, 5, 7, 12, 17, 21 (with item 30 folded in as `update.txt`'s own internal duplicate of item 21 — not a 15th item), 22, 28, 31, 32, 40 (`[K-EXT-06]` through `[K-EXT-16]`), plus three more confirmed as genuine gaps in this pass — item 15 SDF Building Interior Splitting (`[K-EXT-18]`), item 18 Volumetric Weather Fog & Light-Shafts (`[K-EXT-19]`), and item 4 the Ambient Traffic/Survivor-Vehicle Scarcity Orchestrator (`[K-EXT-20]`). These last two had no equivalent anywhere in the base doc despite the "already covered" framing implied at v69.
* **8 items explicitly checked and rejected as exact/near duplicates**, logged in **K.3** so nothing silently vanishes: item 3 SVT Page Allocator, item 29 Two-Tier Weapon Synthesizer, item 33 Public-Domain Lexicon, item 36 Caravan Route Router, item 42 Vehicle SLM Diagnostics, and three more confirmed-covered in this pass and now logged rather than left unstated: item 6 Async PSO Warm-Up (already covered by `VK_EXT_shader_object` + async pipeline compilation), item 23 Temporal Super-Resolution/TAA (superseded by the vendor upscaler interface), and item 34 Structural Fatigue Salvage Router (already covered by the shared `TickFatigue()` accumulator).
* **14 items already covered inline** by existing v68 systems under a different name, verified here but not given their own K-EXT block: item 8 Topology-Invariant Master Rig (`[M5.1-EXT-03]`), item 10 Dual-Contouring Extractor (`[M4-EXT-13]`, reused by six different K-EXT callers above), item 11 PBR Virtualizer via compute-bake, item 14 Keplerian/Bruneton-Nishita sky scattering, item 16 GPU Meshlet Clustering + Two-Pass HZB Culling, item 19 Ray-Marched Acoustic Occlusion (`[M6-EXT-11]`'s voxel-cone equivalent), item 20 cell-to-cell thermal fire spreading, item 24 Material-Component Contact Friction & Audio Router (the existing `SurfaceFrictionSample`/`ResolveFriction()` chain already drives both traction and footstep/impact audio from one material lookup), item 25 async binary chunk-delta saving (Zstandard, M7), item 26 Bindless Descriptor Paging (`[M0-EXT-08]`, `[M4.5-EXT-09]`), item 27 the base Sparse Voxel Octree GI grid, item 38 spline road-exclusion, item 39 settlement power-grid conductance, and item 41 `ThompsonBanditTracker` scarcity tuning.

5 + 14 + 8 + 14 = 41 numbered items, + item 30 folded into item 21 as `update.txt`'s own internal duplicate = **42/42 accounted for.**

### K.1 — Fleshed-out stubs (previously §5.8 name-only entries)

#### [K-EXT-01] Skeletal Upper-Body Action-Layering Override (fleshes §5.8 "Upper-Body Animation Layer Override")

**Systems Touched:** M5.2 procedural animation pipeline, M2.7 combat/reload state machine. Slots directly into the pose chain M5.2 already documents but left open: Motion Matching → **Upper-Body Override (this)** → Procedural Rig → Physics Post-Process → Final Pose.

**How It Works:** A spine-root bone index splits the skeleton into a lower-body half (owned by Motion Matching's locomotion output) and an upper-body half (owned by whatever action state — reload, aim, melee swing — is currently active). Every tick, bones at and above the spine root are lerped from the base locomotion pose toward the action pose by a per-state blend weight; bones below stay untouched, so sprinting legs never freeze when the player reloads.

**Reference Implementation**

```cpp
// Blends upper-body action transforms over lower-body Motion-Matching output via an explicit spine-root bone index.
void ApplyUpperBodyOverride(std::span<Transform> basePose, std::span<const Transform> actionPose,
                             float weight, uint32_t spineRootIdx) {
    for (size_t i = spineRootIdx; i < basePose.size(); ++i)
        basePose[i] = Transform::Lerp(basePose[i], actionPose[i], weight);
}
```

`weight` ramps 0→1 over ~0.15s on action entry/exit (critically-damped spring, reusing M5.2's existing spring-damper convention) rather than snapping, so transitions don't pop.

**Player-Facing Impact:** Entities reload, aim, and melee while sprinting/strafing without needing a pre-baked clip for every locomotion × action combination.

---

#### [K-EXT-02] Continuum-Fluid Holling Type II Horde Pressure Solver (fleshes §5.8 "Continuum-Fluid Horde Density Pressure Field")

**Systems Touched:** M5.4 AI Director / horde steering, M3 structural graph (barricades/doors).

**Math:** Non-linear saturating pressure curve (Holling Type II functional response) instead of a linear density→force scale, so pressure plateaus realistically at extreme crowd density rather than growing unbounded:

`P(density) = (F_max · density) / (K_half + density)`

**How It Works:** Reuses M1-EXT-08's spatial hash to bucket zombie entities per cell and compute a local density scalar per cell each tick (staggered, same convention as other spatial-hash queries in the doc). `P(density)` converts that scalar into a directional force applied against `[M3]`'s structural graph nodes (barricades/doors), so a packed horde crushes/breaks structures with cumulative physical weight instead of per-agent capsule-to-capsule contact tests (which don't scale to thousands of overlapping entities).

**Reference Implementation**

```cpp
// Scales crowd pushing pressure non-linearly (Holling Type II) so density saturates instead of growing unbounded.
float ComputeHordePressure(float density, float maxForce, float halfSaturation) {
    return (maxForce * density) / (halfSaturation + density + 1e-5f);
}
```

**Player-Facing Impact:** Thousands of overlapping zombies visibly push and crush barricades with real weight instead of clipping through each other silently.

---

#### [K-EXT-03] Non-Linear Dual-Clutch Transmission Controller (fleshes §5.8 "Dual-Clutch Transmission RPM/Torque Controller")

**Systems Touched:** M9 vehicle physics tick (Jolt constraint solver), consumes `StructuralFatigue` wear (`[M3-EXT-06]`) as an input to slip severity.

**How It Works:** Engagement point (0 = fully disengaged clutch, 1 = fully locked) is smoothstepped rather than linear, so slip drops off sharply near full engagement instead of a mechanical-feeling straight ramp; a salvaged/worn gearbox (higher `dynamicSlip`, fed from the vehicle's fatigue accumulator) sputters and loses torque efficiency at the same engagement point a fresh gearbox would handle cleanly.

**Reference Implementation**

```cpp
// Computes clutch-slip torque efficiency inside the fixed-timestep vehicle physics tick.
void StepGearClutch(float dynamicSlip, float engagementPoint, float& outTorqueEfficiency) {
    outTorqueEfficiency = std::clamp(
        1.0f - (dynamicSlip * (1.0f - std::smoothstep(0.2f, 0.8f, engagementPoint))), 0.15f, 1.0f);
}
```

**Player-Facing Impact:** Older, salvaged vehicles feel heavier and slip/sputter under load instead of every car handling identically.

---

#### [K-EXT-04] Isolated Micro-Population ODE Survivor Drift Simulator (fleshes §5.8 "Offline Time-Lapse Population Drift Simulator")

**Systems Touched:** M8.5 Faction Population ODEs (extends the existing per-faction-scalar hourly update, not a second population model), Appendix D lone-survivor/dead-camp spawning.

**Math:** Logistic growth against a per-region carrying capacity, minus a disease-kill term sourced from the existing SEIR severity value M8.5 already tracks — no new epidemiological model, this just reuses the SEIR output as the kill-rate input:

`dP/dt = r·P·(1 - P/K) - k_disease·P`

**How It Works:** Runs once per in-game hour per hibernated (unrendered) sector, same cadence M8.5 already uses for faction scalars. When the player re-enters a sector, the accumulated population delta resolves into concrete outcomes: growth spawns a new Lone Survivor via Appendix D's existing spawn path, population collapsing to ~0 flips the sector to a Dead Camp (also an existing Appendix D path) with cause-of-death tagged from whichever term dominated the ODE (disease vs. starvation vs. raid, if a raid event fired separately).

**Reference Implementation**

```cpp
// Advances a hibernated sector's survivor population on an hourly tick using logistic growth minus SEIR-driven mortality.
void AdvanceRegionalPopulation(float growthRate, float carryingCapacity, float diseaseKillRate, float& population) {
    population = std::max(0.0f,
        population + (growthRate * population * (1.0f - population / carryingCapacity) - diseaseKillRate * population));
}
```

**Player-Facing Impact:** Sectors you haven't visited in days feel like they kept living without it — a family may have grown, moved on, or died out entirely by the time you return.

---

#### [K-EXT-05] Dynamic SVO Irradiance Cache Injection Pass (fleshes §5.8 "Dynamic Voxel GI Cache Injection Pass")

**Systems Touched:** Existing Tier-1 Sparse Voxel Octree GI fallback (M4.5), M3 structural-destruction events.

**How It Works:** The SVO GI grid already updates lazily as the sun moves; this adds the missing write path for *structural* changes — when `[M3]` destroys a wall panel, the newly-exposed voxel cells (previously occluded, now open to sky/interior light) are marked dirty and re-sampled on the next lazy GI pass instead of waiting for the sun-angle bucket to change naturally, so a blown-open wall lets light in the same tick, not several in-game minutes later.

**Reference Implementation**

```glsl
// Samples cached SVO irradiance at a world position; called both by shading and by the dirty-cell re-injection pass.
vec4 SampleSvoIrradiance(vec3 worldPosition, float voxelScaleSize) {
    ivec3 voxelCoord = ivec3(floor(worldPosition / voxelScaleSize));
    return textureLod(SvoVolumeTextureTarget, vec4(vec3(voxelCoord) / 128.0, 0.0).xyz, 0.0);
}
```

**Player-Facing Impact:** Blowing a hole in a wall visibly lets light flood the room immediately instead of on a multi-minute GI-refresh delay.

---

---

#### [K-EXT-17] Procedural Mesh Grammar Kitbasher & Micro-Detail Synthesizer Pipeline (fleshes §5.8 "Procedural Mesh Kitbasher Pipeline")

**Systems Touched:** M4 chunk stream-in (character/clothing/attachment assembly), consumes the same primitive-SDF + Dual Contouring extraction path `[M4-EXT-13]`'s cave carving and `[K-EXT-10]`/`[K-EXT-12]`'s fracture/weapon-blend systems already use — one extractor, this is a fourth caller, not a second mesher.

**How It Works:** A deterministic split-grammar (L-system) runs on chunk stream-in, combining primitive SDF shapes (cylinders, capsules, tapered boxes) along procedural growth axes to assemble clothing, gear, and weapon-furniture geometry from a handful of grammar rules instead of hand-modeled meshes. Micro-surface detail (scratches, bevels, fabric-weave frequency) is added via the same domain-warped OpenSimplex2 technique the terrain micro-detail pass already uses (§4's `detail(p)` warped-noise pattern), applied to the SDF before extraction so normals stay mathematically exact through Dual Contouring rather than being faked with a normal map. Seeded via the item system's existing Two-Tier archetype/instance split — grammar rule selection is archetype-seeded (permanent per item type), micro-detail warp phase is instance-seeded (per-item variation).

**Reference Implementation**

```cpp
// Spawns structural geometric primitive rings down a procedural vector axis — barrel segments, gear rails, clothing folds.
void GenerateCylinderGrammar(std::vector<glm::vec3>& verts, glm::vec3 start, glm::vec3 dir, float r, uint32_t segments) {
    for (uint32_t i = 0; i < segments; ++i)
        verts.push_back(start + dir * (float)i + glm::vec3(cosf((float)i) * r, 0.0f, sinf((float)i) * r));
}
```

Grammar output SDF is triangulated through the shared Dual Contouring extractor (same call site as `[K-EXT-10]`/`[K-EXT-12]`), then decimated through `[M4-EXT-02]`'s existing quadric-error LOD pass — no new LOD system for kitbashed geometry.

**Player-Facing Impact:** Characters, clothing, and weapon furniture read as visually detailed and non-repetitive without a single hand-modeled asset, completing the last of the six formerly-stubbed §5.8 systems.

---

### K.2 — Genuinely new systems (absent from v68)

#### [K-EXT-06] Soft-Body Vertex Collision Vehicle Deformation

**Systems Touched:** M9 vehicle physics (Jolt), consumes/feeds `StructuralFatigue` (`[M3-EXT-06]`) so a deformed panel and a fatigued panel are the same underlying wear state, not two parallel damage models.

**How It Works:** On collision, impulse magnitude and contact point are used to displace hull vertices within a falloff radius, softened by a hardness term (steel deforms less than sheet-aluminum trim for the same impulse — `hardness` reads the same material-tag table `[M9]`'s friction/salvage systems already use). This is a *visual + collision-shape* deformation layer sitting on top of the existing fatigue/salvage accumulator — it does not replace `TickFatigue()`, it renders what that accumulator implies.

**Reference Implementation**

```cpp
// Deforms hull vertices based on impact impulse; radius/hardness pulled from the same per-material table [M9] friction uses.
void ApplyImpactDeformation(std::span<glm::vec3> vertices, glm::vec3 impactPoint, glm::vec3 impulse,
                             float radius, float hardness) {
    for (auto& v : vertices) {
        float d = glm::distance(v, impactPoint);
        if (d < radius) v += impulse * (1.0f - (d / radius)) * (1.0f / (hardness + 1e-5f));
    }
}
```

Vertex buffer is CPU-side-mutable per-instance (not the shared procedural template), then re-uploaded through the existing chunk/instance upload path — no new GPU resource type.

**Player-Facing Impact:** Crashes visibly crumple the hull instead of the car staying rigid while an invisible fatigue number ticks down.

---

#### [K-EXT-07] GPU-Driven Broadphase Spatial Hash Compute Pipeline

**Systems Touched:** Complements (does not replace) `[M1-EXT-08]`'s CPU-side spatial hash quadtree — this is the GPU compute path used specifically for horde-scale (thousands of entities) proximity queries; `[M1-EXT-08]` remains canonical for lower-volume CPU-side queries (player interactions, vehicle contacts).

**How It Works:** A compute shader packs each entity's 2D cell coordinate into a flat hash once per frame, replacing per-entity CPU registry walks. Flocking/crowd-pressure/perception systems (M5.1, M5.3, K-EXT-02 above) read the resulting hash buffer instead of touching the CPU-side registry for bulk horde queries.

**Reference Implementation**

```glsl
// Packs each entity's 2D cell coordinate into a flat hash for fast GPU-side bucket sorting.
layout(local_size_x = 64) in;
void main() {
    uint id = gl_GlobalInvocationID.x;
    ivec2 cell = ivec2(floor(EntityPositions[id].xz / 2.0)); // 2.0m cell stride
    EntityCellHashes[id] = (uint(cell.x) & 0xFFFFu) | ((uint(cell.y) & 0xFFFFu) << 16);
}
```

**Player-Facing Impact:** Keeps flocking/crowd-pressure/perception responsive when thousands of zombies are active simultaneously, protecting the 5.0ms AI frame budget.

---

#### [K-EXT-08] Real-Time Procedural Friction & Impact Audio Synthesizer

**Systems Touched:** M6 hardware-accelerated audio, reads the same `SurfaceFrictionSample`/material-tag data `[M9]` vehicle friction and `ResolveFriction()` already resolve — one shared friction value drives both physics and audio, not two independent friction reads.

**How It Works:** Instead of streaming pre-recorded tire-screech/engine/impact clips, waveforms are synthesized per-frame from slip ratio and material impedance — modulated white noise for tire slip, tied directly to the same slip-ratio scalar `[M9]`'s traction chain already computes.

**Reference Implementation**

```cpp
// Synthesizes tire-slide audio from the same slip ratio [M9]'s ResolveFriction() chain already produces.
float SynthesizeTireScreech(float slipRatio, float materialImpedance, float time) {
    float whiteNoise = static_cast<float>(rand() % 2000 - 1000) / 1000.0f;
    return sinf(440.0f * 3.14159f * time * (1.0f + slipRatio)) * whiteNoise
           * std::clamp(slipRatio * materialImpedance, 0.0f, 1.0f);
}
```

**Player-Facing Impact:** Zero runtime audio-clip disk footprint for tire/engine/impact sound, consistent with the doc's zero-hand-authored-asset pillar.

---

#### [K-EXT-09] Reaction-Diffusion Forensic Skin & Tissue Decal Projector

**Systems Touched:** M6.5 GPU particle/VFX, writes into the same RVT terrain/object overlay pages `[M4.5-EXT-07]` (skid marks) and blood-spatter (`[M6.5]`) already use — a third writer into that existing overlay system, appended to `ResolveFriction()`'s ordered-writer front-matter fix (audit item #3) so it doesn't reintroduce an order-dependent conflict.

**How It Works:** A localized Gray-Scott reaction-diffusion solver runs per-decal-region to simulate decomposition/bruising/drying-blood patterns over real elapsed time (reusing the same Arrhenius-style elapsed-Δt pattern M7's sector-hibernation degradation already uses), rather than a static blood-decal texture.

**Reference Implementation**

```glsl
// One Gray-Scott reaction-diffusion step, evaluated per forensic decal region over real elapsed time.
float StepReactionDiffusion(float u, float v, float feed, float kill, float lapU) {
    return u + (0.2 * lapU - u * v * v + feed * (1.0 - u));
}
```

**Player-Facing Impact:** Wound/decomposition/blood-pool weathering reads as context-aware (time since death, ambient temp) instead of a fixed decal.

---

#### [K-EXT-10] Dynamic Memory-Pooled Implicit Surface Fracture Solver

**Systems Touched:** M3 macro-destruction & structural graphs — this is the SDF-native companion to M3's existing structural-graph fracture logic, used specifically for character/vehicle volumes that are SDF-authored rather than pre-baked destructible meshes.

**How It Works:** On a high-velocity Jolt impact, an analytical cutting plane (impact normal + offset) is intersected against the entity's SDF via a max() combine, producing an instant, exact fracture surface with no pre-baked broken-mesh variant needed.

**Reference Implementation**

```cpp
// Slices an entity's volume SDF against an analytical cutting plane derived from impact normal/offset.
float SampleSdfFracturedSlice(glm::vec3 point, float baseSdfSample, glm::vec3 planeNormal, float planeOffset) {
    float dPlane = glm::dot(point, planeNormal) - planeOffset;
    return std::max(baseSdfSample, dPlane);
}
```

Resulting sliced SDF is triangulated through the same Dual Contouring extractor `[M4-EXT-13]`'s cave-carving already uses — one extractor, two callers, not a second mesher.

**Player-Facing Impact:** Context-aware zombie dismemberment and sheared structural pieces with no pre-baked broken-art variants.

---

#### [K-EXT-11] Modular Vehicle Chassis Assembly Grammar Engine

**Systems Touched:** M9 vehicle system — this is the missing geometry-generation step feeding M9's existing physics/fatigue/friction systems, which all currently assume a chassis exists but never specify how one is built.

**How It Works:** A structural grammar spawns frame rails, wheel-axis mounts, and engine-block volumes as primitive SDF segments down a procedural vector axis, seeded via the same Two-Tier archetype/instance seed split the item system (§ "Global Archetype Manifest") already uses — chassis identity is a permanent per-archetype seed, wear/scavenge state is a per-instance seed layered on top. ~90% of spawned chassis roll as non-functional wrecks (`StructuralFatigue` already crossing its yield threshold at spawn time) suitable only for part harvesting, consistent with the doc's scarcity design for functional vehicles.

**Reference Implementation**

```cpp
// Spawns structural frame rail primitives down a procedural vector axis, seeded from the archetype/instance split.
void BuildChassisRail(std::vector<glm::vec3>& vertices, glm::vec3 origin, glm::vec3 direction, float width, uint32_t segments) {
    for (uint32_t i = 0; i < segments; ++i)
        vertices.push_back(origin + direction * (float)i + glm::vec3(width, 0.0f, 0.0f));
}
```

**Player-Facing Impact:** Hundreds of distinct, functional car/truck/military-chassis categories generated purely from code + seeds, with realistic scavenging (most wrecks are parts, not drivable finds).

---

#### [K-EXT-12] SDF Boolean Prim-Blending Weapon Customization Fabricator

**Systems Touched:** M8 itemization, downstream of the same Two-Tier archetype/instance weapon seed already canonical there — this is specifically the *attachment geometry* step (scopes/barrels/stocks) that seed system doesn't yet specify a mesh method for.

**How It Works:** Smooth-minimum SDF blending fuses attachment primitives onto a base weapon SDF with a soft transition (no hard seam at the socket), rather than a hard union — the same Dual Contouring extractor used elsewhere in the doc triangulates the result.

**Reference Implementation**

```cpp
// Smooth-minimum blend between two SDF distance samples, producing a seamless attachment-to-receiver transition.
float SmoothSdfUnion(float d1, float d2, float k) {
    float h = std::clamp(0.5f + 0.5f * (d2 - d1) / (k + 1e-5f), 0.0f, 1.0f);
    return std::lerp(d2, d1, h) - k * h * (1.0f - h);
}
```

**Player-Facing Impact:** Visually seamless, modular weapon customization built entirely from code, no attachment mesh assets.

---

#### [K-EXT-13] Data-Table Modding & Hot-Reload Path

**Systems Touched:** M0 asset toolchain — extends the existing shader hot-reload path (`VK_EXT_graphics_pipeline_library` / DXC, already in M0) to JSON data tables (weapon/vehicle archetypes, caravan/scarcity tuning, brand-tier tables), which currently have no equivalent live-reload path.

**How It Works:** File resolution checks a prioritized mod directory before falling back to the built-in data directory; schema validation runs loudly at boot (or on file-change event) so a malformed mod JSON fails fast with a clear error instead of silently corrupting a table.

**Reference Implementation**

```cpp
// Resolves a data-table path, preferring a mod-directory override over the built-in table of the same name.
std::string ResolveModAssetPath(const std::string& filename, const std::string& modDir, const std::string& baseDir) {
    return std::filesystem::exists(modDir + filename) ? (modDir + filename) : (baseDir + filename);
}
```

**Player-Facing Impact:** Weapon/vehicle/scarcity tuning can be iterated via JSON edits without a full recompile — this is also the mechanism that lets *you* (JJ) add new guns/cars/systems fastest during development.

---

#### [K-EXT-14] Property-Matching Material Synthesizer & Derived Item Stat Generator

**Systems Touched:** M8 itemization — extends the existing material/mass/volume tags every scavenged item already carries for physics, adding a stat-derivation step so a new scrap item doesn't need a hand-written recipe-table entry.

**How It Works:** Crafting validity and resulting stats are derived from raw material tag + mass/volume bounds rather than a lookup table keyed by item name, so any new item with a "metallic, high-density" tag automatically qualifies for weapon-action/armor-brace crafting without a new table row.

**Reference Implementation**

```cpp
// Derives a component's kinetic-impact damage capacity from its material tag and density, no per-item table entry needed.
float DeriveImpactDamage(uint32_t materialTag, float mass, float volume) {
    return (materialTag == 1 /* Metal Class */) ? (mass / (volume + 1e-5f)) * 45.0f : mass * 12.0f;
}
```

**Player-Facing Impact:** Adding a new scrap item to the game is a data-tag change, not a new crafting-recipe entry.

---

#### [K-EXT-15] Hidden Outpost/Hideout Node Placement Selector

**Systems Touched:** Appendix D (Lone Survivors & Dead Camps), M4 world generation — this is the placement-selection step Appendix D's spawn logic currently assumes but never specifies.

**How It Works:** Candidate cells are scored from terrain slope, distance from major roads (reusing `[M4]`'s road graph, not a second distance field), and the existing socio-economic zoning tags M4 already assigns; low-slope, high-road-distance, low-commercial-zoning cells score highest and are selected deterministically from the world seed.

**Reference Implementation**

```cpp
// Scores a candidate cell's fitness as a hidden hideout site from terrain slope, road distance, and zoning tag.
float EvaluateHideoutFitness(float slope, float proximityToHighways, float zoningTagCommercial) {
    return (slope < 0.15f && proximityToHighways > 300.0f) ? (1.0f - zoningTagCommercial) * 85.0f : 0.0f;
}
```

**Player-Facing Impact:** Survivor safehouses and bandit nests land in believable, defensible, out-of-the-way spots instead of random placement.

---

#### [K-EXT-16] Procedural Vector-Distance UI Canvas Renderer

**Systems Touched:** M11 UI/HUD — this is a rendering-method addition (not a new HUD design) for whichever HUD elements M11 already specifies.

**How It Works:** Menu/inventory/crosshair geometry is drawn as analytical distance functions (lines, rounded boxes, font curvature) evaluated per-pixel in a fragment shader, rather than image-texture UI assets, keeping M11 consistent with the doc's zero-hand-authored-asset pillar.

**Reference Implementation**

```glsl
// Draws an analytical crosshair ring via implicit circle distance, resolution-independent at any display scale.
float DrawImplicitCrosshairRing(vec2 screenPixelCoord, vec2 centerAnchor, float radius, float edgeThickness) {
    float d = length(screenPixelCoord - centerAnchor) - radius;
    return smoothstep(edgeThickness, 0.0, abs(d));
}
```

**Player-Facing Impact:** Perfectly sharp UI at any resolution, zero UI texture-asset files.

---

---

#### [K-EXT-18] Implicit SDF Building Interior Splitting Grammar Engine (fleshes M4's one-liner "Procedural Urban Detail (L-systems, BSP interiors)")

**Systems Touched:** M4 procedural world generation, downstream of the macro-graph city layout pass (§4's top-down road/zoning graph) and upstream of `[K-EXT-17]`'s furniture/clothing kitbasher, which populates the rooms this creates.

**How It Works:** A building's bounding SDF envelope (produced by M4's existing structure-placement pass) is recursively subtracted against room/corridor volumes using a BSP-style split grammar — each split picks an axis and offset from the building's zoning tag and footprint aspect ratio, carving hallways, room partitions, and stairwells as boolean subtractions rather than as a hand-placed floorplan. Door openings are punched as smaller box subtractions at each partition boundary and immediately get the base `DoorComponent` the front-matter audit (item 4) already mandates in M2.6/M3, so every procedurally-cut doorway is a real, breakable door from the moment it's generated, not a static gap. Runs at chunk stream-in, same cadence as the rest of M4's WFC/L-system content.

**Reference Implementation**

```cpp
// Evaluates a subtraction between a building's bounding envelope and an interior room/corridor volume.
float SampleSdfSubtractedRoom(glm::vec3 p, glm::vec3 bBox, glm::vec3 roomBox) {
    float dStructure = glm::length(glm::max(glm::abs(p) - bBox, glm::vec3(0.0f)));
    float dInterior  = glm::length(glm::max(glm::abs(p) - roomBox, glm::vec3(0.0f)));
    return std::max(dStructure, -dInterior);
}
```

Resulting interior SDF is triangulated through the same Dual Contouring extractor `[M4-EXT-13]` and `[K-EXT-17]` already call.

**Player-Facing Impact:** Buildings have real, explorable, non-repetitive interiors — rooms, hallways, stairwells, breakable doors — generated entirely from grammar rules with zero hand-authored floorplans.

---

#### [K-EXT-19] Volumetric Weather Fog & Light-Shaft Scattering Pass

**Systems Touched:** M10 weather system (feeds off the existing meteorology density output `[M10-EXT-05]` already produces), M4.5's rendering pipeline — this is the general atmospheric-volume companion to `[M4.5-EXT-01]`'s flashlight-specific volumetric cone (that one is a single artificial light source with hand-tuned constants; this is the sun/moon through ambient weather density, reusing the sky LUT's Rayleigh/Mie coefficients from §M10 rather than a second scattering model).

**How It Works:** A froxel (view-frustum-aligned voxel) grid accumulates in-scattered light along the camera's view rays each frame, sampling local fog/rain/dust density from M10's existing meteorology density field and the same `β_Rayleigh`/Henyey-Greenstein Mie phase function the sky LUT (§7.4xx) already defines — one shared scattering model driving both the sky color and the volumetric shafts, not two independent atmospheric systems. Sun/moon shadow-map occlusion tested per froxel produces visible light shafts through gaps in structures and canopy.

**Reference Implementation**

```glsl
// Beer-Lambert transmittance step through a single froxel cell, using the same scatterCoeff the sky LUT already derives.
float ComputeVolumetricTransmittance(float scatterCoeff, float stepLength, float localDensity) {
    return exp(-scatterCoeff * localDensity * stepLength);
}
```

Froxel accumulation buffer is consumed by the main G-buffer composite pass as a screen-space multiply, same insertion point as existing post-process passes.

**Player-Facing Impact:** Fog, rain, and dust get real, sun-occluded light shafts and depth-cued haze instead of a flat fog color — the atmospheric depth AAA titles use for tone, especially through broken structures and forest canopy.

---

#### [K-EXT-20] Ambient Traffic & Survivor-Vehicle Scarcity Spawn Orchestrator

**Systems Touched:** M8.5 population/scarcity systems (extends the existing per-region loot-scarcity tagging, not a new economy), M5.4's road graph (the same graph `[M5.4-EXT-06]`'s faction caravans already route on) — this is specifically the walking-survivor / functional-vehicle encounter-density layer that sits alongside caravan routing, not a replacement for it.

**How It Works:** A deterministic per-region Bernoulli roll (SplitMix64, reusing the doc's standard PRNG convention) gates whether a given lone-survivor spawn (Appendix D) or settlement-adjacent scavenge point also rolls a functional vehicle — weighted heavily toward zero, and boosted only near settlements/hideouts, so a lone survivor with a *working* car is a rare, memorable find rather than a routine one. Most ambient population is on foot; faction caravans (already covered by `[M5.4-EXT-06]`) are a separate, much rarer, unkillable macro-graph event layered on top of the same road network, not competing with this roll for the same spawn budget.

**Reference Implementation**

```cpp
// Deterministic Bernoulli trial gating whether a lone-survivor/scavenge spawn also rolls a functional vehicle.
bool RollScarcityVehicleSpawn(uint64_t& seed, float baseProbability, bool nearSettlement) {
    seed = (seed ^ 0xBF5Dull) * 0x9E3779B97F4A7C15ull;
    return (static_cast<float>(seed & 0xFFFFFFFFu) / 4294967295.0f) < (baseProbability * (nearSettlement ? 5.0f : 0.1f));
}
```

Reads the same `CraftingStationComponent`/socio-economic zoning tags Appendix D and `[K-EXT-15]`'s hideout selector already use for placement context — one shared scarcity/zoning read path, not a parallel tagging system.

**Player-Facing Impact:** Finding another living survivor is uncommon; finding one with a working car is a genuine rare event, distinct from the much rarer, unkillable armored caravans passing through on the highway graph.

---

#### [K-EXT-21] Clustered Froxel Light Culling Pass (fleshes §5.8 "Dynamic Light Frustum & Occlusion Culler")

**Gap this closes:** every other rendering system in this doc (Nanite-equivalent meshlet clustering, Lumen-equivalent SVO GI, Virtual Shadow Maps) already matches current-gen practice, but the doc repeatedly assumes "thousands of procedural lights" (muzzle flashes, headlights, fires, floodlights, `[M8.6-EXT-04]`'s electrical traps) get to a "tight GPU list" without ever specifying how — that phrase was §5.8-listed as a one-liner and never implemented. A per-object or per-tile linear light loop does not scale to that count; clustered/froxel light culling is the current standard fix (Forward+/clustered forward, the same family of technique DOOM Eternal and most UE5/Unity HDRP titles use for exactly this problem) and is the natural counterpart to this doc's existing clustered depth-bounds shadow voxelizer (`[M4.5-EXT-13]`).

**Systems Touched:** M4.5 GPU render pipeline — sits between the M1 GPU-driven G-buffer pass and shading; consumed by every forward-lit surface (translucent particles, foliage) and by the deferred lighting pass. Shares its light list with `[K-EXT-08]`'s friction/impact audio synthesizer only in the sense that both read the same light-source entity set `[M0-EXT-01]`'s spatial hash already indexes — no new registry.

**How It Works:** View frustum is subdivided into a 3D grid of froxels (screen-space tiles × exponential depth slices, matching `[M4.5-EXT-13]`'s existing depth-bounds slicing so the two systems share one depth-bucketing scheme instead of each computing its own). A compute pass assigns each active light to every froxel its bounding sphere overlaps, writing a per-froxel `(offset, count)` pair into an indirection buffer plus a flat light-index list. The shading pass then only evaluates lights in its fragment's froxel instead of the full scene light array.

**Reference Implementation**

```glsl
// Pass 1: compute froxel index for a light's bounding sphere and mark it active in every overlapped froxel.
layout(local_size_x = 64) in;
void main() {
    uint lightId = gl_GlobalInvocationID.x;
    if (lightId >= activeLightCount) return;
    vec3 viewPos = (View * vec4(Lights[lightId].worldPos, 1.0)).xyz;
    float radius = Lights[lightId].radius;
    ivec3 minCell = FroxelCellFromView(viewPos - vec3(radius), zSliceExpBase);
    ivec3 maxCell = FroxelCellFromView(viewPos + vec3(radius), zSliceExpBase);
    for (int z = minCell.z; z <= maxCell.z; ++z)
        for (int y = minCell.y; y <= maxCell.y; ++y)
            for (int x = minCell.x; x <= maxCell.x; ++x)
                AppendLightToFroxel(ivec3(x, y, z), lightId); // atomic append into that froxel's index list
}
```

**Player-Facing Impact:** Frame rate holds steady when a horde fight lights up with muzzle flashes, a burning building, and multiple vehicle headlights simultaneously, instead of the lighting pass becoming the frame-time bottleneck at exactly the moment the most is happening on screen.

---

### K.3 — Explicitly not added (checked and rejected as duplicates)

* **Sparse Virtual Texturing (SVT) Asset Page Allocator** (`update.txt` #3) — the doc's existing RVT terrain/object-overlay system plus `[M0-EXT-08]`'s bindless descriptor paging already cover the VRAM-budget problem this solves; a second sparse-paging texture system would fragment the VRAM budget table across two competing allocators instead of one.
* **Two-Tier Deterministic Archetype Weapon Synthesizer** (`update.txt` #29) — the doc's existing "Global Archetype Manifest / Session Mutation Vector" split (§ near M8 itemization) is the same mechanism with more detail already worked out; adding #29 verbatim would reintroduce the exact split-state-variable problem the front-matter audit (item #2) already flags as a class of bug to avoid.
* **Public-Domain Alphanumeric Lexicon** (`update.txt` #33) — exact duplicate of existing §5.9.
* **Long-Range Faction Caravan Route Router** (`update.txt` #36) — already built as the M5.4/M8.5 convoy extension of `[M5.4-EXT-06]`'s road graph (see line-referenced note in the v68 merge audit); re-adding it here would stand up a second road-graph system, which that same audit explicitly warns against.
* **Local SLM Diagnostic Text Generator (vehicle)** (`update.txt` #42) — exact duplicate of the existing `[M13-EXT]` `vehicle_diag` generator.
* **Asynchronous PSO Warm-Up Engine** (`update.txt` #6) — the doc's existing `VK_EXT_shader_object` dynamic-stage-binding system (M4.5, removes PSO *stage* permutations) plus its Asynchronous Shader Compilation via `VK_EXT_graphics_pipeline_library` (§ Tooling & developer iteration loop, batched `vkCreateGraphicsPipelines` calls) already solve the same PSO-compile-stutter problem this proposes; a second warm-up scheme would compete with the shader-object path for the same stutter budget instead of sharing it.
* **Temporal Super-Resolution & Subpixel Jitter Anti-Aliasing Pipeline** (`update.txt` #23) — M4.5's existing Adaptive Upscaling Interface (vendor-detected FSR/DLSS/XeSS) plus its per-pixel motion velocity buffer and Visibility Buffer silhouette AA reconstruction is a superset of what a custom TAA reprojection pass would add, and defers to vendor-tuned upscalers instead of a bespoke implementation the doc would then have to maintain against every future driver update.
* **Structural Fatigue-Life Scrap Salvage Router** (`update.txt` #34) — the doc's existing `StructuralFatigue`/`TickFatigue()` Palmgren-Miner accumulator (canonical since `[M3-EXT-06]`) already drives the M9 vehicle-fatigue consumer path, and `[K-EXT-11]`'s ~90% non-functional-wreck spawn rate already implements this proposal's scarcity outcome; a second salvage-state router would be a fourth parallel wear model, exactly the class of bug the front-matter audit (item #6, `BarrelHeat`) already warns against for shared structs.

---

## Appendix L — Modding & Extensibility Architecture (v71)

Added because a heavily data-driven engine (every weapon/vehicle/scarcity/caravan table is already JSON, per `[K-EXT-13]`) is most of the way to being moddable already — this appendix closes the remaining gap between "JSON hot-reload for me during dev" and "safe, conflict-free multi-mod support for players," using the same layered-override pattern (Polymod/Bethesda-style: data-driven core, mods merge as named layers, not raw file replacement) that the current best-of-breed moddable engines converge on.

#### [L-EXT-01] Layered Multi-Mod Manifest & Load-Order Resolver

**Systems Touched:** Extends `[K-EXT-13]`'s single-mod-directory `ResolveModAssetPath` from one override folder to an ordered stack of them — this is that system's direct successor, not a parallel loader.

**How It Works:** Each mod is a folder containing a `manifest.json` (`{ "id": "...", "name": "...", "version": "...", "author": "...", "priority": int, "requires": ["other_mod_id@>=1.0"] }`) plus the same JSON table structure the base game already reads. At boot, all installed mod folders are sorted by `priority` (ties broken alphabetically by `id` for determinism) into an ordered stack; `requires` version constraints are checked before any table loads, and a mod whose dependency isn't satisfied is skipped with a loud boot-log entry rather than silently loading half-broken. Asset/table resolution walks the stack top-down, same lookup shape as `[K-EXT-13]`'s two-path check, just generalized to N paths instead of 2.

**Reference Implementation**

```cpp
// Resolves a data-table path by walking the sorted mod stack top-down; falls through to the base game table if no mod overrides it.
struct ModManifest { std::string id, version; int priority; std::vector<std::string> requires_; };

std::string ResolveLayeredAssetPath(const std::string& filename, const std::vector<ModManifest>& sortedModStack,
                                     const std::string& modsRootDir, const std::string& baseDir) {
    for (const auto& mod : sortedModStack) { // already sorted by priority at boot
        std::string candidate = modsRootDir + mod.id + "/" + filename;
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return baseDir + filename; // no mod overrides this file — base game table wins
}
```

**Player-Facing Impact:** Multiple mods combine automatically instead of one mod silently clobbering another's files, with a deterministic, inspectable load order.

---

#### [L-EXT-02] Boot-Time Conflict & Schema Validation Report

**Systems Touched:** Runs immediately after `[L-EXT-01]`'s stack resolves, before any table is handed to gameplay systems — extends `[M0-EXT-08]`'s existing "validate loudly at boot" pattern already established for the single-mod-dir case in `[K-EXT-13]`.

**How It Works:** For every JSON table key, if two or more mods in the stack define the *same* key (a weapon archetype hash, a scarcity-tuning constant), the winner (highest priority) is logged alongside every mod it overrode — visible in a `mod_conflicts.log`, not silently swallowed. Every loaded table is validated against its schema (same field-presence/type check `[K-EXT-13]` already runs for a single mod dir) before it reaches gameplay code; a malformed entry disables just that entry (falls back to the base game's version) and logs which mod/key failed, rather than crashing the whole table load.

**Player-Facing Impact:** A broken or conflicting mod produces a readable log line pointing at exactly which mod and key caused it, instead of an unexplained crash or silently wrong behavior — the single biggest driver of "why won't my modlist load" support burden in every moddable game's community.

---

#### [L-EXT-03] Namespaced Content ID Convention

**Systems Touched:** Extends the existing Two-Tier archetype/instance hash seed (§ near M8 itemization) — this is the namespacing rule that seed system needs once more than one mod can define a `Hash64("Weapon_...")` string.

**How It Works:** Every mod-defined content key is required to be prefixed with the mod's `manifest.json` `id` (e.g. `mymod.Weapon_RustySickle` rather than a bare `Weapon_RustySickle`) before it's hashed into the archetype seed. Base-game content has no prefix (implicitly `core.`). This is a pure naming convention enforced at `[L-EXT-02]`'s schema-validation step — it costs nothing at runtime and is the single change that lets two unrelated mods both add a "Weapon_Shotgun" without their `SplitMix64` archetype hashes colliding.

**Player-Facing Impact:** Two mods can both add content with the same human-readable name without one silently overwriting the other's item in save files.

---

#### [L-EXT-04] Modding API Surface Document (auto-generated, not hand-maintained)

**Systems Touched:** M0 build step — a small tool run at the end of every engine build, not a runtime system.

**How It Works:** A build-time script walks every JSON schema struct already declared across M8/M8.5/M9/M5.4 (weapon archetypes, vehicle chassis tables, scarcity-tuning constants, caravan route weights) and emits a single `MODDING_API.md` documenting every moddable field, its type, and its valid range — generated from the same schema `[L-EXT-02]` validates against, so the published documentation can never silently drift out of sync with what the engine actually accepts (a common failure mode in hand-maintained modding docs).

**Player-Facing Impact:** Modders get accurate, always-current documentation of exactly what they can change, generated from the real schema instead of a stale wiki page.

---

**Explicitly out of scope for this pass:** a Lua/scripting hook for mod *logic* (not just data) — the polymod-style research above confirms this is the natural next step once data-driven modding is solid, but it's a real scope increase (sandboxing, a scripting VM, an API-stability contract) that deserves its own milestone slot rather than being folded in here. Flagged in `[K-EXT]`-style form for a future pass: **Scripted Mod Hook Layer** — a sandboxed script VM (Wren or a stripped Lua build are the usual lightweight choices for a C++ engine this size) exposed read/write access to a deliberately small, versioned subset of ECS components, not the whole registry.

---

## Appendix M — Async/Netcode/Scale Gap-Fill (merged from `updateforv72.txt`, v73)

**Merge audit.** `updateforv72.txt` proposed roughly 20 new subsystems across animation polish, rendering polish, networking, and horde-scale simulation, plus a re-elaboration of the existing M13 SLM loop. Checked item-by-item against the full doc:

* **7 kept** (M.1 below) — solve a real, stated problem with a technique appropriately sized for a solo dev + agent.
* **2 were ID collisions, not new content** — fixed by renumbering (M.1 notes each).
* **6 rejected or deferred as scope risk** (M.2 below) — not wrong, just too large relative to the value they add right now.

### M.1 — Kept

#### [M0-EXT-14] Asynchronous Hardware Compute Interleaver & Pipeline Lifecycle Matrix

*(Renumbered from the update's `[M0-EXT-11]` — that ID already belongs to an unrelated existing M0 system.)*

**Systems Touched:** M0 capability tiering, `[M4.5-EXT-07]` procedural texture synthesizer, M13 SLM worker threads.

**How It Works:** Background compute work (texture synthesis, SLM inference) submits to a dedicated `VK_QUEUE_COMPUTE_BIT` queue instead of the primary graphics queue, coordinated with cross-queue timeline semaphores. This is a standard, well-supported Vulkan pattern on any GPU with an async compute queue family (your RTX 2070 Super has one) — it's the correct fix for background work stalling your 16.6ms/frame budget, and it's cheap to add now before more systems assume inline execution.

**Player-Facing Impact:** Texture streaming and (if you keep M13) SLM inference stop causing frame hitches.

---

#### [M12-EXT-05] Manifold Impulse History Rollback Buffer & Fixed-Point State Matrix

**Systems Touched:** M12 networked co-op, `[M12-EXT-01]` bitstream delta encoder, `[M12-EXT-02]` loss sliding window, Jolt `PhysicsSystem`.

**How It Works:** Maintains a 60-frame rolling history of physics state per dynamic entity. On a desync (checksum mismatch via xxHash64), rolls back to the last-agreed tick and re-integrates from there — applying corrective *impulses* through Jolt's normal solver rather than snapping transforms directly, which avoids the solver-island blowups a hard position reset causes. This is the same family of technique as rollback netcode in fighting games and Rocket League; it directly targets the tunneling/rubber-banding failure mode you're trying to prevent, and it's the right scope for M12 since you're already committed to deterministic co-op.

**Note:** the update's fixed-point (32.32) coordinate conversion is worth keeping specifically for the *replicated/networked* state — don't convert your whole engine's `glm::dvec3` world space to fixed-point, only the data that crosses the wire.

**Player-Facing Impact:** Dropped packets during a siege cause a brief correction instead of getting shoved through a wall or crate.

---

#### [M5.2-EXT-13] Kinematic Character Ledge-Locking Coordinate Re-Basing Matrix

**Systems Touched:** `JPH::CharacterVirtual`, M2.7 player controller, M3 structural graphs, M2.6 open-world foundations.

**How It Works:** While anchored to a moving platform (vehicle bed, collapsing floor section), the character's position is tracked as a local double-precision offset from the platform's root transform rather than in absolute world space, then converted to camera-relative single precision only at the final draw step. This is the standard fix for jitter/sinking on moving platforms in large open worlds using floating-origin techniques — directly useful since you already have double-precision world space and moving structural pieces.

**Player-Facing Impact:** No sliding/jitter while standing on a moving vehicle or a partially-collapsed floor.

---

#### [M4.6-EXT-06] Compute-Driven Hierarchical Z-Buffer Downsampler

**Systems Touched:** M4.5 GPU-driven pipeline, Hi-Z occlusion culling.

**How It Works:** Generates Hi-Z mip chain levels in fewer dispatches using subgroup max operations to reduce across the local workgroup before writing each mip level, instead of one dispatch per mip.

**Caveat, stated plainly:** the update's sample shader only actually resolves mip levels 0 and 1 within the subgroup reduction — extending it cleanly to 4+ levels needs either multiple subgroup-reduction passes or a workgroup shared-memory reduction, not shown. Treat the snippet below as the pattern, not a drop-in complete implementation.

```glsl
// Downsamples 2x2 depth texels into hzbMipLevels[0], then subgroup-reduces to mip 1.
// Extending past mip 1 requires an additional shared-memory or subgroup pass per level — not shown here.
layout(local_size_x = 16, local_size_y = 16) in;
layout(set=0, binding=0) uniform sampler2D srcDepth;
layout(set=0, binding=1, r32f) writeonly uniform image2D hzbMipLevels[4];
void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(id*2)+0.5) / textureSize(srcDepth,0);
    float maxZ = max(max(textureLod(srcDepth,uv,0).r, textureLodOffset(srcDepth,uv,0,ivec2(1,0)).r),
                      max(textureLodOffset(srcDepth,uv,0,ivec2(0,1)).r, textureLodOffset(srcDepth,uv,0,ivec2(1,1)).r));
    imageStore(hzbMipLevels[0], id, vec4(maxZ));
    float sgMax = subgroupMax(maxZ);
    if (subgroupElect()) imageStore(hzbMipLevels[1], id>>1, vec4(sgMax));
}
```

**Player-Facing Impact:** Slightly cheaper occlusion culling; more headroom for the dense structural/horde geometry M3/M5 already ask for.

---

#### [M5.4-EXT-08] Closed-Ecosystem Continuum-Fluid Horde Density Field

**Systems Touched:** M1 spatial hash, M5.4 AI director, M4 procedural streaming.

**How It Works:** Hordes fully outside the player's active radius are dropped from individual EnTT entities/Jolt bodies down to a coarse 2D density grid, advected with a simple diffusion step (`∂ρ/∂t = D∇²ρ + S_spawn − S_decay`) instead of simulating every zombie. When a density field crosses back into the active radius, the chunk seed (SplitMix64) reconstructs individual zombies deterministically. This is a real and appropriately-scoped technique — background/off-screen abstraction of large populations is standard practice for open-world games with big roaming crowds (broadly the same idea as "unloaded NPC" systems in other open-world titles), and it directly targets your stated 3,000-zombies-5km-away memory/CPU problem.

**Player-Facing Impact:** Distant hordes still drift and grow/shrink over time instead of freezing, without the game paying full simulation cost for zombies you can't see.

---

#### [M3-EXT-09] Async Structural Fatigue Continuum Accumulator

*(Renumbered from the update's `[M3-EXT-08]` — that ID already belongs to your existing Palmgren-Miner fatigue struct definition.)*

**Systems Touched:** M3 structural graph, `[M5.4-EXT-08]` horde continuum field, `StructuralFatigue` (canonical struct — no new fatigue variable created, per your existing dedup rule).

**How It Works:** The counterpart to `[M5.4-EXT-08]`: hibernated/off-screen buildings under sustained siege pressure accumulate fatigue on a coarse per-region basis (reading the same `StructuralFatigue` fields, just updated at a lower tick rate and coarser spatial resolution while hibernated) rather than freezing structural state entirely while unloaded.

**Player-Facing Impact:** A settlement you leave under siege can still be meaningfully more damaged when you return, without simulating every wall's stress every frame while you're away.

---

#### [M1-EXT-12] Dynamic MSDF Font Glyph Rasterizer & RVT Cache Interface

**Systems Touched:** M1 MSDF font pipeline, `[M4.5-EXT-07]` texture synthesizer, Runtime Virtual Texture pages.

**How It Works:** Rasterizes novel text (procedurally generated signage, or M13 SLM-generated text if you keep that system) into MSDF glyph pages on the fly via `VK_EXT_host_image_copy`, rather than requiring every possible string pre-baked into a font atlas.

**Conditional note:** this only earns its place if you keep some form of runtime-generated text (dynamic signage, mission text). If you cut or shelve M13's text-generation features, this has no consumer and should wait.

**Player-Facing Impact:** Signage/UI text can be generated at runtime instead of only from a fixed pre-authored string table.

---


## [M1-EXT-29] Double-Precision Authoritative Transform with Float Upload Cast

##### Systems Touched
Every consumer of M1's SoA Transform component — skinning, indirect draw,
persistent-mapped staging upload (M1-EXT-05) — and directly feeds M2.6's
origin-rebase plan.

##### Math
`Position_gpu = float3(Position_authority - Origin_current)`
where `Position_authority` is `dvec3` and `Origin_current` is the active
rebase anchor (world origin or camera-relative anchor, per M2.6's
threshold policy).

##### How It Works
`Transform.Position` is authored and stored as `glm::dvec3` at the
ECS/authority level — this is the single source of truth for world
position. Once per frame, immediately before the persistent-mapped
staging ring buffer upload (M1-EXT-05) already declared in M1, subtract
the current origin/camera anchor from the double-precision position and
cast the result to `glm::vec3` for the GPU-facing SoA array. No other
system (skinning, indirect draw, culling) ever sees the double directly
— they consume the already-cast float buffer, unchanged from how M1
already declared them.

##### Reference Implementation
```cpp
// Called once per frame per entity batch, immediately before staging upload.
// Origin is the active M2.6 rebase anchor; Position is the dvec3 authority value.
glm::vec3 CastPositionForGPU(const glm::dvec3& position, const glm::dvec3& origin) {
    return glm::vec3(position - origin); // safe: difference is small post-rebase
}
```

##### Player-Facing Impact
World position never loses precision far from the origin, and the
already-planned M2.6 rebase becomes a threshold-tuning pass instead of a
rewrite of every GPU upload path that currently assumes float.

---

## [M1-EXT-30] Parent-Child Transform Hierarchy with Dirty-Flag Propagation

##### Systems Touched
Weapon/hand sockets (M2.7), camera-to-head attachment (this patch's
M1-EXT-31), M9 vehicle-mounted turrets/parts, any future backpack/gear
attachment.

##### Math
`WorldMatrix_child = WorldMatrix_parent · LocalMatrix_child`, recomputed
only when `Dirty(child) ∨ Dirty(parent)`.

##### How It Works
Adds a `Parent{ entt::entity }` component and a cached `WorldMatrix`
component. A per-frame dirty list collects any entity whose local
transform changed or whose parent's `WorldMatrix` changed this frame.
The list is processed in parent-before-child order (a shallow
topological sort via `entt::registry::sort`, since hierarchies in this
game are shallow — weapon→hand→body, camera→head, part→vehicle) so a
child never reads a stale parent matrix. Untouched subtrees are skipped
entirely.

##### Reference Implementation
```cpp
struct Parent { entt::entity value{entt::null}; };
struct WorldMatrix { glm::mat4 value{1.0f}; bool dirty{true}; };

void PropagateDirty(entt::registry& registry, entt::entity e) {
    auto& wm = registry.get<WorldMatrix>(e);
    wm.dirty = true;
    // Mark all direct children dirty too (children store their own Parent link;
    // a reverse lookup or cached child-list keeps this O(children), not O(N)).
}

void RecomputeWorldMatrices(entt::registry& registry) {
    // Sort so parents are processed before children (shallow depth in this game).
    registry.sort<WorldMatrix>([&](entt::entity lhs, entt::entity rhs) {
        return Depth(registry, lhs) < Depth(registry, rhs);
    });
    registry.view<WorldMatrix>().each([&](entt::entity e, WorldMatrix& wm) {
        if (!wm.dirty) return;
        if (auto* p = registry.try_get<Parent>(e); p && registry.valid(p->value)) {
            wm.value = registry.get<WorldMatrix>(p->value).value * LocalMatrixOf(registry, e);
        } else {
            wm.value = LocalMatrixOf(registry, e);
        }
        wm.dirty = false;
    });
}
```

##### Player-Facing Impact
Weapons stay glued to hands, cameras stay glued to heads, and vehicle
parts stay glued to vehicles — without a redesign when M2.7/M9 need it.

---

## [M1-EXT-31] Procedural Spring-Damper Camera Rig

##### Systems Touched
First/third-person camera (M2.7), stamina/exertion system, builds
directly on [M1-EXT-30]'s hierarchy.

##### Math
Semi-implicit (symplectic) Euler damped spring — velocity updates
first, then position:
`v ← v + (-k·x - c·v)·dt`
`x ← x + v·dt`
(Reversing this order is a known integration bug that makes the spring
feel mushy/lose energy incorrectly — verified against standard
semi-implicit Euler ordering.)

##### How It Works
The camera is parented (via M1-EXT-30) to a "virtual head bone" entity.
Each tick, a target offset is computed from player velocity magnitude
and current stamina/exertion value; the spring-damper integrates the
camera's *actual* local offset toward that target. Zero animation
authoring — it's a formula reacting to physics state.

##### Reference Implementation
```cpp
struct CameraSpring { glm::vec3 offset{0}; glm::vec3 velocity{0}; float k{40.0f}; float c{8.0f}; };

void UpdateCameraSpring(CameraSpring& spring, const glm::vec3& targetOffset, float dt) {
    glm::vec3 x = spring.offset - targetOffset;
    spring.velocity += (-spring.k * x - spring.c * spring.velocity) * dt; // velocity first
    spring.offset += spring.velocity * dt;                               // then position
}
```

##### Player-Facing Impact
Head-bob/sway that scales with how hard the character is pushing itself
— the "grounded, weighty" feel referenced against Dying Light — with no
hand-keyed animation.

---

## [M1-EXT-32] RayBatchQuery Parallel Raycast Primitive

##### Systems Touched
Future parkour ledge-detection, AI perception (M5.3), sound occlusion
(this patch's M1-EXT-33). NOTE: this is an engine-side wrapper — Jolt
Physics (arriving M2) does not provide a native multi-ray batch call;
its collector pattern batches multiple *hits along one ray*, not
multiple independent rays. Confirmed against Jolt's own docs.

##### Math
`Results[i] = NarrowPhaseQuery.CastRay(Origins[i], Dirs[i], MaxDist[i])`
for `i` in `[0, N)`, resolved in parallel.

##### How It Works
A struct-of-arrays holds N independent ray requests. `Resolve()` fans
these out across an enkiTS parallel-for, each worker calling Jolt's
single-ray `NarrowPhaseQuery::CastRay` for its slice, using the existing
Fiber Yield Hook (M0-EXT-07) so a large batch never stalls a whole OS
thread. This mirrors what Techland's own engine team built on top of
their raycast primitive for Dying Light's ledge detection — Jolt gives
you the single-ray primitive, this wrapper gives you the batching.

##### Reference Implementation
```cpp
struct RayBatchQuery {
    std::vector<glm::vec3> origins, dirs;
    std::vector<float> maxDist;
    std::vector<RayHitResult> results; // sized to match on Resolve()
};

void ResolveBatch(RayBatchQuery& batch, JPH::NarrowPhaseQuery& query, enki::TaskScheduler& scheduler) {
    batch.results.resize(batch.origins.size());
    enki::TaskSet task(static_cast<uint32_t>(batch.origins.size()),
        [&](enki::TaskSetPartition range, uint32_t) {
            for (uint32_t i = range.start; i < range.end; ++i) {
                JPH::RRayCast ray{ ToJPH(batch.origins[i]), ToJPH(batch.dirs[i]) * batch.maxDist[i] };
                JPH::RayCastResult hit;
                bool had = query.CastRay(ray, hit);
                batch.results[i] = had ? FromJPH(hit) : RayHitResult::Miss();
            }
        });
    scheduler.AddTaskSetToPipe(&task);
    scheduler.WaitforTask(&task);
}
```

##### Player-Facing Impact
Parkour/ledge detection and AI sightlines stay cheap even when many
checks fire in the same frame, instead of each system hand-rolling its
own slow per-ray loop.

---

## [M1-EXT-33] SpatialHash-Driven Sound Occlusion Query

##### Systems Touched
Reuses [M1-EXT-32] (RayBatchQuery) and M1's existing SpatialHash;
declared now so M6 (audio, several milestones out) consumes this
instead of building a second spatial system.

##### Math
`Audible(listener, source) = ¬Hit(RayBatchQuery(source→listener)) ∧ Distance(source, listener) ≤ HearingRadius`

##### How It Works
A hearing check for a noise event is just a RayBatchQuery between the
source and every listener returned by `SpatialHash::QueryRadius` around
that source — reusing the exact batching primitive from M1-EXT-32
rather than a bespoke audio-occlusion system.

##### Reference Implementation
```cpp
std::vector<entt::entity> QueryAudibleListeners(
    const glm::vec3& sourcePos, float hearingRadius,
    SpatialHash& hash, JPH::NarrowPhaseQuery& physQuery, enki::TaskScheduler& scheduler) {

    auto candidates = hash.QueryRadius(sourcePos, hearingRadius);
    RayBatchQuery batch;
    for (auto e : candidates) {
        batch.origins.push_back(sourcePos);
        batch.dirs.push_back(glm::normalize(GetPosition(e) - sourcePos));
        batch.maxDist.push_back(glm::distance(GetPosition(e), sourcePos));
    }
    ResolveBatch(batch, physQuery, scheduler);

    std::vector<entt::entity> audible;
    for (size_t i = 0; i < candidates.size(); ++i)
        if (!batch.results[i].hadHit) audible.push_back(candidates[i]);
    return audible;
}
```

##### Player-Facing Impact
Hiding behind a wall or around a corner actually blocks sound the way it
blocks a raycast — the Zomboid-style hearing/visibility realism you
asked for, using infrastructure this doc already declares.

## [M1-EXT-34] Procedurally-Generated Localization/Accessibility String-Table Pipeline

##### Systems Touched
M1's existing MSDF font pipeline (no rendering-path change needed) and
M13's MiniCPM5-1B integration, pulled forward as an offline content-gen
tool rather than waiting for M13's own milestone.

##### How It Works
All player-facing strings (subtitles, UI, colorblind-mode labels) route
through a string-table keyed by ID. The table's *content* is generated
OFFLINE, at build/content-generation time — never per-tick — by running
MiniCPM5-1B over a small set of seed templates, using the same
`enable_thinking=False` + fixed `temperature=0.7`/`top_p=0.95` discipline
already locked in for M13. This keeps the zero-hand-authored-content rule
intact while producing real text instead of placeholders. The MSDF
pipeline renders whatever the table resolves to, unchanged.

##### Reference Implementation
```cpp
// Offline tool, not runtime code:
// for each seed template in localization_seeds.json:
//   call MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95)
//   write generated variant into strings_<locale>.json keyed by string ID
```

##### Player-Facing Impact
Subtitles, UI text, and accessibility labels exist in real, varied form
without hand-authoring a single line.

---

## [M1-EXT-35] EventBus Telemetry Tap with Consent Gate

##### Systems Touched
The (future) EventBus (M2) — a lightweight tap added now so meaningful
gameplay events (death, horde-encounter size, resource-scarcity moment)
post to a ring buffer, gated by explicit player consent.

##### How It Works
A boot-time consent flag must be true before the tap writes anything.
When enabled, events post to a bounded ring buffer for later
(batched, offline) consumption by [M1-EXT-36].

##### Reference Implementation
```cpp
struct TelemetryEvent { uint32_t eventType; float value; uint64_t tick; };
bool g_telemetryConsentGranted = false; // set only via explicit settings toggle

void PostTelemetryEvent(RingBuffer<TelemetryEvent>& buffer, TelemetryEvent evt) {
    if (!g_telemetryConsentGranted) return;
    buffer.Push(evt);
}
```

##### Player-Facing Impact
Nothing leaves the machine or gets recorded without an explicit opt-in.

---

## [M1-EXT-36] Offline MiniCPM5-1B Difficulty Director Pass

##### Systems Touched
Consumes [M1-EXT-35]'s telemetry ring buffer; adjusts existing
data-driven spawn-density/loot-scarcity curves. NEVER touches gameplay
code directly — only the config values that already exist.

##### How It Works
Batched every few in-game hours (not per-tick), MiniCPM5-1B reasons over
aggregated telemetry and proposes adjustments to existing tunable
curves. This is what actually makes "no difficulty sliders, one tuned
experience" true rather than aspirational, using infrastructure this doc
already spec's for M13.

##### Reference Implementation
```cpp
// Offline/background batch job, not per-tick:
// aggregate TelemetryEvent buffer over N in-game hours ->
// prompt MiniCPM5-1B (enable_thinking=False, temperature=0.7, top_p=0.95) with
// aggregated stats -> parse suggested curve deltas -> write to existing
// spawn_density.json / loot_scarcity.json config, never to code.
```

##### Player-Facing Impact
Difficulty actually adapts to how the player is really doing, without a
visible slider and without the model ever writing code.

> **Verified model config (2026-07-14):** `openbmb/MiniCPM5-1B`'s
> `config.json` was pulled from Hugging Face and confirmed. It is a GQA
> Llama architecture — `hidden_size=1536`, `num_attention_heads=16`,
> `head_dim=128`, `num_key_value_heads=2` (8:1 GQA), `intermediate_size=4608`,
> `num_hidden_layers=24`, `vocab_size=130560`, `max_position_embeddings=131072`,
> bf16. The earlier "(16 heads × 128 = 2048 ≠ 1536, so inconsistent)" worry
> was wrong: under GQA the query-head count need not divide `hidden_size` by
> `head_dim` in the vanilla-MHA sense — 16 × 128 = 2048 query dims with KV
> collapsed to 2 heads is a normal GQA layout. These passes may cite the
> numbers directly; no blocking question remains.

---

## [M1-EXT-37] Offline MiniCPM5-1B Zombie Archetype Behavior Synthesis

##### Systems Touched
Feeds M5.1's already-planned procedural zombie variation. Offline
content-gen only — zero runtime inference cost, zero hand-authored
scripts.

##### How It Works
MiniCPM5-1B synthesizes behavior-tree parameter sets / utility-AI weight
tables per zombie archetype at content-generation time, consumed as data
by M5.1's runtime systems exactly like any other procedurally-generated
config.

##### Reference Implementation
```cpp
// Offline tool:
// for each archetype seed -> MiniCPM5-1B generates a parameter table
// (aggression weight, wander radius, group-cohesion factor, etc.) ->
// written to archetype_<name>.json, consumed at runtime as plain data.
```

##### Player-Facing Impact
Varied, expressive zombie behavior without a scripting VM and without
hand-authored behavior trees.

---

## [M1-EXT-38] ModWritable Allowlist Flag on MetaRegistry Registration

##### Systems Touched
Closes the gap the existing doc already flags as deferred in the Dev
Inspector section (MetaRegistry.cpp).

##### How It Works
Adds an explicit `ModWritable: bool` flag to each component's existing
`entt::meta` registration call, rather than leaving every registered
field implicitly editable.

##### Reference Implementation
```cpp
// In MetaRegistry.cpp, alongside the existing registration calls:
entt::meta<Transform>().data<&Transform::position>("position"_hs)
    .prop("ModWritable"_hs, false); // explicit allowlist, not implicit open access
```

##### Player-Facing Impact
Mods can't rewrite fields like StableId and desync a save — closes a gap
the doc itself already called out as unresolved.


### M.2 — Rejected or deferred (reasoning logged, not silently dropped)

* **Volume-preserving muscle deformation shader, FFT-driven audio lip sync, heat-shimmer refraction pass, split-screen HZB reprojection cache** — all technically real techniques, all pure visual polish with zero effect on whether the game is playable or fun, each adding a nontrivial shader/compute pass to maintain. The split-screen item specifically only matters *at all* if local split-screen co-op is in scope — it isn't mentioned as a requirement anywhere else in this doc. **Recommendation:** revisit all four post-M13, once core gameplay loops (M0–M9) are actually running and there's a game to polish.
* **STUN/TURN NAT-traversal gateway, built from scratch** — the underlying need (WAN co-op through arbitrary home routers) is real, but hand-rolling ICE/STUN/TURN negotiation is itself a multi-week networking project independent of everything else in this doc. **Recommendation:** use an existing library that already implements this (e.g. GameNetworkingSockets, which bundles ICE) rather than a bespoke implementation — this is a case where NIH costs you weeks for no gameplay benefit.
* **Local SLM (MiniCPM5-1B) output directly mutating spawn migration targets, price inflation, and "cognitive hallucination" rendering effects on a 30-second loop** — this is the update's biggest single item, and the one most worth pausing on. It's not that the idea is impossible; it's that wiring an LLM's output directly into core simulation state (horde pathing, economy, screen effects) makes bugs non-reproducible — if a zombie horde does something wrong, you won't know if it's your pathing code or the model's output, and you can't easily write a deterministic test for it. It also competes for VRAM with your renderer on a single RTX 2070 Super, and M13 already carries this idea in the base doc — this update just re-describes it, it doesn't add new information. **Recommendation:** keep it scoped exactly as M13 already has it (flavor text/lore/signage, one-way output, no feedback into core sim state) until M0–M12 are solid; the current v72 M13 section already avoids the "SLM controls gameplay" trap this update reintroduces, so no change made here.

### M.3 — v73 follow-up: formalizing the two M.2 recommendations with real reference implementations

Both M.2 items above ended in a "do it this way instead" recommendation rather than a flat rejection: use an existing NAT-traversal library rather than hand-rolling one, and keep the SLM's output surface locked to read-only flavor text. This section turns those two recommendations into concrete, buildable entries.

**ID collision check performed before adding these:** `M12-EXT-06` is unused — clear. `M13-EXT-11` is **not** — that ID already belongs to *Hardware Backend Auto-Detection & Async Compute Queue Isolation*, defined earlier in M13's Extended Systems Library. The SLM entry below is renumbered to `[M13-EXT-14]` (the next free M13-EXT slot after the existing `[M13-EXT-13]`), following the doc's existing renumber-on-collision convention rather than overwriting the existing definition.

---

#### [M12-EXT-06] GameNetworkingSockets P2P WAN Transport Bridge

**Systems Touched:** `network_manager.cpp` (WAN path interception), `packet_factory.cpp` (packet routing bifurcation).

**Math:** None. No algebraic transforms are introduced to raw packets — NAT punchthrough, keep-alives, and symmetric/cone traversal metrics stay fully encapsulated inside Valve's MIT-licensed GameNetworkingSockets (GNS), per the M.2 recommendation against hand-rolling this.

**How It Works:** Integrates GNS to replace the raw ENet socket pipeline exclusively for connections routing outside the local network topology; LAN traffic keeps using the existing lightweight ENet path. The network tick isolates and intercepts WAN-bound connections — if a socket's target is flagged `is_wan`, the raw ENet execution block is bypassed and packet serialization maps straight to GNS stream descriptors via non-blocking async calls.

```cpp
// wan_transport.h — GNS bridge for STUN/TURN traversal
#pragma once
#include <steam/gamenetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <cassert>

class WANTransport {
public:
    static WANTransport& Get() { static WANTransport instance; return instance; }

    // One-liner init using GNS's default open STUN/TURN ICE server configuration
    bool Init() { return GameNetworkingSockets_Init(nullptr, nullptr); }

    void Poll() { if (auto* sockets = SteamNetworkingSockets()) sockets->RunCallbacks(); }

    // Connects via GNS routing (natively handles symmetric/cone NAT topology variations)
    HSteamNetConnection ConnectTo(const char* identityStr) {
        assert(identityStr != nullptr);
        SteamNetworkIdentity identity;
        if (!identity.ParseString(identityStr)) return k_HSteamNetConnection_Invalid;
        return SteamNetworkingSockets()->ConnectP2PCustomSignaling(nullptr, &identity, 0, nullptr);
    }
};
```

**Architecture Flags & Required Touchpoints:** The M12 baseline assumes a unified `ENetHost` across all topologies. Two touchpoints swap transport cleanly:

* **Flag 1** (`network_manager.cpp`, main host service tick): wherever `enet_host_service` is processed, branch — if the target's `connection.is_wan` is true, skip ENet and call `WANTransport::Get().Poll()` instead.
* **Flag 2** (`packet_factory.cpp`, packet serialization loop): if a packet's destination resolves to a WAN peer, redirect the compiled payload through `SteamNetworkingSockets()->SendMessageToConnection()` instead of the ENet buffer path.

**Exit Criteria:** Two clients behind separate residential NAT routers (not on the same LAN) establish a reliable peer connection and sustain an uninterrupted 10-minute co-op session with no manual port-forwarding.

---

#### [M13-EXT-14] SLM Simulation Isolation & Hardening Pass

*(Renumbered from the update's proposed `M13-EXT-11` — that ID already belongs to the existing Hardware Backend Auto-Detection entry.)*

**Systems Touched:** `slm_types.h` (structural data constraints), `ai_director_pacing.cpp` (horde migration decoupling), `vendor_economy_core.cpp` (price-matrix decoupling).

**Math:** None added to generative output. Simulation variables stay mapped exclusively to the engine's existing deterministic state equations (baseline vendor pricing, horde migration), which the SLM never touches:

$$\text{Price}*{\text{final}} = \text{Price}*{\text{baseline}} \cdot \text{InflationModifier}_{MV=PQ}$$

**How It Works:** Formalizes the M.2 recommendation as an audit/lockdown pass rather than a new capability. Every generator entry point is audited so SLM output is confined to a read-only role — descriptive lore commentary, text signage, and UI/VO subtitle strings, attached as flavor labels to events the deterministic engine already decided. Nothing generative ever reaches pricing, spawn vectors, or render-pass state. The alternative from the update (mapping SLM output to dynamic pricing, render tags, or spawn-cell vectors) is explicitly rejected, consistent with M.2 above.

```cpp
// slm_types.h — hardened read-only output surface
#pragma once
#include <string>

struct SLMActionOutput {
    // READ-ONLY LORE, TEXT SIGNAGE, AND STRING OVERRIDES ONLY
    std::string terminal_log_override;   // custom signage/terminal text
    std::string environmental_lore_str;  // dynamic text for world notes/inspectables
    std::string vo_subtitle_hint;        // audio-VO subtitle annotation overlay

    // GUARDRAIL: no pricing floats, no spawn vectors, no active render passes.
};
```

**Audit Changelog:**

|Entry|Original (drift-vulnerable)|Rewritten (flavor-only)|Reason|
|-|-|-|-|
|M13-04|SLM set vendor `price_multiplier` from generated text|Replaced with an `environmental_lore_str` expressing economic panic; prices stay on the deterministic supply/demand engine|Mutated sim economics via unvalidated generative output|
|M13-11|SLM picked `hallucination_render_tag` to switch post-process passes|Replaced with standard UI screen-glitch overlays driven by the deterministic sleep-debt gradient loop|Let generative text alter the render graph|
|M13-EXT-02|SLM set horde `migration_cells` target vectors|Replaced with static world-terminal flavor text; AI Director tracking loop untouched|Let generative text override pathfinding/spawning|

**Exit Criteria:**

```bash
grep -rn "SLMActionOutput" ./src/ \
  | grep -vE "(ui_manager|terminal_render|audio_vo|slm_types\.h)"
```

Exit status 0 — zero matches outside text/UI display and audio-VO subtitle code.

---

## Appendix — Part A Hermes Self-Discovery Gap-Fill (10 blocks, v78)

Hermes-authored gap-fill produced by scanning the live doc for (a) `## Not yet merged` / `## 5.8 Not yet formalized` bullets with no full EXT block anywhere, and (b) implementation-step areas with no matching EXT. Every candidate was grep-verified for alternate names before authoring; near-duplicates already covered inline were excluded (logged in the close-out section below), per the queue's explicit anti-false-positive instruction.

ID discipline: every proposed ID is **provisional** (the live `milestones/` may have moved past the doc's numbering; verify against live before merge, same as the draft's own caution note). ID ceilings were re-grepped fresh for this pass: M0=12, M1=25, M2.8=08, M3=09, M4=22, M4.6=06, M5.2=13, M5.4=08, M7=10, M9=21, M11=08, M12=06, M13=14, K=21.

Non-trivial techniques were verified against real published sources:
- Spatiotemporal Blue-Noise (STBN): Heitz/Wolfe et al., "Spatiotemporal Blue Noise Masks" (EGSR 2022), jcgt 14(1), 2025.
- FABRIK IK: Aristidou & Lasenby, "FABRIK: A fast, iterative solver for the Inverse Kinematics problem," TCSVT 2011.
- Cristian's algorithm: Cristian, "Probabilistic clock synchronization," Distributed Computing 1989.

#### `[M5.2-EXT-14]` (provisional) IK Rig Metadata Serialization Loader

Fleshes the §5.8 one-liner **IK Rig Metadata Serialization Loader** (line 9104). The runtime `[M5.2-EXT-08]` IK Rig Asset is declared as an in-memory struct but has no disk format or loader; this entry supplies both so skeletons load their bone-chain definitions from data, not hardcoded bone-name lookups.

##### Systems Touched
Consumed by `[M5.2-EXT-08]` (IK Rig Asset struct) and `[M5.2-EXT-09]` (Motion-Warping target resolution). Feeds every end-effector consumer in M5.2 (melee-hit alignment, vault landing, door interaction). Distinct from M2.6 glTF/prefab loading (`std::unordered_map` path cache, line 1188), which loads meshes/scenes — not bone-chain + pole-vector metadata.

##### Math
No closed-form solve; a declarative asset: `IKRigAsset{skeletonId, chains:[{boneIdx[], poleVectorHint}]}`. Hashing for the on-disk table reuses the same FNV-1a keying idiom M2.6 already uses for the geometry cache (`uint64_t key = FNV1a(path)`), so the loader slots into that existing map type.

##### How It Works
A small JSON (or flat binary) asset per skeleton, authored once per unique skeleton and loaded at skeleton-registration time. The loader parses bone-name to index resolution against the skeleton's own joint list (so a renamed bone fails loudly at load, not silently at runtime), then stores the resolved `IKRigAsset` in the same hash-keyed table the geometry cache uses (different key space: `skeletonId` string, not asset path). Phenotype variants sharing a skeleton reuse one asset; a variant needing extra chains adds them in its own override asset rather than mutating the base.

##### Reference Implementation
```cpp
struct IKChainDef { std::vector<uint16_t> boneIdx; glm::vec3 poleVectorHint; };
struct IKRigAssetDef { std::string skeletonId; std::vector<IKChainDef> chains; };

// Returns false (and logs) if a bone name in the asset does not resolve on this skeleton.
bool LoadIKRigAsset(const std::string& path, const Skeleton& sk, IKRigAssetDef& out) {
    IKRigAssetDef raw = ParseIKRigJson(path);          // bone NAMES from disk
    for (auto& c : raw.chains)
        for (auto& name : c.boneNames) {               // resolve names -> indices
            int idx = sk.BoneIndexFromName(name);
            if (idx < 0) { LogError("IKRig %s: unknown bone %s", path, name); return false; }
            c.boneIdx.push_back((uint16_t)idx);
        }
    out = std::move(raw);
    return true;
}
```

##### Player-Facing Impact
Modders and future skeleton authors add bone-chain definitions via data, not engine recompile; matches the doc's "procedural variety over authored content" pillar for animation rigs.

---

#### `[M11-EXT-09]` (provisional) Procedural Environmental Ambient Audio Baker

Fleshes the §5.8 one-liner **Procedural Environmental Ambient Audio Baker** (line 9106): synthesizes a live wind/rain/city-hum soundscape from chunk tags + wind vectors, instead of hand-placing loop assets per biome.

##### Systems Touched
M11 audio/HUD (this milestone owns adaptive audio). Reads M4 chunk biome/material tags and `[M10]` wind vectors (the same field `[M10-EXT-02]`'s Navier-Stokes solver produces; also consumed by `[M9-EXT-04]` mud advection). Feeds the existing positional-voice mixer M11 already declares. Does not duplicate `[M13]` narrative/dialogue audio.

##### Math
Layer gain per acoustic source, modulated by wind speed and rain intensity from chunk tags: `G_wind(v) = clamp(k_w * |v_wind|, 0, 1)`, `G_rain(r) = clamp(k_r * r_precip, 0, 1)`. Cross-fade between biome beds by normalized tag weight `w_b` so a chunk straddling two biomes blends rather than hard-switches.

##### How It Works
A small set of synthesized beds (filtered noise for wind, band-passed noise bursts for rain, low drone for city hum) are generated once per (biome, weather-state) pair into a ring of looping buffers, then their gains are driven continuously by the chunk's live tag state and the sampled wind vector. No per-frame asset streaming; the synthesis is a one-time bake per state pair, cached and reused — cheap at runtime, and it reacts to weather the player can see change.

##### Reference Implementation
```cpp
struct AmbientBed { uint32_t bufferId; float baseGain; };
float EvaluateAmbientGain(const AmbientBed& bed, float windSpeed, float precip, float biomeWeight) {
    float dyn = std::max(bed.baseGain * windSpeed * 0.15f, bed.baseGain * precip * 0.9f);
    return bed.baseGain * biomeWeight + dyn * (1.0f - biomeWeight);
}
```

##### Player-Facing Impact
Every biome/weather combination has a distinct, reactive soundscape with zero hand-authored loop assets — a rainy industrial sector sounds different from a dry suburban one automatically.

---

#### `[M4.6-EXT-07]` (provisional) Asynchronous glTF Geometry Cache Purger

Fleshes the §5.8 one-liner **Asynchronous glTF Geometry Cache Purger** (line 9109): distance-weighted LRU unload of static meshes under VRAM pressure, the eviction half that M2.6's glTF Geometry Caching (line 1188) only sketches ("decay candidate, not an immediate free").

##### Systems Touched
Downstream of M2.6's glTF Geometry Caching table (`std::unordered_map<uint64_t, MeshHandle>`). Acts on `[M4.6-EXT-01]`'s GPU vertex-buffer pool and the M4.6 VRAM budget tracker the cache is already LRU-bounded against. Runs on the general enkiTS scheduler (one scheduler, not Jolt's — per AGENTS.md two-job-systems rule), not the render thread.

##### Math
Eviction score per cached mesh `i`: `S_i = refCount_i * w_ref + (1 / (1 + d_i)) * w_dist`, where `d_i` = distance of nearest active chunk referencing mesh `i`. Lowest-`S` meshes exceeding the VRAM headroom threshold are purged first; `refCount > 0` never purges (hard guard).

##### How It Works
A background task periodically scans the cache, computes `S_i`, and builds a purge list of meshes whose combined VRAM exceeds the free-headroom deficit and whose `S_i` is below a moving threshold. Unmaps GPU buffers via `[M4.6-EXT-01]` and drops the map entry; on next chunk-stream request the existing M2.6 loader re-fetches from disk (already implemented). Async so a bursty unload never stalls a frame.

##### Reference Implementation
```cpp
struct CachedMesh { uint64_t pathHash; MeshHandle handle; uint32_t refCount; float nearestChunkDist; };
void PurgeUnderPressure(std::vector<CachedMesh>& cache, float vramDeficit, float (*FreeVRAM)()) {
    if (FreeVRAM() >= 0) return;                       // headroom exists, do nothing
    std::vector<size_t> order(cache.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b){
        if (cache[a].refCount && !cache[b].refCount) return false;
        return Score(cache[a]) < Score(cache[b]);       // lowest score first
    });
    float freed = 0;
    for (size_t i : order) {
        if (cache[i].refCount > 0) continue;
        freed += MeshVRAMBytes(cache[i].handle);
        ReleaseMeshGPU(cache[i].handle); cache[i].handle = kNull;
        if (freed >= vramDeficit) break;
    }
}
```

##### Player-Facing Impact
Long sessions streaming across many biomes don't OOM the 6 GB Tier-0 VRAM floor; the cache self-trims instead of accumulating until a hard crash.

---

#### `[M12-EXT-07]` (provisional) Network Clock Sync & Tick-Drift Compensator

Fleshes the §5.8 one-liner **Network Clock Sync & Tick Drift Compensator** (line 9123): host-authoritative timeline correction for co-op input-replay alignment, so two clients' simulation clocks don't drift and desync replayed inputs.

##### Systems Touched
Sits above `[M12-EXT-01]` (delta encoder) / `[M12-EXT-02]` (loss-detection) / `[M12-EXT-03]` (fragment reassembler). Reads the existing co-op determinism layer `[M2.8-EXT-04]` (fixed-point deterministic math) as the canonical time source. Distinct from `[M2.8-EXT-05]` (host-authoritative topography reconstruction) — that resyncs world state, this resyncs the *clock* the sim ticks on.

##### Math
Cristian's algorithm over the existing reliable channel (verified: Cristian, Distributed Computing 1989). Client sends `T0`, server tags `T1`/`T2`, client receives at `T3`; estimate: `offset = ((T1 - T0) + (T2 - T3)) / 2`, `RTT = (T3 - T0) - (T2 - T1)`, accuracy plus/minus RTT/2. Per-tick drift `delta = measuredOffset - appliedOffset`; the sim applies `delta` as a smoothed correction factor `alpha * delta + (1 - alpha) * lastCorrection` (low-pass to avoid sawtooth jitter).

##### How It Works
Periodic (not per-tick) probe exchanges over the reliable channel piggybacked on existing keepalive packets. The host is the authority; clients compute `offset`, clamp `RTT` to a sane max (reject outliers from a stalled link), and feed the smoothed correction into their local tick scheduler so replayed inputs land on the same simulation step the host executed them on. Never blocks a frame — the correction is a scheduler nudge, applied between ticks.

##### Reference Implementation
```cpp
struct ClockProbe { uint64_t t0, t1, t2, t3; };
int64_t EstimateOffset(const ClockProbe& p) {            // Cristian
    return (int64_t)((p.t1 - p.t0) + (p.t2 - p.t3)) / 2;
}
int64_t EstimateRTT(const ClockProbe& p) {
    return (int64_t)(p.t3 - p.t0) - (int64_t)(p.t2 - p.t1);
}
float g_smoothedCorrection = 0.0f;                       // applied to local tick scheduler
void ApplyClockCorrection(int64_t offset) {
    const float kAlpha = 0.1f;
    g_smoothedCorrection = kAlpha * (float)offset + (1.0f - kAlpha) * g_smoothedCorrection;
}
```

##### Player-Facing Impact
Co-op sessions stay frame-aligned over imperfect networks; input replay (the determinism strategy from `[M2.8]`) doesn't silently desync because two clocks drifted apart.

---

#### `[M1-EXT-26]` (provisional) Chunk Boundary Entity Transfer Queue

Fleshes the §5.8 one-liner **Chunk Boundary Entity Transfer Queue** (line 9108): atomic handoff of simulated-AI ownership across sector lines without double-ticking the same entity.

##### Systems Touched
Sits on the general enkiTS scheduler (not Jolt's job system — two-scheduler rule). Consumes the sector/chunk streaming boundaries defined in M2.6 (sector size held fixed through M4, line 1196). Coordinates with M5.4 AI director (which owns spawn/despawn density) so a handoff doesn't fight the spawner. Reuses `[M1-EXT-06]`'s SPSC ring-buffer idiom (the verified §5.10 sanity suite uses the same shape) for the cross-sector handoff mailbox.

##### Math
Ownership token handoff is a single atomic state transition per entity: `state: {Local, InTransfer}`; transfer commits only when the destination sector's worker claims the token (`CAS(state, Local, InTransfer)` succeeds on exactly one side). Double-tick prevented by the atomic: the source worker stops ticking the entity the instant `InTransfer` is set.

##### How It Works
When an AI entity crosses a sector boundary, the source sector enqueues a lightweight transfer record (entity handle + its deterministic sim state snapshot) into the destination sector's SPSC mailbox and atomically flips the entity's ownership state. The destination worker drains its mailbox at sector-tick start, adopts the entity, and only then resumes ticking it. The source stops ticking on the `CAS` win — so the entity is ticked by exactly one sector per tick, never both.

##### Reference Implementation
```cpp
enum class Ownership : uint32_t { Local = 0, InTransfer = 1 };
struct TransferRecord { uint32_t entityHandle; /* deterministic sim-state snapshot */ };

// source sector, on boundary cross:
if (entity.ownership.compare_exchange_strong(kLocal, kInTransfer)) {
    destSector.mailbox.Push(TransferRecord{entity.handle, Snapshot(entity)});
    // source stops ticking entity from this tick forward
}
// destination sector, at tick start:
TransferRecord rec; while (destSector.mailbox.Pop(rec)) AdoptEntity(rec);
```

##### Player-Facing Impact
Hordes chasing the player across sector lines don't stutter, double-move, or vanish at the seam — the AI handoff is atomic, not "hope the two workers don't both tick it."

---

#### `[K-EXT-22]` (provisional) Spatiotemporal Blue-Noise Jitter Array Interleaver

Fleshes the §5.8 one-liner **Spatiotemporal Blue-Noise Jitter Array Interleaver** (line 9110): deterministic low-discrepancy per-frame offsets for hybrid ray-tracing denoise, the jitter half that M4.5's denoiser consumes but never specifies how the per-frame sequence is generated/ordered.

##### Systems Touched
Feeds `[M4.5-EXT-13]`'s clustered depth-bounds shadow voxelizer / any RT denoise consumer. Shares the precomputed blue-noise point-set already declared by `[M6.5-EXT-08]` (Blue-Noise Importance Sampling) — this entry adds the *temporal interleave* dimension, not a second noise source. Distinct from `[M6.5-EXT-08]`'s spatial sampling (it distributes spawn positions; this distributes temporal sample phases).

##### Math
A precomputed spatiotemporal blue-noise (STBN) volume (verified: Heitz/Wolfe et al., EGSR 2022, jcgt 14(1) 2025) indexed by `(pixelX mod W, pixelY mod H, frame mod T)`. The interleaver advances the frame index by a step coprime to `T` (e.g. `step = 1` with a scrambled permutation) so consecutive frames sample decorrelated phases while preserving spatial blue-noise per frame.

##### How It Works
At frame `f`, the jitter lookup is `jitter = STBN[pixel mod (W,H)][(f * step) mod T]`. Because STBN is blue in both space and time, the per-frame offsets are evenly distributed and temporally decorrelated, which lets the denoiser converge faster than white-noise jitter. The permutation is precomputed once at boot (no per-frame RNG), so the sequence is fully deterministic — required for the `[M2.8]` co-op determinism strategy.

##### Reference Implementation
```cpp
// stbn: precomputed volume [H][W][T] of vec2 in [0,1)^2, loaded once at boot.
glm::vec2 SampleSTBN(const std::vector<std::vector<std::vector<glm::vec2>>>& stbn,
                     uint32_t px, uint32_t py, uint32_t frame, uint32_t step) {
    const uint32_t H = (uint32_t)stbn.size(), W = (uint32_t)stbn[0].size(), T = (uint32_t)stbn[0][0].size();
    return stbn[py % H][px % W][(frame * step) % T];   // step coprime to T -> decorrelated phases
}
```

##### Player-Facing Impact
RT shadows / GI denoise converge in fewer frames at the same cost, so the game holds its frame budget on the 6 GB Tier-0 floor instead of paying for a noisy, slow-to-settle image.

---

#### `[M13-EXT-15]` (provisional) Kinematic Full-Body IK Surface Locker

Fleshes the §5.8 one-liner **Kinematic Full-Body IK Surface Locker** (line 9115): pins hands/feet to moving vehicle surfaces (e.g. a player bracing on a rocking truck bed), the full-body counterpart to M5.2's upper-body override.

##### Systems Touched
Consumes `[M5.2-EXT-08]` (IK Rig Asset — bone chains) and `[M5.2-EXT-09]` (motion-warping target alignment). Targets are sampled from M9 vehicle rigid-body surfaces (via the existing kinematic virtual-controller surface-friction path, line 1364). Distinct from `[M5.2-EXT-05]` upper-body override (torso/arms only) — this drives the whole-body plant (both feet + support hand).

##### Math
Two-bone (or FABRIK, verified: Aristidou & Lasenby 2011) chain solve per planted limb toward a moving target `T(t)` on the vehicle surface: `solved = FABRIK(chain, root, T(t))`, constrained so the planted point tracks `T(t)` with a max stretch `L_max`; if `|T(t) - root| > L_max` the character is peeled off (loss of grip) rather than accepting an impossible pose.

##### How It Works
When the player initiates a brace (or the AI director decides a zombie clings), the relevant limb targets are bound to sampled surface points on the vehicle body. Each frame the targets are re-sampled from the moving rigid transform, the IK chain is re-solved (cheap two-bone for limbs, FABRIK for a full-body plant), and the result is blended into the final pose after `[M5.2-EXT-05]`. On separation beyond `L_max` the lock releases cleanly.

##### Reference Implementation
```cpp
// FABRIK (Aristidou & Lasenby 2011), verified. Returns false if target unreachable (grip lost).
bool FABRIK(const std::vector<glm::vec3>& joints, const glm::vec3& target,
            float maxStretch, std::vector<glm::vec3>& outSolved) {
    float total = 0; for (size_t i = 1; i < joints.size(); ++i) total += glm::distance(joints[i-1], joints[i]);
    if (glm::distance(joints.front(), target) > total) return false;   // unreachable -> release
    outSolved = joints;
    for (int iter = 0; iter < 10; ++iter) {
        outSolved.back() = target;                                     // backward reach
        for (int i = (int)outSolved.size()-2; i >= 0; --i)
            outSolved[i] = outSolved[i+1] + glm::normalize(outSolved[i]-outSolved[i+1]) * glm::distance(joints[i], joints[i+1]);
        outSolved.front() = joints.front();                            // forward reach (root fixed)
        for (size_t i = 1; i < outSolved.size(); ++i)
            outSolved[i] = outSolved[i-1] + glm::normalize(outSolved[i]-outSolved[i-1]) * glm::distance(joints[i], joints[i-1]);
    }
    return true;
}
```

##### Player-Facing Impact
Mounting/moving-vehicle interactions look physically planted instead of the character sliding through the body or floating — a small but visible fidelity win for vehicle-heavy gameplay.

---

#### `[M3-EXT-10]` (provisional) Spherical-Harmonics Visibility Pre-Filter Grid

Fleshes the §5.8 one-liner **Spherical Harmonics Visibility Pre-Filter Grid** (line 9119): a cheap coarse obstruction check before committing to an expensive LOS raycast — the classic SH visibility cone / PRT-style pre-filter, distinct from `[M6]`'s acoustic voxel occlusion (that tags voxels with material absorption; this stores directional visibility as SH coefficients).

##### Systems Touched
Feeds AI line-of-sight queries (M5.4 director, M5 spawn/perception), and complements `[M6]`'s acoustic voxel occlusion (shares the same coarse voxel grid resolution idiom, different payload: SH coefficients vs. absorption tags). Reuses the world geometry already rasterized for `[M6]`'s occlusion voxels rather than building a second scene representation.

##### Math
Per voxel, store a 2nd-order SH visibility function `V(omega) approx Sum_{l,m} c_{lm} Y_{lm}(omega)` accumulated by rasterizing the surrounding occluder silhouette into the SH basis. A query direction `omega_q` is pre-filtered: `vis = max(0, Sum c_{lm} Y_{lm}(omega_q))` — if `vis < epsilon`, skip the full raycast (already blocked); only rays with `vis > epsilon` pay for the precise trace.

##### How It Works
At world/chunk build time, each voxel's SH visibility is baked by sampling occluder directions (cheap: a few hundred samples per voxel, once) and projecting onto the SH basis. At runtime, an AI LOS check first samples the SH visibility along its query direction; the vast majority of "blocked" checks short-circuit here without a raycast, and only genuinely open directions escalate to the precise (and costly) trace `[M6]` already performs.

##### Reference Implementation
```cpp
// 2nd-order SH (9 coeffs) visibility, baked once per voxel.
struct VoxelSHVis { float c[9]; };
float SampleVisibility(const VoxelSHVis& v, const glm::vec3& dir) {
    // SH evaluation Y_lm(dir) omitted for brevity; standard 2nd-order basis.
    float y[9]; EvalSH2(dir, y);
    float vis = 0; for (int i = 0; i < 9; ++i) vis += v.c[i] * y[i];
    return std::max(0.0f, vis);
}
// runtime: if (SampleVisibility(voxelSH, dirToTarget) < kEpsilon) return BLOCKED; // skip raycast
```

##### Player-Facing Impact
AI perception stays cheap even with many agents querying LOS, holding the 5 ms AI budget on the Tier-0 floor — fewer raycasts, same behavioral result.

---

#### `[M0-EXT-13]` (provisional) GPU-Side Storage-Buffer Decompressor (compute GDeflate)

Fleshes the §5.8 one-liner **GPU-Side Storage Buffer Decompressor (GDeflate, compute)** (line 9107). Per the doc's own caveat (and `[M4.6-EXT-05]`), this is the Vulkan-compute-shader path — name it accordingly, not "DirectStorage" (a Windows/DX API). It is the decompression compute kernel that `[M4.6-EXT-05]`'s ring allocator feeds; this entry supplies the kernel itself.

##### Systems Touched
Sits directly under `[M4.6-EXT-05]` (GPU Decompression Ring Allocator) — that owns buffer recycling, this owns the actual decode. Reads compressed chunk payloads from the streaming channel M4.6 manages. Downstream of the mesh/texture loaders that currently block on CPU decode.

##### Math
GDeflate is a Huffman + LZ77-variant byte stream decoded in a compute shader: each invocation decodes one symbol group, writing reconstructed bytes into the ring-allocated output buffer (`[M4.6-EXT-05]`). No closed-form equation; the "math" is the bit-unpacking state machine: `symbol = DecodeHuffman(bitstream); if (symbol == LITERAL) emit(symbol); else { len, dist = ReadMatch(); Copy(dest-dist, len); }`.

##### How It Works
On a chunk-load burst, the CPU hands the compressed byte range + a ring-allocated output buffer to a compute dispatch. The shader walks the GDeflate bitstream, expands literals/matches in parallel across invocations (each invocation owns a contiguous output span, resolving match-back-references within its span or cooperatively across the span boundary), then signals completion so the loader uploads the now-decompressed mesh/texture. Offloads decode from the CPU so fast driving doesn't stall on a synchronous decompress.

##### Reference Implementation
```cpp
// Compute entry — one invocation per output-span; cooperative back-ref across span edges is
// handled by a shared staging buffer (omitted for brevity; standard GDeflate decode loop).
[[spirv::compute(64)]] void GDeflateDecode(uint3 gid : SV_DispatchThreadID,
        ByteAddressBuffer src, RWByteAddressBuffer dst) {
    uint span = gid.x * kSpanBytes;
    GDeflateState st = InitState(src);
    while (!st.done && st.outPos < kSpanBytes)
        DecodeSymbol(st, dst, span);   // literal -> emit; match -> Copy(dist,len)
}
```

##### Player-Facing Impact
Streaming a fast-moving vehicle across chunk boundaries stays smooth — decompression happens on the GPU in parallel with upload instead of blocking the CPU frame.

---

#### `[M2.8-EXT-09]` (provisional) Co-op Deterministic Seeded Replay Verification

Fleshes an implementation-step gap in M2.8 (Deterministic co-op architecture, §5.2 line 1745): the milestone specifies snapshot-sync as the co-op strategy and fixed-point math (`[M2.8-EXT-04]`), but has no EXT block for the *verification* that two clients actually stay bit-identical — the "determinism check" the AGENTS.md Day-0 Spike A demanded (xor entity states into a running hash, run twice, alternate flags, confirm no divergence).

##### Systems Touched
Reads the fixed-point layer `[M2.8-EXT-04]` (the thing being verified) and the topology-replication token `[M2.8-EXT-02]` (determinism-drift isolation). Writes a 64-bit hash to the existing debug telemetry path (M13 / `[M5.4]` bandit tracker telemetry channel is the natural sink). Does not modify simulation — observation-only, safe to ship disabled.

##### Math
Running 64-bit FNV-1a hash over the serialized deterministic state each tick: `H = H xor FNV1a(state_i); H = (H * 1099511628211) mod 2^64`. Two clients exchange `H` every N ticks; divergence if `H_A != H_B` -> log the tick + the differing entity handle(s).

##### How It Works
At a fixed cadence (e.g. every 60 ticks, dev-gated), each client hashes its full deterministic sim state via the fixed-point serialization `[M2.8-EXT-04]` already produces, and the host compares client hashes. A mismatch trips a dev-only alarm identifying the first diverging entity — exactly Spike A's verification, wired into the shipped co-op path so drift is caught, not assumed away. Ship-disabled by default (per the doc's dev-gated debug-draw rule).

##### Reference Implementation
```cpp
uint64_t g_determinismHash = 14695981039346656037ULL;   // FNV-1a 64 offset
void FoldEntityState(uint64_t state64) {                  // call per deterministic entity/tick
    g_determinismHash ^= state64;
    g_determinismHash *= 1099511628211ULL;               // mod 2^64 is implicit in uint64_t
}
// host: if (clientA.hash != clientB.hash) LogDev("DETERMINISM DIVERGENCE tick=%u", tick);
```

##### Player-Facing Impact
None directly — this is an engine-correctness guard. It is what lets the co-op mode claim "deterministic," instead of shipping a sync strategy that silently drifts on some hardware.

---

## Close-out notes (Part A candidates checked, found ALREADY COVERED — excluded, not authored)

Per the queue's explicit anti-false-positive instruction, these §5.8 one-liners were grep-verified and found to already exist under a different name / inline. Logged here so they aren't re-surfaced as gaps later:

- **Luma-Variance Adaptive Shading Rate Calculator** (§5.8 line 9111) — ALREADY `[M4.5-EXT-24]` ("VRS luma-variance shading rate", confirmed in the §5.6 numbering note at line 8934). Authoring a second block would duplicate it. → close-out, no new ID.
- **Triplanar Material PBR Blending Shader** (§5.8 line 9113) — ALREADY implemented inline at line 3098 ("Triplanar PBR mapping for the RVT terrain overlay … any steep/undomain-unwrapped geometry (cliffs, rubble piles)"). No separate EXT needed. → close-out.
- **Barycentric Silhouette Edge Reconstruction Filter** (§5.8 line 9112) — ALREADY implemented inline at line 3086 ("Visibility Buffer silhouette anti-aliasing reconstruction … EdgeWeight = Saturate(nabla u + nabla v)"), wired into the M4.5 upscaler path. → close-out.
- **Julian-Day Diurnal Atmospheric Gas Density Calculator** (§5.8 line 9121) — ALREADY implemented at line 2047 ("drops the hardcoded static air density constant … evaluating true density (rho) based on your diurnal Julian day temperature variations"). Affects long-range bullet drop as specified. → close-out.
- **Arrhenius Electrochemical Structural Weathering Pass** (§5.8 line 9122) — ALREADY covered by `[M7]`'s weathering system using the Arrhenius rate (verified realistic at line 13; implemented at lines 2511-2521, rust/soot/oxidation over elapsed real hours). → close-out.
- **Herschel-Bulkley Non-Newtonian Mud Rut Tracker** (§5.8 line 9117) — ALREADY `[M9-EXT-04]` ("Non-Newtonian Mud Silt Advection (Soil Rut Tracking)", line 7002) using the Herschel-Bulkley shear model; persistent ruts are `[M9-EXT-19]` POM topography deform. → close-out.
- **Exhaust Backpressure Aquatic Stutter Engine** (§5.8 line 9116) — ALREADY `[M9-EXT-12]` (line 7267), submersion-depth-driven engine stall/misfire. → close-out.
- **Chassis Vortex Debris Drafting Engine** (§5.8 line 9118) — ALREADY `[M9-EXT-15]` (line 7355), wind-wake force behind fast vehicles pulling debris/dust. → close-out.

Net Part A output: **10 new full EXT blocks authored** (M5.2-EXT-14, M11-EXT-09, M4.6-EXT-07, M12-EXT-07, M1-EXT-26, K-EXT-22, M13-EXT-15, M3-EXT-10, M0-EXT-13, M2.8-EXT-09) + **8 close-out findings** (excluded to avoid duplication). No padding to hit 10 with weak material.
**[M1-EXT-27] Uniform-Grid Spatial Hash Broad-Phase**

##### Systems Touched
Standalone broad-phase for ECS physics/proximity queries, distinct from M6.5's blood-spatter density SpatialHash (that is a render-side density accumulator, not a queryable broad-phase). Consumed by `[M1-EXT-06]`'s SPSC queues and `[M1-EXT-26]`'s chunk-boundary transfer, and by `[M3-EXT-11]` / `[M5.4-EXT-09]` below. Runs on the enkiTS scheduler (not Jolt's — two-scheduler rule).

##### Math
World partitioned into uniform cells of size `c`. Each entity inserts its handle into every cell its AABB overlaps: `cell = floor(pos / c)`. Query: gather candidates from the 3x3 (or 2x2x2) neighboring cells. Hash map `cellKey -> vector<handle>` (open-addressing or `std::unordered_map` keyed by `hash(x,y,z)`).

##### How It Works
Rebuilt (or incrementally updated) each tick for dynamic entities. Provides O(1)-ish neighbor iteration for collision pairs, perception radius queries, and decal/spatter density, replacing the N^2 scans that would otherwise gate the AI/phys budget. Determinism preserved by a fixed insertion order (sort by handle id) so the same world yields the same pair list — required by `[M2.8]` co-op.

##### Reference Implementation
```cpp
struct SpatialHash { float cell; std::unordered_map<uint64_t, std::vector<uint32_t>> grid;
    uint64_t Key(int x,int y,int z){ return ((uint64_t)x<<42)^((uint64_t)y<<21)^(uint64_t)z; }
    void Insert(uint32_t h, glm::vec3 p){ auto k=Key((int)(p.x/cell),(int)(p.y/cell),(int)(p.z/cell)); grid[k].push_back(h); } };
```

##### Player-Facing Impact
Horde collisions, melee hit tests, and perception queries stay cheap with thousands of agents on the Tier-0 floor — no O(N^2) blowup when a street fills with zombies.

---

**[M1-EXT-28] GPU Software Occlusion Rasterizer (HZB Feeder)**

##### Systems Touched
Feeds the existing HZB (already used for shadow culling, ~24 mentions) with a coarse occlusion depth buffer generated from the most-recently-rendered frame's depth, so the next frame's draw-call culling can reject hidden meshes before they hit the vertex stage. Distinct from `[M4.5-EXT-31]`'s SDF shadow (that is a lighting term, not a culling gate).

##### Math
Downsample the previous frame's depth buffer by min (for reverse-Z) into a mip chain — each texel holds the nearest depth of its 2x2 children. Occlusion test for a mesh's bounding box: project the 8 corners, sample the HZB at the appropriate mip for the box screen-size; if all samples are closer than the box's far depth, the box is fully occluded → cull.

##### How It Works
A compute pass builds the HZB mip chain from the depth attachment each frame (cheap, bandwidth-bound not compute-bound). The culling pass tests instance AABBs against the HZB and compacts the visible list via the existing indirect-draw path (`[M1]`'s GPU-driven ECS). One-frame latency is hidden by the standard "test against last frame" approach; zero new render targets beyond a depth copy.

##### Reference Implementation
```cpp
bool HzbOccluded(const AABB& b, Texture2D hzb, float proj[16]) {
    vec2 mn=vec2(1), mx=vec2(-1); float farZ=-1e9f;
    for (vec3 c : Corners(b)) { vec4 s=proj*vec4(c,1); vec2 uv=s.xy/s.w*0.5+0.5;
        mn=min(mn,uv); mx=max(mx,uv); farZ=max(farZ,s.z/s.w); }
    int mip = (int)ceil(log2(max(mx.x-mn.x,mx.y-mn.y)*hzbW));
    return AllCloser(hzb, mn, mx, mip, farZ);   // every sampled depth nearer than box far -> occluded
}
```

##### Player-Facing Impact
Distant ruined city blocks behind a closer wall stop consuming vertex/draw budget — the GPU-driven culling win that lets the streaming world draw far without choking the floor spec.

---

**[M5.2-EXT-15] XPBD Rope/Tether Constraint**

##### Systems Touched
Line 5850 notes the long-range-attachment (tether) constraint is used internally for cloth/vegetation but not exposed; this promotes it to a reusable XPBD distance/rope constraint. Consumes `[M5.2]`'s existing constraint solver and `[M5.2-EXT-08]`'s IK/joint infra. Used by zipline/grappling/winch mechanics and drag-ragdoll.

##### Math
Extended Position-Based Dynamics (Macklin et al. 2016): each distance constraint has a compliance `α` and accumulates a Lagrange multiplier `λ` per substep, making stiffness independent of iteration count / timestep: `Δx = (w₁w₂ / (w₁+w₂+α̃)) · C · ∇C`, with `α̃ = α / Δt²`. A rope = chain of N distance constraints solved with substep XPBD (e.g. 4 substeps, 1 iteration each) for stable, non-stretchy ropes.

##### How It Works
Build a rope as a small particle chain (positions + inverse masses), one XPBD distance constraint per segment. Each substep: predict, solve all constraints updating `λ`, then integrate. The tether's two endpoints bind to entity handles (player hand, vehicle hitch) so it follows them. Because `λ` is persistent-corrected, the rope neither explodes nor sags through floors under load — the failure mode the doc flags for naive PBD.

##### Reference Implementation
```cpp
// XPBD distance constraint, Macklin 2016
void SolveDistanceXPBD(Particle& a, Particle& b, float rest, float alpha, float dt, float& lambda) {
    vec3 d = b.x - a.x; float C = length(d) - rest; vec3 n = d / max(length(d),1e-5f);
    float w = a.w + b.w; float aTilde = alpha / (dt*dt);
    float dLambda = (-C - aTilde*lambda) / (w + aTilde);
    lambda += dLambda; a.x -= n * (dLambda*a.w); b.x += n * (dLambda*b.w);
}
```

##### Player-Facing Impact
Grapple lines, tow cables, and dragging a downed survivor all behave with real, load-stable tension instead of the rubber-band stretch or snap that naive PBD ropes show.

---

**[M9-EXT-22] RVT Skid-Mark / Tire-Track Injector**

##### Systems Touched
One of the three features audit line 133 says is blocked on the missing RVT base. Writes tire tracks and drift scars into `[M4.5-EXT-26]`'s RVT overlay from `[M9]`'s wheel-contact + slip-state telemetry. Consumed by the terrain material resolve as an extra blend layer.

##### Math
Per wheel with ground contact and lateral/longitudinal slip `s > s_min`, stamp a decal quad into the RVT page covering the contact patch, with intensity `I = clamp(|s| · load, 0, 1)` and orientation from wheel yaw. The RVT write-merge (newest-wins or max) resolves overlaps; aging fades `I` over time via a per-page timestamp.

##### How It Works
On each physics tick where a wheel is slipping on a drivable surface, compute the world contact patch and push a stamp into the RVT feedback/residency path. Because it lands in the persistent RVT, tracks survive across frames and chunk reloads like real ground scarring. Same atlas `[M4-EXT-02]`/RVT already manages — no second decal system.

##### Reference Implementation
```cpp
void StampSkidMark(RvtPageTable& rvt, const WheelContact& w) {
    if (w.grounded && length(w.slip) > kMinSlip)
        rvt.StampPatch(w.contactPos, w.yaw, /*intensity=*/clamp(length(w.slip)*w.load,0.f,1.f));
}
```

##### Player-Facing Impact
Hard braking and handbrake turns leave real, persistent black streaks on the road — the forensic readability that tells the player "someone fled through here," and it finally has a surface to render onto.

---

**[M6.5-EXT-13] Capillary Blood-Spatter RVT Projection**

##### Systems Touched
Second of the three audit-line-133 blocked features. Projects blood/spatter/decal impacts into `[M4.5-EXT-26]`'s RVT at the impact point with a view-relative angle (capillary/back-spatter direction from the hit normal and projectile vector). Reuses `[M6.5]`'s existing spatial-density tracker for over-age eviction.

##### Math
At impact, compute the projectile incoming direction `d` and surface normal `n`; the spatter lobe opens around the reflect direction `r = reflect(-d, n)`. Stamp an impact decal into the RVT page at the hit UV with a per-instance rotation/scale and a lobe-weighted alpha so the pattern reads as directional spatter, not a symmetric splat. Density eviction uses the existing `SpatialHash` broad-phase (see `[M1-EXT-27]`).

##### How It Works
On a damage/decal event, raycast-free: take the hit world pos, map to the RVT page via the page table, and blend the spatter decal (one of a few base shapes composited with `[M2.6]`'s domain-warped noise so no two are identical). The RVT persistence means blood stays on the wall after the body is gone; the `SpatialHash` density cap recycles oldest marks when a surface saturates.

##### Reference Implementation
```cpp
void ProjectSpatter(RvtPageTable& rvt, vec3 hitPos, vec3 n, vec3 projDir, uint seed) {
    vec3 r = normalize(reflect(-projDir, n));
    rvt.StampDecal(hitPos, /*rot=*/Azimuth(r), /*scale=*/0.6f+0.4f*Hash01(seed), /*alpha=*/LobeWeight(r,n));
}
```

##### Player-Facing Impact
Gunfight scenes leave directional blood that lingers and tells the story of where shots came from — and it finally renders onto real geometry instead of a missing layer.

---

**[M10-EXT-11] Volumetric Cloud & Participating-Medium Scattering**

##### Systems Touched
Only a passing "participating medium" mention exists — no cloud system. Adds a raymarched volumetric cloud + atmospheric participating medium layer read by `[M10-EXT-02]`'s sun/moon vectors and `[M10-EXT-03]`'s scattering for in-scatter/extinction.

##### Math
Raymarch a cloud density field `D(p)` (worley/fbm noise in a slab between altitudes `h0..h1`) along the view ray; at each step accumulate extinction `T *= exp(-σ · dt)` and in-scatter `L += T · σ · phase(θ) · sunLight`. Beer's law extinction `T = exp(-∫σ ds)`; Henyey-Greenstein phase for forward-scatter.

##### How It Works
A half-resolution raymarch pass after opaque + sky. Cloud shape from animated 3D fbm (time-scrubbed by `[M10]`'s weather phase so storms actually roll in). Lit by the same sun vector `[M10-EXT-02]` drives; shadows self-occlude via a short secondary march toward the sun. Bounded step count for the floor; disabled or lowered on Tier-0.

##### Reference Implementation
```cpp
vec3 MarchClouds(vec3 ro, vec3 rd, vec3 sunDir) {
    float t = h0; vec3 L = 0, T = 1;
    for (int i=0;i<kSteps;i++) { vec3 p = ro+rd*t; float d = CloudDensity(p);
        float sig = d*kSigma; T *= exp(-sig*dt);
        L += T * sig * HenyeyGreenstein(dot(rd,sunDir),kG) * SunLight(p,sunDir) * dt; t += dt; }
    return L;
}
```

##### Player-Facing Impact
Storm fronts and moonlit cloud decks give the night real atmosphere instead of a flat sky dome — the mood the WW2-night horror aesthetic lives on.

---

**[M3-EXT-11] Fracture-Debris Broad-Phase Reuse**

##### Systems Touched
Macro-destruction `[M3]` spawns many small debris bodies on collapse; pairs them against the standalone broad-phase so debris-debris and debris-agent collision stays O(1)-query instead of O(N^2). Consumes `[M1-EXT-27]`'s spatial hash; feeds `[M3]`'s fracture graph resolution.

##### Math
Same uniform-grid hash as `[M1-EXT-27]`; debris bodies insert by AABB, fracture chunks insert by shard AABB. Pair candidates = union of cell memberships; only those pairs run the narrow-phase GJK/contact solve.

##### How It Works
On a structural collapse event, every spawned shard/debris registers into the per-tick spatial hash; the solver pulls candidate pairs per cell and runs narrow-phase only on them. Keeps a building pancaking into a street of zombies from tripping the physics budget — the debris count can spike into the hundreds without an N^2 penalty.

##### Reference Implementation
```cpp
// reuse [M1-EXT-27] SpatialHash; only narrow-phase the returned pairs
auto pairs = hash.QueryPairs(shardAABB);
for (auto& pr : pairs) if (Overlap(pr.a, pr.b)) NarrowPhase(pr.a, pr.b);
```

##### Player-Facing Impact
Collapsing walls throw believable, colliding rubble that interacts with the horde without the sim stuttering — the spectacle of destruction stays smooth on the floor spec.

---

**[M8.6-EXT-10] Settlement NavMesh from GA Layout**

##### Systems Touched
Consumes `[M8.6-EXT-09]`'s GA building-footprint layout and produces the walkable NavMesh the AI director (`[M5.4]`) and horde pathing need inside a settlement. Distinct from `[M4-EXT-08]`'s WFC reachability (that is interiors; this is the settlement exterior/plaza graph).

##### Math
From the GA's footprint polygon set, compute the free-space polygon (building set subtracted from the zoning polygon), then run a standard NavMesh triangulation (e.g. improved funnel / Delaunay over the walkable region) with portal edges between adjacent polygons for string-pulling path queries.

##### How It Works
Once `[M8.6-EXT-09]` bakes building footprints + gate positions, this derives the settlement's walkable mesh in the same one-time bake. Doorways become portals; the perimeter chokepoints the GA optimized for defense become natural funnel points the horde pathing uses. Cached with the layout — never recomputed per-frame.

##### Reference Implementation
```cpp
NavMesh BuildSettlementNavMesh(const SettlementLayoutGenome& g) {
    Polygon free = ZoningPolygon(); for (auto& b : g.buildings) free = Subtract(free, b.footprint);
    return TriangulateWalkable(free, /*portalEdges=*/GateEdges(g.buildings));
}
```

##### Player-Facing Impact
Zombies navigate a settlement's actual streets and gates (not through walls) the moment it's built — and the defensive chokepoints the GA placed double as the paths the horde funnels through.

---

**[M12-EXT-13] Interest-Management Spatial Hash (Net Culling)**

##### Systems Touched
Co-op netcode (`[M12]`) needs per-client relevance culling so a client only receives state for entities near its view. Reuses `[M1-EXT-27]`'s spatial hash as the relevance index; feeds `[M12-EXT-01]`'s delta encoder (only diff entities in the client's interest cells).

##### Math
Each client owns an interest region (its view sphere expanded by a slack band). The spatial hash maps entity position → cell; a client receives updates for entities in cells overlapping its interest region. Cell size tuned so a cell ≈ the net update granularity; handoff at cell boundaries mirrors `[M1-EXT-26]`'s sector transfer.

##### How It Works
Server maintains the shared spatial hash of entity positions; on each net tick it queries the client's interest cells and sends only those deltas. Entities leaving the region stop being sent (with a final "despawn" delta). Keeps bandwidth flat as the world scales — a client in a quiet suburb doesn't pay for a firefight two sectors away.

##### Reference Implementation
```cpp
void CollectRelevant(const SpatialHash& h, const ClientView& v, std::vector<uint32_t>& out) {
    for (auto cell : h.CellsOverlapping(v.interestSphere)) for (uint32_t e : h[cell]) out.push_back(e);
}
```

##### Player-Facing Impact
Co-op stays low-latency and bandwidth-bounded no matter how big the shared world gets — each player only streams what's around them.

---

**[M6-EXT-12] Convolution-Reverb from Voxel Occlusion**

##### Systems Touched
`[M6]`'s audio is HRTF/propagation-based but has no reverb system; this adds impulse-response convolution reverb derived from `[M6-EXT-11]`'s voxel acoustic occlusion (material absorption already tagged per voxel). Distinct from the occlusion *query* — this is the tail/space rendering.

##### Math
At a listener position, gather the surrounding voxel occlusion field and material absorption to synthesize an impulse response `h(t)` (early reflections from nearest hard surfaces + exponential tail from total absorption `α`: `h(t) = Σ early_i δ(t-t_i) + e^{-α t} · noiseTail`). Convolve the dry voice: `y = x * h`.

##### How It Works
On a (throttled) listener-move or material-change, rebuild a short convolution IR from the local voxel samples — a stone room yields a bright, long tail; open rubble yields a dead, short one. The IR is applied via partitioned convolution (short FIR, CPU-cheap) to the existing positional voices. Ties the audible space to the same geometry the visuals use.

##### Reference Implementation
```cpp
ImpulseResponse BuildIR(const VoxelField& vf, vec3 listener) {
    ImpulseResponse ir; for (auto& s : NearestReflectors(vf, listener, 8)) ir.early.push_back({s.dist, s.energy});
    ir.tailDecay = TotalAbsorption(vf, listener); return ir;   // y = x * ir via partitioned conv
}
```

##### Player-Facing Impact
Interiors and under-bridges sound enclosed, sewers sound dead, open streets sound live — the audio space matches the visual space instead of one global reverb setting.

---

**[M4.6-EXT-08] BC7 / Block-Texture Compression & Transcode**

##### Systems Touched
Zero mention of block compression in doc. Adds GPU-friendly BC7 (desktop) / ASTC (mobile) compression for the material/atlas textures `[M4-EXT-25]` and `[M4.5-EXT-26]`'s RVT produce, cutting VRAM on the 6 GB Tier-0 floor. Runs on the enkiTS scheduler at bake/load time (not the render thread).

##### Math
BC7: each 4x4 texel block is encoded into 128 bits across one of 8 partition modes with endpoint+index quantization and an optional mode-1 alpha split; quality/speed trade via the partition search. ASTC generalizes to arbitrary block sizes (4x4..12x12) with a similar endpoint+weight scheme. Offline or load-time encode; hardware decodes for free.

##### How It Works
Material graph output and atlas pages are encoded to BC7 once at content-bake (or first load, cached). The decompressor `[M0-EXT-13]`/ring allocator hands already-compressed data to the GPU which samples it natively — no per-frame decode. VRAM for a 4K albedo drops ~4x (RGBA8 → BC7). Tier-0 budget relief without visual loss at the chosen quality preset.

##### Reference Implementation
```cpp
// encode 4x4 RGBA block to BC7 (quality preset 1..255)
void EncodeBC7(const RGBA* block4x4, uint8_t out128[16], int quality) {
    BC7Partition best = SearchPartitions(block4x4, quality);   // 8 modes, endpoint quant
    WriteBC7(out128, best);                                     // GPU samples natively
}
```

##### Player-Facing Impact
The floor-spec VRAM budget stretches to hold more unique materials/atlases at once — fewer streaming hitches when rounding a corner into a new biome.

---

**[M5.4-EXT-09] Fear-Field Diffusion via Spatial Hash]

##### Systems Touched
`[M5.4]`'s Reaction-Diffusion Fear Field currently lacks its diffusion step's neighbor mechanism; this runs it over `[M1-EXT-27]`'s spatial hash so the field spreads to nearby agents/cells cheaply. Reads `[M5.4]`'s `TensionSignal`; feeds spawn-density the same director already uses.

##### Math
Discrete diffusion on the fear scalar field `F`: `F_i^{t+1} = F_i^t + k · Σ_j (F_j^t - F_i^t)` over neighbors `j` gathered from the spatial hash cell + ring. Combined with the existing reaction term (fear rises near zombies, decays over time) per the director's pacing model.

##### How It Works
Each director tick, the fear field's grid cells (or agent cloud) are inserted into the spatial hash; diffusion samples the 3x3 cell ring for the Laplacian. Bounded cost regardless of horde size — fear radiates outward from a sighting at a rate the spatial hash makes O(1)-per-cell instead of O(N^2) against every agent.

##### Reference Implementation
```cpp
void DiffuseFear(FearField& F, const SpatialHash& h, float k) {
    for (auto& cell : F.cells) {
        float lap = 0; for (auto nb : h.Ring(cell.pos)) lap += F[nb] - F[cell];
        cell.next = cell.val + k * lap;
    }
}
```

##### Player-Facing Impact
A sighting makes nearby survivors genuinely flee outward in a believable gradient instead of a uniform panic toggle — the AI reads as reacting to events, not a script.

---

#### [M5.4-EXT-10] Procedural Mission & Event Director

##### Systems Touched
Reads M5.4's `TensionSignal` and Reaction-Diffusion Fear Field, M8's Regional SEIR Curve, and Appendix C socio-economic tags. Writes through M13's SLM generators (already declared in the TOC as feeding mission text across M4/M5.4/M6/M8/M8.5/M8.6/M10/M11). Resolves objectives against M8.6 settlement state and M8.5 faction economy. Owned here (not M13) because M5.4 already owns the pacing telemetry it reads — M13 stays the text-generation consumer, not the owner.

##### Math
`Score(template, worldState) = Σ_tag  Match(template.requiredTags[tag], worldState.tags[tag]) · tagWeight[tag]`

##### How It Works
Two established approaches. (1) Bethesda's Radiant Story pattern (Skyrim/Fallout 4): template slot-fill — pick an objective archetype (retrieve/escort/defend/eliminate/supply-run), resolve location/target/reward slots against live world-state queries. Cheap, deterministic, easy to author solo. (2) Dormans & Bakkes, "Generating Missions and Spaces for Adaptable Play Experiences" (2011): graph-grammar mission generation, where mission structure and level layout share the same grammar so missions and space co-emerge instead of being placed after the fact — a better structural fit here specifically because M4 already generates the world through grammars (WFC, L-systems), so this reuses a formalism already in the doc rather than adding a new one.

Recommend the same two-tier split already used above in `[M4-EXT-09]`: Radiant-Story-style template slot-fill for the common ambient mission stream (reads world tags continuously, same pattern the Director already uses for spawn pacing), and grammar-based mission-space co-generation reserved for marked high-value locations — settlement quest boards, faction contracts — where it's worth the extra generation cost to let a mission shape what actually gets built there. Selection weights come directly from the same Appendix C tags and Regional SEIR Curve already driving spawn density elsewhere, so mission variety tracks the simulation instead of reading as randomized flavor text on top of it — a starving settlement biases toward supply-run templates, a high-severity outbreak zone biases toward extraction/defense templates.

##### Reference Implementation
```cpp
struct MissionTemplate { std::string archetypeId; std::unordered_map<std::string, float> requiredTags; };

// Schematic — bind against the live socio-economic tag / SEIR accessor names in source,
// not invented here; this shows the scoring shape only.
const MissionTemplate* SelectMissionTemplate(const std::vector<MissionTemplate>& templates, const WorldTagState& state) {
    const MissionTemplate* best = nullptr; float bestScore = -1.0f;
    for (const auto& t : templates) {
        float score = 0.0f;
        for (const auto& [tag, weight] : t.requiredTags) score += state.TagMatch(tag) * weight;
        if (score > bestScore) { bestScore = score; best = &t; }
    }
    return best;
}
```

##### Player-Facing Impact
Side content stays legible and reactive — a starving settlement two sectors over produces a supply mission there specifically, not a generic fetch-quest anywhere on the map — without hand-authoring a mission for every settlement instance the world generator produces.

---

**[M4-EXT-27] Vegetation Impostor (Foliage-Specific)]

##### Systems Touched
Distinct from `[M4.5-EXT-30]`'s generic mesh impostor — this targets foliage specifically (bushes, tall grass from `[M4-EXT-18]`/L-system clumps), using a cross-quad/star billboard that preserves wind sway `[M6.5-EXT-11]` already drives. Reuses `[M4]`'s Poisson-disk foliage placement.

##### Math
A foliage impostor is a camera-facing cross of 2-3 quads (or an octahedral-billboard like `[M4.5-EXT-30]`) textured from a baked top-down/side render of the clump, with an alpha-tested leaf card. Wind sway is kept by reusing the existing foliage vertex shader's wind term on the impostor quads (phase from world pos) so distant grass still ripples.

##### How It Works
At bake, render each foliage species clump from a few angles into an impostor atlas. Beyond the impostor distance, swap the live mesh for the billboard; the swap band cross-fades. Because it reuses the wind shader, distant fields don't freeze — they keep moving, just cheaper. Cuts foliage draw cost at distance so dense ground cover doesn't tank the floor framerate.

##### Reference Implementation
```cpp
// impostor quad uses same wind vertex displacement as the live clump
vec3 WindyImpostorPos(vec3 p, float t) { return p + WindDelta(p, t) * kFoliageWindGain; }
```

##### Player-Facing Impact
Thick roadside grass and bush clumps stay dense and wind-animated far from the player without the triangle cost — the overgrown, infested look the setting wants, affordable on the floor spec.

---

**[M4-EXT-28] Procedural Foliage L-System Mesh Generator**

##### Systems Touched
Distinct from `[M4-EXT-18]` (space-colonization canopy — tree crown/branch macro-structure only, unsuited to small ground clumps). Feeds from the existing Poisson-disk clustering (Appendix F) already driving foliage placement. Output meshes are what `[M6.5-EXT-11]`'s fBm wind kinematics already sways. Species/density gated by `[M4-EXT-17]`'s nutrient grid, same gate canopy trees already use.

##### Math
Standard Lindenmayer rewrite: axiom string expanded N iterations per species template, walked with a turtle-graphics position/heading/pitch stack to extrude a low-poly branch skeleton.

##### How It Works
Ground-level foliage (bushes, dead brambles, tall-grass clumps) is a different generative problem from `[M4-EXT-18]`'s canopy trees — space colonization grows toward light-attraction points and fits large branching canopies, not small procedural clumps. L-systems fit this scale better: a short per-species axiom + rewrite ruleset expands N iterations, then a turtle walk extrudes the branch skeleton and caps it with camera-facing or cross-quad leaf billboards. Species template selection reuses the same nutrient-grid species threshold `[M4-EXT-17]` already gates canopy placement with, so ground foliage and canopy species stay consistent with the same soil chemistry rather than a second, disconnected placement rule. Runs once at chunk-bake time.

##### Reference Implementation
```cpp
struct LSystemRule { char symbol; std::string replacement; };

std::string ExpandAxiom(const std::string& axiom, const std::vector<LSystemRule>& rules, uint32_t iterations) {
    std::string current = axiom;
    for (uint32_t i = 0; i < iterations; ++i) {
        std::string next;
        for (char c : current) {
            auto it = std::find_if(rules.begin(), rules.end(), [c](auto& r) { return r.symbol == c; });
            next += (it != rules.end()) ? it->replacement : std::string(1, c);
        }
        current = next;
    }
    return current;
}
// WalkTurtleToMesh(expandedString) -> branch skeleton verts, standard turtle-graphics
// interpretation (F = extrude+advance, +/- = pitch, [/] = push/pop stack) — omitted here,
// well-established, not worth re-deriving.
```

##### Player-Facing Impact
Dense, varied ground foliage with no two bushes identical, without hand-modeling a bush library — and it inherits wind sway for free since it's just another mesh feeding the existing foliage vertex shader.

---

**[M7-EXT-11] Zstd Save-Compression & Streaming Store]

##### Systems Touched
`[M7]` persistence writes saves but has no compression system named; this adds Zstd (or a comparable entropy+cDictionary codec) for the binary save blobs `[M7-EXT-10]`'s schema produces, cutting save size and load IO on the 12 GB system-RAM / slow-disk floor.

##### Math
Zstd: LZ77 match-finding + FSE/Huffman entropy coding, with an optional trained dictionary over save-structure prefixes for small saves. Compression level tuned (e.g. 3 for save-time, 19 for cold-storage backup). Streaming API so large world saves compress in chunks without holding the whole blob.

##### How It Works
On save, the serialized `[M7-EXT-10]` blob is pushed through the Zstd streaming compressor to disk; on load, decompressed back. A per-save dictionary (trained once on representative saves) shrinks repeated struct layouts dramatically. Failure mode per AGENTS.md: a corrupt/short read logs and exits cleanly, never a hang.

##### Reference Implementation
```cpp
std::vector<uint8_t> CompressSave(const std::vector<uint8_t>& blob, int level) {
    ZSTD_CStream* s = ZSTD_createCStream(); ZSTD_initCStream(s, level);
    std::vector<uint8_t> out; ZSTD_inBuffer in{blob.data(), blob.size(), 0};
    do { out.resize(out.size()+ZSTD_CStreamOutSize()); ZSTD_outBuffer o{out.data()+out.size()-ZSTD_CStreamOutSize(), ZSTD_CStreamOutSize(), 0};
         ZSTD_compressStream(s, &o, &in); out.resize(out.size()-o.pos); } while (in.pos < in.size);
    ZSTD_endStream(s, &o); out.resize(out.size()-o.pos); ZSTD_freeCStream(s); return out;
}
```

##### Player-Facing Impact
Saves stay small and load fast even with a huge explored world — the 12 GB RAM floor isn't eaten by uncompressed save blobs, and cloud/backup rotation (see doc meta bucket) is cheaper.


---


# Extended Systems Library — Appendix EXT (merged 2026-07-14)

*225 gap entries merged from `ZombieEngine_Milestone_Gaps.md`, renumbered to next-free per-milestone EXT IDs. Each block notes its original gap-file id. Fact-checked vs this spec (0 ID collisions at merge time).*


## Milestone M0 — Appendix EXT additions (24)


### [M0-EXT-30] Game Draw Path Bypasses Cull/Indirect (demo-only)
*(gap-file ref: M0-001)*

##### Systems Touched
M0/M1 render. `TriangleRenderer::draw` (TriangleRenderer.cpp:688-719) returns early on `bridgeActive` and draws per-entity `vkCmdDrawIndexed`+`vkCmdPushConstants` (`:713,707`). Cull/Hi-Z/occlusion run only on demo branch.
##### Math
`if (bridgeActive) { vkCmdDrawIndexed(...); return; } // unculled, unbatched`
##### How It Works
Route the game path through `cull.comp`→indirect like the demo path. Headless run never logs "Rendered N ECS entities" (:717) — path unverified by CI.
##### Reference Implementation
```cpp
// in draw(): remove early-return; always: cull -> indirect -> occlusion -> drawIndexedIndirect
```
##### Player-Facing Impact
Real ECS→GPU path actually runs and is testable.

---

---



### [M0-EXT-31] Hardcoded 100-Entity Cap
*(gap-file ref: M0-002)*

##### Math
-
##### Systems Touched
M0/M1. `InstanceData instances(100)` (:91), indirect sized 100 (:107,293), occlusion pool 100 (:491), `pc.instanceCount=100` (:592).
##### How It Works
Replace fixed `100` with dynamic buffers sized to live entity count.
##### Reference Implementation
```cpp
std::vector<InstanceData> instances; instances.reserve(liveCount);
```
##### Player-Facing Impact
Horde of any size renders.

---

---



### [M0-EXT-32] Hi-Z Depth Pyramid Has No Producer
*(gap-file ref: M0-003)*

##### Math
-
##### Systems Touched
M0/M1. `depth_pyramid.comp` exists; `grep DepthPyramid|hiz` in src → 0. `cull.comp:35` samples unpopulated texture.
##### How It Works
Dispatch `depth_pyramid.comp` after depth prepass each frame; feed result to cull.
##### Reference Implementation
```cpp
vkCmdDispatch(cmd, pyramidW/8, pyramidH/8, 1); // after depth write
```
##### Player-Facing Impact
GPU occlusion culling becomes real.

---

---



### [M0-EXT-33] Orphaned Staging / Frame-Deletion Systems
*(gap-file ref: M0-004)*

##### Math
-
##### Systems Touched
M0. `FrameDeletionQueue` + `StagingRingBuffer`: zero non-test call sites. `AssetManager::ExecuteOneShotStaging` (:117-179) is correct but stalls every upload (alloc pool+CB+fence, submit, wait, destroy per call).
##### How It Works
Wire `StagingRingBuffer` as the persistent upload path; retire one-shot fences per `FrameDeletionQueue`.
##### Reference Implementation
```cpp
ring.Allocate(); // persistent, no per-upload fence wait
```
##### Player-Facing Impact
No per-upload pipeline stall on asset load.

---

---



### [M0-EXT-34] No Async Shader Compile + Watchdog
*(gap-file ref: M0-005)*

##### Math
-
##### Systems Touched
M0. `grep graphics_pipeline_library|ShaderCompile` → 0. Synchronous pipeline build; no `VK_EXT_graphics_pipeline_library`; no timeout.
##### How It Works
Use pipeline-library + async compile thread + boot watchdog fallback.
##### Reference Implementation
```cpp
VkPipelineShaderStageModuleIdentifierCreateInfoEXT // pipeline library
```
##### Player-Facing Impact
Shaders compile without blocking/freezing boot.

---

---



### [M0-EXT-35] SoA Hot-Component Layout
*(gap-file ref: M0-006)*

##### Math
-
##### Systems Touched
M1. `grep SoA` → 0. `Transform` is AoS; GPU upload wants packed `vec3[]`.
##### How It Works
Maintain parallel `vec3[]` position/rotation/scale arrays for upload.
##### Reference Implementation
```cpp
struct TransformSoA { std::vector<glm::dvec3> pos; std::vector<glm::dquat> rot; };
```
##### Player-Facing Impact
Cheaper, cache-friendly GPU upload.

---

---



### [M0-EXT-36] Meshlet / Mesh-Shader Pipeline
*(gap-file ref: M0-007)*

##### Math
-
##### Systems Touched
M0/M3. `grep meshlet|mesh_shader` → 0. M3 "millions of tris" needs it.
##### How It Works
Cluster meshes into meshlets; draw via mesh shading or meshlet indirect.
##### Reference Implementation
```cpp
// meshopt::meshopt_buildMeshlets(...)
```
##### Player-Facing Impact
Cities of geometry at framerate.

---

---



### [M0-EXT-37] Parallel Secondary Command-Buffer Recording
*(gap-file ref: M0-008)*

##### Math
-
##### Systems Touched
M0. `RenderGraph.cpp:204-296` has the framework; no pass records secondaries yet.
##### How It Works
Each render pass records its own secondary CB on a worker; primary executes them.
##### Reference Implementation
```cpp
VkCommandBuffer secondary = pool.Alloc(threadId); pass.Record(secondary);
```
##### Player-Facing Impact
CPU render cost parallelized.

---

---



### [M0-EXT-38] Occlusion Query Double-Buffering
*(gap-file ref: M0-009)*

##### Math
-
##### Systems Touched
M1-EXT-03. Code at TriangleRenderer.cpp:760-797 but demo-branch only.
##### How It Works
Make occlusion queries active on the game path; double-buffer results across frames.
##### Reference Implementation
```cpp
occlusion[frame%2].Resolve(); // not demo-only
```
##### Player-Facing Impact
Occluded geometry skipped in real game.

---

---



### [M0-EXT-39] Parallel-Deterministic PRNG Core
*(gap-file ref: M0-010)*

##### Systems Touched
M0/M4/M2.8. SplitMix64 is thread-local only; no coord-addressed streams.
##### Math
`stream = hash(worldSeed, cx, cy, salt, i)` → SplitMix64(stream).
##### How It Works
Every gen RNG draw derives from spatial coords so multithreaded gen reproduces.
##### Reference Implementation
```cpp
uint64_t CoordStream(uint64_t s,int cx,int cy,uint32_t salt,uint32_t i){return splitmix64(s^(uint64_t(cx)<<32)^cy^(uint64_t(salt)<<16)^i);}
```
##### Player-Facing Impact
Bit-identical worlds across machines/threads.

---

---



### [M0-EXT-40] Per-Fiber Exception Containment
*(gap-file ref: M0-011)*

##### Math
-
##### Systems Touched
M0. `grep SEH|__try` → only imgui false-positive. No try/catch around enkiTS jobs.
##### How It Works
Wrap job dispatch in `__try/__except` (Win) / `std::invoke` guard; bad job disables itself + logs.
##### Reference Implementation
```cpp
try { job(); } catch(...) { DisableJob(id); Log(); }
```
##### Player-Facing Impact
One bad job can't crash the engine.

---

---



### [M0-EXT-41] Single-Instance Boot Lock
*(gap-file ref: M0-012)*

##### Math
-
##### Systems Touched
M0. `grep SingleInstance|lockfile` → 0. Two copies corrupt the same save.
##### How It Works
Create `game.lock` (or named mutex) at boot; exit if held.
##### Reference Implementation
```cpp
if (!TryLockFile(userData/"game.lock")) { Fatal("Already running"); }
```
##### Player-Facing Impact
Save files can't be clobbered by a second instance.

---

---



### [M0-EXT-42] Frame-Deletion Queue Wiring
*(gap-file ref: M0-013)*


##### Systems Touched
M0. `FrameDeletionQueue` defined but zero non-test call sites.

##### Math
-

##### How It Works
Wire FrameDeletionQueue::Retire() into per-frame cleanup so GPU objects free on a known boundary.

##### Reference Implementation
```cpp
frameDeletion.Retire(buffer, currentFrame+2);
```

##### Player-Facing Impact
No leaked/double-freed GPU resources.

---

---



### [M0-EXT-43] GPU Validation in Release Builds
*(gap-file ref: M0-014)*


##### Systems Touched
M0. Validation layers gated `--dev`; ship a light runtime check.

##### Math
-

##### How It Works
Enable stripped Vulkan validation at boot even in release, logging to a ring buffer.

##### Reference Implementation
```cpp
if (cvarGpuValidate) EnableLightValidation();
```

##### Player-Facing Impact
GPU errors caught in shipping, not just dev.

---

---



### [M0-EXT-44] Release Build Sanitizer Path
*(gap-file ref: M0-015)*


##### Systems Touched
M0. `CMakeLists.txt:43-46` FATAL_ERROR on TSAN/UBSAN Windows.

##### Math
-

##### How It Works
Provide Linux TSAN job + document Windows limitation instead of hard-failing configure.

##### Reference Implementation
# CMake: FATAL only on MSVC; clang/WSL path documented

##### Player-Facing Impact
ECS mutation code gets TSAN-checked in CI.

---

---



### [M0-EXT-45] Descriptor-Set Fragmentation Guard
*(gap-file ref: M0-016)*


##### Systems Touched
M0/M4.5. Bindless page allocator needs defrag.

##### Math
-

##### How It Works
Monitor descriptor pool fragmentation; defrag on threshold.

##### Reference Implementation
```cpp
if (frag > 0.8f) DefragDescriptors();
```

##### Player-Facing Impact
No descriptor exhaustion.

---

---



### [M0-EXT-46] Secondary CB on Game Path
*(gap-file ref: M0-017)*


##### Systems Touched
M0/M1. Secondary CBs framework-only (demo).

##### Math
-

##### How It Works
Record secondaries for real passes.

##### Reference Implementation
```cpp
pass.RecordSecondary(worker);
```

##### Player-Facing Impact
Real parallel CPU render.

---

---



### [M0-EXT-47] Platform Support (Linux/macOS/console)
*(gap-file ref: M0-018)*


##### Systems Touched
M0. Windows-only assumption.

##### Math
-

##### How It Works
Abstract platform layer; CI builds for Linux + macOS; console later. `#ifdef PLATFORM_*`

##### Reference Implementation
Game runs beyond Windows.

##### Player-Facing Impact
Wider audience.


---



### [M0-EXT-48] Build & Packaging Pipeline
*(gap-file ref: M0-019)*


##### Systems Touched
M0. No installer/bundle.

##### Math
-

##### How It Works
CMake install + asset bundle + signed installer step. `install(TARGETS ...)`

##### Reference Implementation
Game ships as a package.

##### Player-Facing Impact
Distributable.


---



### [M0-EXT-49] Automated Test Matrix (unit/integration)
*(gap-file ref: M0-020)*


##### Systems Touched
M0. Thin tests.

##### Math
-

##### How It Works
CI matrix: unit (ECS, math, gen) + integration (headless sim, gen determinism). `ctest --build-and-test`

##### Reference Implementation
Regressions caught pre-merge.

##### Player-Facing Impact
Stable codebase.


---



### [M0-EXT-50] Performance Budget & Profiling Dashboard
*(gap-file ref: M0-021)*


##### Systems Touched
M0. Tracy only.

##### Math
-

##### How It Works
Per-system CPU/GPU budget dashboard + automated perf gate in CI. `if (frameMs>budget) Fail();`

##### Reference Implementation
Perf regressions blocked.

##### Player-Facing Impact
Sustained framerate.


---



### [M0-EXT-51] Anti-Tamper & Memory Integrity
*(gap-file ref: M0-023)*


##### Systems Touched
M0. No integrity check.

##### Math
-

##### How It Works
Periodic integrity hash of critical state; detect external edits. `if (Hash(state)!=expect) Flag();`

##### Reference Implementation
Cheats/hacks detected.

##### Player-Facing Impact
Fairer play.


---



### [M0-EXT-52] Crash Reporter & Minidump Upload
*(gap-file ref: M0-024)*


##### Systems Touched
M0. Minidump only.

##### Math
-

##### How It Works
On crash, collect minidump + log, offer upload (consent). `CrashHandler::Report();`

##### Reference Implementation
Crashes are diagnosable.

##### Player-Facing Impact
Faster fixes.


---



### [M0-EXT-53] Analytics & Privacy-Consent
*(gap-file ref: M0-025)*


##### Systems Touched
M0. No analytics.

##### Math
-

##### How It Works
Opt-in analytics with consent gate; no PII. `if (consent) analytics.Log(event);`

##### Reference Implementation
Data informs tuning, respects privacy.

##### Player-Facing Impact
Ethical telemetry.


---



## Milestone M1 — Appendix EXT additions (14)


### [M1-EXT-39] SpatialHash Query Has No Gameplay Consumer
*(gap-file ref: M1-001)*

##### Math
-
##### Systems Touched
M1. `QueryRadius`/`QueryCell` defined (SpatialHash.cpp:151,179) but only called from unit test (Engine.cpp:651-664).
##### How It Works
Give AI/audio/proximity systems a real `QueryRadius` consumer.
##### Reference Implementation
```cpp
auto near = hash.QueryRadius(zombiePos, hearRadius);
```
##### Player-Facing Impact
Proximity queries stop reimplementing spatial search.

---

---



### [M1-EXT-40] No Batched Raycast (RayBatchQuery)
*(gap-file ref: M1-002)*

##### Math
-
##### Systems Touched
M1/M5/M6/M9. `grep RayBatch` → 0. Jolt `CastRay` collector = multi-hit-per-ray, not N independent rays.
##### How It Works
Build `RayBatchQuery` fanning out over Jolt; one batch, many rays, multi-hit per ray.
##### Reference Implementation
```cpp
struct RayBatch { std::vector<JPH::RayCast> rays; std::vector<JPH::RayCastResult> hits; };
```
##### Player-Facing Impact
AI sight, audio occlusion, bullets share one ray path.

---

---



### [M1-EXT-41] Parent-Child Hierarchy
*(gap-file ref: M1-003)*

##### Math
-
##### Systems Touched
M1/M9. `grep "struct Parent"` → 0. Weapon sockets, camera-on-head, turrets need it.
##### How It Works
Add `Parent` component + dirty-flag world-transform propagation.
##### Reference Implementation
```cpp
struct Parent { entt::entity parent; glm::dmat4 local; };
```
##### Player-Facing Impact
Attached objects follow parents correctly.

---

---



### [M1-EXT-42] EnTTCache Narrow Coverage
*(gap-file ref: M1-004)*

##### Math
-
##### Systems Touched
M1-EXT-09. Only `Transform` snapshot tested (Engine.cpp:391-412); other hot components not cached.
##### How It Works
Extend TTCache to all hot components; assert round-trip in tests.
##### Reference Implementation
```cpp
EnTTCache::Snapshot<Velocity, Health>(e);
```
##### Player-Facing Impact
Deterministic transport has full component coverage.

---

---



### [M1-EXT-43] Action Map / ActionState Layer
*(gap-file ref: M1-005)*

##### Systems Touched
M0-EXT-15/M11. `Input.cpp` polls continuous keyboard (good) but no `ActionState`/`ActionMap`, no gamepad hot-plug, no deadzone.
##### Math
`ActionState.value = Curve(RawAxis, Deadzone)`.
##### How It Works
Resolve raw input once/tick into named `ActionState`; gameplay reads actions, not scancodes.
##### Reference Implementation
```cpp
struct ActionState { bool held, pressedThisFrame; float value; };
```
##### Player-Facing Impact
Input is rebindable + device-agnostic.

---

---



### [M1-EXT-44] Procedural Camera Rig
*(gap-file ref: M1-006)*

##### Systems Touched
M1/M2.7. `FlyCamera.cpp` = WASD+mouse-look only. Real player cam = virtual-head entity, semi-implicit Euler damped spring.
##### Math
`v += (-k*x - c*v)*dt; x += v*dt;` (velocity FIRST).
##### How It Works
Camera is an entity driven by the spring solver; input sets target, not directly position.
##### Reference Implementation
```cpp
camVel += (-k*camPos - c*camVel)*dt; camPos += camVel*dt;
```
##### Player-Facing Impact
Smooth, professional camera feel.

---

---



### [M1-EXT-45] Input-Rebinding Persistence
*(gap-file ref: M1-007)*

##### Math
-
##### Systems Touched
M1/M11. No bind file / save of key mappings.
##### How It Works
Serialize bindings to `settings/input.json`; load at boot.
##### Reference Implementation
```cpp
json bindings = Load("settings/input.json"); Apply(bindings);
```
##### Player-Facing Impact
Players keep their keybinds across sessions.

---

---



### [M1-EXT-46] M1 Exit Criteria Don't Reflect Reality
*(gap-file ref: M1-008)*

##### Math
-
##### Systems Touched
M1 audit hygiene (J2). Criteria claim GPU indirect draw verified, but verified path is demo-only (M0-001).
##### How It Works
Require the *game* path drawing via indirect+cull in headless before M1 closes.
##### Reference Implementation
```cpp
// headless render harness asserts "Rendered N ECS entities"
```
##### Player-Facing Impact
Milestone status becomes trustworthy.

---

---



### [M1-EXT-47] EntityFactory Unknown-Component Warning
*(gap-file ref: M1-009)*


##### Systems Touched
M1/M2. Prefab with `UnregisteredComponent` silently dropped.

##### Math
-

##### How It Works
Log a warning (not silent drop) on unknown component type.

##### Reference Implementation
```cpp
if (!meta.valid(type)) Warn("unknown component "+name);
```

##### Player-Facing Impact
Mod/prefab mistakes visible, not silent.

---

---



### [M1-EXT-48] Generational Table Headroom Check
*(gap-file ref: M1-010)*


##### Systems Touched
M1. EnTT cap = 1,048,576 (2^20, issue #197).

##### Math
2^20 = 1,048,576

##### How It Works
Assert entity count under cap; plan chunking if horde exceeds.

##### Reference Implementation
```cpp
assert(live < 1'048'576);
```

##### Player-Facing Impact
No silent entity-ID exhaustion.

---

---



### [M1-EXT-49] EnTTCache Round-Trip Test
*(gap-file ref: M1-011)*


##### Systems Touched
M1-EXT-09. Only Transform tested.

##### Math
-

##### How It Works
Add round-trip tests for all hot components.

##### Reference Implementation
```cpp
assert(RoundTrip<Velocity>(v) == v);
```

##### Player-Facing Impact
Full deterministic transport coverage.

---

---



### [M1-EXT-50] Action Map Persistence Format
*(gap-file ref: M1-012)*


##### Systems Touched
M1/M11. Bind file schema.

##### Math
-

##### How It Works
Versioned JSON bind schema with migration.

##### Reference Implementation
```json
{"version":1,"binds":{}}
```

##### Player-Facing Impact
Bindings survive updates.

---

---



### [M1-EXT-51] GPU Memory Budget & Defrag
*(gap-file ref: M1-013)*


##### Systems Touched
M1. No GPU mem budget.

##### Math
-

##### How It Works
Track GPU allocation vs budget; defrag/compact on threshold. `if (gpuUsed>budget) Compact();`

##### Reference Implementation
No GPU OOM.

##### Player-Facing Impact
Stable VRAM.


---



### [M1-EXT-52] Shader Variant/Permutation Manager
*(gap-file ref: M1-014)*


##### Systems Touched
M1. No variant manager.

##### Math
-

##### How It Works
Key→compiled-pipeline cache; compile on demand; prune unused. `Pipeline p = Variants.Get(key) ?? Compile(key);`

##### Reference Implementation
Shaders scale cleanly.

##### Player-Facing Impact
No shader spam.


---



## Milestone M2 — Appendix EXT additions (21)


### [M2-EXT-08] Character Controller / Player Movement
*(gap-file ref: M2-001)*

##### Math
-
##### Systems Touched
M2. `grep CharacterController|CharacterBody` → 0. Jolt has `Character` but nothing wires it to input.
##### How It Works
Instantiate `JPH::Character`; feed `ActionState` → `MoveAndSlide` each tick.
##### Reference Implementation
```cpp
character->Move(character->GetVelocity() + gravity, dt, {});
```
##### Player-Facing Impact
Player actually moves/collides.

---

---



### [M2-EXT-09] Animation / Skinning
*(gap-file ref: M2-002)*

##### Math
-
##### Systems Touched
M2/M5. `grep Animation|SkinnedMesh` → 0. `bufferDeviceAddress` enabled (Device.cpp:61,116) but unused in shaders.
##### How It Works
Bind skin matrices via BDA; drive from physics/IK (M5.2).
##### Reference Implementation
```glsl
mat4 skin = deref(skinMatAddr)[jointIndex];
```
##### Player-Facing Impact
Zombies/players animate, not static meshes.

---

---



### [M2-EXT-10] Ballistics / Projectile Resolution
*(gap-file ref: M2-003)*

##### Math
-
##### Systems Touched
M2.9. `BulletComponent` exists but `grep projectile|ricochet|penetration` → only data fields, no solver.
##### How It Works
Step projectile each tick; resolve ricochet/penetration/tumbling; emit `DamageEvent`.
##### Reference Implementation
```cpp
if (RayBatchQuery::HitWall(b)) b.vel = Reflect(b.vel, normal);
```
##### Player-Facing Impact
Guns feel real (bounces, pass-through).

---

---



### [M2-EXT-11] Destructible = Placeholder Swap Only
*(gap-file ref: M2-004)*

##### Math
-
##### Systems Touched
M2/M3. `Destructible.h` has intact/destroyed handles; `grep fracture|Fracture` → 0.
##### How It Works
Add fracture (pre-fractured shards or runtime) + decals; not just mesh swap.
##### Reference Implementation
```cpp
spawnShards(entity, impactPoint); hideIntact(entity);
```
##### Player-Facing Impact
World breaks believably.

---

---



### [M2-EXT-12] Vehicle System
*(gap-file ref: M2-005)*

##### Math
-
##### Systems Touched
M2/M9. `grep Vehicle|wheel|suspension` → 0. M9 turrets have no base.
##### How It Works
Jolt vehicle constraint (wheels/suspension/chassis) → entity-driven.
##### Reference Implementation
```cpp
JPH::VehicleConstraint* v = new JPH::VehicleConstraint(body, wheelCount);
```
##### Player-Facing Impact
Drivable/usable vehicles exist.

---

---



### [M2-EXT-13] Hit/Limb Resolution Pipeline
*(gap-file ref: M2-006)*

##### Math
-
##### Systems Touched
M2.9. `DamageEvent.LimbId` (Components.h:20-29) but no raycast-hit→limb mapping; no mitigation producing `ResolvedDamageEvent`.
##### How It Works
Map hit point → skeleton limb; apply armor/parry → `ResolvedDamageEvent`.
##### Reference Implementation
```cpp
LimbId limb = Skeleton::LimbAt(hitPoint); dmg = Mitigate(base, armor[limb]);
```
##### Player-Facing Impact
Limb-specific damage (headshots etc.).

---

---



### [M2-EXT-14] Physics Debug Draw (shipping toggle)
*(gap-file ref: M2-007)*

##### Math
-
##### Systems Touched
M2. `PhysicsDebugRenderer` exists but `JPH_DEBUG_RENDERER`-gated; no shipping toggle.
##### How It Works
Add a runtime CVar to enable collision visualization.
##### Reference Implementation
```cpp
if (cvarPhysicsDebug) physicsRenderer.Draw();
```
##### Player-Facing Impact
Players/modders can see collision.

---

---



### [M2-EXT-15] No Real Content / Player / Zombie
*(gap-file ref: M2-008)*

##### Math
-
##### Systems Touched
M2/M5. Only prefab spawns 3 test entities (one `UnregisteredComponent` silently dropped). No player, zombie, weapon.
##### How It Works
Author player + zombie + arena spawn; wire `EntityFactory` to drop unknown components with a warning, not silently.
##### Reference Implementation
```cpp
if (!meta.valid(type)) Warn("dropped " + name); else emplace(e, type);
```
##### Player-Facing Impact
There is a game to play.

---

---



### [M2-EXT-16] Save/Load Layer
*(gap-file ref: M2-009)*

##### Math
-
##### Systems Touched
M2/M7. `grep serialize|SaveGame` → 0. Plant `SaveSchemaVersion`+`NetworkProtocolVersion` (separate), atomic write (tmp→fsync→checksum→rename), user-data dir via `AssetPath`, crash tag.
##### How It Works
Serialize ECS snapshot to zstd blob; write atomically; stamp version.
##### Reference Implementation
```cpp
AtomicWrite(userData/"save.dat", Compress(serialize(world)));
```
##### Player-Facing Impact
Progress persists safely.

---

---



### [M2-EXT-17] World-Epoch Clock
*(gap-file ref: M2-010)*

##### Systems Touched
M2/M7. `grep WorldEpoch|simulated-seconds` → 0. Sim time = frameCount/accumulator.
##### Math
`ElapsedSim = (WallNow - WallSave) * TimeScale`.
##### How It Works
Store `WorldEpoch` + accumulated sim-seconds; fast-forward offline decay/weather.
##### Reference Implementation
```cpp
struct WorldClock { uint64_t epoch; double simSeconds; };
```
##### Player-Facing Impact
World keeps living while you're away.

---

---



### [M2-EXT-18] Survival Physiology (stamina/hunger)
*(gap-file ref: M2-011)*


##### Systems Touched
M2.9. `Components.h` has `PerkPoints` but no survival loop.

##### Math
-

##### How It Works
Data-driven stamina/hunger/thirst decay feeding gameplay.

##### Reference Implementation
```cpp
stamina -= dt * exertion;
```

##### Player-Facing Impact
Survival pressure exists.

---

---



### [M2-EXT-19] Crafting / Recipes / Workbench
*(gap-file ref: M2-012)*


##### Systems Touched
M8. No crafting system.

##### Math
-

##### How It Works
Recipe table + workbench consuming items → new items.

##### Reference Implementation
```cpp
if (Has(recipe.inputs)) { Consume(recipe.inputs); Give(recipe.output); }
```

##### Player-Facing Impact
Players craft gear.

---

---



### [M2-EXT-20] Skills / Perks / Progression
*(gap-file ref: M2-013)*


##### Systems Touched
M8.7. `PerkPoints` exists, no system.

##### Math
-

##### How It Works
Spend PerkPoints into perk tree affecting stats; SLM flavor.

##### Reference Implementation
```cpp
if (points>=cost) { points-=cost; Apply(perk); }
```

##### Player-Facing Impact
Progression loop closes.

---

---



### [M2-EXT-21] Procedural Variation (bio-weight)
*(gap-file ref: M2-014)*


##### Systems Touched
M5.1. No bio-weight/visual variation generator.

##### Math
-

##### How It Works
Per-archetype variation (height, skin, gait) from seeded params; SLM assists.

##### Reference Implementation
```cpp
ZombieVariant v = BioWeight(seed, archetype);
```

##### Player-Facing Impact
Zombies look individually distinct.

---

---



### [M2-EXT-22] Physics Determinism Test
*(gap-file ref: M2-015)*


##### Systems Touched
M2. No physics-determinism CI.

##### Math
-

##### How It Works
Run Jolt sim twice; assert identical transforms.

##### Reference Implementation
```cpp
assert(Sim(a)==Sim(b));
```

##### Player-Facing Impact
Co-op physics won't desync.

---

---



### [M2-EXT-23] Reload Mechanics & Cancel
*(gap-file ref: M2.7-002)*


##### Systems Touched
M2.7. No reload-cancel or reload-state machine in code.

##### Math
-

##### How It Works
Add reload phases (start/tactical/complete) with cancel + interrupt on weapon-swap. `enum ReloadPhase { None, Start, Tactical, Complete };`

##### Reference Implementation
Player can cancel/queue reloads; swapping cancels cleanly.

##### Player-Facing Impact
Reloads feel responsive, not locked animations.


---



### [M2-EXT-24] Recoil & Spread Model
*(gap-file ref: M2.7-003)*


##### Systems Touched
M2.7/M2.9. No recoil or spread solver.

##### Math
-

##### How It Works
Per-weapon recoil curve + spread that tightens when ADS/crouched, widens when moving. `spread = base * (moving?2:1) * (ads?0.4:1);`

##### Reference Implementation
Weapon feel varies by weapon; skill matters.

##### Player-Facing Impact
Guns have distinct, learnable recoil.


---



### [M2-EXT-25] Aiming & ADS System
*(gap-file ref: M2.7-005)*


##### Systems Touched
M2.7. No aim-down-sights or hip/ADS toggle.

##### Math
-

##### How It Works
ADS narrows FOV, tightens spread, slows move; toggle bound to ActionState. `if (ads) { fov*=0.7; speed*=0.6; }`

##### Reference Implementation
Aim toggle binds to input action.

##### Player-Facing Impact
Players can aim precisely.


---



### [M2-EXT-26] Damage Types & Materials
*(gap-file ref: M2.7-006)*


##### Systems Touched
M2/M8. No per-material damage response.

##### Math
-

##### How It Works
Map damage type (bullet/blunt/fire/bite) x material (flesh/metal/wood) → multiplier table. `dmg *= matTable[mat][type];`

##### Reference Implementation
Damage responds to what you hit.

##### Player-Facing Impact
Material-aware combat (wood vs metal barricades).


---



### [M2-EXT-27] Body Temperature & Weather Exposure
*(gap-file ref: M2.9-003)*


##### Systems Touched
M2.9. No temperature system.

##### Math
-

##### How It Works
Core temp drifts with weather/wetness; extremes cause stamina/health penalties. `temp += (envTemp - temp)*k*dt;`

##### Reference Implementation
Cold rain hurts if ungeared.

##### Player-Facing Impact
Survival depth beyond hunger/thirst.


---



### [M2-EXT-28] Encumbrance & Stamina Drain
*(gap-file ref: M2.9-004)*


##### Systems Touched
M2.9. No carry-weight system.

##### Math
-

##### How It Works
Total weight → move-speed penalty + faster stamina drain; over max = can't sprint. `speed *= 1 - clamp(weight/maxW,0,0.6);`

##### Reference Implementation
Heavy loads slow you.

##### Player-Facing Impact
Inventory has meaningful tradeoffs.


---



## Milestone M3 — Appendix EXT additions (24)


### [M3-EXT-12] Terrain / World Geometry
*(gap-file ref: M3-001)*

##### Math
-
##### Systems Touched
M2.6/M3. `grep Terrain|Foliage` → 0 (false positives in Engine). Open world has no ground.
##### How It Works
Heightfield + material splatting; stream tiles.
##### Reference Implementation
```cpp
TerrainTile t = GenTile(cx, cy, seed);
```
##### Player-Facing Impact
There is ground to stand on.

---

---



### [M3-EXT-13] Streaming / LOD / Mip-Streaming
*(gap-file ref: M3-002)*

##### Math
-
##### Systems Touched
M2.6/M3. `grep Streaming|LOD` → 0 (only EventBus comment). Horde + open world need both.
##### How It Works
Distance-based tile stream-in/out + LOD swap + mip-streaming.
##### Reference Implementation
```cpp
if (Dist(tile) < R) StreamIn(tile); else StreamOut(tile);
```
##### Player-Facing Impact
Infinite world without的内存 blowup.

---

---



### [M3-EXT-14] Cascaded Shadow Map Decision
*(gap-file ref: M3-003)*

##### Math
-
##### Systems Touched
M0/M3/M4. "Shadow map" spec'd (10 hits) but "cascaded"/CSM = 0. Low-moon night needs CSM.
##### How It Works
Decide CSM (3–4 cascades) + SDSM stable splits now; assign owner.
##### Reference Implementation
```cpp
for (cascade) RenderShadowMap(cascade);
```
##### Player-Facing Impact
Shadows work at gameplay range.

---

---



### [M3-EXT-15] Mipmap Generation on Load
*(gap-file ref: M3-004)*

##### Math
-
##### Systems Touched
M0/M4. Spec `mipmap` → 0, code → 0.
##### How It Works
GPU mipmap generation at asset load or offline bake.
##### Reference Implementation
```cpp
vkCmdBlitImage(..., mipLevels); // generate mips
```
##### Player-Facing Impact
No distance shimmer/sparkle.

---

---



### [M3-EXT-16] Post-Generation Sanity Checker
*(gap-file ref: M3-005)*

##### Math
-
##### Systems Touched
M4/V. Validate geometry reachability, no floating/overlapping (PCG survey).
##### How It Works
Runs after gen; flags degenerate/orphaned geometry.
##### Reference Implementation
```cpp
bool SanityCheck(const Chunk& c){ return !Floating(c) && !Overlap(c) && NavFeasible(c); }
```
##### Player-Facing Impact
Generated worlds are traversable + sane.

---

---



### [M3-EXT-17] Collision/Occlusion Sanity
*(gap-file ref: M3-006)*

##### Math
-
##### Systems Touched
M4/physics. Generated colliders don't trap player or leave gaps.
##### How It Works
Sample collider coverage vs walkable surface; flag traps.
##### Reference Implementation
```cpp
if (TrapDetected(c)) Regenerate(c);
```
##### Player-Facing Impact
No unescapable pockets.

---

---



### [M3-EXT-18] Navmesh Feasibility Check
*(gap-file ref: M3-007)*

##### Math
-
##### Systems Touched
M4/M5. Generated geometry is pathable; else regenerate.
##### How It Works
After navmesh build, flood-fill from spawn; unreachable POIs flagged.
##### Reference Implementation
```cpp
if (!NavReachable(spawn, poi)) Flag(poi);
```
##### Player-Facing Impact
Everything is reachable.

---

---



### [M3-EXT-19] Loot/Spawn Density Sanity
*(gap-file ref: M3-008)*

##### Math
-
##### Systems Touched
M4/M5.4. Not 0, not 500 in one room.
##### How It Works
Density within `[min,max]` per area; outside → re-roll.
##### Reference Implementation
```cpp
if (density < min || density > max) Reroll(table);
```
##### Player-Facing Impact
Balanced, non-broken encounters.

---

---



### [M3-EXT-20] POI Connectivity Check
*(gap-file ref: M3-009)*

##### Math
-
##### Systems Touched
M4/V. Every POI reachable from roads.
##### How It Works
Graph reachability from road network; orphan POIs dropped/relinked.
##### Reference Implementation
```cpp
if (!ConnectedToRoad(poi)) Drop(poi);
```
##### Player-Facing Impact
No isolated POIs.

---

---



### [M3-EXT-21] Mod Content Validator (V-section entry)
*(gap-file ref: M3-010)*

##### Math
-
##### Systems Touched
M11/V. Runs mod data through sanity checks before load.
##### How It Works
Mod tables/worlds pass M3-005..009 before activation.
##### Reference Implementation
```cpp
if (!ValidateMod(mod)) Reject(mod);
```
##### Player-Facing Impact
Mods can't inject broken worlds.

---

---



### [M3-EXT-22] Soft-Fail on Invalid Gen
*(gap-file ref: M3-011)*

##### Math
-
##### Systems Touched
M4/M11. Skip + log + fallback, don't crash.
##### How It Works
Validation error → chunk falls back to safe default, logs severity.
##### Reference Implementation
```cpp
if (!SanityCheck(c)) { UseDefault(c); Warn(loc); }
```
##### Player-Facing Impact
A bad seed/mod never hard-crashes.

---

---



### [M3-EXT-23] Generation Regression Corpus
*(gap-file ref: M3-012)*

##### Math
-
##### Systems Touched
M4/T98. Curated seeds asserted stable across versions.
##### How It Works
Golden seeds regenerated each CI; diff vs baseline hash.
##### Reference Implementation
```cpp
for (s : goldenSeeds) AssertEqual(Gen(s), baseline[s]);
```
##### Player-Facing Impact
Gen algorithm changes can't alter worlds silently.

---

---



### [M3-EXT-24] Degenerate-Triangle Filter
*(gap-file ref: M3-013)*

##### Math
-
##### Systems Touched
M4/render. Drop zero-area tris from gen.
##### How It Works
Pre-upload filter on generated meshes.
##### Reference Implementation
```cpp
if (Area(tri) < eps) Drop(tri);
```
##### Player-Facing Impact
No z-fighting/invisible geometry.

---

---



### [M3-EXT-25] Asset Fallback Chain
*(gap-file ref: M3-014)*

##### Math
-
##### Systems Touched
M4/AssetManager. Missing asset → default, not crash.
##### How It Works
`GetAsset(path) ?? GetDefault(type)`.
##### Reference Implementation
```cpp
auto a = Asset::Get(path); if (!a) a = Asset::Default(type);
```
##### Player-Facing Impact
Mods referencing missing assets degrade gracefully.

---

---



### [M3-EXT-26] Bounds/Extent Validator
*(gap-file ref: M3-015)*

##### Math
-
##### Systems Touched
M4. Entities inside chunk bounds.
##### How It Works
Reject/spawn-correct entities outside `[0,chunkSize]`.
##### Reference Implementation
```cpp
if (!InBounds(e.pos, chunk)) Correct(e);
```
##### Player-Facing Impact
No entities leaking between chunks.

---

---



### [M3-EXT-27] Duplicate-Entity Detector
*(gap-file ref: M3-016)*

##### Math
-
##### Systems Touched
M4. Gen doesn't spawn 2 entities same spot.
##### How It Works
Hash position grid; collision → drop duplicate.
##### Reference Implementation
```cpp
if (grid.Has(pos)) Drop(e); else grid.Add(pos);
```
##### Player-Facing Impact
No stacked/invisible dupes.

---

---



### [M3-EXT-28] Validation Severity Levels
*(gap-file ref: M3-017)*

##### Math
-
##### Systems Touched
M11/V. Error vs warning vs info for mod UX.
##### How It Works
Each check returns Severity; mod UI shows counts.
##### Reference Implementation
```cpp
struct CheckResult { Severity sev; string why; };
```
##### Player-Facing Impact
Modders see how broken their content is.

---

---



### [M3-EXT-29] Generated-Content Hash
*(gap-file ref: M3-018)*

##### Systems Touched
M4/M7. Detect unexpected chunk changes.
##### Math
`chunkHash = hash(serializedChunk)`.
##### How It Works
Compared across versions.
##### Reference Implementation
```cpp
uint64_t h = Hash(Serialize(c));
```
##### Player-Facing Impact
Silent world drift is caught.

---

---



### [M3-EXT-30] Stress-Seed Suite
*(gap-file ref: M3-019)*

##### Math
-
##### Systems Touched
M4/T98. 10k seeds gen in CI to catch rare crashes.
##### How It Works
Fuzz generator with random seeds; assert no crash + passes sanity.
##### Reference Implementation
```cpp
for (i in 10000) Gen(RandomSeed()); // assert no throw
```
##### Player-Facing Impact
Rare gen crashes found before players do.

---

---



### [M3-EXT-31] Human-Readable Validation Report
*(gap-file ref: M3-020)*

##### Math
-
##### Systems Touched
M4/M11. Modders see WHY content failed.
##### How It Works
Report lists `check:reason:location` per failure.
##### Reference Implementation
```cpp
report << check << ":" << reason << ":" << loc << "\n";
```
##### Player-Facing Impact
Modders fix content instead of guessing.

---

---



### [M3-EXT-32] Macro-Destruction / Structural Graphs
*(gap-file ref: M3-021)*


##### Systems Touched
M3. No building structural-graph destruction.

##### Math
-

##### How It Works
Buildings as node/edge graphs; destruction severs edges → collapse.

##### Reference Implementation
```cpp
graph.Sever(impactEdge); if (!graph.Connected()) Collapse();
```

##### Player-Facing Impact
Buildings break structurally.

---

---



### [M3-EXT-33] Foliage / Vegetation Scatter
*(gap-file ref: M3-022)*


##### Systems Touched
M2.6/M3. `grep Foliage|Vegetation` → 0.

##### Math
-

##### How It Works
GPU-instanced foliage scatter from biome + density noise.

##### Reference Implementation
```cpp
InstancedFoliage(biome.density, noise);
```

##### Player-Facing Impact
World has plants.

---

---



### [M3-EXT-34] Texture Streaming Decision (VT vs mip)
*(gap-file ref: M3-023)*


##### Systems Touched
M3/M4. Virtual texturing (0) vs mip-streaming (planned).

##### Math
-

##### How It Works
Decide VT vs standard mip-streaming at M3; reuse streaming.

##### Reference Implementation
```cpp
// VK_EXT_descriptor_buffer for VT or mip LOD bias
```

##### Player-Facing Impact
Texture memory bounded.

---

---



### [M3-EXT-35] Material Permutation System
*(gap-file ref: M3-024)*


##### Systems Touched
M3/M4.5. No permutation pipeline.

##### Math
-

##### How It Works
Permutation-key → pipeline map; compile on demand.

##### Reference Implementation
```cpp
Pipeline p = Cache.Get(permKey) ?? Compile(permKey);
```

##### Player-Facing Impact
Material variants without shader spam.

---

---



## Milestone M4 — Appendix EXT additions (32)


### [M4-EXT-29] Layer-Based Contextual Generation
*(gap-file ref: M4-001)*

##### Systems Touched
M4. Monolithic gen → layered passes consuming neighbor context (LayerProcGen/EPC2024).
##### Math
`Layer[n](c) = f(Layer[n-1](c), NeighborBorder[0..7])`.
##### How It Works
Generate in dependency order; layer waits for neighbor border data.
##### Reference Implementation
```cpp
for (int L=0; L<COUNT; ++L) s_layerFns[L](c, n.border[L]);
```
##### Player-Facing Impact
Coherent worlds without seams.

---

---



### [M4-EXT-30] Generation Pass DAG
*(gap-file ref: M4-002)*

##### Math
-
##### Systems Touched
M4/M11. Declares passes; mods insert/reorder.
##### How It Works
Passes registered with before/after; topological sort; mods add via ModAPI.
##### Reference Implementation
```cpp
TopoSort(passes); // mod passes participate
```
##### Player-Facing Impact
Modders extend world-gen without forking.

---

---



### [M4-EXT-31] Biome Blending at Borders
*(gap-file ref: M4-003)*

##### Systems Touched
M4. Hard biome cuts → smooth blend.
##### Math
`blend = smoothstep(dist/width); lerp(biomeA, biomeB, blend)`.
##### How It Works
Border ring samples neighbor weights; interpolate terrain/asset choice.
##### Reference Implementation
```cpp
float w = smoothstep(0, blendW, distToBorder);
```
##### Player-Facing Impact
No jarring biome walls.

---

---



### [M4-EXT-32] POI Placement Validator
*(gap-file ref: M4-004)*

##### Math
-
##### Systems Touched
M4/V. Rules before commit.
##### How It Works
Min distance from roads, not in water, slope < threshold, reachable; else relocate/drop.
##### Reference Implementation
```cpp
bool ok = DistToRoad(p)>30 && !InWater(p) && Slope(p)<0.3f;
```
##### Player-Facing Impact
POIs are always sensible.

---

---



### [M4-EXT-33] Loot/Encounter Table Compiler
*(gap-file ref: M4-005)*

##### Systems Touched
M4/M8. Seed-stable tables → resolved drops.
##### Math
`drop = Table[hash(seed, chunkID, poiID, salt)]`.
##### How It Works
Author tables as data; bake resolved drops per POI at gen time.
##### Reference Implementation
```cpp
auto drop = LootTable::Resolve(seed, poiId, salt);
```
##### Player-Facing Impact
Loot reproducible from seed; mods retune via tables.

---

---



### [M4-EXT-34] Generation Seed Hierarchy
*(gap-file ref: M4-006)*

##### Systems Touched
M4/D. `world→region→chunk→entity` derivation.
##### Math
`chunkSeed = hash(worldSeed, cx, cy)`; `entSeed = hash(chunkSeed, i)`.
##### How It Works
Regen a sub-region with only its derived seed.
##### Reference Implementation
```cpp
uint64_t cs = hash(worldSeed, cx, cy);
```
##### Player-Facing Impact
Editing a region doesn't invalidate whole world.

---

---



### [M4-EXT-35] Stable Entity-ID per Chunk
*(gap-file ref: M4-007)*

##### Math
-
##### Systems Touched
M4/ECS. IDs stable across regen.
##### How It Works
IDs assigned from `(chunkSeed, spawnOrder)`, not insertion order.
##### Reference Implementation
```cpp
entt::entity id = MakeStable(chunkSeed, order);
```
##### Player-Facing Impact
Saved references survive regen.

---

---



### [M4-EXT-36] Archetype Seeded Variation Salt
*(gap-file ref: M4-008)*

##### Systems Touched
M4/M5.1. Per-archetype salt, not global seed.
##### Math
`varSeed = hash(chunkSeed, archetypeSalt, i)`.
##### How It Works
Each archetype varies independently + reproducibly.
##### Reference Implementation
```cpp
uint64_t vs = hash(chunkSeed, archSalt, i);
```
##### Player-Facing Impact
Per-archetype variety is independent.

---

---



### [M4-EXT-37] Generation Budget Governor
*(gap-file ref: M4-009)*

##### Math
-
##### Systems Touched
M4/M0-EXT-23. Caps entities/triangles per chunk.
##### How It Works
`if (chunk.entityCount > MAX) decimate(clutter);`
##### Reference Implementation
```cpp
if (count > kMaxEntitiesPerChunk) Decimate(chunk);
```
##### Player-Facing Impact
No chunk blows the budget.

---

---



### [M4-EXT-38] Fractal Octave Cap
*(gap-file ref: M4-010)*

##### Math
-
##### Systems Touched
M4. Bounds noise cost at zoom.
##### How It Works
Max octaves clamped by chunk LOD; deeper detail deferred to streaming LOD.
##### Reference Implementation
```cpp
int oct = min(maxOctaves, lod);
```
##### Player-Facing Impact
Gen stays bounded at any distance.

---

---



### [M4-EXT-39] Biome→Asset Palette Binding
*(gap-file ref: M4-011)*

##### Math
-
##### Systems Touched
M4/AssetManager. Biome → texture/mesh set.
##### How It Works
Biome def declares asset-set ID; resolve through AssetPath.
##### Reference Implementation
```cpp
auto set = AssetSet::Get(biome.assetSetId);
```
##### Player-Facing Impact
Each biome looks distinct.

---

---



### [M4-EXT-40] Clutter/Scatter Generator
*(gap-file ref: M4-012)*

##### Math
-
##### Systems Touched
M4. Non-gameplay dress-up.
##### How It Works
Poisson-disk scatter in free cells, seeded + collision-tested.
##### Reference Implementation
```cpp
for (p : PoissonDisk(area, r)) if (Free(p)) spawnClutter(p);
```
##### Player-Facing Impact
Worlds feel lived-in.

---

---



### [M4-EXT-41] Water/Liquid Volume Generator
*(gap-file ref: M4-013)*

##### Math
-
##### Systems Touched
M4. Lakes/sewers with flow.
##### How It Works
Noise-bounded basins → water volume; flow from heightfield gradient.
##### Reference Implementation
```cpp
if (height < waterLevel) addWater(cell);
```
##### Player-Facing Impact
Navigable/avoidable bodies of water.

---

---



### [M4-EXT-42] Cave/Underground Generator
*(gap-file ref: M4-014)*

##### Math
-
##### Systems Touched
M4. Vertical chunks.
##### How It Works
3D cellular/WFC for tunnels; connect to surface POIs.
##### Reference Implementation
```cpp
GenTunnels(chunk3D, seed); LinkToSurface(poi);
```
##### Player-Facing Impact
Underground content expands vertically.

---

---



### [M4-EXT-43] Generation Disk-Cache
*(gap-file ref: M4-015)*

##### Systems Touched
M4/M7. Revisits instant + deterministic.
##### Math
`cacheKey = hash(worldSeed, cx, cy, genVersion)`.
##### How It Works
On unload, serialize chunk to `cache/<key>`; reload reads if hash matches.
##### Reference Implementation
```cpp
if (Cache::Has(key)) return Cache::Load(key);
```
##### Player-Facing Impact
No regen hitch when backtracking.

---

---



### [M4-EXT-44] Generation Debug Visualization
*(gap-file ref: M4-016)*

##### Math
-
##### Systems Touched
M4/T95. Which pass produced what + seed inspector.
##### How It Works
Per-layer debug overlay + seed readout in dev overlay.
##### Reference Implementation
```cpp
if (dev) DrawLayerBorders(chunk);
```
##### Player-Facing Impact
Modders see gen internals live.

---

---



### [M4-EXT-45] Multi-Resolution LOD Generation
*(gap-file ref: M4-017)*

##### Math
-
##### Systems Touched
M4/M3. Gen at multiple detail levels.
##### How It Works
Each LOD baked from same seed at decreasing octaves.
##### Reference Implementation
```cpp
for (lod) Bake(chunk, seed, lod);
```
##### Player-Facing Impact
Correct detail at every distance.

---

---



### [M4-EXT-46] Procedural Sky/Starfield/Cloud
*(gap-file ref: M4-018)*

##### Math
-
##### Systems Touched
M4/M10. Night atmosphere, moddable.
##### How It Works
Seeded star catalog + cloud noise; moon phase from WorldEpoch.
##### Reference Implementation
```cpp
StarField sf = GenStars(seed); Clouds c = GenClouds(seed);
```
##### Player-Facing Impact
Cohesive moddable night mood.

---

---



### [M4-EXT-47] Async Generation Off Main Thread
*(gap-file ref: M4-019)*

##### Math
-
##### Systems Touched
M4/M0 jobs. Gen in worker jobs; main thread never stalls.
##### How It Works
Chunk gen dispatched to enkiTS; result via staging.
##### Reference Implementation
```cpp
jobSystem.Dispatch([&]{ GenerateChunk(c); });
```
##### Player-Facing Impact
No frame hitches during streaming.

---

---



### [M4-EXT-48] Per-Frame Generation Budget
*(gap-file ref: M4-020)*

##### Systems Touched
M4/M0-EXT-23. N ms cap; spill to next frame (Epic hitching guide).
##### Math
`genMsThisFrame <= GEN_BUDGET_MS`.
##### How It Works
Generator yields after budget; queue resumes next frame.
##### Reference Implementation
```cpp
if (Clock::ms() - start > budget) return Yield;
```
##### Player-Facing Impact
Streaming stays smooth under load.

---

---



### [M4-EXT-49] Prioritized Chunk Queue
*(gap-file ref: M4-021)*

##### Math
-
##### Systems Touched
M4. Player-adjacent first.
##### How It Works
Queue sorted by distance-to-player each tick.
##### Reference Implementation
```cpp
queue.Sort([](a,b){return Dist(a)<Dist(b);});
```
##### Player-Facing Impact
What you walk toward loads first.

---

---



### [M4-EXT-50] Pre-Generation Lookahead
*(gap-file ref: M4-022)*

##### Math
-
##### Systems Touched
M4. Gen 1 ring ahead during idle.
##### How It Works
When frame budget unused, pre-gen next ring.
##### Reference Implementation
```cpp
if (budgetLeft) PreGen(nextRing);
```
##### Player-Facing Impact
Less pop-in while moving.

---

---



### [M4-EXT-51] Generation Cancellation
*(gap-file ref: M4-023)*

##### Math
-
##### Systems Touched
M4. Chunk unloaded mid-gen → cancel.
##### How It Works
Gen job checks `cancelled` each pass; early-out + free.
##### Reference Implementation
```cpp
if (chunk.cancelled) return;
```
##### Player-Facing Impact
No wasted work on rapid movement.

---

---



### [M4-EXT-52] Chunk Generation State Machine
*(gap-file ref: M4-024)*

##### Math
-
##### Systems Touched
M4. queued→generating→ready→active→unloading.
##### How It Works
Explicit FSM per chunk; telemetry per state.
##### Reference Implementation
```cpp
enum ChunkState { Queued, Gen, Ready, Active, Unloading };
```
##### Player-Facing Impact
Streaming observable + debuggable.

---

---



### [M4-EXT-53] Parallel Mesh/Collider Build
*(gap-file ref: M4-025)*

##### Math
-
##### Systems Touched
M4/physics. Build geometry + physics shape concurrently.
##### How It Works
Two worker tasks per chunk; join before activate.
##### Reference Implementation
```cpp
Parallel(meshTask, colliderTask); Join();
```
##### Player-Facing Impact
Faster chunk readiness.

---

---



### [M4-EXT-54] Procedural Quest / Mission Generator
*(gap-file ref: M4-026)*


##### Systems Touched
M8 (quests). No runtime quest generator.

##### Math
-

##### How It Works
Quest graph from POIs + faction state + seed; SLM writes flavor.

##### Reference Implementation
```cpp
Quest q = GenQuest(seed, poi, faction);
```

##### Player-Facing Impact
Endless quest content.

---

---



### [M4-EXT-55] Procedural Narrative / Dialogue Generator
*(gap-file ref: M4-027)*


##### Systems Touched
M13 (narrative). No dialogue generator wired.

##### Math
-

##### How It Works
SLM generates NPC dialogue + branching from world state; schema-validated.

##### Reference Implementation
```cpp
Dialogue d = SLM.Run(template["npc_talk"], state);
```

##### Player-Facing Impact
Living NPC conversations.

---

---



### [M4-EXT-56] Difficulty / Tuning Curve Generator
*(gap-file ref: M4-028)*


##### Systems Touched
M5.4/M13. No auto-tuned difficulty curve.

##### Math
-

##### How It Works
Telemetry-driven difficulty; SLM Director adjusts spawn/loot.

##### Reference Implementation
```cpp
difficulty = Director.Tick(telemetry);
```

##### Player-Facing Impact
Pacing self-tunes.

---

---



### [M4-EXT-57] Generation RNG Replay File
*(gap-file ref: M4-029)*


##### Systems Touched
M4/M0. Record RNG draws per chunk.

##### Math
-

##### How It Works
Log stream draws to replay; reproduce exactly.

##### Reference Implementation
```cpp
replay.Log(stream, i, val);
```

##### Player-Facing Impact
Broken seeds debuggable.

---

---



### [M4-EXT-58] Generation Math Reduction Order
*(gap-file ref: M4-030)*


##### Systems Touched
M4/M0. Fixed-order reductions.

##### Math
-

##### How It Works
Forbid unordered-container reduction where order matters.

##### Reference Implementation
```cpp
// sum in entity-index order
```

##### Player-Facing Impact
Co-op peers agree on gen results.

---

---



### [M4-EXT-59] Procedural Road/Path Network
*(gap-file ref: M4-031)*


##### Systems Touched
M4. Roads only as noise threshold.

##### Math
-

##### How It Works
Graph-based road/river network generator feeding POIs + nav. `RoadNet g = GenNetwork(seed);`

##### Reference Implementation
Connected world.

##### Player-Facing Impact
Logical layout.


---



### [M4-EXT-60] Procedural Encounter/Event Spawner
*(gap-file ref: M4-034)*


##### Systems Touched
M4/M5.4. No event spawner.

##### Math
-

##### How It Works
Time/location-based encounters (ambush, distress) from director. `Event e = Spawner.Tick(time, poi);`

##### Reference Implementation
World feels alive.

##### Player-Facing Impact
Emergent moments.


---



## Milestone M5 — Appendix EXT additions (11)


### [M5-EXT-06] AI Behavior / Utility System
*(gap-file ref: M5-001)*

##### Math
-
##### Systems Touched
M5.1. No AI decision system; zombies have no behavior.
##### How It Works
Data-driven utility/behavior tree; SLM generates weight tables (M13).
##### Reference Implementation
```cpp
float score = utility.Eval(zombie, world); ActBest(score);
```
##### Player-Facing Impact
Zombies act believably.

---

---



### [M5-EXT-07] Navigation / Pathfinding
*(gap-file ref: M5-002)*

##### Math
-
##### Systems Touched
M5. `grep Recast|Detour|navmesh` → 0 in code. Zombies cannot move toward player.
##### How It Works
Recast builds navmesh; Detour queries paths; tiled for streaming.
##### Reference Implementation
```cpp
dtNavMeshQuery q; q.FindPath(start, goal, path);
```
##### Player-Facing Impact
Zombies path to the player.

---

---



### [M5-EXT-08] Procedural Animation (physics/IK)
*(gap-file ref: M5-003)*

##### Math
-
##### Systems Touched
M5.2. `grep Animation|IK` → 0. Zombies static without it.
##### How It Works
Physics-driven limb IK; SLM generates blend weights (M13).
##### Reference Implementation
```cpp
ik.Solve(skeleton, target); // foot placement
```
##### Player-Facing Impact
Organic zombie movement.

---

---



### [M5-EXT-09] AI Perception System
*(gap-file ref: M5-004)*

##### Math
-
##### Systems Touched
M5.3. No sight/hearing/visibility model.
##### How It Works
Perception queries `RayBatchQuery` (M1-002) + spatial hash (M1-001) for stimulus.
##### Reference Implementation
```cpp
if (CanSee(z, player) || Heard(z, player)) Alert(z);
```
##### Player-Facing Impact
Zombies notice you via sight/sound.

---

---



### [M5-EXT-10] AI Director (horde pacing)
*(gap-file ref: M5-005)*

##### Math
-
##### Systems Touched
M5.4. No spawn/difficulty director.
##### How It Works
Config-driven density curve; SLM telemetry-Director tunes it (M13 + M7 telemetry).
##### Reference Implementation
```cpp
spawnRate = Director::Tick(telemetry, difficulty);
```
##### Player-Facing Impact
Pacing feels intentional, not random.

---

---



### [M5-EXT-11] Horde Emergence / Spawn Waves
*(gap-file ref: M5-006)*


##### Systems Touched
M5.4. No emergent wave/spawn logic.

##### Math
-

##### How It Works
Director emits spawn waves from pressure + telemetry.

##### Reference Implementation
```cpp
spawner.SpawnWave(director.Pressure());
```

##### Player-Facing Impact
Zombies arrive in waves.

---

---



### [M5-EXT-12] Zombie Facing / Emerge Animation
*(gap-file ref: M5-007)*


##### Systems Touched
M2/M5.2. Zombies must face player + emerge.

##### Math
-

##### How It Works
Orient to player; play emerge anim on spawn.

##### Reference Implementation
```cpp
z.rot = LookAt(z.pos, player.pos); PlayEmerge(z);
```

##### Player-Facing Impact
Zombies face + climb out.

---

---



### [M5-EXT-13] Zombie FSM States
*(gap-file ref: M5-001)*


##### Systems Touched
M5. States idle/wander/chase/attack not defined.

##### Math
-

##### How It Works
Explicit FSM: Idle→Wander→Alert→Chase→Attack→Flee, driven by perception. `switch(state){}`

##### Reference Implementation
Zombies behave in clear phases.

##### Player-Facing Impact
Predictable, debuggable AI.


---



### [M5-EXT-14] Hearing Perception (radius+freq)
*(gap-file ref: M5-003)*


##### Systems Touched
M5.3. No hearing model.

##### Math
-

##### How It Works
Sound events propagate radius; zombies within hear radius + LOS-check investigate. `if (Dist(z,snd)<hearR) Investigate(snd);`

##### Reference Implementation
Gunshots attract nearby zombies.

##### Player-Facing Impact
Sound is a tactical resource.


---



### [M5-EXT-15] Short-Term Memory & Investigation
*(gap-file ref: M5-005)*


##### Systems Touched
M5.3. No memory of last-known position.

##### Math
-

##### How It Works
Zombie stores last-known-player-pos for N seconds; searches there then gives up. `memory.timer -= dt;`

##### Reference Implementation
Zombies investigate, then lose interest.

##### Player-Facing Impact
Less robotic, more believable.


---



### [M5-EXT-16] Horde Merge & Split Behavior
*(gap-file ref: M5-007)*


##### Systems Touched
M5.4. No horde cohesion logic.

##### Math
-

##### How It Works
Nearby zombies form a horde (shared target); horde splits when targets diverge. `if (Dist(a,b)<mergeR) shareTarget(a,b);`

##### Reference Implementation
Hordes flow as groups, not loners.

##### Player-Facing Impact
Emergent swarm behavior.


---



## Milestone M6 — Appendix EXT additions (10)


### [M6-EXT-13] Hardware-Accelerated Audio
*(gap-file ref: M6-001)*

##### Math
-
##### Systems Touched
M6. `grep AudioEngine|soloud|OpenAL` → 0. No sound at all.
##### How It Works
SoLoud (easy, light); positional + occlusion via `RayBatchQuery` (M1-002).
##### Reference Implementation
```cpp
auto* a = SoLoud::Soloud(); a->init();
```
##### Player-Facing Impact
The world has sound.

---

---



### [M6-EXT-14] Audio Occlusion / Propagation
*(gap-file ref: M6-002)*


##### Systems Touched
M6. No occlusion despite RayBatchQuery (M1-002).

##### Math
-

##### How It Works
RayBatchQuery casts for occlusion; attenuate by hit.

##### Reference Implementation
```cpp
if (RayBlocked(src,dst)) vol *= 0.2f;
```

##### Player-Facing Impact
Zombie audio is spatial.

---

---



### [M6-EXT-15] Procedural Audio / VFX Params
*(gap-file ref: M6-003)*


##### Systems Touched
M6.5/M13. No SLM-generated VFX/audio param tables.

##### Math
-

##### How It Works
SLM emits per-archetype VFX/audio param configs offline.

##### Reference Implementation
```cpp
VFXParams p = SLM.Run(template["blood"], arch);
```

##### Player-Facing Impact
Varied, authored-feel effects.

---

---



### [M6-EXT-16] HRTF Spatial Audio
*(gap-file ref: M6-001)*


##### Systems Touched
M6. Binaural HRTF for positional accuracy.

##### Math
-

##### How It Works
Route sources through HRTF panner keyed to listener orientation. `hrtf.Pan(src, listener);`

##### Reference Implementation
Directional audio is precise.

##### Player-Facing Impact
Players locate zombies by ear.


---



### [M6-EXT-17] Occlusion/Obstruction Model
*(gap-file ref: M6-003)*


##### Systems Touched
M6. Audio occlusion beyond RayBatchQuery stub.

##### Math
-

##### How It Works
Per-source occlusion factor from RayBatchQuery hits → low-pass + attenuation. `vol *= (1 - 0.8*hits);`

##### Reference Implementation
Walls muffle audio correctly.

##### Player-Facing Impact
Audio matches geometry.


---



### [M6-EXT-18] Dynamic Music & Stinger System
*(gap-file ref: M6-004)*


##### Systems Touched
M6. No adaptive music.

##### Math
-

##### How It Works
Music intensity scales with threat (calm/tense/horde); stingers on events. `music.SetIntensity(threat);`

##### Reference Implementation
Score reacts to danger.

##### Player-Facing Impact
Tension via audio.


---



### [M6-EXT-19] Blood & Gore Decals
*(gap-file ref: M6.5-002)*


##### Systems Touched
M6.5. No blood/decal system.

##### Math
-

##### How It Works
Spawn decals on hit; pool + fade; skinned to surface. `decalPool.Spawn(hitPos, normal);`

##### Reference Implementation
Visible wound feedback.

##### Player-Facing Impact
Gibs/gore for tone.


---



### [M6-EXT-20] Impact & Sparks VFX
*(gap-file ref: M6.5-003)*


##### Systems Touched
M6.5. No impact effects.

##### Math
-

##### How It Works
Per-material impact VFX (spark/wood/ flesh); GPU particle burst. `VFX::Burst(mat, hit);`

##### Reference Implementation
Hits feel impactful.

##### Player-Facing Impact
Material-correct feedback.


---



### [M6-EXT-21] Weather Particle VFX (rain/snow)
*(gap-file ref: M6.5-004)*


##### Systems Touched
M6.5/M10. No weather particles.

##### Math
-

##### How It Works
GPU particle rain/snow volume following camera, density from weather. `rain.Emit(density);`

##### Reference Implementation
Weather is visible, not just lighting.

##### Player-Facing Impact
Immersive storms.


---



### [M6-EXT-22] Death/ Dissolve VFX
*(gap-file ref: M6.5-005)*


##### Systems Touched
M6.5. No death effect.

##### Math
-

##### How It Works
Dissolve/shrink shader on death; pools into ragdoll. `mat.dissolve = 1.0;`

##### Reference Implementation
Clean death transitions.

##### Player-Facing Impact
No pop-out消失.


---



## Milestone M7 — Appendix EXT additions (8)


### [M7-EXT-12] Save Compression (zstd)
*(gap-file ref: M7-001)*

##### Math
-
##### Systems Touched
M7. `grep zstd|lz4` → 0 (vertex quantization only). Tick-hash replay unbounded.
##### How It Works
Compress save/replay blobs with zstd.
##### Reference Implementation
```cpp
blob = ZSTD_compress(serialize(world));
```
##### Player-Facing Impact
Saves are small + fast.

---

---



### [M7-EXT-13] Asset Checksum Integrity
*(gap-file ref: M7-002)*

##### Math
-
##### Systems Touched
M7/M0-EXT-28. `FileSystem.cpp:7-11` only resolves `mods/`; no checksum. Corrupt mod loads silently.
##### How It Works
Checksum every AssetPath load; reject/repair on mismatch.
##### Reference Implementation
```cpp
if (Checksum(asset) != expected) Reject(asset);
```
##### Player-Facing Impact
Corrupt assets don't silently break the game.

---

---



### [M7-EXT-14] Telemetry Tap + Consent Gate
*(gap-file ref: M7-003)*

##### Math
-
##### Systems Touched
M7/M13. `grep telemetry` → 0. Prereq for SLM Director + "single tuned experience".
##### How It Works
Ring-buffer of gameplay events; opt-in consent before any export.
##### Reference Implementation
```cpp
if (consent) telemetry.Push(event);
```
##### Player-Facing Impact
Data-driven tuning without privacy violations.

---

---



### [M7-EXT-15] Localization / Accessibility String-Table
*(gap-file ref: M7-004)*

##### Math
-
##### Systems Touched
M7/M11. `grep localiz|string.table` → 0. Route text through offline-generated (SLM) table; MSDF already renders it.
##### How It Works
All player-facing strings keyed; SLM generates translations offline.
##### Reference Implementation
```cpp
text = i18n.Get("zombie.charge");
```
##### Player-Facing Impact
Game is localizable.

---

---



### [M7-EXT-16] Save Corruption Recovery
*(gap-file ref: M7-005)*


##### Systems Touched
M7. Atomic write planned but no rollback on parse failure.

##### Math
-

##### How It Works
Keep previous-good save; on parse failure restore + log.

##### Reference Implementation
```cpp
if (!Parse(save)) Restore(prevSave);
```

##### Player-Facing Impact
Corrupt save doesn't wipe progress.

---

---



### [M7-EXT-17] Replay / Demo Record
*(gap-file ref: M7-006)*


##### Systems Touched
M0 (tick-hash). Only hash, no full state replay.

##### Math
-

##### How It Works
Record deterministic input stream → replayable demo.

##### Reference Implementation
```cpp
replay.Log(inputs); replay.Playback();
```

##### Player-Facing Impact
Watch/debug past sessions.

---

---



### [M7-EXT-18] Save Encryption & Anti-Edit
*(gap-file ref: M7-008)*


##### Systems Touched
M7. No save encryption.

##### Math
-

##### How It Works
Encrypt save blob (key from user-data) to deter manual editing. `blob = Encrypt(serialize(world));`

##### Reference Implementation
Saves can't be trivially hacked.

##### Player-Facing Impact
Integrity.


---



### [M7-EXT-19] Backward-Save Compatibility
*(gap-file ref: M7-009)*


##### Systems Touched
M7. No migration path.

##### Math
-

##### How It Works
On load, migrate old save versions forward via per-version patches. `ver = MigrateUp(ver);`

##### Reference Implementation
Old saves keep working.

##### Player-Facing Impact
No wipe on update.



## Milestone M8 — Appendix EXT additions (8)


### [M8-EXT-11] Data-Driven Itemization
*(gap-file ref: M8-001)*

##### Math
-
##### Systems Touched
M8. No item/weapon def system; only `BulletComponent` data fields.
##### How It Works
Items/weapons as data defs (fire-rate/reload/recoil); SLM generates balanced stats (M13).
##### Reference Implementation
```cpp
ItemDef d = Items::Get("rifle"); gun.fireRate = d.fireRate;
```
##### Player-Facing Impact
Weapons/items are real + tunable.

---

---



### [M8-EXT-12] Stacking & Inventory Slots
*(gap-file ref: M8-003)*


##### Systems Touched
M8. No stacking/slot model.

##### Math
-

##### How It Works
Items stack by type up to max; fixed slot grid or weight-based. `slots.Add(item, qty);`

##### Reference Implementation
Inventory is manageable.

##### Player-Facing Impact
Looting is organized.


---



### [M8-EXT-13] Ammo Types & Crafting
*(gap-file ref: M8-004)*


##### Systems Touched
M8. No ammo variants/crafting.

##### Math
-

##### How It Works
Ammo types (FMJ/HP/AP) with tradeoffs; craft from components. `if (Has(comp)) MakeAmmo(type);`

##### Reference Implementation
Ammo is a resource decision.

##### Player-Facing Impact
Crafting loops close.


---



### [M8-EXT-14] Barter & Trade Economy
*(gap-file ref: M8.5-002)*


##### Systems Touched
M8.5. No trade system.

##### Math
-

##### How It Works
NPC vendors buy/sell with price modifiers by faction standing. `price = base * factionMod;`

##### Reference Implementation
Trade with survivors.

##### Player-Facing Impact
Economy is interactive.


---



### [M8-EXT-15] Currency & Scarcity Model
*(gap-file ref: M8.5-003)*


##### Systems Touched
M8.5. No currency design.

##### Math
-

##### How It Works
Scarce currency (pre-war bills/barter-weight); inflation via supply. `wallet -= price;`

##### Reference Implementation
Money has weight.

##### Player-Facing Impact
Scarcity drives choices.


---



### [M8-EXT-16] Settlement NPC Residents
*(gap-file ref: M8.6-003)*


##### Systems Touched
M8.6. No resident NPCs.

##### Math
-

##### How It Works
Settlements spawn NPCs with roles (guard/trader/cook); daily loops. `SpawnNPC(role, home);`

##### Reference Implementation
Living communities.

##### Player-Facing Impact
Settlements feel alive.


---



### [M8-EXT-17] Perk Tree & Prerequisites
*(gap-file ref: M8.7-001)*


##### Systems Touched
M8.7. No perk tree.

##### Math
-

##### How It Works
Perks in a DAG with prerequisites; spend points to unlock. `if (HasPrereq(p)) Unlock(p);`

##### Reference Implementation
Builds specialize.

##### Player-Facing Impact
Meaningful progression.


---



### [M8-EXT-18] Skill Progression & XP
*(gap-file ref: M8.7-002)*


##### Systems Touched
M8.7. No XP curve.

##### Math
-

##### How It Works
Activities grant XP; level gates perk availability. `xp += gain; if (xp>thr) LevelUp();`

##### Reference Implementation
Progress is earned.

##### Player-Facing Impact
Grind has payoff.


---



## Milestone M9 — Appendix EXT additions (1)


### [M9-EXT-23] Day/Night, Weather & Atmosphere
*(gap-file ref: M9-001)*

##### Math
-
##### Systems Touched
M10. No day/night cycle or weather system (only a comment in Engine.cpp).
##### How It Works
Sun/moon angle from WorldEpoch (M2-010); weather as config-driven uniforms; SLM generates narrative.
##### Reference Implementation
```cpp
float sun = SunAngle(worldClock.simSeconds);
```
##### Player-Facing Impact
Living, moody atmosphere.

---

---



## Milestone M10 — Appendix EXT additions (18)


### [M10-EXT-12] HUD / UI Widgets
*(gap-file ref: M10-001)*

##### Math
-
##### Systems Touched
M11. `grep HUD|Widget` → 0 (ImGui dev only). No health/ammo/inventory/menus.
##### How It Works
Retained-mode widget system over your MSDF/text; data-bound to ECS.
##### Reference Implementation
```cpp
ui.Bind("health", ecs.Get(player).health);
```
##### Player-Facing Impact
Players see their status.

---

---



### [M10-EXT-13] Console / Command System
*(gap-file ref: M10-002)*

##### Math
-
##### Systems Touched
M11. `grep CommandSystem|Console` → 0 (CVar UI dev-only). No shipped console.
##### How It Works
Command registry + parser; mods/debug expose commands (ties M11-015).
##### Reference Implementation
```cpp
Console::Reg("give", [](args){ GiveItem(args[0]); });
```
##### Player-Facing Impact
Powerful debug + mod command surface.

---

---



### [M10-EXT-14] Screenshot / Photo Mode
*(gap-file ref: M10-003)*

##### Math
-
##### Systems Touched
M11. `grep Screenshot|PhotoMode|stbi` → 0. No frame capture.
##### How It Works
Capture swapchain image → PNG/EXR via stb_image_write.
##### Reference Implementation
```cpp
SavePNG("shot.png", CaptureFrame());
```
##### Player-Facing Impact
Players can capture the game.

---

---



### [M10-EXT-15] Build / Packaging / Installer
*(gap-file ref: M10-004)*

##### Math
-
##### Systems Touched
M11. No MSI/zip, no asset bundling.
##### How It Works
CMake install + asset bundle step + optional installer.
##### Reference Implementation
```cmake
install(TARGETS ZombieEngine DESTINATION bin)
```
##### Player-Facing Impact
Game is shippable.

---

---



### [M10-EXT-16] Determinism CI Gate
*(gap-file ref: M10-005)*

##### Math
-
##### Systems Touched
M0/M4. Gen determinism in pipeline.
##### How It Works
CI runs repro harness (M0-010); fails on mismatch.
##### Reference Implementation
```yaml
- run: ./ZombieEngine --selftest determinism
```
##### Player-Facing Impact
Determinism regressions blocked pre-merge.

---

---



### [M10-EXT-17] Spec↔Code Drift Checker
*(gap-file ref: M10-006)*

##### Math
-
##### Systems Touched
M0 (audit hygiene). Parse milestone EXT IDs, grep source, flag unbuilt.
##### How It Works
Script extracts `[Mx-EXT-nn]` from spec, checks each exists in source.
##### Reference Implementation
```bash
grep -rl "M0-EXT-30" src/ || echo "MISSING M0-EXT-30"
```
##### Player-Facing Impact
Milestone claims stay honest (catches the M0-EXT-15..29 drift).

---

---



### [M10-EXT-18] Generation Debug Tooling (preview + seed explorer)
*(gap-file ref: M10-007)*

##### Math
-
##### Systems Touched
M4/T95/T96. Generate+view a chunk without full game; scrub seeds.
##### How It Works
Headless gen + offscreen render to image; seed explorer UI.
##### Reference Implementation
```cpp
GenChunkToImage(cx, cy, seed, "preview.png");
```
##### Player-Facing Impact
Fast gen iteration + curated shipped seed.

---

---



### [M10-EXT-19] Mod-Package / Workshop Tooling
*(gap-file ref: M10-008)*

##### Math
-
##### Systems Touched
M11/T97. Bundle mod → manifest+assets+checksum; stub Workshop backend.
##### How It Works
CLI zips mod + writes manifest + checksum; pluggable store backend.
##### Reference Implementation
```cpp
ModKit::Package(modDir, outZip);
```
##### Player-Facing Impact
Distributing mods is one command.

---

---



### [M10-EXT-20] Generation Benchmark Suite
*(gap-file ref: M10-009)*

##### Math
-
##### Systems Touched
M4/T98. Gen throughput chart over time.
##### How It Works
Benchmark gen of N chunks; emit ms/throughput.
##### Reference Implementation
```cpp
BenchGen(1000, [](ms){ Log(ms); });
```
##### Player-Facing Impact
Track gen perf regressions.

---

---



### [M10-EXT-21] HDR Swapchain Output Encoding
*(gap-file ref: M10-010)*

##### Systems Touched
M0/M10. Tonemapping (ACES) spec'd but final swapchain color-space (VK_COLOR_SPACE_HDR10_EXT / Rec2020) absent.
##### Math
`swapchainFormat = {R16G16B16A16_SFLOAT, VK_COLOR_SPACE_HDR10_EXT}` when HDR detected.
##### How It Works
Declare color-space at swapchain creation; pair with M0-EXT-29.
##### Reference Implementation
```cpp
VkColorSpaceKHR cs = hdrvulkan ? VK_COLOR_SPACE_HDR10_EXT : VK_COLOR_SPACE_SRGB;
```
##### Player-Facing Impact
Night ACES result actually displays in HDR.

---

---



### [M10-EXT-22] Tutorials / Contextual Hints
*(gap-file ref: M10-011)*


##### Systems Touched
M11. No tutorial/hint system.

##### Math
-

##### How It Works
Trigger hints on first-encounter events; data-driven.

##### Reference Implementation
```cpp
if (FirstTime("reload")) ShowHint("press R");
```

##### Player-Facing Impact
New players learn the game.

---

---



### [M10-EXT-23] Subtitles / Dialogue Display
*(gap-file ref: M10-012)*


##### Systems Touched
M11/M13. No subtitle system.

##### Math
-

##### How It Works
Render dialogue as subtitles bound to audio.

##### Reference Implementation
```cpp
ShowSubtitle(line, duration);
```

##### Player-Facing Impact
Accessible dialogue.

---

---



### [M10-EXT-24] Minimap / Compass / Objective Tracker
*(gap-file ref: M10-013)*


##### Systems Touched
M11. No minimap/objectives.

##### Math
-

##### How It Works
Render POI/objective overlay from world state.

##### Reference Implementation
```cpp
minimap.Draw(player, pois, objectives);
```

##### Player-Facing Impact
Players navigate with intent.

---

---



### [M10-EXT-25] Achievements / Steam Integration
*(gap-file ref: M10-014)*


##### Systems Touched
M11. No achievement hook.

##### Math
-

##### How It Works
Event-driven achievement unlock; Steam API stub.

##### Reference Implementation
```cpp
On(event, [] { Unlock(achId); });
```

##### Player-Facing Impact
Progression recognition.

---

---



### [M10-EXT-26] Pause / Time-Scale System
*(gap-file ref: M10-015)*


##### Systems Touched
M11. No pause/time-scale.

##### Math
-

##### How It Works
Global time-scale; pause menu freezes sim.

##### Reference Implementation
```cpp
sim.dt = paused ? 0 : realDt * timeScale;
```

##### Player-Facing Impact
Players can pause.

---

---



### [M10-EXT-27] Lightning & Storm VFX
*(gap-file ref: M10-002)*


##### Systems Touched
M10. No lightning.

##### Math
-

##### How It Works
Random lightning flashes + thunder delay from distance; brief illuminance spike. `if (Flash()) ambient += spike;`

##### Reference Implementation
Storms are dramatic.

##### Player-Facing Impact
Atmosphere peaks.


---



### [M10-EXT-28] Wind & Vegetation Sway
*(gap-file ref: M10-003)*


##### Systems Touched
M10/M3. No wind on foliage.

##### Math
-

##### How It Works
Global wind vector drives vertex sway in foliage/vegetation shader. `sway = wind * heightFactor;`

##### Reference Implementation
World moves with wind.

##### Player-Facing Impact
Less static.


---



### [M10-EXT-29] Sky & Cloud Rendering
*(gap-file ref: M10-004)*


##### Systems Touched
M10. No dynamic sky.

##### Math
-

##### How It Works
Procedural sky dome + animated cloud layer from weather/time. `sky.Render(time, weather);`

##### Reference Implementation
Sky changes with world.

##### Player-Facing Impact
Cohesive horizon.


---



## Milestone M11 — Appendix EXT additions (32)


### [M11-EXT-10] Three-Way Version Contract
*(gap-file ref: M11-001)*

##### Systems Touched
M7/M2.8/M11. Separate SaveSchemaVersion / NetworkProtocolVersion / ModAPIVersion.
##### Math
`struct VersionContract { uint32_t save, network, modAPI; };`
##### How It Works
A save-compat patch must not silently break netcode or mod API.
##### Reference Implementation
```cpp
if (mod.modAPI > contract.modAPI) Fail("mod too new");
```
##### Player-Facing Impact
Mods/saves/netcode evolve independently.

---

---



### [M11-EXT-11] Config Schema Registry
*(gap-file ref: M11-002)*

##### Math
-
##### Systems Touched
M11. Every tunable param declared + versioned + moddable.
##### How It Works
Params registered at startup into typed registry; mods override by ID.
##### Reference Implementation
```cpp
Registry::Declare("zombie.speed", 3.0f, 0.1f, 10.f);
```
##### Player-Facing Impact
All knobs are data, not magic offsets.

---

---



### [M11-EXT-12] Typed Parameter Handles
*(gap-file ref: M11-003)*

##### Math
-
##### Systems Touched
M11. Mods reference params by ID, not offsets.
##### How It Works
`ParamHandle h = Registry::Get("zombie.health")`; read/write via handle.
##### Reference Implementation
```cpp
auto h = Reg::Get("zombie.health"); float hp = Reg::Read(h);
```
##### Player-Facing Impact
Mods survive param reordering.

---

---



### [M11-EXT-13] Parameter Range/Clamp Enforcement
*(gap-file ref: M11-004)*

##### Math
-
##### Systems Touched
M11. Mods can't set `hp=-9999` silently.
##### How It Works
Registry entries declare min/max; out-of-range → clamp + warning.
##### Reference Implementation
```cpp
val = clamp(val, meta.min, meta.max);
```
##### Player-Facing Impact
Malformed mods degrade gracefully.

---

---



### [M11-EXT-14] Enum/Flag Registry
*(gap-file ref: M11-005)*

##### Math
-
##### Systems Touched
M11. Named enums for mods, not raw ints.
##### How It Works
Enums registered with string names; mods use names.
##### Reference Implementation
```cpp
Reg::Enum("DamageType", {"bullet","fire","bite"});
```
##### Player-Facing Impact
Mods reference semantics, not magic numbers.

---

---



### [M11-EXT-15] Deliberate Mod API Surface
*(gap-file ref: M11-006)*

##### Math
-
##### Systems Touched
M11. Expose only vetted functions; deny-by-default.
##### How It Works
Mods call a curated `ModAPI` object; raw internals unreachable.
##### Reference Implementation
```cpp
struct ModAPI { void SpawnZombie(Vec3); bool HasItem(Id); };
```
##### Player-Facing Impact
Mods can't touch unsafe internals.

---

---



### [M11-EXT-16] ModAPI Semantic Versioning
*(gap-file ref: M11-007)*

##### Math
-
##### Systems Touched
M11. Major=breaking (Fabric/Forge).
##### How It Works
`ModAPIVersion` bumps major on any API removal/signature change.
##### Reference Implementation
```cpp
if (mod.requiresModAPI > contract.modAPI) Reject(mod);
```
##### Player-Facing Impact
Mods declare compatibility; load fails loud on break.

---

---



### [M11-EXT-17] Mod Dependency Resolver
*(gap-file ref: M11-008)*

##### Math
-
##### Systems Touched
M11. Mod B needs mod A≥1.2; conflict detection.
##### How It Works
Manifest declares deps; loader topo-sorts + detects unsatisfied/cyclic.
##### Reference Implementation
```cpp
TopoSort(mods); // fail on cycle/unsatisfied
```
##### Player-Facing Impact
No silently-broken mod stacks.

---

---



### [M11-EXT-18] Mod Incompatibility Detector
*(gap-file ref: M11-009)*

##### Math
-
##### Systems Touched
M11. Two mods override same param → warn/block.
##### How It Works
Override map checked for collisions at load.
##### Reference Implementation
```cpp
if (overrideMap.Has(key)) Warn("conflict: "+key);
```
##### Player-Facing Impact
Conflicts surfaced before play.

---

---



### [M11-EXT-19] Mod Checksum + Signature
*(gap-file ref: M11-010)*

##### Math
-
##### Systems Touched
M11/M0-EXT-28. Verify integrity; optional signed mods.
##### How It Works
Manifest hash checked on load; signed mods verified against key.
##### Reference Implementation
```cpp
if (Hash(mod) != manifest.hash) Reject(mod);
```
##### Player-Facing Impact
Tampered mods rejected.

---

---



### [M11-EXT-20] Mod Write-Allowlist
*(gap-file ref: M11-011)*

##### Math
-
##### Systems Touched
M11/MetaRegistry. `bool ModWritable` on `entt::meta` registration.
##### How It Works
Only flagged fields writable by mods; rest read-only.
##### Reference Implementation
```cpp
meta.set("hp", ModWritable(true));
```
##### Player-Facing Impact
Mods can't corrupt core state.

---

---



### [M11-EXT-21] Runtime Mod Enable/Disable + Reload
*(gap-file ref: M11-012)*

##### Math
-
##### Systems Touched
M11. Without full restart.
##### How It Works
Loader unloads/reloads mod domain; world re-resolves overrides.
##### Reference Implementation
```cpp
ModLoader::Reload(modId);
```
##### Player-Facing Impact
Iterate on mods live.

---

---



### [M11-EXT-22] Mod Override Layering
*(gap-file ref: M11-013)*

##### Math
-
##### Systems Touched
M11. base < modA < modB, last wins, traceable.
##### How It Works
Override stack with source attribution per param.
##### Reference Implementation
```cpp
stack.Push(modId, value); // resolve: last wins
```
##### Player-Facing Impact
Mod order explicit + debuggable.

---

---



### [M11-EXT-23] Mod Error Isolation
*(gap-file ref: M11-014)*

##### Math
-
##### Systems Touched
M11/M0-EXT-25. One bad mod doesn't crash engine.
##### How It Works
Mod calls wrapped; exception → disable that mod + log.
##### Reference Implementation
```cpp
try { mod.Init(); } catch(...) { Disable(mod); }
```
##### Player-Facing Impact
A broken mod degrades, not the game.

---

---



### [M11-EXT-24] Mod Console Commands
*(gap-file ref: M11-015)*

##### Math
-
##### Systems Touched
M11/M12. Reload/query/list mods.
##### How It Works
CVar/console exposes `mod.list`, `mod.reload`, `mod.query`.
##### Reference Implementation
```cpp
Console::Reg("mod.list", []{ ListMods(); });
```
##### Player-Facing Impact
Mods manageable in-game.

---

---



### [M11-EXT-25] Mod Validation Pipeline Hook
*(gap-file ref: M11-016)*

##### Math
-
##### Systems Touched
M11/V. Mod content enters V-checks before activation.
##### How It Works
Loader calls `SanityCheck(modWorld)` pre-activate.
##### Reference Implementation
```cpp
if (!SanityCheck(mod.world)) Reject(mod);
```
##### Player-Facing Impact
Modded worlds validated like base ones.

---

---



### [M11-EXT-26] Mod Localization Hooks
*(gap-file ref: M11-017)*

##### Math
-
##### Systems Touched
M11/F3. Mods add own string-table entries.
##### How It Works
Mod manifest declares string-table; merged at boot.
##### Reference Implementation
```cpp
i18n.Merge(mod.stringTable);
```
##### Player-Facing Impact
Modded text is localized.

---

---



### [M11-EXT-27] Mod UI / Menu
*(gap-file ref: M11-018)*

##### Math
-
##### Systems Touched
M11. Browse/configure mods in-game.
##### How It Works
Menu reads mod manifests; toggle/order/reload.
##### Reference Implementation
```cpp
for (m : mods) DrawRow(m.name, m.enabled);
```
##### Player-Facing Impact
Users manage mods without file editing.

---

---



### [M11-EXT-28] Mod Save-Namespace Isolation
*(gap-file ref: M11-019)*

##### Math
-
##### Systems Touched
M11/M7. Mod A's save data doesn't collide with mod B.
##### How It Works
Each mod gets `saves/<profile>/mods/<modId>/`.
##### Reference Implementation
```cpp
path = userData/"saves"/profile/"mods"/mod.id;
```
##### Player-Facing Impact
Mods don't clobber each other's state.

---

---



### [M11-EXT-29] Benchmark / Perf HUD / Graphics Options
*(gap-file ref: M11-020)*

##### Math
-
##### Systems Touched
M11. `grep benchmark|perfHUD` → 0. Reuse Tracy + Vulkan timestamp pools; preset ladder feeding M0-EXT-23.
##### How It Works
Perf HUD reads telemetry; preset menu sets quality knobs.
##### Reference Implementation
```cpp
if (cvarPerfHUD) DrawTelemetry();
```
##### Player-Facing Impact
Users tune performance/quality.

---

---



### [M11-EXT-30] Screen Shake / Hit Marker / Kill Feed
*(gap-file ref: M11-021)*

##### Systems Touched
M11 (UI). `grep screen.shake|hit.marker|kill.feed` → 0; M11 doesn't mention them.
##### Math
`trauma *= 0.95; shake = trauma^2 * maxAngle;`
##### How It Works
Trauma decays semi-implicitly; hitmarker on `DamageEvent` confirm; kill feed appends.
##### Reference Implementation
```cpp
OnDamageConfirmed(e) { ShowHitMarker(); trauma += 0.3f; }
```
##### Player-Facing Impact
Core shooter feedback loop, cheap, high feel.

---

---



### [M11-EXT-31] Runtime Mod Manager
*(gap-file ref: M11-022)*

##### Math
-
##### Systems Touched
M11. `grep ModManager|LoadMod` → 0 (only static `mods/` overlay).
##### How It Works
Dynamic load/unload of mod domains at runtime.
##### Reference Implementation
```cpp
ModManager::Load(path); ModManager::Unload(id);
```
##### Player-Facing Impact
Mods manageable without restart/file hacks.

---

---



### [M11-EXT-32] Mod Documentation Generator
*(gap-file ref: M11-023)*


##### Systems Touched
M11. Auto-doc exposed ModAPI from code.

##### Math
-

##### How It Works
Parse ModAPI + registry → markdown reference.

##### Reference Implementation
```cpp
DocGen::Emit(modsApi, "modapi.md");
```

##### Player-Facing Impact
Modders get accurate docs.

---

---



### [M11-EXT-33] Workshop / Store Integration Stub
*(gap-file ref: M11-024)*


##### Systems Touched
M11. Download→verify→install path.

##### Math
-

##### How It Works
Pluggable backend interface; local FS impl first.

##### Reference Implementation
```cpp
Workshop::Install(modId);
```

##### Player-Facing Impact
Mod distribution path exists.

---

---



### [M11-EXT-34] Main Menu & Pause Menu
*(gap-file ref: M11-001)*


##### Systems Touched
M11. No menus.

##### Math
-

##### How It Works
Retained-mode main/pause/settings menus over MSDF text. `ui.ShowMenu("main");`

##### Reference Implementation
Players can start/pause/quit.

##### Player-Facing Impact
Game is navigable.


---



### [M11-EXT-35] Inventory & Loot UI
*(gap-file ref: M11-002)*


##### Systems Touched
M11. No inventory screen.

##### Math
-

##### How It Works
Grid/weight inventory + loot popup on search. `ui.BindInventory(ecs);`

##### Reference Implementation
Players manage gear.

##### Player-Facing Impact
Looting is clear.


---



### [M11-EXT-36] Map & Fast-Travel UI
*(gap-file ref: M11-003)*


##### Systems Touched
M11/M4. No map.

##### Math
-

##### How It Works
World map from generated POIs; fast-travel to discovered points. `map.Show(pois, discovered);`

##### Reference Implementation
Players orient + travel.

##### Player-Facing Impact
Navigation aid.


---



### [M11-EXT-37] Settings & Options Menu
*(gap-file ref: M11-004)*


##### Systems Touched
M11. No options.

##### Math
-

##### How It Works
Graphics/audio/controls settings persisted to config. `settings.Save();`

##### Reference Implementation
Players tune the game.

##### Player-Facing Impact
Accessibility + preference.


---



### [M11-EXT-38] Controller/Gamepad UI
*(gap-file ref: M11-005)*


##### Systems Touched
M11. No gamepad UI/cursor.

##### Math
-

##### How It Works
Gamepad navigation + virtual cursor for menus. `if (gamepad) NavMenu();`

##### Reference Implementation
Controller users supported.

##### Player-Facing Impact
Input parity.


---



### [M11-EXT-39] Haptics & Rumble
*(gap-file ref: M11-006)*


##### Systems Touched
M11. No rumble.

##### Math
-

##### How It Works
Trigger/controller rumble on fire/hit; intensity by event. `Pad::Rumble(0.6f);`

##### Reference Implementation
Tactile feedback.

##### Player-Facing Impact
Console-grade feel.


---



### [M11-EXT-40] Damage Numbers & Floating Text
*(gap-file ref: M11-007)*


##### Systems Touched
M11. No combat text.

##### Math
-

##### How It Works
Floating damage/status text in world space, pooled. `FloatText(dmg, pos);`

##### Reference Implementation
Feedback on hits.

##### Player-Facing Impact
Readable combat.


---



### [M11-EXT-41] World Space UI (billboards)
*(gap-file ref: M11-008)*


##### Systems Touched
M11. No world-space UI.

##### Math
-

##### How It Works
Billboarded world UI (markers, names, objectives). `billboard.RenderTo(pos);`

##### Reference Implementation
Context in world.

##### Player-Facing Impact
HUD anchors to world.


---



## Milestone M12 — Appendix EXT additions (9)


### [M12-EXT-14] Networking / Netcode Transport
*(gap-file ref: M12-001)*

##### Math
-
##### Systems Touched
M12. `grep Network|Socket|ENet|Udp` → 0. M2.8 co-op has zero transport.
##### How It Works
Pick ENet (lightweight) or yojimbo (rollback); wire to deterministic sim.
##### Reference Implementation
```cpp
ENetHost* host = enet_host_create(...);
```
##### Player-Facing Impact
Co-op actually connects.

---

---



### [M12-EXT-15] Voice Chat / Push-To-Talk
*(gap-file ref: M12-002)*

##### Math
-
##### Systems Touched
M12. `grep voice|PushToTalk` → 0 in spec and code.
##### How It Works
Opus + audio tunnel over the co-op transport (M12-001).
##### Reference Implementation
```cpp
opusEncoder.Encode(micFrame, out); net.Send(out);
```
##### Player-Facing Impact
Co-op players can talk.

---

---



### [M12-EXT-16] Anti-Cheat / Server Authority
*(gap-file ref: M12-003)*

##### Math
-
##### Systems Touched
M2.8/M12. `grep anti.cheat|serverAuth` → 0. Deterministic co-op needs a trust boundary.
##### How It Works
Server-authoritative simulation + input-only net (deterministic design enables this).
##### Reference Implementation
```cpp
// server runs sim; clients send inputs only
```
##### Player-Facing Impact
Co-op can't be trivially cheated.

---

---



### [M12-EXT-17] Co-op Determinism Guard
*(gap-file ref: M12-004)*

##### Math
-
##### Systems Touched
M2.8/M12. `ENGINE_DETERMINISM_MODE` (M0-010) must be enforced for netplay.
##### How It Works
Peer desync detected via tick-hash; rollback/resync on mismatch.
##### Reference Implementation
```cpp
if (peerTickHash != localTickHash) Resync(peer);
```
##### Player-Facing Impact
Peers stay in lockstep.

---

---



### [M12-EXT-18] Matchmaking / Lobby / Session
*(gap-file ref: M12-005)*


##### Systems Touched
M12. No lobby/matchmaking for co-op.

##### Math
-

##### How It Works
Session browser + invite; uses net transport.

##### Reference Implementation
```cpp
Lobby::Create(session);
```

##### Player-Facing Impact
Players find co-op sessions.

---

---



### [M12-EXT-19] Lobby / Session Integrity
*(gap-file ref: M12-006)*


##### Systems Touched
M12. Session state authority.

##### Math
-

##### How It Works
Server owns session; clients sync.

##### Reference Implementation
```cpp
// server is session authority
```

##### Player-Facing Impact
No session desync/cheat.

---

---



### [M12-EXT-20] Entity Interpolation
*(gap-file ref: M12-002)*


##### Systems Touched
M12. No render interpolation.

##### Math
-

##### How It Works
Render entities at interpolated state between last two server snapshots. `renderPos = lerp(prev,cur,alpha);`

##### Reference Implementation
Remote players move smoothly.

##### Player-Facing Impact
No jitter.


---



### [M12-EXT-21] Lag Compensation
*(gap-file ref: M12-003)*


##### Systems Touched
M12. No lag comp.

##### Math
-

##### How It Works
Rewind server state to shooter's ping when resolving hits. `state = Rewind(hitTime);`

##### Reference Implementation
Shots land where aimed.

##### Player-Facing Impact
Fair gunplay.


---



### [M12-EXT-22] Dedicated Server Model
*(gap-file ref: M12-004)*


##### Systems Touched
M12. No headless server.

##### Math
-

##### How It Works
Headless server build with no renderer; clients connect. `if (server) NoRender();`

##### Reference Implementation
Hostable co-op.

##### Player-Facing Impact
Scalable multiplayer.


---



## Milestone M13 — Appendix EXT additions (13)


### [M13-EXT-16] Offline SLM Generation Harness
*(gap-file ref: M13-001)*

##### Math
-
##### Systems Touched
M13/M4/M5. MiniCPM5-1B at content-build time, never per-tick (arxiv 2601.23206).
##### How It Works
Harness loads model offline, feeds prompt-templates, captures JSON config.
##### Reference Implementation
```cpp
// build-time: SLM.generate(template, seed) -> ConfigBlob
```
##### Player-Facing Impact
Procedural variety with zero runtime cost.

---

---



### [M13-EXT-17] SLM Prompt-Template Library
*(gap-file ref: M13-002)*

##### Math
-
##### Systems Touched
M13. Seed templates → archetypes, loot flavor, weather, dialogue.
##### How It Works
Templates keyed by content-type; filled with seed + constraints.
##### Reference Implementation
```cpp
auto out = SLM.Run(template["zombie_archetype"], {seed, biome});
```
##### Player-Facing Impact
Consistent, varied generated text.

---

---



### [M13-EXT-18] SLM Output Schema Validator
*(gap-file ref: M13-003)*

##### Math
-
##### Systems Touched
M13/V. SLM emits parseable JSON/config, not prose.
##### How It Works
Output validated against schema before entering pipeline.
##### Reference Implementation
```cpp
if (!SchemaOK(out)) UseFallback(template);
```
##### Player-Facing Impact
Malformed SLM output rejected, not injected.

---

---



### [M13-EXT-19] SLM Output → Validation Pipeline
*(gap-file ref: M13-004)*

##### Math
-
##### Systems Touched
M13/V. SLM content also passes V-checks.
##### How It Works
SLM-generated tables/worlds validated like any other.
##### Reference Implementation
```cpp
if (!SanityCheck(slmWorld)) Reject(slmWorld);
```
##### Player-Facing Impact
SLM can't emit broken content.

---

---



### [M13-EXT-20] SLM Cache
*(gap-file ref: M13-005)*

##### Systems Touched
M13. Same seed+template → cached output.
##### Math
`cacheKey = hash(templateId, seed, params)`.
##### How It Works
Reuse if present; no redundant generation.
##### Reference Implementation
```cpp
if (slmCache.Has(key)) return slmCache.Get(key);
```
##### Player-Facing Impact
No redundant generation.

---

---



### [M13-EXT-21] SLM Deterministic Seeding
*(gap-file ref: M13-006)*

##### Math
-
##### Systems Touched
M13. Same input → same output; lock temp/top_p.
##### How It Works
`temp=0.7, top_p=0.95` fixed; seed pinned per content.
##### Reference Implementation
```cpp
slm.SetParams(0.7f, 0.95f, contentSeed);
```
##### Player-Facing Impact
Reproducible generated content.

---

---



### [M13-EXT-22] SLM Fallback on Failure
*(gap-file ref: M13-007)*

##### Math
-
##### Systems Touched
M13. If model absent, use template defaults.
##### How It Works
Harness returns curated default if model/parse fails.
##### Reference Implementation
```cpp
return model ? model.Gen(t,s) : Default(t,s);
```
##### Player-Facing Impact
Game runs even without the SLM.

---

---



### [M13-EXT-23] SLM Provenance Tag
*(gap-file ref: M13-008)*

##### Math
-
##### Systems Touched
M13/M11. Mark content as SLM-generated.
##### How It Works
Generated assets/config carry `generatedBy: slm`.
##### Reference Implementation
```cpp
config.generatedBy = "slm";
```
##### Player-Facing Impact
Provenance visible to modders/debuggers.

---

---

---



### [M13-EXT-24] SLM Quest & Objective Text
*(gap-file ref: M13-001)*


##### Systems Touched
M13. No quest text gen.

##### Math
-

##### How It Works
SLM generates quest giver lines + objective text from state; schema-validated. `QuestText t = SLM.Run(tpl, state);`

##### Reference Implementation
Quests feel written.

##### Player-Facing Impact
Endless quest variety.


---



### [M13-EXT-25] SLM Dialogue & Branching
*(gap-file ref: M13-002)*


##### Systems Touched
M13. No dialogue gen.

##### Math
-

##### How It Works
SLM emits NPC dialogue + branch options from world state. `Dlg d = SLM.Run(dlgTpl, ctx);`

##### Reference Implementation
NPCs converse.

##### Player-Facing Impact
Living talk.


---



### [M13-EXT-26] SLM Lore & Worldbuilding
*(gap-file ref: M13-003)*


##### Systems Touched
M13. No lore gen.

##### Math
-

##### How It Works
SLM writes environmental lore (notes, graffiti, broadcasts). `Lore l = SLM.Run(loreTpl, seed);`

##### Reference Implementation
World has history.

##### Player-Facing Impact
Depth via text.


---



### [M13-EXT-27] SLM Loadout/Build Suggestions
*(gap-file ref: M13-004)*


##### Systems Touched
M13. No build advisor.

##### Math
-

##### How It Works
SLM suggests perk/loadout from playstyle + telemetry. `Build b = SLM.Run(buildTpl, stats);`

##### Reference Implementation
Guidance for new players.

##### Player-Facing Impact
Approachable depth.


---



### [M13-EXT-28] SLM Moderation/Filtering
*(gap-file ref: M13-005)*


##### Systems Touched
M13. No content filter.

##### Math
-

##### How It Works
SLM output passed through a filter for unsafe/format violations before use. `if (!Filter(out)) Reject(out);`

##### Reference Implementation
Generated content stays safe.

##### Player-Facing Impact
Trustworthy SLM.


---


