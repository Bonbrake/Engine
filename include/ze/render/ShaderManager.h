#pragma once

#include <volk.h>

namespace render {

class Device;

// [M0-EXT-02] Continuous Driver-Side Shader Object State Manager
struct ShaderObjectState { 
    VkShaderEXT stages[6] = {}; 
};

class ShaderManager {
public:
    static void init(Device* device);
    static void shutdown();
    static void bindChangedStages(VkCommandBuffer cmd, const ShaderObjectState& next, ShaderObjectState& current);
    
private:
    static Device* device_;
    static bool shaderObjectsSupported_;
};

} // namespace render
