#pragma once

#include <string>
#include <unordered_set>

namespace core {

struct Config {
    bool headless = false;
    bool forceTier0 = false;
    bool devMode = false;
    bool recordInput = false;
    bool replayInput = false;
    bool flyCamera = false;   // [M2.6 Phase 2] enable debug fly-camera (dev)
    int crashFrame = -1;
    int quitFrame = -1;
    float exposure = 1.0f;   // [M4.5-EXT-33] AgX tonemapper exposure multiplier
    std::string settingsPath = "settings.json";
    // Frame-dump tool (windowed --dev only; headless has no swapchain to capture).
    std::string dumpFramePath;   // empty => disabled
    int dumpFrameAt = -1;        // render-call index to capture; -1 => disabled
    // [M2.6 Phase 2] Scripted-input (self-verify without a display) + state-dump.
    std::string scriptInput;     // path to scripted-input text file; empty => disabled
    std::string dumpState;       // path for per-frame FlyCamera pose JSON; empty => disabled
    std::unordered_set<std::string> overriddenFields;

    static Config& get();
    void parseCommandLine(int argc, char* argv[]);
    void loadSettings();
};

} // namespace core
