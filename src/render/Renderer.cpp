#include "Renderer.h"
#include "../core/EventBus.h"
#include <iostream>
#include <stdexcept>

namespace render {

Renderer::Renderer(SDL_Window* window) {
    ctx_ = std::make_unique<VulkanContext>(window);

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = ctx_->graphics_queue_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(ctx_->device.device, &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(ctx_->device.device, &alloc_info, &command_buffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateSemaphore(ctx_->device.device, &semaphore_info, nullptr, &image_available_semaphore_);
    vkCreateSemaphore(ctx_->device.device, &semaphore_info, nullptr, &render_finished_semaphore_);
    vkCreateFence(ctx_->device.device, &fence_info, nullptr, &in_flight_fence_);
}

Renderer::~Renderer() {
    wait_idle();
    vkDestroySemaphore(ctx_->device.device, image_available_semaphore_, nullptr);
    vkDestroySemaphore(ctx_->device.device, render_finished_semaphore_, nullptr);
    vkDestroyFence(ctx_->device.device, in_flight_fence_, nullptr);
    vkDestroyCommandPool(ctx_->device.device, command_pool_, nullptr);
    ctx_.reset();
}

void Renderer::wait_idle() {
    vkDeviceWaitIdle(ctx_->device.device);
}

void Renderer::draw() {
    vkWaitForFences(ctx_->device.device, 1, &in_flight_fence_, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult acquire_result = vkAcquireNextImageKHR(ctx_->device.device, ctx_->swapchain.swapchain, UINT64_MAX, image_available_semaphore_, VK_NULL_HANDLE, &image_index);

    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        std::cerr << "Swapchain out of date. Exiting gracefully for M0.\n";
        core::EventBus::get().publish(core::WindowCloseEvent{});
        return;
    } else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    vkResetFences(ctx_->device.device, 1, &in_flight_fence_);
    vkResetCommandBuffer(command_buffer_, 0);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer_, &begin_info);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = ctx_->swapchain_images[image_index];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Deep, dark pitch black horror aesthetic
    VkClearColorValue clear_color = {{0.005f, 0.005f, 0.008f, 1.0f}};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdClearColorImage(command_buffer_, ctx_->swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &range);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(command_buffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(command_buffer_);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphore_};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_TRANSFER_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer_;

    VkSemaphore signal_semaphores[] = {render_finished_semaphore_};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(ctx_->graphics_queue, 1, &submit_info, in_flight_fence_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {ctx_->swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(ctx_->graphics_queue, &present_info);
}

} // namespace render