#include "ze/save/SchemaVersion.h"
#include <algorithm>
#include <cstring>

namespace save {

uint32_t SchemaVersion::currentVersion() {
    return CURRENT;
}

bool SchemaVersion::isCompatible(uint32_t version) {
    return version >= MIN_COMPATIBLE && version <= CURRENT;
}

bool SchemaVersion::migrate(uint8_t* data, size_t size, uint32_t fromVersion) {
    if (!data || size == 0) return false;
    if (!isCompatible(fromVersion)) return false;
    if (fromVersion == CURRENT) return true;

    // Future: handle schema migrations between versions
    // v1 → v2: add entity flags field (example)
    // v2 → v3: add component version field (example)
    return false;
}

const char* SchemaVersion::versionString(uint32_t version) {
    switch (version) {
    case 1: return "Initial schema — ECS snapshot + checksum";
    default: return "Unknown version";
    }
}

} // namespace save
