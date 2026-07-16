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
(no separate blur pass to "soften"; aliasing solved at geometry/specular level — T1 mip
 filtering + T3 MSAA-stencil, NOT a post smear)
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
NEW GAP T1 — BRDF mipmap/trilinear/aniso filtering (target: M4.5-EXT-20, append)
═══════════════════════════════════════════════════════
#### add to [M4.5-EXT-20] — Filtered Mipmap BRDF (specular-aliasing fix)
> **ctx addition** · TI T1: "Halflife Alex combat specular aliasing using filtered MIP maps"
>   (no temporal). VERIFIER confirmed M4.5-EXT-19 (Heiligenschein only) and -20 (Burley only)
>   have NO mipmap/trilinear/aniso filtering for the BRDF. Add it so low-res mip levels don't
>   distort lighting (TI: Callisto chars look better up-close BECAUSE mipmaps are unfiltered).
##### How It Works addition
- Material BRDF samples use trilinear (mip-lerp) + anisotropic filtering; roughness drives a
  mip LOD bias so specular highlights don't alias at distance (Lazarov 2015 "specular AA" via
  roughness-mip-footprint — screen-space derivative of roughness used to bias mip).
- This is the geometry/specular-level AA (T1), complementing T3 MSAA-stencil. No TAA-smear.

═══════════════════════════════════════════════════════
NEW GAP T3 — MSAA edge-detection stencil (target: M4.5, new — EXT-11 is upscaler-edge only)
═══════════════════════════════════════════════════════
#### [M4.5-EXT-34] MSAA Edge-Detection Stencil (stable edges, no temporal smear)
> **tags** · AA, msaa, stencil, TI-gospel
> **tl;dr** · Custom edge-detection stencil marks geometric edges so MSAA shades them at
>   sample frequency WITHOUT temporal reprojection/smear (TI: "custom edge detection stencil
>   does recognize these edges properly"). VERIFIER: M4.5-EXT-11 is a depth-Laplacian
>   UPSCALER-edge aid, NOT this — so T3 is a genuine gap.
> **ctx** · MSAA Edge-Detection Stencil -- pairs with [M4.5-EXT-11] (upscaler edge) + [M4.5-EXT-20].
##### Systems Touched
MSAA resolve, depth prepass ([M4.5]), stencil buffer.
##### Math
edge = detectDepthDiscontinuity(N/S/E/W) OR detectNonMSAAEdge()
stencil.edges = edge ? 1 : 0   // only edge pixels get per-sample shading
##### How It Works
1. After prepass, a stencil pass tags geometric edges (depth/normal discontinuity).
2. MSAA resolve shades ONLY stencil-tagged pixels at sample frequency; interior = 1 sample.
3. No history buffer, no temporal blend -> no shimmer, no smear (TI R7).
##### Reference Implementation
```cpp
// T3: edge stencil (TI: stable edges, no temporal smear)
float edge = depthDisc(dpdx,dpdy) | normalDisc;
if (edge > thr) stencilMask |= EDGE_BIT;   // MSAA shades these per-sample
```
##### Player-Facing Impact
Crisp silhouettes on moving geometry with zero temporal shimmer — the HL Alyx look TI praises.

═══════════════════════════════════════════════════════
NEW GAP T4 — GBuffer normal format RGB10A2 (target: M4.5-EXT-22, fold into R2 draft)
═══════════════════════════════════════════════════════
#### add to [M4.5-EXT-22] — Normal buffer = RGB10A2 (+ material tags in alpha)
> **ctx addition** · TI T4: "normals are usually stored in 32-bit formats like RGBA8 or
>   preferably in RGB10A2." VERIFIER: no EXT sets normal format (M4.5-EXT-12 is visibility-buffer
>   compaction, not format). Fold into R2's GBuffer layout mandate.
##### How It Works addition
- GBuffer normal encoded RGB10A2 (10-bit channels, 2-bit unused or pack material tag in A).
- Higher precision than RGBA8 normals -> less banding on smooth surfaces; cheap (32-bit).

═══════════════════════════════════════════════════════
NEW GAP T5 — Shadow-mask stencil (target: M4.5-EXT-13, append)
═══════════════════════════════════════════════════════
#### add to [M4.5-EXT-13] — Shadow-Mask Stencil (cheap projection)
> **ctx addition** · TI T5: "stencil channel is used to restrict expensive shadow map
>   projection invocations." VERIFIER: M4.5-EXT-13 is VSM generation (no stencil-mask). Add a
>   stencil pre-pass that marks only pixels needing shadow projection; conservative stencils in
>   one shadow atlas -> "shadow masking cost would be way lower."
##### How It Works addition
- Before shadow projection, a stencil pass tags pixels inside shadow-caster silhouette.
- Shadow projection shader early-outs where stencil == 0 (no wasted invocations).
- Conservative stencil (slightly over-mark) avoids leaks; cost << blind full-screen project.

═══════════════════════════════════════════════════════
NEW GAP T8 — Subsurface scattering LUT (target: M4.5, new)
═══════════════════════════════════════════════════════
#### [M4.5-EXT-35] Subsurface Scattering LUT (cheap, rich skin)
> **tags** · sss, skin, TI-gospel
> **tl;dr** · Subsurface scattering driven by a custom lookup table (TI: "custom lookup table…
>   lavish subsurface scattering skin"), NOT brute-force volumetric. VERIFIER: M4.5-EXT-19 is
>   Heiligenschein wet-surface only — no SSS LUT. Genuine gap.
> **ctx** · SSS LUT -- pairs with [M4.5-EXT-20] (Burley/Beer-Lambert). Skin/flesh forward path
>   (T7) uses this for rich subsurface without per-pixel path cost.
##### Systems Touched
Skin/translucent shading ([M4.5-EXT-20]), forward path (T7), material system.
##### Math
sss = texture(SSSLut, vec2(NoL, curvature)) * scatterColor   // LUT keyed by light angle + curvature
##### How It Works
1. Precompute SSS response (warp/wrap diffuse + transmission) into a 2D LUT (NoL x curvature).
2. Skin shader samples LUT instead of multi-sample volumetric -> ~free, rich subsurface.
3. Forward-rendered skin (T7) only — minority screen (TI: 240-bit TCP skin buffer = 4% screen).
##### Reference Implementation
```cpp
vec3 SSS(vec3 N, vec3 L, float curv, vec3 scatter){
  float nol = max(dot(N,L),0.0);
  return texture(uSSSLut, vec2(nol, curv)).rgb * scatter; }
```
##### Player-Facing Impact
Flesh reads alive (not plastic) on zombies/survivors; cheap enough for Tier-0 forward skin.

═════════════════════════════════════════════════════════════════════════════════
NEW GAP T7 — Forward-Rendered Skin/Hair Path (target: M4.5, new)
═════════════════════════════════════════════════════════════════════════════════
#### [M4.5-EXT-32] Forward-Rendered Skin/Hair Path (not deferred)
> **tags** · forward, skin, hair, TI-gospel
> **tl;dr** · Skin and hair are FORWARD-rendered (opaque world = tiled deferred). TI: "hair and
>   skin are forward rendered." VERIFIER: M4.5-EXT-20 (Burley/Beer-Lambert) has NO forward skin/
>   hair pass — "flesh" appears only as translucent Beer-Lambert transmission, "forward-scatter"
>   is a scattering term. T7 was falsely marked COVERED; re-classified GAP. (TI quote is
>   transcript-primary; no external citable spec mandates it — kept as GAP, not invented citation.)
> **ctx** · Forward-Rendered Skin/Hair Path -- pairs with [M4.5-EXT-20] (Burley shading) + T8
>   (SSS LUT) for rich forward skin. Forward path avoids deferred GBuffer precision/normal
>   artifacts on thin/translucent hair + subsurface skin.
##### Systems Touched
Forward shading path ([M4.5]), material system ([M4-EXT-14]), skinning ([M4.5-EXT-14]/18).
##### Math
forward_skin(N,L,V,curv) = Burley(N,L,V) + SSS_LUT(NoL,curv)   // T8 LUT, no GBuffer round-trip
hair = alpha-tested forward, depth-sorted, no deferred resolve
##### How It Works
1. Skin/hair meshes routed to a dedicated forward pass (opaque = tiled deferred per TI).
2. Forward skin uses Burley + SSS LUT (T8) directly — no GBuffer encode/decode precision loss.
3. Hair = alpha-tested forward, drawn after opaque, depth-sorted; minority screen (~4%, TI).
##### Reference Implementation
```cpp
// T7: skin/hair forward (TI: "hair and skin are forward rendered")
if (mat.isSkin || mat.isHair) { out = ShadeForward(p); }  // not deferred resolve
```
##### Player-Facing Impact
Correct thin-geometry + subsurface look on zombies/survivors/hair without deferred GBuffer
banding. Matches TI's praised HL Alyx forward-skin result.

═════════════════════════════════════════════════════════════════════════════════
NEW GAP T12 — Parallax Occlusion Mapping (target: M4.5-EXT-20, append)
═════════════════════════════════════════════════════════════════════════════════
#### add to [M4.5-EXT-20] — Parallax Occlusion Mapping (base-pass surface detail)
> **ctx addition** · TI T12: "renders parallax occlusion detail on top of existing geometry"
> (no extra depth pass). VERIFIER: EXT-20 body has Burley/Beer-Lambert only — no POM. Add so
> materials get relief detail without a second geometry/depth pass.
##### How It Works addition
- Material BRDF samples use parallax occlusion mapping in the base pass: ray-march the height
  field along the view vector in the pixel shader, offset UVs per layer. Detail "on top of"
  existing geometry — no extra depth prepass, no vertex displacement.
- Step count bounded (TI: cheap, base-pass only); combines with T1 filtered-mip BRDF so the
  parallax samples don't alias at distance.

═════════════════════════════════════════════════════════════════════════════════
STILL-MISSING / FOLLOW-UPS (flagged, not drafted)
═══════════════════════════════════════════════════════
- Web independent verification of TI claims: egress recovered 2026-07-16 (resume). Confirmed
  Lazarov 2015 "Specular AA" = roughness-driven mip footprint / prefilter (SIGGRAPH 2015
  shading course, canonical) — T1 draft grounded. T7 "hair and skin are forward rendered" is
  TI-transcript-primary; no external citable spec rule found (Bing exact-quote returned only
  TI's own wording) — correctly kept as GAP, not invented citation. RGB10A2 / TAA-hysteresis
  claims remain TI-primary (Disney/Filament math for Burley loaded fine).
- 4 missing transcripts (5lDkHQ1bxG0, w1OzfuqCS10, aB5qxp6SPPQ, oD1cvng8SJE) still
  pending home-IP cooldown re-pull.
- VERIFIER: recon/ti_gate.py (merged) gates all COVERED/GAP claims against real block bodies.
  Re-run after any plan edit. Exit 0 = safe to call "done".
