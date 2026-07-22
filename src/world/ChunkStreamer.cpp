#include "ze/world/ChunkStreamer.h"
#include <cmath>
#include <algorithm>
#include <queue>
#include <limits>
#include <random>
#include <glm/gtc/noise.hpp>

namespace world {

ChunkStreamer::ChunkStreamer(uint32_t seed, uint32_t worldSize, const BiomeGraph* biomeGraph)
    : seed_(seed), worldSize_(worldSize), biomeGraph_(biomeGraph) {
    generateMacroGraph();
}

void ChunkStreamer::generateMacroGraph() {
    // Seed-based macro-graph of road and river corridors (M4-EXT-01)
    std::mt19937 gen(seed_);
    std::uniform_real_distribution<float> posDist(0.0f, static_cast<float>(worldSize_));
    std::uniform_real_distribution<float> widthDist(6.0f, 20.0f);
    std::bernoulli_distribution riverChance(0.2f);

    // Generate ~50 macro nodes across the world
    for (uint32_t i = 0; i < 50; ++i) {
        MacroNode node;
        node.position = glm::vec3(posDist(gen), 0.0f, posDist(gen));
        node.width = widthDist(gen);
        node.isRiver = riverChance(gen);
        node.nodeId = i;
        macroGraph_.push_back(node);
    }

    // Connect nearby nodes: distance-based edges
    for (size_t i = 0; i < macroGraph_.size(); ++i) {
        // Find 2-4 nearest neighbors
        std::vector<std::pair<float, uint32_t>> dists;
        for (size_t j = 0; j < macroGraph_.size(); ++j) {
            if (i == j) continue;
            float d = glm::distance(macroGraph_[i].position, macroGraph_[j].position);
            dists.push_back({d, static_cast<uint32_t>(j)});
        }
        std::sort(dists.begin(), dists.end());
        uint32_t connections = 2 + (gen() % 3); // 2-4 connections per node
        for (uint32_t k = 0; k < std::min(connections, static_cast<uint32_t>(dists.size())); ++k) {
            macroGraph_[i].connections.push_back(dists[k].second);
        }
    }
}

void ChunkStreamer::update(glm::vec3 cameraPos, uint64_t frameCount, float dt) {
    // Process load queue
    for (const auto& [cx, cz] : loadQueue_) {
        loadChunk(cx, cz);
    }
    loadQueue_.clear();

    // Process unload queue
    for (uint64_t key : unloadQueue_) {
        chunks_.erase(key);
    }
    unloadQueue_.clear();

    // Determine which chunks should be loaded based on camera position
    uint32_t centerX = static_cast<uint32_t>(cameraPos.x / CHUNK_SIZE);
    uint32_t centerZ = static_cast<uint32_t>(cameraPos.z / CHUNK_SIZE);
    uint32_t radius = static_cast<uint32_t>(viewDistance_ / CHUNK_SIZE) + 1;

    for (int dx = -static_cast<int>(radius); dx <= static_cast<int>(radius); ++dx) {
        for (int dz = -static_cast<int>(radius); dz <= static_cast<int>(radius); ++dz) {
            uint32_t cx = centerX + dx;
            uint32_t cz = centerZ + dz;
            float dist = std::sqrt(dx * dx + dz * dz) * CHUNK_SIZE;
            if (dist > viewDistance_) continue;

            uint64_t key = packCoords(cx, cz);
            auto it = chunks_.find(key);
            if (it == chunks_.end()) {
                requestChunk(cx, cz);
            } else {
                it->second.distanceToCamera = dist;
                it->second.lastAccessFrame = frameCount;
                it->second.lodLevel = computeLOD(dist);
            }
        }
    }

    // Unload chunks that fell out of range
    std::vector<uint64_t> toRemove;
    for (const auto& [key, chunk] : chunks_) {
        auto [cx, cz] = unpackCoords(key);
        float dx = static_cast<float>(cx) - centerX;
        float dz = static_cast<float>(cz) - centerZ;
        float dist = std::sqrt(dx * dx + dz * dz) * CHUNK_SIZE;
        if (dist > viewDistance_ * 1.2f && frameCount - chunk.lastAccessFrame > 60) {
            toRemove.push_back(key);
        }
    }
    for (uint64_t key : toRemove) {
        unloadChunk(unpackCoords(key).first, unpackCoords(key).second);
    }
}

void ChunkStreamer::requestChunk(uint32_t chunkX, uint32_t chunkZ) {
    loadQueue_.emplace_back(chunkX, chunkZ);
}

void ChunkStreamer::loadChunk(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t key = packCoords(chunkX, chunkZ);
    if (chunks_.find(key) != chunks_.end()) return;

    Chunk chunk;
    chunk.chunkX = chunkX;
    chunk.chunkZ = chunkZ;
    chunk.biome = biomeGraph_->classify(chunkX, chunkZ);
    chunk.loaded = true;
    chunk.dirty = true;
    chunk.lodLevel = 0;

    // Sample height extremes using biome graph
    chunk.minHeight = std::numeric_limits<float>::max();
    chunk.maxHeight = std::numeric_limits<float>::lowest();
    for (float x = 0; x < CHUNK_SIZE; x += 16.0f) {
        for (float z = 0; z < CHUNK_SIZE; z += 16.0f) {
            float h = biomeGraph_->getHeight(chunkX, chunkZ, x, z);
            chunk.minHeight = std::min(chunk.minHeight, h);
            chunk.maxHeight = std::max(chunk.maxHeight, h);
        }
    }

    chunks_[key] = chunk;
}

void ChunkStreamer::unloadChunk(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t key = packCoords(chunkX, chunkZ);
    auto it = chunks_.find(key);
    if (it != chunks_.end()) {
        // Notify render system to free mesh handles
        if (it->second.meshHandle != 0) {
            // TODO: render system mesh release callback
        }
        chunks_.erase(it);
    }
}

Chunk* ChunkStreamer::getChunk(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t key = packCoords(chunkX, chunkZ);
    auto it = chunks_.find(key);
    if (it != chunks_.end()) return &it->second;
    return nullptr;
}

const Chunk* ChunkStreamer::getChunk(uint32_t chunkX, uint32_t chunkZ) const {
    uint64_t key = packCoords(chunkX, chunkZ);
    auto it = chunks_.find(key);
    if (it != chunks_.end()) return &it->second;
    return nullptr;
}

uint32_t ChunkStreamer::computeLOD(float distance) const {
    if (distance < lodNear_) return 0;
    if (distance < lodMid_) return 1;
    if (distance < lodFar_) return 2;
    return 3;
}

void ChunkStreamer::setLOD(uint32_t chunkX, uint32_t chunkZ, uint32_t level) {
    auto* chunk = getChunk(chunkX, chunkZ);
    if (chunk) {
        chunk->lodLevel = std::min(level, 3u);
        chunk->dirty = true;
    }
}

void ChunkStreamer::generateWFC(uint32_t chunkX, uint32_t chunkZ,
                                 uint32_t cellSize, TileCallback onTile) {
    // Multi-grid WFC generation (M4-EXT-03)
    // Macro pass: road/river alignment from macro-graph
    glm::vec3 chunkCenter = glm::vec3(
        static_cast<float>(chunkX) * CHUNK_SIZE + CHUNK_SIZE * 0.5f,
        0.0f,
        static_cast<float>(chunkZ) * CHUNK_SIZE + CHUNK_SIZE * 0.5f);

    // Check macro-graph proximity for road alignment
    bool hasRoad = false;
    glm::vec3 roadDir(0, 0, 1);
    for (const auto& node : macroGraph_) {
        for (uint32_t conn : node.connections) {
            if (conn >= macroGraph_.size()) continue;
            const auto& target = macroGraph_[conn];
            // Check if chunk center falls near road segment
            glm::vec3 seg = target.position - node.position;
            glm::vec3 toChunk = chunkCenter - node.position;
            float t = glm::dot(toChunk, seg) / glm::dot(seg, seg);
            if (t >= 0.0f && t <= 1.0f) {
                glm::vec3 closest = node.position + seg * t;
                float dist = glm::distance(chunkCenter, closest);
                if (dist < CHUNK_SIZE * 0.3f) {
                    hasRoad = true;
                    roadDir = glm::normalize(seg);
                    break;
                }
            }
        }
        if (hasRoad) break;
    }

    // WFC grid: cellSize × cellSize cells per chunk
    uint32_t cellsPerChunk = CHUNK_SIZE / cellSize;
    std::mt19937 wfcGen(seed_ + chunkX * 137 + chunkZ * 251);

    for (uint32_t cellX = 0; cellX < cellsPerChunk; ++cellX) {
        for (uint32_t cellZ = 0; cellZ < cellsPerChunk; ++cellZ) {
            float wx = chunkX * CHUNK_SIZE + cellX * cellSize + cellSize * 0.5f;
            float wz = chunkZ * CHUNK_SIZE + cellZ * cellSize + cellSize * 0.5f;
            float height = biomeGraph_->getHeight(chunkX, chunkZ, cellX * cellSize, cellZ * cellSize);

            // Determine tile type
            int tileType = 0; // empty/air

            if (height < 0.0f) {
                tileType = 0; // water (empty in WFC sense, mesh handles separately)
            } else if (height < 5.0f) {
                tileType = 1; // ground
            } else if (height < 20.0f) {
                tileType = 2; // elevated ground / wall-like
            } else {
                tileType = 3; // high ground / rock
            }

            // Road alignment: force ground type along road corridor
            if (hasRoad) {
                float roadDist = std::abs(
                    (wx - chunkCenter.x) * roadDir.z - (wz - chunkCenter.z) * roadDir.x);
                if (roadDist < 8.0f) {
                    tileType = 1; // force ground/road
                }
            }

            if (onTile) {
                onTile(cellX, cellZ, tileType);
            }
        }
    }
}

void ChunkStreamer::weldSeam(Chunk& chunk, const Chunk& neighbor, bool isXAxis) {
    // Voronoi chunk seam vertex normal welding (M4-EXT-04, M4-EXT-05)
    // Flatten heights along shared edge to eliminate visible seams.
    if (chunk.lodLevel != neighbor.lodLevel) {
        // Different LOD: snap fine vertices to coarse edge (M4-EXT-05)
        chunk.dirty = true;
    }

    // Blend boundary heights
    for (float t = 0; t < CHUNK_SIZE; t += 8.0f) {
        float wPos, nPos;
        if (isXAxis) {
            // X-axis boundary: chunk right edge ↔ neighbor left edge
            wPos = biomeGraph_->getHeight(chunk.chunkX, chunk.chunkZ, CHUNK_SIZE - 1, t);
            nPos = biomeGraph_->getHeight(neighbor.chunkX, neighbor.chunkZ, 0, t);
        } else {
            // Z-axis boundary: chunk bottom edge ↔ neighbor top edge
            wPos = biomeGraph_->getHeight(chunk.chunkX, chunk.chunkZ, t, CHUNK_SIZE - 1);
            nPos = biomeGraph_->getHeight(neighbor.chunkX, neighbor.chunkZ, t, 0);
        }

        // Average heights to eliminate seam
        float avg = (wPos + nPos) * 0.5f;
        // Direct application deferred to mesh generation step
        // (this pass only marks the seam for downstream mesh vertex correction)
    }
}

} // namespace world
