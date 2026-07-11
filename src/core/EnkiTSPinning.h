#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <cstdint>
#include <cassert>

namespace core {

// [M0-EXT-01] VMA pinned memory wrapper
inline size_t AlignToCacheLine(size_t size) {
    if (size > SIZE_MAX - 63) {
        return 0; // Guard against overflow
    }
    return (size + 63) & ~size_t(63);
}

// [M0-EXT-01] Pins VMA buffer allocation to 64-byte cache-line boundary
inline VkResult CreatePinnedBuffer(
    VmaAllocator allocator,
    const VkBufferCreateInfo* bufferInfo,
    const VmaAllocationCreateInfo* allocInfo,
    VkBuffer* buffer,
    VmaAllocation* allocation,
    VmaAllocationInfo* allocationInfo)
{
    return vmaCreateBufferWithAlignment(
        allocator,
        bufferInfo,
        allocInfo,
        64, // 64-byte cache line alignment
        buffer,
        allocation,
        allocationInfo
    );
}

} // namespace core
