#version 450
// [M4.5-EXT-33] AgX filmic tonemapper (fullscreen post pass).
// AgX approximation: three.js port of Troy Sobotka's AgX (MIT). The engine presents to an
// SDR UNORM swapchain that is treated as display-linear, so we apply a 1/2.2 gamma at the end
// (matches the spec math: color = AgX(x * exposure); out = pow(color, 1/2.2)).
layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D hdrTex;
layout(binding = 1) uniform ExposureUBO { float exposure; } uExposure;

vec3 agxDefaultContrastApprox(vec3 x) {
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return + 15.5   * x4 * x2
           - 40.14  * x4 * x
           + 31.96  * x4
           -  6.868 * x2 * x
           +  0.4298 * x2
           +  0.1191 * x
           -  0.00232;
}

vec3 agx(vec3 val) {
    const mat3 agx_mat = mat3(
        0.842479062853211,  0.0423282422610123, 0.0423756549057051,
        0.0784335999999992, 0.878468636469772,  0.0784336,
        0.0792237451477641, 0.0791661274605434, 0.879142973793104);
    const mat3 agx_mat_inv = mat3(
        1.196879005824241,   -0.0528968517574562, -0.0529716355144438,
       -0.0980208811401368,  1.151903129904171,  -0.0980434501171241,
       -0.0990297440797205, -0.0989611768448433,  1.151073672641160);
    const float minEv = -12.47393;
    const float maxEv =   4.026069;

    val = agx_mat * val;
    val = clamp(log2(val), minEv, maxEv);
    val = (val - minEv) / (maxEv - minEv);
    val = agxDefaultContrastApprox(val);
    val = agx_mat_inv * val;
    return clamp(val, 0.0, 1.0);
}

void main() {
    vec3 hdr = texture(hdrTex, vUv).rgb;
    vec3 mapped = agx(hdr * uExposure.exposure);
    outColor = vec4(pow(mapped, vec3(1.0 / 2.2)).bgr, 1.0);
}
