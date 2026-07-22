#include "ze/world/BiomeGraph.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace world {

// Whittaker biome anchor table [temperature °C × precipitation mm/year]
static constexpr BiomeAnchor kWhittakerAnchors[] = {
    {BiomeType::Tundra,         -10,   250},
    {BiomeType::Taiga,          -5,    500},
    {BiomeType::TemperateForest, 10,  1000},
    {BiomeType::TropicalForest,  25,  2000},
    {BiomeType::Grassland,       15,   600},
    {BiomeType::Savanna,         22,   800},
    {BiomeType::Desert,          30,   100},
    {BiomeType::Shrubland,       18,   400},
    {BiomeType::Swamp,           18,  1500},
    {BiomeType::Mountain,         2,   800},
    {BiomeType::Ocean,           15,   500},
    {BiomeType::Snow,            -15,  200},
};
static constexpr int kNumBiomes = sizeof(kWhittakerAnchors) / sizeof(kWhittakerAnchors[0]);

BiomeGraph::BiomeGraph(uint32_t seed, uint32_t worldSize)
    : seed_(seed), worldSize_(worldSize) {}

float BiomeGraph::fbmNoise(float x, float y, uint32_t seed, int octaves,
                            float lacunarity, float gain) const {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxAmplitude = 0.0f;

    // Inject seed as offset (GLM noise is seedless)
    float ox = (seed & 0xFFFF) * 1000.0f;
    float oy = ((seed >> 16) & 0xFFFF) * 1000.0f;

    for (int i = 0; i < octaves; ++i) {
        float n = glm::simplex(glm::vec2(x * frequency + ox, y * frequency + oy));
        value += amplitude * n;
        maxAmplitude += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return value / maxAmplitude;
}

float BiomeGraph::ridgeNoise(float x, float y, uint32_t seed, int octaves) const {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxAmplitude = 0.0f;
    float ox = (seed & 0xFFFF) * 1000.0f;
    float oy = ((seed >> 16) & 0xFFFF) * 1000.0f;

    for (int i = 0; i < octaves; ++i) {
        float n = glm::simplex(glm::vec2(x * frequency + ox, y * frequency + oy));
        n = 1.0f - std::abs(n); // ridge: invert to create valleys
        n = n * n * n;          // sharpen
        value += amplitude * n;
        maxAmplitude += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    return value / maxAmplitude;
}

float BiomeGraph::getTemperature(float worldX, float worldZ) const {
    // Temperature: latitude-based gradient + noise perturbation
    float latFactor = 1.0f - std::abs(worldZ) / (worldSize_ * 0.5f); // equator at z=0
    float baseTemp = latFactor * 40.0f - 10.0f; // -10..30°C
    float noise = fbmNoise(worldX * 0.002f, worldZ * 0.002f, seed_ + 1, 4, 2.0f, 0.5f);
    return baseTemp + noise * 10.0f; // ±10°C perturbation
}

float BiomeGraph::getPrecipitation(float worldX, float worldZ) const {
    // Precipitation from noise + orographic lift (simplified)
    float baseNoise = fbmNoise(worldX * 0.003f, worldZ * 0.003f, seed_ + 2, 3, 1.8f, 0.4f);
    float precip = (baseNoise * 0.5f + 0.5f) * 2500.0f; // 0..2500mm

    // Orographic: more rain near mountains (noise high-freq)
    float orographic = ridgeNoise(worldX * 0.005f, worldZ * 0.005f, seed_ + 3, 3);
    precip += orographic * 500.0f;
    return std::max(0.0f, precip);
}

BiomeType BiomeGraph::classify(uint32_t chunkX, uint32_t chunkZ) const {
    float cx = static_cast<float>(chunkX) * CHUNK_SIZE + CHUNK_SIZE * 0.5f;
    float cz = static_cast<float>(chunkZ) * CHUNK_SIZE + CHUNK_SIZE * 0.5f;
    float temperature = getTemperature(cx, cz);
    float precipitation = getPrecipitation(cx, cz);
    return classifyClimate(temperature, precipitation);
}

BiomeType BiomeGraph::classifyClimate(float temperatureC, float precipitationMM) const {
    float bestDist = std::numeric_limits<float>::max();
    int bestIdx = 0;

    for (int i = 0; i < kNumBiomes; ++i) {
        float dTemp = temperatureC - kWhittakerAnchors[i].temperature;
        float dPrecip = precipitationMM - kWhittakerAnchors[i].precipitation;
        // Normalized: temperature range ~50°C, precip range ~2500mm
        float dist = std::sqrt(
            (dTemp / 25.0f) * (dTemp / 25.0f) +
            (dPrecip / 1250.0f) * (dPrecip / 1250.0f)
        );
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }
    return kWhittakerAnchors[bestIdx].type;
}

float BiomeGraph::getBlendFactor(uint32_t chunkX, uint32_t chunkZ, BiomeType neighbor) const {
    float cx = static_cast<float>(chunkX) * CHUNK_SIZE + CHUNK_SIZE * 0.5f;
    float cz = static_cast<float>(chunkZ) * CHUNK_SIZE + CHUNK_SIZE * 0.5f;
    BiomeType self = classify(chunkX, chunkZ);
    if (self == neighbor) return 1.0f;

    constexpr float BLEND_RADIUS = 2.5f * CHUNK_SIZE;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dz == 0) continue;
            float nx = cx + dx * CHUNK_SIZE;
            float nz = cz + dz * CHUNK_SIZE;
            BiomeType candidate = classify(static_cast<uint32_t>(chunkX + dx),
                                            static_cast<uint32_t>(chunkZ + dz));
            if (candidate == neighbor) {
                float dist = std::sqrt(
                    (dx * CHUNK_SIZE) * (dx * CHUNK_SIZE) +
                    (dz * CHUNK_SIZE) * (dz * CHUNK_SIZE)
                );
                float blend = 1.0f - (dist / BLEND_RADIUS);
                if (blend > 0.0f) return blend;
            }
        }
    }
    return 0.0f;
}

float BiomeGraph::getHeight(uint32_t chunkX, uint32_t chunkZ, float localX, float localZ) const {
    BiomeType biome = classify(chunkX, chunkZ);
    float wx = static_cast<float>(chunkX) * CHUNK_SIZE + localX;
    float wz = static_cast<float>(chunkZ) * CHUNK_SIZE + localZ;

    float baseHeight = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;

    // Biome-first terrain: each biome defines unique terrain fingerprint
    // (Tantan 2025 methodology: biome over terrain, not terrain over biome)
    switch (biome) {
    case BiomeType::Ocean:
        amplitude = 20.0f; frequency = 0.005f; baseHeight = -10.0f; break;
    case BiomeType::Desert:
        amplitude = 15.0f; frequency = 0.003f; baseHeight = 5.0f; break;
    case BiomeType::Tundra:
        amplitude = 25.0f; frequency = 0.004f; baseHeight = 10.0f; break;
    case BiomeType::Taiga:
        amplitude = 40.0f; frequency = 0.006f; baseHeight = 15.0f; break;
    case BiomeType::TemperateForest:
        amplitude = 35.0f; frequency = 0.005f; baseHeight = 10.0f; break;
    case BiomeType::TropicalForest:
        amplitude = 50.0f; frequency = 0.007f; baseHeight = 20.0f; break;
    case BiomeType::Mountain:
        amplitude = 80.0f; frequency = 0.008f; baseHeight = 30.0f; break;
    case BiomeType::Grassland:
        amplitude = 20.0f; frequency = 0.003f; baseHeight = 5.0f; break;
    case BiomeType::Savanna:
        amplitude = 15.0f; frequency = 0.003f; baseHeight = 8.0f; break;
    case BiomeType::Shrubland:
        amplitude = 20.0f; frequency = 0.004f; baseHeight = 8.0f; break;
    case BiomeType::Swamp:
        amplitude = 10.0f; frequency = 0.002f; baseHeight = 2.0f; break;
    case BiomeType::Snow:
        amplitude = 30.0f; frequency = 0.005f; baseHeight = 15.0f; break;
    default:
        break;
    }

    // Multi-octave height shaping
    float n = fbmNoise(wx * frequency, wz * frequency, seed_ + 10, 4, 2.0f, 0.5f);
    n += fbmNoise(wx * frequency * 2.1f, wz * frequency * 2.1f, seed_ + 11, 3, 2.0f, 0.5f) * 0.5f;
    n += fbmNoise(wx * frequency * 4.3f, wz * frequency * 4.3f, seed_ + 12, 2, 2.0f, 0.5f) * 0.25f;

    return baseHeight + n * amplitude;
}

BiomePalette BiomeGraph::getPalette(BiomeType biome) const {
    BiomePalette p{};
    switch (biome) {
    case BiomeType::Desert:
        p = {{0.76f, 0.60f, 0.42f}, {0.50f, 0.70f, 0.30f}, {0.20f, 0.40f, 0.60f}, 0.1f, 0.05f};
        break;
    case BiomeType::TropicalForest:
        p = {{0.30f, 0.50f, 0.20f}, {0.10f, 0.80f, 0.15f}, {0.15f, 0.35f, 0.55f}, 0.9f, 0.02f};
        break;
    case BiomeType::Mountain:
        p = {{0.45f, 0.40f, 0.35f}, {0.35f, 0.55f, 0.25f}, {0.30f, 0.50f, 0.70f}, 0.1f, 0.30f};
        break;
    case BiomeType::Tundra:
        p = {{0.65f, 0.60f, 0.55f}, {0.40f, 0.50f, 0.30f}, {0.40f, 0.60f, 0.80f}, 0.2f, 0.10f};
        break;
    case BiomeType::Taiga:
        p = {{0.35f, 0.30f, 0.25f}, {0.15f, 0.45f, 0.15f}, {0.25f, 0.45f, 0.65f}, 0.7f, 0.08f};
        break;
    case BiomeType::Swamp:
        p = {{0.30f, 0.35f, 0.20f}, {0.20f, 0.50f, 0.15f}, {0.15f, 0.30f, 0.25f}, 0.5f, 0.03f};
        break;
    case BiomeType::Grassland:
        p = {{0.50f, 0.55f, 0.30f}, {0.30f, 0.60f, 0.20f}, {0.25f, 0.45f, 0.60f}, 0.4f, 0.04f};
        break;
    case BiomeType::Savanna:
        p = {{0.60f, 0.55f, 0.30f}, {0.40f, 0.65f, 0.20f}, {0.20f, 0.40f, 0.55f}, 0.3f, 0.06f};
        break;
    case BiomeType::Shrubland:
        p = {{0.45f, 0.50f, 0.35f}, {0.30f, 0.55f, 0.25f}, {0.25f, 0.40f, 0.55f}, 0.3f, 0.08f};
        break;
    default: // TemperateForest, Ocean, Snow
        p = {{0.40f, 0.55f, 0.25f}, {0.20f, 0.70f, 0.20f}, {0.20f, 0.40f, 0.60f}, 0.6f, 0.05f};
        break;
    }
    return p;
}

} // namespace world
