#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragCameraRelPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

// 128-byte push constants: mat4 mvp (64B) + mat4 model (64B)
layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} pc;

void main() {
    // Transform position to camera-relative world space
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    fragCameraRelPos = worldPos.xyz;
    
    // Normal in camera-relative world space
    fragNormal = normalize(mat3(pc.model) * inNormal);
    fragTexCoord = inTexCoord;
    
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
}
