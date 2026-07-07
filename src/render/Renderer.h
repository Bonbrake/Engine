#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "VulkanContext.h"

namespace render {

class Renderer {
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    void draw();
    void wait_idle();

private:
    std::unique_ptr<VulkanContext> ctx_;

    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;

    VkSemaphore image_available_semaphore_;
    VkSemaphore render_finished_semaphore_;
    VkFence in_flight_fence_;
};

} // namespace render