#include "ze/modding/Modding.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace modding {

bool Modding::initialize(const std::string& modsPath) {
    modsPath_ = modsPath;
    try {
        std::filesystem::create_directories(modsPath_);
        initialized_ = true;
        return true;
    } catch (...) {
        return false;
    }
}

void Modding::shutdown() {
    mods_.clear();
    initialized_ = false;
}

bool Modding::loadMod(const std::string& modPath) {
    if (!initialized_) return false;
    ModInfo info;
    info.name = std::filesystem::path(modPath).filename().string();
    info.version = "1.0.0";
    info.enabled = true;
    mods_.push_back(info);
    return true;
}

bool Modding::unloadMod(const std::string& modName) {
    auto it = std::find_if(mods_.begin(), mods_.end(),
        [&](const ModInfo& m) { return m.name == modName; });
    if (it != mods_.end()) {
        mods_.erase(it);
        return true;
    }
    return false;
}

std::vector<ModInfo> Modding::listMods() const {
    return mods_;
}

uint32_t Modding::registerHook(const std::string& name, HookFn fn) {
    // Hooks are stored externally via event bus; placeholder
    return 0;
}

void Modding::triggerHook(const std::string& name) {
    // Placeholder: iterate registered hooks matching name
}

std::string Modding::getModDataPath(const std::string& modName) const {
    return modsPath_ + "/" + modName;
}

} // namespace modding
