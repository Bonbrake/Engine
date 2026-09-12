#include "ze/render/PipelineWarmup.h"
#include <fstream>
#include <cstring>

namespace ze::render {

PipelineWarmup::PipelineWarmup() {
}

void PipelineWarmup::registerPipelineDesc(const WarmupPipelineDesc& desc) {
    m_registeredPipelines.push_back(desc);
}

bool PipelineWarmup::executeWarmup() {
    m_warmedCount = 0;
    for (const auto& desc : m_registeredPipelines) {
        // In full Vulkan pipeline, vkCreateGraphicsPipelines / vkCreateComputePipelines
        // is invoked with VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT.
        (void)desc;
        ++m_warmedCount;
    }
    m_warmupComplete = true;
    return true;
}

bool PipelineWarmup::saveCacheToFile(const std::string& cachePath,
                                     uint32_t vendorID,
                                     uint32_t deviceID,
                                     const uint8_t uuid[16],
                                     const std::vector<uint8_t>& cacheData) {
    std::ofstream out(cachePath, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }

    PipelineCacheHeader header;
    header.headerLength = sizeof(PipelineCacheHeader);
    header.headerVersion = 1;
    header.vendorID = vendorID;
    header.deviceID = deviceID;
    std::memcpy(header.pipelineCacheUUID, uuid, 16);

    out.write(reinterpret_cast<const char*>(&header), sizeof(PipelineCacheHeader));
    if (!cacheData.empty()) {
        out.write(reinterpret_cast<const char*>(cacheData.data()), cacheData.size());
    }
    return true;
}

bool PipelineWarmup::loadCacheFromFile(const std::string& cachePath,
                                     uint32_t expectedVendorID,
                                     uint32_t expectedDeviceID,
                                     const uint8_t expectedUuid[16],
                                     std::vector<uint8_t>& outCacheData) {
    std::ifstream in(cachePath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    PipelineCacheHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(PipelineCacheHeader));
    if (in.gcount() != sizeof(PipelineCacheHeader)) {
        return false;
    }

    if (header.headerLength != sizeof(PipelineCacheHeader) || header.headerVersion != 1) {
        return false;
    }

    // Invalidate cache if GPU vendor, device, or driver UUID changed
    if (header.vendorID != expectedVendorID || header.deviceID != expectedDeviceID) {
        return false;
    }

    if (std::memcmp(header.pipelineCacheUUID, expectedUuid, 16) != 0) {
        return false;
    }

    in.seekg(0, std::ios::end);
    size_t totalSize = static_cast<size_t>(in.tellg());
    size_t dataSize = totalSize - sizeof(PipelineCacheHeader);

    in.seekg(sizeof(PipelineCacheHeader), std::ios::beg);
    outCacheData.resize(dataSize);
    if (dataSize > 0) {
        in.read(reinterpret_cast<char*>(outCacheData.data()), dataSize);
    }
    return true;
}

} // namespace ze::render
