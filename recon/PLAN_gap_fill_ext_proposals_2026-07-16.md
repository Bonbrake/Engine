# Draft: Gap-Filling EXT Proposals (TI Gospel) — PLAN STAGE, NOT YET IN SPEC

> These are ready-to-paste EXT blocks for the GAPS identified in
> PLAN_threat_interactive_gospel_2026-07-16.md. Milestone spec files are READ-ONLY to
> the agent per standing gate — these are proposals for user GO, then paste into the
> correct milestone file (M4 / M4.5 / M4.6) following the 5-subsection skeleton +
> YAML-less H4 format + <a id> anchor + tags/tl;dr/meta/ctx.
>
> Each block below states: TARGET FILE, where to insert, and the block body.
> IDs are SUGGESTED (next free in cluster) — verify against live Mx.index.json before
> committing (no ID collisions).

═══════════════════════════════════════════════════════
GAP R2 — Base-color precision (target: M4.5, near M4.5-EXT-20)
═══════════════════════════════════════════════════════
#### [M4.5-EXT-22] GBuffer Base-Color Full-Precision Mandate (no 16-bit crush)
> **tags** · gbuffer, precision, TI-gospel
> **tl;dr** · Base-color/albedo carried at full material-source precision through the
>   lighting stage; never truncated to 16-bit between GBuffer write and lighting resolve.
>   TI: Crysis 3 "stores base color that is almost always 24 bits using only 16 bits" = fault.
> **ctx** · GBuffer Base-Color Full-Precision Mandate -- pairs with [M4.5-EXT-20] (Burley
>   shading). Prevents the TI-documented "specular instability" from mid-stage precision loss.
##### Systems Touched
GBuffer layout ([M4.5]), deferred lighting resolve, material system ([M4-EXT-14]).
##### Math
albedo_resolved = f16->f32(upstream_albedo) BEFORE lighting arithmetic; store GBuffer
albedo in RGBA16 (or RGB10A2 where alpha packs material tags) — never RGBA8/16-bit-pack.
##### How It Works
1. GBuffer albedo written at >=24-bit effective precision (RGBA16 or RGB10A2+tags).
2. Lighting resolve reads full-precision albedo; no 16-bit crush at prepass->basepass boundary.
3. Tier-0 safe: RGBA16 GBuffer albedo costs ~+8 bytes/px vs RGBA8; bounded by 6GB VRAM budget.
##### Reference Implementation
```cpp
// GBuffer albedo: full precision (TI R2 — never crush to 16-bit)
layout(rgba16f) uniform image2D gAlbedo;   // NOT rgba8
vec4 alb = imageLoad(gAlbedo, px);         // already full precision at resolve
```
##### Player-Facing Impact
Eliminates TI's documented specular instability / "plastic" banding from precision loss.
Matches Dead Space remake / NFS2015 richness bar.

═══════════════════════════════════════════════════════
GAP R3+R4 — Prepass discipline (target: M4.5, new guard EXT)
═══════════════════════════════════════════════════════
#### [M4.5-EXT-23] Depth-Prepass Discipline (depth-only, 1-BC4 mask, no pixel-shader draws)
> **tags** · prepass, overdraw, TI-gospel
> **tl;dr** · Depth prepass binds ZERO material textures and issues NO pixel-shader draws;
>   opacity/alpha-tested geo that needs textures is the exception, minimized. Prepass inputs
>   capped at a single BC4 mask for opaque. TI: Dead Space "21 extra textures per base-pass
>   draw" = the killer; prepass must not repeat it.
> **ctx** · Depth-Prepass Discipline -- pairs with [M4.5-EXT-08] (HZB culling). Enforces TI
>   R3/R4 at the API level so a bad draw call fails loudly, not silently.
##### Systems Touched
Depth prepass ([M4.5]), HZB culling ([M4.5-EXT-08]), material system ([M4-EXT-14]).
##### Math
prepass_bind_set = { depth-only, 0 material SRVs, optional 1x BC4 mask for alpha-test }
if (prepass_draw.num_texture_bindings > 1) -> assert/error  (TI: 21-texture fault)
if (prepass_draw.uses_pixel_shader && !is_alpha_test) -> assert/error
##### How It Works
1. Prepass pipeline state = depth-write only; pixel shader disallowed except alpha-tested cutout.
2. Opaque prepass binds at most ONE BC4 mask (no multi-BC4/BC7 like the Dead Space fault).
3. Dense alpha-tested/quad-overdraw geo minimized in prepass (TI: "extremely dense alpha tested
   content" in Crysis 3 prepass = bad; HL Alyx omits it).
##### Reference Implementation
```cpp
// Prepass PSO validation (TI R3/R4)
assert(pso.depthWrite && !pso.pixelShader || pso.alphaTestCutout);
assert(pso.materialTextureBindings <= 1);  // 1 BC4 mask max
```
##### Player-Facing Impact
Cheap, stable prepass (TI: optimized prepass ~0.35ms vs UE compute bloat ~6ms). No silent
VRAM/bandwidth bleed from over-bound prepass draws.

═══════════════════════════════════════════════════════
GAP R5 — BC1/BC4 path + BC5-normal caveat (target: M4-EXT-25 / M4-EXT-89)
═══════════════════════════════════════════════════════
#### [M4-EXT-90] BC1/BC4 Preferred Path + BC5-Normal Cost Caveat
> **tags** · compression, texture, TI-gospel
> **tl;dr** · Prefer BC1 (color) / BC4 (scalar) for material textures; BC5-normal is COSTLY
>   (TI: "single BC5 texture is crazy expensive") — use only when BC1 artifacts are proven.
>   BC7 reserved for where needed (TI: "BC7 performs identically to near uncompressed").
> **ctx** · BC1/BC4 Preferred Path -- extends [M4-EXT-25]/[M4-EXT-89] (BC7/ASTC). Adds the
>   TI cost hierarchy so Tier-0 6GB VRAM is spent where it earns quality, not by default.
##### Systems Touched
Texture compression ([M4-EXT-25]/[M4-EXT-89]), material system ([M4-EXT-14]), bake pipeline.
##### Math
cost_rank: BC1 < BC4 < BC7 ~ uncompressed ; BC5-normal >> BC1 (memory-bound, not ALU)
select_format(tex): if color && artifacts_ok(BC1) -> BC1
                    if scalar(mask/AO) -> BC4
                    if normal && !prove_BC1_bad -> BC1 (packed) else BC5/BC7
##### How It Works
1. Default material color = BC1; scalar masks = BC4; both "insanely efficient" (TI).
2. Normal maps: try BC1-packed normal first; only fall back to BC5/BC7 if visible artifacts.
3. BC7 = last resort for complex albedo where BC1 banding shows. ASTC equivalent on mobile.
##### Reference Implementation
```cpp
// TI R5: BC1 king, BC5-normal expensive
TexFormat pick(MaterialTex t){
  if(t.isScalar) return BC4;
  if(t.isColor && !t.provenBC1Bad) return BC1;   // default
  if(t.isNormal && !t.provenBC1Bad) return BC1;   // packed normal
  return BC7;                                      // only where needed
}
```
##### Player-Facing Impact
Maximum VRAM/bandwidth headroom on 6GB Tier-0 (TI: textures are the VRAM dominant cost);
same look, fraction of footprint. No default BC5-normal waste.

═══════════════════════════════════════════════════════
GAP R6 — Material texture-sample cap (target: M4-EXT-14 material compiler)
═══════════════════════════════════════════════════════
#### [M4-EXT-91] Material Texture-Sample Budget (3–4 max, memory-bound)
> **tags** · material, sampling, TI-gospel
> **tl;dr** · Material compiler enforces <=3–4 texture samples per material. Draws are
>   MEMORY-BOUND by texture addressing, not ALU (TI: "we could double material complexity
>   without much impact"). Spend ALU headroom on better shading, not more fetches.
> **ctx** · Material Texture-Sample Budget -- pairs with [M4-EXT-14] (material system). Hard
>   cap so artists can't silently blow the bandwidth budget per draw.
##### Systems Touched
Material compiler ([M4-EXT-14]), shader generation, bindless data array ([M0-EXT-08]).
##### Math
sample_count(mat) <= 4  (3 preferred for simple materials)
if (count > 4) -> compile error with suggestion to pack/atlas
##### How It Works
1. Compiler counts textureSample() in generated material pixel shader.
2. >4 -> hard error; suggest combining maps (ORM pack) or using procedural (Material Maker).
3. ALU ops unbounded (cheap); only texture fetches capped.
##### Reference Implementation
```cpp
// TI R6: 3–4 samples max
if (materialPixelShader.sampleCount() > 4)
    throw CompileError("material exceeds 4 texture samples (TI R6)");
```
##### Player-Facing Impact
Stable per-draw bandwidth on Tier-0; no single material starves the texture cache. Frees
ALU for Burley/POM/SSS richness (TI: memory-bound, not ALU-bound).

═══════════════════════════════════════════════════════
GAP R9 — Noise-free VT mandate (target: M4.5-EXT-26)
═══════════════════════════════════════════════════════
#### add to [M4.5-EXT-26] — Noise-Free Streaming Mandate (TI R9)
> **ctx addition** · TI R9: "Borderlands 4 cost tripled by unoptimized materials and NOISY
>   virtual textures." Mandate: RVT page streaming MUST be noise-free (no shimmering/trudging
>   tile pops). If clean streaming cannot be guaranteed on a platform, DISABLE VT there
>   (fall back to BC1/BC4 packed materials, M4-EXT-90) rather than ship noisy VT.
##### How It Works addition
- Page resize/eviction uses hysteresis + STBN dither (reuse K-EXT-22) so tile transitions
  are imperceptible; never raw nearest-pop.
- If feedback shows >N tile-pops/frame on Tier-0, auto-degrade to non-VT material path.

═══════════════════════════════════════════════════════
GAP R11 — Tonemapper EXT (target: M4.5, new)
═══════════════════════════════════════════════════════
#### [M4.5-EXT-33] Filmic/ACES Tonemapper (crisp, not blur-mask)
> **tags** · post, tonemap, TI-gospel
> **tl;dr** · Ship a proper filmic (ACES/AgX-style) tonemapper as the default; NOT the engine
>   default and NOT a "blurry filmic filter" used to hide shading errors (TI R13: motion blur /
>   film grain / blurry filmic = deficiency-hiding). Tonemap is crisp; aliasing is solved at
>   geometry level (M4.5-EXT-11), not smeared in post.
> **ctx** · Filmic Tonemapper -- post chain ([M4.5]). TI: "There is demand for better tone
>   mappers." Pairs with [M4.5-EXT-15] (TAA clamper) so post doesn't fight AA.
##### Systems Touched
Post-processing ([M4.5]), HDR output, exposure/color-grade (LUT).
##### Math
color = ACESFilm(x * exposure);  // or AgX
out = pow(color, 1/2.2)          // gamma
(no separate blur pass to "soften"; AA handled by M4.5-EXT-11)
##### How It Works
1. HDR scene -> exposure -> ACES/AgX tonemap -> gamma. Single pass.
2. NO motion-blur/film-grain pass used to mask shading deficiencies (TI R13).
3. Tonemapper is the IQ lever; default engine tonemap replaced.
##### Reference Implementation
```cpp
vec3 aces(vec3 x){ const float a=2.51,b=0.03,c=2.43,d=0.59,e=0.14;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.,1.); }
```
##### Player-Facing Impact
Richer, less "CGI" image (TI bar). Crisp edges preserved; no temporal mush from post.

═══════════════════════════════════════════════════════
STILL-MISSING / FOLLOW-UPS (flagged, not drafted)
═══════════════════════════════════════════════════════
- T1 (filtered-mip BRDF): confirm M4.5-EXT-19/20 ships trilinear/aniso + mipmap filtering
  for BRDF; if not, add a line to M4.5-EXT-20. (Not a new EXT — append.)
- T4 (RGB10A2 normals): fold into M4.5-EXT-22 (R2) GBuffer layout note.
- Web independent verification of TI claims (RGB10A2, specular-alias-via-mip, TAA
  hysteresis) BLOCKED — DDG/Bing bot-blocked, egress throttled. Claims are TI-primary.
- 4 missing transcripts (5lDkHQ1bxG0, w1OzfuqCS10, aB5qxp6SPPQ, oD1cvng8SJE) still
  pending home-IP cooldown re-pull.
