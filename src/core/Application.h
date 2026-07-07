#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "render/Renderer.h"

namespace core {

class Application {
public:
    Application();
    ~Application();

    void run();

    void on_window_close(const struct WindowCloseEvent& event);

private:
    SDL_Window* window_{nullptr};
    bool running_{false};

    std::unique_ptr<render::Renderer> renderer_;
};

} // namespace core