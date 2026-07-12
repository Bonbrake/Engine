#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <entt/entt.hpp>
#include <cmath>
#include <span>

namespace ecs {

// [M1-EXT-08] Dynamic Spatial Hash Cell Quadtree Subdivision Splitter
struct SpatialCellHeader {
    uint32_t entityCount{0};
    uint32_t subQuadrantMask{0}; // bitmask flag tracking populated leaf entries
    bool isCellSubdivided{false};
};

inline uint64_t SpatialHashKey(float x, float z) {
    int64_t cx = static_cast<int64_t>(std::floor(x / 2.0f));
    int64_t cz = static_cast<int64_t>(std::floor(z / 2.0f));
    return (static_cast<uint64_t>(cx) << 32) | (static_cast<uint64_t>(cz) & 0xFFFFFFFFull);
}

inline uint64_t ComputeSubdividedCellKey(uint64_t parentSpatialKey, int32_t subX, int32_t subZ) {
    uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03);
    return (parentSpatialKey << 4) | static_cast<uint64_t>(leafOffset & 0x0Full);
}

class SpatialHash {
public:
    static constexpr float CELL_SIZE = 2.0f;
    static constexpr uint32_t SUBDIVISION_THRESHOLD = 64;

    void Insert(entt::entity entity, float x, float z, entt::registry* registry = nullptr);
    void Remove(entt::entity entity, float x, float z);
    void Update(entt::entity entity, float oldX, float oldZ, float newX, float newZ, entt::registry* registry = nullptr);

    void Clear();

    // [M1-EXT-08] Quadtree Re-Bucketing
    void Rebucket(entt::registry& registry);

    // Query APIs
    std::vector<entt::entity> QueryRadius(float x, float z, float radius) const;
    std::vector<entt::entity> QueryCell(int32_t cx, int32_t cz) const;

private:
    std::unordered_map<uint64_t, SpatialCellHeader> headers;
    std::unordered_map<uint64_t, std::vector<entt::entity>> buckets;
    std::unordered_map<entt::entity, std::pair<float, float>> entityPositions;

    uint64_t GetEntityTargetKey(float x, float z, bool isSubdivided) const;
    std::pair<int32_t, int32_t> GetSubquadrantCoords(float x, float z) const;
};

} // namespace ecs
