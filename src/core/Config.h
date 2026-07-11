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
    int crashFrame = -1;
    int quitFrame = -1;
    std::string settingsPath = "settings.json";
    std::unordered_set<std::string> overriddenFields;

    static Config& get();
    void parseCommandLine(int argc, char* argv[]);
    void loadSettings();
};

} // namespace core
