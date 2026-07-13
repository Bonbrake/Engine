#pragma once

#include <SDL3/SDL.h>
#include <volk.h>
#include <VkBootstrap.h>
#include <glm/glm.hpp>
#include <memory>
#include "ecs/GenerationalTable.h"

namespace debug { class ImGuiOverlay; }

namespace render {

class Device;
class Swapchain;
class AssetManager;
class MaterialSystem;

class VulkanContext {
public:
    VulkanContext(SDL_Window* window);
    ~VulkanContext();

    void renderFrame(debug::ImGuiOverlay* imguiOverlay);

    // [M2.6 Phase 2] Forward the debug fly-camera view into the triangle renderer.
    void setDevView(const glm::mat4& view, const glm::dvec3& cameraPos);

    vkb::Instance getInstance() const { return vkbInstance_; }
    VkSurfaceKHR getSurface() const { return surface_; }
    Device* getDevice() const { return device_.get(); }

private:
    void initVulkan(SDL_Window* window);
    void cleanup();

    vkb::Instance vkbInstance_;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    SDL_Window* window_ = nullptr;

    std::unique_ptr<Device> device_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<AssetManager> assetManager_;
    std::unique_ptr<MaterialSystem> materialSystem_;

    // Dev-test mesh handle: first-ever LoadMesh invocation (Slice-0a A1 precondition).
    ecs::Handle devTestMeshHandle_{0xFFFFFFFF, 0};

    // Frame-dump: counts executed windowed render calls; matched against Config::dumpFrameAt.
    uint64_t renderCallCount_ = 0;
};

} // namespace render
