#include "MaterialSystem.h"
#include "ze/core/Logger.h"

namespace render {

void MaterialSystem::Initialize(Device* device) {
    device_ = device;

    // Create default sampler
    VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    
    if (vkCreateSampler(device_->getLogicalDevice(), &samplerInfo, nullptr, &defaultSampler_) != VK_SUCCESS) {
        LOG_ERROR("Failed to create default sampler for MaterialSystem");
    }

    // Determine aligned block size for a single material's descriptors
    const auto& props = device_->getDescriptorBufferProperties();
    VkDeviceSize alignment = props.descriptorBufferOffsetAlignment;
    // For now, assuming material set just contains 1 combined image sampler at binding 0.
    // If the pipeline changes, we might query the layout, but we need enough room for the sampler.
    VkDeviceSize requiredSize = props.combinedImageSamplerDescriptorSize;
    alignedBlockSize_ = (requiredSize + alignment - 1) & ~(alignment - 1);

    // Create mapped descriptor buffer
    VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufInfo.size = MAX_MATERIALS * alignedBlockSize_;
    bufInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | 
                    VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | 
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (vmaCreateBuffer(device_->getAllocator(), &bufInfo, &allocInfo, &descriptorBuffer_, &descriptorAllocation_, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate MaterialSystem descriptor buffer");
        return;
    }

    VmaAllocationInfo vmaInfo;
    vmaGetAllocationInfo(device_->getAllocator(), descriptorAllocation_, &vmaInfo);
    descriptorBufferMapped_ = vmaInfo.pMappedData;

    VkBufferDeviceAddressInfoEXT addrInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT };
    addrInfo.buffer = descriptorBuffer_;
    descriptorBufferAddress_ = vkGetBufferDeviceAddress(device_->getLogicalDevice(), &addrInfo);

    LOG_INFO("MaterialSystem Initialized: Capacity {}, Aligned Block Size {} bytes", MAX_MATERIALS, alignedBlockSize_);
}

void MaterialSystem::Destroy() {
    if (defaultSampler_ != VK_NULL_HANDLE) {
        vkDestroySampler(device_->getLogicalDevice(), defaultSampler_, nullptr);
        defaultSampler_ = VK_NULL_HANDLE;
    }
    
    if (descriptorBuffer_ != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_->getAllocator(), descriptorBuffer_, descriptorAllocation_);
        descriptorBuffer_ = VK_NULL_HANDLE;
        descriptorAllocation_ = VK_NULL_HANDLE;
    }
}

ecs::Handle MaterialSystem::CreateMaterial(const MaterialAsset& config, AssetManager* assetManager) {
    if (materials_.freeIndices.empty() && materials_.data.size() >= MAX_MATERIALS) {
        LOG_ERROR("MaterialSystem overflow: Maximum capacity of {} materials reached", MAX_MATERIALS);
        return ecs::Handle{0xFFFFFFFF, 0};
    }

    TextureAsset* tex = assetManager->GetTexture(config.albedoTexture);
    if (!tex || tex->view == VK_NULL_HANDLE) {
        LOG_WARN("MaterialSystem CreateMaterial failed: Invalid or missing albedo texture handle");
        return ecs::Handle{0xFFFFFFFF, 0};
    }

    ecs::Handle handle = materials_.Insert(config);

    // Write descriptor
    VkDescriptorImageInfo imgInfo = {};
    imgInfo.sampler = defaultSampler_;
    imgInfo.imageView = tex->view;
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorGetInfoEXT getInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    getInfo.data.pCombinedImageSampler = &imgInfo;

    const auto& props = device_->getDescriptorBufferProperties();
    void* dst = static_cast<uint8_t*>(descriptorBufferMapped_) + (handle.index * alignedBlockSize_);
    
    vkGetDescriptorEXT(device_->getLogicalDevice(), &getInfo, props.combinedImageSamplerDescriptorSize, dst);

    return handle;
}

void MaterialSystem::DestroyMaterial(ecs::Handle handle) {
    materials_.Remove(handle);
}

MaterialAsset* MaterialSystem::GetMaterial(ecs::Handle handle) {
    return materials_.Get(handle);
}

} // namespace render