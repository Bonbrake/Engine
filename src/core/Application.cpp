#include "Application.h"
#include "Time.h"
#include "../ecs/Components.h"
#include "../ecs/Systems.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    // Setup ECS
    auto entity = registry_.create();
    registry_.emplace<ecs::Position>(entity, glm::vec3(0.0f, 0.0f, -5.0f));
    registry_.emplace<ecs::Velocity>(entity, glm::vec3(0.5f, 0.2f, 0.0f)); // Slowly move
    registry_.emplace<ecs::Renderable>(entity);
}

Application::~Application() {
    renderer_.reset();
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Application::run() {
    if (!window_) return;

    running_ = true;
    Time::init();

    SDL_Event event;

    // Static Camera M1
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    proj[1][1] *= -1; // Vulkan Y is flipped

    while (running_) {
        Time::update();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running_ = false;
            }
        }

        while (Time::consume_fixed_step()) {
            ecs::Systems::MovementSystem(registry_, static_cast<float>(TIME_STEP));
        }

        renderer_->BeginFrame();

        auto render_view = registry_.view<ecs::Position, ecs::Renderable>();
        for (auto entity : render_view) {
            const auto& pos = render_view.get<ecs::Position>(entity);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos.value);
            glm::mat4 mvp = proj * view * model;

            renderer_->DrawEntity(mvp);
        }

        renderer_->EndFrame();
    }

    renderer_->wait_idle();
}

} // namespace core

int main(int argc, char* argv[]) {
    core::Application app;
    app.run();
    return 0;
}