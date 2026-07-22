#include "ze/audio/AudioVoice.h"
#include <algorithm>
#include <cmath>

namespace audio {

VoiceCuller::VoiceCuller(uint32_t maxVoices)
    : maxVoices_(maxVoices) {
    allocations_.reserve(maxVoices);
}

float VoiceCuller::evaluatePriority(float volumeBase, float distance,
                                     float viewAngleCosine) const {
    constexpr float eps = 0.001f;
    return (volumeBase / ((distance * distance) + eps)) * viewAngleCosine;
}

bool VoiceCuller::submitCandidate(uint32_t sourceId, float priority,
                                   uint64_t frameCounter) {
    // Distance cadence check — if voice exists, skip if not on cadence
    for (auto& a : allocations_) {
        if (a.sourceId == sourceId) {
            if (frameCounter < a.nextTick) {
                return false; // voice not due yet
            }
            a.nextTick = frameCounter + a.cadenceInterval;
            a.priority = priority;
            return true;
        }
    }

    // New voice: check budget
    uint32_t active = 0;
    for (const auto& a : allocations_) {
        if (a.active) ++active;
    }

    // If at budget, check priority against lowest active
    if (active >= maxVoices_) {
        float lowestPriority = std::numeric_limits<float>::max();
        int lowestIdx = -1;
        for (size_t i = 0; i < allocations_.size(); ++i) {
            if (allocations_[i].active && allocations_[i].priority < lowestPriority) {
                lowestPriority = allocations_[i].priority;
                lowestIdx = static_cast<int>(i);
            }
        }
        if (priority <= lowestPriority) return false; // not better than lowest
        // Replacing lowest
        if (lowestIdx >= 0) {
            allocations_[lowestIdx] = VoiceAllocation{};
        }
    }

    VoiceAllocation va;
    va.sourceId = sourceId;
    va.priority = priority;
    va.cadenceInterval = 1; // default
    va.nextTick = frameCounter + va.cadenceInterval;
    va.active = true;
    allocations_.push_back(va);
    return true;
}

uint32_t VoiceCuller::getCadence(float distance) const {
    uint32_t cadence = static_cast<uint32_t>(distance / 10.0f);
    if (cadence < 1) cadence = 1;
    if (cadence > 8) cadence = 8;
    return cadence;
}

uint32_t VoiceCuller::activeCount() const {
    uint32_t count = 0;
    for (const auto& a : allocations_) {
        if (a.active) ++count;
    }
    return count;
}

void VoiceCuller::resetFrame() {
    // Keep allocations but reset per-tick state
    for (auto& a : allocations_) {
        // Keep active; it's trimmed by submitCandidate if needed
        a.priority = 0.0f;
    }
}

} // namespace audio
