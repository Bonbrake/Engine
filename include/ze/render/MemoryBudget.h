#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace render {

// [M0-EXT-05] Discrete GPU Memory-Budget Tracker
inline float GetHeapHeadroom(const VkPhysicalDeviceMemoryBudgetPropertiesEXT& budget, uint32_t heapIndex) {
    uint64_t totalBudget = budget.heapBudget[heapIndex];
    uint64_t currentUsage = budget.heapUsage[heapIndex];
    return (currentUsage >= totalBudget) ? 0.0f : static_cast<float>(totalBudget - currentUsage);
}

} // namespace render
