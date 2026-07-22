#include "ze/survival/StaminaSystem.h"
#include <algorithm>
#include <cmath>

namespace survival {

void StaminaSystem::tick(float dt, float exertion, float bodyTemp,
                          float hungerLevel, float thirstLevel, bool sprinting) {
    updateRegenMultipliers(bodyTemp, hungerLevel, thirstLevel);

    // Stamina cost from exertion
    float exertionCost = exertion * 12.0f * dt;
    if (sprinting) {
        exertionCost *= 3.0f;
    }

    // Apply cost before regen
    state_.stamina -= exertionCost;
    if (state_.stamina < 0.0f) state_.stamina = 0.0f;

    // Stamina regeneration (when below max)
    if (state_.stamina < state_.maxStamina && exertion < 0.3f) {
        float regen = state_.staminaRegen * dt;
        state_.stamina = std::min(state_.maxStamina, state_.stamina + regen);
    }

    // Fatigue accumulation
    if (exertion > 0.3f) {
        state_.fatigue += exertion * 5.0f * dt;
    } else {
        // Fatigue recovery
        state_.fatigue -= state_.fatigueDecay * dt;
    }
    state_.fatigue = std::max(0.0f, std::min(100.0f, state_.fatigue));

    // Oxygen recovery (only when not holding breath)
    if (!sprinting) {
        state_.oxygen = std::min(100.0f, state_.oxygen + state_.oxygenRegen * dt);
    }
}

bool StaminaSystem::sprint(float dt) {
    if (state_.stamina < 5.0f) return false; // exhausted

    float cost = 15.0f * dt;
    state_.stamina -= cost;
    state_.oxygen -= 8.0f * dt;

    if (state_.oxygen < 0.0f) {
        state_.oxygen = 0.0f;
        return false; // out of breath
    }
    return true;
}

bool StaminaSystem::performAction(float cost) {
    if (state_.stamina < cost) return false;
    state_.stamina -= cost;
    state_.fatigue = std::min(100.0f, state_.fatigue + cost * 0.1f);
    return true;
}

bool StaminaSystem::holdBreath(float dt) {
    state_.oxygen -= 10.0f * dt;
    if (state_.oxygen < 0.0f) {
        state_.oxygen = 0.0f;
        return false;
    }
    return true;
}

void StaminaSystem::updateRegenMultipliers(float bodyTemp, float hunger, float thirst) {
    // Temperature penalty
    float tempPenalty = 0.0f;
    if (bodyTemp < 36.0f) tempPenalty = (36.0f - bodyTemp) * 0.2f;
    if (bodyTemp > 38.0f) tempPenalty = (bodyTemp - 38.0f) * 0.3f;

    // Hunger/thirst penalty
    float needPenalty = (hunger + thirst) * 0.5f;

    // Combined penalty reduces regen rate
    float totalPenalty = std::min(1.0f, tempPenalty + needPenalty);
    state_.staminaRegen = 8.0f * (1.0f - totalPenalty);
    state_.fatigueDecay = 2.0f * (1.0f - totalPenalty * 0.5f);
}

} // namespace survival
