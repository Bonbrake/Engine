#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>

#include "ze/ecs/Components.h"
#include "ze/render/TriangleRenderer.h"

// [M1:EXIT-1] GPU-free proof of the camera-relative MVP math that the ECS->render
// bridge relies on. This is the math that M2.6 Phase 2 could only probe with a
// hardcoded cube (no scene path); now it runs against a real Transform-driven
// entity in a unit test, so the precision claim is verified without a GPU.
//
// BuildEntityMVP is a free function declared friend in TriangleRenderer.h and
// living in the `render` namespace. It performs a single camera-relative cast:
//   renderPos = (vec3)(entityPos - camPos)
// so world magnitude (km-scale) never enters single-precision math.

namespace render {
    glm::mat4 BuildEntityMVP(const ecs::Transform& t, const glm::dvec3& camPos,
                             const glm::mat4& view, const glm::mat4& proj);
}

TEST_CASE("BuildEntityMVP is camera-relative at 50km (M2.6 precision)", "[M1][M2.6][precision]") {
    // Reference camera at origin looking down -Z (matches the renderer's default fallback).
    const glm::dvec3 camPos{0.0, 0.0, 4.0};
    const glm::mat4 view = glm::lookAt(glm::vec3(camPos), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 200000.0f);

    // A real entity at ~50km world magnitude (the old M2.6 far-cube probe position).
    ecs::Transform far;
    far.position = glm::dvec3(50000.0, 0.0, 0.0);
    far.rotation = glm::dquat(1.0, 0.0, 0.0, 0.0);
    far.scale    = glm::dvec3(1.0);

    const glm::mat4 mvp = render::BuildEntityMVP(far, camPos, view, proj);

    // The entity is ~50km away along +X. After camera-relative subtraction the GPU
    // only ever sees renderPos = (vec3)(50000,0,-4), so the clip-space position must
    // be large-but-finite (in front of camera), NOT NaN/Inf and NOT collapsed to origin.
    const glm::vec4 clip = mvp * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(std::isfinite(clip.x));
    REQUIRE(std::isfinite(clip.y));
    REQUIRE(std::isfinite(clip.w));
    REQUIRE(clip.w > 0.0f); // in front of the camera

    // Camera-relative property (the actual M2.6 rebase invariant): translating the
    // WHOLE world by a large constant (simulating worldOriginOffset rebasing) must
    // leave the on-screen clip position unchanged, because only the relative offset
    // matters. If the absolute dvec3 ever entered single-precision math, this would
    // drift. Use K = 1e6 so a naive path would lose precision.
    const double K = 1'000'000.0;
    const glm::dvec3 camRebased{0.0 + K, 0.0, 4.0};
    ecs::Transform farRebased = far;
    farRebased.position += glm::dvec3(K, 0.0, 0.0);
    const glm::mat4 mvpRebased = render::BuildEntityMVP(farRebased, camRebased, view, proj);
    const glm::vec4 clipRebased = mvpRebased * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(clip.x == Catch::Approx(clipRebased.x).epsilon(1e-3));
    REQUIRE(clip.y == Catch::Approx(clipRebased.y).epsilon(1e-3));
}

TEST_CASE("view<Transform, MeshComponent> traversal drives the bridge", "[M1][ecs]") {
    entt::registry reg;

    // Entity with both Transform + MeshComponent (should be drawn).
    entt::entity a = reg.create();
    reg.emplace<ecs::Transform>(a, glm::dvec3(1.0, 2.0, 3.0));
    reg.emplace<ecs::MeshComponent>(a, ecs::MeshComponent{ecs::Handle{0, 1}});

    // Entity with Transform only (must NOT be in the bridge view).
    entt::entity b = reg.create();
    reg.emplace<ecs::Transform>(b, glm::dvec3(0.0));

    // Entity with MeshComponent only (must NOT be in the bridge view).
    entt::entity c = reg.create();
    reg.emplace<ecs::MeshComponent>(c, ecs::MeshComponent{ecs::Handle{1, 1}});

    auto view = reg.view<ecs::Transform, ecs::MeshComponent>();
    uint32_t count = 0;
    for (entt::entity e : view) { (void)e; ++count; }
    REQUIRE(count == 1);
    REQUIRE(view.contains(a));
    REQUIRE(!view.contains(b));
    REQUIRE(!view.contains(c));
}

TEST_CASE("MeshComponent default handle is the null Handle", "[M1][ecs]") {
    ecs::MeshComponent m;
    REQUIRE(m.meshHandle.index == 0xFFFFFFFF);
    REQUIRE(m.meshHandle.generation == 0);
}