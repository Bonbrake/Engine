#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/detail/type_quat.hpp>
#include <functional>
#include <chrono>

namespace save {

// Save slot metadata
struct SaveSlot {
    uint32_t slotIndex;
    std::string name;
    uint64_t timestamp;     // seconds since epoch
    uint64_t playtimeSeconds;
    uint32_t version;        // schema version
    uint32_t chunkCount;     // number of chunks in this save
    uint64_t worldSeed;      // world seed for verification
    std::string playerName;
    glm::vec3 playerPosition;
};

// Deterministic world snapshot (M9, M2.8-EXT-09)
// Schema: binary archive with versioned headers.
//   [Header][ChunkData][EntityData][PlayerData][Metadata]

struct SnapshotHeader {
    char magic[8] = {'Z','E','S','A','V','E','0','1'};
    uint32_t version = 1;
    uint64_t timestamp;
    uint64_t worldSeed;
    uint32_t chunkCount;
    uint32_t entityCount;
    uint32_t checksum;
};

// Entity snapshot: serializes ECS component data
struct EntitySnapshot {
    uint32_t entityId;
    uint32_t componentMask; // bitmask of which components are present
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 velocity;
    float health, maxHealth;
    // Extension data blob (for components not in fixed schema)
    std::vector<uint8_t> extensionData;
};

class SaveSystem {
public:
    SaveSystem() = default;
    ~SaveSystem() = default;

    // Initialize save directory
    bool initialize(const std::string& savePath);
    void shutdown();

    // Save/Load operations
    bool saveGame(uint32_t slot, const std::string& name,
                  uint64_t worldSeed, uint32_t chunkCount,
                  const std::vector<EntitySnapshot>& entities,
                  const glm::vec3& playerPos, const std::string& playerName);

    bool loadGame(uint32_t slot, std::vector<EntitySnapshot>& entities,
                  glm::vec3& playerPos, std::string& playerName);

    // Slot management
    bool deleteSlot(uint32_t slot);
    std::vector<SaveSlot> listSlots() const;
    bool slotExists(uint32_t slot) const;

    // Schema management
    uint32_t currentSchemaVersion() const { return CURRENT_SCHEMA; }
    bool migrateSchema(const std::string& filePath, uint32_t fromVersion);

    // Configuration
    void setAutosave(bool enabled) { autosave_ = enabled; }
    void setAutosaveInterval(uint32_t seconds) { autosaveInterval_ = seconds; }
    bool autosaveEnabled() const { return autosave_; }
    float timeSinceLastAutosave() const { return autosaveTimer_; }

    // Per-tick: triggers autosave if interval elapsed
    void tick(float dt);

    // Serialization helpers
    static bool serialize(const std::string& path, const SnapshotHeader& header,
                          const std::vector<EntitySnapshot>& entities);
    static bool deserialize(const std::string& path, SnapshotHeader& header,
                            std::vector<EntitySnapshot>& entities);

    // Checksum computation (M2.8-EXT-09 deterministic verification)
    static uint32_t computeChecksum(const uint8_t* data, size_t size);
    static bool verifyChecksum(const std::string& path);

    // Error handling
    std::string lastError() const { return lastError_; }

    // Constants
    static constexpr uint32_t MAX_SLOTS = 50;
    static constexpr uint32_t CURRENT_SCHEMA = 1;
    static constexpr uint32_t SCHEMA_MIN_COMPATIBLE = 1;

private:
    std::string savePath_;
    bool initialized_ = false;
    bool autosave_ = true;
    uint32_t autosaveInterval_ = 300; // 5 minutes
    float autosaveTimer_ = 0.0f;
    mutable std::string lastError_;

    std::string slotPath(uint32_t slot) const;
    bool validateHeader(const SnapshotHeader& header, uint64_t expectedSeed) const;
};

} // namespace save
