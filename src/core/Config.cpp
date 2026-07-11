#include "Config.h"
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace core {

Config& Config::get() {
    static Config instance;
    return instance;
}

void Config::parseCommandLine(int argc, char* argv[]) {
    try {
        cxxopts::Options options("ZombieEngine", "Google Antigravity 2.0 Zombie Survival Engine M0");
        
        bool cl_headless = false;
        bool cl_forceTier0 = false;
        bool cl_devMode = false;
        bool cl_recordInput = false;
        bool cl_replayInput = false;
        int cl_crashFrame = -1;
        int cl_quitFrame = -1;
        std::string cl_settingsPath = "settings.json";

        options.add_options()
            ("headless", "Run without display", cxxopts::value<bool>(cl_headless))
            ("force-tier0", "Force Tier 0 capability", cxxopts::value<bool>(cl_forceTier0))
            ("dev", "Developer mode (validation layers)", cxxopts::value<bool>(cl_devMode))
            ("record-input", "Record input for replay", cxxopts::value<bool>(cl_recordInput))
            ("replay-input", "Replay recorded input", cxxopts::value<bool>(cl_replayInput))
            ("crash-frame", "Crash at specific frame number", cxxopts::value<int>(cl_crashFrame)->default_value("-1"))
            ("quit-frame", "Quit at specific frame number", cxxopts::value<int>(cl_quitFrame)->default_value("-1"))
            ("settings", "Path to settings.json", cxxopts::value<std::string>(cl_settingsPath)->default_value("settings.json"))
            ("h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (result.count("settings")) {
            settingsPath = cl_settingsPath;
        }

        // Apply command line values that were explicitly passed
        if (result.count("headless")) { headless = cl_headless; overriddenFields.insert("headless"); }
        if (result.count("force-tier0")) { forceTier0 = cl_forceTier0; overriddenFields.insert("forceTier0"); }
        if (result.count("dev")) { devMode = cl_devMode; overriddenFields.insert("devMode"); }
        if (result.count("record-input")) { recordInput = cl_recordInput; overriddenFields.insert("recordInput"); }
        if (result.count("replay-input")) { replayInput = cl_replayInput; overriddenFields.insert("replayInput"); }
        if (result.count("crash-frame")) { crashFrame = cl_crashFrame; overriddenFields.insert("crashFrame"); }
        if (result.count("quit-frame")) { quitFrame = cl_quitFrame; overriddenFields.insert("quitFrame"); }

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        exit(1);
    }
}

void Config::loadSettings() {
    std::ifstream file(settingsPath);
    if (file.is_open()) {
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains("headless") && !overriddenFields.count("headless")) headless = j["headless"].get<bool>();
            if (j.contains("forceTier0") && !overriddenFields.count("forceTier0")) forceTier0 = j["forceTier0"].get<bool>();
            if (j.contains("devMode") && !overriddenFields.count("devMode")) devMode = j["devMode"].get<bool>();
            if (j.contains("recordInput") && !overriddenFields.count("recordInput")) recordInput = j["recordInput"].get<bool>();
            if (j.contains("replayInput") && !overriddenFields.count("replayInput")) replayInput = j["replayInput"].get<bool>();
            if (j.contains("crashFrame") && !overriddenFields.count("crashFrame")) crashFrame = j["crashFrame"].get<int>();
            if (j.contains("quitFrame") && !overriddenFields.count("quitFrame")) quitFrame = j["quitFrame"].get<int>();
            std::cout << "Loaded settings from " << settingsPath << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse settings.json: " << e.what() << std::endl;
        }
    }
}

} // namespace core
