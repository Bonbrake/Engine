#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ze::render {

#pragma pack(push, 1)
struct PipelineCacheHeader {
    uint32_t headerLength = 32;
    uint32_t headerVersion = 1; // VK_PIPELINE_CACHE_HEADER_VERSION_ONE
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uint8_t  pipelineCacheUUID[16] = {};
};
#pragma pack(pop)

struct WarmupPipelineDesc {
    std::string pipelineName;
    uint32_t shaderStageBits = 0; // VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
    bool depthTestEnable = true;
    bool blendEnable = false;
    uint32_t cullMode = 2; // VK_CULL_MODE_BACK_BIT
};

class PipelineWarmup {
public:
    PipelineWarmup();
    ~PipelineWarmup() = default;

    void registerPipelineDesc(const WarmupPipelineDesc& desc);

    // Warm up all registered pipelines against the cache
    bool executeWarmup();

    size_t getRegisteredPipelineCount() const { return m_registeredPipelines.size(); }
    size_t getWarmedPipelineCount() const { return m_warmedCount; }
    bool isWarmupComplete() const { return m_warmupComplete; }

    // Disk Cache Serialization
    static bool saveCacheToFile(const std::string& cachePath,
                                uint32_t vendorID,
                                uint32_t deviceID,
                                const uint8_t uuid[16],
                                const std::vector<uint8_t>& cacheData);

    static bool loadCacheFromFile(const std::string& cachePath,
                                uint32_t expectedVendorID,
                                uint32_t expectedDeviceID,
                                const uint8_t expectedUuid[16],
                                std::vector<uint8_t>& outCacheData);

private:
    std::vector<WarmupPipelineDesc> m_registeredPipelines;
    size_t m_warmedCount = 0;
    bool m_warmupComplete = false;
};

} // namespace ze::render
