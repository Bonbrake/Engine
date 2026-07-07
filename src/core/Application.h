#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <entt/entt.hpp>
#include "../render/Renderer.h"

namespace core {

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    SDL_Window* window_{nullptr};
    bool running_{false};

    std::unique_ptr<render::Renderer> renderer_;
    entt::registry registry_;
};

} // namespace core