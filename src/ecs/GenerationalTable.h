#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

namespace ecs {

// [M1-EXT-02] Generational Resource Table Pointer Validator
struct Handle {
    uint32_t index;
    uint32_t generation;

    bool operator==(const Handle& o) const { return index == o.index && generation == o.generation; }
    bool operator!=(const Handle& o) const { return !(*this == o); }
};

inline bool IsHandleValid(const Handle& h, const std::vector<uint32_t>& generations) {
    return h.index < generations.size() && h.generation == generations[h.index];
}

template<typename T>
class GenerationalTable {
public:
    std::vector<uint32_t> generations;
    std::vector<T> data;
    std::vector<uint32_t> freeIndices;

    Handle Insert(T value) {
        if (!freeIndices.empty()) {
            uint32_t idx = freeIndices.back();
            freeIndices.pop_back();
            data[idx] = std::move(value);
            return Handle{idx, generations[idx]};
        }
        
        uint32_t idx = static_cast<uint32_t>(data.size());
        data.push_back(std::move(value));
        generations.push_back(1);
        return Handle{idx, 1};
    }

    void Remove(Handle h) {
        if (IsHandleValid(h, generations)) {
            generations[h.index]++;
            data[h.index] = T{}; // Release active asset resources
            freeIndices.push_back(h.index);
        }
    }

    bool isSlotCleared(uint32_t index) const {
        if (index >= data.size()) return true;
        T empty{};
        return std::memcmp(&data[index], &empty, sizeof(T)) == 0;
    }

    T* Get(Handle h) {
        if (IsHandleValid(h, generations)) {
            return &data[h.index];
        }
        return nullptr;
    }
};

} // namespace ecs
