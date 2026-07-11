#pragma once

namespace render {

// [M0-EXT-09] Host-Side Command Queue Swapchain Frame-Pacing Interceptor
class FramePacing {
public:
    inline void EvaluateQueuePacingIntercept(VkDevice device, VkSemaphore timelineSemaphore, uint64_t currentFrame, uint32_t maxFramesInFlight) {
        if (currentFrame >= maxFramesInFlight) {
            VkSemaphoreWaitInfo waitInfo{};
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            uint64_t waitValue = currentFrame - (maxFramesInFlight - 1);
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores = &timelineSemaphore;
            waitInfo.pValues = &waitValue;
            
            vkWaitSemaphores(device, &waitInfo, UINT64_MAX);
        }
    }
};

} // namespace render
