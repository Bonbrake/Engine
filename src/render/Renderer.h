#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>
#include "VulkanContext.h"

namespace render {

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct PushConstant {
    glm::mat4 mvp;
};

class Renderer {
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void BeginFrame();
    void EndFrame();
    void DrawEntity(const glm::mat4& mvp);

    void wait_idle();

private:
    void create_allocator();
    void create_render_pass();
    void create_framebuffers();
    void create_pipeline();
    void create_cube_buffer();

    VkShaderModule load_shader_module(const char* filepath);

    std::unique_ptr<VulkanContext> ctx_;
    VmaAllocator allocator_;

    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;

    VkSemaphore image_available_semaphore_;
    VkSemaphore render_finished_semaphore_;
    VkFence in_flight_fence_;

    VkRenderPass render_pass_;
    std::vector<VkFramebuffer> framebuffers_;

    VkPipelineLayout pipeline_layout_;
    VkPipeline graphics_pipeline_;

    VkBuffer vertex_buffer_;
    VmaAllocation vertex_buffer_allocation_;

    uint32_t current_image_index_{0};
    bool is_frame_started_{false};
};

} // namespace render