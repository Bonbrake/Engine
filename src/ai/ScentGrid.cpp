#include "ze/ai/ScentGrid.h"
#include <cmath>
#include <algorithm>

namespace ai {

ScentGrid::ScentGrid(float worldSize, float cellSize)
    : worldSize_(worldSize), cellSize_(cellSize) {
    // cell count for half-open interval [-worldSize/2, worldSize/2)
    gridDim_ = static_cast<int>(std::ceil(worldSize_ / cellSize_));
    cells_.resize(gridDim_ * gridDim_);
}

int ScentGrid::cellIndex(glm::vec3 pos) const {
    // Map world position to cell index, assuming pos is in [-worldSize/2, worldSize/2)
    int x = static_cast<int>(std::floor((pos.x + worldSize_ * 0.5f) / cellSize_));
    int z = static_cast<int>(std::floor((pos.z + worldSize_ * 0.5f) / cellSize_));
    x = std::max(0, std::min(x, gridDim_ - 1));
    z = std::max(0, std::min(z, gridDim_ - 1));
    return z * gridDim_ + x;
}

void ScentGrid::addScent(const glm::vec3& position, float strength, uint32_t sourceId, uint8_t scentType) {
    int idx = cellIndex(position);
    if (idx >= 0 && idx < static_cast<int>(cells_.size())) {
        ScentParticle p;
        p.position = position;
        p.strength = std::min(strength, 1.0f);
        p.lifetime = 10.0f + strength * 20.0f; // 10-30 seconds
        p.sourceId = sourceId;
        p.scentType = scentType;
        cells_[idx].push_back(p);
    }
}

float ScentGrid::queryScent(const glm::vec3& position, uint32_t& nearestSourceId) const {
    int idx = cellIndex(position);
    float totalStrength = 0.0f;
    float bestDist = std::numeric_limits<float>::max();
    nearestSourceId = 0;

    if (idx < 0 || idx >= static_cast<int>(cells_.size())) return 0.0f;

    for (const auto& p : cells_[idx]) {
        if (p.lifetime <= 0.0f) continue;
        float dist = glm::distance(position, p.position);
        float strengthAtPoint = p.strength / (dist * dist + 0.01f);
        totalStrength += strengthAtPoint;
        if (dist < bestDist) {
            bestDist = dist;
            nearestSourceId = p.sourceId;
        }
    }
    return totalStrength;
}

void ScentGrid::applyWind(const glm::vec3& windVector, float dt) {
    // Move particles by wind * dt and decay
    for (auto& cell : cells_) {
        for (auto& p : cell) {
            if (p.lifetime <= 0.0f) continue;
            p.position += windVector * dt * 2.0f;
            p.strength *= (1.0f - dt * 0.1f); // wind scatter reduces intensity
        }
    }
}

void ScentGrid::tick(float dt) {
    for (auto& cell : cells_) {
        cell.erase(
            std::remove_if(cell.begin(), cell.end(),
                [dt](ScentParticle& p) {
                    p.lifetime -= dt;
                    p.strength *= (1.0f - dt * 0.05f); // natural decay
                    return p.lifetime <= 0.0f || p.strength < 0.01f;
                }),
            cell.end());
    }
}

void ScentGrid::clear() {
    for (auto& cell : cells_) {
        cell.clear();
    }
}

} // namespace ai
