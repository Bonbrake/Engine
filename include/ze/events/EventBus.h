#pragma once
#include <entt/entt.hpp>
#include "ze/ecs/Components.h"

namespace events {

// [M2-EXT-02] Sleep/bridge event emitted when a Jolt body activates/deactivates.
struct HibernationEvent {
    uint64_t uuid = 0;
    bool isActivated = false;
};

class EventBus {
public:
    EventBus() = default;

    // Forward event to internal dispatcher queue
    template<typename Event>
    void enqueue(const Event& ev) {
        dispatcher_.enqueue(ev);
    }

    // Enqueue a resolved-damage event for downstream consumers (UI, hit-markers, VFX)
    void enqueueResolved(const ecs::ResolvedDamageEvent& ev) {
        dispatcher_.enqueue(ev);
    }

    // Access raw dispatcher for sink/connect patterns
    entt::dispatcher& raw() { return dispatcher_; }
    const entt::dispatcher& raw() const { return dispatcher_; }

    // Flush queued events to connected listeners
    void flush() { dispatcher_.update(); }

private:
    entt::dispatcher dispatcher_;
};

} // namespace events
