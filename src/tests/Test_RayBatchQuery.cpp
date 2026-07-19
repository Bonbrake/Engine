#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

#include "physics/PhysicsSystem.h"
#include "core/Logger.h"

// [M1-EXT-40] RayBatchQuery primitive - packs N independent world-space rays into one
// narrow-phase pass (Jolt has no native N-ray batch; it only does per-ray or multi-hit-per-ray).
TEST_CASE("M1-EXT-40: RayBatchQuery casts N independent rays and reports per-ray hits", "[M1-EXT-40][physics][raycast]") {
    core::Logger::init();  // required: PhysicsSystem() ctor emits LOG_INFO; null logger would SIGSEGV
    physics::PhysicsSystem::initializeGlobal();

    physics::PhysicsSystem ps;
    // Static 1x1x1 box centered at origin - reliable hit target.
    JPH::BodyID boxId = ps.createBox({0.0, 0.0, 0.0}, {0.5f, 0.5f, 0.5f}, /*isStatic=*/true, 0.0f);
    REQUIRE(boxId != JPH::BodyID());

    // Advance one tick so the body is fully in the broad/narrow phase (mirrors engine usage).
    entt::registry reg;
    entt::dispatcher disp;
    ps.step(reg, disp);

    // Three rays fired down -Z from in front of the box, each grazing the origin.
    physics::RayBatch batch;
    batch.rays = {
        JPH::RRayCast(JPH::RVec3(-0.1, -0.1, 5.0), JPH::Vec3( 0.1,  0.1, -5.0)),
        JPH::RRayCast(JPH::RVec3( 0.0,  0.0, 5.0), JPH::Vec3( 0.0,  0.0, -5.0)),
        JPH::RRayCast(JPH::RVec3( 0.1,  0.1, 5.0), JPH::Vec3(-0.1, -0.1, -5.0)),
    };
    batch.hits.resize(batch.rays.size());

    bool anyHit = ps.CastRayBatch(batch);
    REQUIRE(anyHit);

    for (size_t i = 0; i < batch.hits.size(); ++i) {
        // Near face of the box sits at z = 0.5 along a 5-unit ray -> fraction ~0.9.
        REQUIRE(batch.hits[i].mFraction > 0.0f);   // hit somewhere in front of the near face
        REQUIRE(batch.hits[i].mFraction < 1.0f);   // and before the far end of the ray (Jolt miss sentinel is 1+eps)
        REQUIRE(batch.hits[i].mBodyID == boxId);
    }
}

// Under-sized hits buffer must NOT silently drop rays - the loop hard-stops at the buffer end.
TEST_CASE("M1-EXT-40: RayBatchQuery guards against under-sized hits buffer", "[M1-EXT-40][physics][raycast][contract]") {
    core::Logger::init();  // idempotent guard in Logger::init() keeps the second call safe
    physics::PhysicsSystem::initializeGlobal();
    physics::PhysicsSystem ps;

    physics::RayBatch batch;
    batch.rays.resize(4);  // 4 rays...
    batch.hits.resize(2);  // ...but only 2 hit slots; loop must process exactly 2, never read OOB.
    bool anyHit = ps.CastRayBatch(batch);
    // No bodies in this system, so anyHit is false; the point is a clean return without OOB access.
    REQUIRE_FALSE(anyHit);
}
