#include "ze/combat/ParrySystem.h"
#include <cmath>
#include <algorithm>

namespace combat {

bool ParrySystem::directionMatches(GuardDirection attack, GuardDirection guard) const {
    // Directional matching: if guard direction matches attack direction,
    // parry is more effective. Adjacent directions still work at reduced efficiency.
    if (attack == guard) return true;
    // Adjacent check: High↔Mid, Mid↔Low (circular)
    if (attack == GuardDirection::High && guard == GuardDirection::Mid) return true;
    if (attack == GuardDirection::Mid && (guard == GuardDirection::High || guard == GuardDirection::Low)) return true;
    if (attack == GuardDirection::Low && guard == GuardDirection::Mid) return true;
    return false;
}

ParryResult ParrySystem::evaluate(float attackerForce, float defenderStaggerResist,
                                   GuardDirection attackDirection, GuardDirection guardDirection,
                                   float inputTiming, float parryWindow) {
    ParryResult result;
    float actualWindow = parryWindow > 0.0f ? parryWindow : parryWindow_;

    // Check if input falls within parry window (M2.7-EXT-04)
    // inputTiming: 0 = perfect, positive = late, negative = early
    float normalizedTiming = std::abs(inputTiming) / actualWindow;

    if (normalizedTiming > 1.0f) {
        // Missed the window: parry failed, defender is stunned
        result.success = false;
        result.stunFrames = attackerForce / std::max(defenderStaggerResist, 0.1f);
        return result;
    }

    // Within window: check direction match
    bool dirMatch = directionMatches(attackDirection, guardDirection);
    if (!dirMatch) {
        // Wrong direction: partial parry
        result.success = true;
        result.reflectedDamage = attackerForce * parryEfficiency_ * 0.3f;
        result.stunFrames = attackerForce / std::max(defenderStaggerResist, 0.1f) * 0.5f;
        result.bestDirection = attackDirection;
        return result;
    }

    // Successful parry
    result.success = true;
    result.reflectedDamage = attackerForce * parryEfficiency_ * (1.0f - normalizedTiming * 0.5f);
    result.stunFrames = 0.0f; // no stun on successful parry
    result.bestDirection = attackDirection;

    return result;
}

} // namespace combat
