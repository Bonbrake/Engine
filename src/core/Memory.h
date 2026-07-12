#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <vk_mem_alloc.h>
#include <optional>

namespace core {
namespace memory {

// [M0-EXT-01] Hardware-Topology enkiTS Pinning Allocator
// Rounds up to the nearest 64-byte cache-line boundary; Size=0 -> 0
inline size_t AlignToCacheLine(size_t size) {
    assert(size <= SIZE_MAX - 63); // overflow guard
    return (size + 63) & ~size_t(63);
}


struct SubAllocation {
    VkDeviceSize offset;
    VmaVirtualAllocation handle;
};

class WorkerSubAllocator {
public:
    WorkerSubAllocator() = default;

    ~WorkerSubAllocator() {
        if (block_ != VK_NULL_HANDLE) {
            vmaDestroyVirtualBlock(block_);
        }
    }

    // Disable copy/move for simplicity given the RAII handle
    WorkerSubAllocator(const WorkerSubAllocator&) = delete;
    WorkerSubAllocator& operator=(const WorkerSubAllocator&) = delete;

    void Create(VkDeviceSize totalSize) {
        assert(block_ == VK_NULL_HANDLE && "Virtual block already created");
        VmaVirtualBlockCreateInfo blockInfo = {};
        blockInfo.size = totalSize;
        VkResult res = vmaCreateVirtualBlock(&blockInfo, &block_);
        assert(res == VK_SUCCESS && "Failed to create VmaVirtualBlock");
        (void)res;
    }

    std::optional<SubAllocation> Allocate(size_t size) {
        assert(block_ != VK_NULL_HANDLE && "Virtual block not created");
        if (size == 0) return std::nullopt;

        VmaVirtualAllocationCreateInfo allocInfo = {};
        allocInfo.size = size;
        allocInfo.alignment = 64; // Base cache-line alignment

        // Apply our specific padding logic on top of VMA's base alignment
        allocInfo.size = AlignToCacheLine(size);

        VmaVirtualAllocation alloc;
        VkDeviceSize offset;
        VkResult res = vmaVirtualAllocate(block_, &allocInfo, &alloc, &offset);

        if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res != VK_SUCCESS) {
            return std::nullopt;
        }

        return SubAllocation{offset, alloc};
    }

    void Free(VmaVirtualAllocation handle) {
        assert(block_ != VK_NULL_HANDLE && "Virtual block not created");
        if (handle != VK_NULL_HANDLE) {
            vmaVirtualFree(block_, handle);
        }
    }

private:
    VmaVirtualBlock block_ = VK_NULL_HANDLE;
};


} // namespace memory
} // namespace core
