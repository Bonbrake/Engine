#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <array>
#include <vector>

namespace ai {

// Zombie sensory state (M2.7-EXT-21..23, M5)
struct ZombieMemory {
    enum class StimulusType : uint8_t { None, Sight, Sound, Scent, Damage };
    StimulusType lastStimulus = StimulusType::None;
    glm::vec3 stimulusPosition{};
    float stimulusStrength = 0.0f;    // 0..1, decays over time
    float memoryTimer = 0.0f;         // seconds remaining before forgetting
    uint32_t groupAlertLevel = 0;     // 0=idle, 1=curious, 2=alert, 3=feral
};

// Zombie locomotion/behavior state
enum class ZombieState : uint8_t {
    Idle, Patrol, Investigate, Chase, Attack, FeignDeath, Dead
};

// Zombie FSM configuration per spec M2.7
class ZombieFSM {
public:
    ZombieFSM();

    // Per-tick update
    void tick(float dt, const glm::vec3& selfPos, const glm::vec3& playerPos,
              float surfaceSlope, float health, float infection, float limbIntegrity);

    // Stimulus input
    void receiveStimulus(ZombieMemory::StimulusType type, const glm::vec3& position, float strength);
    void receiveGroupAlert(uint32_t level);

    // Queries
    ZombieState currentState() const { return state_; }
    const ZombieMemory& memory() const { return memory_; }
    float stateTimer() const { return stateTimer_; }
    float feignDeathChance() const { return feignChance_; }

    // Configuration
    void setSightRange(float meters) { sightRange_ = meters; }
    void setHearing(float sensitivity) { hearingSensitivity_ = sensitivity; }
    void setFeignDeathChance(float chance) { feignChance_ = chance; }

    // Reset
    void reset();

private:
    ZombieState state_ = ZombieState::Idle;
    ZombieMemory memory_;
    float stateTimer_ = 0.0f;
    float feignChance_ = 0.1f;

    // Sensory parameters
    float sightRange_ = 50.0f;
    float hearingSensitivity_ = 1.0f;
    float memoryDuration_ = 30.0f;  // seconds

    // Internal state transitions
    void transitionTo(ZombieState newState, float dt);
    void evaluateSensory(const glm::vec3& selfPos, const glm::vec3& playerPos);
    bool hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const;
    void decayMemory(float dt);
};

} // namespace ai
