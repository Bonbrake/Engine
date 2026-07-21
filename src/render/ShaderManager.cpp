#include "ze/render/ShaderManager.h"
#include "ze/render/Device.h"
#include "ze/core/Logger.h"

namespace render {

Device* ShaderManager::device_ = nullptr;
bool ShaderManager::shaderObjectsSupported_ = false;

void ShaderManager::init(Device* device) {
    device_ = device;
    shaderObjectsSupported_ = false;
    auto available_extensions = device->getVkbDevice().physical_device.get_extensions();
    for (const auto& ext : available_extensions) {
        if (ext == VK_EXT_SHADER_OBJECT_EXTENSION_NAME) {
            shaderObjectsSupported_ = true;
            break;
        }
    }
    LOG_INFO("ShaderManager: Shader Object support status = {}", shaderObjectsSupported_ ? "SUPPORTED" : "UNSUPPORTED");
}

void ShaderManager::shutdown() {
    device_ = nullptr;
}

void ShaderManager::bindChangedStages(VkCommandBuffer cmd, const ShaderObjectState& next, ShaderObjectState& current) {
    if (!shaderObjectsSupported_) {
        // Fallback or assert if we are strictly using this
        return;
    }

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