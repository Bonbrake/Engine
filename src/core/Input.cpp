#include "Input.h"
#include "Config.h"
#include "Logger.h"
#include <cstdio>

namespace core {

InputState Input::state_;

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

void Input::poll() {
    state_.events.clear();
    state_.quit = false;

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
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            state_.quit = true;
        }
        state_.events.push_back(event);
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
