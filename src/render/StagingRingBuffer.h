#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>
#include <cstddef>

namespace render {

// [M1-EXT-05] Persistent-Mapped Staging Ring Buffer
class StagingRingBuffer {
public:
    void Initialize(VmaAllocator allocator, size_t totalSize, size_t maxFrameAllocation) {
        this->allocator = allocator;
        this->totalPoolSize = totalSize;
        this->maxFrameSize = maxFrameAllocation;

        VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufInfo.size = totalSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(allocator, &bufInfo, &allocInfo, &buffer, &allocation, nullptr);
        vmaMapMemory(allocator, allocation, &mappedPtr);
    }

    void Destroy() {
        if (mappedPtr) {
            vmaUnmapMemory(allocator, allocation);
            mappedPtr = nullptr;
        }
        if (buffer) {
            vmaDestroyBuffer(allocator, buffer, allocation);
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }

    void* GetFrameStagingRegion(uint32_t frameIdx) {
        size_t offset = (static_cast<size_t>(frameIdx) * maxFrameSize) % totalPoolSize;
        return static_cast<uint8_t*>(mappedPtr) + offset;
    }
    
    size_t GetFrameOffset(uint32_t frameIdx) const {
        return (static_cast<size_t>(frameIdx) * maxFrameSize) % totalPoolSize;
    }
    
    VkBuffer GetBuffer() const { return buffer; }
    size_t GetMaxFrameSize() const { return maxFrameSize; }

private:
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void* mappedPtr = nullptr;
    
    size_t totalPoolSize = 0;
    size_t maxFrameSize = 0;
};

} // namespace render
