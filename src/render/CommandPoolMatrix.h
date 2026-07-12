#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace render {

// [M1-EXT-03] Multi-Threaded Command Pool Matrix
class CommandPoolMatrix {
public:
    // Internal/test-support seam: computes the pool layout ONLY (sets threadCount,
    // sizes the pool array). Performs ZERO Vulkan calls, so it can be used to exercise
    // GetPoolIndex math without a live VkDevice. NOT a substitute for Initialize in
    // production code — it does not create any command pools. Production callers must
    // use Initialize(), which creates the actual VkCommandPool objects.
    void ConfigureLayout(uint32_t numFrameResources, uint32_t numThreads) {
        this->threadCount = numThreads;
        pools.assign(static_cast<size_t>(numFrameResources) * numThreads, VK_NULL_HANDLE);
    }

    void Initialize(VkDevice device, uint32_t queueFamilyIndex, uint32_t numFrameResources, uint32_t numThreads) {
        ConfigureLayout(numFrameResources, numThreads);
        if (device == VK_NULL_HANDLE) {
            throw std::invalid_argument("CommandPoolMatrix::Initialize requires a valid VkDevice (got VK_NULL_HANDLE)");
        }
        this->device = device;

        for (auto& pool : pools) {
            VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.queueFamilyIndex = queueFamilyIndex;
            // Command buffer reuse pooling: pools are never destroyed/recreated, only reset
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            // device is guaranteed non-null here (Initialize throws above on VK_NULL_HANDLE)
            vkCreateCommandPool(device, &poolInfo, nullptr, &pool);
        }
    }

    void Destroy() {
        for (auto& pool : pools) {
            if (pool != VK_NULL_HANDLE) {
                vkDestroyCommandPool(device, pool, nullptr);
            }
        }
        pools.clear();
    }
    
    // Flat linear index mapping each (in-flight frame, recording thread) to a dedicated pool
    uint32_t GetPoolIndex(uint32_t frameResourceIndex, uint32_t threadId) const {
        if (threadId >= threadCount) {
            throw std::out_of_range("ThreadId out of bounds in CommandPoolMatrix");
        }
        return (frameResourceIndex * threadCount) + threadId;
    }
    
    VkCommandPool GetPool(uint32_t frameResourceIndex, uint32_t threadId) const {
        return pools[GetPoolIndex(frameResourceIndex, threadId)];
    }

private:
    VkDevice device = VK_NULL_HANDLE;
    uint32_t threadCount = 0;
    std::vector<VkCommandPool> pools;
};

} // namespace render
