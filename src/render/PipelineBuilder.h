#pragma once

#include <volk.h>
#include <vector>
#include <cstdint>

namespace render {

class Device;

struct PipelineLayoutData {
    std::vector<VkDescriptorSetLayout> setLayouts;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
};

class PipelineBuilder {
public:
    // Builds a single VkPipelineLayout and an array of VkDescriptorSetLayouts from a set of SPIR-V modules.
    // Call this exactly once per pipeline (e.g. passing [vert, frag] together, or [comp] alone).
    static PipelineLayoutData buildLayouts(const std::vector<std::vector<uint32_t>>& spirvModules, Device* device);
};

} // namespace render
