#pragma once

#include <volk.h>

namespace render {

// [M0-EXT-02] Continuous Driver-Side Shader Object State Manager
struct ShaderObjectState { 
    VkShaderEXT stages[6] = {}; 
};

inline void BindChangedStages(VkCommandBuffer cmd, const ShaderObjectState& next, ShaderObjectState& current) {
    const VkShaderStageFlagBits kStageBits[6] = {
        VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT
    };
    for (int i = 0; i < 6; ++i) {
        if (next.stages[i] != current.stages[i]) {
            vkCmdBindShadersEXT(cmd, 1, &kStageBits[i], &next.stages[i]);
            current.stages[i] = next.stages[i];
        }
    }
}

} // namespace render
