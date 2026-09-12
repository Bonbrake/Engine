#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace render {

// [M1-EXT-05] Persistent-Mapped Staging Ring Buffer
class StagingRingBuffer {
public:
    void Initialize(VmaAllocator allocator, size_t totalSize) {
        this->allocator = allocator;
        this->totalPoolSize = totalSize;
        this->head = 0;
        this->tail = 0;
        this->usedCount = 0;

        VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufInfo.size = totalSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        if (allocator != VK_NULL_HANDLE) {
            vmaCreateBuffer(allocator, &bufInfo, &allocInfo, &buffer, &allocation, nullptr);
            vmaMapMemory(allocator, allocation, &mappedPtr);
        }
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

    // Allocate continuous block of memory in the ring buffer.
    // Uses `usedCount` to disambiguate head == tail (empty vs full).
    size_t Allocate(size_t size, size_t alignment = 64) {
        if (size == 0) return head;
        if (alignment == 0) alignment = 1;
        size_t actualSize = (size + (alignment - 1)) & ~(alignment - 1);
        if (actualSize > totalPoolSize) throw std::out_of_range("Allocation exceeds total pool size");

        // Align head
        size_t alignedHead = (head + (alignment - 1)) & ~(alignment - 1);
        size_t headPad = alignedHead - head;

        // If buffer is completely empty, reset head and tail to 0 if allocation doesn't fit at end
        if (usedCount == 0 && alignedHead + actualSize > totalPoolSize) {
            head = 0;
            tail = 0;
            wrapPoint = 0;
            alignedHead = 0;
            headPad = 0;
        }

        // Check if fits contiguously at the end
        if (alignedHead + actualSize <= totalPoolSize) {
            // When tail > head, head cannot cross tail
            if (head < tail && alignedHead + actualSize > tail) {
                throw std::out_of_range("Ring buffer full");
            }
            if (usedCount + headPad + actualSize > totalPoolSize) {
                throw std::out_of_range("Ring buffer full");
            }

            size_t allocatedOffset = alignedHead;
            head = (alignedHead + actualSize) % totalPoolSize;
            usedCount += headPad + actualSize;
            return allocatedOffset;
        }

        // Need to wrap around to offset 0
        if (tail <= actualSize) {
            throw std::out_of_range("Ring buffer full (fragmented)");
        }
        size_t endWasted = totalPoolSize - head;
        if (usedCount + endWasted + actualSize > totalPoolSize) {
            throw std::out_of_range("Ring buffer full");
        }

        wrapPoint = head;
        usedCount += endWasted;
        head = 0;

        size_t allocatedOffset = 0;
        head = actualSize % totalPoolSize;
        usedCount += actualSize;
        return allocatedOffset;
    }

    void Free(size_t size, size_t alignment = 64) {
        if (size == 0) return;
        if (alignment == 0) alignment = 1;
        size_t actualSize = (size + (alignment - 1)) & ~(alignment - 1);
        if (actualSize > usedCount) throw std::out_of_range("Freeing more than used");

        if (wrapPoint > 0) {
            if (tail + actualSize >= wrapPoint) {
                size_t endWasted = totalPoolSize - wrapPoint;
                size_t totalFreed = actualSize + endWasted;
                if (totalFreed > usedCount) {
                    usedCount = (actualSize <= usedCount) ? (usedCount - actualSize) : 0;
                } else {
                    usedCount -= totalFreed;
                }
                tail = (tail + actualSize) - wrapPoint;
                wrapPoint = 0;
                return;
            }
        }

        tail = (tail + actualSize) % totalPoolSize;
        usedCount -= actualSize;
    }
    
    size_t GetHead() const { return head; }
    size_t GetTail() const { return tail; }
    size_t GetUsedCount() const { return usedCount; }
    size_t GetWrapPoint() const { return wrapPoint; }

    VkBuffer GetBuffer() const { return buffer; }
    void* GetMappedPtr() const { return mappedPtr; }

private:
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void* mappedPtr = nullptr;
    
    size_t totalPoolSize = 0;
    size_t head = 0;
    size_t tail = 0;
    size_t usedCount = 0; // Disambiguation strategy: tracks exact bytes used
    size_t wrapPoint = 0; // Tracks offset of wrap to reclaim end-padding on free
};

} // namespace render
