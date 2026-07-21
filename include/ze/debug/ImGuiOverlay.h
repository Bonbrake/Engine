#pragma once

#include <volk.h>
#include <SDL3/SDL.h>
#include <vector>
#include <string>

namespace debug {

class ImGuiOverlay {
public:
    void Initialize(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, uint32_t queueFamily, SDL_Window* window);
    void Destroy(VkDevice device);

    void NewFrame();
    void Render(VkCommandBuffer cmd);
    void ProcessEvent(const SDL_Event* event);

    bool IsVisible() const { return isVisible; }
    void ToggleVisibility() { isVisible = !isVisible; }

    void SetPassTimings(const std::vector<std::pair<std::string, float>>& timings, bool supported) {
        passTimings = timings;
        timestampsSupported = supported;
    }

private:
    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    bool isVisible = false;
    bool timestampsSupported = false;
    std::vector<std::pair<std::string, float>> passTimings;
};

} // namespace debug
