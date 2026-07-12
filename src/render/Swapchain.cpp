#include "Swapchain.h"
#include "Device.h"
#include "../core/Logger.h"
#include "../core/JobSystem.h"
#include "../debug/ImGuiOverlay.h"

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

void Swapchain::create() {
    vkb::SwapchainBuilder swapchainBuilder{device_->getVkbDevice()};
    
    // The vkb swapchain builder handles querying surface formats and capabilities.
    auto vkb_swapchain_ret = swapchainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .build();

    if (!vkb_swapchain_ret) {
        LOG_CRITICAL("Failed to create swapchain: {}", vkb_swapchain_ret.error().message());
    } else {
        vkbSwapchain_ = vkb_swapchain_ret.value();
        auto images_ret = vkbSwapchain_.get_images();
        if (images_ret) {
            swapchainImages_ = images_ret.value();
        }
        
        auto views_ret = vkbSwapchain_.get_image_views();
        if (views_ret) {
            swapchainImageViews_ = views_ret.value();
        }

        depthImages_.resize(swapchainImages_.size());
        depthAllocations_.resize(swapchainImages_.size());
        depthImageViews_.resize(swapchainImages_.size());
        passNamesPerFrame_.resize(swapchainImages_.size());

        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = vkbSwapchain_.extent.width;
            imageInfo.extent.height = vkbSwapchain_.extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_D32_SFLOAT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            // Need sampled bit for compute shader to read it, and depth stencil attachment bit to render to it
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

        commandBuffers_.resize(swapchainImages_.size());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool_;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

        vkAllocateCommandBuffers(device_->getLogicalDevice(), &allocInfo, commandBuffers_.data());

        imageAvailableSemaphores_.resize(swapchainImages_.size());
        renderFinishedSemaphores_.resize(swapchainImages_.size());
        
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            vkCreateSemaphore(device_->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
            vkCreateSemaphore(device_->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
        }

        triangleRenderer_.init(device_, vkbSwapchain_.image_format);

        // Initialize CommandPoolMatrix
        commandPoolMatrix_.Initialize(device_->getLogicalDevice(), device_->getGraphicsQueueIndex(), static_cast<uint32_t>(swapchainImages_.size()), core::JobSystem::get()->GetNumTaskThreads());
    }
}

void Swapchain::cleanup() {
    commandPoolMatrix_.Destroy();
    
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
        return; // Cannot recreate swapchain with 0 extent
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
            return; // Skip rendering when minimized or zero extent
        }
    }

    // Frame pacing wait on timeline semaphore (3 frames in flight max)
    framePacing_.EvaluateQueuePacingIntercept(device_->getLogicalDevice(), frameTimelineSemaphore_, frameTimelineValue_, 3);
    
    // Readback the count from the previous frame's slot (GPU has finished writing it by now)
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

    frameTimelineValue_++;

    uint32_t framesInFlight = (uint32_t)swapchainImages_.size();

    if (device_->getCapabilities().queryTimestamps) {
        uint32_t prevFrame = (imageIndex + framesInFlight - 1) % framesInFlight;
        const auto& prevNames = passNamesPerFrame_[prevFrame];
        size_t passCount = prevNames.size();
        if (passCount > 0) {
            // Readback buffer sizing: (MAX_PASSES * 2) queries, each writes 2 uint64_t values (value + availability)
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
    
    if (device_->getCapabilities().queryTimestamps) {
        vkCmdResetQueryPool(cmd, device_->getQueryPool(), imageIndex * render::RenderGraph::MAX_PASSES * 2, render::RenderGraph::MAX_PASSES * 2);
    }

    renderGraph_.Clear();
    passNamesPerFrame_[imageIndex].clear();

    render::PassNode cullPass;
    cullPass.name = "Compute Culling Pass";
    
    // We sample the previous frame's depth buffer, so we need it transitioned to READ_ONLY
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

    render::PassNode swapchainPass;
    swapchainPass.name = "Swapchain Main Pass";
    
    render::PassAttachment colorAttachment{};
    colorAttachment.view = swapchainImageViews_[imageIndex];
    colorAttachment.format = vkbSwapchain_.image_format;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.1f, 0.1f, 0.1f, 1.0f} };
    swapchainPass.colorAttachments.push_back(colorAttachment);

    render::PassAttachment depthAttachment{};
    depthAttachment.view = depthImageViews_[imageIndex];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT; // Or queried depth format
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
    swapchainPass.depthAttachment = depthAttachment;

    swapchainPass.renderArea.extent = vkbSwapchain_.extent;

    render::PassDependency writeDep{};
    writeDep.image = swapchainImages_[imageIndex];
    writeDep.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    writeDep.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    writeDep.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    swapchainPass.writes.push_back(writeDep);

    render::PassDependency depthWriteDep{};
    depthWriteDep.image = depthImages_[imageIndex];
    depthWriteDep.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    depthWriteDep.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthWriteDep.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthWriteDep.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    swapchainPass.writes.push_back(depthWriteDep);

    swapchainPass.executeCallback = [this, imageIndex, imguiOverlay, materialSystem](VkCommandBuffer cmd) {
        triangleRenderer_.draw(cmd, imageIndex, materialSystem);
        // Draw calls go here
        if (imguiOverlay) {
            imguiOverlay->Render(cmd);
        }
    };

    renderGraph_.AddPass(std::move(swapchainPass));
    renderGraph_.CompileAndExecute(cmd, device_, &commandPoolMatrix_, imageIndex, device_->getCapabilities().queryTimestamps ? device_->getQueryPool() : VK_NULL_HANDLE, imageIndex * render::RenderGraph::MAX_PASSES * 2, &passNamesPerFrame_[imageIndex]);

    if (imguiOverlay) {
        imguiOverlay->SetPassTimings(lastFrameTimings_, device_->getCapabilities().queryTimestamps);
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
    uint64_t signalValues[] = {0, frameTimelineValue_}; // binary semaphore takes 0, timeline takes frameTimelineValue_
    
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
    currentFrame_ = (currentFrame_ + 1) % swapchainImages_.size();
}

} // namespace render
