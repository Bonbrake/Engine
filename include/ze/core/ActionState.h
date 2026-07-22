#ifndef ZE_CORE_ACTIONSTATE_H
#define ZE_CORE_ACTIONSTATE_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_gamepad.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace core {

enum class Action : uint8_t {
    None = 0,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Crouch,
    Sprint,
    Interact,
    Attack,
    Aim,
    Reload,
    Inventory,
    Map,
    Flashlight,
    COUNT
};

enum class ActionSource : uint8_t {
    None = 0,
    KeyboardMouse = 1,
    GamepadAxis = 2,
    GamepadButton = 3
};

struct Binding {
    Action action = Action::None;
    ActionSource source = ActionSource::None;
    SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
    uint32_t mouseButton = 0;
    SDL_GamepadAxis axis = SDL_GAMEPAD_AXIS_INVALID;
    int axisSign = 0;
    SDL_GamepadButton button = SDL_GAMEPAD_BUTTON_INVALID;
    float axisDeadzone = 0.15f;
};

struct ActionState {
    bool pressed = false;
    bool held = false;
    bool released = false;
    float analogValue = 0.0f;
};

class ActionMap {
public:
    static ActionMap& get();

    void setDefaultBindings();
    void load(const std::string& path);
    void save(const std::string& path) const;

    void update(const SDL_Event* events, size_t count);
    void setGamepadConnected(bool connected);
    bool isGamepadConnected() const;
    const ActionState& get(Action action) const;
    void resetFrameState();

private:
    void processEvent(const SDL_Event& event);
    void applyAxis(Action action, float value);
    void applyButton(Action action, bool down);
    void set(Action action, bool down, float analogValue = 0.0f);

    std::unordered_map<Action, std::vector<Binding>> bindings_;
    std::unordered_map<Action, ActionState> states_;
    std::unordered_map<SDL_Scancode, Action> keyMap_;
    std::unordered_map<uint32_t, Action> mouseMap_;
    std::unordered_map<SDL_GamepadAxis, std::vector<std::pair<Action, int>>> axisMap_;
    std::unordered_map<SDL_GamepadButton, Action> gamepadButtonMap_;
    bool gamepadConnected_ = false;
};

} // namespace core

#endif // ZE_CORE_ACTIONSTATE_H