#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <cassert>

namespace render {

// [M1-EXT-03] Multi-Threaded Command Pool Matrix
class CommandPoolMatrix {
public:
    void Initialize(VkDevice device, uint32_t queueFamilyIndex, uint32_t numFrameResources, uint32_t numThreads) {
        this->device = device;
        this->threadCount = numThreads;
        
        pools.resize(numFrameResources * numThreads);
        
        for (auto& pool : pools) {
            VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.queueFamilyIndex = queueFamilyIndex;
            // Command buffer reuse pooling: pools are never destroyed/recreated, only reset
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            
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
