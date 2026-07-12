#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace core {
namespace memory {

// [M0-EXT-01] Hardware-Topology enkiTS Pinning Allocator
// General alignment helper. 'alignment' MUST be a power of two.
// Note: In Release builds (NDEBUG), precondition asserts compile out for performance.
inline size_t AlignSize(size_t size, size_t alignment = 64) {
    if (size == 0) return 0;

    // Precondition: alignment must be a power of two (and > 0)
    assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");

    // Precondition: guard against overflow
    assert(size <= SIZE_MAX - (alignment - 1) && "Size too large for requested alignment");

    return (size + alignment - 1) & ~(alignment - 1);
}

} // namespace memory
} // namespace core
