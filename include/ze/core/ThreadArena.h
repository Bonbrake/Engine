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

} // namespace core
