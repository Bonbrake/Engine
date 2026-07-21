#include "MetaRegistry.h"
#include <entt/meta/factory.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../ecs/Components.h"
#include "../ecs/Destructible.h"
#include "ze/core/Logger.h"

namespace debug {

void MetaRegistry::Initialize() {
#ifdef ENGINE_DEV_TOOLS
    using namespace entt::literals;
    
    entt::meta_factory<ecs::Transform>{}
        .type("Transform"_hs)
        .data<&ecs::Transform::position>("position"_hs)
        .data<&ecs::Transform::rotation>("rotation"_hs)
        .data<&ecs::Transform::scale>("scale"_hs);

    entt::meta_factory<ecs::Health>{}
        .type("Health"_hs)
        .data<&ecs::Health::current>("current"_hs)
        .data<&ecs::Health::max>("max"_hs);

    entt::meta_factory<ecs::DestructibleComponent>{}
        .type("DestructibleComponent"_hs)
        .data<&ecs::DestructibleComponent::isDestroyed>("isDestroyed"_hs)
        .data<&ecs::DestructibleComponent::intactMeshHandle>("intactMeshHandle"_hs)
        .data<&ecs::DestructibleComponent::destroyedMeshHandle>("destroyedMeshHandle"_hs);

    entt::meta_factory<ecs::DamageEvent>{}
        .type("DamageEvent"_hs)
        .data<&ecs::DamageEvent::amount>("amount"_hs)
        .data<&ecs::DamageEvent::target>("target"_hs)
        .data<&ecs::DamageEvent::source>("source"_hs)
        .data<&ecs::DamageEvent::instigator>("instigator"_hs)
        .data<&ecs::DamageEvent::hit_location>("hit_location"_hs)
        .data<&ecs::DamageEvent::penetration_depth>("penetration_depth"_hs)
        .data<&ecs::DamageEvent::tag>("tag"_hs);

    entt::meta_factory<ecs::ResolvedDamageEvent>{}
        .type("ResolvedDamageEvent"_hs)
        .data<&ecs::ResolvedDamageEvent::source_event>("source_event"_hs)
        .data<&ecs::ResolvedDamageEvent::final_amount>("final_amount"_hs)
        .data<&ecs::ResolvedDamageEvent::was_parried>("was_parried"_hs)
        .data<&ecs::ResolvedDamageEvent::was_mitigated>("was_mitigated"_hs);

    entt::meta_factory<ecs::StableId>{}
        .type("StableId"_hs)
        .data<&ecs::StableId::uuid>("uuid"_hs);

    entt::meta_factory<ecs::PerkPoints>{}
        .type("PerkPoints"_hs)
        .data<&ecs::PerkPoints::spent>("spent"_hs)
        .data<&ecs::PerkPoints::available>("available"_hs);

    entt::meta_factory<ecs::BarrelHeat>{}
        .type("BarrelHeat"_hs)
        .data<&ecs::BarrelHeat::kelvin>("kelvin"_hs)
        .data<&ecs::BarrelHeat::fouling>("fouling"_hs)
        .data<&ecs::BarrelHeat::grimeFactor>("grimeFactor"_hs);

    entt::meta_factory<ecs::BulletComponent>{}
        .type("BulletComponent"_hs)
        .data<&ecs::BulletComponent::velocity>("velocity"_hs)
        .data<&ecs::BulletComponent::mass>("mass"_hs)
        .data<&ecs::BulletComponent::dragCoefficient>("dragCoefficient"_hs)
        .data<&ecs::BulletComponent::flags>("flags"_hs);

    entt::meta_factory<ecs::SurfaceFrictionSample>{}
        .type("SurfaceFrictionSample"_hs)
        .data<&ecs::SurfaceFrictionSample::materialMuMultiplier>("materialMuMultiplier"_hs)
        .data<&ecs::SurfaceFrictionSample::weatherMuMultiplier>("weatherMuMultiplier"_hs)
        .data<&ecs::SurfaceFrictionSample::corpseOverrideMultiplier>("corpseOverrideMultiplier"_hs)
        .data<&ecs::SurfaceFrictionSample::bloodMuMultiplier>("bloodMuMultiplier"_hs)
        .data<&ecs::SurfaceFrictionSample::tireThermalMuMultiplier>("tireThermalMuMultiplier"_hs);
        
    LOG_INFO("MetaRegistry initialized with 9 reflected components");
#endif
}

static ecs::LimbId ParseLimbId(const std::string& str) {
    if (str == "Head") return ecs::LimbId::Head;
    if (str == "Torso") return ecs::LimbId::Torso;
    if (str == "ArmLeft") return ecs::LimbId::ArmLeft;
    if (str == "ArmRight") return ecs::LimbId::ArmRight;
    if (str == "LegLeft") return ecs::LimbId::LegLeft;
    if (str == "LegRight") return ecs::LimbId::LegRight;
    return ecs::LimbId::Torso;
}

static ecs::DamageEvent::DamageTag ParseDamageTag(const std::string& str) {
    if (str == "Melee") return ecs::DamageEvent::DamageTag::Melee;
    if (str == "Ballistic") return ecs::DamageEvent::DamageTag::Ballistic;
    if (str == "Fire") return ecs::DamageEvent::DamageTag::Fire;
    if (str == "Fall") return ecs::DamageEvent::DamageTag::Fall;
    if (str == "Environmental") return ecs::DamageEvent::DamageTag::Environmental;
    if (str == "Infection") return ecs::DamageEvent::DamageTag::Infection;
    return ecs::DamageEvent::DamageTag::Melee;
}

bool MetaRegistry::EmplaceComponent(const std::string& name, entt::registry& registry, entt::entity entity, const nlohmann::json& json) {
    if (name == "Transform") {
        ecs::Transform t;
        // [M2.6] Transform is double-precision (dvec3/dquat/dvec3). Read JSON as double so
        // km-scale positions survive save/load bit-exact; Jolt::Quat is float by design, so
        // rotation is read as double then widened (lossless) into dquat.
        if (json.contains("position") && json["position"].is_array() && json["position"].size() >= 3) {
            auto pos = json["position"];
            t.position = glm::dvec3(pos[0].get<double>(), pos[1].get<double>(), pos[2].get<double>());
        }
        if (json.contains("rotation") && json["rotation"].is_array() && json["rotation"].size() >= 4) {
            auto rot = json["rotation"];
            t.rotation = glm::dquat(rot[0].get<double>(), rot[1].get<double>(), rot[2].get<double>(), rot[3].get<double>());
        }
        if (json.contains("scale") && json["scale"].is_array() && json["scale"].size() >= 3) {
            auto scl = json["scale"];
            t.scale = glm::dvec3(scl[0].get<double>(), scl[1].get<double>(), scl[2].get<double>());
        }
        registry.emplace<ecs::Transform>(entity, t);
        return true;
    }
    else if (name == "Health") {
        ecs::Health h;
        if (json.contains("max")) {
            h.max = json["max"].get<float>();
        }
        if (json.contains("current")) {
            h.current = json["current"].get<float>();
        } else if (json.contains("max")) {
            // Health missing current: fallback to max
            h.current = h.max;
        }
        registry.emplace<ecs::Health>(entity, h);
        return true;
    }
    else if (name == "DestructibleComponent") {
        ecs::DestructibleComponent d;
        if (json.contains("isDestroyed")) d.isDestroyed = json["isDestroyed"].get<bool>();
        // destroyedMeshHandle is now a generation-safe ecs::Handle{index, generation}.
        if (json.contains("destroyedMeshHandle_index"))
            d.destroyedMeshHandle.index = json["destroyedMeshHandle_index"].get<uint32_t>();
        if (json.contains("destroyedMeshHandle_generation"))
            d.destroyedMeshHandle.generation = json["destroyedMeshHandle_generation"].get<uint32_t>();
        if (json.contains("intactMeshHandle_index"))
            d.intactMeshHandle.index = json["intactMeshHandle_index"].get<uint32_t>();
        if (json.contains("intactMeshHandle_generation"))
            d.intactMeshHandle.generation = json["intactMeshHandle_generation"].get<uint32_t>();
        registry.emplace<ecs::DestructibleComponent>(entity, d);
        return true;
    }
    else if (name == "DamageEvent") {
        ecs::DamageEvent d;
        if (json.contains("amount")) d.amount = json["amount"].get<float>();
        if (json.contains("target")) d.target = static_cast<entt::entity>(json["target"].get<uint32_t>());
        if (json.contains("hit_location")) d.hit_location = ParseLimbId(json["hit_location"].get<std::string>());
        if (json.contains("penetration_depth")) d.penetration_depth = json["penetration_depth"].get<float>();
        if (json.contains("tag")) d.tag = ParseDamageTag(json["tag"].get<std::string>());
        // Note: source and instigator cannot easily be set from raw JSON entity load unless they are UUIDs or entities,
        // but we default them to entt::null.
        registry.emplace<ecs::DamageEvent>(entity, d);
        return true;
    }
    else if (name == "ResolvedDamageEvent") {
        ecs::ResolvedDamageEvent r;
        if (json.contains("final_amount")) r.final_amount = json["final_amount"].get<float>();
        if (json.contains("was_parried")) r.was_parried = json["was_parried"].get<bool>();
        if (json.contains("was_mitigated")) r.was_mitigated = json["was_mitigated"].get<bool>();
        if (json.contains("source_event")) {
            auto se = json["source_event"];
            if (se.contains("amount")) r.source_event.amount = se["amount"].get<float>();
            if (se.contains("hit_location")) r.source_event.hit_location = ParseLimbId(se["hit_location"].get<std::string>());
            if (se.contains("penetration_depth")) r.source_event.penetration_depth = se["penetration_depth"].get<float>();
            if (se.contains("tag")) r.source_event.tag = ParseDamageTag(se["tag"].get<std::string>());
        }
        registry.emplace<ecs::ResolvedDamageEvent>(entity, r);
        return true;
    }
    else if (name == "StableId") {
        ecs::StableId s;
        if (json.contains("uuid")) {
            s.uuid = json["uuid"].get<uint64_t>();
        }
        registry.emplace<ecs::StableId>(entity, s);
        return true;
    }
    else if (name == "PerkPoints") {
        ecs::PerkPoints p;
        if (json.contains("spent")) p.spent = json["spent"].get<uint32_t>();
        if (json.contains("available")) p.available = json["available"].get<uint32_t>();
        registry.emplace<ecs::PerkPoints>(entity, p);
        return true;
    }
    else if (name == "BarrelHeat") {
        ecs::BarrelHeat b;
        if (json.contains("kelvin")) b.kelvin = json["kelvin"].get<float>();
        if (json.contains("fouling")) b.fouling = json["fouling"].get<float>();
        if (json.contains("grimeFactor")) b.grimeFactor = json["grimeFactor"].get<float>();
        registry.emplace<ecs::BarrelHeat>(entity, b);
        return true;
    }
    else if (name == "BulletComponent") {
        ecs::BulletComponent b;
        if (json.contains("velocity") && json["velocity"].is_array() && json["velocity"].size() >= 3) {
            auto vel = json["velocity"];
            b.velocity = glm::vec3(vel[0].get<float>(), vel[1].get<float>(), vel[2].get<float>());
        }
        if (json.contains("mass")) b.mass = json["mass"].get<float>();
        if (json.contains("dragCoefficient")) b.dragCoefficient = json["dragCoefficient"].get<float>();
        if (json.contains("flags")) b.flags = json["flags"].get<uint32_t>();
        registry.emplace<ecs::BulletComponent>(entity, b);
        return true;
    }
    else if (name == "SurfaceFrictionSample") {
        ecs::SurfaceFrictionSample s;
        if (json.contains("materialMuMultiplier")) s.materialMuMultiplier = json["materialMuMultiplier"].get<float>();
        if (json.contains("weatherMuMultiplier")) s.weatherMuMultiplier = json["weatherMuMultiplier"].get<float>();
        if (json.contains("corpseOverrideMultiplier")) s.corpseOverrideMultiplier = json["corpseOverrideMultiplier"].get<float>();
        if (json.contains("bloodMuMultiplier")) s.bloodMuMultiplier = json["bloodMuMultiplier"].get<float>();
        if (json.contains("tireThermalMuMultiplier")) s.tireThermalMuMultiplier = json["tireThermalMuMultiplier"].get<float>();
        registry.emplace<ecs::SurfaceFrictionSample>(entity, s);
        return true;
    }
    
    else if (name == "MeshComponent") {
        ecs::MeshComponent m;
        if (json.contains("meshHandle")) {
            auto h = json["meshHandle"];
            if (h.is_object() && h.contains("index") && h.contains("generation")) {
                m.meshHandle.index = h["index"].get<uint32_t>();
                m.meshHandle.generation = h["generation"].get<uint32_t>();
            }
        }
        registry.emplace<ecs::MeshComponent>(entity, m);
        return true;
    }
    
    LOG_WARN("MetaRegistry::EmplaceComponent: Unregistered component '{}'", name);
    return false;
}

} // namespace debug