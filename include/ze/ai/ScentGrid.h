#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace ai {

// Particle-based scent emitter (M6, M5, M2.7)
struct ScentParticle {
    glm::vec3 position;
    float strength;        // 0..1, decays over time
    float lifetime;        // seconds remaining
    uint32_t sourceId;     // entity that produced the scent
    uint8_t scentType;     // 0=food, 1=blood, 2=sweat, 3=smoke, 4=decoy
};

class ScentGrid {
public:
    ScentGrid(float worldSize = 2000.0f, float cellSize = 50.0f);

    // Add scent at position
    void addScent(const glm::vec3& position, float strength, uint32_t sourceId, uint8_t scentType);

    // Query scent at position (returns aggregate strength)
    float queryScent(const glm::vec3& position, uint32_t& nearestSourceId) const;

    // Wind transport
    void applyWind(const glm::vec3& windVector, float dt);

    // Decay
    void tick(float dt);

    // Clear
    void clear();

private:
    float worldSize_;
    float cellSize_;
    int gridDim_;
    std::vector<std::vector<ScentParticle>> cells_;

    int cellIndex(glm::vec3 pos) const;
};

} // namespace ai
