#pragma once
#include <filesystem>
#include "../core/Logger.h"

namespace render {

// [M0-EXT-11] Asynchronous SPIR-V Shader Cache Garbage Collector
class ShaderCacheGC {
public:
    static void RunGarbageCollection(const std::string& cachePath, size_t maxSizeBytes = 10 * 1024 * 1024) {
        if (std::filesystem::exists(cachePath)) {
            size_t size = std::filesystem::file_size(cachePath);
            if (size > maxSizeBytes) {
                LOG_WARN("ShaderCacheGC: Cache size {} bytes exceeds limit of {} bytes. Garbage collecting (deleting) stale cache.", size, maxSizeBytes);
                std::filesystem::remove(cachePath);
            } else {
                LOG_INFO("ShaderCacheGC: Cache size is within limits ({} bytes)", size);
            }
        }
    }
};

} // namespace render
