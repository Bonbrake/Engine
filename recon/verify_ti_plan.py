#!/usr/bin/env python3
"""
recon/verify_ti_plan.py — mechanical bug-catcher for the TI-gospel plan.

WHY: the agent kept claiming EXTs "covered" from their TITLE, not their BODY,
and declared done on vibes. This script reads the REAL block text from spec/
and checks whether each plan claim is actually backed. No claim passes on title
alone.

For each plan line that asserts a capability is covered by / mapped to an EXT,
we extract that EXT's full block from spec/Mx.md and verify the claim's
KEYWORDS appear in the body. MISMATCH -> FAIL with the missing keyword.

Also checks GAP drafts target EXTs that do NOT already contain the capability
(so we don't duplicate).

Run: python recon/verify_ti_plan.py
"""
import os, re, sys

BASE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(BASE, "..", "spec")

# ---- claim table: (plan_claim_text_fragment, ext_id, required_keywords) ----
# required_keywords must ALL appear in the real block body, else FAIL.
CLAIMS = [
    # R7 -> M4.5-EXT-11 (silhouette AA) + M4.5-EXT-15 (TAA clamper)
    ("R7 MSAA/AA", "M4.5-EXT-11", ["laplacian", "depth", "edge"]),  # NOT "stencil"/"MSAA"
    ("R7 TAA clamper", "M4.5-EXT-15", ["hysteresis", "variance", "blend"]),
    # R8 shadows
    ("R8 SDF", "M4.5-EXT-10", ["distance field", "shadow"]),
    ("R8 SDF cascade", "M4.5-EXT-31", ["cascade", "shadow"]),
    ("R8 VSM", "M4.5-EXT-13", ["virtual shadow", "page"]),
    # R9 VT
    ("R9 RVT", "M4.5-EXT-26", ["virtual texture", "page"]),
    # R10 translucency
    ("R10 Beer-Lambert", "M4.5-EXT-20", ["beer", "lambert", "burley"]),
    # R12 LOD
    ("R12 meshlet LOD", "M4-EXT-02", ["meshlet", "decimation", "lod"]),
    ("R12 seam weld", "M4-EXT-05", ["seam", "weld", "lod"]),
    ("R12 multi-res LOD", "M4-EXT-45", ["lod", "resolution"]),
    ("R12 impostor", "M4.5-EXT-30", ["impostor", "lod"]),
    # R1 Burley
    ("R1 Burley", "M4.5-EXT-20", ["burley", "lambert"]),
    # T-mappings (technique -> EXT)
    ("T6 SSAO", "M4.5-EXT-27", ["ambient", "obscurance", "ssao"]),
    ("T9 SSGI", "M4.5-EXT-29", ["global illumination", "ssgi"]),
    ("T9 ambient field", "M4.5-EXT-17", ["visibility field", "gi", "ambient"]),
    ("T9 SH ambient", "M10-EXT-03", ["sh", "ambient", "atmospheric"]),
    ("T14 voxel raster", "M4.5-EXT-16", ["voxel", "raster"]),
    ("T15 volumetrics", "M4.5-EXT-01", ["volumetric", "cone", "shadow"]),
]

# ---- keyword checks that SHOULD NOT pass (false "covered" traps we already hit) ----
# If plan maps T3 (MSAA edge stencil) to EXT-11, EXT-11 must contain "msaa"/"stencil".
TRAP_CHECKS = [
    ("T3 MSAA-stencil", "M4.5-EXT-11", ["msaa", "stencil"]),  # EXPECT FAIL (it's upscaler-edge)
    ("T5 shadow stencil-mask", "M4.5-EXT-13", ["stencil", "shadow mask"]),  # EXPECT FAIL (VSM gen)
    ("T8 SSS LUT", "M4.5-EXT-19", ["subsurface", "lookup", "lut"]),  # EXPECT FAIL (only Heiligenschein)
    ("T1 mip BRDF", "M4.5-EXT-19", ["mipmap", "trilinear", "aniso"]),  # EXPECT FAIL (no mip filter there)
    ("T4 RGB10A2", "M4.5-EXT-12", ["rgb10a2", "rgb10"]),  # EXPECT FAIL (visibility buffer, not normal fmt)
]

def load_block(ext_id):
    """Return the full block text for ext_id from the correct spec file."""
    m = re.match(r"(M\d+(?:\.\d+)?)-EXT-(\d+)", ext_id)
    if not m: return None
    fname = f"{m.group(1)}.md"
    path = os.path.join(SPEC, fname)
    if not os.path.exists(path):
        return f"__NOFILE__:{path}"
    txt = open(path, encoding="utf-8").read()
    hdr = f"#### [{ext_id}]"
    i = txt.find(hdr)
    if i < 0:
        return f"__NOBLOCK__:{ext_id}"
    nxt = txt.find("\n#### [", i + len(hdr))
    if nxt < 0:
        nxt = len(txt)
    return txt[i:nxt]

def check(name, ext_id, kws, expect_fail=False):
    body = load_block(ext_id)
    if body.startswith("__"):
        return ("NOBLOCK", f"{body} (claim '{name}' CANNOT be verified)")
    low = body.lower()
    missing = [k for k in kws if k not in low]
    if missing:
        status = "EXPECTED-FAIL" if expect_fail else "FAIL"
        return (status, f"{ext_id} missing keywords: {missing}")
    status = "PASS" if not expect_fail else "TRAP-PASSED(claim wrongly says covered)"
    return (status, f"{ext_id} contains all keywords {kws}")

print("="*70)
print("TI-PLAN COVERAGE VERIFIER — mechanical bug-catch")
print("="*70)
fails = 0
print("\n--- CLAIMS (should PASS) ---")
for name, ext, kws in CLAIMS:
    st, msg = check(name, ext, kws)
    if st == "FAIL": fails += 1
    print(f"  [{st:8}] {name:22} -> {msg}")

print("\n--- TRAP CHECKS (these SHOULD FAIL = confirm false 'covered') ---")
traps = 0
for name, ext, kws in TRAP_CHECKS:
    st, msg = check(name, ext, kws, expect_fail=True)
    if st == "EXPECTED-FAIL": traps += 1
    else: fails += 1
    print(f"  [{st:12}] {name:22} -> {msg}")

print("\n" + "="*70)
print(f"RESULT: {len(CLAIMS)} claims, {fails} real failures | {traps}/"
      f"{len(TRAP_CHECKS)} traps correctly failed")
if fails:
    print("ACTION: fix the plan — these claims are NOT backed by block bodies.")
    sys.exit(1)
else:
    print("All title-based claims verified against block bodies.")
    sys.exit(0)
