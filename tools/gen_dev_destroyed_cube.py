#!/usr/bin/env python3
"""
tools/gen_dev_destroyed_cube.py
===============================

Purpose
-------
Generates the *destroyed* placeholder mesh paired with `dev_test_cube.glb` for the
M2 destructible test entity:

    assets/models/dev_test_cube_destroyed.gltf   (human-readable JSON)
    assets/models/dev_test_cube_destroyed.bin    (raw vertex/index bytes)

Why this script exists
----------------------
M2's destructible test entity must visually swap from an intact cube to a
"destroyed" mesh on destruction (exit criterion: "visibly swaps meshes"). The
swap is proven via a before/after windowed frame-dump, so the destroyed geometry
must be *obviously* broken at a glance — not a subtly shorter cube that only a
log line could confirm. This generator produces a cube that has shattered into
several offset/rotated debris chunks, so a human looking at the after-shot
immediately reads "that's destroyed".

Design constraints (must match the engine's loader)
--------------------------------------------------
`AssetManager::LoadMesh` (src/render/AssetManager.cpp) reads glTF 2.0 via fastgltf
with `LoadExternalBuffers`, takes the first mesh + first primitive, requires a
POSITION attribute (NORMAL/TEXCOORD_0 optional), and uploads triangle-list indices.
We emit exactly that: one mesh, one primitive, POSITION + NORMAL + TEXCOORD_0
accessors, UNSIGNED_SHORT indices, referencing an external .bin buffer.

Reproducibility
---------------
No third-party deps (no pygltflib/numpy) — pure stdlib — so a future session can
regenerate the asset without a pip step. We emit a `.gltf` + `.bin` pair rather
than a binary `.glb` container: it is simpler (no chunk padding/alignment rules to
get wrong) and human-readable, so the intent stays obvious on cold inspection.
Run:
    python3 tools/gen_dev_destroyed_cube.py
Outputs next to this file's repo root: assets/models/dev_test_cube_destroyed.{gltf,bin}
"""

import json
import struct
import math
import os


def box(scale, center, rot_axes, rot_angles):
    """Return (positions, normals, uvs, indices) for one small box, transformed."""
    h = scale * 0.5
    corners = [
        (-h, -h, -h), ( h, -h, -h), ( h,  h, -h), (-h,  h, -h),
        (-h, -h,  h), ( h, -h,  h), ( h,  h,  h), (-h,  h,  h),
    ]
    faces = [
        ([0, 1, 2, 3], ( 0,  0, -1)),
        ([4, 5, 6, 7], ( 0,  0,  1)),
        ([0, 4, 7, 3], (-1,  0,  0)),
        ([1, 5, 6, 2], ( 1,  0,  0)),
        ([3, 2, 6, 7], ( 0,  1,  0)),
        ([0, 1, 5, 4], ( 0, -1,  0)),
    ]
    def rot(p):
        x, y, z = p
        for ax, ang in zip(rot_axes, rot_angles):
            c, s = math.cos(ang), math.sin(ang)
            if ax == 'x':
                y, z = y*c - z*s, y*s + z*c
            elif ax == 'y':
                x, z = x*c + z*s, -x*s + z*c
            elif ax == 'z':
                x, y = x*c - y*s, x*s + y*c
        return (x + center[0], y + center[1], z + center[2])

    positions, normals, uvs, indices = [], [], [], []
    base = 0
    for fidx, (quad, nrm) in enumerate(faces):
        for c in quad:
            p = rot(corners[c])
            positions.append(p)
            normals.append(rot(tuple(nrm)))
            uvs.append((0.0, 0.0))
        indices.append(base + 0); indices.append(base + 1); indices.append(base + 2)
        indices.append(base + 0); indices.append(base + 2); indices.append(base + 3)
        base += 4
    return positions, normals, uvs, indices


def build_shattered_cube():
    """Assemble several debris chunks: one shrunken central remnant + K scattered bits."""
    all_pos, all_nrm, all_uv, all_idx = [], [], [], []
    offset = 0

    # Central remnant: a smaller, slightly tilted cube (the "still-standing" core)
    p, n, u, i = box(0.45, (0.0, -0.1, 0.0), ['z', 'x'], [0.3, 0.2])
    all_pos += p; all_nrm += n; all_uv += u
    for x in i: all_idx.append(x + offset)
    offset += len(p)

    # Scattered debris chunks (offset/rotated so silhouette clearly reads "broken")
    chunks = [
        (0.30, ( 0.55,  0.35,  0.10), ['x', 'y'], [0.9, 0.6]),
        (0.26, (-0.50,  0.40, -0.20), ['y', 'z'], [1.1, 0.4]),
        (0.22, ( 0.30, -0.45,  0.35), ['z', 'x'], [0.5, 1.2]),
        (0.20, (-0.35, -0.40, -0.30), ['x', 'y'], [1.4, 0.8]),
        (0.18, ( 0.10,  0.55, -0.45), ['y', 'z'], [0.7, 1.0]),
    ]
    for scale, center, axes, angs in chunks:
        p, n, u, i = box(scale, center, axes, angs)
        all_pos += p; all_nrm += n; all_uv += u
        for x in i: all_idx.append(x + offset)
        offset += len(p)

    return all_pos, all_nrm, all_uv, all_idx


def main():
    out_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                           "assets", "models")
    os.makedirs(out_dir, exist_ok=True)
    gltf_path = os.path.join(out_dir, "dev_test_cube_destroyed.gltf")
    bin_path  = os.path.join(out_dir, "dev_test_cube_destroyed.bin")

    positions, normals, uvs, indices = build_shattered_cube()
    n_vert = len(positions)

    # --- Binary buffer: positions, normals, uvs, then indices (uint16) ---
    pos_b = b''.join(struct.pack('<fff', *p) for p in positions)
    nrm_b = b''.join(struct.pack('<fff', *n) for n in normals)
    uv_b  = b''.join(struct.pack('<ff',  *v) for v in uvs)
    idx_b = b''.join(struct.pack('<H', i) for i in indices)

    pos_off = 0
    nrm_off = len(pos_b)
    uv_off  = nrm_off + len(nrm_b)
    idx_off = uv_off + len(uv_b)
    blob = pos_b + nrm_b + uv_b + idx_b

    minp = [min(p[a] for p in positions) for a in range(3)]
    maxp = [max(p[a] for p in positions) for a in range(3)]

    gltf = {
        "asset": {"version": "2.0",
                  "generator": "tools/gen_dev_destroyed_cube.py (shattered cube placeholder)"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0}],
        "meshes": [{
            "primitives": [{
                "attributes": {"POSITION": 0, "NORMAL": 1, "TEXCOORD_0": 2},
                "indices": 3,
                "material": 0,
                "mode": 4
            }]
        }],
        "materials": [{
            "pbrMetallicRoughness": {
                "baseColorFactor": [0.85, 0.15, 0.25, 1.0],
                "metallicFactor": 0.0,
                "roughnessFactor": 0.9
            }
        }],
        "buffers": [{"byteLength": len(blob), "uri": "dev_test_cube_destroyed.bin"}],
        "bufferViews": [
            {"buffer": 0, "byteOffset": pos_off, "byteLength": len(pos_b), "target": 34962},
            {"buffer": 0, "byteOffset": nrm_off, "byteLength": len(nrm_b), "target": 34962},
            {"buffer": 0, "byteOffset": uv_off,  "byteLength": len(uv_b),  "target": 34962},
            {"buffer": 0, "byteOffset": idx_off, "byteLength": len(idx_b), "target": 34963},
        ],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": n_vert, "type": "VEC3",
             "min": minp, "max": maxp},
            {"bufferView": 1, "componentType": 5126, "count": n_vert, "type": "VEC3"},
            {"bufferView": 2, "componentType": 5126, "count": n_vert, "type": "VEC2"},
            {"bufferView": 3, "componentType": 5123, "count": len(indices), "type": "SCALAR"},
        ],
    }

    with open(gltf_path, 'w') as f:
        json.dump(gltf, f, separators=(',', ':'))
    with open(bin_path, 'wb') as f:
        f.write(blob)

    print(f"Wrote {gltf_path}")
    print(f"Wrote {bin_path}")
    print(f"  vertices={n_vert} indices={len(indices)} bin_bytes={len(blob)}")


if __name__ == "__main__":
    main()
