#include "RenderGraph.h"
#include "HazardValidator.h"
#include <queue>
#include <algorithm>
#include <volk.h>
#include "../core/Logger.h"
#include "../core/JobSystem.h"
#include "Device.h"
#include "CommandPoolMatrix.h"

namespace render {

std::vector<size_t> RenderGraph::TopologicalSort() {
    std::vector<size_t> sorted;
    std::vector<std::vector<size_t>> adj(passes.size());
    std::vector<int> inDegree(passes.size(), 0);

    struct ResourceAccess {
        size_t passIndex;
        bool isWrite;
    };
    
    std::unordered_map<VkBuffer, std::vector<ResourceAccess>> bufferAccesses;
    std::unordered_map<VkImage, std::vector<ResourceAccess>> imageAccesses;

    for (size_t i = 0; i < passes.size(); ++i) {
        auto addDependency = [&](size_t from, size_t to) {
            if (from != to) {
                adj[from].push_back(to);
                inDegree[to]++;
            }
        };

        const auto& pass = passes[i];
        for (const auto& read : pass.reads) {
            if (read.buffer != VK_NULL_HANDLE) {
                for (const auto& acc : bufferAccesses[read.buffer]) {
                    if (acc.isWrite) addDependency(acc.passIndex, i);
                }
                bufferAccesses[read.buffer].push_back({i, false});
            }
            if (read.image != VK_NULL_HANDLE) {
                for (const auto& acc : imageAccesses[read.image]) {
                    if (acc.isWrite) addDependency(acc.passIndex, i);
                }
                imageAccesses[read.image].push_back({i, false});
            }
        }
        
        for (const auto& write : pass.writes) {
            if (write.buffer != VK_NULL_HANDLE) {
                for (const auto& acc : bufferAccesses[write.buffer]) {
                    addDependency(acc.passIndex, i);
                }
                bufferAccesses[write.buffer].push_back({i, true});
            }
            if (write.image != VK_NULL_HANDLE) {
                for (const auto& acc : imageAccesses[write.image]) {
                    addDependency(acc.passIndex, i);
                }
                imageAccesses[write.image].push_back({i, true});
            }
        }
    }

    std::queue<size_t> q;
    for (size_t i = 0; i < passes.size(); ++i) {
        if (inDegree[i] == 0) q.push(i);
    }

    while (!q.empty()) {
        size_t u = q.front();
        q.pop();
        sorted.push_back(u);

        for (size_t v : adj[u]) {
            if (--inDegree[v] == 0) {
                q.push(v);
            }
        }
    }
    
    if (sorted.size() != passes.size()) {
        // Fallback to sequential if cycle detected
        sorted.clear();
        for (size_t i = 0; i < passes.size(); ++i) {
            sorted.push_back(i);
        }
    }
    return sorted;
}

void RenderGraph::InsertBarriersForPass(VkCommandBuffer cmd, const PassNode& pass) {
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;

    auto processDependency = [&](const PassDependency& dep, bool isWrite) {
        if (dep.buffer != VK_NULL_HANDLE) {
            ResourceState& state = bufferStates[dep.buffer];
            if (state.stageMask != dep.stageMask || state.accessMask != dep.accessMask) {
                // Hazard validation check
                ResourceStateOwner currentOwner{};
                currentOwner.queueFamilyOwner = 0; // baseline
                currentOwner.isWritingActive = (state.accessMask & (VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0;
                
                if (CheckBarrierHazard(currentOwner, 0, isWrite)) {
                    LOG_TRACE("HazardValidator: Resolved hazard on buffer");
                }

                VkBufferMemoryBarrier2 bb = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
                bb.srcStageMask = state.stageMask == 0 ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : state.stageMask;
                bb.srcAccessMask = state.accessMask;
                bb.dstStageMask = dep.stageMask;
                bb.dstAccessMask = dep.accessMask;
                bb.buffer = dep.buffer;
                bb.offset = 0;
                bb.size = VK_WHOLE_SIZE;
                bufferBarriers.push_back(bb);

                if (isWrite) {
                    state.stageMask = dep.stageMask;
                    state.accessMask = dep.accessMask;
                }
            }
        }
        
        if (dep.image != VK_NULL_HANDLE) {
            ResourceState& state = imageStates[dep.image];
            if (state.layout != dep.layout || state.stageMask != dep.stageMask || state.accessMask != dep.accessMask) {
                // Hazard validation check
                ResourceStateOwner currentOwner{};
                currentOwner.queueFamilyOwner = 0; // baseline
                currentOwner.isWritingActive = (state.accessMask & (VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0;
                
                if (CheckBarrierHazard(currentOwner, 0, isWrite)) {
                    LOG_TRACE("HazardValidator: Resolved hazard on image");
                }

                VkImageMemoryBarrier2 ib = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
                ib.srcStageMask = state.stageMask == 0 ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : state.stageMask;
                ib.srcAccessMask = state.accessMask;
                ib.dstStageMask = dep.stageMask;
                ib.dstAccessMask = dep.accessMask;
                ib.oldLayout = state.layout;
                ib.newLayout = dep.layout;
                ib.image = dep.image;
                ib.subresourceRange = { dep.aspectMask, 0, 1, 0, 1 };
                imageBarriers.push_back(ib);

                if (isWrite || state.layout != dep.layout) {
                    state.stageMask = dep.stageMask;
                    state.accessMask = dep.accessMask;
                    state.layout = dep.layout;
                }
            }
        }
    };

    for (const auto& read : pass.reads) {
        processDependency(read, false);
    }
    
    for (const auto& write : pass.writes) {
        processDependency(write, true);
    }

    if (!bufferBarriers.empty() || !imageBarriers.empty()) {
        VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());
        depInfo.pBufferMemoryBarriers = bufferBarriers.data();
        depInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
        depInfo.pImageMemoryBarriers = imageBarriers.data();
        
        vkCmdPipelineBarrier2(cmd, &depInfo);
    }
}

void RenderGraph::CompileAndExecute(VkCommandBuffer cmd, Device* device, CommandPoolMatrix* poolMatrix, uint32_t frameIndex, VkQueryPool pool, uint32_t baseQueryIndex, std::vector<std::string>* executedPassNames) {
    bufferStates.clear();
    imageStates.clear();
    
    auto sortedIndices = TopologicalSort();
    
    uint32_t currentPassCount = 0;
    bool clamped = false;

    // Allocate an array to store the secondary command buffers for each pass
    std::vector<VkCommandBuffer> secondaryCmds(passes.size(), VK_NULL_HANDLE);
    
    struct PassTask : enki::ITaskSet {
        const PassNode* pass;
        Device* device;
        CommandPoolMatrix* poolMatrix;
        uint32_t frameIndex;
        VkCommandBuffer* outCmd;

        void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
            if (!pass->executeCallback) return;

            VkCommandPool threadPool = poolMatrix->GetPool(frameIndex, threadnum);

            VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandPool = threadPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = 1;
            
            vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, outCmd);

            VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
            
            std::vector<VkFormat> colorFormats;
            for (const auto& att : pass->colorAttachments) {
                colorFormats.push_back(att.format);
            }
            inheritanceRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
            inheritanceRenderingInfo.pColorAttachmentFormats = colorFormats.data();
            
            if (pass->depthAttachment.view != VK_NULL_HANDLE) {
                inheritanceRenderingInfo.depthAttachmentFormat = pass->depthAttachment.format;
            }

            VkCommandBufferInheritanceInfo inheritanceInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
            bool isDynamicRendering = !pass->isCompute && (!pass->colorAttachments.empty() || pass->depthAttachment.view != VK_NULL_HANDLE);
            
            if (isDynamicRendering) {
                inheritanceInfo.pNext = &inheritanceRenderingInfo;
            }

            VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            if (isDynamicRendering) {
                beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            }
            beginInfo.pInheritanceInfo = &inheritanceInfo;

            vkBeginCommandBuffer(*outCmd, &beginInfo);
            pass->executeCallback(*outCmd);
            vkEndCommandBuffer(*outCmd);
        }
    };

    std::vector<std::unique_ptr<PassTask>> tasks;
    auto* scheduler = core::JobSystem::get();

    for (size_t idx : sortedIndices) {
        if (passes[idx].executeCallback) {
            auto task = std::make_unique<PassTask>();
            task->pass = &passes[idx];
            task->device = device;
            task->poolMatrix = poolMatrix;
            task->frameIndex = frameIndex;
            task->outCmd = &secondaryCmds[idx];
            
            scheduler->AddTaskSetToPipe(task.get());
            tasks.push_back(std::move(task));
        }
    }

    for (auto& task : tasks) {
        scheduler->WaitforTask(task.get());
    }

    for (size_t idx : sortedIndices) {
        const auto& pass = passes[idx];
        
        VkPipelineStageFlags2 stageMask = pass.isCompute ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        if (pool != VK_NULL_HANDLE && executedPassNames) {
            if (currentPassCount >= MAX_PASSES) {
                if (!clamped) {
                    LOG_WARN("RenderGraph exceeded MAX_PASSES ({}). Timestamp queries truncated.", MAX_PASSES);
                    clamped = true;
                }
            } else {
                vkCmdWriteTimestamp2(cmd, stageMask, pool, baseQueryIndex + currentPassCount * 2);
            }
        }

        InsertBarriersForPass(cmd, pass);
        
        bool isDynamicRendering = !pass.isCompute && (!pass.colorAttachments.empty() || pass.depthAttachment.view != VK_NULL_HANDLE);
        
        if (isDynamicRendering) {
            std::vector<VkRenderingAttachmentInfo> colorInfos;
            for (const auto& att : pass.colorAttachments) {
                VkRenderingAttachmentInfo info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
                info.imageView = att.view;
                info.imageLayout = att.layout;
                info.loadOp = att.loadOp;
                info.storeOp = att.storeOp;
                info.clearValue = att.clearValue;
                colorInfos.push_back(info);
            }
            
            VkRenderingInfo renderInfo = { VK_STRUCTURE_TYPE_RENDERING_INFO };
            renderInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
            renderInfo.renderArea = pass.renderArea;
            renderInfo.layerCount = 1;
            renderInfo.colorAttachmentCount = static_cast<uint32_t>(colorInfos.size());
            renderInfo.pColorAttachments = colorInfos.data();
            
            VkRenderingAttachmentInfo depthInfo = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
            if (pass.depthAttachment.view != VK_NULL_HANDLE) {
                depthInfo.imageView = pass.depthAttachment.view;
                depthInfo.imageLayout = pass.depthAttachment.layout;
                depthInfo.loadOp = pass.depthAttachment.loadOp;
                depthInfo.storeOp = pass.depthAttachment.storeOp;
                depthInfo.clearValue = pass.depthAttachment.clearValue;
                renderInfo.pDepthAttachment = &depthInfo;
            }
            
            vkCmdBeginRendering(cmd, &renderInfo);
        }
        
        if (secondaryCmds[idx] != VK_NULL_HANDLE) {
            vkCmdExecuteCommands(cmd, 1, &secondaryCmds[idx]);
        }
        
        if (isDynamicRendering) {
            vkCmdEndRendering(cmd);
        }

        if (pool != VK_NULL_HANDLE && executedPassNames) {
            if (currentPassCount < MAX_PASSES) {
                vkCmdWriteTimestamp2(cmd, stageMask, pool, baseQueryIndex + currentPassCount * 2 + 1);
                executedPassNames->push_back(pass.name);
                currentPassCount++;
            }
        }
    }
}

} // namespace render
