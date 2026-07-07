#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <iostream>
#include <chrono>
#include <vector>

#ifndef ENABLE_VULKAN_VALIDATION_LAYERS
#define ENABLE_VULKAN_VALIDATION_LAYERS 0
#endif

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Zombie Engine", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return -1;
    }

    uint32_t ext_count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&ext_count);
    if (!extensions) {
        std::cerr << "Failed to get SDL Vulkan extensions: " << SDL_GetError() << "\n";
        return -1;
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
        std::cerr << "Failed to create Vulkan instance: " << inst_ret.error().message() << "\n";
        return -1;
    }
    vkb::Instance vkb_inst = inst_ret.value();

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, vkb_inst.instance, nullptr, &surface)) {
        std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << "\n";
        return -1;
    }

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto phys_ret = selector.set_surface(surface)
        .set_minimum_version(1, 1)
        .select();

    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan physical device: " << phys_ret.error().message() << "\n";
        return -1;
    }

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device: " << dev_ret.error().message() << "\n";
        return -1;
    }
    vkb::Device vkb_device = dev_ret.value();

    vkb::SwapchainBuilder swapchain_builder{vkb_device};
    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        std::cerr << "Failed to create Vulkan swapchain: " << swap_ret.error().message() << "\n";
        return -1;
    }
    vkb::Swapchain vkb_swapchain = swap_ret.value();

    std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();

    VkQueue graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    uint32_t graphics_queue_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool command_pool;
    vkCreateCommandPool(vkb_device.device, &pool_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vkb_device.device, &alloc_info, &command_buffer);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    vkCreateSemaphore(vkb_device.device, &semaphore_info, nullptr, &image_available_semaphore);
    vkCreateSemaphore(vkb_device.device, &semaphore_info, nullptr, &render_finished_semaphore);
    vkCreateFence(vkb_device.device, &fence_info, nullptr, &in_flight_fence);

    bool running = true;
    SDL_Event event;

    const double dt = 1.0 / 60.0;
    double accumulator = 0.0;
    auto current_time = std::chrono::high_resolution_clock::now();

    while (running) {
        auto new_time = std::chrono::high_resolution_clock::now();
        double frame_time = std::chrono::duration<double>(new_time - current_time).count();
        current_time = new_time;

        accumulator += frame_time;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        while (accumulator >= dt) {
            // Fixed update tick - YAGNI (No game logic yet)
            accumulator -= dt;
        }

        vkWaitForFences(vkb_device.device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

        uint32_t image_index;
        VkResult acquire_result = vkAcquireNextImageKHR(vkb_device.device, vkb_swapchain.swapchain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);

        if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
            std::cerr << "Swapchain out of date. Exiting gracefully for M0.\n";
            running = false;
            continue;
        }

        vkResetFences(vkb_device.device, 1, &in_flight_fence);
        vkResetCommandBuffer(command_buffer, 0);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = swapchain_images[image_index];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        // Pitch-black/dark horror aesthetic
        VkClearColorValue clear_color = {{0.01f, 0.01f, 0.015f, 1.0f}};
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdClearColorImage(command_buffer, swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &range);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {image_available_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_TRANSFER_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        VkSemaphore signal_semaphores[] = {render_finished_semaphore};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence);

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapchains[] = {vkb_swapchain.swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;
        present_info.pImageIndices = &image_index;

        vkQueuePresentKHR(graphics_queue, &present_info);
    }

    vkDeviceWaitIdle(vkb_device.device);

    vkDestroySemaphore(vkb_device.device, image_available_semaphore, nullptr);
    vkDestroySemaphore(vkb_device.device, render_finished_semaphore, nullptr);
    vkDestroyFence(vkb_device.device, in_flight_fence, nullptr);
    vkDestroyCommandPool(vkb_device.device, command_pool, nullptr);

    vkb::destroy_swapchain(vkb_swapchain);
    vkb::destroy_device(vkb_device);
    vkb::destroy_surface(vkb_inst.instance, surface);
    vkb::destroy_instance(vkb_inst);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
