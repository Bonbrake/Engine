#pragma once

#include <volk.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace render {

struct PassDependency {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkPipelineStageFlags2 stageMask = 0;
    VkAccessFlags2 accessMask = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED; // For images
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
};

struct PassAttachment {
    VkImageView view = VK_NULL_HANDLE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearValue = {};
};

struct PassNode {
    std::string name;
    bool isCompute = false;
    
    std::vector<PassDependency> reads;
    std::vector<PassDependency> writes;
    
    std::vector<PassAttachment> colorAttachments;
    PassAttachment depthAttachment = {}; 
    VkRect2D renderArea = {{0,0}, {0,0}};
    
    // Callback to record commands for this pass
    std::function<void(VkCommandBuffer)> executeCallback;
};

// Centralized Render Graph with topologically sorted barriers/transitions
class RenderGraph {
public:
    void AddPass(PassNode pass) {
        passes.push_back(std::move(pass));
    }

    void Clear() {
        passes.clear();
    }

    static constexpr uint32_t MAX_PASSES = 16;

    // [M1-EXT-10] Render Graph Pass Dependency DAG Flattener
    // Compiles the framegraph to auto-insert barriers/transitions instead of hand-placed ones
    void CompileAndExecute(VkCommandBuffer cmd, VkQueryPool pool = VK_NULL_HANDLE, uint32_t baseQueryIndex = 0, std::vector<std::string>* executedPassNames = nullptr);

private:
    std::vector<PassNode> passes;
    
    struct ResourceState {
        VkPipelineStageFlags2 stageMask = 0;
        VkAccessFlags2 accessMask = 0;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    };
    
    std::unordered_map<VkBuffer, ResourceState> bufferStates;
    std::unordered_map<VkImage, ResourceState> imageStates;

    void InsertBarriersForPass(VkCommandBuffer cmd, const PassNode& pass);
    std::vector<size_t> TopologicalSort();
};

} // namespace render
