#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace core {
namespace memory {

// [M0-EXT-01] Hardware-Topology enkiTS Pinning Allocator
// Rounds up to the nearest 64-byte cache-line boundary; Size=0 -> 0
inline size_t AlignToCacheLine(size_t size) {
    assert(size <= SIZE_MAX - 63); // overflow guard
    return (size + 63) & ~size_t(63);
}

// TODO: thin wrapper over VMA sub-allocation that pins worker allocations
// We will integrate this deeply once VMA allocator is created in the render context.

} // namespace memory
} // namespace core
