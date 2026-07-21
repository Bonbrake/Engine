#pragma once
// [M2] Destructible component — composable with Health per M2 spec.
// "Health and Destructible act as separate, composable components."
// When health reaches 0 the DestructibleSystem swaps meshes and removes colliders.
#include <entt/entt.hpp>
#include <string>
#include <cstdint>
#include "ze/ecs/GenerationalTable.h"   // ecs::Handle (index + generation)

namespace ecs {

// Forward declarations (full defs live in Components.h / EventBus.h)
struct DamageEvent;

// Mesh handle into AssetManager's mesh table. Carries the full generation so the
// handle stays valid across table remove/reinsert (a bare uint32_t index would
// silently point at a stale slot after a generation bump).
using MeshHandle = ecs::Handle;
static constexpr MeshHandle INVALID_MESH_HANDLE = ecs::Handle{0xFFFFFFFF, 0xFFFFFFFF};

struct DestructibleComponent {
    // Intact + destroyed meshes, both generation-safe handles. A future destructible
    // (barrel/wall/zombie) sets these from real loaded assets; the type is correct once.
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