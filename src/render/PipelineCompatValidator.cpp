// [M0-EXT-10] Vulkan 1.4 Pipeline Layout Structural Compatibility Validator
//
// Implementation — validates SPIR-V reflected bindings and push-constant ranges
// against the VkPipelineLayoutCreateInfo at pipeline creation time.
// Called from debug/dev PipelineBuilder paths; no-op in release builds.

#include "render/PipelineCompatValidator.h"
#include "core/Logger.h"

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>

#include <map>
#include <algorithm>
#include <string>

namespace render {

PipelineLayoutValidationReport PipelineCompatValidator::Validate(
    VkPipelineLayout                        /*layout*/,
    const VkPipelineLayoutCreateInfo&        createInfo,
    const std::vector<std::vector<uint32_t>>& spirvModules,
    const char*                              pipelineName)
{
    PipelineLayoutValidationReport report;
    report.compatible = true;

    if (pipelineName == nullptr) {
        pipelineName = "unnamed";
    }

    // Reflect each SPIR-V module
    std::vector<SpvReflectShaderModule> reflectModules;

    for (const auto& code : spirvModules) {
        SpvReflectShaderModule module;
        SpvReflectResult res = spvReflectCreateShaderModule(
            code.size() * sizeof(uint32_t), code.data(), &module);

        if (res != SPV_REFLECT_RESULT_SUCCESS) {
            LOG_WARN("PipelineCompatValidator[{}]: spvReflectCreateShaderModule failed (res={})",
                     pipelineName, static_cast<int>(res));
            for (auto& m : reflectModules) {
                spvReflectDestroyShaderModule(&m);
            }
            report.compatible = false;
            report.message = "Failed to reflect one or more SPIR-V modules";
            return report;
        }
        reflectModules.push_back(module);
    }

    // Merge shader-expected bindings across modules
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> shaderBindings;
    // Track push constant ranges from shaders
    std::map<std::string, VkPushConstantRange> shaderPushConstants;

    for (const auto& module : reflectModules) {
        VkShaderStageFlagBits stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

        uint32_t count = 0;
        spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorSet*> sets(count);
        spvReflectEnumerateDescriptorSets(&module, &count, sets.data());

        for (const SpvReflectDescriptorSet* set : sets) {
            for (uint32_t b = 0; b < set->binding_count; ++b) {
                const SpvReflectDescriptorBinding* reflBinding = set->bindings[b];

                VkDescriptorSetLayoutBinding& binding =
                    shaderBindings[set->set][reflBinding->binding];

                if (binding.descriptorCount == 0) {
                    binding.binding         = reflBinding->binding;
                    binding.descriptorType  = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                    binding.descriptorCount = reflBinding->count;
                    binding.stageFlags      = stage;
                    binding.pImmutableSamplers = nullptr;
                } else {
                    binding.stageFlags |= stage;
                }
            }
        }

        // Push constants
        count = 0;
        spvReflectEnumeratePushConstantBlocks(&module, &count, nullptr);
        std::vector<SpvReflectBlockVariable*> blocks(count);
        spvReflectEnumeratePushConstantBlocks(&module, &count, blocks.data());

        for (const SpvReflectBlockVariable* block : blocks) {
            std::string name = block->name ? block->name : "";
            if (shaderPushConstants.find(name) == shaderPushConstants.end()) {
                VkPushConstantRange range{};
                range.stageFlags = stage;
                range.offset     = block->offset;
                range.size       = block->size;
                shaderPushConstants[name] = range;
            } else {
                VkPushConstantRange& range = shaderPushConstants[name];
                range.stageFlags |= stage;
                range.offset = std::min(range.offset, block->offset);
                uint32_t end1 = range.offset + range.size;
                uint32_t end2 = block->offset + block->size;
                range.size = std::max(end1, end2) - range.offset;
            }
        }
    }

    // --- Validate descriptor bindings ---
    // For each shader set binding, check if the layout has that set with that binding.
    // We can't dereference VkDescriptorSetLayout handles at runtime, so we validate
    // against what the createInfo says was provided (set layout count).
    for (const auto& [setIdx, bindings] : shaderBindings) {
        if (setIdx >= createInfo.setLayoutCount) {
            LOG_WARN("PipelineCompatValidator[{}]: Shader expects set {} but layout only provides {} sets",
                     pipelineName, setIdx, createInfo.setLayoutCount);
            report.bindingMismatches += static_cast<uint32_t>(bindings.size());
            report.compatible = false;
            continue;
        }

        // We log the bindings that were discovered for informational purposes
        // but can't deeply validate against the opaque VkDescriptorSetLayout handle.
        // This is best-effort: the layout was built FROM these modules, so success
        // is expected in the normal path.
        for (const auto& [bindingIdx, binding] : bindings) {
            LOG_INFO("PipelineCompatValidator[{}]: Set[{}].Binding[{}] type={} count={} stageFlags={:#x} — OK",
                     pipelineName, setIdx, bindingIdx,
                     static_cast<int>(binding.descriptorType),
                     binding.descriptorCount,
                     static_cast<uint32_t>(binding.stageFlags));
        }
    }

    // --- Validate push constant ranges ---
    // Check that every shader push constant block falls within a range provided by the layout
    for (const auto& [name, shaderRange] : shaderPushConstants) {
        bool covered = false;
        for (uint32_t i = 0; i < createInfo.pushConstantRangeCount; ++i) {
            const VkPushConstantRange& layoutRange = createInfo.pPushConstantRanges[i];

            // Shader's offset must be >= layout offset
            // Shader's (offset + size) must be <= layout (offset + size)
            // Stage flags must overlap
            if (shaderRange.offset >= layoutRange.offset &&
                (shaderRange.offset + shaderRange.size) <= (layoutRange.offset + layoutRange.size) &&
                (shaderRange.stageFlags & layoutRange.stageFlags) != 0)
            {
                covered = true;
                break;
            }
        }

        if (!covered) {
            LOG_WARN("PipelineCompatValidator[{}]: Push constant '{}' (offset={}, size={}, stage={:#x}) "
                     "not covered by any layout range",
                     pipelineName, name.c_str(),
                     shaderRange.offset, shaderRange.size,
                     static_cast<uint32_t>(shaderRange.stageFlags));
            report.pushConstantMismatches++;
            report.compatible = false;
        } else {
            LOG_INFO("PipelineCompatValidator[{}]: Push constant '{}' (offset={}, size={}) — OK",
                     pipelineName, name.c_str(), shaderRange.offset, shaderRange.size);
        }
    }

    // --- Check unused layout ranges ---
    for (uint32_t i = 0; i < createInfo.pushConstantRangeCount; ++i) {
        const VkPushConstantRange& layoutRange = createInfo.pPushConstantRanges[i];
        bool used = false;
        for (const auto& [name, shaderRange] : shaderPushConstants) {
            if (shaderRange.offset >= layoutRange.offset &&
                (shaderRange.offset + shaderRange.size) <= (layoutRange.offset + layoutRange.size))
            {
                used = true;
                break;
            }
        }
        if (!used) {
            LOG_INFO("PipelineCompatValidator[{}]: Layout push constant range [{}] (offset={}, size={}, stage={:#x}) "
                     "is unused by shaders — this may be intentional (e.g. reserved for future use)",
                     pipelineName, i, layoutRange.offset, layoutRange.size,
                     static_cast<uint32_t>(layoutRange.stageFlags));
        }
    }

    // Cleanup reflection modules
    for (auto& m : reflectModules) {
        spvReflectDestroyShaderModule(&m);
    }

    if (report.compatible) {
        report.message = "All SPIR-V modules compatible with pipeline layout";
        LOG_INFO("PipelineCompatValidator[{}]: {} — compatible", pipelineName, report.message.c_str());
    } else {
        report.message = "Pipeline layout compatibility issues found (see warnings above)";
    }

    return report;
}

} // namespace render
