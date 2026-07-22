#include "ze/audio/AudioPropagation.h"
#include "ze/audio/AudioEngine.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <limits>

namespace audio {

// ─── DiffractionCache ──────────────────────────────────────────────────

void DiffractionCache::addNode(uint32_t probeId, const glm::vec3& pos,
                                const glm::vec3& edgeDir, float atten) {
    nodes_.push_back({probeId, pos, glm::normalize(edgeDir), atten});
}

DiffractionNode DiffractionCache::findNearest(uint32_t probeId,
                                               const glm::vec3& queryPos) const {
    DiffractionNode best{};
    float bestDist = std::numeric_limits<float>::max();
    for (const auto& n : nodes_) {
        if (n.probeId != probeId) continue;
        float d = glm::distance(n.worldPos, queryPos);
        if (d < bestDist) {
            bestDist = d;
            best = n;
        }
    }
    return best;
}

void DiffractionCache::clear() {
    nodes_.clear();
}

// ─── PortalWeaver ───────────────────────────────────────────────────────

PortalWeaver::VoxelCoord PortalWeaver::worldToVoxel(glm::vec3 pos) const {
    int x = static_cast<int>(std::floor(pos.x / voxelWorldSize_));
    int y = static_cast<int>(std::floor(pos.y / voxelWorldSize_));
    int z = static_cast<int>(std::floor(pos.z / voxelWorldSize_));
    return {x, y, z};
}

uint32_t PortalWeaver::flatten(glm::ivec3 c) const {
    // convert to size_t before multiplication to avoid overflow
    return static_cast<uint32_t>(
        static_cast<size_t>(c.z) * static_cast<size_t>(bounds_.y * bounds_.x) +
        static_cast<size_t>(c.y) * static_cast<size_t>(bounds_.x) +
        static_cast<size_t>(c.x));
}

void PortalWeaver::floodFill(uint32_t startIdx, uint32_t zoneId, const uint8_t* grid) {
    // Simple 6-direction flood fill through air cells
    // (grid value 0 = air/empty, non-0 = solid)
    std::queue<uint32_t> q;
    q.push(startIdx);
    voxelZoneMap_[startIdx] = zoneId;

    constexpr int dirs[6][3] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};

    while (!q.empty()) {
        uint32_t idx = q.front(); q.pop();
        int z = idx / (bounds_.y * bounds_.x);
        int y = (idx % (bounds_.y * bounds_.x)) / bounds_.x;
        int x = idx % bounds_.x;

        for (int d = 0; d < 6; ++d) {
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];
            int nz = z + dirs[d][2];
            if (nx < 0 || nx >= bounds_.x || ny < 0 || ny >= bounds_.y || nz < 0 || nz >= bounds_.z)
                continue;
            uint32_t nidx = flatten({nx, ny, nz});
            if (voxelZoneMap_[nidx] == 0 && grid[nidx] == 0) {
                voxelZoneMap_[nidx] = zoneId;
                q.push(nidx);
            }
        }
    }
}

void PortalWeaver::buildFromVoxelGrid(const uint8_t* voxelGrid, glm::ivec3 bounds,
                                       float voxelWorldSize) {
    bounds_ = bounds;
    voxelWorldSize_ = voxelWorldSize;
    size_t totalVoxels = static_cast<size_t>(bounds.x) * static_cast<size_t>(bounds.y) * static_cast<size_t>(bounds.z);
    voxelZoneMap_.resize(totalVoxels, 0);
    portals_.clear();
    zones_.clear();

    // Flood-fill each unassigned air voxel
    uint32_t currentZone = 1; // 0 = solid/unassigned
    for (size_t i = 0; i < totalVoxels; ++i) {
        if (voxelGrid[i] == 0 && voxelZoneMap_[i] == 0) {
            floodFill(static_cast<uint32_t>(i), currentZone, voxelGrid);
            AcousticZone zone;
            zone.id = currentZone;
            zone.dominantMaterial = AcousticMaterial::Concrete; // default
            zones_.push_back(zone);
            ++currentZone;
        }
    }

    // Portal detection: adjacent air voxels in different zones
    constexpr int dirs[3] = {1, 0, 0}; // sweep X for simplicity
    for (int z = 0; z < bounds_.z; ++z) {
        for (int y = 0; y < bounds_.y; ++y) {
            for (int x = 0; x < bounds_.x; ++x) {
                uint32_t idx = flatten({x, y, z});
                if (voxelGrid[idx] != 0) continue;
                uint32_t zoneA = voxelZoneMap_[idx];
                // Check +X neighbor
                if (x + 1 < bounds_.x) {
                    uint32_t nidx = flatten({x + 1, y, z});
                    uint32_t zoneB = voxelZoneMap_[nidx];
                    if (zoneA != zoneB && voxelGrid[nidx] == 0) {
                        // Found portal between zones
                        AcousticPortal portal;
                        portal.position = glm::vec3(
                            (x + 0.5f) * voxelWorldSize_,
                            (y + 0.5f) * voxelWorldSize_,
                            (z + 0.5f) * voxelWorldSize_);
                        portal.faceNormal = glm::vec3(-1, 0, 0); // facing -X
                        portal.apertureArea = voxelWorldSize_ * voxelWorldSize_;
                        portal.zoneA = zoneA;
                        portal.zoneB = zoneB;
                        portals_.push_back(portal);
                    }
                }
            }
        }
    }
}

uint32_t PortalWeaver::getZoneAt(glm::vec3 worldPos) const {
    VoxelCoord vc = worldToVoxel(worldPos);
    if (vc.x < 0 || vc.x >= bounds_.x || vc.y < 0 || vc.y >= bounds_.y || vc.z < 0 || vc.z >= bounds_.z)
        return 0;
    uint32_t idx = flatten({vc.x, vc.y, vc.z});
    if (idx >= voxelZoneMap_.size()) return 0;
    return voxelZoneMap_[idx];
}

float PortalWeaver::getPortalAttenuation(glm::vec3 from, glm::vec3 to) const {
    uint32_t fromZone = getZoneAt(from);
    uint32_t toZone = getZoneAt(to);
    if (fromZone == 0 || toZone == 0 || fromZone == toZone) return 1.0f;

    // Find portal connecting the two zones
    for (const auto& portal : portals_) {
        if ((portal.zoneA == fromZone && portal.zoneB == toZone) ||
            (portal.zoneA == toZone && portal.zoneB == fromZone)) {
            glm::vec3 soundDir = glm::normalize(to - from);
            float dist = glm::distance(from, to);
            return calcPortalAttenuation(portal.apertureArea, soundDir,
                                          portal.faceNormal, dist);
        }
    }
    return 0.0f; // no portal path → fully occluded
}

void PortalWeaver::onDestruction(glm::vec3 worldPos) {
    // Mark voxel as air; next buildFromVoxelGrid will re-run
    VoxelCoord vc = worldToVoxel(worldPos);
    if (vc.x >= 0 && vc.x < bounds_.x && vc.y >= 0 && vc.y < bounds_.y && vc.z >= 0 && vc.z < bounds_.z) {
        uint32_t idx = flatten({vc.x, vc.y, vc.z});
        if (idx < voxelZoneMap_.size() && voxelZoneMap_[idx] != 0) {
            voxelZoneMap_[idx] = 0; // mark for rebuild
        }
    }
}

} // namespace audio
