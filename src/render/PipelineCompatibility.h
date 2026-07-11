#pragma once
#include <volk.h>
#include <vector>
#include "../core/Logger.h"

namespace render {

// [M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator
class PipelineCompatibility {
public:
    static bool ValidateLayoutCompatibility(
        const std::vector<VkDescriptorSetLayout>& layoutsA,
        const std::vector<VkDescriptorSetLayout>& layoutsB) 
    {
        size_t minSets = std::min(layoutsA.size(), layoutsB.size());
        for (size_t i = 0; i < minSets; ++i) {
            if (layoutsA[i] != layoutsB[i]) {
                LOG_WARN("Pipeline Compatibility Warning: Descriptor set layout mismatch at index {}!", i);
                return false;
            }
        }
        return true;
    }
};

} // namespace render
