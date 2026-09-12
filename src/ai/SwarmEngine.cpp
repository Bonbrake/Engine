#include "ze/ai/SwarmEngine.h"
#include <random>

namespace ze::ai {

SwarmEngine::SwarmEngine() {
    initialize(1024);
}

void SwarmEngine::initialize(size_t maxAgents) {
    agents_.clear();
    agents_.resize(maxAgents);
    for (size_t i = 0; i < agents_.size(); ++i) {
        agents_[i].active = false;
        agents_[i].tier = SwarmAgentTier::MacroFlocker;
        agents_[i].pyramidState = PyramidState::GroundFlocking;
        agents_[i].entityId = nextEntityId_++;
    }
    activeCount_ = 0;
    promotedCombatCount_ = 0;
}

void SwarmEngine::reset() {
    for (auto& a : agents_) {
        a.active = false;
        a.health = 100.0f;
        a.pyramidState = PyramidState::GroundFlocking;
        a.tier = SwarmAgentTier::MacroFlocker;
        a.climbProgress = 0.0f;
    }
    activeCount_ = 0;
    promotedCombatCount_ = 0;
    anchors_.clear();
}

uint16_t SwarmEngine::registerPyramidAnchor(const glm::vec3& basePos, const glm::vec3& ledgePos) {
    PyramidClimbingAnchor anchor;
    anchor.anchorId = static_cast<uint16_t>(anchors_.size());
    anchor.basePosition = basePos;
    anchor.ledgePosition = ledgePos;
    anchor.ledgeHeight = std::abs(ledgePos.y - basePos.y);
    if (anchor.ledgeHeight < 1.0f) anchor.ledgeHeight = 3.5f;
    anchor.baseRadius = 3.5f;
    anchor.baseStructuralHealth = 500.0f;
    anchor.maxStructuralHealth = 500.0f;
    anchor.activeClimberCount = 0;
    anchor.isCollapsed = false;
    anchor.collapseRecoveryTimer = 0.0f;

    anchors_.push_back(anchor);
    return anchor.anchorId;
}

size_t SwarmEngine::spawnSwarmWave(size_t count, const glm::vec3& spawnOrigin, float initialDispersionRadius) {
    size_t spawned = 0;
    for (size_t i = 0; i < agents_.size() && spawned < count; ++i) {
        if (!agents_[i].active) {
            SwarmAgent& a = agents_[i];
            a.active = true;
            a.health = 100.0f;
            a.tier = SwarmAgentTier::MacroFlocker;
            a.pyramidState = PyramidState::GroundFlocking;
            a.climbProgress = 0.0f;
            a.speed = config.flockerSpeed + (static_cast<float>(spawned % 7) - 3.0f) * 0.2f;

            // Deterministic circular dispersion
            float angle = static_cast<float>(spawned) * 0.6283f; // Golden ratio spiral
            float radius = (static_cast<float>((spawned % 10) + 1) / 10.0f) * initialDispersionRadius;

            a.position.x = spawnOrigin.x + radius * std::cos(angle);
            a.position.y = spawnOrigin.y;
            a.position.z = spawnOrigin.z + radius * std::sin(angle);
            a.velocity = glm::vec3(0.0f);
            a.animPhase = static_cast<uint8_t>((spawned * 17) % 256);

            spawned++;
        }
    }
    activeCount_ += spawned;
    return spawned;
}

void SwarmEngine::update(float dt, const glm::vec3& survivorCenter, float /*survivorFlowDist*/) {
    if (activeCount_ == 0) return;

    updateFlowField(survivorCenter);
    updateFlockingMovement(dt, survivorCenter);
    updatePyramidClimbing(dt);
    updateTwoTierPromotion(survivorCenter);
    updateAudioCentroid();
}

void SwarmEngine::updateFlowField(const glm::vec3& survivorCenter) {
    flowField_.generate(survivorCenter);
}

void SwarmEngine::updateFlockingMovement(float dt, const glm::vec3& /*survivorCenter*/) {
    for (auto& a : agents_) {
        if (!a.active) continue;

        // Advance animation phase for Vertex Animation Texture (VAT)
        a.animPhase = static_cast<uint8_t>((a.animPhase + static_cast<int>(dt * 60.0f * 4.0f)) % 256);

        if (a.pyramidState == PyramidState::AscendingPyramid) {
            // Managed by updatePyramidClimbing
            continue;
        }

        if (a.pyramidState == PyramidState::FallingRagdoll) {
            // Gravity physics for collapsing zombies
            a.velocity.y -= 19.6f * dt;
            a.position.x += a.velocity.x * dt;
            a.position.y += a.velocity.y * dt;
            a.position.z += a.velocity.z * dt;

            // Floor contact
            if (a.position.y <= 0.0f) {
                a.position.y = 0.0f;
                a.velocity = glm::vec3(0.0f);
                a.pyramidState = PyramidState::GroundFlocking; // Recover to ground
            }
            continue;
        }

        // Sample O(1) Flow Field Direction
        float dirX = 0.0f, dirY = 0.0f, dirZ = 0.0f;
        flowField_.sampleDirection(a.position, dirX, dirY, dirZ);

        glm::vec3 desiredVel(dirX * a.speed, dirY * a.speed, dirZ * a.speed);

        // Smooth acceleration toward desired flow direction
        a.velocity.x += (desiredVel.x - a.velocity.x) * (dt * 8.0f);
        a.velocity.z += (desiredVel.z - a.velocity.z) * (dt * 8.0f);

        // Integrate position
        a.position.x += a.velocity.x * dt;
        a.position.z += a.velocity.z * dt;
    }
}

void SwarmEngine::updatePyramidClimbing(float dt) {
    for (auto& anchor : anchors_) {
        if (anchor.isCollapsed) {
            anchor.collapseRecoveryTimer += dt;
            if (anchor.collapseRecoveryTimer >= 6.0f) {
                anchor.isCollapsed = false;
                anchor.baseStructuralHealth = anchor.maxStructuralHealth;
                anchor.collapseRecoveryTimer = 0.0f;
            }
            continue;
        }

        // Count zombies pooling at the base
        uint32_t poolCount = 0;
        for (const auto& a : agents_) {
            if (!a.active) continue;
            float dx = a.position.x - anchor.basePosition.x;
            float dz = a.position.z - anchor.basePosition.z;
            float distSq = dx * dx + dz * dz;
            if (distSq <= anchor.baseRadius * anchor.baseRadius) {
                poolCount++;
            }
        }

        anchor.activeClimberCount = poolCount;

        // If sufficient density at base, pool transitions to climbing
        if (poolCount >= static_cast<uint32_t>(config.pyramidTriggerDensity)) {
            for (auto& a : agents_) {
                if (!a.active) continue;
                if (a.pyramidState == PyramidState::GroundFlocking) {
                    float dx = a.position.x - anchor.basePosition.x;
                    float dz = a.position.z - anchor.basePosition.z;
                    if (dx * dx + dz * dz <= (anchor.baseRadius * 0.75f) * (anchor.baseRadius * 0.75f)) {
                        a.pyramidState = PyramidState::AscendingPyramid;
                        a.pyramidAnchorId = anchor.anchorId;
                        a.climbProgress = 0.05f;
                    }
                } else if (a.pyramidState == PyramidState::AscendingPyramid && a.pyramidAnchorId == anchor.anchorId) {
                    // Advance climbing along pyramid ramp
                    a.climbProgress += (config.pyramidClimbSpeed / anchor.ledgeHeight) * dt;

                    // Interpolate position from base to ledge
                    float t = std::clamp(a.climbProgress, 0.0f, 1.0f);
                    a.position.x = anchor.basePosition.x + (anchor.ledgePosition.x - anchor.basePosition.x) * t;
                    a.position.y = anchor.basePosition.y + (anchor.ledgePosition.y - anchor.basePosition.y) * t;
                    a.position.z = anchor.basePosition.z + (anchor.ledgePosition.z - anchor.basePosition.z) * t;

                    if (a.climbProgress >= 1.0f) {
                        a.pyramidState = PyramidState::CrestedLedge;
                        a.position = anchor.ledgePosition;
                        // Return to normal running from the elevated ledge
                        a.pyramidState = PyramidState::GroundFlocking;
                    }
                }
            }
        }
    }
}

void SwarmEngine::updateTwoTierPromotion(const glm::vec3& survivorCenter) {
    promotedCombatCount_ = 0;
    float promoDistSq = config.promotionDistance * config.promotionDistance;
    float demoDistSq = config.demotionDistance * config.demotionDistance;

    for (auto& a : agents_) {
        if (!a.active) continue;

        float dx = a.position.x - survivorCenter.x;
        float dy = a.position.y - survivorCenter.y;
        float dz = a.position.z - survivorCenter.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (a.tier == SwarmAgentTier::MacroFlocker) {
            if (distSq <= promoDistSq) {
                a.tier = SwarmAgentTier::MicroCombatActor;
            }
        } else if (a.tier == SwarmAgentTier::MicroCombatActor) {
            if (distSq > demoDistSq) {
                a.tier = SwarmAgentTier::MacroFlocker;
            }
        }

        if (a.tier == SwarmAgentTier::MicroCombatActor) {
            promotedCombatCount_++;
        }
    }
}

bool SwarmEngine::damagePyramidBase(uint16_t anchorId, float damage) {
    if (anchorId >= anchors_.size()) return false;
    PyramidClimbingAnchor& anchor = anchors_[anchorId];
    if (anchor.isCollapsed) return false;

    anchor.baseStructuralHealth -= damage;
    if (anchor.baseStructuralHealth <= 0.0f) {
        anchor.isCollapsed = true;
        anchor.baseStructuralHealth = 0.0f;
        anchor.collapseRecoveryTimer = 0.0f;

        // Structural collapse: all climbing zombies convert to falling ragdolls with outward explosion impulse
        for (auto& a : agents_) {
            if (!a.active) continue;
            if (a.pyramidState == PyramidState::AscendingPyramid && a.pyramidAnchorId == anchorId) {
                a.pyramidState = PyramidState::FallingRagdoll;
                // Outward ejection velocity
                float dx = a.position.x - anchor.basePosition.x;
                float dz = a.position.z - anchor.basePosition.z;
                float dist = std::sqrt(dx * dx + dz * dz);
                if (dist > 0.01f) {
                    a.velocity.x = (dx / dist) * 6.0f;
                    a.velocity.z = (dz / dist) * 6.0f;
                } else {
                    a.velocity.x = 4.0f;
                    a.velocity.z = 0.0f;
                }
                a.velocity.y = 3.5f; // Upward-and-outward fling
            }
        }
        return true;
    }
    return false;
}

bool SwarmEngine::applyDamage(size_t agentIndex, float damage, const glm::vec3& impactDir) {
    if (agentIndex >= agents_.size() || !agents_[agentIndex].active) return false;

    SwarmAgent& a = agents_[agentIndex];
    a.health -= damage;
    if (a.health <= 0.0f) {
        a.active = false;
        a.health = 0.0f;
        if (activeCount_ > 0) activeCount_--;
        if (a.tier == SwarmAgentTier::MicroCombatActor && promotedCombatCount_ > 0) {
            promotedCombatCount_--;
        }
        return true; // Agent died
    }

    // Stumble recoil
    a.velocity.x += impactDir.x * 2.0f;
    a.velocity.z += impactDir.z * 2.0f;
    return false;
}

void SwarmEngine::updateAudioCentroid() {
    if (activeCount_ == 0) {
        clusterAudio_.massRoarVolume = 0.0f;
        clusterAudio_.totalActiveZombies = 0;
        return;
    }

    glm::vec3 sumPos(0.0f);
    glm::vec3 sumVel(0.0f);
    uint32_t count = 0;

    for (const auto& a : agents_) {
        if (!a.active) continue;
        sumPos.x += a.position.x;
        sumPos.y += a.position.y;
        sumPos.z += a.position.z;
        sumVel.x += a.velocity.x;
        sumVel.y += a.velocity.y;
        sumVel.z += a.velocity.z;
        count++;
    }

    if (count > 0) {
        clusterAudio_.centroid.x = sumPos.x / static_cast<float>(count);
        clusterAudio_.centroid.y = sumPos.y / static_cast<float>(count);
        clusterAudio_.centroid.z = sumPos.z / static_cast<float>(count);

        clusterAudio_.averageVelocity.x = sumVel.x / static_cast<float>(count);
        clusterAudio_.averageVelocity.y = sumVel.y / static_cast<float>(count);
        clusterAudio_.averageVelocity.z = sumVel.z / static_cast<float>(count);

        clusterAudio_.totalActiveZombies = count;

        // Logarithmic volume scaling: Vol = log10(1 + N) / log10(1 + 1000)
        float logN = std::log10(1.0f + static_cast<float>(count));
        float maxLog = std::log10(1001.0f); // ~3.0
        clusterAudio_.massRoarVolume = std::clamp(logN / maxLog, 0.0f, 1.0f);

        // Pitch Doppler shift based on velocity toward camera
        float speed = std::sqrt(clusterAudio_.averageVelocity.x * clusterAudio_.averageVelocity.x +
                                clusterAudio_.averageVelocity.z * clusterAudio_.averageVelocity.z);
        clusterAudio_.pitchDoppler = 1.0f + std::clamp(speed / 343.0f, 0.0f, 0.15f);
    }
}

} // namespace ze::ai
