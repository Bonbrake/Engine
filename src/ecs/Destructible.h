#pragma once
// [M2] Destructible component — composable with Health per M2 spec.
// "Health and Destructible act as separate, composable components."
// When health reaches 0 the DestructibleSystem swaps meshes and removes colliders.
#include <entt/entt.hpp>
#include <string>
#include <cstdint>
#include "Components.h"

namespace ecs {

// Mesh handle index into AssetManager's mesh table (uint32 slot)
using MeshHandle = uint32_t;
static constexpr MeshHandle INVALID_MESH_HANDLE = UINT32_MAX;

struct DestructibleComponent {
    MeshHandle intactMeshHandle    = INVALID_MESH_HANDLE;
    MeshHandle destroyedMeshHandle = INVALID_MESH_HANDLE;

    // True once the entity has been destroyed (mesh swapped, collider removed)
    bool isDestroyed = false;

    // Optional: entity to spawn as debris on destruction (entt::null = none)
    entt::entity debrisSpawnEntity = entt::null;
};

} // namespace ecs

// Forward declare to avoid circular include
namespace events { class EventBus; }

namespace ecs {

class DamageSystem {
public:
    static void init(entt::registry* registry, events::EventBus* bus);
    static void receive(const DamageEvent& ev);
    
    // Kept for manual tick if needed, though EventBus::flush() will trigger receive()
    static void tick(entt::registry& registry, events::EventBus& bus);

private:
    static entt::registry* s_registry;
    static events::EventBus* s_bus;
};

} // namespace ecs
