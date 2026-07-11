#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

struct InstanceData {
    vec4 position_radius;
};

layout(set = 0, binding = 0, std430) readonly buffer InstanceBuffer {
    InstanceData instances[];
};

void main() {
    vec3 offset = instances[gl_InstanceIndex].position_radius.xyz;
    gl_Position = pc.mvp * vec4(inPosition + offset, 1.0);
    // Use instance index to vary color
    float c = float(gl_InstanceIndex % 3);
    vec3 instanceColor = vec3(c == 0 ? 1.0 : 0.0, c == 1 ? 1.0 : 0.5, c == 2 ? 1.0 : 0.2);
    fragColor = inColor * instanceColor;
}