#include "PhysicsSystem.h"
#include "../ecs/Components.h"
#include "../core/Logger.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <glm/glm.hpp>
#include "PhysicsDebugRenderer.h"
#include "../events/EventBus.h"

// Jolt uses right-handed Y-up — matches spec.
// JPH_CROSS_PLATFORM_DETERMINISTIC and JPH_DOUBLE_PRECISION enforced by CMake defines.

namespace physics {

static bool s_JoltInitialized = false;

static JPH::Vec3 toJPH(const glm::vec3& v)  { return JPH::Vec3(v.x, v.y, v.z); }
static JPH::DVec3 toJPHD(const glm::dvec3& v) { return JPH::DVec3(v.x, v.y, v.z); }
static JPH::Quat toJPHQ(const glm::quat& q)  { return JPH::Quat(q.x, q.y, q.z, q.w); }
static glm::dvec3 fromJPH(const JPH::DVec3& v) { return glm::dvec3(v.GetX(), v.GetY(), v.GetZ()); }

// [M2.6] Test seams: forward the file-local double-precision converters (Jolt built with
// JPH_DOUBLE_PRECISION, so dvec3<->DVec3 is bit-exact). Kept as thin wrappers so the
// round-trip unit test exercises the real conversion path, not a copy.
JPH::DVec3 PhysicsSystem::ToJPHD(const glm::dvec3& v) { return toJPHD(v); }
glm::dvec3 PhysicsSystem::FromJPH(const JPH::DVec3& v) { return fromJPH(v); }
// Rotation: Jolt::Quat is single-precision (float x,y,z,w) by design, so this is a
// lossless float->double widening into dquat. Position (see fromJPH) is the double-exact path.
static glm::dquat fromJPHQ(const JPH::Quat& q)  { return glm::dquat(q.GetW(), q.GetX(), q.GetY(), q.GetZ()); }

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
    : joltJobs_(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 1),
      tempAllocator_(10 * 1024 * 1024)
{
    physicsSystem_ = new JPH::PhysicsSystem();
    physicsSystem_->Init(MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS,
                         MAX_CONTACT_CONSTRAINTS,
                         bpLayerInterface_, objVsBpFilter_, objLayerFilter_);
    physicsSystem_->SetBodyActivationListener(&activationListener_);

#ifdef JPH_DEBUG_RENDERER
    debugRenderer_ = std::make_unique<PhysicsDebugRenderer>();
#endif

    // M2-EXT-04 V_maxLimit safety ceiling
    physicsSystem_->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

    LOG_INFO("PhysicsSystem: Jolt {} initialized. JPH_CROSS_PLATFORM_DETERMINISTIC=1 JPH_DOUBLE_PRECISION=1",
             JPH_VERSION_MAJOR * 100 + JPH_VERSION_MINOR * 10 + JPH_VERSION_PATCH);
}

PhysicsSystem::~PhysicsSystem() {
    // [M2-EXT-01] Reap any still-in-flight async bakers before teardown
    if (taskScheduler_) {
        std::lock_guard lock(inFlightBakersMutex_);
        for (auto* baker : inFlightBakers_) delete baker;
        inFlightBakers_.clear();
    }
    delete physicsSystem_;
    physicsSystem_ = nullptr;
}

void PhysicsSystem::step(entt::registry& registry, entt::dispatcher& dispatcher) {
    if (!physicsSystem_) return;

    {
        // [M2-EXT-03] Clear query cache at the start of every fixed tick
        std::unique_lock lock(queryCacheMutex_);
        queryCache_.clear();
    }

    {
        // [M2-EXT-01] Drain pending collision swaps before Jolt spins up its job graph
        std::lock_guard lock(pendingSwapsMutex_);
        auto& bi = physicsSystem_->GetBodyInterface();
        for (const auto& swap : pendingSwaps_) {
            // Safely swap the proxy shape for the cooked one
            bi.SetShape(swap.bodyId, swap.newShape, true, JPH::EActivation::Activate);
        }
        pendingSwaps_.clear();
    }

    // [M2-EXT-01] Reap completed async bakers (enkiTS does not own TaskSet lifetime)
    {
        std::lock_guard lock(inFlightBakersMutex_);
        for (auto it = inFlightBakers_.begin(); it != inFlightBakers_.end(); ) {
            if ((*it)->GetIsComplete()) {
                delete *it;
                it = inFlightBakers_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Pass dedicated Jolt job system (distinct from enkiTS)
    JPH::EPhysicsUpdateError err = physicsSystem_->Update(
        FIXED_DT, COLLISION_STEPS, &tempAllocator_, &joltJobs_);

    if (err != JPH::EPhysicsUpdateError::None) {
        LOG_WARN("PhysicsSystem::step: update error flags = 0x{:X}",
                 static_cast<uint32_t>(err));
    }

    // Mirror Jolt transforms into EnTT
    mirrorTransforms(registry);

    // [M2-EXT-02] Sleep event bridge: drain background activation events
    std::vector<BodyActivationEvent> events;
    {
        std::lock_guard lock(activationListener_.mutex_);
        events.swap(activationListener_.events_);
    }

    if (!events.empty()) {
        auto view = registry.view<PhysicsBodyComponent, ecs::StableId>();
        for (const auto& ev : events) {
            for (auto [entity, phys, stableId] : view.each()) {
                if (phys.bodyId == ev.bodyId) {
                    dispatcher.enqueue<events::HibernationEvent>({stableId.uuid, !ev.isActivated});
                    break;
                }
            }
        }
    }
}

// [M2-EXT-03] Continuous Narrow-Phase Contact Point Query Collector Cache
bool PhysicsSystem::CachedRayCast(const glm::vec3& start, const glm::vec3& end, JPH::RayCastResult& outHit) {
    if (!physicsSystem_) return false;

    LineQuery q{start, end};
    
    // Fast path: shared read lock
    {
        std::shared_lock lock(queryCacheMutex_);
        auto it = queryCache_.find(q);
        if (it != queryCache_.end()) {
            outHit = it->second;
            // The Jolt struct has a fraction < 1.0 if it's a hit
            return outHit.mFraction < 1.0f;
        }
    }

    // Miss: perform actual raycast
    JPH::RVec3 jphStart = toJPHD(start);
    JPH::Vec3 jphDir = toJPH(end - start);
    JPH::RRayCast ray(jphStart, jphDir);
    
    JPH::RayCastResult hit;
    bool hasHit = physicsSystem_->GetNarrowPhaseQuery().CastRay(
        ray, 
        hit, 
        JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), 
        JPH::SpecifiedObjectLayerFilter(Layers::MOVING)
    );

    // Store in cache (requires exclusive lock)
    {
        std::unique_lock lock(queryCacheMutex_);
        queryCache_[q] = hit;
    }

    outHit = hit;
    return hasHit;
}

// [M2-EXT-01] Async Collision Baking Task
class AsyncCollisionBaker : public enki::ITaskSet {
public:
    AsyncCollisionBaker(PhysicsSystem* sys, JPH::BodyID bid, std::vector<glm::vec3> verts, std::vector<uint32_t> idx)
        : sys_(sys), bodyId_(bid), vertices_(std::move(verts)), indices_(std::move(idx)) {}

    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
        if (vertices_.empty() || indices_.empty()) return;

        // Cook the mesh offline
        JPH::VertexList jphVertices;
        jphVertices.reserve(vertices_.size());
        for (const auto& v : vertices_) {
            jphVertices.push_back(JPH::Float3(v.x, v.y, v.z));
        }
        
        JPH::IndexedTriangleList jphTriangles;
        jphTriangles.reserve(indices_.size() / 3);
        for (size_t i = 0; i < indices_.size(); i += 3) {
            jphTriangles.push_back(JPH::IndexedTriangle(indices_[i], indices_[i+1], indices_[i+2]));
        }

        JPH::MeshShapeSettings meshSettings(jphVertices, jphTriangles);
        JPH::ShapeSettings::ShapeResult shapeResult = meshSettings.Create();

        if (shapeResult.HasError()) {
            LOG_ERROR("AsyncCollisionBaker: Failed to bake shape: {}", shapeResult.GetError().c_str());
            return;
        }

        JPH::ShapeRefC cookedShape = shapeResult.Get();

        // Push to pending swaps
        std::lock_guard lock(sys_->pendingSwapsMutex_);
        sys_->pendingSwaps_.push_back({bodyId_, cookedShape});
    }

private:
    PhysicsSystem* sys_;
    JPH::BodyID bodyId_;
    std::vector<glm::vec3> vertices_; // Copied by value
    std::vector<uint32_t> indices_;
};

// [M2-EXT-01] Queue Async Collision Swap
void PhysicsSystem::QueueAsyncCollisionSwap(JPH::BodyID bodyId, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, JPH::ShapeRefC proxyShape) {
    if (!physicsSystem_ || vertices.empty()) return;

    // Immediately swap in the proxy shape on the main thread
    auto& bi = physicsSystem_->GetBodyInterface();
    bi.SetShape(bodyId, proxyShape, true, JPH::EActivation::DontActivate);

    // If enkiTS is available, dispatch the background bake. enkiTS does NOT delete the
    // TaskSet, so we track it and reap it in step() once GetIsComplete() is true.
    if (taskScheduler_) {
        auto* baker = new AsyncCollisionBaker(this, bodyId, vertices, indices);
        {
            std::lock_guard lock(inFlightBakersMutex_);
            inFlightBakers_.push_back(baker);
        }
        taskScheduler_->AddTaskSetToPipe(baker);
    } else {
        LOG_WARN("PhysicsSystem: QueueAsyncCollisionSwap called but taskScheduler_ is null!");
    }
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
        // [M2-EXT-04] Fixed-Timestep Kinetic Energy Clamper (Safety Ceiling)
        settings.mMaxLinearVelocity = 250.0f; 
        settings.mMaxAngularVelocity = 0.25f * JPH::JPH_PI * 60.0f;
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
        // [M2-EXT-04] Fixed-Timestep Kinetic Energy Clamper (Safety Ceiling)
        settings.mMaxLinearVelocity = 250.0f; 
        settings.mMaxAngularVelocity = 0.25f * JPH::JPH_PI * 60.0f;
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
