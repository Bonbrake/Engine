#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace render {
class Device;
class TriangleRenderer {
public:
    void init(Device* device, VkFormat colorFormat);
    void cleanup(Device* device);
    
    // Add cull function for compute pass
    void cull(VkCommandBuffer cmd, uint32_t imageIndex, VkImageView currentDepthView, Device* device);
    void draw(VkCommandBuffer cmd, uint32_t imageIndex, class MaterialSystem* materialSystem);
    
    // For logging reduction
    void readbackCount(Device* device, uint32_t imageIndex);
    
private:
    void createBuffers(Device* device);
    void createPipelines(Device* device, VkFormat colorFormat);
    void createDescriptorSets(Device* device);

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipeline wireframePipeline = VK_NULL_HANDLE; // [M1-EXT-04] Derivative variant
    
    VkPipelineLayout cullPipelineLayout = VK_NULL_HANDLE;
    VkPipeline cullPipeline = VK_NULL_HANDLE;
    
    std::vector<VkDescriptorSetLayout> graphicsSetLayouts;
    std::vector<VkDescriptorSetLayout> computeSetLayouts;
    VkBuffer descriptorBuffer = VK_NULL_HANDLE;
    VmaAllocation descriptorAllocation = VK_NULL_HANDLE;
    void* descriptorBufferMapped = nullptr;
    VkDeviceAddress descriptorBufferAddress = 0;
    VkDeviceSize graphicsSetOffset[3] = {0, 0, 0};
    VkDeviceSize computeSetOffset[3] = {0, 0, 0};

    // Buffers
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation vertexAllocation = VK_NULL_HANDLE;
    
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VmaAllocation indexAllocation = VK_NULL_HANDLE;
    
    VkBuffer instanceBuffer = VK_NULL_HANDLE;
    VmaAllocation instanceAllocation = VK_NULL_HANDLE;
    
    VkBuffer indirectBuffer[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    VmaAllocation indirectAllocation[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    
    VkBuffer countBuffer[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    VmaAllocation countAllocation[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    
    // Readback buffer
    VkBuffer countReadbackBuffer[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    VmaAllocation countReadbackAllocation[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };

    // [M1-EXT-03] Occlusion Query Double-Buffering
    VkQueryPool occlusionPools[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
    uint32_t occlusionResults[100] = {0}; // Results for 100 instances

    // To track current frame across functions
    uint32_t frameCounter = 0;
    
    // Dummy texture for frame 1
    VkImage dummyDepthImage = VK_NULL_HANDLE;
    VmaAllocation dummyDepthAllocation = VK_NULL_HANDLE;
    VkImageView dummyDepthView = VK_NULL_HANDLE;
    VkSampler depthSampler = VK_NULL_HANDLE;
};
}
