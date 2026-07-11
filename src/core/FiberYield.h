#pragma once

#include <thread>
#include "JobSystem.h"

namespace core {

// [M0-EXT-07] Device-Agnostic Work-Stealing Job Scheduler Fiber Yield Hook
// Cooperatively yields active fiber execution
inline void YieldActiveFiber() {
    std::this_thread::yield();
}

} // namespace core
