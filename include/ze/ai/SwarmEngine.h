#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <memory>
#include <glm/glm.hpp>

namespace ze::ai {

// Simulation tier for World War Z dual-tier architecture
enum class SwarmAgentTier : uint8_t {
    MacroFlocker = 0,    // Flow-field flocker, simplified collision, VAT animation
    MicroCombatActor = 1 // Promoted within 6m: full skeletal hitboxes, dismemberment, melee grapple
};

// Zombie pyramid climbing state
enum class PyramidState : uint8_t {
    GroundFlocking = 0,
    AscendingPyramid = 1,
    CrestedLedge = 2,
    FallingRagdoll = 3   // Triggered when pyramid base structural health collapses
};

// Flow Field 2D/3D direction cell for O(1) pathfinding
struct FlowFieldCell {
    float dirX{ 0.0f };
    float dirY{ 0.0f };
    float dirZ{ 0.0f };
    float cost{ 9999.0f };
};

// Flow Field Grid (128x128 cells, 0.5m cell size = 64m x 64m zone)
class FlowFieldGrid {
public:
    static constexpr uint32_t GRID_DIM = 128;
    static constexpr float CELL_SIZE = 0.5f;

    FlowFieldGrid() {
        cells_.resize(GRID_DIM * GRID_DIM);
    }

    void generate(const glm::vec3& targetWorldPos) {
        targetPos_ = targetWorldPos;
        originX_ = targetWorldPos.x - (GRID_DIM * CELL_SIZE * 0.5f);
        originZ_ = targetWorldPos.z - (GRID_DIM * CELL_SIZE * 0.5f);

        for (uint32_t z = 0; z < GRID_DIM; ++z) {
            for (uint32_t x = 0; x < GRID_DIM; ++x) {
                float cellWorldX = originX_ + (static_cast<float>(x) + 0.5f) * CELL_SIZE;
                float cellWorldZ = originZ_ + (static_cast<float>(z) + 0.5f) * CELL_SIZE;

                float dx = targetWorldPos.x - cellWorldX;
                float dz = targetWorldPos.z - cellWorldZ;
                float dist = std::sqrt(dx * dx + dz * dz);

                FlowFieldCell& cell = cells_[z * GRID_DIM + x];
                cell.cost = dist;
                if (dist > 0.001f) {
                    cell.dirX = dx / dist;
                    cell.dirY = 0.0f;
                    cell.dirZ = dz / dist;
                } else {
                    cell.dirX = 0.0f;
                    cell.dirY = 0.0f;
                    cell.dirZ = 0.0f;
                }
            }
        }
    }

    void sampleDirection(const glm::vec3& worldPos, float& outDirX, float& outDirY, float& outDirZ) const {
        int32_t gridX = static_cast<int32_t>((worldPos.x - originX_) / CELL_SIZE);
        int32_t gridZ = static_cast<int32_t>((worldPos.z - originZ_) / CELL_SIZE);

        if (gridX >= 0 && gridX < static_cast<int32_t>(GRID_DIM) &&
            gridZ >= 0 && gridZ < static_cast<int32_t>(GRID_DIM)) {
            const FlowFieldCell& c = cells_[gridZ * GRID_DIM + gridX];
            outDirX = c.dirX;
            outDirY = c.dirY;
            outDirZ = c.dirZ;
        } else {
            // Outside grid: fallback to direct normalized vector to target
            float dx = targetPos_.x - worldPos.x;
            float dz = targetPos_.z - worldPos.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist > 0.001f) {
                outDirX = dx / dist;
                outDirY = 0.0f;
                outDirZ = dz / dist;
            } else {
                outDirX = 0.0f;
                outDirY = 0.0f;
                outDirZ = 0.0f;
            }
        }
    }

private:
    std::vector<FlowFieldCell> cells_;
    glm::vec3 targetPos_{ 0.0f };
    float originX_{ 0.0f };
    float originZ_{ 0.0f };
};

// 64-byte Cache-Aligned Swarm Agent (Structure of Arrays / Packed AoS)
struct alignas(64) SwarmAgent {
    glm::vec3 position{ 0.0f };
    float health{ 100.0f };

    glm::vec3 velocity{ 0.0f };
    float speed{ 4.5f }; // meters/second sprinting speed

    SwarmAgentTier tier{ SwarmAgentTier::MacroFlocker };
    PyramidState pyramidState{ PyramidState::GroundFlocking };
    uint16_t pyramidAnchorId{ 0 };
    uint8_t animPhase{ 0 }; // 0-255 loop phase for Vertex Animation Texture (VAT)
    bool active{ false };

    float climbProgress{ 0.0f }; // 0.0 (base) to 1.0 (crested ledge)
    uint32_t entityId{ 0 };
};

// Climbing anchor for World War Z Living Zombie Pyramids
struct PyramidClimbingAnchor {
    uint16_t anchorId{ 0 };
    glm::vec3 basePosition{ 0.0f };
    glm::vec3 ledgePosition{ 0.0f };
    float ledgeHeight{ 4.5f };           // Meters high (e.g. shipping container or roof ledge)
    float baseRadius{ 3.5f };            // Footprint radius where flockers pool
    float baseStructuralHealth{ 500.0f };// Damaging the base causes pyramid collapse
    float maxStructuralHealth{ 500.0f };
    uint32_t activeClimberCount{ 0 };
    bool isCollapsed{ false };
    float collapseRecoveryTimer{ 0.0f };
};

// Hierarchical audio clustering for 1,000-zombie swarm
struct SwarmClusterAudio {
    glm::vec3 centroid{ 0.0f };
    glm::vec3 averageVelocity{ 0.0f };
    uint32_t totalActiveZombies{ 0 };
    float massRoarVolume{ 0.0f }; // Logarithmic volume: 0.0 to 1.0
    float pitchDoppler{ 1.0f };
};

// Saber Interactive Swarm Engine 1,000-Zombie Horde Architecture
class SwarmEngine {
public:
    SwarmEngine();
    ~SwarmEngine() = default;

    void initialize(size_t maxAgents = 1024);
    void reset();

    // Register climbable environmental structures (fences, buses, rooftops)
    uint16_t registerPyramidAnchor(const glm::vec3& basePos, const glm::vec3& ledgePos);

    // Spawn massive waves (called by L4D2Director during crescendos/hordes)
    size_t spawnSwarmWave(size_t count, const glm::vec3& spawnOrigin, float initialDispersionRadius = 5.0f);

    // Simulation tick update (60Hz / variable dt)
    void update(float dt, const glm::vec3& survivorCenter, float survivorFlowDist);

    // Combat interactions: Damage structural base of living pyramid
    bool damagePyramidBase(uint16_t anchorId, float damage);

    // Damage a swarm agent by raycast or explosion
    bool applyDamage(size_t agentIndex, float damage, const glm::vec3& impactDir);

    // Queries
    size_t getActiveAgentCount() const { return activeCount_; }
    size_t getPromotedCombatActorCount() const { return promotedCombatCount_; }
    const SwarmAgent* getAgents() const { return agents_.data(); }
    const SwarmClusterAudio& getAudioTelemetry() const { return clusterAudio_; }
    const std::vector<PyramidClimbingAnchor>& getPyramidAnchors() const { return anchors_; }

    // Tuning configuration
    struct Config {
        float flockerSpeed{ 4.8f };             // Sprinting speed (m/s)
        float promotionDistance{ 6.0f };        // Proximity to promote to Tier B Combat Actor
        float demotionDistance{ 8.5f };         // Proximity to demote back to Tier A Flocker
        float separationRadius{ 0.65f };        // Soft flocker-flocker repulsion distance
        float separationStrength{ 12.0f };      // Repulsion force
        float pyramidClimbSpeed{ 2.2f };        // Vertical climbing speed (m/s)
        float pyramidTriggerDensity{ 8.0f };    // Local zombies needed at base to trigger climbing
    } config;

private:
    void updateFlowField(const glm::vec3& survivorCenter);
    void updateFlockingMovement(float dt, const glm::vec3& survivorCenter);
    void updatePyramidClimbing(float dt);
    void updateTwoTierPromotion(const glm::vec3& survivorCenter);
    void updateAudioCentroid();

    std::vector<SwarmAgent> agents_;
    std::vector<PyramidClimbingAnchor> anchors_;
    FlowFieldGrid flowField_;

    size_t activeCount_{ 0 };
    size_t promotedCombatCount_{ 0 };
    SwarmClusterAudio clusterAudio_;
    uint32_t nextEntityId_{ 10000 };
};

} // namespace ze::ai
