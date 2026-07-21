#pragma once

#include <volk.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

namespace render {

class VulkanContext;

struct CapabilityTier {
    bool meshShaders = false;
    bool rtPipeline = false;
    bool deviceGeneratedCommands = false;
    bool descriptorBuffer = false;
    bool graphicsPipelineLibrary = false;
    bool subgroupPartitioned = false;
    bool variableRateShading = false;
    bool opacityMicromap = false;
    bool fragmentShaderInterlock = false;
    bool attachmentFeedbackLoop = false;
    bool unifiedImageLayouts = false;
    bool queryTimestamps = false;
    bool shaderObject = false;
    
    uint32_t vendorID = 0;
    bool supportsNVCheckpoints = false;
    bool supportsAMDMarkers = false;
    
    // Tier-0 minimum hardware constraint check
    bool isTier0 = true;
};

class Device {
public:
    Device(VulkanContext* context);
    ~Device();

    void waitIdle();

    vkb::Device getVkbDevice() const { return vkbDevice_; }
    VkDevice getLogicalDevice() const { return vkbDevice_.device; }
    VkPhysicalDevice getPhysicalDevice() const { return vkbDevice_.physical_device.physical_device; }
    VmaAllocator getAllocator() const { return allocator_; }
    const CapabilityTier& getCapabilities() const { return caps_; }

    VkQueryPool getQueryPool() const { return queryPool_; }
    float getTimestampPeriod() const { return timestampPeriod_; }
    
    const VkPhysicalDeviceDescriptorBufferPropertiesEXT& getDescriptorBufferProperties() const { return descriptorBufferProperties_; }

    VkQueue getGraphicsQueue() const { return graphicsQueue_; }
    uint32_t getGraphicsQueueIndex() const { return graphicsQueueIndex_; }
    VkQueue getComputeQueue() const { return computeQueue_; }
    uint32_t getComputeQueueIndex() const { return computeQueueIndex_; }

    void setDebugObjectName(VkObjectType objectType, uint64_t objectHandle, const char* name);

private:
    void selectPhysicalDevice(VulkanContext* context);
    void createLogicalDevice();
    void initVMA(VulkanContext* context);
    void checkCapabilities();

    vkb::Device vkbDevice_;
    VmaAllocator allocator_ = VK_NULL_HANDLE;

    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueIndex_ = 0;

    // [M0-EXT-14] Async Compute Queue Separation
    VkQueue computeQueue_ = VK_NULL_HANDLE;
    uint32_t computeQueueIndex_ = 0;

    CapabilityTier caps_;
    VkQueryPool queryPool_ = VK_NULL_HANDLE;
    float timestampPeriod_ = 0.0f;
    
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT};
};

} // namespace render
