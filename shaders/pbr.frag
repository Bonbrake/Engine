#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Normal Distribution Function: Trowbridge-Reitz GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / max(denom, 0.0000001);
}

// Geometry Function: Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / max(denom, 0.0000001);
}

// Geometry Function: Smith's method
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel Function: Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Normal vector
    vec3 N = normalize(fragNormal);
    
    // View vector (assuming camera is looking from +Z / standard coordinate)
    // Camera is roughly at (0, 0, 4) in camera-relative coords
    vec3 camPos = vec3(0.0, 0.0, 4.0);
    vec3 V = normalize(camPos - fragWorldPos);
    
    // Material parameters (PBR metallic-roughness baseline)
    vec3 albedo = vec3(0.75, 0.72, 0.68); // Realistic concrete/zombie flesh base
    float metallic = 0.08;                // Dielectric with slight surface conductance
    float roughness = 0.45;               // Semi-rough diffuse surface
    float ao = 1.0;                       // Full ambient occlusion baseline
    
    // Calculate reflectance at normal incidence F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // Directional Sun Light (T1-06)
    vec3 lightDir = normalize(vec3(0.45, 0.85, 0.35));
    vec3 lightColor = vec3(1.0, 0.98, 0.92) * 2.8; // Warm direct sunlight
    
    vec3 L = lightDir;
    vec3 H = normalize(V + L);
    
    float NdotL = max(dot(N, L), 0.0);
    
    // Cook-Torrance Specular BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    // Outgoing radiance from directional light
    vec3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL;
    
    // Ambient lighting (simulated sky hemisphere irradiance)
    vec3 ambientColor = vec3(0.12, 0.15, 0.20); // Cool sky dome ambient
    vec3 ambient = ambientColor * albedo * ao;
    
    vec3 color = ambient + Lo;
    
    // HDR Reinhard tone mapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction (sRGB curve approx 2.2)
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}
