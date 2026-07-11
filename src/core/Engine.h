#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "../debug/ImGuiOverlay.h"

namespace render { class VulkanContext; }
namespace ecs { class ECSContext; }

namespace core {

class Engine {
public:
    Engine();
    ~Engine();

    void run();
    bool verifyHeadlessInit() const;

private:
    void mainLoop();

    bool running_ = false;
    SDL_Window* window_ = nullptr;
    std::unique_ptr<render::VulkanContext> vulkanContext_;
    std::unique_ptr<ecs::ECSContext> ecsContext_;
    debug::ImGuiOverlay imguiOverlay_;
};

} // namespace core
