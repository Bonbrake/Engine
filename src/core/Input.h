#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <string>

namespace core {

struct InputState {
    // Flat input state, easily serializable for replay
    bool quit = false;
    std::vector<SDL_Event> events;

    // [M2.6 Phase 2] Fly-camera state, derived from events in poll()
    uint8_t keyboardState[SDL_SCANCODE_COUNT] = {0};
    int mouseDX = 0;
    int mouseDY = 0;
};

// [M2.6 Phase 2] Scripted-input: one entry per frame, emitted by poll().
struct ScriptFrame {
    std::vector<SDL_Scancode> keysDown;   // KEY_DOWN this frame
    std::vector<SDL_Scancode> keysUp;     // KEY_UP this frame
    int mouseDX = 0;
    int mouseDY = 0;
};

class Input {
public:
    static void init();
    static void poll();
    static void shutdown();
    static const InputState& getState() { return state_; }

    // Test-only hook: lets headless unit tests drive held-key state without an
    // SDL window. Not used by gameplay code (which calls poll()).
    static InputState& getMutableStateForTest() { return state_; }

    // [M2.6 Phase 2] Scripted-input mode (no display needed for self-verify).
    // Loads a tiny text script: "K <scancode|WASD> <f0> <f1>" = hold key frames
    // [f0,f1]; "M <dx> <dy> <f0> <f1>" = mouse delta over [f0,f1]. Returns false
    // on parse error. poll() then replays the per-frame events instead of SDL.
    static bool loadScript(const std::string& path);

private:
    static InputState state_;
    static std::vector<ScriptFrame> script_;
    static size_t scriptFrame_;
};

} // namespace core
