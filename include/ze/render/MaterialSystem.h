#pragma once

#include "Device.h"
#include "AssetManager.h"
#include "AssetTypes.h"
#include "ze/ecs/GenerationalTable.h"

namespace render {

class MaterialSystem {
public:
    static constexpr uint32_t MAX_MATERIALS = 4096;

    void Initialize(Device* device);
    void Destroy();

    ecs::Handle CreateMaterial(const MaterialAsset& config, AssetManager* assetManager);
    void DestroyMaterial(ecs::Handle handle);
    
    MaterialAsset* GetMaterial(ecs::Handle handle);

    VkBuffer GetDescriptorBuffer() const { return descriptorBuffer_; }
    VkDeviceAddress GetDescriptorBufferAddress() const { return descriptorBufferAddress_; }
    VkDeviceSize GetAlignedBlockSize() const { return alignedBlockSize_; }

private:
    Device* device_ = nullptr;

    VkSampler defaultSampler_ = VK_NULL_HANDLE;

    VkBuffer descriptorBuffer_ = VK_NULL_HANDLE;
    VmaAllocation descriptorAllocation_ = VK_NULL_HANDLE;
    VkDeviceAddress descriptorBufferAddress_ = 0;
    void* descriptorBufferMapped_ = nullptr;
    
    VkDeviceSize alignedBlockSize_ = 0;

    ecs::GenerationalTable<MaterialAsset> materials_;
};

} // namespace render
