#pragma once

#include <SDL3/SDL.h>
#include <vector>

namespace core {

struct InputState {
    // Flat input state, easily serializable for replay
    bool quit = false;
    std::vector<SDL_Event> events;
};

class Input {
public:
    static void init();
    static void poll();
    static void shutdown();
    static const InputState& getState() { return state_; }

private:
    static InputState state_;
};

} // namespace core
