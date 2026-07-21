#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>
#include "ze/ecs/GenerationalTable.h"

namespace ecs {

struct Transform {
    glm::dvec3 position{0.0, 0.0, 0.0};   // double-precision authoritative world position
    glm::dquat rotation{1.0, 0.0, 0.0, 0.0};
    glm::dvec3 scale{1.0, 1.0, 1.0};
};

struct Health {
    float current{100.0f};
    float max{100.0f};
};

enum class LimbId : uint8_t { Head, Torso, ArmLeft, ArmRight, LegLeft, LegRight, Count };

struct DamageEvent {
    float amount = 0.0f;                  // raw, pre-mitigation
    entt::entity target = entt::null;     // The entity receiving the damage
    entt::entity source = entt::null;     // dealer; entt::null for environmental/scripted damage
    entt::entity instigator = entt::null; // credit-attribution owner when source is indirect
    LimbId hit_location = LimbId::Torso;
    float penetration_depth = 0.0f;       // cm penetrated before this hit resolved
    enum class DamageTag : uint8_t { Melee, Ballistic, Fire, Fall, Environmental, Infection } tag = DamageTag::Melee;
};

struct ResolvedDamageEvent {
    DamageEvent source_event;
    float final_amount = 0.0f; // after parry/armor/perk mitigation has run
    bool was_parried = false;
    bool was_mitigated = false;
};

struct StableId {
    uint64_t uuid = 0;
};

// [M1:EXIT-1] ECS->render bridge: ties an entity to a loaded mesh so
// TriangleRenderer can traverse `view<Transform, MeshComponent>` and draw it.
// meshHandle defaults to the null Handle ({0xFFFFFFFF,0}); a valid Insert
// yields generation >= 1, so the default is unmistakably "no mesh".
struct MeshComponent {
    ecs::Handle meshHandle{0xFFFFFFFF, 0};
};

struct PerkPoints {
    uint32_t spent = 0;
    uint32_t available = 0;
};

struct BarrelHeat {
    float kelvin = 293.15f;
    float fouling = 0.0f;       // carbon fouling accumulation (0-1)
    float grimeFactor = 0.0f;   // maintenance neglect scalar
};

struct BulletComponent {
    glm::vec3 velocity{0.0f};
    float mass = 0.0f;
    float dragCoefficient = 0.0f; // C_d0, baseline (non-tumbling) value
    enum Flags : uint32_t { None = 0, Tumbling = 1 << 0, Ricocheted = 1 << 1, Spalled = 1 << 2 };
    uint32_t flags = Flags::None; // M2.9-EXT-04 sets Tumbling on a steep glance-angle impact
};

struct SurfaceFrictionSample {
    float materialMuMultiplier      = 1.0f; // surface material (asphalt/dirt/gravel/etc.)
    float weatherMuMultiplier       = 1.0f; // aquaplaning (M9-EXT-03) * mud shear (M9-EXT-04)
    float corpseOverrideMultiplier  = 1.0f; // corpse-pile flattening (M7-EXT-01), 1.0 if none present
    float bloodMuMultiplier         = 1.0f; // blood-slick viscosity (M9-EXT-01)
    float tireThermalMuMultiplier   = 1.0f; // tire-thermal breakdown (M9-EXT-02), 1.0 for foot traction

    float ResolveFriction(float baseMu) const {
        float mu = baseMu;
        mu *= materialMuMultiplier;
        mu *= weatherMuMultiplier;
        mu *= corpseOverrideMultiplier;
        mu *= bloodMuMultiplier;
        mu *= tireThermalMuMultiplier;
        return mu;
    }
};

} // namespace ecs
