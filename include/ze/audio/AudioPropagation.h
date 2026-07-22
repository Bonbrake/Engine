#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "ze/audio/AudioEngine.h"

namespace audio {

// Represents a cached acoustic diffraction node (M6-EXT-08)
struct DiffractionNode {
    uint32_t probeId;
    glm::vec3 worldPos;
    glm::vec3 edgeDir;
    float attenuation; // UTD-based diffraction attenuation (0-1)
};

// Cached diffracted node topology; reused across frames.
// diffraction = UTD(node); cache node topology per probe; reuse across frames.
class DiffractionCache {
public:
    void addNode(uint32_t probeId, const glm::vec3& pos, const glm::vec3& edgeDir, float atten);
    DiffractionNode findNearest(uint32_t probeId, const glm::vec3& queryPos) const;
    void clear();
    size_t size() const { return nodes_.size(); }

private:
    std::vector<DiffractionNode> nodes_;
};

// Portal data for voxel acoustic propagation (M6-EXT-04)
struct AcousticPortal {
    glm::vec3 position;
    glm::vec3 faceNormal; // points into the room
    float apertureArea;   // m²
    uint32_t zoneA, zoneB; // connected zone indices
};

// Acoustic zone: a connected volume of air separated by portals
struct AcousticZone {
    uint32_t id;
    AcousticMaterial dominantMaterial;
    std::vector<uint32_t> portalIndices;
};

// Automated portal weaver: flood-fills world into zones and maps portal connections
class PortalWeaver {
public:
    PortalWeaver() = default;

    // Run flood-fill to discover zones and portals from voxel data
    // voxelGrid: 3D array of material tags (null = air, non-null = solid)
    // bounds: world-space extent of grid in voxels
    // voxelWorldSize: size of one voxel in meters
    void buildFromVoxelGrid(const uint8_t* voxelGrid, glm::ivec3 bounds, float voxelWorldSize);

    // Query zone for a world position
    uint32_t getZoneAt(glm::vec3 worldPos) const;

    // Query portal attenuation between two positions
    float getPortalAttenuation(glm::vec3 from, glm::vec3 to) const;

    // Update after destruction: remove portal if wall broken
    void onDestruction(glm::vec3 worldPos);

    const std::vector<AcousticPortal>& portals() const { return portals_; }
    const std::vector<AcousticZone>& zones() const { return zones_; }

private:
    std::vector<AcousticPortal> portals_;
    std::vector<AcousticZone> zones_;
    glm::ivec3 bounds_{};
    float voxelWorldSize_ = 1.0f;
    std::vector<uint32_t> voxelZoneMap_; // flattened 3D grid, 0 = solid/unassigned

    struct VoxelCoord { int x, y, z; };
    VoxelCoord worldToVoxel(glm::vec3 pos) const;
    uint32_t flatten(glm::ivec3 c) const;
    void floodFill(uint32_t startIdx, uint32_t zoneId, const uint8_t* grid);
};

} // namespace audio
