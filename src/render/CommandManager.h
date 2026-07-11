#pragma once

#include <volk.h>
#include <vector>

namespace render {

class Device;

class CommandManager {
public:
    CommandManager(Device* device);
    ~CommandManager();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    Device* device_ = nullptr;
    VkCommandPool graphicsPool_ = VK_NULL_HANDLE;
};

} // namespace render
