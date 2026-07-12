#pragma once
// [M2] EventBus — thin wrapper around entt::dispatcher
// Dispatch order for damage: raw DamageEvent -> parry (M2.7-EXT-01) -> armor mitigation
// -> perk hooks (M8.7) -> ResolvedDamageEvent -> Health application.
// Systems needing pre-mitigation: subscribe to DamageEvent.
// Systems that only care about final HP loss: subscribe to ResolvedDamageEvent.
#include <entt/entt.hpp>
#include "../ecs/Components.h"

namespace events {

// [M2-EXT-02] Sleep bridge event — emitted when Jolt island sleeps or wakes.
// Downstream: M7-EXT-01 (corpse flattening), LOD system, AI perception exclusion.
struct HibernationEvent {
    uint64_t targetStableId;
    bool isEnteringSleep;
};

// Re-export canonical damage types declared in Components.h so event subscribers
// only need to include EventBus.h
using ecs::DamageEvent;
using ecs::ResolvedDamageEvent;
using ecs::LimbId;

// The single EventBus instance — one dispatcher per game world.
class EventBus {
public:
    // Enqueue a raw damage event for processing this tick.
    void enqueue(const DamageEvent& ev) { dispatcher_.enqueue<DamageEvent>(ev); }

    // Enqueue a resolved damage event (post-mitigation) for UI/VFX consumers.
    void enqueueResolved(const ResolvedDamageEvent& ev) {
        dispatcher_.enqueue<ResolvedDamageEvent>(ev);
    }

    // Enqueue sleep state change from Jolt bridge [M2-EXT-02]
    void enqueueSleep(uint64_t stableId, bool isEnteringSleep) {
        dispatcher_.enqueue<HibernationEvent>({stableId, isEnteringSleep});
    }

    // Flush and dispatch all enqueued events to subscribers. Call once per tick.
    void flush() { dispatcher_.update(); }

    // Direct access for subscribers registering via connect<Event>()
    entt::dispatcher& raw() { return dispatcher_; }

private:
    entt::dispatcher dispatcher_;
};

} // namespace events
