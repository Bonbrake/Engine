#pragma once

#include <volk.h>
#include <string>

namespace render {

class Device;

// [M0-EXT-03] Disk-Persisted Pipeline Cache Header Validator
class PipelineCacheManager {
public:
    static void init(Device* device, const std::string& cacheFilePath);
    static void shutdown(Device* device);
    
    static VkPipelineCache getCache() { return cache_; }

private:
    static bool validateHeader(const std::string& cacheFilePath, Device* device);
    
    static VkPipelineCache cache_;
    static std::string filePath_;
};

} // namespace render
