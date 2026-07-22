#include "ze/save/SaveSystem.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <algorithm>

namespace save {

bool SaveSystem::initialize(const std::string& savePath) {
    savePath_ = savePath;
    try {
        std::filesystem::create_directories(savePath_);
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        return false;
    }
}

void SaveSystem::shutdown() {
    initialized_ = false;
}

std::string SaveSystem::slotPath(uint32_t slot) const {
    return savePath_ + "/slot_" + std::to_string(slot) + ".zesave";
}

uint32_t SaveSystem::computeChecksum(const uint8_t* data, size_t size) {
    // Simple FNV-1a hash for deterministic checksum (M9, M2.8-EXT-09)
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

bool SaveSystem::verifyChecksum(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    size_t fileSize = file.tellg();
    if (fileSize < sizeof(SnapshotHeader)) return false;

    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    SnapshotHeader header;
    std::memcpy(&header, data.data(), sizeof(SnapshotHeader));

    // Compute checksum of everything after the stored checksum field
    static constexpr size_t CHECKSUM_OFFSET = offsetof(SnapshotHeader, checksum);
    uint32_t computed = computeChecksum(
        data.data() + CHECKSUM_OFFSET + sizeof(uint32_t),
        fileSize - CHECKSUM_OFFSET - sizeof(uint32_t));

    return computed == header.checksum;
}

bool SaveSystem::validateHeader(const SnapshotHeader& header, uint64_t expectedSeed) const {
    if (std::memcmp(header.magic, "ZESAVE01", 8) != 0) {
        lastError_ = "Invalid save file magic";
        return false;
    }
    if (header.version < SCHEMA_MIN_COMPATIBLE || header.version > CURRENT_SCHEMA) {
        lastError_ = "Incompatible schema version";
        return false;
    }
    if (header.worldSeed != expectedSeed) {
        lastError_ = "World seed mismatch";
        return false;
    }
    return true;
}

bool SaveSystem::serialize(const std::string& path, const SnapshotHeader& header,
                            const std::vector<EntitySnapshot>& entities) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    // Write header
    SnapshotHeader writableHeader = header;
    file.write(reinterpret_cast<const char*>(&writableHeader), sizeof(SnapshotHeader));

    // Write entity count
    uint32_t count = static_cast<uint32_t>(entities.size());
    // (already stored in header.entityCount)

    // Write each entity
    for (const auto& ent : entities) {
        file.write(reinterpret_cast<const char*>(&ent.entityId), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&ent.componentMask), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&ent.position), sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(&ent.rotation), sizeof(glm::quat));
        file.write(reinterpret_cast<const char*>(&ent.velocity), sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(&ent.health), sizeof(float));
        file.write(reinterpret_cast<const char*>(&ent.maxHealth), sizeof(float));

        uint32_t extSize = static_cast<uint32_t>(ent.extensionData.size());
        file.write(reinterpret_cast<const char*>(&extSize), sizeof(uint32_t));
        if (extSize > 0) {
            file.write(reinterpret_cast<const char*>(ent.extensionData.data()), extSize);
        }
    }

    file.close();
    return true;
}

bool SaveSystem::deserialize(const std::string& path, SnapshotHeader& header,
                              std::vector<EntitySnapshot>& entities) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    // Read header
    file.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));

    if (std::memcmp(header.magic, "ZESAVE01", 8) != 0) {
        file.close();
        return false;
    }

    // Read entities
    for (uint32_t i = 0; i < header.entityCount && file; ++i) {
        EntitySnapshot ent;
        file.read(reinterpret_cast<char*>(&ent.entityId), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&ent.componentMask), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&ent.position), sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(&ent.rotation), sizeof(glm::quat));
        file.read(reinterpret_cast<char*>(&ent.velocity), sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(&ent.health), sizeof(float));
        file.read(reinterpret_cast<char*>(&ent.maxHealth), sizeof(float));

        uint32_t extSize = 0;
        file.read(reinterpret_cast<char*>(&extSize), sizeof(uint32_t));
        if (extSize > 0) {
            ent.extensionData.resize(extSize);
            file.read(reinterpret_cast<char*>(ent.extensionData.data()), extSize);
        }

        entities.push_back(ent);
    }

    file.close();
    return true;
}

bool SaveSystem::saveGame(uint32_t slot, const std::string& name,
                           uint64_t worldSeed, uint32_t chunkCount,
                           const std::vector<EntitySnapshot>& entities,
                           const glm::vec3& playerPos, const std::string& playerName) {
    if (!initialized_ || slot >= MAX_SLOTS) return false;

    SnapshotHeader header;
    header.timestamp = std::time(nullptr);
    header.worldSeed = worldSeed;
    header.chunkCount = chunkCount;
    header.entityCount = static_cast<uint32_t>(entities.size());
    header.checksum = 0; // placeholder

    std::string path = slotPath(slot);
    if (!serialize(path, header, entities)) {
        lastError_ = "Failed to write save file";
        return false;
    }

    // Read back and compute checksum
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;
    size_t fileSize = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    // Recompute checksum without the current checksum field
    static constexpr size_t CHECKSUM_OFFSET = offsetof(SnapshotHeader, checksum);
    uint32_t checksum = computeChecksum(
        data.data() + CHECKSUM_OFFSET + sizeof(uint32_t),
        fileSize - CHECKSUM_OFFSET - sizeof(uint32_t));

    // Rewrite header with correct checksum
    std::fstream rewrite(path, std::ios::binary | std::ios::in | std::ios::out);
    if (rewrite) {
        rewrite.seekp(CHECKSUM_OFFSET);
        rewrite.write(reinterpret_cast<const char*>(&checksum), sizeof(uint32_t));
        rewrite.close();
    }

    return true;
}

bool SaveSystem::loadGame(uint32_t slot, std::vector<EntitySnapshot>& entities,
                           glm::vec3& playerPos, std::string& playerName) {
    if (!initialized_ || slot >= MAX_SLOTS) return false;

    std::string path = slotPath(slot);
    if (!std::filesystem::exists(path)) {
        lastError_ = "Save file not found";
        return false;
    }

    SnapshotHeader header;
    entities.clear();

    if (!deserialize(path, header, entities)) {
        lastError_ = "Failed to read save file";
        return false;
    }

    // Populate player info from first entity
    if (!entities.empty()) {
        playerPos = entities[0].position;
    }

    return true;
}

bool SaveSystem::deleteSlot(uint32_t slot) {
    if (slot >= MAX_SLOTS) return false;
    try {
        return std::filesystem::remove(slotPath(slot));
    } catch (...) {
        return false;
    }
}

std::vector<SaveSlot> SaveSystem::listSlots() const {
    std::vector<SaveSlot> slots;
    if (!initialized_) return slots;

    for (uint32_t i = 0; i < MAX_SLOTS; ++i) {
        std::string path = slotPath(i);
        if (!std::filesystem::exists(path)) continue;

        SaveSlot slot;
        slot.slotIndex = i;
        slot.name = "Slot " + std::to_string(i);

        // Read header for metadata
        std::ifstream file(path, std::ios::binary);
        if (file) {
            SnapshotHeader header;
            file.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));
            slot.timestamp = header.timestamp;
            slot.version = header.version;
            slot.chunkCount = header.chunkCount;
            slot.worldSeed = header.worldSeed;
            file.close();
        }

        // File modification time
        auto ftime = std::filesystem::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        slot.timestamp = std::chrono::system_clock::to_time_t(sctp);

        slots.push_back(slot);
    }
    return slots;
}

bool SaveSystem::slotExists(uint32_t slot) const {
    return std::filesystem::exists(slotPath(slot));
}

bool SaveSystem::migrateSchema(const std::string& filePath, uint32_t fromVersion) {
    // Future: handle schema migrations between versions
    // Current schema (v1) is the only version
    return fromVersion >= SCHEMA_MIN_COMPATIBLE && fromVersion <= CURRENT_SCHEMA;
}

void SaveSystem::tick(float dt) {
    if (!autosave_ || !initialized_) return;
    autosaveTimer_ += dt;
    // Autosave is triggered externally by checking timeSinceLastAutosave()
}

} // namespace save
