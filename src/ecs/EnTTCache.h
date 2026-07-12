#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <algorithm>
#include <entt/entt.hpp>

namespace ecs {

// [M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache
// LIFETIME CONTRACT: The raw pointers in this cache are only valid until the next call to
// SnapshotComponentPool<T> on the same worker slot. Do not yield a fiber while holding this cache.
struct LockedComponentPoolCache {
    uint8_t* rawDataBufferMemoryHead = nullptr;
    const uint32_t* sparseSetDenseIndicesPtr = nullptr;
    size_t componentTypeAllocationStride = 0;
    size_t sparseSetSize = 0;

    template<typename ComponentType>
    [[nodiscard]] inline ComponentType* ResolveComponentPointerDirect(uint32_t rawSparseEntityId) const noexcept {
        if (rawSparseEntityId >= sparseSetSize || !sparseSetDenseIndicesPtr) {
            return nullptr;
        }
        const uint32_t denseTargetIndex = sparseSetDenseIndicesPtr[rawSparseEntityId];
        if (denseTargetIndex == static_cast<uint32_t>(entt::null)) {
            return nullptr;
        }
        ComponentType* const* ptrArray = reinterpret_cast<ComponentType* const*>(rawDataBufferMemoryHead);
        return ptrArray[denseTargetIndex];
    }
};


// Note: This must be called AFTER DrainMutations, as mutation drains can reallocate the backing pools.
template<typename ComponentType>
LockedComponentPoolCache SnapshotComponentPool(entt::registry& registry, std::vector<uint8_t*>& outRawData, std::vector<uint32_t>& outSparseSet) {
    LockedComponentPoolCache cache;
    auto* storage = &registry.storage<ComponentType>();
    if (storage && !storage->empty()) {
        const size_t entity_count = storage->size();
        const auto* entities = storage->data();
        
        outRawData.resize(entity_count);
        for (size_t i = 0; i < entity_count; ++i) {
            outRawData[i] = reinterpret_cast<uint8_t*>(&storage->get(entities[i]));
        }
        
        cache.rawDataBufferMemoryHead = reinterpret_cast<uint8_t*>(outRawData.data());
        cache.componentTypeAllocationStride = sizeof(ComponentType*);
        
        uint32_t max_ent = 0;
        for (size_t i = 0; i < entity_count; ++i) {
            uint32_t ent = static_cast<uint32_t>(entities[i]);
            if (ent > max_ent) max_ent = ent;
        }
        
        outSparseSet.assign(max_ent + 1, static_cast<uint32_t>(entt::null));
        for (size_t i = 0; i < entity_count; ++i) {
            outSparseSet[static_cast<uint32_t>(entities[i])] = static_cast<uint32_t>(i);
        }
        
        cache.sparseSetDenseIndicesPtr = outSparseSet.data();
        cache.sparseSetSize = outSparseSet.size();
    }
    return cache;
}

} // namespace ecs
