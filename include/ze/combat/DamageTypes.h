#pragma once
#include <cstdint>
#include <array>

namespace combat {

// Damage categories (M2.7-EXT-02)
enum class DamageType : uint8_t {
    Blunt, Slash, Pierce, Bullet, Explosion,
    Fire, Acid, Cold, Electric, Radiation, Poison, Bleed
};

// Surface material response (M2.7-EXT-02)
// Maps DamageType × SurfaceMaterial → final damage multipliers
enum class SurfaceMaterial : uint8_t {
    Flesh, Bone, Leather, Kevlar, Metal, Concrete,
    Wood, Glass, Plastic, Cloth, Water, COUNT
};

struct DamageResponse {
    float damageMultiplier = 1.0f;
    float armorPenetration = 0.0f;  // 0..1 fraction of armor ignored
    float bleedChance = 0.0f;
    float fractureChance = 0.0f;
    bool ignoresArmor = false;
    bool causesBleed = false;
    bool causesStagger = false;
};

// Damage resolution result
struct DamageResult {
    float finalDamage = 0.0f;
    float blockedByArmor = 0.0f;
    float bleedAmount = 0.0f;
    bool fractured = false;
    bool staggered = false;
    bool lethal = false;
};

// Hit location (M2.7-EXT-01 limb hit component)
struct HitLocation {
    uint32_t boneIndex;
    uint32_t limbMask;     // bitset: head(0), torso(1), L-arm(2), R-arm(3), L-leg(4), R-leg(5)
    float armorRating;     // local armor thickness at hit point
    SurfaceMaterial surface;
};

class DamageSystem {
public:
    DamageSystem();

    // Register damage/material response
    void setResponse(DamageType dt, SurfaceMaterial m, const DamageResponse& r);
    const DamageResponse& getResponse(DamageType dt, SurfaceMaterial m) const;

    // Resolve a single hit
    DamageResult resolve(float baseDamage, DamageType type, const HitLocation& location,
                         float attackerForce = 1.0f);

    // Parry resolution (M2.7-EXT-04)
    float resolveParry(float attackerForce, float defenderSkill,
                       float parryWindow, float inputTiming) const;

    // Convenience
    float calcPenetrationDepth(float mass, float velocity, float materialHardness,
                                float crossSection) const;

private:
    static constexpr int kDamageTypes = static_cast<int>(DamageType::Bleed) + 1;
    static constexpr int kSurfaceMaterials = static_cast<int>(SurfaceMaterial::COUNT);
    std::array<std::array<DamageResponse, kSurfaceMaterials>, kDamageTypes> table_;
};

} // namespace combat
