#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace survival {

// Body temperature system (M0-M2)
// Uses CLO units: 1 CLO = 0.155 m²·K/W thermal insulation
// Naked ≈ 1 CLO, coat ≈ 4 CLO

struct BodyTempState {
    float coreTemp = 37.0f;       // °C, normal human core
    float skinTemp = 33.0f;       // °C, normal skin temp
    float wetness = 0.0f;         // 0..1, wetness from rain/sweat
    float metabolism = 1.0f;      // 1.0 = resting metabolism multiplier
    float clothingCLO = 1.0f;     // 1 CLO = light clothing

    // Derived
    float heatLoss = 0.0f;        // W/m² (positive = losing heat)
    float shiverIntensity = 0.0f; // 0..1
    float sweatRate = 0.0f;       // 0..1

    // Thresholds
    static constexpr float HYPOTHERMIA_THRESHOLD = 35.0f;
    static constexpr float HYPERTHERMIA_THRESHOLD = 39.0f;
    static constexpr float FATAL_TEMP_MIN = 28.0f;
    static constexpr float FATAL_TEMP_MAX = 42.0f;
};

class BodyTempSystem {
public:
    BodyTempSystem() = default;

    // Per-tick update
    void tick(float dt, float airTemp, float windSpeed, float humidity,
              float exertion, bool isInShelter, bool isInWater);

    // Apply environmental effects
    void setWetness(float w) { state_.wetness = std::min(1.0f, w); }
    void addWetness(float delta) {
        state_.wetness = std::min(1.0f, state_.wetness + delta);
    }
    void setClothingCLO(float clo) { state_.clothingCLO = std::max(0.1f, clo); }

    // Queries
    const BodyTempState& state() const { return state_; }
    bool isHypothermic() const { return state_.coreTemp < BodyTempState::HYPOTHERMIA_THRESHOLD; }
    bool isHyperthermic() const { return state_.coreTemp > BodyTempState::HYPERTHERMIA_THRESHOLD; }
    bool isDeadlyCold() const { return state_.coreTemp < BodyTempState::FATAL_TEMP_MIN; }
    bool isDeadlyHot() const { return state_.coreTemp > BodyTempState::FATAL_TEMP_MAX; }

    // Reset
    void reset() { state_ = BodyTempState{}; }

private:
    BodyTempState state_;

    // Heat transfer: sensible + evaporative + metabolic
    float calcSensibleHeat(float airTemp, float windSpeed, float clothingCLO) const;
    float calcEvaporativeHeat(float airTemp, float windSpeed, float humidity, float wetness) const;
    float calcMetabolicHeat(float exertion) const;
};

} // namespace survival
