#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <entt/entt.hpp>
#include "../debug/ImGuiOverlay.h"

namespace render { class VulkanContext; }
namespace ecs { class ECSContext; }
namespace physics { class PhysicsSystem; }
namespace events { class EventBus; }
namespace debug { class FlyCamera; }

namespace core {

class Engine {
public:
    Engine();
    ~Engine();

    void run();
    bool verifyHeadlessInit() const;

private:
    void mainLoop();
    void physicsTick(); // fixed-timestep physics step
    void dumpVendorCheckpoints();

#if ENGINE_DEV_TOOLS
    // [M2-#4] Dev-only scaffolding: spawn a Jolt body in the windowed --dev path so
    // drawBodies() has non-empty input for the debug-draw overlay eyes-on check.
    // Gated by devMode && !headless; never touches the headless CI/audit path.
    void spawnDevTestBody_();
#endif


    bool running_ = false;
    float physicsAccumulator_ = 0.0f; // [M2] fixed-timestep accumulator

    SDL_Window* window_ = nullptr;
    std::unique_ptr<render::VulkanContext>  vulkanContext_;
    std::unique_ptr<ecs::ECSContext>        ecsContext_;
    std::unique_ptr<physics::PhysicsSystem> physicsSystem_; // [M2] Jolt
    std::unique_ptr<events::EventBus>       eventBus_;      // [M2] entt::dispatcher
    debug::ImGuiOverlay imguiOverlay_;

#if ENGINE_DEV_TOOLS
    // [M2-#4] Dev-test hook state (devMode && !headless only)
    bool devTestBodySpawned_ = false;
    bool devTestDamageFired_ = false;
    entt::entity devTestEntity_ = entt::null;

    // [M2.6 Phase 2] Debug fly-camera owner (dev-gated; only allocated when --fly-camera).
    std::unique_ptr<debug::FlyCamera> flyCamera_;
#endif
};

} // namespace core
