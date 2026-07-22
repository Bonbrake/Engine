#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <array>
#include <memory>
#include <random>
#include <glm/gtc/noise.hpp>

namespace world {

constexpr uint32_t CHUNK_SIZE = 256; // meters

// Canonical biome types (Whittaker classification)
enum class BiomeType : uint8_t {
    Tundra, Taiga, TemperateForest, TropicalForest,
    Grassland, Savanna, Desert, Shrubland,
    Swamp, Mountain, Ocean, Snow, COUNT
};

// Climate anchor for Whittaker classification
struct BiomeAnchor {
    BiomeType type;
    float temperature;    // °C
    float precipitation;  // mm/year
};

// Per-biome visual and ecological properties
struct BiomePalette {
    glm::vec3 groundColor;
    glm::vec3 foliageColor;
    glm::vec3 waterColor;
    float foliageDensity;
    float rockFrequency;
};

// Biome-first terrain graph (Tantan 2025 — biome over terrain)
class BiomeGraph {
public:
    BiomeGraph(uint32_t seed, uint32_t worldSize);

    // Primary classification: Whittaker temperature/precipitation
    BiomeType classify(uint32_t chunkX, uint32_t chunkZ) const;
    BiomeType classifyClimate(float temperatureC, float precipitationMM) const;

    // Blend factor toward neighbor biome over 2-3 chunk boundary
    float getBlendFactor(uint32_t chunkX, uint32_t chunkZ, BiomeType neighbor) const;

    // Biome-first terrain height: biome identity drives height, not inverse
    float getHeight(uint32_t chunkX, uint32_t chunkZ, float localX, float localZ) const;

    // Get visual palette for a biome
    BiomePalette getPalette(BiomeType biome) const;

    // Access climate noise directly for macro-scale features
    float getTemperature(float worldX, float worldZ) const;
    float getPrecipitation(float worldX, float worldZ) const;

    uint32_t seed() const { return seed_; }
    uint32_t worldSize() const { return worldSize_; }

private:
    uint32_t seed_;
    uint32_t worldSize_;

    // Noise evaluation helpers using GLM
    float fbmNoise(float x, float y, uint32_t seed, int octaves, float lacunarity, float gain) const;
    float ridgeNoise(float x, float y, uint32_t seed, int octaves) const;
};

} // namespace world
