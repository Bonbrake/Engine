#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>

#include "ecs/Components.h"
#include "physics/PhysicsSystem.h"
#include "debug/MetaRegistry.h"

// [M2.6] Phase 1 precision regression tests.
// Proof that B2 (silent double->float truncation on world-space Transform) is gone:
//   - position round-trips dvec3 -> Jolt DVec3 -> dvec3 bit-exact at km scale (double epsilon)
//   - MetaRegistry load path reads JSON as double into dvec3/dquat (no float narrowing)
// Rotation through Jolt::Quat is single-precision by design, so its test uses float epsilon.
// No Transform save-serializer exists in the repo yet; load-path is the only persistence edge.

TEST_CASE("Transform position round-trips dvec3<->Jolt DVec3 bit-exact at km scale", "[M2.6][precision]") {
    const glm::dvec3 world(123456.7890123456, -98765.4321098765, 54321.1234567890);

    JPH::DVec3 jolt = physics::PhysicsSystem::ToJPHD(world);
    glm::dvec3 back = physics::PhysicsSystem::FromJPH(jolt);

    //_double_ epsilon: if this passed under the old vec3 Transform, it would be a lie.
    REQUIRE(back.x == Catch::Approx(world.x).epsilon(1e-12));
    REQUIRE(back.y == Catch::Approx(world.y).epsilon(1e-12));
    REQUIRE(back.z == Catch::Approx(world.z).epsilon(1e-12));

    // Component-wise exactness (DVec3 stores doubles, so this is a true bit round-trip)
    REQUIRE(back.x == world.x);
    REQUIRE(back.y == world.y);
    REQUIRE(back.z == world.z);
}

TEST_CASE("MetaRegistry Transform load preserves double-precision position", "[M2.6][precision][persistence]") {
    nlohmann::json j = nlohmann::json::object({
        {"position", {123456.7890123456, -98765.4321098765, 54321.1234567890}},
        {"rotation", {0.7071067811865476, 0.0, 0.7071067811865476, 0.0}},
        {"scale",    {1.0, 1.0, 1.0}},
    });

    entt::registry reg;
    entt::entity e = reg.create();
    bool ok = debug::MetaRegistry::EmplaceComponent("Transform", reg, e, j);
    REQUIRE(ok);
    REQUIRE(reg.all_of<ecs::Transform>(e));

    const auto& t = reg.get<ecs::Transform>(e);
    // Position must be double and carry the full-precision literal, not a float-rounded value.
    REQUIRE(t.position.x == Catch::Approx(123456.7890123456).epsilon(1e-12));
    REQUIRE(t.position.y == Catch::Approx(-98765.4321098765).epsilon(1e-12));
    REQUIRE(t.position.z == Catch::Approx(54321.1234567890).epsilon(1e-12));
    // If the load path narrowed to float, these would each differ by ~1e-4 or more.
    REQUIRE(t.position.x != static_cast<float>(123456.7890123456));
    REQUIRE(t.rotation.w == Catch::Approx(0.7071067811865476).epsilon(1e-6)); // Jolt Quat is float by design
    REQUIRE(t.scale.x == Catch::Approx(1.0).epsilon(1e-12));
}
