#include "ze/ecs/Destructible.h"
#include "ze/ecs/Components.h"
#include "ze/events/EventBus.h"
#include "ze/core/Logger.h"
#include "ze/physics/PhysicsSystem.h"
#include <entt/entt.hpp>

namespace ecs {

entt::registry* DamageSystem::s_registry = nullptr;
events::EventBus* DamageSystem::s_bus = nullptr;

void DamageSystem::init(entt::registry* registry, events::EventBus* bus) {
    s_registry = registry;
    s_bus = bus;
    if (s_bus) {
        // Connect the receive function to the EventBus sink
        s_bus->raw().sink<DamageEvent>().connect<&DamageSystem::receive>();
    }
}

void DamageSystem::receive(const DamageEvent& ev) {
    if (!s_registry || !s_bus) return;
    auto& registry = *s_registry;

    entt::entity target = ev.target;

    ResolvedDamageEvent resolved;
    resolved.source_event = ev;
    resolved.final_amount  = ev.amount;
    resolved.was_parried   = false;
    resolved.was_mitigated = false;

    if (target != entt::null && registry.valid(target)) {
        if (auto* hp = registry.try_get<Health>(target)) {
            hp->current -= resolved.final_amount;
            if (hp->current < 0.0f) hp->current = 0.0f;

            LOG_INFO("DamageSystem: entity {} took {:.1f} damage (tag {}). HP: {:.1f}/{:.1f}",
                static_cast<uint32_t>(target),
                resolved.final_amount,
                static_cast<int>(ev.tag),
                hp->current, hp->max);

            if (hp->current <= 0.0f) {
                if (auto* destr = registry.try_get<DestructibleComponent>(target)) {
                    if (!destr->isDestroyed) {
                        destr->isDestroyed = true;
                        if (destr->destroyedMeshHandle != INVALID_MESH_HANDLE) {
                            // [M2:EXIT-meshswap] Actually apply the swap to the render
                            // component. MeshComponent.meshHandle is a generation-safe
                            // ecs::Handle, so this is valid by construction (no stale-slot
                            // risk). TriangleRenderer's view<Transform,MeshComponent>
                            // traversal then draws the destroyed mesh next frame.
                            if (auto* mc = registry.try_get<MeshComponent>(target)) {
                                mc->meshHandle = destr->destroyedMeshHandle;
                                LOG_INFO("DamageSystem: entity {} DESTROYED — mesh swapped to handle {}/{}",
                                    static_cast<uint32_t>(target),
                                    destr->destroyedMeshHandle.index,
                                    destr->destroyedMeshHandle.generation);
                            } else {
                                LOG_WARN("DamageSystem: entity {} DESTROYED but has no MeshComponent; swap skipped",
                                    static_cast<uint32_t>(target));
                            }
                        }
                        if (registry.all_of<physics::PhysicsBodyComponent>(target)) {
                            auto& phys = registry.get<physics::PhysicsBodyComponent>(target);
                            LOG_INFO("DamageSystem: removing collider body 0x{:X} for destroyed entity {}",
                                phys.bodyId.GetIndexAndSequenceNumber(),
                                static_cast<uint32_t>(target));
                            registry.remove<physics::PhysicsBodyComponent>(target);
                        }
                    }
                }
            }

            // [M2] Emit the ResolvedDamageEvent for downstream consumers (UI, Hit-Markers, Blood VFX)
            s_bus->enqueueResolved(resolved);
        }
    }
}

void DamageSystem::tick(entt::registry& registry, events::EventBus& bus) {
    // Flush the queue, which routes events to receive()
    bus.raw().update<DamageEvent>();
}

} // namespace ecs