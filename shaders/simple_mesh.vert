#version 450

// Slice 0a dev-test cube: matches MeshAsset vertex layout (AssetTypes.h: Vertex{vec3 pos, vec3 norm, vec2 uv} -> stride 32B).
// No per-instance offset (unlike simple.vert, which adds instances[].position_radius and is for the demo triangles).
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    // Shade by normal so the cube reads as a solid lit object rather than a flat silhouette.
    fragColor = abs(normalize(inNormal)) * 0.8f + 0.2f;
}
