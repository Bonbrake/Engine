#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <cstdint>
#include <vk_mem_alloc.h>
#include "../ecs/GenerationalTable.h"

#include <glm/glm.hpp>

namespace render {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct CompressedVertex {
    uint16_t posX; // Quantized position [-100.0, 100.0] -> [0, 65535]
    uint16_t posY;
    uint16_t posZ;
    uint16_t uvX;  // Quantized UV [0.0, 1.0] -> [0, 65535]
    uint16_t uvY;
    uint8_t octNormalX; // Octahedron-encoded normal
    uint8_t octNormalY;
    uint8_t padding[2]; // Align to 12 bytes
};

inline CompressedVertex CompressVertex(const Vertex& v) {
    CompressedVertex cv{};
    // Quantize position between -100.0f and 100.0f
    float x = glm::clamp((v.position.x + 100.0f) / 200.0f, 0.0f, 1.0f);
    float y = glm::clamp((v.position.y + 100.0f) / 200.0f, 0.0f, 1.0f);
    float z = glm::clamp((v.position.z + 100.0f) / 200.0f, 0.0f, 1.0f);
    cv.posX = static_cast<uint16_t>(x * 65535.0f);
    cv.posY = static_cast<uint16_t>(y * 65535.0f);
    cv.posZ = static_cast<uint16_t>(z * 65535.0f);

    // Quantize UV
    float u = glm::clamp(v.uv.x, 0.0f, 1.0f);
    float w = glm::clamp(v.uv.y, 0.0f, 1.0f);
    cv.uvX = static_cast<uint16_t>(u * 65535.0f);
    cv.uvY = static_cast<uint16_t>(w * 65535.0f);

    // Octahedron-encoded normal
    glm::vec3 n = glm::normalize(v.normal);
    float l1 = std::abs(n.x) + std::abs(n.y) + std::abs(n.z);
    glm::vec2 p(n.x, n.y);
    if (l1 > 0.0f) {
        p /= l1;
    }
    if (n.z < 0.0f) {
        glm::vec2 signVal = glm::sign(p);
        p = (1.0f - glm::abs(glm::vec2(p.y, p.x))) * signVal;
    }
    p = p * 0.5f + 0.5f;
    cv.octNormalX = static_cast<uint8_t>(glm::clamp(p.x, 0.0f, 1.0f) * 255.0f);
    cv.octNormalY = static_cast<uint8_t>(glm::clamp(p.y, 0.0f, 1.0f) * 255.0f);

    return cv;
}

inline Vertex DecompressVertex(const CompressedVertex& cv) {
    Vertex v{};
    v.position.x = (static_cast<float>(cv.posX) / 65535.0f) * 200.0f - 100.0f;
    v.position.y = (static_cast<float>(cv.posY) / 65535.0f) * 200.0f - 100.0f;
    v.position.z = (static_cast<float>(cv.posZ) / 65535.0f) * 200.0f - 100.0f;

    v.uv.x = static_cast<float>(cv.uvX) / 65535.0f;
    v.uv.y = static_cast<float>(cv.uvY) / 65535.0f;

    // Octahedron decode
    glm::vec2 p(static_cast<float>(cv.octNormalX) / 255.0f, static_cast<float>(cv.octNormalY) / 255.0f);
    p = p * 2.0f - 1.0f;
    glm::vec3 n(p.x, p.y, 1.0f - std::abs(p.x) - std::abs(p.y));
    if (n.z < 0.0f) {
        glm::vec2 signVal = glm::sign(glm::vec2(n.x, n.y));
        glm::vec2 temp = (1.0f - glm::abs(glm::vec2(n.y, n.x))) * signVal;
        n.x = temp.x;
        n.y = temp.y;
    }
    v.normal = glm::normalize(n);
    return v;
}

struct MeshAsset {
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation vertexAllocation = VK_NULL_HANDLE;
    
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VmaAllocation indexAllocation = VK_NULL_HANDLE;
    
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
};

struct TextureAsset {
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

// [M1-EXT] Basic Material Representation
struct MaterialAsset {
    ecs::Handle albedoTexture;
    float baseColorFactor[4] = {1.f, 1.f, 1.f, 1.f};
    float metallicFactor = 1.f;
    float roughnessFactor = 1.f;
};

} // namespace render
