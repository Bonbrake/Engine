#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <array>

namespace audio {

// Voice priority matrix + cadence stagger (M6-EXT-05, M6-EXT-06)
// Sits under Miniaudio backend decision, gating channel budget at horde scale.

// Per-voice allocation record
struct VoiceAllocation {
    uint32_t sourceId;
    float priority;           // computed per M6-EXT-05
    uint32_t cadenceInterval; // computed per M6-EXT-06
    uint64_t nextTick;        // next frame this voice must update
    bool active = false;
};

// Voice culler: evaluates cheap distance/orientation score per candidate each tick
// and drops anything below structural cutoff before full mixing.
class VoiceCuller {
public:
    VoiceCuller(uint32_t maxVoices = 256);

    // Evaluate priority for a candidate source
    // Priority = Volume_base / (d² + ε) · cos(θ_viewAngle)
    float evaluatePriority(float volumeBase, float distance, float viewAngleCosine) const;

    // Submit candidate for this tick; returns allocation if voice passed cutoff
    bool submitCandidate(uint32_t sourceId, float priority, uint64_t frameCounter);

    // Get cadence interval for distance
    uint32_t getCadence(float distance) const;

    // Get current active voice count
    uint32_t activeCount() const;

    // Reset per-tick state
    void resetFrame();

private:
    uint32_t maxVoices_;
    std::vector<VoiceAllocation> allocations_;
};

} // namespace audio
