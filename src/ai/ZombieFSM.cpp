#include "ze/ai/ZombieFSM.h"
#include <cmath>
#include <algorithm>

namespace ai {

ZombieFSM::ZombieFSM() {
    reset();
}

void ZombieFSM::reset() {
    state_ = ZombieState::Idle;
    memory_ = ZombieMemory{};
    stateTimer_ = 0.0f;
}

void ZombieFSM::receiveStimulus(ZombieMemory::StimulusType type, const glm::vec3& position, float strength) {
    // Only process if this stimulus is more important than current memory
    uint8_t typePriority = static_cast<uint8_t>(type);
    uint8_t currentPriority = static_cast<uint8_t>(memory_.lastStimulus);

    if (typePriority >= currentPriority && strength > memory_.stimulusStrength) {
        memory_.lastStimulus = type;
        memory_.stimulusPosition = position;
        memory_.stimulusStrength = std::min(strength, 1.0f);
        memory_.memoryTimer = memoryDuration_;
    }
}

void ZombieFSM::receiveGroupAlert(uint32_t level) {
    if (level > memory_.groupAlertLevel) {
        memory_.groupAlertLevel = level;
        memory_.stimulusStrength = std::max(memory_.stimulusStrength, level / 3.0f);
    }
}

void ZombieFSM::tick(float dt, const glm::vec3& selfPos, const glm::vec3& playerPos,
                      float surfaceSlope, float health, float infection, float limbIntegrity) {
    stateTimer_ += dt;
    decayMemory(dt);

    // Evaluate sensory input
    evaluateSensory(selfPos, playerPos);

    // State machine transitions (M2.7-EXT-21)
    switch (state_) {
    case ZombieState::Idle:
        // Transition to Patrol after a few seconds of idling
        if (stateTimer_ > 3.0f) {
            transitionTo(ZombieState::Patrol, 0);
        }
        // Direct transitions from memory
        if (memory_.stimulusStrength > 0.3f) {
            transitionTo(ZombieState::Investigate, 0);
        }
        break;

    case ZombieState::Patrol:
        // Patrol for 10-30 seconds, then idle
        if (stateTimer_ > (15.0f + std::fmod(infection * 5.0f, 15.0f))) {
            transitionTo(ZombieState::Idle, 0);
        }
        // Heard or saw something
        if (memory_.stimulusStrength > 0.5f) {
            transitionTo(ZombieState::Investigate, 0);
        }
        // Group alert escalation
        if (memory_.groupAlertLevel >= 2) {
            transitionTo(ZombieState::Chase, 0);
        }
        break;

    case ZombieState::Investigate:
        // Investigate for 5-15 seconds
        if (stateTimer_ > (5.0f + limbIntegrity * 10.0f)) {
            if (memory_.stimulusStrength > 0.7f) {
                transitionTo(ZombieState::Chase, 0);
            } else {
                transitionTo(ZombieState::Patrol, 0);
            }
        }
        // If stimulus position is close, escalate
        if (memory_.stimulusStrength > 0.8f) {
            transitionTo(ZombieState::Chase, 0);
        }
        break;

    case ZombieState::Chase:
        // Chase for up to 30 seconds based on limb integrity
        if (stateTimer_ > (10.0f + limbIntegrity * 20.0f)) {
            transitionTo(ZombieState::Patrol, 0);
        }
        // Feign death trigger (M2.7-EXT-23)
        if (health < 15.0f && feignChance_ > 0.0f) {
            float roll = std::fmod(stateTimer_ * 0.1f, 1.0f);
            if (roll < feignChance_) {
                transitionTo(ZombieState::FeignDeath, 0);
            }
        }
        break;

    case ZombieState::Attack:
        // Attack timer: 2-4 seconds between swings
        if (stateTimer_ > (2.0f + infection * 2.0f)) {
            transitionTo(ZombieState::Chase, 0);
        }
        break;

    case ZombieState::FeignDeath:
        // Stay feigned for 5-15 seconds
        if (stateTimer_ > (5.0f + limbIntegrity * 10.0f) || memory_.stimulusStrength > 0.9f) {
            transitionTo(ZombieState::Idle, 0);
        }
        break;

    case ZombieState::Dead:
        // Terminal state
        break;
    }
}

void ZombieFSM::transitionTo(ZombieState newState, float) {
    state_ = newState;
    stateTimer_ = 0.0f;
}

void ZombieFSM::evaluateSensory(const glm::vec3& selfPos, const glm::vec3& playerPos) {
    float dist = glm::distance(selfPos, playerPos);

    // Sight cone evaluation
    if (dist < sightRange_) {
        glm::vec3 toPlayer = glm::normalize(playerPos - selfPos);
        float dot = glm::dot(toPlayer, glm::vec3(0, 0, -1)); // facing -Z
        if (dot > 0.3f && hasLineOfSight(selfPos, playerPos)) {
            float sightStrength = 1.0f - (dist / sightRange_);
            receiveStimulus(ZombieMemory::StimulusType::Sight, playerPos, sightStrength);
        }
    }

    // Hearing: loudness decay with distance
    if (dist < 100.0f) {
        float hearingStrength = 1.0f - (dist / 100.0f);
        if (hearingStrength > 0.2f) {
            receiveStimulus(ZombieMemory::StimulusType::Sound, playerPos, hearingStrength * hearingSensitivity_);
        }
    }
}

bool ZombieFSM::hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const {
    // Placeholder: assumes line-of-sight. M2.7 spec calls for SpatialHash ray query.
    // TODO: integrate with M1-EXT-40 RayBatchQuery once implemented.
    return true;
}

void ZombieFSM::decayMemory(float dt) {
    memory_.memoryTimer -= dt;
    if (memory_.memoryTimer <= 0.0f) {
        memory_.lastStimulus = ZombieMemory::StimulusType::None;
        memory_.stimulusStrength *= 0.95f; // residual decay
        memory_.groupAlertLevel = 0;
    }
}

} // namespace ai
