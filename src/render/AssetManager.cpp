#include "AssetManager.h"
#include "core/Logger.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace render {

void AssetManager::Initialize(Device* device) {
    device_ = device;
    
    // Create 1x1 magenta fallback texture
    uint32_t magentaPixel = 0xFFFF00FF; // ABGR for full alpha magenta
    
    VkDeviceSize imageSize = 4;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    
    TextureAsset tex{};
    tex.width = 1;
    tex.height = 1;
    tex.format = imageFormat;
    
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    vmaCreateImage(device_->getAllocator(), &imageInfo, &allocInfo, &tex.image, &tex.allocation, nullptr);
    
    ExecuteOneShotStaging(imageSize, &magentaPixel, [&](VkCommandBuffer cmd, VkBuffer stagingBuffer) {
        VkImageMemoryBarrier2 barrier1{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier1.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier1.srcAccessMask = 0;
        barrier1.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier1.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier1.image = tex.image;
        barrier1.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        
        VkDependencyInfo dep1{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dep1.imageMemoryBarrierCount = 1;
        dep1.pImageMemoryBarriers = &barrier1;
        vkCmdPipelineBarrier2(cmd, &dep1);
        
        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {1, 1, 1};
        vkCmdCopyBufferToImage(cmd, stagingBuffer, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        VkImageMemoryBarrier2 barrier2{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier2.image = tex.image;
        barrier2.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        
        VkDependencyInfo dep2{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dep2.imageMemoryBarrierCount = 1;
        dep2.pImageMemoryBarriers = &barrier2;
        vkCmdPipelineBarrier2(cmd, &dep2);
    });
    
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = tex.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageFormat;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCreateImageView(device_->getLogicalDevice(), &viewInfo, nullptr, &tex.view);
    
    fallbackTextureHandle_ = textures_.Insert(tex);

    LOG_INFO("AssetManager initialized with fallback texture");
}

void AssetManager::Destroy() {
    for (size_t i = 0; i < meshes_.data.size(); ++i) {
        MeshAsset* mesh = &meshes_.data[i];
        if (mesh->vertexBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(device_->getAllocator(), mesh->vertexBuffer, mesh->vertexAllocation);
        }
        if (mesh->indexBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(device_->getAllocator(), mesh->indexBuffer, mesh->indexAllocation);
        }
    }
    
    for (size_t i = 0; i < textures_.data.size(); ++i) {
        TextureAsset* tex = &textures_.data[i];
        if (tex->image != VK_NULL_HANDLE) {
            vkDestroyImageView(device_->getLogicalDevice(), tex->view, nullptr);
            vmaDestroyImage(device_->getAllocator(), tex->image, tex->allocation);
        }
    }
    
    LOG_INFO("AssetManager destroyed");
}

void AssetManager::ExecuteOneShotStaging(size_t size, void* data, std::function<void(VkCommandBuffer, VkBuffer)> recordCmd) {
    // Allocate staging buffer
    VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = size;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    
    if (vmaCreateBuffer(device_->getAllocator(), &bufInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &stagingAllocInfo) != VK_SUCCESS) {
        LOG_CRITICAL("Failed to allocate one-shot staging buffer of size {}", size);
        return;
    }
    
    memcpy(stagingAllocInfo.pMappedData, data, size);
    
    // Create transient command buffer
    VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = device_->getGraphicsQueueIndex();
    
    VkCommandPool pool;
    vkCreateCommandPool(device_->getLogicalDevice(), &poolInfo, nullptr, &pool);
    
    VkCommandBufferAllocateInfo cmdAllocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandPool = pool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;
    
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device_->getLogicalDevice(), &cmdAllocInfo, &cmd);
    
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(cmd, &beginInfo);
    
    recordCmd(cmd, stagingBuffer);
    
    vkEndCommandBuffer(cmd);
    
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence fence;
    vkCreateFence(device_->getLogicalDevice(), &fenceInfo, nullptr, &fence);
    
    vkQueueSubmit(device_->getGraphicsQueue(), 1, &submitInfo, fence);
    
    // Fence/sync before destroying staging buffer
    vkWaitForFences(device_->getLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
    
    vkDestroyFence(device_->getLogicalDevice(), fence, nullptr);
    vkDestroyCommandPool(device_->getLogicalDevice(), pool, nullptr);
    vmaDestroyBuffer(device_->getAllocator(), stagingBuffer, stagingAlloc);
}

ecs::Handle AssetManager::LoadTexture(const std::filesystem::path& path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    
    if (!pixels) {
        LOG_WARN("Failed to load texture (soft failure, using fallback): {}", path.string());
        return fallbackTextureHandle_;
    }
    
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB; // Always sRGB for now
    
    TextureAsset tex{};
    tex.width = texWidth;
    tex.height = texHeight;
    tex.format = imageFormat;
    
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    vmaCreateImage(device_->getAllocator(), &imageInfo, &allocInfo, &tex.image, &tex.allocation, nullptr);
    
    ExecuteOneShotStaging(imageSize, pixels, [&](VkCommandBuffer cmd, VkBuffer stagingBuffer) {
        // UNDEFINED -> TRANSFER_DST_OPTIMAL
        VkImageMemoryBarrier2 barrier1{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier1.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier1.srcAccessMask = 0;
        barrier1.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier1.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier1.image = tex.image;
        barrier1.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        
        VkDependencyInfo dep1{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dep1.imageMemoryBarrierCount = 1;
        dep1.pImageMemoryBarriers = &barrier1;
        vkCmdPipelineBarrier2(cmd, &dep1);
        
        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};
        
        vkCmdCopyBufferToImage(cmd, stagingBuffer, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        // TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
        VkImageMemoryBarrier2 barrier2{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier2.image = tex.image;
        barrier2.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        
        VkDependencyInfo dep2{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dep2.imageMemoryBarrierCount = 1;
        dep2.pImageMemoryBarriers = &barrier2;
        vkCmdPipelineBarrier2(cmd, &dep2);
    });
    
    stbi_image_free(pixels);
    
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = tex.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageFormat;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCreateImageView(device_->getLogicalDevice(), &viewInfo, nullptr, &tex.view);
    
    ecs::Handle handle = textures_.Insert(tex);
    LOG_INFO("Loaded texture {} ({}x{})", path.string(), texWidth, texHeight);
    return handle;
}

ecs::Handle AssetManager::LoadMesh(const std::filesystem::path& path) {
    fastgltf::Parser parser;
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None) {
        LOG_WARN("Failed to load GLTF file {}: {}", path.string(), fastgltf::getErrorMessage(data.error()));
        return ecs::Handle();
    }
    
    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers);
    if (asset.error() != fastgltf::Error::None) {
        LOG_WARN("Failed to parse GLTF {}: {}", path.string(), fastgltf::getErrorMessage(asset.error()));
        return ecs::Handle();
    }
    
    // For now, load just the first mesh and its first primitive
    if (asset.get().meshes.empty()) {
        LOG_WARN("No meshes in GLTF {}", path.string());
        return ecs::Handle();
    }
    
    auto& primitive = asset.get().meshes[0].primitives[0];
    fastgltf::Accessor* positionAccessor = nullptr;
    fastgltf::Accessor* normalAccessor = nullptr;
    fastgltf::Accessor* texcoordAccessor = nullptr;
    
    for (auto& attrib : primitive.attributes) {
        if (attrib.name == "POSITION") positionAccessor = &asset.get().accessors[attrib.accessorIndex];
        else if (attrib.name == "NORMAL") normalAccessor = &asset.get().accessors[attrib.accessorIndex];
        else if (attrib.name == "TEXCOORD_0") texcoordAccessor = &asset.get().accessors[attrib.accessorIndex];
    }
    
    if (!positionAccessor) {
        LOG_WARN("GLTF mesh missing positions");
        return ecs::Handle();
    }
    
    std::vector<uint32_t> indices;
    if (primitive.indicesAccessor.has_value()) {
        fastgltf::Accessor& indexAccessor = asset.get().accessors[primitive.indicesAccessor.value()];
        indices.resize(indexAccessor.count);
        fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), indexAccessor,
            [&](uint32_t index, size_t idx) {
                indices[idx] = index;
            });
    } else {
        indices.resize(positionAccessor->count);
        for (size_t i = 0; i < positionAccessor->count; ++i) {
            indices[i] = static_cast<uint32_t>(i);
        }
    }
    
    std::vector<Vertex> vertices;
    vertices.resize(positionAccessor->count);
    
    fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), *positionAccessor,
        [&](glm::vec3 pos, size_t idx) {
            vertices[idx].position = pos;
        });
        
    if (normalAccessor) {
        fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), *normalAccessor,
            [&](glm::vec3 norm, size_t idx) {
                vertices[idx].normal = norm;
            });
    }
    
    if (texcoordAccessor) {
        fastgltf::iterateAccessorWithIndex<glm::vec2>(asset.get(), *texcoordAccessor,
            [&](glm::vec2 uv, size_t idx) {
                vertices[idx].uv = uv;
            });
    }
    
    // Upload logic
    size_t vSize = vertices.size() * sizeof(Vertex);
    size_t iSize = indices.size() * sizeof(uint32_t);
    size_t totalSize = vSize + iSize;
    
    std::vector<uint8_t> mergedData(totalSize);
    memcpy(mergedData.data(), vertices.data(), vSize);
    memcpy(mergedData.data() + vSize, indices.data(), iSize);
    
    MeshAsset mesh{};
    mesh.vertexCount = static_cast<uint32_t>(vertices.size());
    mesh.indexCount = static_cast<uint32_t>(indices.size());
    
    // Create vertex buffer
    VkBufferCreateInfo vInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vInfo.size = vSize;
    vInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo vAllocInfo{};
    vAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    VkResult res1 = vmaCreateBuffer(device_->getAllocator(), &vInfo, &vAllocInfo, &mesh.vertexBuffer, &mesh.vertexAllocation, nullptr);
    if (res1 != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate vertex buffer: VkResult = {}", (int)res1);
        return ecs::Handle();
    }
    
    // Create index buffer
    VkBufferCreateInfo iInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    iInfo.size = iSize;
    iInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo iAllocInfo{};
    iAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    iAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    VkResult res2 = vmaCreateBuffer(device_->getAllocator(), &iInfo, &iAllocInfo, &mesh.indexBuffer, &mesh.indexAllocation, nullptr);
    if (res2 != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate index buffer: VkResult = {}", (int)res2);
        vmaDestroyBuffer(device_->getAllocator(), mesh.vertexBuffer, mesh.vertexAllocation);
        return ecs::Handle();
    }
    
    ExecuteOneShotStaging(totalSize, mergedData.data(), [&](VkCommandBuffer cmd, VkBuffer stagingBuffer) {
        VkBufferCopy vCopy{};
        vCopy.srcOffset = 0;
        vCopy.dstOffset = 0;
        vCopy.size = vSize;
        vkCmdCopyBuffer(cmd, stagingBuffer, mesh.vertexBuffer, 1, &vCopy);
        
        VkBufferCopy iCopy{};
        iCopy.srcOffset = vSize;
        iCopy.dstOffset = 0;
        iCopy.size = iSize;
        vkCmdCopyBuffer(cmd, stagingBuffer, mesh.indexBuffer, 1, &iCopy);
    });
    
    ecs::Handle handle = meshes_.Insert(mesh);
    LOG_INFO("Loaded mesh {} ({} vertices, {} indices)", path.string(), mesh.vertexCount, mesh.indexCount);
    
    return handle;
}

MeshAsset* AssetManager::GetMesh(ecs::Handle handle) {
    return meshes_.Get(handle);
}

TextureAsset* AssetManager::GetTexture(ecs::Handle handle) {
    return textures_.Get(handle);
}

} // namespace render
