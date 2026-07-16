#!/usr/bin/env python3
"""
recon/analyze_transcripts.py — three-pass extractor for Threat Interactive transcripts.
Outputs a structured JSON of: technique claims, format/number facts, and anti-pattern faults.
Run: python recon/analyze_transcripts.py
"""
import os, re, json, glob

BASE = os.path.dirname(os.path.abspath(__file__))
TR = os.path.join(BASE, "transcripts")

# Pass 1: technique nouns (specific rendering methods)
TECH = {
 "filtered_mipmap":"filtered mip", "specular_aliasing":"specular aliasing",
 "parallax_occlusion":"parallax occlusion", "edge_stencil":"edge detection stencil",
 "rgb10a2":"rgb10a2", "rgb10":"rgb10", "rg16":"rg16", "r8_buffer":"r8 ",
 "r32":"r32", "rgba16":"rgba16", "rgba8":"rgba8", "bgr88":"bgr88",
 "ssao":"ssao", "rtao":"rtao", "ssr":"screen space reflection",
 "reflective_shadow_map":"reflective shadow map", "rsm":"rsm",
 "shadow_cascade":"cascade", "shadow_mask":"shadow mask", "shadow_atlas":"shadow atlas",
 "motion_vector":"motion vector", "velocity_buffer":"velocity",
 "virtual_shadow_map":"virtual shadow", "hzb":"hi-z", "hiz":"hi-z",
 "occlusion_query":"occlusion query", "indirect_lighting":"indirect lighting",
 "spherical_harmonic":"spherical harmonic", "probe":"probe lighting",
 "subsurface_scattering":"subsurface scattering", "sss_lut":"lookup table",
 "forward_skin":"skin are forward", "forward_hair":"hair are forward",
 "tiled_deferred":"tiled deferred", "tiled":"tiled deferred",
 "emissive_basepass":"emissives to the lit buffer",
 "dynamic_shadow_res":"dynamic resizing", "fov_cull_shadow":"shadow map fov culling",
 "screen_aware":"screen aware",
 "nanite":"nanite", "compute_raster":"compute shader rasterizer",
 "mesh_shader":"mesh shader", "cluster_cull":"cluster cull",
 "half_resolution":"half resolution", "half_res":"half-res", "quarter_res":"quarter",
 "3_4_res":"3/4 res", "downscale":"downscale", "depth_downscale":"depth downscale",
 "cloud_temporal":"clouds utilize temporal",
 "motion_blur_hide":"motion blur", "film_grain":"film grain", "filmic_blur":"filmic filters",
 "baked_light":"baked lighting", "lightmap":"lightmap",
 "anisotrop":"anisotrop", "trilinear":"trilinear", "bilinear":"bilinear",
 "octahedral":"octahedral", "impostor":"impostor",
 "alpha_test":"alpha tested", "alpha_tested":"alpha tested",
 "parallax":"parallax", "decal_projection":"projection pass", "projection":"projection",
 "stencil_mask":"stencil channel", "stencil_restrict":"restrict expensive",
 "prepass_large":"large objects", "prepass_skip":"skippable",
 "gbuffer":"gbuffer", "render_target":"render target",
 "pixel_invocation":"pixel invocations", "overdraw":"overdraw", "quad":"quad overdraw",
 "fillrate":"fill rate", "fill_rate":"fill rate", "bandwidth":"bandwidth", "vram":"vram",
 "texture_array":"texture array", "bindless":"bindless", "descriptor":"descriptor",
 "pipeline_permutation":"pipeline objects", "psso":"pipeline state", "stutter":"stuttering",
 "compilation":"shader-compilation",
 "bloom":"bloom", "dof":"dof", "depth_of_field":"dof", "exposure":"exposure",
 "color_grade":"color grade", "lut":"lut", "aces":"aces", "filmic":"filmic",
 "tone_map":"tone map", "tonemapper":"tone mapper",
 "gi":"global illum", "voxel":"voxel", "cone_trace":"cone", "radiance":"radiance",
 "ray_march":"raymarch", "raymarch":"raymarch", "screen_space":"screen space",
 "mipmap":"mipmap", "mip_map":"mip map", "mip_filter":"mipmap filtering",
}
# Pass 2: anti-pattern / fault language
FAULT = ["butchered","severely butchered","fault","mistake","wrong","incompetent",
 "regress","abuse","manufacture","lie","lazy","sloppy","waste","wasted","bloated",
 "bloat","punish","crippl","cripple","tripled","3x slower","three times slower",
 "crush","inefficient","unoptimized","poorly optimized","horrendous","disgusting",
 "psychotic","embarrass","shimmer","blur","smear","ghost","distort","plastic",
 "fake","cheat","trick","hid","hide","cover","pretend","deficiency","decay","decayed"]
# Pass 3: number/format facts
NUM = re.compile(r'\b(\d{1,4})\s?(ms|mb|gb|kb|%|x|times|fps|bit|bits|res|squared|px|pixels|draws|textures|samples|materials)\b', re.I)

ids = sorted(glob.glob(os.path.join(TR, "yt_*.txt")))
out = {}
for fp in ids:
    vid = os.path.basename(fp)[3:-4]
    txt = open(fp, encoding="utf-8").read()
    sents = re.split(r'(?<=[.!?])\s+', txt)
    tech_hits = {}
    faults = []
    nums = []
    for s in sents:
        low = s.lower()
        if len(s) < 30: continue
        for key, term in TECH.items():
            if term in low:
                tech_hits.setdefault(key, []).append(s.strip().replace("\n"," "))
        if sum(1 for f in FAULT if f in low) >= 2:
            faults.append(s.strip().replace("\n"," "))
        for m in NUM.finditer(s):
            nums.append(m.group(0))
    out[vid] = {
        "tech": {k: v[:3] for k,v in tech_hits.items()},
        "faults": faults[:8],
        "nums": sorted(set(nums))[:40],
        "words": len(txt.split()),
    }

json.dump(out, open(os.path.join(BASE, "transcript_analysis.json"), "w"), indent=1)
# print summary
for vid, d in out.items():
    print(f"\n[{vid}] {d['words']}w  tech={len(d['tech'])} faults={len(d['faults'])} nums={len(d['nums'])}")
    print("  tech:", ", ".join(sorted(d['tech'].keys())))
    print("  nums:", ", ".join(d['nums'][:25]))
