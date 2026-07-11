#pragma once

#include <volk.h>
#include <cstdint>
#include "Device.h"
#include "../core/Logger.h"

namespace render {

// [M0-EXT-14] Asynchronous Hardware Compute Interleaver & Pipeline Lifecycle Matrix
class AsyncComputeInterleaver {
public:
    AsyncComputeInterleaver() = default;
    ~AsyncComputeInterleaver() = default;

    // Submits compute work to the dedicated compute queue, coordinated with timeline semaphores.
    inline void submitAsyncCompute(Device* device, VkSemaphore timelineSemaphore, VkCommandBuffer cmd, uint64_t waitValue, uint64_t signalValue) {
        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo{};
        timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineSubmitInfo.waitSemaphoreValueCount = waitValue > 0 ? 1 : 0;
        timelineSubmitInfo.pWaitSemaphoreValues = &waitValue;
        timelineSubmitInfo.signalSemaphoreValueCount = 1;
        timelineSubmitInfo.pSignalSemaphoreValues = &signalValue;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = &timelineSubmitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkSemaphore semaphores[] = { timelineSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
        if (waitValue > 0) {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = semaphores;
            submitInfo.pWaitDstStageMask = waitStages;
        }
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = semaphores;

        VkResult res = vkQueueSubmit(device->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        if (res != VK_SUCCESS) {
            LOG_ERROR("AsyncComputeInterleaver: Queue submit failed with exit code: {}", (int)res);
        } else {
            LOG_INFO("AsyncComputeInterleaver: Submitted async compute command. Wait: {}, Signal: {}", waitValue, signalValue);
        }
    }
};

} // namespace render
