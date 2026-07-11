#pragma once

#include <vector>
#include <functional>
#include <cstdint>

namespace render {

// [M1-EXT-04] Frame-Scoped Deletion Queue
struct DeletionRecord {
    std::function<void()> destructor;
    uint64_t frameFenceValue;
};

class FrameDeletionQueue {
public:
    void Push(std::function<void()>&& destructor, uint64_t currentFrameFence) {
        records.push_back({std::move(destructor), currentFrameFence});
    }

    void Flush(uint64_t completedFenceValue) {
        auto it = records.begin();
        while (it != records.end()) {
            if (completedFenceValue >= it->frameFenceValue) {
                it->destructor();
                it = records.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Force flush all remaining records (e.g., during teardown after device wait idle)
    void FlushAll() {
        for (auto& record : records) {
            record.destructor();
        }
        records.clear();
    }

private:
    std::vector<DeletionRecord> records;
};

} // namespace render
