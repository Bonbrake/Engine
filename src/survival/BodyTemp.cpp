#include "ze/survival/BodyTemp.h"
#include <cmath>
#include <algorithm>

namespace survival {

void BodyTempSystem::tick(float dt, float airTemp, float windSpeed, float humidity,
                           float exertion, bool isInShelter, bool isInWater) {
    // Shelter reduces wind and effective heat loss
    float effWind = isInShelter ? windSpeed * 0.1f : windSpeed;

    // Water contact dramatically increases heat loss
    float waterMultiplier = isInWater ? 25.0f : 1.0f;

    // Heat transfer components
    float sensible = calcSensibleHeat(airTemp, effWind, state_.clothingCLO);
    float evaporative = calcEvaporativeHeat(airTemp, effWind, humidity, state_.wetness);
    float metabolic = calcMetabolicHeat(exertion);

    // Net heat flux (W/m²)
    // Positive = body losing heat, negative = body gaining heat
    float netFlux = (sensible * waterMultiplier + evaporative) - metabolic;
    state_.heatLoss = netFlux;

    // Convert heat flux to temperature change
    // Human body: ~3.5 kJ/(kg·K), ~70 kg, ~1.8 m² surface area
    // 1 W = 1 J/s
    float tempDelta = netFlux * dt / (70.0f * 3500.0f / 1.8f);

    // Apply to core and skin temperature
    state_.coreTemp += tempDelta * 0.3f; // core is buffered
    state_.skinTemp += tempDelta * 0.7f; // skin responds faster

    // Wetness decays when not in rain
    if (!isInWater && state_.wetness > 0.0f) {
        state_.wetness -= dt * 0.1f; // dries over ~10 seconds
        if (state_.wetness < 0.0f) state_.wetness = 0.0f;
    }

    // Shiver response when cold
    if (state_.coreTemp < 36.0f) {
        state_.shiverIntensity = (36.0f - state_.coreTemp) / 3.0f;
        state_.shiverIntensity = std::min(1.0f, state_.shiverIntensity);
        // Shivering generates metabolic heat (up to 5x resting)
        metabolic += state_.shiverIntensity * 4.0f * 100.0f;
    } else {
        state_.shiverIntensity = 0.0f;
    }

    // Sweat response when hot
    if (state_.coreTemp > 37.5f) {
        state_.sweatRate = (state_.coreTemp - 37.5f) / 2.0f;
        state_.sweatRate = std::min(1.0f, state_.sweatRate);
        state_.wetness = std::min(1.0f, state_.wetness + state_.sweatRate * dt * 0.5f);
    } else {
        state_.sweatRate = 0.0f;
    }

    // Clamp to fatal limits
    state_.coreTemp = std::max(BodyTempState::FATAL_TEMP_MIN,
        std::min(BodyTempState::FATAL_TEMP_MAX, state_.coreTemp));
    state_.skinTemp = std::max(0.0f, std::min(45.0f, state_.skinTemp));
}

float BodyTempSystem::calcSensibleHeat(float airTemp, float windSpeed,
                                        float clothingCLO) const {
    // Sensible heat loss: convection + radiation
    // Q = h * (T_skin - T_air) where h depends on wind + clothing
    // Clothing insulation: 1 CLO = 0.155 m²·K/W
    float insulation = clothingCLO * 0.155f; // m²·K/W
    float convectiveCoeff = 8.3f * std::sqrt(windSpeed + 0.1f); // W/(m²·K)
    float totalResistance = insulation + (1.0f / convectiveCoeff);

    if (totalResistance < 0.01f) totalResistance = 0.01f;
    float deltaT = state_.skinTemp - airTemp;
    return deltaT / totalResistance;
}

float BodyTempSystem::calcEvaporativeHeat(float airTemp, float windSpeed,
                                           float humidity, float wetness) const {
    // Evaporative heat loss: Q = h_e * (P_skin - P_air) * wetness
    // Saturation vapor pressure at skin temp (~35°C): ~5.6 kPa
    float pSkin = 5.6f; // kPa at skin temperature
    float pAir = humidity * 3.17f; // kPa at 25°C saturated
    float evapCoeff = 16.5f * std::sqrt(windSpeed + 0.1f); // W/(m²·kPa)
    return evapCoeff * (pSkin - pAir) * wetness;
}

float BodyTempSystem::calcMetabolicHeat(float exertion) const {
    // Metabolic heat production
    // Resting: ~100 W, walking: ~200 W, running: ~600 W, sprinting: ~1000 W
    float baseMetabolic = 100.0f;
    float exerciseHeat = exertion * 1500.0f;
    return baseMetabolic + exerciseHeat;
}

} // namespace survival
