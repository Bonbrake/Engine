#pragma once
#include <cstdint>

namespace combat {

// Parry system (M2.7-EXT-04)
enum class GuardDirection : uint8_t {
    High, Mid, Low, None
};

struct ParryResult {
    bool success = false;
    float reflectedDamage = 0.0f;
    float stunFrames = 0.0f;
    GuardDirection bestDirection = GuardDirection::None;
};

class ParrySystem {
public:
    ParrySystem() = default;

    // Evaluate parry input: returns result
    ParryResult evaluate(float attackerForce, float defenderStaggerResist,
                         GuardDirection attackDirection, GuardDirection guardDirection,
                         float inputTiming, float parryWindow = 0.1f);

    // Configuration
    void setParryWindow(float seconds) { parryWindow_ = seconds; }
    float parryWindow() const { return parryWindow_; }
    void setParryEfficiency(float e) { parryEfficiency_ = e; }

private:
    float parryWindow_ = 0.1f;      // 100ms success window
    float parryEfficiency_ = 0.8f;  // fraction of attacker force reflected

    bool directionMatches(GuardDirection attack, GuardDirection guard) const;
};

} // namespace combat
