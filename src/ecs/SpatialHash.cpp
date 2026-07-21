#include "ze/ecs/SpatialHash.h"
#include "ze/ecs/Components.h"
#include <algorithm>

namespace ecs {

std::pair<int32_t, int32_t> SpatialHash::GetSubquadrantCoords(double x, double z) const {
    double remainderX = std::fmod(x, CELL_SIZE);
    double remainderZ = std::fmod(z, CELL_SIZE);
    if (remainderX < 0) remainderX += CELL_SIZE;
    if (remainderZ < 0) remainderZ += CELL_SIZE;

    int32_t subX = static_cast<int32_t>(remainderX / (CELL_SIZE / 4.0f));
    int32_t subZ = static_cast<int32_t>(remainderZ / (CELL_SIZE / 4.0f));
    return { subX, subZ };
}

uint64_t SpatialHash::GetEntityTargetKey(double x, double z, bool isSubdivided) const {
    uint64_t parentKey = SpatialHashKey(x, z);
    if (!isSubdivided) {
        return parentKey;
    }

    auto [subX, subZ] = GetSubquadrantCoords(x, z);
    return ComputeSubdividedCellKey(parentKey, subX, subZ);
}

void SpatialHash::Insert(entt::entity entity, double x, double z, entt::registry* registry) {
    uint64_t parentKey = SpatialHashKey(x, z);
    auto& header = headers[parentKey];

    header.entityCount++;
    entityPositions[entity] = {x, z};

    // Check if we need to subdivide
    if (!header.isCellSubdivided && header.entityCount > SUBDIVISION_THRESHOLD) {
        header.isCellSubdivided = true;

        // Re-bucket existing entities
        std::vector<entt::entity> tempEntities = std::move(buckets[parentKey]);
        buckets.erase(parentKey);

        for (entt::entity ent : tempEntities) {
            double px = 0.0, pz = 0.0;
            bool foundPos = false;

            // [M1-EXT-08] Recompute leaf quadrant using the entity's actual position if registry is provided
            if (registry && registry->valid(ent)) {
                auto* transform = registry->try_get<Transform>(ent);
                if (transform) {
                    // Carry the authoritative dvec3 position as double (no float narrowing).
                    px = transform->position.x;
                    pz = transform->position.z;
                    foundPos = true;
                    // Update cache to match actual position
                    entityPositions[ent] = {px, pz};
                }
            }

            // Fallback to cached position if no registry or transform
            if (!foundPos) {
                auto posIt = entityPositions.find(ent);
                if (posIt != entityPositions.end()) {
                    px = posIt->second.first;
                    pz = posIt->second.second;
                    foundPos = true;
                }
            }

            if (foundPos) {
                uint64_t targetKey = GetEntityTargetKey(px, pz, true);
                buckets[targetKey].push_back(ent);

                auto [subX, subZ] = GetSubquadrantCoords(px, pz);
                uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03);
                header.subQuadrantMask |= (1 << leafOffset);
            }
        }
    }

    uint64_t targetKey = GetEntityTargetKey(x, z, header.isCellSubdivided);
    buckets[targetKey].push_back(entity);

    if (header.isCellSubdivided) {
        auto [subX, subZ] = GetSubquadrantCoords(x, z);
        uint32_t leafOffset = ((static_cast<uint32_t>(subX) & 0x03) << 2) | (static_cast<uint32_t>(subZ) & 0x03);
        header.subQuadrantMask |= (1 << leafOffset);
    }
}

void SpatialHash::Remove(entt::entity entity, double x, double z) {
    uint64_t parentKey = SpatialHashKey(x, z);
    auto it = headers.find(parentKey);
    if (it == headers.end()) return;

    auto& header = it->second;
    header.entityCount--;

    uint64_t targetKey = GetEntityTargetKey(x, z, header.isCellSubdivided);
    auto& bucket = buckets[targetKey];

    auto entIt = std::find(bucket.begin(), bucket.end(), entity);
    if (entIt != bucket.end()) {
        bucket.erase(entIt);
    }
    entityPositions.erase(entity);
}

void SpatialHash::Update(entt::entity entity, double oldX, double oldZ, double newX, double newZ, entt::registry* registry) {
    Remove(entity, oldX, oldZ);
    Insert(entity, newX, newZ, registry);
}

void SpatialHash::Clear() {
    headers.clear();
    buckets.clear();
    entityPositions.clear();
}

void SpatialHash::Rebucket(entt::registry& registry) {
    std::vector<std::tuple<entt::entity, double, double, double, double>> toUpdate;
    std::vector<std::tuple<entt::entity, double, double>> toRemove;

    for (const auto& [entity, pos] : entityPositions) {
        if (!registry.valid(entity)) {
            toRemove.push_back({entity, pos.first, pos.second});
            continue;
        }
        auto* transform = registry.try_get<Transform>(entity);
        if (!transform) {
            toRemove.push_back({entity, pos.first, pos.second});
            continue;
        }

        // Carry the authoritative dvec3 position as double (no float narrowing).
        double newX = transform->position.x;
        double newZ = transform->position.z;
        if (newX != pos.first || newZ != pos.second) {
            toUpdate.push_back({entity, pos.first, pos.second, newX, newZ});
        }
    }

    for (const auto& rm : toRemove) {
        Remove(std::get<0>(rm), std::get<1>(rm), std::get<2>(rm));
    }
    for (const auto& update : toUpdate) {
        Update(std::get<0>(update), std::get<1>(update), std::get<2>(update), std::get<3>(update), std::get<4>(update), &registry);
    }
}

std::vector<entt::entity> SpatialHash::QueryCell(int32_t cx, int32_t cz) const {
    uint64_t parentKey = (static_cast<uint64_t>(cx) << 32) | (static_cast<uint64_t>(cz) & 0xFFFFFFFFull);
    std::vector<entt::entity> result;

    auto it = headers.find(parentKey);
    if (it == headers.end()) return result;

    if (!it->second.isCellSubdivided) {
        auto bucketIt = buckets.find(parentKey);
        if (bucketIt != buckets.end()) {
            result = bucketIt->second;
        }
    } else {
        // Gather from all active sub-quadrants
        for (uint32_t i = 0; i < 16; ++i) {
            if (it->second.subQuadrantMask & (1 << i)) {
                uint64_t subKey = (parentKey << 4) | i;
                auto bucketIt = buckets.find(subKey);
                if (bucketIt != buckets.end()) {
                    result.insert(result.end(), bucketIt->second.begin(), bucketIt->second.end());
                }
            }
        }
    }

    return result;
}

std::vector<entt::entity> SpatialHash::QueryRadius(double x, double z, double radius) const {
    std::vector<entt::entity> result;

    int32_t minCx = static_cast<int32_t>(std::floor((x - radius) / CELL_SIZE));
    int32_t maxCx = static_cast<int32_t>(std::floor((x + radius) / CELL_SIZE));
    int32_t minCz = static_cast<int32_t>(std::floor((z - radius) / CELL_SIZE));
    int32_t maxCz = static_cast<int32_t>(std::floor((z + radius) / CELL_SIZE));

    for (int32_t cx = minCx; cx <= maxCx; ++cx) {
        for (int32_t cz = minCz; cz <= maxCz; ++cz) {
            auto cellResult = QueryCell(cx, cz);
            result.insert(result.end(), cellResult.begin(), cellResult.end());
        }
    }

    // NOTE: This returns all entities in overlapping cells.
    // Consumer code should do exact distance filtering.
    return result;
}

} // namespace ecs