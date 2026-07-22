#include "ze/ai/AIDirector.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ai {

AIDirector::AIDirector() {
    // Default state weights (M5.4-EXT-01)
    stateWeights_[0] = 1.0f;  // Calm
    stateWeights_[1] = 0.3f;  // Tension
    stateWeights_[2] = 0.1f;  // Chase
    stateWeights_[3] = 0.05f; // Siege
    stateWeights_[4] = 0.5f;  // Recovery
    applyPersonality();
}

void AIDirector::applyPersonality() {
    switch (personality_) {
    case DirectorPersonality::Cassandra:
        escalationRate_ = 1.4f;
        calmBias_ = 0.6f;
        entropy_ = 0.3f;
        break;
    case DirectorPersonality::Phoebe:
        escalationRate_ = 1.0f;
        calmBias_ = 1.2f;
        entropy_ = 0.4f;
        break;
    case DirectorPersonality::Randy:
        escalationRate_ = 1.2f;
        calmBias_ = 1.0f;
        entropy_ = 0.9f;
        break;
    }
}

void AIDirector::registerEventTemplate(const EventTemplate& tmpl) {
    templates_.push_back(tmpl);
    cooldowns_.push_back(0.0f);
}

void AIDirector::resetCooldowns() {
    for (auto& c : cooldowns_) {
        c = 0.0f;
    }
}

void AIDirector::evaluateStateWeights(float noiseLevel, float settlementWealth,
                                       float timeOfDay, float health) {
    float nightFactor = (timeOfDay < 6.0f || timeOfDay > 20.0f) ? 1.5f : 1.0f;
    float wealthFactor = settlementWealth * 0.5f;
    float noiseFactor = noiseLevel;
    float healthFactor = std::max(0.0f, 1.0f - health / 100.0f);

    // State scores derived from trendsetter wealth/health/noise model
    stateScores_[0] = calmBias_ * (1.0f - noiseFactor * 0.3f);
    stateScores_[1] = (noiseFactor * 1.2f + healthFactor * 0.5f) * nightFactor;
    stateScores_[2] = (wealthFactor + noiseFactor * 2.0f) * nightFactor;
    stateScores_[3] = (wealthFactor * 1.5f + entropy_ * 0.5f);
    stateScores_[4] = 0.5f * (1.0f - healthFactor);
}

void AIDirector::transitionPhase() {
    // Find max-weighted state with hysteresis
    uint32_t maxState = 0;
    float maxWeight = stateScores_[0] * stateWeights_[0];
    for (uint32_t i = 1; i < 5; ++i) {
        float w = stateScores_[i] * stateWeights_[i];
        if (w > maxWeight * 1.2f) { // 20% hysteresis
            maxWeight = w;
            maxState = i;
        }
    }

    // Transition only if different from current phase and exceed threshold
    uint8_t currentPhase = static_cast<uint8_t>(phase_);
    if (maxState != currentPhase && maxWeight > 0.05f) {
        phase_ = static_cast<DirectorPhase>(maxState);
        phaseTimer_ = 0.0f;
    }
}

uint32_t AIDirector::selectEvent() {
    if (templates_.empty()) return 0;

    // Filter available events
    float totalWeight = 0.0f;
    for (size_t i = 0; i < templates_.size(); ++i) {
        if (cooldowns_[i] <= 0.0f) {
            totalWeight += templates_[i].weight;
        }
    }
    if (totalWeight <= 0.0f) return 0;

    // Weighted random selection
    float roll = std::fmod(phaseTimer_ * 137.0f, totalWeight); // deterministic
    float accum = 0.0f;
    for (size_t i = 0; i < templates_.size(); ++i) {
        if (cooldowns_[i] <= 0.0f) {
            accum += templates_[i].weight;
            if (roll <= accum) {
                cooldowns_[i] = templates_[i].cooldown;
                return templates_[i].id;
            }
        }
    }
    return 0;
}

void AIDirector::decayCooldowns(float dt) {
    for (auto& c : cooldowns_) {
        if (c > 0.0f) {
            c -= dt;
        }
    }
}

void AIDirector::tick(float dt, const glm::vec3& playerPos,
                       float health, float noiseLevel, float settlementWealth,
                       float timeOfDay, uint32_t population) {
    phaseTimer_ += dt;
    decayCooldowns(dt);

    // Evaluate what the player's situation is
    evaluateStateWeights(noiseLevel, settlementWealth, timeOfDay, health);

    // Check if we should transition to a new state
    transitionPhase();

    // If enough time passed in current state, fire an event
    if (phaseTimer_ > 15.0f && phaseTimer_ < 16.0f + entropy_) {
        pendingEvent_ = selectEvent();
    }
}

uint32_t AIDirector::pollEvent() {
    uint32_t event = pendingEvent_;
    pendingEvent_ = 0;
    return event;
}

} // namespace ai
