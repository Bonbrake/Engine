#pragma once

#include <cstdint>
#include <atomic>
#include <thread>

namespace core {

// [M0-EXT-12] Compute-Worker Thread-Affinity Bitmask Allocator
class ThreadAffinityAllocator {
public:
    uint64_t AllocateBitmask() {
        static std::atomic<uint32_t> nextCore{0};
        uint32_t core = nextCore.fetch_add(1, std::memory_order_relaxed);
        uint32_t numCores = std::thread::hardware_concurrency();
        if (numCores == 0) numCores = 1;
        uint32_t targetCore = core % std::min(numCores, 64U);
        return 1ULL << targetCore;
    }
};

} // namespace core
