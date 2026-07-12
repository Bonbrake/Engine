#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "../debug/ImGuiOverlay.h"

namespace render { class VulkanContext; }
namespace ecs { class ECSContext; }
namespace physics { class PhysicsSystem; }
namespace events { class EventBus; }

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


    bool running_ = false;
    float physicsAccumulator_ = 0.0f; // [M2] fixed-timestep accumulator

    SDL_Window* window_ = nullptr;
    std::unique_ptr<render::VulkanContext>  vulkanContext_;
    std::unique_ptr<ecs::ECSContext>        ecsContext_;
    std::unique_ptr<physics::PhysicsSystem> physicsSystem_; // [M2] Jolt
    std::unique_ptr<events::EventBus>       eventBus_;      // [M2] entt::dispatcher
    debug::ImGuiOverlay imguiOverlay_;
};

} // namespace core
