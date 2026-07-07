#define VMA_IMPLEMENTATION
#include "Renderer.h"
#include <iostream>
#include <stdexcept>
#include <fstream>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif

namespace render {

const std::vector<Vertex> CUBE_VERTICES = {
    // Front face
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
    // Back face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    // Left face
    {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.1f, 0.1f}},
    {{-0.5f, -0.5f,  0.5f}, {0.2f, 0.2f, 0.2f}},
    {{-0.5f,  0.5f,  0.5f}, {0.3f, 0.3f, 0.3f}},
    {{-0.5f,  0.5f,  0.5f}, {0.3f, 0.3f, 0.3f}},
    {{-0.5f,  0.5f, -0.5f}, {0.4f, 0.4f, 0.4f}},
    {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.1f, 0.1f}},
    // Right face
    {{ 0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.6f, 0.6f, 0.6f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.7f, 0.7f, 0.7f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.7f, 0.7f, 0.7f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.8f, 0.8f, 0.8f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
    // Top face
    {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.9f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.9f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.9f}},
    {{-0.5f, -0.5f,  0.5f}, {0.9f, 0.9f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.0f, 0.0f}},
    // Bottom face
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.9f, 0.9f}},
    {{-0.5f,  0.5f,  0.5f}, {0.9f, 0.0f, 0.9f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.1f, 0.9f, 0.1f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.9f, 0.9f}}
};

Renderer::Renderer(SDL_Window* window) {
    ctx_ = std::make_unique<VulkanContext>(window);

    create_allocator();

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = ctx_->graphics_queue_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(ctx_->device.device, &pool_info, nullptr, &command_pool_);

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(ctx_->device.device, &alloc_info, &command_buffer_);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateSemaphore(ctx_->device.device, &semaphore_info, nullptr, &image_available_semaphore_);
    vkCreateSemaphore(ctx_->device.device, &semaphore_info, nullptr, &render_finished_semaphore_);
    vkCreateFence(ctx_->device.device, &fence_info, nullptr, &in_flight_fence_);

    create_render_pass();
    create_framebuffers();
    create_pipeline();
    create_cube_buffer();
}

Renderer::~Renderer() {
    wait_idle();

    vmaDestroyBuffer(allocator_, vertex_buffer_, vertex_buffer_allocation_);

    vkDestroyPipeline(ctx_->device.device, graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(ctx_->device.device, pipeline_layout_, nullptr);

    for (auto fb : framebuffers_) {
        vkDestroyFramebuffer(ctx_->device.device, fb, nullptr);
    }
    vkDestroyRenderPass(ctx_->device.device, render_pass_, nullptr);

    vkDestroySemaphore(ctx_->device.device, image_available_semaphore_, nullptr);
    vkDestroySemaphore(ctx_->device.device, render_finished_semaphore_, nullptr);
    vkDestroyFence(ctx_->device.device, in_flight_fence_, nullptr);
    vkDestroyCommandPool(ctx_->device.device, command_pool_, nullptr);

    vmaDestroyAllocator(allocator_);
    ctx_.reset();
}

void Renderer::create_allocator() {
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = ctx_->device.physical_device;
    allocator_info.device = ctx_->device.device;
    allocator_info.instance = ctx_->instance.instance;
    vmaCreateAllocator(&allocator_info, &allocator_);
}

void Renderer::create_render_pass() {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = ctx_->swapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    vkCreateRenderPass(ctx_->device.device, &render_pass_info, nullptr, &render_pass_);
}

void Renderer::create_framebuffers() {
    framebuffers_.resize(ctx_->swapchain_image_views.size());

    for (size_t i = 0; i < ctx_->swapchain_image_views.size(); i++) {
        VkImageView attachments[] = { ctx_->swapchain_image_views[i] };

        VkFramebufferCreateInfo fb_info = {};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass = render_pass_;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = attachments;
        fb_info.width = ctx_->swapchain.extent.width;
        fb_info.height = ctx_->swapchain.extent.height;
        fb_info.layers = 1;

        vkCreateFramebuffer(ctx_->device.device, &fb_info, nullptr, &framebuffers_[i]);
    }
}

VkShaderModule Renderer::load_shader_module(const char* filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error(std::string("Failed to open shader file: ") + filepath);

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = buffer.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shader_module;
    vkCreateShaderModule(ctx_->device.device, &create_info, nullptr, &shader_module);
    return shader_module;
}

void Renderer::create_pipeline() {
    std::string vert_path = std::string(SHADER_DIR) + "/simple.vert.spv";
    std::string frag_path = std::string(SHADER_DIR) + "/simple.frag.spv";

    VkShaderModule vert_shader = load_shader_module(vert_path.c_str());
    VkShaderModule frag_shader = load_shader_module(frag_path.c_str());

    VkPipelineShaderStageCreateInfo vert_stage = {};
    vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage.module = vert_shader;
    vert_stage.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage = {};
    frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage.module = frag_shader;
    frag_stage.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage, frag_stage};

    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(Vertex);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr_desc[2] = {};
    attr_desc[0].binding = 0;
    attr_desc[0].location = 0;
    attr_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attr_desc[0].offset = offsetof(Vertex, position);

    attr_desc[1].binding = 0;
    attr_desc[1].location = 1;
    attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attr_desc[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexAttributeDescriptions = attr_desc;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)ctx_->swapchain.extent.width;
    viewport.height = (float)ctx_->swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = ctx_->swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    VkPushConstantRange push_constant = {};
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant.offset = 0;
    push_constant.size = sizeof(PushConstant);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;

    vkCreatePipelineLayout(ctx_->device.device, &pipeline_layout_info, nullptr, &pipeline_layout_);

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipeline_layout_;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.subpass = 0;

    vkCreateGraphicsPipelines(ctx_->device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_);

    vkDestroyShaderModule(ctx_->device.device, frag_shader, nullptr);
    vkDestroyShaderModule(ctx_->device.device, vert_shader, nullptr);
}

void Renderer::create_cube_buffer() {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = sizeof(CUBE_VERTICES[0]) * CUBE_VERTICES.size();
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(allocator_, &buffer_info, &alloc_info, &vertex_buffer_, &vertex_buffer_allocation_, nullptr);

    void* data;
    vmaMapMemory(allocator_, vertex_buffer_allocation_, &data);
    memcpy(data, CUBE_VERTICES.data(), (size_t)buffer_info.size);
    vmaUnmapMemory(allocator_, vertex_buffer_allocation_);
}

void Renderer::wait_idle() {
    vkDeviceWaitIdle(ctx_->device.device);
}

void Renderer::BeginFrame() {
    vkWaitForFences(ctx_->device.device, 1, &in_flight_fence_, VK_TRUE, UINT64_MAX);

    VkResult acquire_result = vkAcquireNextImageKHR(ctx_->device.device, ctx_->swapchain.swapchain, UINT64_MAX, image_available_semaphore_, VK_NULL_HANDLE, &current_image_index_);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
        // Simple bypass for M1 on resize.
        return;
    }

    vkResetFences(ctx_->device.device, 1, &in_flight_fence_);
    vkResetCommandBuffer(command_buffer_, 0);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer_, &begin_info);

    VkClearValue clear_color = {{{0.005f, 0.005f, 0.008f, 1.0f}}};

    VkRenderPassBeginInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_info.renderPass = render_pass_;
    rp_info.framebuffer = framebuffers_[current_image_index_];
    rp_info.renderArea.offset = {0, 0};
    rp_info.renderArea.extent = ctx_->swapchain.extent;
    rp_info.clearValueCount = 1;
    rp_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer_, &rp_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer_, 0, 1, &vertex_buffer_, offsets);

    is_frame_started_ = true;
}

void Renderer::DrawEntity(const glm::mat4& mvp) {
    if (!is_frame_started_) return;

    PushConstant pc;
    pc.mvp = mvp;
    vkCmdPushConstants(command_buffer_, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &pc);
    vkCmdDraw(command_buffer_, static_cast<uint32_t>(CUBE_VERTICES.size()), 1, 0, 0);
}

void Renderer::EndFrame() {
    if (!is_frame_started_) return;

    vkCmdEndRenderPass(command_buffer_);
    vkEndCommandBuffer(command_buffer_);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphore_};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer_;

    VkSemaphore signal_semaphores[] = {render_finished_semaphore_};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(ctx_->graphics_queue, 1, &submit_info, in_flight_fence_);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {ctx_->swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &current_image_index_;

    vkQueuePresentKHR(ctx_->graphics_queue, &present_info);

    is_frame_started_ = false;
}

} // namespace render