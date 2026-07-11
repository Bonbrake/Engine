#pragma once

#include <cstdint>
#include <cstddef>

namespace core {

// [M1-EXT-07] Thread-Local Zero-Allocation Linear Page-Bump Arena
struct BumpArena {
    uint8_t* memoryBufferPage = nullptr;
    size_t capacity = 0;
    size_t currentOffset = 0;

    inline void Reset() {
        currentOffset = 0;
    }
};

inline void* ArenaAllocateBump(BumpArena& arena, size_t size, size_t alignment = 16) {
    size_t alignedOffset = (arena.currentOffset + alignment - 1) & ~(alignment - 1);
    if (alignedOffset + size > arena.capacity) {
        return nullptr; // Arena page boundary hit
    }
    arena.currentOffset = alignedOffset + size;
    return arena.memoryBufferPage + alignedOffset;
}

} // namespace core
