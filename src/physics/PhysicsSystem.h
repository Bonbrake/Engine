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
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/EPhysicsUpdateError.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <enkiTS/TaskScheduler.h>

namespace ecs { struct Transform; struct Health; }

namespace physics {

#ifdef JPH_DEBUG_RENDERER
class PhysicsDebugRenderer;
#endif

// [M2-EXT-03] Spatial query deduplication cache
struct LineQuery {
    glm::vec3 start;
    glm::vec3 end;
    bool operator==(const LineQuery& other) const {
        return start == other.start && end == other.end;
    }
};

struct LineQueryHash {
    size_t operator()(const LineQuery& q) const {
        std::hash<float> h;
        // Robust 6-float CombineHash
        size_t res = 17;
        res = res * 31 + h(q.start.x);
        res = res * 31 + h(q.start.y);
        res = res * 31 + h(q.start.z);
        res = res * 31 + h(q.end.x);
        res = res * 31 + h(q.end.y);
        res = res * 31 + h(q.end.z);
        return res;
    }
};

// [M2-EXT-01] Pending shape swap definition
struct PendingShapeSwap {
    JPH::BodyID bodyId;
    JPH::ShapeRefC newShape;
};

// [M2-EXT-05] Mechanical Muscle Exhaustion Joint-Friction Damping Adder
// Inline math helper for M2.9 CharacterVirtual consumption
inline glm::vec3 ApplyPhysiologicalVelocityDamping(const glm::vec3& targetVelocity, float liveWBalance, float maxWBalance) {
    float scalar = (maxWBalance > 0.001f) ? (liveWBalance / maxWBalance) : 0.35f;
    float cExhaust = std::clamp(scalar, 0.35f, 1.0f);
    return targetVelocity * cExhaust;
}

// [M2-EXT-07] Kinematic Virtual Sweep Tunneling Safeguard
// Stopgap for kinematic character motion which doesn't use standard dynamic CCD
inline JPH::AABox VirtualSweepTunnelingSafeguard(const JPH::AABox& currentBounds, const JPH::Vec3& velocity, float dt) {
    JPH::Vec3 displacement = velocity * dt;
    JPH::AABox projected = currentBounds;
    projected.Translate(displacement);
    JPH::AABox swept = currentBounds;
    swept.Encapsulate(projected);
    return swept;
}

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

// Extracted sleep event data from Jolt background threads
struct BodyActivationEvent {
    JPH::BodyID bodyId;
    bool isActivated;
};

class PhysicsSystem {
public:
    static void initializeGlobal();

    static constexpr float FIXED_DT = 1.0f / 60.0f;
    static constexpr int   COLLISION_STEPS = 1;

    PhysicsSystem();
    ~PhysicsSystem();

    void setTaskScheduler(enki::TaskScheduler* ts) { taskScheduler_ = ts; }

    // Non-copyable
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;

    // Step physics by one fixed tick. Call once per game tick.
    // dt accumulation is handled by the caller (Engine).
    void step(entt::registry& registry, entt::dispatcher& dispatcher);

    // [M2-EXT-03] Cached narrow-phase raycast. Thread-safe for concurrent AI readers.
    bool CachedRayCast(const glm::vec3& start, const glm::vec3& end, JPH::RayCastResult& outHit);

    // [M2-EXT-01] Enqueue an async collision bake and swap task
    void QueueAsyncCollisionSwap(JPH::BodyID bodyId, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, JPH::ShapeRefC proxyShape);

    // Exposed for EXT-01 background tasks
    JPH::PhysicsSystem* GetRawSystem() { return physicsSystem_; }

    // Mirror Jolt transforms back to EnTT Transform components
    void mirrorTransforms(entt::registry& registry);

    // [M2.6] Exposed for unit tests: double-precision position conversion
    // (dvec3 <-> Jolt DVec3). Jolt::Quat is single-precision by design, so the
    // analogous fromJPHQ is tested for float-precision losslessness, not double.
    // Defined in PhysicsSystem.cpp (the file-local toJPHD/fromJPH statics live there).
    static JPH::DVec3 ToJPHD(const glm::dvec3& v);
    static glm::dvec3 FromJPH(const JPH::DVec3& v);

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
    
    // [M2-EXT-03] Cache state
    std::unordered_map<LineQuery, JPH::RayCastResult, LineQueryHash> queryCache_;
    std::shared_mutex queryCacheMutex_;

    // [M2-EXT-01] Pending swaps
    std::vector<PendingShapeSwap> pendingSwaps_;
    std::mutex pendingSwapsMutex_;
    friend class AsyncCollisionBaker;

    // [M2-EXT-01] In-flight async bakers. enkiTS does NOT own the TaskSet lifetime,
    // so we track them and delete once GetIsComplete() is true (reaped in step()).
    std::vector<enki::ITaskSet*> inFlightBakers_;
    std::mutex inFlightBakersMutex_;

    // [M2-EXT-02] Activation events
    class ActivationListener : public JPH::BodyActivationListener {
    public:
        void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override {
            std::lock_guard lock(mutex_);
            events_.push_back({inBodyID, true});
        }

        void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override {
            std::lock_guard lock(mutex_);
            events_.push_back({inBodyID, false});
        }

        std::mutex mutex_;
        std::vector<BodyActivationEvent> events_;
    };
    
    ActivationListener activationListener_;
    
    // Reference to the global task scheduler for [M2-EXT-01]
    enki::TaskScheduler* taskScheduler_ = nullptr;
};

} // namespace physics
