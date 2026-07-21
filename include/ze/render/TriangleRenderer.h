#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>
#include <vector>
#include "AssetTypes.h"
#include "core/ThreadArena.h"   // BumpArena (frameArena_)
#include "ecs/GenerationalTable.h"  // ecs::Handle

namespace ecs { class ECSContext; }
namespace render {
class Device;
class MeshAsset;
class TriangleRenderer {
public:
    void init(Device* device, VkFormat colorFormat);
    void cleanup(Device* device);
    
    // Add cull function for compute pass
    void cull(VkCommandBuffer cmd, uint32_t imageIndex, VkImageView currentDepthView, Device* device);
    void draw(VkCommandBuffer cmd, uint32_t imageIndex, class MaterialSystem* materialSystem);
    
    // Slice 0a: dev-test mesh (first LoadMesh) rendered as a gated cube in --dev.
    void setDevTestMesh(ecs::Handle h) { devTestMeshHandle_ = h; }

    // [M1:EXIT-1] ECS->render bridge: bind the entity registry + asset manager so
    // draw() can traverse view<Transform, MeshComponent>. Paired with Engine's setScene.
    void setScene(ecs::ECSContext* ecsCtx, class AssetManager* assetManager);

    // [M2.6 Phase 2] Inject the debug fly-camera view + camera position for the
    // dev cube path. When set, draw() uses it instead of the hardcoded lookAt.
    void setDevView(const glm::mat4& view, const glm::dvec3& cameraPos) {
        devView_ = view; devCamPos_ = cameraPos; devViewSet_ = true;
    }
    
    // For logging reduction
    void readbackCount(Device* device, uint32_t imageIndex);
    
private:
    void createBuffers(Device* device);
    void createPipelines(Device* device, VkFormat colorFormat);
    void createDescriptorSets(Device* device);

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipeline wireframePipeline = VK_NULL_HANDLE; // [M1-EXT-04] Derivative variant
    VkPipeline meshPipeline = VK_NULL_HANDLE;       // Slice 0a: dev-test cube (pos+normal, 32B stride)
    
    // Slice 0a: dev-test mesh HANDLE (first LoadMesh), gated to --dev rendering.
    // Stored as a handle, resolved via GetMesh() at point of use — NOT a cached
    // raw MeshAsset*, because GenerationalTable::Insert can reallocate the
    // backing std::vector<MeshAsset>, invalidating any cached pointer (UAF).
    ecs::Handle devTestMeshHandle_ = ecs::Handle{0xFFFFFFFF, 0xFFFFFFFF};

    // [M1:EXIT-1] ECS->render bridge bindings (set via setScene).
    ecs::ECSContext* ecsCtx_ = nullptr;
    AssetManager*   sceneAssets_ = nullptr;
    core::BumpArena frameArena_{};   // per-frame scratch (64KB), Reset-only

    // [M2.6 Phase 2] Injected dev fly-camera view (set via setDevView).
    glm::mat4  devView_{1.0f};
    glm::dvec3 devCamPos_{0.0};
    bool       devViewSet_ = false;
    
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
