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

// Position is carried as double so the authoritative dvec3 world position from
// Transform is never truncated at world scale (>~1-2 km). CELL_SIZE is a float
// literal for the cell math but the inputs/keys are double.
inline uint64_t SpatialHashKey(double x, double z) {
    int64_t cx = static_cast<int64_t>(std::floor(x / 2.0));
    int64_t cz = static_cast<int64_t>(std::floor(z / 2.0));
    // Two's-complement cast: well-defined, preserves unique bit pattern for negative
    // cells (world spans negative coords around origin). Left-shifting a signed NEGATIVE
    // value is UB, so widen via uint32_t first.
    uint32_t ix = static_cast<uint32_t>(cx);
    uint32_t iz = static_cast<uint32_t>(cz);
    return (static_cast<uint64_t>(ix) << 32) | static_cast<uint64_t>(iz);
}

inline uint64_t ComputeSubdividedCellKey(uint64_t parentSpatialKey, int32_t subX, int32_t subZ) {
    uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03);
    return (parentSpatialKey << 4) | static_cast<uint64_t>(leafOffset & 0x0Full);
}

class SpatialHash {
public:
    static constexpr float CELL_SIZE = 2.0f;
    static constexpr uint32_t SUBDIVISION_THRESHOLD = 64;

    void Insert(entt::entity entity, double x, double z, entt::registry* registry = nullptr);
    void Remove(entt::entity entity, double x, double z);
    void Update(entt::entity entity, double oldX, double oldZ, double newX, double newZ, entt::registry* registry = nullptr);

    void Clear();

    // [M1-EXT-08] Quadtree Re-Bucketing
    void Rebucket(entt::registry& registry);

    // Query APIs
    std::vector<entt::entity> QueryRadius(double x, double z, double radius) const;
    std::vector<entt::entity> QueryCell(int32_t cx, int32_t cz) const;

private:
    std::unordered_map<uint64_t, SpatialCellHeader> headers;
    std::unordered_map<uint64_t, std::vector<entt::entity>> buckets;
    std::unordered_map<entt::entity, std::pair<double, double>> entityPositions;

    uint64_t GetEntityTargetKey(double x, double z, bool isSubdivided) const;
    std::pair<int32_t, int32_t> GetSubquadrantCoords(double x, double z) const;
};

} // namespace ecs
