#include "ze/render/CommandManager.h"
#include "ze/render/Device.h"

namespace render {

CommandManager::CommandManager(Device* device) : device_(device) {
    // Basic implementation for M0. 
    // The pool-per-thread strategy (L*T + N) would be managed here based on EnkiTS worker count.
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = device_->getGraphicsQueueIndex();

    vkCreateCommandPool(device_->getLogicalDevice(), &poolInfo, nullptr, &graphicsPool_);
}

CommandManager::~CommandManager() {
    if (graphicsPool_) {
        vkDestroyCommandPool(device_->getLogicalDevice(), graphicsPool_, nullptr);
    }
}

VkCommandBuffer CommandManager::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphicsPool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device_->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device_->getGraphicsQueue());

    vkFreeCommandBuffers(device_->getLogicalDevice(), graphicsPool_, 1, &commandBuffer);
}

} // namespace render