#include "TriangleRenderer.h"
#include "MaterialSystem.h"
#include "Device.h"
#include "PipelineBuilder.h"
#include "../core/Logger.h"
#include "../core/CVarSystem.h"
#include "../ecs/ECS.h"
#include "../render/AssetManager.h"
#include "../ecs/Components.h"          // ecs::Transform, ecs::MeshComponent
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>      // glm::qua, glm::mat4_cast
#include <fstream>
#include <vector>
#include <string.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace render {

struct InstanceData {
    float pos_rad[4];
};

struct DrawIndexedIndirectCommand {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    uint32_t firstInstance;
};

static std::vector<uint32_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open {}", filename);
        return {};
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();
    return buffer;
}

static VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void TriangleRenderer::createBuffers(Device* device) {
    // Vertex buffer
    float vertices[] = {
         0.0f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };
    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = sizeof(vertices);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vertexBuffer, &vertexAllocation, nullptr);
    void* data;
    vmaMapMemory(device->getAllocator(), vertexAllocation, &data);
    memcpy(data, vertices, sizeof(vertices));
    vmaUnmapMemory(device->getAllocator(), vertexAllocation);

    // Index buffer
    uint32_t indices[] = {0, 1, 2};
    bufferInfo.size = sizeof(indices);
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &indexBuffer, &indexAllocation, nullptr);
    vmaMapMemory(device->getAllocator(), indexAllocation, &data);
    memcpy(data, indices, sizeof(indices));
    vmaUnmapMemory(device->getAllocator(), indexAllocation);

    // Instance buffer (100 entities)
    // SHADER_DEVICE_ADDRESS_BIT is required: init() calls vkGetBufferDeviceAddress on this
    // buffer to write its storage-buffer descriptor into the descriptor buffer.
    std::vector<InstanceData> instances(100);
    for (int i = 0; i < 100; i++) {
        instances[i].pos_rad[0] = (i % 10) * 1.5f - 7.0f;
        instances[i].pos_rad[1] = (i / 10) * 1.5f - 7.0f;
        instances[i].pos_rad[2] = 0.5f; // depth inside [0, 1]
        instances[i].pos_rad[3] = 0.5f; // radius
    }
    bufferInfo.size = instances.size() * sizeof(InstanceData);
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &instanceBuffer, &instanceAllocation, nullptr);
    vmaMapMemory(device->getAllocator(), instanceAllocation, &data);
    memcpy(data, instances.data(), bufferInfo.size);
    vmaUnmapMemory(device->getAllocator(), instanceAllocation);

    // Indirect buffer (3 buffers for triple-buffering)
    for (int i = 0; i < 3; i++) {
        bufferInfo.size = 100 * sizeof(DrawIndexedIndirectCommand);
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // Compute writes to it
        vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &indirectBuffer[i], &indirectAllocation[i], nullptr);

        // Count buffer (3 buffers for triple-buffering)
        bufferInfo.size = sizeof(uint32_t);
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &countBuffer[i], &countAllocation[i], nullptr);

        // Readback buffer (3 buffers for triple-buffering)
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &countReadbackBuffer[i], &countReadbackAllocation[i], nullptr);
    }
}

void TriangleRenderer::setScene(ecs::ECSContext* ecsCtx, AssetManager* assetManager) {
    ecsCtx_ = ecsCtx;
    sceneAssets_ = assetManager;
    // [M1-EXT-07] One 64KB scratch page per frame for the visible-entity list.
    // Reset-only, never reallocated across frames (pool semantics per EXT-05/EXT-03).
    if (!frameArena_.memoryBufferPage) {
        static std::vector<uint8_t> s_page(64 * 1024);
        frameArena_.memoryBufferPage = s_page.data();
        frameArena_.capacity = s_page.size();
        frameArena_.Reset();
    }
}

// [M1:EXIT-1] Single camera-relative cast: renderPos = (vec3)(entityPos - camPos).
// Authoritative position is dvec3; the GPU only ever sees a vec3 relative to the
// camera, so world magnitude (km-scale, M2.6) never enters single-precision math.
glm::mat4 BuildEntityMVP(const ecs::Transform& t, const glm::dvec3& camPos,
                         const glm::mat4& view, const glm::mat4& proj) {
    const glm::vec3 renderPos = glm::vec3(t.position - camPos);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), renderPos);
    // dquat -> quat (rotation is float-precision by design). Build quat from the
    // dquat's components to avoid the missing dquat->quat cast in glm.
    const glm::quat rot = glm::quat(static_cast<float>(t.rotation.w),
                                    static_cast<float>(t.rotation.x),
                                    static_cast<float>(t.rotation.y),
                                    static_cast<float>(t.rotation.z));
    model = model * glm::mat4_cast(rot);
    model = glm::scale(model, glm::vec3(t.scale));
    return proj * view * model;
}

void TriangleRenderer::createDescriptorSets(Device* device) {
    if (computeSetLayouts.empty() || graphicsSetLayouts.empty()) {
        LOG_ERROR("Cannot create descriptor sets: Layouts are not built.");
        return;
    }
    
    VkDescriptorSetLayout graphicsLayout = graphicsSetLayouts.empty() ? VK_NULL_HANDLE : graphicsSetLayouts[0];
    VkDescriptorSetLayout computeLayout = computeSetLayouts.empty() ? VK_NULL_HANDLE : computeSetLayouts[0];

    const auto& props = device->getDescriptorBufferProperties();
    
    VkDeviceSize gSize = 0;
    if (graphicsLayout != VK_NULL_HANDLE) {
        vkGetDescriptorSetLayoutSizeEXT(device->getLogicalDevice(), graphicsLayout, &gSize);
    }
    
    VkDeviceSize cSize = 0;
    if (computeLayout != VK_NULL_HANDLE) {
        vkGetDescriptorSetLayoutSizeEXT(device->getLogicalDevice(), computeLayout, &cSize);
    }

    // Align compute offset
    VkDeviceSize alignment = props.descriptorBufferOffsetAlignment;
    VkDeviceSize cAlignedSize = (cSize + alignment - 1) & ~(alignment - 1);
    VkDeviceSize gAlignedSize = (gSize + alignment - 1) & ~(alignment - 1);
    
    VkDeviceSize totalBufferSize = 3 * (cAlignedSize + gAlignedSize);
    if (totalBufferSize == 0) return;

    for (int i = 0; i < 3; i++) {
        computeSetOffset[i] = i * (cAlignedSize + gAlignedSize);
        graphicsSetOffset[i] = computeSetOffset[i] + cAlignedSize;
    }

    LOG_INFO("TriangleRenderer: computeSet0={}, graphicsSet0={}, totalBufferSize={}, align={}", computeSetOffset[0], graphicsSetOffset[0], totalBufferSize, alignment);

    VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = totalBufferSize;
    bufInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    VmaAllocationInfo allocResult;
    vmaCreateBuffer(device->getAllocator(), &bufInfo, &allocInfo, &descriptorBuffer, &descriptorAllocation, &allocResult);
    descriptorBufferMapped = allocResult.pMappedData;

    VkBufferDeviceAddressInfo dbAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    dbAddressInfo.buffer = descriptorBuffer;
    descriptorBufferAddress = vkGetBufferDeviceAddress(device->getLogicalDevice(), &dbAddressInfo);

    // Sampler
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(device->getLogicalDevice(), &samplerInfo, nullptr, &depthSampler);

    // Dummy depth image
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { 1, 1, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(device->getAllocator(), &imageInfo, &vmaAllocInfo, &dummyDepthImage, &dummyDepthAllocation, nullptr);
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = dummyDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_D32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &dummyDepthView);

    // Update descriptor buffer (write the descriptors once for now)
    VkBufferDeviceAddressInfo bdaInst{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    bdaInst.buffer = instanceBuffer;
    VkDeviceAddress addrInst = vkGetBufferDeviceAddress(device->getLogicalDevice(), &bdaInst);

    VkDeviceAddress addrInd[3];
    VkDeviceAddress addrCount[3];
    for (int i = 0; i < 3; ++i) {
        VkBufferDeviceAddressInfo bdaInd{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        bdaInd.buffer = indirectBuffer[i];
        addrInd[i] = vkGetBufferDeviceAddress(device->getLogicalDevice(), &bdaInd);

        VkBufferDeviceAddressInfo bdaCount{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        bdaCount.buffer = countBuffer[i];
        addrCount[i] = vkGetBufferDeviceAddress(device->getLogicalDevice(), &bdaCount);
    }

    VkDescriptorImageInfo imgInfo{depthSampler, dummyDepthView, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL};

    VkDescriptorGetInfoEXT getInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT};

    // Helper lambda to write a storage buffer descriptor
    auto writeStorageBuffer = [&](VkDescriptorSetLayout layout, VkDeviceSize setOffset, uint32_t binding, VkDeviceAddress addr, VkDeviceSize size) {
        VkDeviceSize bindingOffset = 0;
        vkGetDescriptorSetLayoutBindingOffsetEXT(device->getLogicalDevice(), layout, binding, &bindingOffset);
        
        VkDescriptorAddressInfoEXT addrInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT};
        addrInfo.address = addr;
        addrInfo.range = size;
        addrInfo.format = VK_FORMAT_UNDEFINED;
        
        getInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        getInfo.data.pStorageBuffer = &addrInfo;
        
        void* dst = static_cast<uint8_t*>(descriptorBufferMapped) + setOffset + bindingOffset;
        vkGetDescriptorEXT(device->getLogicalDevice(), &getInfo, props.storageBufferDescriptorSize, dst);
    };

    // Helper lambda to write a sampler descriptor
    auto writeSampler = [&](VkDescriptorSetLayout layout, VkDeviceSize setOffset, uint32_t binding, VkDescriptorImageInfo* pImgInfo) {
        VkDeviceSize bindingOffset = 0;
        vkGetDescriptorSetLayoutBindingOffsetEXT(device->getLogicalDevice(), layout, binding, &bindingOffset);
        
        getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        getInfo.data.pCombinedImageSampler = pImgInfo;
        
        void* dst = static_cast<uint8_t*>(descriptorBufferMapped) + setOffset + bindingOffset;
        vkGetDescriptorEXT(device->getLogicalDevice(), &getInfo, props.combinedImageSamplerDescriptorSize, dst);
    };

    // Real buffer sizes (cannot use VK_WHOLE_SIZE: validation rejects it for these
    // storage-buffer descriptors, and a whole-size range into a 1600B instance buffer
    // faults the GPU). Must match the sizes allocated in createBuffers().
    const VkDeviceSize instSize = 100 * sizeof(InstanceData);
    const VkDeviceSize indSize  = 100 * sizeof(DrawIndexedIndirectCommand);
    const VkDeviceSize cntSize  = sizeof(uint32_t);

    if (graphicsLayout != VK_NULL_HANDLE) {
        for (int i = 0; i < 3; i++) {
            writeStorageBuffer(graphicsLayout, graphicsSetOffset[i], 0, addrInst, instSize);
        }
    }
    
    if (computeLayout != VK_NULL_HANDLE) {
        for (int i = 0; i < 3; i++) {
            writeStorageBuffer(computeLayout, computeSetOffset[i], 0, addrInst, instSize);
            writeStorageBuffer(computeLayout, computeSetOffset[i], 1, addrInd[i], indSize);
            writeStorageBuffer(computeLayout, computeSetOffset[i], 2, addrCount[i], cntSize);
            writeSampler(computeLayout, computeSetOffset[i], 3, &imgInfo);
        }
    }
}

void TriangleRenderer::createPipelines(Device* device, VkFormat colorFormat) {
    // 1. Graphics Pipeline
    auto vertCode = readFile("build/shaders/simple.vert.spv");
    auto fragCode = readFile("build/shaders/simple.frag.spv");
    VkShaderModule vertModule = createShaderModule(device->getLogicalDevice(), vertCode);
    VkShaderModule fragModule = createShaderModule(device->getLogicalDevice(), fragCode);

    VkPipelineShaderStageCreateInfo vertStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertModule;
    vertStageInfo.pName = "main";
    VkPipelineShaderStageCreateInfo fragStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragModule;
    fragStageInfo.pName = "main";
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(float) * 6;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = sizeof(float) * 3;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo colorBlending{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto graphicsLayoutData = PipelineBuilder::buildLayouts({vertCode, fragCode}, device);
    graphicsSetLayouts = graphicsLayoutData.setLayouts;
    pipelineLayout = graphicsLayoutData.pipelineLayout;

    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    VkPipelineRenderingCreateInfo renderingInfo{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;
    renderingInfo.depthAttachmentFormat = depthFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    
    // Create a wireframe variant for derivative testing
    VkPipelineRasterizationStateCreateInfo wireframeRasterizer = rasterizer;
    wireframeRasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    VkGraphicsPipelineCreateInfo wireframeInfo = pipelineInfo;
    wireframeInfo.pRasterizationState = &wireframeRasterizer;

    std::vector<VkGraphicsPipelineCreateInfo> batch = { pipelineInfo, wireframeInfo };
    // [M1-EXT-04] Batched Pipeline Creation + Pipeline Derivatives
    auto pipelines = PipelineBuilder::buildPipelines(batch, device);
    if (pipelines.size() >= 2) {
        pipeline = pipelines[0];
        wireframePipeline = pipelines[1]; // Store it, need to declare this in TriangleRenderer.h
    }

    // [Slice 0a] Dev-test cube pipeline: matches MeshAsset vertex layout (pos+normal, 32B stride).
    // Uses its own shader (simple_mesh.vert) which does NOT apply the per-instance offset that
    // simple.vert uses for the demo triangles. No instance/descriptor buffers needed.
    auto meshVertCode = readFile("build/shaders/simple_mesh.vert.spv");
    VkShaderModule meshVertModule = createShaderModule(device->getLogicalDevice(), meshVertCode);

    VkPipelineShaderStageCreateInfo meshVertStage = vertStageInfo;
    meshVertStage.module = meshVertModule;
    VkPipelineShaderStageCreateInfo meshStages[] = { meshVertStage, fragStageInfo };

    VkVertexInputBindingDescription meshBinding{};
    meshBinding.binding = 0;
    meshBinding.stride = sizeof(float) * 8; // vec3 pos + vec3 normal + vec2 uv (AssetTypes.h Vertex)
    meshBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription meshAttribs[2]{};
    meshAttribs[0].binding = 0; meshAttribs[0].location = 0;
    meshAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT; meshAttribs[0].offset = 0;
    meshAttribs[1].binding = 0; meshAttribs[1].location = 1;
    meshAttribs[1].format = VK_FORMAT_R32G32B32_SFLOAT; meshAttribs[1].offset = sizeof(float) * 3;

    VkPipelineVertexInputStateCreateInfo meshVii{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    meshVii.vertexBindingDescriptionCount = 1;
    meshVii.pVertexBindingDescriptions = &meshBinding;
    meshVii.vertexAttributeDescriptionCount = 2;
    meshVii.pVertexAttributeDescriptions = meshAttribs;

    VkPipelineRasterizationStateCreateInfo meshRast = rasterizer;
    meshRast.cullMode = VK_CULL_MODE_NONE; // whole cube visible from front

    VkGraphicsPipelineCreateInfo meshInfo = pipelineInfo;
    meshInfo.stageCount = 2;
    meshInfo.pStages = meshStages;
    meshInfo.pVertexInputState = &meshVii;
    meshInfo.pRasterizationState = &meshRast;
    // meshInfo.layout = pipelineLayout (shared, push-constant compatible); depthStencil from pipelineInfo.
    std::vector<VkGraphicsPipelineCreateInfo> meshBatch = { meshInfo };
    auto meshPipelines = PipelineBuilder::buildPipelines(meshBatch, device);
    if (!meshPipelines.empty()) meshPipeline = meshPipelines[0];
    vkDestroyShaderModule(device->getLogicalDevice(), meshVertModule, nullptr);

    vkDestroyShaderModule(device->getLogicalDevice(), vertModule, nullptr);
    vkDestroyShaderModule(device->getLogicalDevice(), fragModule, nullptr);

    // 2. Compute Pipeline (Cull)
    auto cullCode = readFile("build/shaders/cull.comp.spv");
    VkShaderModule cullModule = createShaderModule(device->getLogicalDevice(), cullCode);
    VkPipelineShaderStageCreateInfo cullStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    cullStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cullStage.module = cullModule;
    cullStage.pName = "main";
    
    auto computeLayoutData = PipelineBuilder::buildLayouts({cullCode}, device);
    computeSetLayouts = computeLayoutData.setLayouts;
    cullPipelineLayout = computeLayoutData.pipelineLayout;

    VkComputePipelineCreateInfo computeInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    computeInfo.stage = cullStage;
    computeInfo.layout = cullPipelineLayout;
    // [M4.5-EXT-33] Compute pipeline binds descriptors via the descriptor buffer,
    // so it MUST carry the descriptor-buffer flag (VUID 08600/08117).
    if (device->getCapabilities().descriptorBuffer) {
        computeInfo.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    }
    vkCreateComputePipelines(device->getLogicalDevice(), VK_NULL_HANDLE, 1, &computeInfo, nullptr, &cullPipeline);
    vkDestroyShaderModule(device->getLogicalDevice(), cullModule, nullptr);
}

void TriangleRenderer::init(Device* device, VkFormat colorFormat) {
    createBuffers(device);
    createPipelines(device, colorFormat);
    createDescriptorSets(device);
    
    // [M1-EXT-28] Occlusion Query Pools (GPU Software Occlusion Rasterizer / HZB feeder)
    VkQueryPoolCreateInfo queryPoolInfo{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    queryPoolInfo.queryType = VK_QUERY_TYPE_OCCLUSION;
    queryPoolInfo.queryCount = 100;
    // [M4.5-EXT-33] RESET_BIT enables vkCmdResetQueryPool at the start of each draw frame.
    // Requires VK_KHR_maintenance9 (enabled via device extensions).
    queryPoolInfo.flags = VK_QUERY_POOL_CREATE_RESET_BIT_KHR;
    for (int i = 0; i < 3; i++) {
        vkCreateQueryPool(device->getLogicalDevice(), &queryPoolInfo, nullptr, &occlusionPools[i]);
        // Initial reset: the first read of this pool happens before any vkCmdResetQueryPool.
        vkResetQueryPool(device->getLogicalDevice(), occlusionPools[i], 0, 100);
    }
}

void TriangleRenderer::cleanup(Device* device) {
    for (int i = 0; i < 3; i++) {
        if (occlusionPools[i] != VK_NULL_HANDLE) {
            vkDestroyQueryPool(device->getLogicalDevice(), occlusionPools[i], nullptr);
            occlusionPools[i] = VK_NULL_HANDLE;
        }
    }

    if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device->getLogicalDevice(), pipeline, nullptr);
    if (wireframePipeline != VK_NULL_HANDLE) vkDestroyPipeline(device->getLogicalDevice(), wireframePipeline, nullptr);
    if (meshPipeline != VK_NULL_HANDLE) vkDestroyPipeline(device->getLogicalDevice(), meshPipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device->getLogicalDevice(), pipelineLayout, nullptr);
    if (cullPipeline != VK_NULL_HANDLE) vkDestroyPipeline(device->getLogicalDevice(), cullPipeline, nullptr);
    
    if (vertexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), vertexBuffer, vertexAllocation);
    if (indexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), indexBuffer, indexAllocation);
    if (instanceBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), instanceBuffer, instanceAllocation);
    for (int i = 0; i < 3; i++) {
        if (indirectBuffer[i] != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), indirectBuffer[i], indirectAllocation[i]);
        if (countBuffer[i] != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), countBuffer[i], countAllocation[i]);
        if (countReadbackBuffer[i] != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), countReadbackBuffer[i], countReadbackAllocation[i]);
    }

    for(auto layout : graphicsSetLayouts) {
        if (layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(device->getLogicalDevice(), layout, nullptr);
    }
    for(auto layout : computeSetLayouts) {
        if (layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(device->getLogicalDevice(), layout, nullptr);
    }
    
    if (descriptorBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device->getAllocator(), descriptorBuffer, descriptorAllocation);
    }
    
    if (depthSampler != VK_NULL_HANDLE) vkDestroySampler(device->getLogicalDevice(), depthSampler, nullptr);
    if (dummyDepthImage != VK_NULL_HANDLE) {
        vkDestroyImageView(device->getLogicalDevice(), dummyDepthView, nullptr);
        vmaDestroyImage(device->getAllocator(), dummyDepthImage, dummyDepthAllocation);
    }
}

void TriangleRenderer::cull(VkCommandBuffer cmd, uint32_t imageIndex, VkImageView currentDepthView, Device* device) {
    frameCounter++;
    
    // Update descriptor set with the depth view (reproject previous frame depth for Hi-Z)
    VkDescriptorImageInfo imgInfo{depthSampler, currentDepthView, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL};
    VkDescriptorGetInfoEXT getInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT};
    getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    getInfo.data.pCombinedImageSampler = &imgInfo;
    
    // We already aligned computeSetOffset, and we know binding 3 offset from querying
    VkDeviceSize bindingOffset = 0;
    vkGetDescriptorSetLayoutBindingOffsetEXT(device->getLogicalDevice(), computeSetLayouts[0], 3, &bindingOffset);
    
    const auto& props = device->getDescriptorBufferProperties();
    void* dst = static_cast<uint8_t*>(descriptorBufferMapped) + computeSetOffset[imageIndex] + bindingOffset;
    vkGetDescriptorEXT(device->getLogicalDevice(), &getInfo, props.combinedImageSamplerDescriptorSize, dst);

    // 1. Clear count buffer
    vkCmdFillBuffer(cmd, countBuffer[imageIndex], 0, sizeof(uint32_t), 0);
    
    // Barrier to wait for clear
    VkBufferMemoryBarrier2 clearBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
    clearBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    clearBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    clearBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    clearBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    clearBarrier.buffer = countBuffer[imageIndex];
    clearBarrier.size = VK_WHOLE_SIZE;
    VkDependencyInfo clearDep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    clearDep.bufferMemoryBarrierCount = 1;
    clearDep.pBufferMemoryBarriers = &clearBarrier;
    vkCmdPipelineBarrier2(cmd, &clearDep);

    // 2. Dispatch cull
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, cullPipeline);
    
    VkDescriptorBufferBindingInfoEXT bindingInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT};
    bindingInfo.address = descriptorBufferAddress;
    bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);
    
    uint32_t bufferIndex = 0;
    VkDeviceSize offset = computeSetOffset[imageIndex];
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, cullPipelineLayout, 0, 1, &bufferIndex, &offset);
    
    struct PC {
        float mvp[16];
        uint32_t instanceCount;
        uint32_t frameCounter;
        uint32_t cullEnabled;
    } pc;
    // Dummy view proj
    memset(pc.mvp, 0, sizeof(pc.mvp));
    pc.mvp[0] = 1.0f; pc.mvp[5] = 1.0f; pc.mvp[10] = 1.0f; pc.mvp[15] = 1.0f;
    pc.instanceCount = 100;
    pc.frameCounter = frameCounter;
    
    // Register/Read CVar for culling
    core::CVar* cullCvar = core::CVarSystem::Get().GetCVar("r_EnableCulling");
    if (!cullCvar) cullCvar = core::CVarSystem::Get().RegisterBool("r_EnableCulling", true);
    pc.cullEnabled = cullCvar->val.b ? 1 : 0;
    
    vkCmdPushConstants(cmd, cullPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PC), &pc);
    
    vkCmdDispatch(cmd, (100 + 63) / 64, 1, 1);
    
    // 3. CmdIndirectDrawBarrier [M1-EXT-11]
    VkBufferMemoryBarrier2 b[2]{};
    for (int i = 0; i < 2; i++) {
        VkBufferMemoryBarrier2& x = b[i];
        x.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        x.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        x.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        x.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        x.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        x.buffer = (i == 0) ? indirectBuffer[imageIndex] : countBuffer[imageIndex];
        x.offset = 0;
        x.size = VK_WHOLE_SIZE;
    }
    VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    dep.bufferMemoryBarrierCount = 2;
    dep.pBufferMemoryBarriers = b;
    vkCmdPipelineBarrier2(cmd, &dep);
}

void TriangleRenderer::draw(VkCommandBuffer cmd, uint32_t imageIndex, MaterialSystem* materialSystem) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (pipeline == VK_NULL_HANDLE) return;

    // [M1:EXIT-1] ECS->render bridge is the real path. Path A (Slice 0a) is bootstrap
    // scaffolding: draw the static intact cube ONLY when the bridge isn't wired yet
    // (ecsCtx_/sceneAssets_ null). Once the bridge is active it draws every entity via
    // mc.meshHandle — including the destructible swap — so Path A must yield, or the
    // always-on intact cube masks the swap.
    const bool bridgeActive = (ecsCtx_ != nullptr) && (sceneAssets_ != nullptr);

    // proj/view/pc are shared by both Path A (bootstrap) and the ECS bridge, so they live
    // in the outer scope before the two-way branch below.
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
    glm::mat4 view = devViewSet_
        ? devView_
        : glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    struct PC { float mvp[16]; } pc;

    // [M1:EXIT-1] The mesh pipeline uses VK_DYNAMIC_STATE_VIEWPORT/SCISSOR, so the
    // viewport+scissor MUST be set on the command buffer before any draw. Set it once,
    // unconditionally, here — both Path A and the ECS bridge rely on it (gating Path A
    // off would otherwise leave the bridge drawing into an unset viewport → blank frame).
    VkViewport viewport{};
    viewport.width = 800.0f;
    viewport.height = 600.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.extent = {800, 600};
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // [Slice 0a] Dev-test cube (bootstrap fallback, only when bridge absent): draw the
    // intact mesh directly, bypassing the ECS bridge. Gated to --dev via setDevTestMesh().
    MeshAsset* devTestMesh = sceneAssets_ ? sceneAssets_->GetMesh(devTestMeshHandle_) : nullptr;
    if (!bridgeActive && devTestMesh && devTestMesh->vertexBuffer != VK_NULL_HANDLE &&
        devTestMesh->indexBuffer != VK_NULL_HANDLE && meshPipeline != VK_NULL_HANDLE) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPipeline);

        // Real perspective MVP for the Path A static cube (centered at origin).
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                           glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                           glm::scale(glm::mat4(1.0f), glm::vec3(0.7f));
        glm::mat4 mvp = proj * view * model;

        memcpy(pc.mvp, &mvp[0][0], sizeof(pc.mvp));
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &devTestMesh->vertexBuffer, offsets);
        vkCmdBindIndexBuffer(cmd, devTestMesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, devTestMesh->indexCount, 1, 0, 0, 0);

        return; // bootstrap cube drawn; skip bridge + demo-triangle path
    }

    // [M1:EXIT-1] ECS->render bridge: traverse view<Transform, MeshComponent> and draw
    // each entity through the real camera-relative path (BuildEntityMVP). This also drives
    // the M2.6 far-cube precision probe: a real entity at dvec3(50000,0,0) now renders via
    // the same cast as everything else, closing the "no scene render path" caveat.
    // The dev-test cube above is a separate gated proxy; the loop below is the actual bridge.
    if (bridgeActive) {
        uint32_t renderedEntities = 0;
        auto viewEnts = ecsCtx_->GetRegistry().view<ecs::Transform, ecs::MeshComponent>();
        frameArena_.Reset();
        // Bridge draws real meshes through meshPipeline — must be bound (Path A did this at
        // line 650). With Path A gated off under bridgeActive, nothing else binds it, so the
        // draws would hit an unbound pipeline -> black frame.
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPipeline);
        for (entt::entity e : viewEnts) {
            const auto& mc = viewEnts.get<ecs::MeshComponent>(e);
            MeshAsset* mesh = sceneAssets_->GetMesh(mc.meshHandle);
            if (!mesh || mesh->vertexBuffer == VK_NULL_HANDLE || mesh->indexBuffer == VK_NULL_HANDLE) {
                LOG_WARN("Render: entity {} meshHandle {{index={}, gen={}}} invalid; skipped",
                         static_cast<uint32_t>(e), mc.meshHandle.index, mc.meshHandle.generation);
                continue;
            }
            const glm::dvec3 camPos = devViewSet_ ? devCamPos_ : glm::dvec3(0.0, 0.0, 4.0);
            glm::mat4 eMvp = BuildEntityMVP(viewEnts.get<ecs::Transform>(e), camPos, view, proj);

            memcpy(pc.mvp, &eMvp[0][0], sizeof(pc.mvp));
            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

            VkDeviceSize eOffsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vertexBuffer, eOffsets);
            vkCmdBindIndexBuffer(cmd, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, mesh->indexCount, 1, 0, 0, 0);
            ++renderedEntities;
        }
        if (frameCounter % 10 == 0 && renderedEntities > 0) {
            LOG_INFO("Rendered {} ECS entities via view<Transform, MeshComponent>", renderedEntities);
        }
        return; // bridge entities drawn; skip demo-triangle path
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    VkDescriptorBufferBindingInfoEXT bindingInfos[2] = {};
    bindingInfos[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfos[0].address = descriptorBufferAddress;
    bindingInfos[0].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    
    uint32_t bufferCount = 1;

    if (materialSystem && materialSystem->GetDescriptorBuffer() != VK_NULL_HANDLE) {
        bindingInfos[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        bindingInfos[1].address = materialSystem->GetDescriptorBufferAddress();
        bindingInfos[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        bufferCount = 2;
    }

    vkCmdBindDescriptorBuffersEXT(cmd, bufferCount, bindingInfos);

    uint32_t bufferIndex = 0;
    VkDeviceSize offset = graphicsSetOffset[0];
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &bufferIndex, &offset);

    if (bufferCount == 2) {
        // [TODO] When graphics pipeline has set=1 (material system bindings), bind dynamic offset here:
        // uint32_t matBufferIndex = 1;
        // VkDeviceSize matOffset = handle.index * materialSystem->GetAlignedBlockSize();
        // vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &matBufferIndex, &matOffset);
    }

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // [M1:EXIT-1] viewport+scissor already set unconditionally above (dynamic-state pipeline).
    memset(pc.mvp, 0, sizeof(pc.mvp));
    pc.mvp[0] = 1.0f; pc.mvp[5] = 1.0f; pc.mvp[10] = 1.0f; pc.mvp[15] = 1.0f;
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

    // [M1-EXT-28] Occlusion Query Double-Buffering (GPU Software Occlusion Rasterizer / HZB feeder)
    vkCmdResetQueryPool(cmd, occlusionPools[imageIndex], 0, 100);
    vkCmdBeginQuery(cmd, occlusionPools[imageIndex], 0, 0);
    
    // M1 Exit Criterion 1: vkCmdDrawIndexedIndirectCount
    vkCmdDrawIndexedIndirectCount(cmd, indirectBuffer[imageIndex], 0, countBuffer[imageIndex], 0, 100, sizeof(DrawIndexedIndirectCommand));

    vkCmdEndQuery(cmd, occlusionPools[imageIndex], 0);

    // After drawing, copy count buffer to readback buffer for logging
    VkBufferMemoryBarrier2 copyBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
    copyBarrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    copyBarrier.srcAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    copyBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    copyBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    copyBarrier.buffer = countBuffer[imageIndex];
    copyBarrier.size = VK_WHOLE_SIZE;
    VkDependencyInfo copyDep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    copyDep.bufferMemoryBarrierCount = 1;
    copyDep.pBufferMemoryBarriers = &copyBarrier;
    vkCmdPipelineBarrier2(cmd, &copyDep);

    VkBufferCopy copyRegion{};
    copyRegion.size = sizeof(uint32_t);
    vkCmdCopyBuffer(cmd, countBuffer[imageIndex], countReadbackBuffer[imageIndex], 1, &copyRegion);
}

void TriangleRenderer::readbackCount(Device* device, uint32_t imageIndex) {
    if (countReadbackBuffer[imageIndex] != VK_NULL_HANDLE && frameCounter > 1) {
        void* mapped;
        vmaMapMemory(device->getAllocator(), countReadbackAllocation[imageIndex], &mapped);
        uint32_t count = *(uint32_t*)mapped;
        vmaUnmapMemory(device->getAllocator(), countReadbackAllocation[imageIndex]);
        
        // [M1-EXT-28] Occlusion Query Double-Buffering (GPU Software Occlusion Rasterizer / HZB feeder)
        // Read without VK_QUERY_RESULT_WAIT_BIT to avoid GPU/CPU sync stall
        if (occlusionPools[imageIndex] != VK_NULL_HANDLE) {
            VkResult res = vkGetQueryPoolResults(device->getLogicalDevice(), occlusionPools[imageIndex], 0, 1, sizeof(uint32_t), &occlusionResults[0], sizeof(uint32_t), 0);
            if (res == VK_SUCCESS && frameCounter % 10 == 0) {
                // Log the hardware occlusion query result (passed samples)
                LOG_INFO("Hardware Occlusion Query: {} samples passed", occlusionResults[0]);
            }
        }
        
        // Ensure we print only occasionally so we don't spam the log, or log if count < 100
        if (frameCounter % 10 == 0) {
            LOG_INFO("Hi-Z Compute Culling: {}/100 instances visible ({}% reduction)", count, 100 - count);
        }
    }
}
} // namespace render
