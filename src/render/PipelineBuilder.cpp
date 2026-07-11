#include "PipelineBuilder.h"
#include "Device.h"
#include "../core/Logger.h"
#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>
#include <map>
#include <algorithm>
#include <string>

namespace render {

PipelineLayoutData PipelineBuilder::buildLayouts(const std::vector<std::vector<uint32_t>>& spirvModules, Device* device) {
    PipelineLayoutData result;
    
    std::vector<SpvReflectShaderModule> reflectModules;
    
    for (const auto& code : spirvModules) {
        SpvReflectShaderModule module;
        SpvReflectResult res = spvReflectCreateShaderModule(code.size() * sizeof(uint32_t), code.data(), &module);
        if (res != SPV_REFLECT_RESULT_SUCCESS) {
            LOG_ERROR("PipelineBuilder: spvReflectCreateShaderModule failed with result {}", static_cast<int>(res));
            // Cleanup already parsed modules
            for (auto& m : reflectModules) {
                spvReflectDestroyShaderModule(&m);
            }
            return result;
        }
        reflectModules.push_back(module);
    }
    
    // Merge bindings across all modules
    // Key: set index, Value: map of binding index to VkDescriptorSetLayoutBinding
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> setBindings;
    
    // Merge push constants across all modules
    // Key: block name, Value: VkPushConstantRange
    std::map<std::string, VkPushConstantRange> pushConstants;
    
    for (const auto& module : reflectModules) {
        VkShaderStageFlagBits stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);
        
        uint32_t count = 0;
        spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorSet*> sets(count);
        spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
        
        for (const SpvReflectDescriptorSet* set : sets) {
            for (uint32_t b = 0; b < set->binding_count; ++b) {
                const SpvReflectDescriptorBinding* reflBinding = set->bindings[b];
                
                VkDescriptorSetLayoutBinding& binding = setBindings[set->set][reflBinding->binding];
                if (binding.descriptorCount == 0) {
                    // New binding
                    binding.binding = reflBinding->binding;
                    binding.descriptorType = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                    binding.descriptorCount = reflBinding->count;
                    binding.stageFlags = stage;
                    binding.pImmutableSamplers = nullptr;
                } else {
                    // Merge
                    binding.stageFlags |= stage;
                    // Note: We assume the descriptorType and descriptorCount match across stages if they share (set, binding).
                }
                
                LOG_INFO("PipelineBuilder: Discovered Binding (Set={}, Binding={}, Type={}, Count={}, Stage={})",
                    binding.binding, binding.binding, static_cast<int>(binding.descriptorType), binding.descriptorCount, static_cast<int>(binding.stageFlags));
            }
        }
        
        count = 0;
        spvReflectEnumeratePushConstantBlocks(&module, &count, nullptr);
        std::vector<SpvReflectBlockVariable*> blocks(count);
        spvReflectEnumeratePushConstantBlocks(&module, &count, blocks.data());
        
        for (const SpvReflectBlockVariable* block : blocks) {
            std::string name = block->name ? block->name : "";
            if (pushConstants.find(name) == pushConstants.end()) {
                // New push constant block
                VkPushConstantRange range{};
                range.stageFlags = stage;
                range.offset = block->offset;
                range.size = block->size;
                pushConstants[name] = range;
            } else {
                // Merge push constant block
                VkPushConstantRange& range = pushConstants[name];
                range.stageFlags |= stage;
                uint32_t oldOffset = range.offset;
                range.offset = std::min(range.offset, block->offset);
                // Size needs to cover the max extent
                uint32_t end1 = oldOffset + range.size;
                uint32_t end2 = block->offset + block->size;
                range.size = std::max(end1, end2) - range.offset;
            }
            
            LOG_INFO("PipelineBuilder: Discovered Push Constant Block '{}' (Offset={}, Size={}, Stage={})",
                name, pushConstants[name].offset, pushConstants[name].size, static_cast<int>(pushConstants[name].stageFlags));
        }
    }
    
    // Create Descriptor Set Layouts
    // Find the max set index to fill gaps
    uint32_t maxSetIndex = 0;
    if (!setBindings.empty()) {
        maxSetIndex = setBindings.rbegin()->first;
    }
    
    result.setLayouts.resize(setBindings.empty() ? 0 : maxSetIndex + 1, VK_NULL_HANDLE);
    
    for (uint32_t s = 0; s <= maxSetIndex; ++s) {
        std::vector<VkDescriptorSetLayoutBinding> bindingsArray;
        
        if (setBindings.find(s) != setBindings.end()) {
            for (const auto& pair : setBindings[s]) {
                bindingsArray.push_back(pair.second);
            }
        }
        
        VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindingsArray.size());
        layoutInfo.pBindings = bindingsArray.data();
        
        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(device->getLogicalDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            LOG_ERROR("PipelineBuilder: Failed to create descriptor set layout for set {}", s);
        }
        result.setLayouts[s] = layout;
    }
    
    // Create Pipeline Layout
    std::vector<VkPushConstantRange> pushConstantRanges;
    for (const auto& pair : pushConstants) {
        pushConstantRanges.push_back(pair.second);
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(result.setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = result.setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    
    if (vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &result.pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("PipelineBuilder: Failed to create pipeline layout");
    }
    
    // Cleanup
    for (auto& m : reflectModules) {
        spvReflectDestroyShaderModule(&m);
    }
    
    return result;
}

} // namespace render
