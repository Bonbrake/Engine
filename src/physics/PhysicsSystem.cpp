#include "PhysicsSystem.h"
#include "../ecs/Components.h"
#include "../core/Logger.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <glm/glm.hpp>
#include "PhysicsDebugRenderer.h"

// Jolt uses right-handed Y-up — matches spec.
// JPH_CROSS_PLATFORM_DETERMINISTIC and JPH_DOUBLE_PRECISION enforced by CMake defines.

namespace physics {

static bool s_JoltInitialized = false;

static JPH::Vec3 toJPH(const glm::vec3& v)  { return JPH::Vec3(v.x, v.y, v.z); }
static JPH::DVec3 toJPHD(const glm::dvec3& v) { return JPH::DVec3(v.x, v.y, v.z); }
static JPH::Quat toJPHQ(const glm::quat& q)  { return JPH::Quat(q.x, q.y, q.z, q.w); }
static glm::dvec3 fromJPH(const JPH::DVec3& v) { return glm::dvec3(v.GetX(), v.GetY(), v.GetZ()); }
static glm::quat  fromJPHQ(const JPH::Quat& q)  { return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ()); }

void PhysicsSystem::initializeGlobal() {
    if (!s_JoltInitialized) {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        s_JoltInitialized = true;
    }
}

PhysicsSystem::PhysicsSystem()
    // Jolt JobSystemThreadPool: dedicated, NOT enkiTS (spec: two job systems only)
    : joltJobs_(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                static_cast<int>(std::thread::hardware_concurrency()) - 1),
      tempAllocator_(10 * 1024 * 1024)
{

    physicsSystem_ = new JPH::PhysicsSystem();
    physicsSystem_->Init(MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS,
                         MAX_CONTACT_CONSTRAINTS,
                         bpLayerInterface_, objVsBpFilter_, objLayerFilter_);

#ifdef JPH_DEBUG_RENDERER
    debugRenderer_ = std::make_unique<PhysicsDebugRenderer>();
#endif

    // Right-handed Y-up gravity (spec: Y-up alignment)
    physicsSystem_->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

    LOG_INFO("PhysicsSystem: Jolt {} initialized. JPH_CROSS_PLATFORM_DETERMINISTIC=1 JPH_DOUBLE_PRECISION=1",
             JPH_VERSION_MAJOR * 100 + JPH_VERSION_MINOR * 10 + JPH_VERSION_PATCH);
}

PhysicsSystem::~PhysicsSystem() {
    delete physicsSystem_;
    physicsSystem_ = nullptr;
    // Factory/types are process-lifetime singletons; leave registered.
}

void PhysicsSystem::step(entt::registry& registry, entt::dispatcher& dispatcher) {
    // Fixed-timestep: caller is responsible for accumulator; we step once per call.
    // EMotionQuality::LinearCast set per-body in createBox/createSphere for CCD.
    JPH::EPhysicsUpdateError err = physicsSystem_->Update(
        FIXED_DT, COLLISION_STEPS, &tempAllocator_, &joltJobs_);

    if (err != JPH::EPhysicsUpdateError::None) {
        LOG_WARN("PhysicsSystem::step: update error flags = 0x{:X}",
                 static_cast<uint32_t>(err));
    }

    // Mirror Jolt transforms into EnTT
    mirrorTransforms(registry);

    // [M2-EXT-02] Sleep event bridge: walk active-body set and detect transitions
    // Real implementation requires a Jolt ContactListener; for M2 we poll body sleep state.
    // Dispatcher bridge kept here for downstream subscribers (M7-EXT-01 etc.)
    (void)dispatcher; // wired at M7 when first subscriber appears
}

void PhysicsSystem::mirrorTransforms(entt::registry& registry) {
    auto& bi = physicsSystem_->GetBodyInterface();
    auto view = registry.view<PhysicsBodyComponent, ecs::Transform>();
    for (auto [entity, phys, transform] : view.each()) {
        if (!bi.IsActive(phys.bodyId) && !bi.IsAdded(phys.bodyId)) continue;
        JPH::DVec3 pos = bi.GetCenterOfMassPosition(phys.bodyId);
        JPH::Quat  rot = bi.GetRotation(phys.bodyId);
        // [M2-EXT-06] Jolt single-precision -> dvec3 authoritative position
        transform.position = fromJPH(pos);
        transform.rotation = fromJPHQ(rot);
    }
}

JPH::BodyID PhysicsSystem::createBox(const glm::dvec3& position, const glm::vec3& halfExtents,
                                      bool isStatic, float mass) {
    auto& bi = physicsSystem_->GetBodyInterface();
    JPH::BoxShapeSettings shapeSettings(toJPH(halfExtents));
    shapeSettings.mConvexRadius = 0.02f;
    auto shapeResult = shapeSettings.Create();
    if (shapeResult.HasError()) {
        LOG_ERROR("PhysicsSystem::createBox: shape error: {}", shapeResult.GetError().c_str());
        return JPH::BodyID();
    }
    JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer      = isStatic ? Layers::NON_MOVING : Layers::MOVING;
    JPH::BodyCreationSettings settings(shapeResult.Get(), toJPHD(position),
                                        JPH::Quat::sIdentity(), motionType, layer);
    if (!isStatic) {
        settings.mMassPropertiesOverride.mMass = mass;
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMotionQuality = JPH::EMotionQuality::LinearCast; // CCD per spec
    }
    JPH::Body* body = bi.CreateBody(settings);
    if (!body) {
        LOG_ERROR("PhysicsSystem::createBox: body limit reached");
        return JPH::BodyID();
    }
    bi.AddBody(body->GetID(), JPH::EActivation::Activate);
    return body->GetID();
}

JPH::BodyID PhysicsSystem::createSphere(const glm::dvec3& position, float radius,
                                         bool isStatic, float mass) {
    auto& bi = physicsSystem_->GetBodyInterface();
    JPH::SphereShapeSettings shapeSettings(radius);
    auto shapeResult = shapeSettings.Create();
    if (shapeResult.HasError()) {
        LOG_ERROR("PhysicsSystem::createSphere: shape error: {}", shapeResult.GetError().c_str());
        return JPH::BodyID();
    }
    JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer      = isStatic ? Layers::NON_MOVING : Layers::MOVING;
    JPH::BodyCreationSettings settings(shapeResult.Get(), toJPHD(position),
                                        JPH::Quat::sIdentity(), motionType, layer);
    if (!isStatic) {
        settings.mMassPropertiesOverride.mMass = mass;
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMotionQuality = JPH::EMotionQuality::LinearCast;
    }
    JPH::Body* body = bi.CreateBody(settings);
    if (!body) {
        LOG_ERROR("PhysicsSystem::createSphere: body limit reached");
        return JPH::BodyID();
    }
    bi.AddBody(body->GetID(), JPH::EActivation::Activate);
    return body->GetID();
}

void PhysicsSystem::destroyBody(JPH::BodyID id) {
    if (id.IsInvalid()) return;
    auto& bi = physicsSystem_->GetBodyInterface();
    bi.RemoveBody(id);
    bi.DestroyBody(id);
}

#ifdef JPH_DEBUG_RENDERER
void PhysicsSystem::drawBodies() {
    if (debugRenderer_) {
        debugRenderer_->clearLines();
        JPH::BodyManager::DrawSettings drawSettings;
        drawSettings.mDrawBoundingBox = false;
        drawSettings.mDrawShape = true;
        physicsSystem_->DrawBodies(drawSettings, debugRenderer_.get());
    }
}
#endif

} // namespace physics
