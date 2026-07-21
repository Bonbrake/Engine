#pragma once

#include <vector>
#include <atomic>
#include <cstdint>

namespace core {

typedef void* OSFileHandle; // Placeholder for native handle

// [M0-EXT-04] Asynchronous Native OS File Handle Ring Buffer
struct FileHandleRing {
    std::vector<OSFileHandle> handles;
    std::atomic<uint32_t> cursor{0};
};

inline OSFileHandle AcquireHandle(FileHandleRing& ring) {
    if (ring.handles.empty()) return nullptr;
    uint32_t idx = ring.cursor.fetch_add(1, std::memory_order_relaxed);
    return ring.handles[idx % ring.handles.size()];
}

} // namespace core
