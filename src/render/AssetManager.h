#pragma once

#include "AssetTypes.h"
#include "ze/ecs/GenerationalTable.h"
#include "ze/render/Device.h"
#include <filesystem>
#include <string>

namespace render {

class AssetManager {
public:
    void Initialize(Device* device);
    void Destroy();

    ecs::Handle LoadMesh(const std::filesystem::path& path);
    ecs::Handle LoadTexture(const std::filesystem::path& path);

    MeshAsset* GetMesh(ecs::Handle handle);
    TextureAsset* GetTexture(ecs::Handle handle);

private:
    Device* device_ = nullptr;
    
    ecs::GenerationalTable<MeshAsset> meshes_;
    ecs::GenerationalTable<TextureAsset> textures_;
    
    ecs::Handle fallbackTextureHandle_{0xFFFFFFFF, 0};
    
    void ExecuteOneShotStaging(size_t size, void* data, std::function<void(VkCommandBuffer, VkBuffer)> recordCmd);
};

} // namespace render