#include "ze/render/Swapchain.h"
#include "ze/render/Device.h"
#include "ze/core/Logger.h"
#include "ze/core/JobSystem.h"
#include "ze/core/Config.h"
#include "ze/render/PipelineBuilder.h"
#include "ze/debug/ImGuiOverlay.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vk_mem_alloc.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iterator>

namespace render {

Swapchain::Swapchain(Device* device, SDL_Window* window) 
    : device_(device), window_(window) {
    create();
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphoreTypeCreateInfo timelineCreateInfo{};
    timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineCreateInfo.initialValue = 0;

    VkSemaphoreCreateInfo timelineInfo{};
    timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    timelineInfo.pNext = &timelineCreateInfo;

    vkCreateSemaphore(device_->getLogicalDevice(), &timelineInfo, nullptr, &frameTimelineSemaphore_);
}

Swapchain::~Swapchain() {
    vkDestroySemaphore(device_->getLogicalDevice(), frameTimelineSemaphore_, nullptr);
    cleanup();
}

static std::vector<uint32_t> readSpvFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    size_t sz = f.tellg();
    f.seekg(0);
    std::vector<uint32_t> data(sz / 4);
    f.read(reinterpret_cast<char*>(data.data()), sz);
    return data;
}

void Swapchain::create() {
    vkb::SwapchainBuilder swapchainBuilder{device_->getVkbDevice()};
    
    // Query the actual surface format first; fall back to R8G8B8A8_UNORM
    // for RTSS/overlay workflows that inject STORAGE_BIT.
    VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    uint32_t formatCount = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device_->getPhysicalDevice(), device_->getSurface(), &formatCount, nullptr) == VK_SUCCESS && formatCount > 0) {
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(device_->getPhysicalDevice(), device_->getSurface(), &formatCount, formats.data()) == VK_SUCCESS) {
            surfaceFormat = formats[0];
        }
    }
    
    auto vkb_swapchain_ret = swapchainBuilder
        .set_desired_format({surfaceFormat.format, surfaceFormat.colorSpace})
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        // Explicit usage: color attachment (AgX tonemap + ImGui) + transfer for readback/dump.
        // Include STORAGE_BIT explicitly: RTSS.exe (RivaTuner overlay) injects this flag
        // into the swapchain, and R8G8B8A8_UNORM supports it (unlike B8G8R8A8_SRGB).
        // Being explicit avoids VUID-VkSwapchainCreateInfoKHR-imageFormat-01778 validation errors.
        .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                               VK_IMAGE_USAGE_STORAGE_BIT)
        .build();

    if (!vkb_swapchain_ret) {
        LOG_CRITICAL("Failed to create swapchain: {}", vkb_swapchain_ret.error().message());
    } else {
        vkbSwapchain_ = vkb_swapchain_ret.value();
        
        // [M4.5-EXT-33] Pull swapchain images + views via vkb API (required pattern)
        auto images_ret = vkbSwapchain_.get_images();
        if (images_ret) {
            swapchainImages_ = images_ret.value();
        }
        
        auto views_ret = vkbSwapchain_.get_image_views();
        if (views_ret) {
            swapchainImageViews_ = views_ret.value();
        }

        const uint32_t imageCount = static_cast<uint32_t>(swapchainImages_.size());
        depthImages_.resize(imageCount);
        depthAllocations_.resize(imageCount);
        depthImageViews_.resize(imageCount);
        commandBuffers_.resize(imageCount);
        passNamesPerFrame_.resize(imageCount);

        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = {vkbSwapchain_.extent.width, vkbSwapchain_.extent.height, 1};
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_D32_SFLOAT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            vmaCreateImage(device_->getAllocator(), &imageInfo, &allocInfo, &depthImages_[i], &depthAllocations_[i], nullptr);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages_[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_D32_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(device_->getLogicalDevice(), &viewInfo, nullptr, &depthImageViews_[i]);
        }

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device_->getGraphicsQueueIndex();
        
        vkCreateCommandPool(device_->getLogicalDevice(), &poolInfo, nullptr, &commandPool_);

        commandBuffers_.resize(imageCount);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool_;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = imageCount;

        vkAllocateCommandBuffers(device_->getLogicalDevice(), &allocInfo, commandBuffers_.data());

        imageAvailableSemaphores_.resize(imageCount);
        renderFinishedSemaphores_.resize(imageCount);
        
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            vkCreateSemaphore(device_->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
            vkCreateSemaphore(device_->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
        }

        // [M4.5-EXT-33] AgX HDR targets
        hdrFormat_ = VK_FORMAT_R16G16B16A16_SFLOAT;
        hdrFrames_.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            VkImageCreateInfo imgInfo{};
            imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imgInfo.imageType = VK_IMAGE_TYPE_2D;
            imgInfo.format = hdrFormat_;
            imgInfo.extent = {vkbSwapchain_.extent.width, vkbSwapchain_.extent.height, 1};
            imgInfo.mipLevels = 1;
            imgInfo.arrayLayers = 1;
            imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imgInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VmaAllocationCreateInfo vmaInfo{};
            vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(device_->getAllocator(), &imgInfo, &vmaInfo, &hdrFrames_[i].image, &hdrFrames_[i].allocation, nullptr);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = hdrFrames_[i].image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = hdrFormat_;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(device_->getLogicalDevice(), &viewInfo, nullptr, &hdrFrames_[i].view);
        }

        createTonemapResources();

        triangleRenderer_.init(device_, hdrFormat_);

        // Initialize CommandPoolMatrix
        commandPoolMatrix_.Initialize(device_->getLogicalDevice(), device_->getGraphicsQueueIndex(), 
            static_cast<uint32_t>(swapchainImages_.size()), core::JobSystem::get()->GetNumTaskThreads());
    }
}

void Swapchain::createTonemapResources() {
    const auto& props = device_->getDescriptorBufferProperties();
    const float exposure = core::Config::get().exposure;

    // Sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(device_->getLogicalDevice(), &samplerInfo, nullptr, &tonemapSampler_);

    // Exposure UBO
    VkBufferCreateInfo uboInfo{};
    uboInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uboInfo.size = sizeof(float);
    uboInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VmaAllocationCreateInfo uboAlloc{};
    uboAlloc.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    uboAlloc.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vmaCreateBuffer(device_->getAllocator(), &uboInfo, &uboAlloc, &exposureUBO_, &exposureAlloc_, nullptr);
    VmaAllocationInfo uboAllocInfo;
    vmaGetAllocationInfo(device_->getAllocator(), exposureAlloc_, &uboAllocInfo);
    exposureMapped_ = uboAllocInfo.pMappedData;
    *static_cast<float*>(exposureMapped_) = exposure;

    // Pipeline layout via PipelineBuilder
    auto vertCode = readSpvFile("build/shaders/tonemap.vert.spv");
    auto fragCode = readSpvFile("build/shaders/tonemap.frag.spv");
    if (!vertCode.empty() && !fragCode.empty()) {
        auto layoutData = PipelineBuilder::buildLayouts({vertCode, fragCode}, device_);
        tonemapSetLayouts_ = std::move(layoutData.setLayouts);
        tonemapPipelineLayout_ = layoutData.pipelineLayout;

        // Descriptor buffer: one set per swapchain image
        VkDeviceSize alignment = props.descriptorBufferOffsetAlignment;
        VkDeviceSize samplerSize = props.combinedImageSamplerDescriptorSize;
        VkDeviceSize uboSize = props.uniformBufferDescriptorSize;
        VkDeviceSize blockSize = samplerSize + uboSize;
        VkDeviceSize alignedBlockSize = (blockSize + alignment - 1) & ~(alignment - 1);
        VkDeviceSize bufferSize = alignedBlockSize * hdrFrames_.size();

        VkBufferCreateInfo dbInfo{};
        dbInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        dbInfo.size = bufferSize;
        dbInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | 
                       VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | 
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VmaAllocationCreateInfo dbAlloc{};
        dbAlloc.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        dbAlloc.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vmaCreateBuffer(device_->getAllocator(), &dbInfo, &dbAlloc, &tonemapDescBuffer_, &tonemapDescAlloc_, nullptr);
        VmaAllocationInfo dbAllocInfo;
        vmaGetAllocationInfo(device_->getAllocator(), tonemapDescAlloc_, &dbAllocInfo);
        tonemapDescMapped_ = dbAllocInfo.pMappedData;
        
        VkBufferDeviceAddressInfoEXT addrInfo{};
        addrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT;
        addrInfo.buffer = tonemapDescBuffer_;
        tonemapDescAddress_ = vkGetBufferDeviceAddress(device_->getLogicalDevice(), &addrInfo);

        // Write descriptors for each image
        for (size_t i = 0; i < hdrFrames_.size(); i++) {
            // Sampler (binding 0)
            VkDescriptorImageInfo imgInfo{};
            imgInfo.sampler = tonemapSampler_;
            imgInfo.imageView = hdrFrames_[i].view;
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorGetInfoEXT getInfo{};
            getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
            getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            getInfo.data.pCombinedImageSampler = &imgInfo;

            VkDeviceSize bindingOffset = 0;
            vkGetDescriptorSetLayoutBindingOffsetEXT(device_->getLogicalDevice(), tonemapSetLayouts_[0], 0, &bindingOffset);
            void* dst = static_cast<uint8_t*>(tonemapDescMapped_) + i * alignedBlockSize + bindingOffset;
            vkGetDescriptorEXT(device_->getLogicalDevice(), &getInfo, samplerSize, dst);

            // UBO (binding 1)
            VkDescriptorAddressInfoEXT uboAddrInfo{};
            uboAddrInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
            VkBufferDeviceAddressInfoEXT uboAddr{};
            uboAddr.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT;
            uboAddr.buffer = exposureUBO_;
            uboAddrInfo.address = vkGetBufferDeviceAddress(device_->getLogicalDevice(), &uboAddr);
            uboAddrInfo.range = sizeof(float);
            uboAddrInfo.format = VK_FORMAT_UNDEFINED;

            getInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            getInfo.data.pUniformBuffer = &uboAddrInfo;

            bindingOffset = 0;
            vkGetDescriptorSetLayoutBindingOffsetEXT(device_->getLogicalDevice(), tonemapSetLayouts_[0], 1, &bindingOffset);
            dst = static_cast<uint8_t*>(tonemapDescMapped_) + i * alignedBlockSize + bindingOffset;
            vkGetDescriptorEXT(device_->getLogicalDevice(), &getInfo, uboSize, dst);
        }

        tonemapDescOffset_ = alignedBlockSize;

        // Build graphics pipeline (fullscreen triangle)
        VkShaderModule vertModule = VK_NULL_HANDLE, fragModule = VK_NULL_HANDLE;
        VkShaderModuleCreateInfo vertCreate{};
        vertCreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertCreate.codeSize = vertCode.size() * sizeof(uint32_t);
        vertCreate.pCode = vertCode.data();
        vkCreateShaderModule(device_->getLogicalDevice(), &vertCreate, nullptr, &vertModule);

        VkShaderModuleCreateInfo fragCreate{};
        fragCreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragCreate.codeSize = fragCode.size() * sizeof(uint32_t);
        fragCreate.pCode = fragCode.data();
        vkCreateShaderModule(device_->getLogicalDevice(), &fragCreate, nullptr, &fragModule);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = vertModule;
        vertStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = fragModule;
        fragStage.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

        VkPipelineVertexInputStateCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vi.vertexBindingDescriptionCount = 0;
        vi.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo ia{};
        ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo vp{};
        vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vp.viewportCount = 1;
        vp.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rs{};
        rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_NONE;
        rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rs.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{};
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState cbAttach{};
        cbAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo cb{};
        cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cb.attachmentCount = 1;
        cb.pAttachments = &cbAttach;

        VkPipelineDepthStencilStateCreateInfo ds{};
        ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds.depthTestEnable = VK_FALSE;
        ds.depthWriteEnable = VK_FALSE;

        std::vector<VkDynamicState> dynStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn{};
        dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dyn.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
        dyn.pDynamicStates = dynStates.data();

        VkPipelineRenderingCreateInfoKHR renderInfo{};
        renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        VkFormat sdrFormat = vkbSwapchain_.image_format;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachmentFormats = &sdrFormat;
        renderInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

        VkGraphicsPipelineCreateInfo gpInfo{};
        gpInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        gpInfo.pNext = &renderInfo;
        // [M4.5-EXT-33] This pipeline binds descriptors via the descriptor buffer,
        // so it MUST be created with the descriptor-buffer flag (VUID 08600/08117).
        if (device_->getCapabilities().descriptorBuffer) {
            gpInfo.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        }
        gpInfo.stageCount = 2;
        gpInfo.pStages = stages;
        gpInfo.pVertexInputState = &vi;
        gpInfo.pInputAssemblyState = &ia;
        gpInfo.pViewportState = &vp;
        gpInfo.pRasterizationState = &rs;
        gpInfo.pMultisampleState = &ms;
        gpInfo.pColorBlendState = &cb;
        gpInfo.pDepthStencilState = &ds;
        gpInfo.pDynamicState = &dyn;
        gpInfo.layout = tonemapPipelineLayout_;

        vkCreateGraphicsPipelines(device_->getLogicalDevice(), VK_NULL_HANDLE, 1, &gpInfo, nullptr, &tonemapPipeline_);

        vkDestroyShaderModule(device_->getLogicalDevice(), vertModule, nullptr);
        vkDestroyShaderModule(device_->getLogicalDevice(), fragModule, nullptr);
    }
}

void Swapchain::cleanup() {
    commandPoolMatrix_.Destroy();

    // [M4.5-EXT-33] AgX tonemap cleanup
    if (tonemapPipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_->getLogicalDevice(), tonemapPipeline_, nullptr);
        tonemapPipeline_ = VK_NULL_HANDLE;
    }
    if (tonemapPipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_->getLogicalDevice(), tonemapPipelineLayout_, nullptr);
        tonemapPipelineLayout_ = VK_NULL_HANDLE;
    }
    for (auto& layout : tonemapSetLayouts_) {
        if (layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_->getLogicalDevice(), layout, nullptr);
        }
    }
    tonemapSetLayouts_.clear();
    
    if (tonemapDescBuffer_ != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_->getAllocator(), tonemapDescBuffer_, tonemapDescAlloc_);
        tonemapDescBuffer_ = VK_NULL_HANDLE;
        tonemapDescAlloc_ = VK_NULL_HANDLE;
        tonemapDescMapped_ = nullptr;
        tonemapDescAddress_ = 0;
        tonemapDescOffset_ = 0;
    }
    if (exposureUBO_ != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_->getAllocator(), exposureUBO_, exposureAlloc_);
        exposureUBO_ = VK_NULL_HANDLE;
        exposureAlloc_ = VK_NULL_HANDLE;
        exposureMapped_ = nullptr;
    }
    if (tonemapSampler_ != VK_NULL_HANDLE) {
        vkDestroySampler(device_->getLogicalDevice(), tonemapSampler_, nullptr);
        tonemapSampler_ = VK_NULL_HANDLE;
    }
    for (auto& hdr : hdrFrames_) {
        if (hdr.view != VK_NULL_HANDLE) vkDestroyImageView(device_->getLogicalDevice(), hdr.view, nullptr);
        if (hdr.image != VK_NULL_HANDLE) vmaDestroyImage(device_->getAllocator(), hdr.image, hdr.allocation);
    }
    hdrFrames_.clear();

    if (commandPool_) {
        vkDestroyCommandPool(device_->getLogicalDevice(), commandPool_, nullptr);
        commandPool_ = VK_NULL_HANDLE;
    }
    if (vkbSwapchain_.swapchain != VK_NULL_HANDLE) {
        triangleRenderer_.cleanup(device_);
        vkbSwapchain_.destroy_image_views(swapchainImageViews_);
        vkb::destroy_swapchain(vkbSwapchain_);
        
        for (size_t i = 0; i < depthImages_.size(); i++) {
            vkDestroyImageView(device_->getLogicalDevice(), depthImageViews_[i], nullptr);
            vmaDestroyImage(device_->getAllocator(), depthImages_[i], depthAllocations_[i]);
            
            vkDestroySemaphore(device_->getLogicalDevice(), imageAvailableSemaphores_[i], nullptr);
            vkDestroySemaphore(device_->getLogicalDevice(), renderFinishedSemaphores_[i], nullptr);
        }
        depthImages_.clear();
        depthImageViews_.clear();
        depthAllocations_.clear();
    }
}

void Swapchain::recreate() {
    int width = 0, height = 0;
    SDL_GetWindowSizeInPixels(window_, &width, &height);
    if (width == 0 || height == 0) {
        return;
    }

    device_->waitIdle();
    cleanup();
    create();
}

void Swapchain::acquireAndPresent(debug::ImGuiOverlay* imguiOverlay, MaterialSystem* materialSystem) {
    if (window_) {
        int width = 0, height = 0;
        SDL_GetWindowSizeInPixels(window_, &width, &height);
        if (width == 0 || height == 0) {
            return;
        }
    }

    triangleRenderer_.readbackCount(device_, lastImageIndex_);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device_->getLogicalDevice(), vkbSwapchain_.swapchain, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate();
        return;
    } else if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to acquire swapchain image!");
        return;
    }

    // [M0-EXT-09] Frame pacing: wait on previously signaled timeline value.
    // This limits GPU latency to maxFramesInFlight before allowing the next frame to progress.
    framePacing_.EvaluateQueuePacingIntercept(device_->getLogicalDevice(), frameTimelineSemaphore_, frameTimelineValue_, 3);

    frameTimelineValue_++;

    uint32_t framesInFlight = static_cast<uint32_t>(swapchainImages_.size());

    if (device_->getCapabilities().queryTimestamps) {
        uint32_t prevFrame = (imageIndex + framesInFlight - 1) % framesInFlight;
        const auto& prevNames = passNamesPerFrame_[prevFrame];
        size_t passCount = prevNames.size();
        if (passCount > 0) {
            std::vector<uint64_t> queryData(render::RenderGraph::MAX_PASSES * 4, 0);

            VkResult res = vkGetQueryPoolResults(
                device_->getLogicalDevice(), device_->getQueryPool(),
                prevFrame * render::RenderGraph::MAX_PASSES * 2, static_cast<uint32_t>(passCount * 2),
                queryData.size() * sizeof(uint64_t), queryData.data(),
                2 * sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);

            if (res == VK_SUCCESS) {
                lastFrameTimings_.clear();
                for (size_t p = 0; p < passCount; ++p) {
                    uint64_t startAvail = queryData[p * 4 + 1];
                    uint64_t endAvail   = queryData[p * 4 + 3];
                    if (startAvail != 0 && endAvail != 0) {
                        uint64_t startVal = queryData[p * 4 + 0];
                        uint64_t endVal   = queryData[p * 4 + 2];
                        double ms = (endVal - startVal) * device_->getTimestampPeriod() * 1e-6;
                        lastFrameTimings_.push_back(std::make_pair(prevNames[p], static_cast<float>(ms)));
                    }
                }
            }
        }
    }

    VkCommandBuffer cmd = commandBuffers_[imageIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    // [M4.5-EXT-33] Reset this frame's timestamp query range.
    // Works because the pool was created with VK_QUERY_POOL_CREATE_RESET_BIT_KHR.
    if (device_->getCapabilities().queryTimestamps) {
        vkCmdResetQueryPool(cmd, device_->getQueryPool(),
            imageIndex * render::RenderGraph::MAX_PASSES * 2,
            render::RenderGraph::MAX_PASSES * 2);
    }

    renderGraph_.Clear();
    passNamesPerFrame_[imageIndex].clear();

    render::PassNode cullPass;
    cullPass.name = "Compute Culling Pass";
    
    render::PassDependency depthReadDep{};
    depthReadDep.image = depthImages_[imageIndex];
    depthReadDep.stageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    depthReadDep.accessMask = VK_ACCESS_2_SHADER_READ_BIT;
    depthReadDep.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthReadDep.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    cullPass.reads.push_back(depthReadDep);

    cullPass.executeCallback = [this, imageIndex](VkCommandBuffer cmd) {
        triangleRenderer_.cull(cmd, imageIndex, depthImageViews_[imageIndex], device_);
    };
    renderGraph_.AddPass(cullPass);

    // [M4.5-EXT-33] Scene pass renders to HDR target (no ImGui here!)
    render::PassNode scenePass;
    scenePass.name = "HDR Scene Pass";

    render::PassAttachment colorAttachment{};
    colorAttachment.view = hdrFrames_[imageIndex].view;
    colorAttachment.format = hdrFormat_;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    scenePass.colorAttachments.push_back(colorAttachment);

    render::PassAttachment depthAttachment{};
    depthAttachment.view = depthImageViews_[imageIndex];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};
    scenePass.depthAttachment = depthAttachment;

    scenePass.renderArea.extent = vkbSwapchain_.extent;

    render::PassDependency writeDep{};
    writeDep.image = hdrFrames_[imageIndex].image;
    writeDep.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    writeDep.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    writeDep.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    scenePass.writes.push_back(writeDep);

    render::PassDependency depthWriteDep{};
    depthWriteDep.image = depthImages_[imageIndex];
    depthWriteDep.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    depthWriteDep.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthWriteDep.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthWriteDep.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    scenePass.writes.push_back(depthWriteDep);

    scenePass.executeCallback = [this, imageIndex, materialSystem](VkCommandBuffer cmd) {
        triangleRenderer_.draw(cmd, imageIndex, materialSystem);
    };

    renderGraph_.AddPass(std::move(scenePass));

    // [M4.5-EXT-33] Tonemap pass: HDR -> SDR swapchain (ImGui renders here only)
    if (tonemapPipeline_ != VK_NULL_HANDLE) {
        render::PassNode tonemapPass;
        tonemapPass.name = "AgX Tonemap Pass";

        render::PassAttachment tonemapColor{};
        tonemapColor.view = swapchainImageViews_[imageIndex];
        tonemapColor.format = vkbSwapchain_.image_format;
        tonemapColor.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        tonemapColor.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        tonemapColor.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        tonemapColor.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        tonemapPass.colorAttachments.push_back(tonemapColor);

        tonemapPass.renderArea.extent = vkbSwapchain_.extent;

        render::PassDependency tonemapRead{};
        tonemapRead.image = hdrFrames_[imageIndex].image;
        tonemapRead.stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        tonemapRead.accessMask = VK_ACCESS_2_SHADER_READ_BIT;
        tonemapRead.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        tonemapRead.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        tonemapPass.reads.push_back(tonemapRead);

        render::PassDependency tonemapWrite{};
        tonemapWrite.image = swapchainImages_[imageIndex];
        tonemapWrite.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        tonemapWrite.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        tonemapWrite.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        tonemapPass.writes.push_back(tonemapWrite);

        tonemapPass.executeCallback = [this, imageIndex, imguiOverlay](VkCommandBuffer cmd) {
            *static_cast<float*>(exposureMapped_) = core::Config::get().exposure;

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonemapPipeline_);

            VkDescriptorBufferBindingInfoEXT bindingInfo{};
            bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
            bindingInfo.address = tonemapDescAddress_;
            bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
            vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);

            uint32_t bufferIndex = 0;
            VkDeviceSize offset = imageIndex * tonemapDescOffset_;
            vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonemapPipelineLayout_, 0, 1, &bufferIndex, &offset);

            VkViewport viewport{};
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = static_cast<float>(vkbSwapchain_.extent.width);
            viewport.height = static_cast<float>(vkbSwapchain_.extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = vkbSwapchain_.extent;
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            vkCmdDraw(cmd, 3, 1, 0, 0);

            if (imguiOverlay) {
                imguiOverlay->Render(cmd);
            }
        };

        renderGraph_.AddPass(std::move(tonemapPass));
    }

    renderGraph_.CompileAndExecute(cmd, device_, &commandPoolMatrix_, imageIndex, 
        device_->getCapabilities().queryTimestamps ? device_->getQueryPool() : VK_NULL_HANDLE, 
        imageIndex * render::RenderGraph::MAX_PASSES * 2, &passNamesPerFrame_[imageIndex]);

    if (imguiOverlay) {
        imguiOverlay->SetPassTimings(lastFrameTimings_, device_->getCapabilities().queryTimestamps);
    }

    VkBuffer dumpBuffer = VK_NULL_HANDLE;
    VmaAllocation dumpAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo dumpAllocInfo{};
    const uint32_t dumpW = vkbSwapchain_.extent.width;
    const uint32_t dumpH = vkbSwapchain_.extent.height;
    const bool doDump = !pendingDumpPath_.empty();
    if (doDump) {
        VkDeviceSize dumpSize = static_cast<VkDeviceSize>(dumpW) * dumpH * 4;
        VkBufferCreateInfo bufInfo{};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.size = dumpSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if (vmaCreateBuffer(device_->getAllocator(), &bufInfo, &allocInfo, &dumpBuffer, &dumpAlloc, &dumpAllocInfo) != VK_SUCCESS) {
            LOG_ERROR("Frame-dump: failed to allocate readback buffer ({}x{})", dumpW, dumpH);
            dumpBuffer = VK_NULL_HANDLE;
        } else {
            VkImageMemoryBarrier2 toSrc{};
            toSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            toSrc.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            toSrc.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            toSrc.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            toSrc.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            toSrc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            toSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            toSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toSrc.image = swapchainImages_[imageIndex];
            toSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            toSrc.subresourceRange.baseMipLevel = 0;
            toSrc.subresourceRange.levelCount = 1;
            toSrc.subresourceRange.baseArrayLayer = 0;
            toSrc.subresourceRange.layerCount = 1;
            VkDependencyInfo depA{};
            depA.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depA.imageMemoryBarrierCount = 1;
            depA.pImageMemoryBarriers = &toSrc;
            vkCmdPipelineBarrier2(cmd, &depA);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {dumpW, dumpH, 1};
            vkCmdCopyImageToBuffer(cmd, swapchainImages_[imageIndex],
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dumpBuffer, 1, &region);

            VkImageMemoryBarrier2 backToColor{};
            backToColor.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            backToColor.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            backToColor.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            backToColor.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            backToColor.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            backToColor.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            backToColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            backToColor.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            backToColor.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            backToColor.image = swapchainImages_[imageIndex];
            backToColor.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            backToColor.subresourceRange.baseMipLevel = 0;
            backToColor.subresourceRange.levelCount = 1;
            backToColor.subresourceRange.baseArrayLayer = 0;
            backToColor.subresourceRange.layerCount = 1;
            VkDependencyInfo depB{};
            depB.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depB.imageMemoryBarrierCount = 1;
            depB.pImageMemoryBarriers = &backToColor;
            vkCmdPipelineBarrier2(cmd, &depB);
        }
    }

    VkImageMemoryBarrier2 barrierToPresent{};
    barrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrierToPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrierToPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrierToPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrierToPresent.dstAccessMask = 0;
    barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrierToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToPresent.image = swapchainImages_[imageIndex];
    barrierToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierToPresent.subresourceRange.baseMipLevel = 0;
    barrierToPresent.subresourceRange.levelCount = 1;
    barrierToPresent.subresourceRange.baseArrayLayer = 0;
    barrierToPresent.subresourceRange.layerCount = 1;

    VkDependencyInfo depInfoToPresent{};
    depInfoToPresent.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfoToPresent.imageMemoryBarrierCount = 1;
    depInfoToPresent.pImageMemoryBarriers = &barrierToPresent;

    vkCmdPipelineBarrier2(cmd, &depInfoToPresent);
    
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_], frameTimelineSemaphore_};
    uint64_t signalValues[] = {0, frameTimelineValue_};
    
    VkTimelineSemaphoreSubmitInfo timelineInfo{};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.signalSemaphoreValueCount = 2;
    timelineInfo.pSignalSemaphoreValues = signalValues;
    
    submitInfo.pNext = &timelineInfo;
    submitInfo.signalSemaphoreCount = 2;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(device_->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        LOG_ERROR("Failed to submit draw command buffer!");
    }

    if (doDump && dumpBuffer != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_->getLogicalDevice());
        vmaInvalidateAllocation(device_->getAllocator(), dumpAlloc, 0, VK_WHOLE_SIZE);
        auto* pixels = static_cast<uint8_t*>(dumpAllocInfo.pMappedData);
        if (pixels) {
            const size_t pxCount = static_cast<size_t>(dumpW) * dumpH;
            for (size_t i = 0; i < pxCount; ++i) {
                std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]);
            }
            int ok = stbi_write_png(pendingDumpPath_.c_str(), static_cast<int>(dumpW), static_cast<int>(dumpH), 4, pixels, static_cast<int>(dumpW) * 4);
            if (ok) {
                LOG_INFO("Frame-dump: wrote {} ({}x{})", pendingDumpPath_, dumpW, dumpH);
            } else {
                LOG_ERROR("Frame-dump: stbi_write_png failed for {}", pendingDumpPath_);
            }
        }
        vmaDestroyBuffer(device_->getAllocator(), dumpBuffer, dumpAlloc);
        pendingDumpPath_.clear();
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[] = {vkbSwapchain_.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(device_->getGraphicsQueue(), &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate();
    } else if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to present swapchain image!");
    }
    
    lastImageIndex_ = imageIndex;
    currentFrame_ = (currentFrame_ + 1) % static_cast<uint32_t>(swapchainImages_.size());
}

} // namespace render