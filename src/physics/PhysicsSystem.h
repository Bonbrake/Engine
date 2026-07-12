#pragma once
// [M2] Physics system — Jolt 5.5.0 integration
// JPH_CROSS_PLATFORM_DETERMINISTIC and JPH_DOUBLE_PRECISION are defined in CMakeLists.
// Fixed timestep: 1/60 s. EMotionQuality::LinearCast for CCD.
// JobSystemThreadPool is DEDICATED to Jolt only — enkiTS handles everything else.
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/EPhysicsUpdateError.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>
#include <memory>

namespace ecs { struct Transform; struct Health; }

namespace physics {

#ifdef JPH_DEBUG_RENDERER
class PhysicsDebugRenderer;
#endif

// Object layers — kept minimal for M2; expanded in M2.6+
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING      = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS  = 2;
}

// Broadphase layers
namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING{0};
    static constexpr JPH::BroadPhaseLayer MOVING{1};
    static constexpr uint32_t NUM_LAYERS = 2;
}

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
        switch (a) {
            case Layers::NON_MOVING: return b == Layers::MOVING;
            case Layers::MOVING:     return true;
            default: return false;
        }
    }
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BroadPhaseLayerInterfaceImpl() {
        objToBroadPhase_[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        objToBroadPhase_[Layers::MOVING]     = BroadPhaseLayers::MOVING;
    }
    uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
        return objToBroadPhase_[layer];
    }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
        return layer == BroadPhaseLayers::NON_MOVING ? "NON_MOVING" : "MOVING";
    }
#endif
private:
    JPH::BroadPhaseLayer objToBroadPhase_[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadPhase) const override {
        switch (layer) {
            case Layers::NON_MOVING: return broadPhase == BroadPhaseLayers::MOVING;
            case Layers::MOVING:     return true;
            default: return false;
        }
    }
};

// Maps JPH::BodyID -> EnTT entity for transform mirroring and sleep bridging
struct PhysicsBodyComponent {
    JPH::BodyID bodyId;
};

class PhysicsSystem {
public:
    static void initializeGlobal();

    static constexpr float FIXED_DT = 1.0f / 60.0f;
    static constexpr int   COLLISION_STEPS = 1;

    PhysicsSystem();
    ~PhysicsSystem();

    // Non-copyable
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;

    // Step physics by one fixed tick. Call once per game tick.
    // dt accumulation is handled by the caller (Engine).
    void step(entt::registry& registry, entt::dispatcher& dispatcher);

    // Mirror Jolt transforms back to EnTT Transform components
    void mirrorTransforms(entt::registry& registry);

#ifdef JPH_DEBUG_RENDERER
    PhysicsDebugRenderer* getDebugRenderer() const { return debugRenderer_.get(); }
    void drawBodies();
#endif

    // Body creation helpers
    JPH::BodyID createBox(const glm::dvec3& position, const glm::vec3& halfExtents,
                          bool isStatic, float mass = 1.0f);
    JPH::BodyID createSphere(const glm::dvec3& position, float radius,
                             bool isStatic, float mass = 1.0f);

    void destroyBody(JPH::BodyID id);

    JPH::PhysicsSystem& raw() { return *physicsSystem_; }
    JPH::BodyInterface& bodyInterface() { return physicsSystem_->GetBodyInterface(); }

private:
    void registerSleepCallback(entt::dispatcher& dispatcher);

    static constexpr uint32_t MAX_BODIES           = 65536;
    static constexpr uint32_t NUM_BODY_MUTEXES      = 0; // 0 = default (1 per 32 bodies)
    static constexpr uint32_t MAX_BODY_PAIRS        = 65536;
    static constexpr uint32_t MAX_CONTACT_CONSTRAINTS = 10240;

    // Jolt allocates internally; 10MB temp arena per M0 budget
    JPH::TempAllocatorImpl          tempAllocator_{10 * 1024 * 1024};
    // Dedicated Jolt job system — NOT enkiTS (spec rule: two job systems, not four)
    JPH::JobSystemThreadPool        joltJobs_;

    BroadPhaseLayerInterfaceImpl         bpLayerInterface_;
    ObjectVsBroadPhaseLayerFilterImpl    objVsBpFilter_;
    ObjectLayerPairFilterImpl            objLayerFilter_;

    JPH::PhysicsSystem*  physicsSystem_ = nullptr;

#ifdef JPH_DEBUG_RENDERER
    std::unique_ptr<PhysicsDebugRenderer> debugRenderer_;
#endif
};

} // namespace physics
