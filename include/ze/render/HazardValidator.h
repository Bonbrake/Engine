#pragma once

#include <cstdint>

namespace render {

// [M0-EXT-06] Cross-Queue Vulkan Dependency Hazard Validation Layer
struct ResourceStateOwner { 
    uint32_t queueFamilyOwner; 
    bool isWritingActive; 
};

inline bool CheckBarrierHazard(const ResourceStateOwner& current, uint32_t targetQueueFamily, bool upcomingWrite) {
    return current.isWritingActive && (current.queueFamilyOwner != targetQueueFamily || upcomingWrite);
}

} // namespace render
