#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ze::world {

// Bethesda-style Physical Item State Lifecycle
enum class ItemPhysicalState : uint8_t {
    WorldDynamic = 0,    // Active in Jolt rigid-body simulation (bouncing, tumbling, falling)
    WorldSettled = 1,    // Sleeping on a surface (shelf, floor, desk); zero tick overhead, exact transform locked
    GrabbedPhysics = 2,  // Skyrim/Fallout telekinetic/hand grab constraint (lift and arrange on tables/barricades)
    ContainerStored = 3, // Inside a persistent container (safe, locker, trunk, corpse inventory)
    PlayerInventory = 4, // Carried by player
    Destroyed = 5        // Consumed, incinerated, or purged
};

// 3-Tier Item Persistence Classification (prevents Skyrim save bloat)
enum class PersistenceTier : uint8_t {
    Tier1_Eternal = 0,    // Weapons, ammo boxes, medical supplies, containers, barricades (permanent)
    Tier2_ClutterLRU = 1, // Empty cans, mugs, bottles, loose junk (capped at 256 per chunk)
    Tier3_Ephemeral = 2   // Spent brass casings, blood decals, gore (purged on chunk unload)
};

// Persistent record for every physical object in the world
struct PersistentItemData {
    uint64_t persistentGuid{ 0 };     // Unique 64-bit UUID across the entire world
    uint32_t archetypeId{ 0 };        // Base FormID (Rifle, AmmoBox, Medkit, Mug, SodaCan, Skull, etc.)
    glm::vec3 position{ 0.0f };       // Authoritative resting or dynamic position
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f }; // Authoritative orientation quaternion
    glm::vec3 linearVelocity{ 0.0f }; // Current physical velocity
    glm::vec3 angularVelocity{ 0.0f };// Current rotational velocity
    ItemPhysicalState state{ ItemPhysicalState::WorldDynamic };
    PersistenceTier tier{ PersistenceTier::Tier1_Eternal };
    uint32_t chunkX{ 0 };             // Spatial chunk coordinate X
    uint32_t chunkZ{ 0 };             // Spatial chunk coordinate Z
    float condition{ 1.0f };          // Item durability / wear factor (0.0 to 1.0)
    uint32_t stackCount{ 1 };         // Stack count (e.g. quantity of loose rounds)
    uint64_t containerGuid{ 0 };      // GUID of parent container if ContainerStored
    float settleTimer{ 0.0f };        // Rest duration counter before transitioning to WorldSettled
    bool isKinematicSleeper{ true };  // Loaded as kinematic sleeper to eliminate Havok explosions
    uint32_t softContactFrames{ 0 };  // 3-frame soft contact relaxation filter on wake
};

// Skyrim/Fallout 6-DOF Physics Spring Grab Constraint
struct PhysicsGrabConstraint {
    bool active{ false };
    uint64_t grabbedItemGuid{ 0 };
    glm::vec3 targetAnchor{ 0.0f };
    glm::quat targetOrientation{ 1.0f, 0.0f, 0.0f, 0.0f };
    float springStiffness{ 140.0f };   // Hooke's spring stiffness for responsive hand-holding
    float damping{ 18.0f };           // Critical damping to prevent wild spinning
    float holdDistance{ 1.25f };       // Distance in front of eye camera (meters)
    float maxReachDistance{ 2.50f };   // Maximum interaction raycast range
};

// Chunk Spatial Item Delta Record
struct ChunkItemDelta {
    uint32_t chunkX{ 0 };
    uint32_t chunkZ{ 0 };
    std::vector<uint64_t> itemGuids;
};

// Authoritative Bethesda-Style World Item Persistence Subsystem
class ItemPersistenceSystem {
public:
    ItemPersistenceSystem();
    ~ItemPersistenceSystem() = default;

    void initialize(uint64_t worldSeed = 13370420ULL);
    void reset();

    // Spawning & World Drops
    uint64_t spawnWorldItem(uint32_t archetypeId, const glm::vec3& position, 
                            const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                            const glm::vec3& initialVelocity = glm::vec3(0.0f),
                            uint32_t stackCount = 1, float condition = 1.0f,
                            PersistenceTier tier = PersistenceTier::Tier1_Eternal);

    bool pickUpItem(uint64_t itemGuid, uint64_t inventoryOwnerId);
    uint64_t dropItemFromInventory(uint64_t itemGuid, const glm::vec3& eyePos, const glm::vec3& throwVelocity = glm::vec3(0.0f));

    // Wake item from kinematic sleep into dynamic physics with soft contact relaxation
    void wakeItem(uint64_t itemGuid);
    void enforceClutterCap(uint32_t chunkX, uint32_t chunkZ);

    // Skyrim/Fallout Hold-to-Grab Mechanics (Lift and arrange items on shelves/barricades)
    bool startPhysicsGrab(uint64_t itemGuid, const glm::vec3& eyePos, const glm::vec3& forwardDir);
    void updatePhysicsGrab(const glm::vec3& eyePos, const glm::vec3& forwardDir, float dt);
    void rotateGrabbedItem(float yawDeltaDegrees, float pitchDeltaDegrees);
    void releasePhysicsGrab();
    bool isGrabbingItem() const { return grabConstraint_.active; }
    uint64_t getGrabbedItemGuid() const { return grabConstraint_.grabbedItemGuid; }

    // Simulation Tick: Updates physics settling (transitions dynamic to sleeping settled)
    void update(float dt);

    // Chunk Streaming Lifecycle Integration
    void onChunkLoaded(uint32_t chunkX, uint32_t chunkZ);
    void onChunkUnloaded(uint32_t chunkX, uint32_t chunkZ);

    // Binary Snapshot Serialization (Integrates with SaveSystem .zesave snapshots)
    bool serializeToSnapshot(std::vector<uint8_t>& outBinary) const;
    bool deserializeFromSnapshot(const uint8_t* data, size_t size);

    // Queries
    const PersistentItemData* getItem(uint64_t guid) const;
    PersistentItemData* getMutableItem(uint64_t guid);
    size_t getTotalTrackedItems() const { return items_.size(); }
    size_t getSettledItemCount() const;
    size_t getDynamicItemCount() const;
    std::vector<uint64_t> getItemsInChunk(uint32_t chunkX, uint32_t chunkZ) const;

    static constexpr size_t MAX_CLUTTER_PER_CHUNK = 256;

    // Spatial coordinate calculation helper (256m chunk grid)
    static void worldPosToChunkCoords(const glm::vec3& pos, uint32_t& outChunkX, uint32_t& outChunkZ);

private:
    uint64_t generateNextGuid();
    static uint64_t packChunkKey(uint32_t x, uint32_t z);

    uint64_t worldSeed_{ 13370420ULL };
    uint64_t nextGuidCounter_{ 1ULL };

    std::unordered_map<uint64_t, PersistentItemData> items_;
    std::unordered_map<uint64_t, std::vector<uint64_t>> chunkDeltas_; // packedChunkKey -> itemGuids
    PhysicsGrabConstraint grabConstraint_;

    // Tuning constants
    static constexpr float VELOCITY_SETTLE_EPSILON = 0.04f; // m/s
    static constexpr float ANGULAR_SETTLE_EPSILON = 0.05f;  // rad/s
    static constexpr float REQUIRED_SETTLE_DURATION = 0.40f;// seconds at rest before sleep
    static constexpr float CHUNK_SIZE_METERS = 256.0f;
};

} // namespace ze::world
