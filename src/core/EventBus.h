#pragma once

#include <entt/entt.hpp>

namespace core {

class EventBus {
public:
    static EventBus& get() {
        static EventBus instance;
        return instance;
    }

    template<typename Event>
    void publish(const Event& event) {
        dispatcher_.trigger(event);
    }

    template<typename Event, auto Listener, typename Instance>
    void subscribe(Instance* instance) {
        dispatcher_.sink<Event>().template connect<Listener>(instance);
    }

    template<typename Event, auto Listener, typename Instance>
    void unsubscribe(Instance* instance) {
        dispatcher_.sink<Event>().template disconnect<Listener>(instance);
    }

private:
    EventBus() = default;
    entt::dispatcher dispatcher_;
};

// Common Events
struct WindowCloseEvent {};

} // namespace core