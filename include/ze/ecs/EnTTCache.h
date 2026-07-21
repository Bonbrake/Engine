#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <algorithm>
#include <entt/entt.hpp>

namespace ecs {

// [M1-EXT-09] EnTT Concurrent Component Archetype View Iteration Cache
struct LockedComponentPoolCache {
    std::shared_ptr<std::vector<uint8_t*>> rawDataBuffer;
    std::shared_ptr<std::vector<uint32_t>> sparseSet;
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

template<typename ComponentType>
LockedComponentPoolCache SnapshotComponentPool(entt::registry& registry) {
    LockedComponentPoolCache cache;
    auto* storage = &registry.storage<ComponentType>();
    if (storage && !storage->empty()) {
        const size_t entity_count = storage->size();
        const auto* entities = storage->data();
        
        auto rawData = std::make_shared<std::vector<uint8_t*>>(entity_count);
        for (size_t i = 0; i < entity_count; ++i) {
            (*rawData)[i] = reinterpret_cast<uint8_t*>(&storage->get(entities[i]));
        }
        
        cache.rawDataBuffer = rawData;
        cache.rawDataBufferMemoryHead = reinterpret_cast<uint8_t*>(rawData->data());
        cache.componentTypeAllocationStride = sizeof(ComponentType*);
        
        uint32_t max_ent = 0;
        for (size_t i = 0; i < entity_count; ++i) {
            uint32_t ent = static_cast<uint32_t>(entities[i]);
            if (ent > max_ent) max_ent = ent;
        }
        
        auto sparseSet = std::make_shared<std::vector<uint32_t>>(max_ent + 1, static_cast<uint32_t>(entt::null));
        for (size_t i = 0; i < entity_count; ++i) {
            (*sparseSet)[static_cast<uint32_t>(entities[i])] = static_cast<uint32_t>(i);
        }
        
        cache.sparseSet = sparseSet;
        cache.sparseSetDenseIndicesPtr = sparseSet->data();
        cache.sparseSetSize = sparseSet->size();
    }
    return cache;
}

} // namespace ecs
