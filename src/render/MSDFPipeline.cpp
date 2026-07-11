#include "MSDFPipeline.h"
#include "../core/Logger.h"
#include "Device.h"
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <vector>

namespace render {

static VkImage s_AtlasImage = VK_NULL_HANDLE;
static VmaAllocation s_AtlasAlloc = VK_NULL_HANDLE;
static VkImageView s_AtlasView = VK_NULL_HANDLE;

void MSDFPipeline::GenerateAtlas(Device* device, const std::string& ttfPath) {
    msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
    if (!ft) {
        LOG_ERROR("Failed to initialize Freetype for MSDF");
        return;
    }

    msdfgen::FontHandle* font = msdfgen::loadFont(ft, ttfPath.c_str());
    if (!font) {
        LOG_ERROR("Failed to load font: {}", ttfPath);
        msdfgen::deinitializeFreetype(ft);
        return;
    }

    // ASCII 32 to 126
    const int glyphWidth = 32;
    const int glyphHeight = 32;
    const int cols = 16;
    const int rows = 6;
    const int atlasWidth = cols * glyphWidth;
    const int atlasHeight = rows * glyphHeight;
    
    std::vector<float> atlasData(atlasWidth * atlasHeight * 3, 0.0f); // RGB floats
    
    int generatedGlyphs = 0;
    
    for (int i = 32; i < 127; i++) {
        msdfgen::Shape shape;
        if (msdfgen::loadGlyph(shape, font, i)) {
            shape.normalize();
            msdfgen::edgeColoringSimple(shape, 3.0);
            msdfgen::Bitmap<float, 3> msdf(glyphWidth, glyphHeight);
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(0.0, 0.0));
            
            int col = (i - 32) % cols;
            int row = (i - 32) / cols;
            
            for (int y = 0; y < glyphHeight; y++) {
                for (int x = 0; x < glyphWidth; x++) {
                    int ax = col * glyphWidth + x;
                    int ay = row * glyphHeight + y;
                    atlasData[(ay * atlasWidth + ax) * 3 + 0] = msdf(x, y)[0];
                    atlasData[(ay * atlasWidth + ax) * 3 + 1] = msdf(x, y)[1];
                    atlasData[(ay * atlasWidth + ax) * 3 + 2] = msdf(x, y)[2];
                }
            }
            generatedGlyphs++;
        }
    }
    
    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(ft);
    
    LOG_INFO("MSDF Atlas Generated: {}x{}, {} glyphs", atlasWidth, atlasHeight, generatedGlyphs);
    
    // Convert float RGB to byte RGBA for standard Vulkan texture upload
    std::vector<uint8_t> rgbaData(atlasWidth * atlasHeight * 4);
    for (int i = 0; i < atlasWidth * atlasHeight; i++) {
        rgbaData[i * 4 + 0] = msdfgen::clamp(int(atlasData[i * 3 + 0] * 256.f), 0, 255);
        rgbaData[i * 4 + 1] = msdfgen::clamp(int(atlasData[i * 3 + 1] * 256.f), 0, 255);
        rgbaData[i * 4 + 2] = msdfgen::clamp(int(atlasData[i * 3 + 2] * 256.f), 0, 255);
        rgbaData[i * 4 + 3] = 255;
    }
    
    // Upload to GPU
    VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufInfo.size = rgbaData.size();
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    
    VkBuffer stagingBuf;
    VmaAllocation stagingAlloc;
    vmaCreateBuffer(device->getAllocator(), &bufInfo, &allocInfo, &stagingBuf, &stagingAlloc, nullptr);
    
    void* mappedData;
    vmaMapMemory(device->getAllocator(), stagingAlloc, &mappedData);
    memcpy(mappedData, rgbaData.data(), rgbaData.size());
    vmaUnmapMemory(device->getAllocator(), stagingAlloc);
    
    VkImageCreateInfo imgInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgInfo.extent.width = atlasWidth;
    imgInfo.extent.height = atlasHeight;
    imgInfo.extent.depth = 1;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    
    VmaAllocationCreateInfo imgAllocInfo = {};
    imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    s_AtlasImage = VK_NULL_HANDLE;
    s_AtlasAlloc = VK_NULL_HANDLE;
    vmaCreateImage(device->getAllocator(), &imgInfo, &imgAllocInfo, &s_AtlasImage, &s_AtlasAlloc, nullptr);
    
    // Transfer logic
    VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = device->getGraphicsQueueIndex();
    VkCommandPool cmdPool;
    vkCreateCommandPool(device->getLogicalDevice(), &poolInfo, nullptr, &cmdPool);
    
    VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = s_AtlasImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    s_AtlasView = VK_NULL_HANDLE;
    vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &s_AtlasView);
    
    VkCommandBufferAllocateInfo cmdAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdAllocInfo.commandPool = cmdPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device->getLogicalDevice(), &cmdAllocInfo, &cmd);
    
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);
    
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = s_AtlasImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { (uint32_t)atlasWidth, (uint32_t)atlasHeight, 1 };
    vkCmdCopyBufferToImage(cmd, stagingBuf, s_AtlasImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    vkEndCommandBuffer(cmd);
    
    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());
    
    vkDestroyCommandPool(device->getLogicalDevice(), cmdPool, nullptr);
    vmaDestroyBuffer(device->getAllocator(), stagingBuf, stagingAlloc);
    
    LOG_INFO("MSDF Atlas successfully resident on GPU");
    
    LOG_INFO("MSDF Atlas successfully resident on GPU");
}

void MSDFPipeline::Cleanup(Device* device) {
    if (s_AtlasView != VK_NULL_HANDLE) {
        vkDestroyImageView(device->getLogicalDevice(), s_AtlasView, nullptr);
        s_AtlasView = VK_NULL_HANDLE;
    }
    if (s_AtlasImage != VK_NULL_HANDLE) {
        vmaDestroyImage(device->getAllocator(), s_AtlasImage, s_AtlasAlloc);
        s_AtlasImage = VK_NULL_HANDLE;
        s_AtlasAlloc = VK_NULL_HANDLE;
    }
}

} // namespace render
