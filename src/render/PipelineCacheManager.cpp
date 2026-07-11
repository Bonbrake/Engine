#include "PipelineCacheManager.h"
#include "Device.h"
#include "ShaderCacheGC.h"
#include "../core/Logger.h"
#include "../core/FileSystem.h"
#include <fstream>
#include <vector>

namespace render {

VkPipelineCache PipelineCacheManager::cache_ = VK_NULL_HANDLE;
std::string PipelineCacheManager::filePath_;

void PipelineCacheManager::init(Device* device, const std::string& cacheFilePath) {
    filePath_ = core::FileSystem::resolveAssetPath(cacheFilePath);
    ShaderCacheGC::RunGarbageCollection(filePath_);
    
    std::vector<char> cacheData;
    if (validateHeader(filePath_, device)) {
        std::ifstream file(filePath_, std::ios::ate | std::ios::binary);
        if (file.is_open()) {
            size_t fileSize = (size_t)file.tellg();
            cacheData.resize(fileSize);
            file.seekg(0);
            file.read(cacheData.data(), fileSize);
        }
    } else {
        LOG_WARN("Pipeline cache header mismatch or absent. Starting fresh.");
    }
    
    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.initialDataSize = cacheData.size();
    createInfo.pInitialData = cacheData.data();
    
    if (vkCreatePipelineCache(device->getLogicalDevice(), &createInfo, nullptr, &cache_) != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline cache");
    }
}

void PipelineCacheManager::shutdown(Device* device) {
    if (cache_ == VK_NULL_HANDLE) return;
    
    size_t dataSize = 0;
    if (vkGetPipelineCacheData(device->getLogicalDevice(), cache_, &dataSize, nullptr) == VK_SUCCESS) {
        std::vector<char> data(dataSize);
        if (vkGetPipelineCacheData(device->getLogicalDevice(), cache_, &dataSize, data.data()) == VK_SUCCESS) {
            std::ofstream file(filePath_, std::ios::binary | std::ios::trunc);
            if (file.is_open()) {
                file.write(data.data(), dataSize);
                LOG_INFO("Saved pipeline cache to {}", filePath_);
            }
        }
    }
    
    vkDestroyPipelineCache(device->getLogicalDevice(), cache_, nullptr);
    cache_ = VK_NULL_HANDLE;
}

bool PipelineCacheManager::validateHeader(const std::string& cacheFilePath, Device* device) {
    std::ifstream file(cacheFilePath, std::ios::binary);
    if (!file.is_open()) return false;
    
    // Header is 32 bytes
    uint32_t headerLength;
    uint32_t headerVersion;
    uint32_t vendorID;
    uint32_t deviceID;
    uint8_t pipelineCacheUUID[VK_UUID_SIZE];
    
    file.read(reinterpret_cast<char*>(&headerLength), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&headerVersion), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&vendorID), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&deviceID), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(pipelineCacheUUID), VK_UUID_SIZE);
    
    VkPhysicalDeviceProperties props = device->getVkbDevice().physical_device.properties;
    
    if (vendorID != props.vendorID || deviceID != props.deviceID) return false;
    
    for (int i = 0; i < VK_UUID_SIZE; ++i) {
        if (pipelineCacheUUID[i] != props.pipelineCacheUUID[i]) return false;
    }
    
    return true;
}

} // namespace render
