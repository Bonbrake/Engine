#pragma once
#include <cstdint>
#include <cstddef>

namespace save {

// Schema versioning for save data compatibility (M9)
class SchemaVersion {
public:
    static constexpr uint32_t CURRENT = 1;
    static constexpr uint32_t MIN_COMPATIBLE = 1;

    static uint32_t currentVersion();
    static bool isCompatible(uint32_t version);
    static bool migrate(uint8_t* data, size_t size, uint32_t fromVersion);
    static const char* versionString(uint32_t version);
};

} // namespace save
