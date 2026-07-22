#include "ze/combat/DamageTypes.h"
#include <algorithm>
#include <cmath>

namespace combat {

DamageSystem::DamageSystem() {
    // Initialize default response table
    for (auto& row : table_) {
        for (auto& cell : row) {
            cell = DamageResponse{1.0f, 0.0f, 0.0f, 0.0f, false, false, false};
        }
    }

    // Flesh: soft, bleeds easily
    setResponse(DamageType::Slash, SurfaceMaterial::Flesh, {1.2f, 0.0f, 0.8f, 0.1f, false, true, true});
    setResponse(DamageType::Pierce, SurfaceMaterial::Flesh, {1.5f, 0.2f, 0.3f, 0.05f, false, true, false});
    setResponse(DamageType::Blunt, SurfaceMaterial::Flesh, {0.8f, 0.0f, 0.1f, 0.3f, false, false, true});
    setResponse(DamageType::Bullet, SurfaceMaterial::Flesh, {2.0f, 0.3f, 0.6f, 0.4f, false, true, true});

    // Kevlar: resistant to slash/pierce/bullet
    setResponse(DamageType::Slash, SurfaceMaterial::Kevlar, {0.3f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Pierce, SurfaceMaterial::Kevlar, {0.5f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Bullet, SurfaceMaterial::Kevlar, {0.6f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Blunt, SurfaceMaterial::Kevlar, {0.9f, 0.0f, 0.1f, 0.15f, false, false, true});

    // Metal: hard, resistant to most
    setResponse(DamageType::Blunt, SurfaceMaterial::Metal, {0.5f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Slash, SurfaceMaterial::Metal, {0.2f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Bullet, SurfaceMaterial::Metal, {0.7f, 0.5f, 0.0f, 0.0f, false, false, false});

    // Concrete: very hard
    setResponse(DamageType::Bullet, SurfaceMaterial::Concrete, {0.4f, 0.0f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Explosion, SurfaceMaterial::Concrete, {0.5f, 0.0f, 0.0f, 0.0f, false, false, false});

    // Wood: medium
    setResponse(DamageType::Bullet, SurfaceMaterial::Wood, {0.8f, 0.2f, 0.0f, 0.0f, false, false, false});
    setResponse(DamageType::Slash, SurfaceMaterial::Wood, {0.4f, 0.0f, 0.0f, 0.0f, false, false, false});
}

void DamageSystem::setResponse(DamageType dt, SurfaceMaterial m, const DamageResponse& r) {
    int di = static_cast<int>(dt);
    int mi = static_cast<int>(m);
    if (di < kDamageTypes && mi < kSurfaceMaterials) {
        table_[di][mi] = r;
    }
}

const DamageResponse& DamageSystem::getResponse(DamageType dt, SurfaceMaterial m) const {
    int di = static_cast<int>(dt);
    int mi = static_cast<int>(m);
    if (di < kDamageTypes && mi < kSurfaceMaterials) {
        return table_[di][mi];
    }
    // Return default
    static DamageResponse defaultResponse{1.0f, 0.0f, 0.0f, 0.0f, false, false, false};
    return defaultResponse;
}

DamageResult DamageSystem::resolve(float baseDamage, DamageType type,
                                    const HitLocation& location, float attackerForce) {
    const auto& response = getResponse(type, location.surface);

    DamageResult result;
    float effectiveDamage = baseDamage * response.damageMultiplier * attackerForce;

    // Armor penetration
    float armorEffective = location.armorRating * (1.0f - response.armorPenetration);
    if (response.ignoresArmor) armorEffective = 0.0f;

    result.blockedByArmor = std::min(effectiveDamage, armorEffective);
    result.finalDamage = effectiveDamage - result.blockedByArmor;
    if (result.finalDamage < 0.0f) result.finalDamage = 0.0f;

    // Bleed
    if (response.causesBleed && result.finalDamage > 5.0f) {
        result.bleedAmount = result.finalDamage * response.bleedChance;
    }

    // Fracture check
    if (response.fractureChance > 0.0f) {
        float roll = std::fmod(result.finalDamage * 0.1f, 1.0f);
        result.fractured = roll < response.fractureChance * attackerForce;
    }

    result.staggered = response.causesStagger && result.finalDamage > 20.0f;
    result.lethal = result.finalDamage >= 100.0f;

    return result;
}

float DamageSystem::resolveParry(float attackerForce, float defenderSkill,
                                  float parryWindow, float inputTiming) const {
    // Parry success: focused on timing within parry window (M2.7-EXT-04)
    float parryScore = std::abs(inputTiming) / parryWindow;
    if (parryScore > 1.0f) return 0.0f; // missed window

    float reflectMultiplier = (1.0f - parryScore) * defenderSkill;
    return attackerForce * reflectMultiplier;
}

float DamageSystem::calcPenetrationDepth(float mass, float velocity,
                                          float materialHardness, float crossSection) const {
    // Bullet penetration depth (M2.7-EXT-05)
    if (crossSection < 0.0001f) return 0.0f;
    float kineticEnergy = 0.5f * mass * velocity * velocity;
    return kineticEnergy / (materialHardness * crossSection);
}

} // namespace combat
