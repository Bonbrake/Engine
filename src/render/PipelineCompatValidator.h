#pragma once

#include <volk.h>
#include <vector>
#include <cstdint>
#include <string>

// [M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator
//
// Validates at pipeline/creation time that SPIR-V reflected descriptor bindings
// and push-constant ranges match the active VkPipelineLayout. Catches mismatches
// before they become GPU device-lost hangs or texture corruption — especially
// during shader hot-reload, mod loading, or cross-module layout changes.
//
// Depends-on: M0-EXT-08 (DescriptorSlotAllocator for bindless budget check)

namespace render {

struct PipelineLayoutValidationReport {
    bool            compatible = true;
    std::string     message;
    uint32_t        bindingMismatches = 0;
    uint32_t        pushConstantMismatches = 0;
    uint32_t        stageFlagMismatches = 0;
};

class PipelineCompatValidator {
public:
    // Validate that the given SPIR-V modules are compatible with the pipeline layout
    // described by createInfo. Returns a report with mismatch details.
    // Works in both debug and release — in release builds validation is no-op (returns
    // compatible=true with no checks) for zero overhead.
    static PipelineLayoutValidationReport Validate(
        VkPipelineLayout                 layout,
        const VkPipelineLayoutCreateInfo& createInfo,
        const std::vector<std::vector<uint32_t>>& spirvModules,
        const char*                       pipelineName = nullptr);

private:
    // Internal validation helpers
    static bool CheckDescriptorSetCompatibility(
        const VkDescriptorSetLayoutCreateInfo& layoutCI,
        const void*                            shaderBindings,
        uint32_t                               shaderBindingCount,
        uint32_t                               setIndex,
        PipelineLayoutValidationReport&        report);

    static bool CheckPushConstantCompatibility(
        const VkPushConstantRange* layoutRanges,
        uint32_t                   layoutRangeCount,
        const void*                shaderBlocks,
        uint32_t                   shaderBlockCount,
        PipelineLayoutValidationReport& report);
};

} // namespace render
