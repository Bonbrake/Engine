#include "Input.h"
#include "Config.h"
#include "Logger.h"
#include <cstdio>
#include <sstream>
#include <fstream>

namespace core {

InputState Input::state_;
std::vector<ScriptFrame> Input::script_;
size_t Input::scriptFrame_ = 0;

static FILE* replayFile = nullptr;

void Input::init() {
    state_ = InputState{};
    if (Config::get().recordInput) {
        replayFile = fopen("replay.bin", "wb");
        if (!replayFile) {
            LOG_ERROR("Failed to open replay.bin for writing!");
        }
    } else if (Config::get().replayInput) {
        replayFile = fopen("replay.bin", "rb");
        if (!replayFile) {
            LOG_ERROR("Failed to open replay.bin for reading!");
        }
    }
}

// [M2.6 Phase 2] Scripted-input parser. Format (one directive per line):
//   K <key> <f0> <f1>   hold <key> (scancode name or W/A/S/D) for frames [f0,f1]
//   M <dx> <dy> <f0> <f1>  apply mouse delta (dx,dy) each frame in [f0,f1]
// Lines starting with '#' are comments. Returns false on any parse error.
static SDL_Scancode resolveScancode(const std::string& s) {
    if (s == "W") return SDL_SCANCODE_W;
    if (s == "A") return SDL_SCANCODE_A;
    if (s == "S") return SDL_SCANCODE_S;
    if (s == "D") return SDL_SCANCODE_D;
    if (s == "SPACE") return SDL_SCANCODE_SPACE;
    if (s == "LCTRL") return SDL_SCANCODE_LCTRL;
    // Try the SDL scancode name table (e.g. "SDL_SCANCODE_W" or "W" already done).
    SDL_Scancode sc = SDL_GetScancodeFromName(s.c_str());
    if (sc == SDL_SCANCODE_UNKNOWN) {
        // Allow raw numeric scancode.
        try { return static_cast<SDL_Scancode>(std::stoi(s)); } catch (...) { return SDL_SCANCODE_UNKNOWN; }
    }
    return sc;
}

bool Input::loadScript(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Scripted-input: cannot open '{}'", path);
        return false;
    }
    script_.clear();
    scriptFrame_ = 0;
    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        lineNo++;
        // Trim leading whitespace
        size_t b = line.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) continue;
        if (line[b] == '#') continue;
        std::istringstream iss(line.substr(b));
        std::string op;
        if (!(iss >> op)) continue;
        if (op == "K") {
            std::string key; int f0, f1;
            if (!(iss >> key >> f0 >> f1)) {
                LOG_ERROR("Scripted-input line {}: bad K directive", lineNo);
                return false;
            }
            SDL_Scancode sc = resolveScancode(key);
            if (sc == SDL_SCANCODE_UNKNOWN) {
                LOG_ERROR("Scripted-input line {}: unknown key '{}'", lineNo, key);
                return false;
            }
            if (f1 >= static_cast<int>(script_.size())) script_.resize(f1 + 1);
            for (int f = f0; f <= f1; ++f) {
                if (f >= static_cast<int>(script_.size())) script_.resize(f + 1);
                script_[f].keysDown.push_back(sc);
                // Release on the frame AFTER the hold ends, so the key stays held
                // across [f0,f1] (KEY_DOWN each frame, KEY_UP only at f1+1).
                int up = f1 + 1;
                if (up >= static_cast<int>(script_.size())) script_.resize(up + 1);
                script_[up].keysUp.push_back(sc);
            }
        } else if (op == "M") {
            int dx, dy, f0, f1;
            if (!(iss >> dx >> dy >> f0 >> f1)) {
                LOG_ERROR("Scripted-input line {}: bad M directive", lineNo);
                return false;
            }
            if (f1 >= static_cast<int>(script_.size())) script_.resize(f1 + 1);
            for (int f = f0; f <= f1; ++f) {
                if (f >= static_cast<int>(script_.size())) script_.resize(f + 1);
                script_[f].mouseDX += dx;
                script_[f].mouseDY += dy;
            }
        } else {
            LOG_ERROR("Scripted-input line {}: unknown directive '{}'", lineNo, op);
            return false;
        }
    }
    LOG_INFO("Scripted-input loaded: {} frames", script_.size());
    return true;
}

void Input::poll() {
    state_.events.clear();
    state_.quit = false;

    // [M2.6 Phase 2] Scripted-input mode: replay per-frame events, no display.
    if (!script_.empty()) {
        ScriptFrame sf;
        if (scriptFrame_ < script_.size()) sf = script_[scriptFrame_];
        for (SDL_Scancode sc : sf.keysDown) {
            SDL_Event ev; ev.type = SDL_EVENT_KEY_DOWN; ev.key.scancode = sc; ev.key.down = true;
            state_.events.push_back(ev);
        }
        for (SDL_Scancode sc : sf.keysUp) {
            SDL_Event ev; ev.type = SDL_EVENT_KEY_UP; ev.key.scancode = sc; ev.key.down = false;
            state_.events.push_back(ev);
        }
        if (sf.mouseDX != 0 || sf.mouseDY != 0) {
            SDL_Event ev; ev.type = SDL_EVENT_MOUSE_MOTION; ev.motion.xrel = sf.mouseDX; ev.motion.yrel = sf.mouseDY;
            state_.events.push_back(ev);
        }
        state_.mouseDX = sf.mouseDX;
        state_.mouseDY = sf.mouseDY;
        for (const auto& ev : state_.events) {
            if (ev.type == SDL_EVENT_KEY_DOWN)        state_.keyboardState[ev.key.scancode] = 1;
            else if (ev.type == SDL_EVENT_KEY_UP)     state_.keyboardState[ev.key.scancode] = 0;
        }
        if (scriptFrame_ + 1 < script_.size() || !script_.empty()) scriptFrame_++;
        return;
    }

    if (Config::get().replayInput && replayFile) {
        size_t count = 0;
        if (fread(&count, sizeof(size_t), 1, replayFile) == 1) {
            if (count > 10000) {
                LOG_ERROR("Replay file count {} is too large (corrupted). Limiting to 0.", count);
                count = 0;
            }
            state_.events.resize(count);
            if (count > 0) {
                fread(state_.events.data(), sizeof(SDL_Event), count, replayFile);
            }
        }
        for (const auto& ev : state_.events) {
            if (ev.type == SDL_EVENT_QUIT) state_.quit = true;
        }
        // Pump events anyway to keep OS happy, but ignore them
        SDL_Event dummy;
        while (SDL_PollEvent(&dummy)) {}

        // [M2.6 Phase 2] Derive fly-camera state from replayed events too.
        state_.mouseDX = 0;
        state_.mouseDY = 0;
        for (const auto& ev : state_.events) {
            if (ev.type == SDL_EVENT_KEY_DOWN)        state_.keyboardState[ev.key.scancode] = 1;
            else if (ev.type == SDL_EVENT_KEY_UP)     state_.keyboardState[ev.key.scancode] = 0;
            else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                state_.mouseDX += (int)ev.motion.xrel;
                state_.mouseDY += (int)ev.motion.yrel;
            }
        }
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            state_.quit = true;
        }
        state_.events.push_back(event);
    }

    // [M2.6 Phase 2] Derive fly-camera state from this frame's events.
    state_.mouseDX = 0;
    state_.mouseDY = 0;
    for (const auto& ev : state_.events) {
        if (ev.type == SDL_EVENT_KEY_DOWN)        state_.keyboardState[ev.key.scancode] = 1;
        else if (ev.type == SDL_EVENT_KEY_UP)     state_.keyboardState[ev.key.scancode] = 0;
        else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
            state_.mouseDX += (int)ev.motion.xrel;
            state_.mouseDY += (int)ev.motion.yrel;
        }
    }

    if (Config::get().recordInput && replayFile) {
        size_t count = state_.events.size();
        fwrite(&count, sizeof(size_t), 1, replayFile);
        if (count > 0) {
            fwrite(state_.events.data(), sizeof(SDL_Event), count, replayFile);
        }
    }
}

void Input::shutdown() {
    if (replayFile) {
        fclose(replayFile);
        replayFile = nullptr;
    }
}

} // namespace core
