#include "Application.h"
#include "Time.h"
#include "EventBus.h"

#include <iostream>

namespace core {

Application::Application() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }

    window_ = SDL_CreateWindow("Zombie Engine", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return;
    }

    renderer_ = std::make_unique<render::Renderer>(window_);
    EventBus::get().subscribe<WindowCloseEvent, &Application::on_window_close>(this);
}

Application::~Application() {
    EventBus::get().unsubscribe<WindowCloseEvent, &Application::on_window_close>(this);
    renderer_.reset(); // Ensure renderer cleans up before window

    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

void Application::on_window_close(const WindowCloseEvent& event) {
    running_ = false;
}

void Application::run() {
    if (!window_) return;

    running_ = true;
    Time::init();

    SDL_Event event;

    while (running_) {
        Time::update();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                EventBus::get().publish(WindowCloseEvent{});
            }
        }

        while (Time::consume_fixed_step()) {
            // Physics / logic update
        }

        renderer_->draw();
    }

    renderer_->wait_idle();
}

} // namespace core

int main(int argc, char* argv[]) {
    core::Application app;
    app.run();
    return 0;
}
