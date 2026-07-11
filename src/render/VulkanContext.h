#pragma once

#include <SDL3/SDL.h>
#include <volk.h>
#include <VkBootstrap.h>
#include <memory>

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
};

} // namespace render
