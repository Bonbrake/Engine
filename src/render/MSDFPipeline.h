#pragma once

#include <volk.h>
#include <string>

namespace render {

class Device;

class MSDFPipeline {
public:
    static void GenerateAtlas(Device* device, const std::string& ttfPath);
    static void Cleanup(Device* device);
};

} // namespace render
