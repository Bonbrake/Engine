#pragma once

#include <string>

namespace core {

class FileSystem {
public:
    static std::string resolveAssetPath(const std::string& virtualPath);
};

} // namespace core
