#include "ze/core/ActionState.h"
#include "ze/core/Config.h"
#include "ze/core/Logger.h"
#include "ze/core/Input.h"
#include <SDL3/SDL_gamepad.h>
#include <cmath>
#include <algorithm>
#include <fstream>

namespace core {

ActionMap& ActionMap::get() {
    static ActionMap instance;
    return instance;
}

static float applyDeadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) {
        return 0.0f;
    }
    const float sign = value < 0.0f ? -1.0f : 1.0f;
    const float mag = (fabsf(value) - deadzone) / (1.0f - deadzone);
    return std::clamp(sign * mag, -1.0f, 1.0f);
}

void ActionMap::setDefaultBindings() {
    bindings_.clear();
    keyMap_.clear();
    mouseMap_.clear();
    axisMap_.clear();
    gamepadButtonMap_.clear();
    states_.clear();

    bindings_[Action::MoveForward].emplace_back(Action::MoveForward, ActionSource::KeyboardMouse, SDL_SCANCODE_W);
    bindings_[Action::MoveBackward].emplace_back(Action::MoveBackward, ActionSource::KeyboardMouse, SDL_SCANCODE_S);
    bindings_[Action::MoveLeft].emplace_back(Action::MoveLeft, ActionSource::KeyboardMouse, SDL_SCANCODE_A);
    bindings_[Action::MoveRight].emplace_back(Action::MoveRight, ActionSource::KeyboardMouse, SDL_SCANCODE_D);
    bindings_[Action::Jump].emplace_back(Action::Jump, ActionSource::KeyboardMouse, SDL_SCANCODE_SPACE);
    bindings_[Action::Crouch].emplace_back(Action::Crouch, ActionSource::KeyboardMouse, SDL_SCANCODE_LCTRL);
    bindings_[Action::Sprint].emplace_back(Action::Sprint, ActionSource::KeyboardMouse, SDL_SCANCODE_LSHIFT);
    bindings_[Action::Interact].emplace_back(Action::Interact, ActionSource::KeyboardMouse, SDL_SCANCODE_E);
    bindings_[Action::Reload].emplace_back(Action::Reload, ActionSource::KeyboardMouse, SDL_SCANCODE_R);
    bindings_[Action::Inventory].emplace_back(Action::Inventory, ActionSource::KeyboardMouse, SDL_SCANCODE_TAB);
    bindings_[Action::Map].emplace_back(Action::Map, ActionSource::KeyboardMouse, SDL_SCANCODE_M);
    bindings_[Action::Flashlight].emplace_back(Action::Flashlight, ActionSource::KeyboardMouse, SDL_SCANCODE_F);

    bindings_[Action::Jump].emplace_back(Action::Jump, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_SOUTH);
    bindings_[Action::Crouch].emplace_back(Action::Crouch, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_EAST);
    bindings_[Action::Attack].emplace_back(Action::Attack, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    bindings_[Action::Aim].emplace_back(Action::Aim, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    bindings_[Action::Reload].emplace_back(Action::Reload, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_WEST);
    bindings_[Action::Flashlight].emplace_back(Action::Flashlight, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    bindings_[Action::Interact].emplace_back(Action::Interact, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_NORTH);
    bindings_[Action::Sprint].emplace_back(Action::Sprint, ActionSource::GamepadButton, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_INVALID, 0, SDL_GAMEPAD_BUTTON_LEFT_STICK);

    bindings_[Action::MoveForward].emplace_back(Action::MoveForward, ActionSource::GamepadAxis, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_LEFTY, -1, SDL_GAMEPAD_BUTTON_INVALID, 0.15f);
    bindings_[Action::MoveBackward].emplace_back(Action::MoveBackward, ActionSource::GamepadAxis, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_LEFTY, 1, SDL_GAMEPAD_BUTTON_INVALID, 0.15f);
    bindings_[Action::MoveLeft].emplace_back(Action::MoveLeft, ActionSource::GamepadAxis, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_LEFTX, -1, SDL_GAMEPAD_BUTTON_INVALID, 0.15f);
    bindings_[Action::MoveRight].emplace_back(Action::MoveRight, ActionSource::GamepadAxis, SDL_SCANCODE_UNKNOWN, 0, SDL_GAMEPAD_AXIS_LEFTX, 1, SDL_GAMEPAD_BUTTON_INVALID, 0.15f);

    for (const auto& [action, actionBindings] : bindings_) {
        for (const auto& binding : actionBindings) {
            if (binding.source == ActionSource::KeyboardMouse && binding.key != SDL_SCANCODE_UNKNOWN) {
                keyMap_[binding.key] = action;
            } else if (binding.source == ActionSource::GamepadAxis && binding.axis != SDL_GAMEPAD_AXIS_INVALID) {
                axisMap_[binding.axis].emplace_back(action, binding.axisSign);
            } else if (binding.source == ActionSource::GamepadButton && binding.button != SDL_GAMEPAD_BUTTON_INVALID) {
                gamepadButtonMap_[binding.button] = action;
            } else if (binding.source == ActionSource::KeyboardMouse && binding.mouseButton != 0) {
                mouseMap_[binding.mouseButton] = action;
            }
        }
    }
}

void ActionMap::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        setDefaultBindings();
        return;
    }
    setDefaultBindings();
}

void ActionMap::save(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (out.is_open()) {
        out << "ZE_BINDINGS_V1\n";
        for (const auto& [action, actionBindings] : bindings_) {
            for (const auto& binding : actionBindings) {
                if (binding.source == ActionSource::KeyboardMouse && binding.key != SDL_SCANCODE_UNKNOWN) {
                    out << "k " << static_cast<int>(action) << " " << static_cast<int>(binding.key) << "\n";
                } else if (binding.source == ActionSource::GamepadButton && binding.button != SDL_GAMEPAD_BUTTON_INVALID) {
                    out << "b " << static_cast<int>(action) << " " << static_cast<int>(binding.button) << "\n";
                } else if (binding.source == ActionSource::GamepadAxis && binding.axis != SDL_GAMEPAD_AXIS_INVALID) {
                    out << "a " << static_cast<int>(action) << " " << static_cast<int>(binding.axis) << " " << binding.axisSign << "\n";
                }
            }
        }
    }
}

void ActionMap::update(const SDL_Event* events, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        processEvent(events[i]);
    }
}

void ActionMap::processEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        auto it = keyMap_.find(static_cast<SDL_Scancode>(event.key.scancode));
        if (it != keyMap_.end()) {
            set(it->second, true, 1.0f);
        }
    } else if (event.type == SDL_EVENT_KEY_UP) {
        auto it = keyMap_.find(static_cast<SDL_Scancode>(event.key.scancode));
        if (it != keyMap_.end()) {
            set(it->second, false, 0.0f);
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        auto it = mouseMap_.find(event.button.button);
        if (it != mouseMap_.end()) {
            set(it->second, true, 1.0f);
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        auto it = mouseMap_.find(event.button.button);
        if (it != mouseMap_.end()) {
            set(it->second, false, 0.0f);
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && gamepadConnected_) {
        auto it = axisMap_.find(static_cast<SDL_GamepadAxis>(event.gaxis.axis));
        if (it != axisMap_.end()) {
            for (const auto& [action, sign] : it->second) {
                const auto& actionBindings = bindings_[action];
                float deadzone = 0.15f;
                for (const auto& binding : actionBindings) {
                    if (binding.source == ActionSource::GamepadAxis && binding.axis == static_cast<SDL_GamepadAxis>(event.gaxis.axis)) {
                        deadzone = binding.axisDeadzone;
                        break;
                    }
                }
                const float value = sign * applyDeadzone(event.gaxis.value / 32767.0f, deadzone);
                applyAxis(action, value);
            }
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && gamepadConnected_) {
        auto it = gamepadButtonMap_.find(static_cast<SDL_GamepadButton>(event.gbutton.button));
        if (it != gamepadButtonMap_.end()) {
            applyButton(it->second, true);
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP && gamepadConnected_) {
        auto it = gamepadButtonMap_.find(static_cast<SDL_GamepadButton>(event.gbutton.button));
        if (it != gamepadButtonMap_.end()) {
            applyButton(it->second, false);
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        if (SDL_OpenGamepad(event.gdevice.which)) {
            gamepadConnected_ = true;
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        SDL_CloseGamepad(SDL_GetGamepadFromID(event.gdevice.which));
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        gamepadConnected_ = count > 0;
        SDL_free(ids);
    }
}

void ActionMap::setGamepadConnected(bool connected) {
    gamepadConnected_ = connected;
}

bool ActionMap::isGamepadConnected() const {
    return gamepadConnected_;
}

void ActionMap::applyAxis(Action action, float value) {
    ActionState& state = states_[action];
    state.analogValue = value;
    if (value != 0.0f) {
        state.held = true;
        state.pressed = true;
    } else {
        state.released = true;
        state.held = false;
    }
}

void ActionMap::applyButton(Action action, bool down) {
    ActionState& state = states_[action];
    state.analogValue = down ? 1.0f : 0.0f;
    if (down) {
        state.pressed = true;
        state.held = true;
        state.released = false;
    } else {
        state.released = true;
        state.held = false;
        state.pressed = false;
    }
}

void ActionMap::set(Action action, bool down, float analogValue) {
    if (down) {
        applyButton(action, true);
    } else {
        applyButton(action, false);
    }
    states_[action].analogValue = analogValue;
}

const ActionState& ActionMap::get(Action action) const {
    static ActionState empty;
    auto it = states_.find(action);
    return it != states_.end() ? it->second : empty;
}

void ActionMap::resetFrameState() {
    for (auto& [_, state] : states_) {
        state.pressed = false;
        state.released = false;
    }
}

} // namespace core