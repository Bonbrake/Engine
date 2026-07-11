#include "FileSystem.h"
#include <filesystem>

namespace core {

std::string FileSystem::resolveAssetPath(const std::string& virtualPath) {
    // Basic virtual path resolver mapping mods/ if needed
    // In a full implementation, it would check multiple root directories.
    std::filesystem::path p(virtualPath);
    if (std::filesystem::exists("mods/" + virtualPath)) {
        return "mods/" + virtualPath;
    }
    return virtualPath; // fallback to CWD
}

} // namespace core
