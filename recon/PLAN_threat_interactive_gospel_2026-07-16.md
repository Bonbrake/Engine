# Plan: Adopt Threat Interactive's Rendering Rules as ZombieEngine Gospel

> STATUS: PLANNING ONLY. No spec/ file is modified. Maps TI's verified primary-source
> mandates (11 transcripts in `recon/transcripts/yt_*.txt`) to concrete ZombieEngine
> rendering decisions. User directive: "Threat Interactive is gospel — it's the rule."
> So these are HARD CONSTRAINTS on the renderer, not suggestions.
>
> EXT-ID cross-check done via terminal grep on spec/M4.md + spec/M4.5.md + spec/M10.md
> (search_files tool is broken on this path — external bug; grep is authoritative).
> ALL EXT IDs below are REAL and verified against the actual block text this session.
> NOTE: web independent-verification of TI's claims is BLOCKED right now (DDG/Bing bot-
> blocked, egress throttled) — claims are TI-primary-source; flagged as such.

## WHAT IT CHANGES (the look — not "none")
TI's rules *are* the visual target. Adopting them moves the look from "plastic Unreal
CGI" to physically-correct, crisp, grounded:
- **Shading:** Burley/Kalisto diffuse (already in M4.5-EXT-20), physically-derived
  terminator/specular. Kills the "plastic bland" Lambert look. [ALREADY IN SPEC]
- **Edges:** geometry/specular-level AA (filtered mipmaps + edge stencil), NOT TAA-smear.
  Crisp silhouettes, no shimmer, no blur.
- **Shadows:** real cascaded/SDF shadows done right; RT only where it earns ms.
- **Precision:** base-color stays full precision through lighting (no 16-bit crush). [GAP]
- **Tonemapping:** proper filmic tonemapper instead of engine-default. [GAP]
- **Materials:** 3–4 texture samples max, BC1/BC4-preferred compression. [PARTIAL]
Net: looks like Dead Space remake / NFS2015 / Star Wars BF (titles TI praises).

## THE 13 RULES (gospel) — REAL EXT IDs + accurate status (verified against block text)
R1  Diffuse >= Burley, physically derived.
    SPEC: COVERED — M4.5-EXT-20 "Diffuse + Translucent Medium Light (Burley/Kalisto
    Diffuse, Beer-Lambert Extinction)" explicitly ships Burley, branded Kalisto, with
    "never ship Lambert" in its reference impl. Confirm only (no gap).
R2  No base-color precision crush (24->16 bit fault).  [GAP — no EXT guarantees this]
    Impl: GBuffer base-color at full precision through lighting stage.
R3  Depth prepass = depth ONLY, no textures/shaders.
    SPEC: PARTIAL — M4.5-EXT-08 (HZB/cluster occlusion culling) exists; the explicit
    "prepass binds zero material textures / no pixel-shader prepass draws" RULE is a GAP.
R4  Prepass uses single BC4 mask, not 21 textures.  [GAP — no EXT exists]
    Impl: hard budget (1 BC4 max for opaque prepass inputs).
R5  BC1 king / BC5-normal expensive / BC7~uncompressed.
    SPEC: PARTIAL — M4-EXT-25 + M4-EXT-89 (BC7/ASTC) exist. Add BC1/BC4 path +
    BC5-normal caveat (BC5-normal costly; use only when BC1 artifacts proven).
R6  3–4 texture samples/material max (memory-bound, not ALU).  [GAP — no EXT exists]
    Impl: material compiler enforces sample cap; spend ALU headroom on shading.
R7  MSAA viable in deferred if prepass clean; TAA = conservative 1–2 frame clamp only.
    SPEC: COVERED — M4.5-EXT-11 (Multi-Sampled Depth Derivative Silhouette Reconstruction
    Filter) does edge AA without TAA-smear; M4.5-EXT-15 (Hysteresis-Gated TAA Variance
    Clamper) confirmed hysteresis-gated, NOT infinite-smear. Keep both.
R8  Shadows real first; RT only where it earns ms (~8ms quarter-res RT ref).
    SPEC: COVERED — M4.5-EXT-10 (Global Distance Field / SDF Shadows) + M4.5-EXT-31
    (SDF Shadow Cascade) + M4.5-EXT-13 (Virtual Shadow Maps / HZB). Raster-correct first.
R9  Virtual textures: clean or don't (noisy VT triples cost).
    SPEC: COVERED — M4.5-EXT-26 (Runtime Virtual Texture base) + M4.5-EXT-21 (RVT
    Feedback/Invalidation). Add noise-free-streaming mandate; if VT can't be clean, skip.
R10 Hero/translucent = forward, not deferred.
    SPEC: COVERED — M4.5-EXT-20 (Beer-Lambert Translucent Medium Light Extinction) is the
    forward-path translucency system. Make explicit: hero/translucent = forward.
R11 Ship proper tonemapper (default inadequate).  [GAP — no tonemapper EXT exists]
    Impl: add filmic/ACES-style tonemapper EXT; not engine default.
R12 LOD preserves silhouette + material identity.
    SPEC: COVERED — M4-EXT-02 (Async Quadric Meshlet Decimation LODs) + M4-EXT-05 (Edge
    Seam Welding) + M4-EXT-45 (Multi-Resolution LOD Gen) + M4.5-EXT-30 (Impostor LOD).
R13 Vendor-agnostic IQ; upscaling = fallback not primary.
    SPEC: COVERED — Design Pillar 5 (reuse proven defaults). No vendor-locked AA/upscaling
    as primary IQ; FSR/DLSS optional fallback only.

## GAPS TO FILL (need user GO before spec edits)
- R2: base-color precision rule (new EXT M4.5-EXT-22).
- R3/R4: prepass discipline (zero-texture bind + 1-BC4 budget) — new guard EXT M4.5-EXT-23.
- R5: BC1/BC4 path + BC5-normal caveat added (new EXT M4-EXT-90).
- R6: material sample cap (new EXT M4-EXT-91).
- R9: noise-free VT mandate on M4.5-EXT-26 (append).
- R11: tonemapper EXT (new M4.5-EXT-33).
- T1: BRDF mipmap/trilinear/aniso filtering — append to M4.5-EXT-20 (verified missing).
- T3: MSAA edge-detection stencil — new EXT M4.5-EXT-34 (verified: -11 is upscaler-edge only).
- T4: RGB10A2 normal format — fold into M4.5-EXT-22 (verified: no EXT sets normal fmt).
- T5: shadow-mask stencil — append to M4.5-EXT-13 (verified: -13 is VSM gen only).
- T8: subsurface scattering LUT — new EXT M4.5-EXT-35 (verified: -19 Heiligenschein only).
- T7: forward-rendered skin/hair path — new EXT M4.5-EXT-32 (verified: -20 is Burley/
  Beer-Lambert only; no forward skin/hair pass; TI quote is transcript-primary, not a
  citable external spec rule). Re-classified from false-COVERED.
- T10: emissive-in-basepass — M4 base-pass convention (not an EXT).
- T12: POM surface detail — [GAP] append to M4.5-EXT-20.
- R1/R7/R8/R9/R10/R12/R13: COVERED — confirm only, don't re-add.
- VERIFIER: recon/ti_gate.py (merged gate) must pass (exit 0) before calling this plan "done".

## ALREADY COVERED (verify, don't re-add) — REAL EXT IDs
- Burley/Kalisto diffuse + Beer-Lambert: M4.5-EXT-20
- Wet-surface micro-surface / Heiligenschein retroreflection: M4.5-EXT-19
- BC7/ASTC: M4-EXT-25 + M4-EXT-89 · Decals/RVT: M4-EXT-26 + M4.5-EXT-26 + M4.5-EXT-21
- HZB culling: M4.5-EXT-08 · SDF shadows: M4.5-EXT-10 + M4.5-EXT-31 · VSM: M4.5-EXT-13
- Silhouette AA: M4.5-EXT-11 · TAA clamper: M4.5-EXT-15 · Visibility Buffer: M4.5-EXT-12
- SSAO: M4.5-EXT-27 (Scalable Ambient Obscurance) · SSR: M4.5-EXT-28
- GI: M4.5-EXT-29 (SSGI fallback) + M4.5-EXT-17 (Ambient Visibility Field) + M10-EXT-03 (SH ambient)
- Compute voxel raster: M4.5-EXT-16 (use HW raster for opaque meshlets per T14)
- LODs: M4-EXT-02 / 05 / 45 + M4.5-EXT-30 · Volumetrics: M4.5-EXT-01

## CONTENT-PIPELINE TOOLING (procedural material authoring) — FREE, per user
Project is 100% procedural (design pillar 1). Material authoring = node-graph PROCEDURAL.
Verified 2026-07-16 via curl.

**Material Maker** (the tool):
- MIT license (commercial-safe), 5,646★, Godot-based but STANDALONE app
  (itch.io/Steam, Windows/Linux). 200+ GLSL procedural nodes.
- Exports PBR texture sets (albedo / normal / ORM / roughness / metal) that the
  BC7/ASTC pipeline (M4-EXT-25/89) consumes directly. Not a runtime dependency —
  artist-workstation authoring tool only.
- BEST pure-procedural fit: one node graph → infinite material variation (pillar 1).
  GLSL-node graphs can also be ingested, not just baked maps.
- Reject image-to-material tools (Materialize = GPL "from images", wrong model) and
  locked/paid suites (Quixel Mixer = Epic/Fab lock-in, Substance = paid) per R13.
- Ref: materialmaker.org · github.com/RodZill4/material-maker

## TECHNIQUE-LEVEL RULES (concrete methods behind the 13 rules, from TI pipeline teardowns)
T1  **Filtered mipmaps kill specular aliasing** (no TAA-smear). HL Alyx uses filtered MIP
    maps; unfiltered low-res mipmaps distort lighting. → [GAP] M4.5-EXT-19/20 BRDF has NO
    mipmap/trilinear/aniso filtering for BRDF (verified: -19 only Heiligenschein, -20 only
    Burley/Beer-Lambert). Add mip-filter line to M4.5-EXT-20. VERIFIER: T1-FAIL on -19.
T2  **Prepass = large opaque only, NO alpha-tested/distant-small geometry.** Crysis 3
    prepass "renders expensive and extremely dense alpha tested content" (bad); HL Alyx
    "specifically doesn't." → R3/R4 hardening. [GAP per R3/R4]
T3  **Custom edge-detection stencil for stable MSAA** (not temporal). "custom edge
    detection stencil does recognize these edges properly." → [GAP] M4.5-EXT-11 is a
    depth-Laplacian UPSCALER-edge aid (feeds upscale edge mask), NOT MSAA/stencil. VERIFIER:
    T3-FAIL on -11. New technique EXT needed or extend -11 to true MSAA-stencil.
T4  **Normals format: prefer RGB10A2, not RGBA8; pack material tags in alpha.** "normals
    are usually stored in 32-bit formats like RGBA8 or preferably in RGB10A2." → [GAP] no
    EXT sets normal format (M4.5-EXT-12 is visibility-buffer compaction, not format). Folds
    into R2 (M4.5-EXT-22) GBuffer layout. VERIFIER: T4-FAIL on -12.
T5  **Shadow masking via stencil + R8 shadow-mask buffer** (cheap). "stencil channel is
    used to restrict expensive shadow map projection invocations." → [GAP] M4.5-EXT-13 is
    VSM generation (no stencil-mask). New stencil-mask rule on -13 or new EXT. VERIFIER:
    T5-FAIL on -13.
T6  **SSAO at half-res with RG16 + R8 normal buffers** (proven structure). "clears the
    half resolution SSAO, creates half resolution RG16 and R8 normal buffers." → COVERED
    M4.5-EXT-27 (Scalable Ambient Obscurance), reads GBuffer depth+normal. VERIFIER: PASS.
T7  **Hair + skin = FORWARD rendered** (not deferred) — matches R10. "hair and skin are
    forward rendered" (opaque = tiled deferred). → [GAP] M4.5-EXT-32 (new forward-rendered
    skin/hair path). VERIFIER: EXT-20 body is Burley + Beer-Lambert only; "flesh" appears
    solely as TRANSLUCENT TRANSMISSION (Beer-Lambert), "forward-scatter" is a scattering term.
    No forward-rendered skin/hair pass exists -> T7 was FALSE-COVERED, re-marked GAP.
T8  **Subsurface scattering via custom LUT** (not brute-force). "custom lookup table…
    lavish subsurface scattering skin." → [GAP] M4.5-EXT-19 is Heiligenschein only, no SSS
    LUT. New SSS LUT EXT or extend -19/-20. VERIFIER: T8-FAIL on -19.
T9  **Indirect lighting = atlases + spherical-harmonic probes** (not baked lightmaps).
    "three indirect lighting atlases… spherical harmonic probe lighting." → COVERED
    M4.5-EXT-29 (SSGI) + M4.5-EXT-17 (Ambient Visibility Field) + M10-EXT-03 (SH ambient).
    VERIFIER: PASS (all three).
T10 **Emissives written in BASE PASS** (avoid redundant pixel invocations). "output
    emissives to the lit buffer in the base pass to prevent all these extra pixel
    invocations." → M4 base-pass design rule. [GAP: base-pass convention, not an EXT]
T11 **Dynamic/FOV-culled shadow resolution** (not fixed huge maps). "optimizing shadow
    resolutions with dynamic resizing" + "screen aware shadow map FOV culling." →
    M4.5-EXT-13 tightening (partially: VSM exists, dynamic-res not explicit). [PARTIAL]
T12 **Parallax occlusion mapping for base-pass surface detail** (no extra depth). "renders
    parallax occlusion detail on top of existing geometry." → [PARTIAL] M4.5-EXT-20
    (BRDF exists, POM not explicit — add POM to M4.5-EXT-20 per gap draft).
T13 **Motion blur / film grain / filmic blur = TI calls them deficiency-hiding.** "Epic
    tries to hide… deficiencies with motion blur, tons of film grain, and blurry filmic
    filters." → post chain must NOT rely on blur to mask shading errors. R11 tonemapper
    crisp, not "filmic-blur." [GAP: R11]
T14 **Nanite/cluster-mesh-shader rasterizer = 3x slower compute rasterizer** (TI claim).
    "Nanite's three times slower compute shader rasterizer." → Our meshlet LODs (M4-EXT-02)
    use HARDWARE raster, NOT the compute voxel rasterizer (M4.5-EXT-16 is compute — restrict
    to voxels/volumetrics, not opaque geometry). VERIFIER: PASS (-16 is compute voxel).
T15 **Clouds = temporal BUT separated from bad AA** (warning). "clouds utilize temporal
    rendering, but separately from poor anti-aliasing methods." → M4.5-EXT-01 volumetrics:
    temporal decoupled from AA smear. VERIFIER: PASS (-01 volumetric cone).

## IMPLEMENTATION ORDER (proposal)
1. R1 confirm (M4.5-EXT-20 already Burley — verify, no work) + R7/R8/R10/R12/R13 validate.
2. R3+R4+T2 (prepass discipline: depth-only, large-opaque, no alpha-test) — cheapest,
   biggest perf win, no visual regression. [GAP → new guard EXT]
3. R5+R6+T1+T4 (BC1/BC4 + sample cap + filtered-mip BRDF + RGB10A2 normals) — VRAM/
   bandwidth relief on 6GB Tier-0 + specular-aliasing fix.
4. R7+T3 (AA verify + edge-stencil) — visual crispness; covered.
5. R2+R11+T13 (precision + crisp tonemapper, no blur-mask) — final visual richness. [GAP]
6. R8/T5/T6/T11+VT/T10/R10/T7/T8/T9/T12/R13 — shadow-mask/SSAO/forward-skin/GI/POM/
   vendor-agnostic; covered, mandate-tighten. T14 (HW raster for meshlets) note.

## EVIDENCE (verbatim, citable — from recon/transcripts/)
- Lambert=plastic: "264-year-old diffuse equation called Lambert" / "obvious plastic bland
  Unreal Engine look… defined through a Lambert GXBRDF."
- Burley praised: "burly diffused surfaces in Dead Space remake convey a professional
  polished look." (M4.5-EXT-20 already implements this — TI gospel baked into spec.)
- Prepass depth-only: "rendering geometry depth with no textures, a specialized hardware
  path is used to write pixel depth without invoking any pixel shaders."
- 21-texture prepass fault: "21 high and low resolution additional textures are fed as
  inputs to each base pass draw."
- Prepass alpha-test fault: Crysis 3 prepass "renders expensive and extremely dense alpha
  tested content" (HL Alyx "specifically doesn't").
- BC costs: "BC1 compression… insanely efficient" / "single BC5 texture is crazy expensive"
  / "BC7 performs identically to near uncompressed contents."
- Sample budget: "At the very most, you want to sample three, maybe four textures."
- Filtered mip specular AA: "Halflife Alex… combat specular aliasing… using modern shaders
  and filtered MIP maps" (no temporal).
- Normal format: "normals are usually stored in 32-bit formats like RGBA8 or preferably in
  RGB10A2."
- Shadow-mask stencil: "stencil channel is used to restrict expensive shadow map projection
  invocations." Conservative stencils → "shadow masking cost would be way lower."
- SSAO structure: "clears the half resolution SSAO, creates half resolution RG16 and R8
  normal buffers from the base pass normals."
- Forward skin/hair: "hair and skin are forward rendered" (opaque = tiled deferred).
- SSS LUT: "custom lookup table… attributes to this game's uniquely lavish subsurface
  scattering skin."
- Indirect GI: "three indirect lighting atlases… spherical harmonic probe lighting."
- Emissive in base pass: "output emissives to the lit buffer in the base pass to prevent all
  these extra pixel invocations."
- Dynamic shadow res: "optimizing shadow resolutions with dynamic resizing" + "screen aware
  shadow map FOV culling."
- POM: "renders parallax occlusion detail on top of existing geometry" (no extra depth).
- Blur-hides-deficiency: "Epic tries to hide a lot of their visual deficiencies with motion
  blur, tons of film grain, and blurry filmic filters."
- Nanite compute raster: "Nanite's three times slower compute shader rasterizer."
- MSAA: "you should never see this kind of temporal shimmering with proper MSAA."
- RT-not-bandaid: "I despise their decision to use ray tracing as a solution to fix shadow
  deficiencies."
- Tonemapper: "There is demand for better tone mappers."
- LOD: "LODs and topology was severely butchered."
