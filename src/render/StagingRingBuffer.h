#pragma once

#include <vulkan/vulkan.h>
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
    size_t Allocate(size_t size) {
        if (size == 0) return head;
        if (size > totalPoolSize) throw std::out_of_range("Allocation exceeds total pool size");

        if (usedCount + size > totalPoolSize) {
            throw std::out_of_range("Ring buffer full");
        }

        // We can only allocate if there's enough contiguous space at the end, 
        // OR we wrap around. If we wrap around, the space at the end is lost 
        // until tail advances.
        size_t availableAtEnd = totalPoolSize - head;
        if (size > availableAtEnd) {
            // Need to wrap around. Check if tail has enough room.
            if (tail < size) {
                throw std::out_of_range("Ring buffer full (fragmented)");
            }
            // Waste the end space by pretending it's used.
            usedCount += availableAtEnd; 
            head = 0; // wrap
        }

        size_t allocatedOffset = head;
        head = (head + size) % totalPoolSize;
        usedCount += size;
        return allocatedOffset;
    }

    void Free(size_t size) {
        if (size > usedCount) throw std::out_of_range("Freeing more than used");
        tail = (tail + size) % totalPoolSize;
        usedCount -= size;
    }
    
    size_t GetHead() const { return head; }
    size_t GetTail() const { return tail; }
    size_t GetUsedCount() const { return usedCount; }

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
};

} // namespace render
