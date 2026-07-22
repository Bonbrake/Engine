#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <functional>
#include "ze/world/BiomeGraph.h"

namespace world {

// A single world chunk: 256×256 m of generated geometry
struct Chunk {
    uint32_t chunkX, chunkZ;
    BiomeType biome;
    float minHeight, maxHeight;
    bool loaded = false;
    bool dirty = false;
    uint32_t lodLevel = 0; // 0=full, 1=half, 2=quarter

    // Mesh handles (assigned by render system)
    uint64_t meshHandle = 0;
    uint64_t collisionHandle = 0;

    // Stream state
    float distanceToCamera = 0.0f;
    uint64_t lastAccessFrame = 0;
};

// Macro-graph node: a point on the road/river network (M4-EXT-01)
struct MacroNode {
    glm::vec3 position;
    float width = 10.0f;       // road/river width in meters
    bool isRiver = false;       // true if water, false if road
    uint32_t nodeId;
    std::vector<uint32_t> connections; // adjacent node IDs
};

// Chunk streamer: manages loading/unloading of world chunks
// Integrates macro-graph constraints (M4-EXT-01), WFC (M4-EXT-03),
// Voronoi seam welding (M4-EXT-04), and LOD transitions (M4-EXT-05).
class ChunkStreamer {
public:
    ChunkStreamer(uint32_t seed, uint32_t worldSize, const BiomeGraph* biomeGraph);

    // Update streaming based on camera position
    void update(glm::vec3 cameraPos, uint64_t frameCount, float deltaTime);

    // Chunk lifecycle
    void loadChunk(uint32_t chunkX, uint32_t chunkZ);
    void unloadChunk(uint32_t chunkX, uint32_t chunkZ);
    void requestChunk(uint32_t chunkX, uint32_t chunkZ);

    // Query
    Chunk* getChunk(uint32_t chunkX, uint32_t chunkZ);
    const Chunk* getChunk(uint32_t chunkX, uint32_t chunkZ) const;
    const std::unordered_map<uint64_t, Chunk>& loadedChunks() const { return chunks_; }
    const std::vector<MacroNode>& macroGraph() const { return macroGraph_; }
    const BiomeGraph* biomeGraph() const { return biomeGraph_; }

    // Macro-graph: generate the global road/river network (M4-EXT-01)
    void generateMacroGraph();

    // WFC: generate chunk content (M4-EXT-03)
    // Callback receives per-cell tile type: 0=empty, 1=ground, 2=wall, 3=roof
    using TileCallback = std::function<void(uint32_t cellX, uint32_t cellZ, int tileType)>;
    void generateWFC(uint32_t chunkX, uint32_t chunkZ, uint32_t cellSize,
                     TileCallback onTile);

    // Voronoi seam metal-weld (M4-EXT-04, M4-EXT-05)
    void weldSeam(Chunk& chunk, const Chunk& neighbor, bool isXAxis);

    // LOD management
    void setLOD(uint32_t chunkX, uint32_t chunkZ, uint32_t level);
    uint32_t computeLOD(float distance) const;

    // Configuration
    void setViewDistance(float meters) { viewDistance_ = meters; }
    void setLODDistances(float near, float mid, float far) {
        lodNear_ = near; lodMid_ = mid; lodFar_ = far;
    }

private:
    uint32_t seed_;
    uint32_t worldSize_;
    const BiomeGraph* biomeGraph_;
    float viewDistance_ = 1000.0f;
    float lodNear_ = 200.0f;
    float lodMid_ = 500.0f;
    float lodFar_ = 800.0f;

    // Loaded chunks (key = pack(chunkX, chunkZ))
    std::unordered_map<uint64_t, Chunk> chunks_;

    // Macro-graph: road/river network
    std::vector<MacroNode> macroGraph_;

    // Pending load/unload queues
    std::vector<std::pair<uint32_t, uint32_t>> loadQueue_;
    std::vector<uint64_t> unloadQueue_;

    // Helpers
    static uint64_t packCoords(uint32_t x, uint32_t z) {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(z);
    }
    static std::pair<uint32_t, uint32_t> unpackCoords(uint64_t key) {
        return {static_cast<uint32_t>(key >> 32), static_cast<uint32_t>(key & 0xFFFFFFFF)};
    }
};

} // namespace world
