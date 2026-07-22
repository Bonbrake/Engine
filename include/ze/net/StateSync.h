#pragma once
#include <cstdint>
#include <vector>
#include <functional>

namespace net {

// Deterministic state sync (M2.8-EXT-09, M10)
struct SyncFrame {
    uint32_t frameNumber;
    uint32_t inputHash;      // hash of player inputs this frame
    uint32_t entityHash;     // hash of entity snapshot this frame
    uint32_t checksum;       // combined checksum for replay validation
};

class StateSync {
public:
    StateSync() = default;

    // Record current frame state
    void recordFrame(uint32_t frameNumber, const uint8_t* inputData, size_t inputSize,
                     const uint8_t* entityData, size_t entitySize);

    // Replay validation: returns true if recorded stream matches expected
    bool validateReplay(uint32_t fromFrame, uint32_t toFrame,
                        const std::vector<SyncFrame>& expectedFrames);

    // Get recorded frames for this session
    const std::vector<SyncFrame>& recordedFrames() const { return frames_; }

    // Clear
    void reset();

    // Checksum helpers
    static uint32_t computeHash(const uint8_t* data, size_t size);

private:
    std::vector<SyncFrame> frames_;
};

} // namespace net
