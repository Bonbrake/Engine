#pragma once
#include <cstdint>

namespace survival {

// Stamina system (M0-M2 core survival stats)
struct StaminaState {
    float stamina = 100.0f;       // 0..100, current stamina
    float maxStamina = 100.0f;    // maximum possible
    float fatigue = 0.0f;         // 0..100, accumulated fatigue
    float oxygen = 100.0f;        // 0..100, breath hold/sprint O₂

    // Recovery rates (per second)
    float staminaRegen = 8.0f;    // normal recovery rate
    float fatigueDecay = 2.0f;    // fatigue recovery rate
    float oxygenRegen = 15.0f;    // oxygen recovery when breathing

    // Thresholds
    static constexpr float EXHAUSTED = 15.0f;
    static constexpr float WINDED = 30.0f;
};

class StaminaSystem {
public:
    StaminaSystem() = default;

    // Per-tick update: dt in seconds
    void tick(float dt, float exertion, float bodyTemp,
              float hungerLevel, float thirstLevel, bool sprinting);

    // Actions that cost stamina
    bool sprint(float dt);                        // returns false if exhausted
    bool performAction(float cost);               // returns false if insufficient
    bool holdBreath(float dt);                    // returns false if out of O₂

    // Queries
    const StaminaState& state() const { return state_; }
    float staminaFraction() const { return state_.stamina / state_.maxStamina; }
    bool isExhausted() const { return state_.stamina < StaminaState::EXHAUSTED; }
    bool isWindled() const { return state_.stamina < StaminaState::WINDED; }
    float fatiquePenalty() const {
        // Accuracy/crafting penalty from fatigue (0..20% reduction)
        return state_.fatigue * 0.002f;
    }

    // Reset
    void reset() { state_ = StaminaState{}; }

private:
    StaminaState state_;

    void updateRegenMultipliers(float bodyTemp, float hunger, float thirst);
};

} // namespace survival
