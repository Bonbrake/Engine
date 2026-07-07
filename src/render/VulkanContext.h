#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vector>

namespace render {

class VulkanContext {
public:
    VulkanContext(SDL_Window* window);
    ~VulkanContext();

    // Delete copy/move constructors
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    vkb::Instance instance;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::Swapchain swapchain;

    VkQueue graphics_queue;
    uint32_t graphics_queue_index;
    std::vector<VkImage> swapchain_images;

private:
    SDL_Window* window_{nullptr};
};

} // namespace render