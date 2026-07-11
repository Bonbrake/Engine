#include "ImGuiOverlay.h"
#include "../core/Logger.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <windows.h>
#include <stdio.h>
#include "../core/Platform.h"
#include "../core/CVarSystem.h"

bool CallImGuiInit(ImGui_ImplVulkan_InitInfo* info) {
    __try {
        return ImGui_ImplVulkan_Init(info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        printf("SEH Exception caught inside ImGui_ImplVulkan_Init! Code: %X\n", code);
        fflush(stdout);
        return false;
    }
}

namespace debug {

void ImGuiOverlay::Initialize(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, uint32_t queueFamily, SDL_Window* window) {
    // 1: create descriptor pool for IMGUI
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;
    
    vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);

    // 2: initialize imgui library
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();

    // 3: init platform/renderer bindings
    ImGui_ImplSDL3_InitForVulkan(window);
    
    // We are using volk natively via IMGUI_IMPL_VULKAN_USE_VOLK
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.QueueFamily = queueFamily;
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool;
    init_info.PipelineInfoMain.RenderPass = VK_NULL_HANDLE; // We use dynamic rendering
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.MinImageCount = 2; // e.g. double buffering
    init_info.ImageCount = 3;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
    
    // Default format for swapchain is usually B8G8R8A8_UNORM, but we should pass it properly
    // Using an arbitrary format here as placeholder; in a real app, pass the actual swapchain format.
    static VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM; 
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    
    auto func1 = vkGetDeviceProcAddr(device, "vkCmdBeginRendering");
    auto func2 = vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
    LOG_INFO("vkGetDeviceProcAddr: vkCmdBeginRendering = {}, KHR = {}", (void*)func1, (void*)func2);
    
    LOG_INFO("ImGui_ImplVulkan_Init starting");
    if (!CallImGuiInit(&init_info)) {
        LOG_CRITICAL("ImGui Init Failed due to SEH Exception!");
        core::Platform::triggerBreakpoint();
    }
    LOG_INFO("ImGui_ImplVulkan_Init finished");

    // Font upload is handled automatically in ImGui 1.90+
}

void ImGuiOverlay::Destroy(VkDevice device) {
    if (ImGui::GetCurrentContext() != nullptr) {
        // Check if ImGui_ImplVulkan_Init was actually called
        ImGuiIO& io = ImGui::GetIO();
        if (io.BackendRendererUserData != nullptr) {
            ImGui_ImplVulkan_Shutdown();
        }
        if (io.BackendPlatformUserData != nullptr) {
            ImGui_ImplSDL3_Shutdown();
        }
        ImGui::DestroyContext();
    }
    
    if (imguiPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, imguiPool, nullptr);
        imguiPool = VK_NULL_HANDLE;
    }
}

void ImGuiOverlay::ProcessEvent(const SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void ImGuiOverlay::NewFrame() {
    LOG_INFO("ImGui NewFrame starting");
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    LOG_INFO("ImGui NewFrame finished");
    
    if (isVisible) {
        ImGui::Begin("Developer Console");
        ImGui::Text("Zombie Engine - Dev Overlay (M1)");
        ImGui::Separator();
        
        core::CVarSystem::Get().DrawImGui();
        
        ImGui::Separator();
        ImGui::Text("GPU Pass Timings");
        if (!timestampsSupported) {
            ImGui::TextDisabled("[GPU Timestamps Unsupported on this device]");
        } else {
            if (passTimings.empty()) {
                ImGui::TextDisabled("Waiting for data...");
            } else {
                for (const auto& timing : passTimings) {
                    ImGui::Text("%s: %.3f ms", timing.first.c_str(), timing.second);
                }
            }
        }
        
        ImGui::End();
    }
}

void ImGuiOverlay::Render(VkCommandBuffer cmd) {
    LOG_INFO("ImGui Render starting");
    ImGui::Render();
    if (isVisible) {
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
    }
    LOG_INFO("ImGui Render finished");
}

} // namespace debug
