#include "VulkanContext.h"
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <stdexcept>

#ifndef ENABLE_VULKAN_VALIDATION_LAYERS
#define ENABLE_VULKAN_VALIDATION_LAYERS 0
#endif

namespace render {

VulkanContext::VulkanContext(SDL_Window* window) : window_(window) {
    uint32_t ext_count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&ext_count);
    if (!extensions) {
        throw std::runtime_error("Failed to get SDL Vulkan extensions");
    }

    vkb::InstanceBuilder builder;
    builder.set_app_name("Zombie Engine")
           .request_validation_layers(ENABLE_VULKAN_VALIDATION_LAYERS)
           .use_default_debug_messenger();

    for (uint32_t i = 0; i < ext_count; ++i) {
        builder.enable_extension(extensions[i]);
    }

    auto inst_ret = builder.build();
    if (!inst_ret) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
    instance = inst_ret.value();

    if (!SDL_Vulkan_CreateSurface(window_, instance.instance, nullptr, &surface)) {
        throw std::runtime_error("Failed to create Vulkan surface");
    }

    vkb::PhysicalDeviceSelector selector{instance};
    auto phys_ret = selector.set_surface(surface).set_minimum_version(1, 1).select();
    if (!phys_ret) {
        throw std::runtime_error("Failed to select Vulkan physical device");
    }

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        throw std::runtime_error("Failed to create Vulkan device");
    }
    device = dev_ret.value();

    vkb::SwapchainBuilder swapchain_builder{device};
    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        throw std::runtime_error("Failed to create Vulkan swapchain");
    }
    swapchain = swap_ret.value();

    swapchain_images = swapchain.get_images().value();
    swapchain_image_views = swapchain.get_image_views().value();

    graphics_queue = device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_index = device.get_queue_index(vkb::QueueType::graphics).value();
}

VulkanContext::~VulkanContext() {
    for (auto image_view : swapchain_image_views) {
        vkDestroyImageView(device.device, image_view, nullptr);
    }
    vkb::destroy_swapchain(swapchain);
    vkb::destroy_device(device);
    vkb::destroy_surface(instance.instance, surface);
    vkb::destroy_instance(instance);
}

} // namespace render