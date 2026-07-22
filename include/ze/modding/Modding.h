#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace modding {

struct ModInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    uint64_t hash;          // content hash for integrity check
    bool enabled = true;
    std::vector<std::string> dependencies;
};

// Mod loader (M11)
class Modding {
public:
    Modding() = default;

    bool initialize(const std::string& modsPath);
    void shutdown();

    bool loadMod(const std::string& modPath);       // single mod
    bool unloadMod(const std::string& modName);
    std::vector<ModInfo> listMods() const;

    // Hook registration
    using HookFn = std::function<void()>;
    uint32_t registerHook(const std::string& name, HookFn fn);
    void triggerHook(const std::string& name);

    // Mod data access
    std::string getModDataPath(const std::string& modName) const;

private:
    std::string modsPath_;
    std::vector<ModInfo> mods_;
    bool initialized_ = false;
};

} // namespace modding
