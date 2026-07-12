#pragma once

#include <volk.h>
#include <VkBootstrap.h>
#include <SDL3/SDL.h>
#include <vector>
#include "RenderGraph.h"
#include "TriangleRenderer.h"
#include "FramePacing.h"

namespace debug { class ImGuiOverlay; }

namespace render {

class Device;

class Swapchain {
public:
    Swapchain(Device* device, SDL_Window* window);
    ~Swapchain();

    void acquireAndPresent(debug::ImGuiOverlay* imguiOverlay, class MaterialSystem* materialSystem = nullptr);
    void recreate();

private:
    void create();
    void cleanup();

    Device* device_ = nullptr;
    SDL_Window* window_ = nullptr;

    vkb::Swapchain vkbSwapchain_;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkImage> depthImages_;
    std::vector<VmaAllocation> depthAllocations_;
    std::vector<VkImageView> depthImageViews_;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    
    // Timeline semaphore for GPU-CPU synchronization (replacing fence)
    VkSemaphore frameTimelineSemaphore_ = VK_NULL_HANDLE;
    uint64_t frameTimelineValue_ = 0;
    uint32_t lastImageIndex_ = 0; // For readbackCount: index used by the previous frame

    RenderGraph renderGraph_;

    std::vector<std::vector<std::string>> passNamesPerFrame_;
    std::vector<std::pair<std::string, float>> lastFrameTimings_;

    TriangleRenderer triangleRenderer_;
    FramePacing framePacing_;
};

} // namespace render
