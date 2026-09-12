#pragma once

#include "AssetTypes.h"
#include "ze/ecs/GenerationalTable.h"
#include "ze/render/Device.h"
#include "ze/render/StagingRingBuffer.h"
#include <filesystem>
#include <string>
#include <functional>

namespace render {

class AssetManager {
public:
    void Initialize(Device* device);
    void Destroy();

    ecs::Handle LoadMesh(const std::filesystem::path& path);
    ecs::Handle LoadTexture(const std::filesystem::path& path);

    MeshAsset* GetMesh(ecs::Handle handle);
    TextureAsset* GetTexture(ecs::Handle handle);

    static std::filesystem::path ResolveAssetPath(const std::filesystem::path& path);

private:
    Device* device_ = nullptr;
    
    ecs::GenerationalTable<MeshAsset> meshes_;
    ecs::GenerationalTable<TextureAsset> textures_;
    
    ecs::Handle fallbackTextureHandle_{0xFFFFFFFF, 0};
    
    StagingRingBuffer stagingRingBuffer_;
    VkCommandPool uploadCommandPool_ = VK_NULL_HANDLE;
    VkFence uploadFence_ = VK_NULL_HANDLE;
    
    void ExecuteStagingUpload(size_t size, const void* data, std::function<void(VkCommandBuffer, VkBuffer, size_t offset)> recordCmd);
};

} // namespace render
