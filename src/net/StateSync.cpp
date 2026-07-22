#include "ze/net/StateSync.h"
#include <cstring>

namespace net {

static uint32_t fnv1a(const uint8_t* data, size_t size) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

void StateSync::recordFrame(uint32_t frameNumber, const uint8_t* inputData, size_t inputSize,
                             const uint8_t* entityData, size_t entitySize) {
    SyncFrame frame;
    frame.frameNumber = frameNumber;
    frame.inputHash = fnv1a(inputData, inputSize);
    frame.entityHash = fnv1a(entityData, entitySize);
    frame.checksum = frame.inputHash ^ frame.entityHash;
    frames_.push_back(frame);
}

bool StateSync::validateReplay(uint32_t fromFrame, uint32_t toFrame,
                                const std::vector<SyncFrame>& expectedFrames) {
    if (expectedFrames.size() < (toFrame - fromFrame)) return false;

    for (uint32_t i = fromFrame; i < toFrame && i < frames_.size(); ++i) {
        uint32_t idx = i - fromFrame;
        if (idx >= expectedFrames.size()) return false;
        if (frames_[i].checksum != expectedFrames[idx].checksum) return false;
    }
    return true;
}

void StateSync::reset() {
    frames_.clear();
}

uint32_t StateSync::computeHash(const uint8_t* data, size_t size) {
    return fnv1a(data, size);
}

} // namespace net
