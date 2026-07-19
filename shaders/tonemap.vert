#version 450
// [M4.5-EXT-33] AgX tonemap fullscreen pass - generates a fullscreen triangle from gl_VertexIndex.
layout(location = 0) out vec2 vUv;

void main() {
    // (0,0),(2,0),(0,2) -> clip-space (-1,-1),(3,-1),(-1,3): standard oversized fullscreen triangle.
    vec2 uv = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    vUv = uv; // interpolates 0..1 across the visible [-1,1] region
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
