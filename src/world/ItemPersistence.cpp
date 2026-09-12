#include "ze/world/ItemPersistence.h"
#include <cstring>
#include <algorithm>

namespace ze::world {

ItemPersistenceSystem::ItemPersistenceSystem() {
    initialize(13370420ULL);
}

void ItemPersistenceSystem::initialize(uint64_t worldSeed) {
    worldSeed_ = worldSeed;
    nextGuidCounter_ = 1ULL;
    items_.clear();
    chunkDeltas_.clear();
    grabConstraint_ = PhysicsGrabConstraint{};
}

void ItemPersistenceSystem::reset() {
    items_.clear();
    chunkDeltas_.clear();
    grabConstraint_ = PhysicsGrabConstraint{};
}

uint64_t ItemPersistenceSystem::generateNextGuid() {
    uint64_t id = nextGuidCounter_++;
    uint64_t salt = (worldSeed_ & 0xFFFF0000ULL) << 32;
    return id | salt;
}

uint64_t ItemPersistenceSystem::packChunkKey(uint32_t x, uint32_t z) {
    return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(z);
}

void ItemPersistenceSystem::worldPosToChunkCoords(const glm::vec3& pos, uint32_t& outChunkX, uint32_t& outChunkZ) {
    int32_t cx = static_cast<int32_t>(std::floor(pos.x / CHUNK_SIZE_METERS));
    int32_t cz = static_cast<int32_t>(std::floor(pos.z / CHUNK_SIZE_METERS));
    outChunkX = static_cast<uint32_t>(cx + 32768);
    outChunkZ = static_cast<uint32_t>(cz + 32768);
}

uint64_t ItemPersistenceSystem::spawnWorldItem(uint32_t archetypeId, const glm::vec3& position, 
                                               const glm::quat& rotation, const glm::vec3& initialVelocity,
                                               uint32_t stackCount, float condition,
                                               PersistenceTier tier) {
    uint64_t guid = generateNextGuid();
    PersistentItemData item;
    item.persistentGuid = guid;
    item.archetypeId = archetypeId;
    item.position = position;
    item.rotation = rotation;
    item.linearVelocity = initialVelocity;
    item.angularVelocity = glm::vec3(0.0f);
    item.tier = tier;
    item.state = (glm::length(initialVelocity) < VELOCITY_SETTLE_EPSILON) 
                 ? ItemPhysicalState::WorldSettled 
                 : ItemPhysicalState::WorldDynamic;
    item.stackCount = stackCount;
    item.condition = condition;
    item.containerGuid = 0;
    item.settleTimer = (item.state == ItemPhysicalState::WorldSettled) ? REQUIRED_SETTLE_DURATION : 0.0f;
    item.isKinematicSleeper = (item.state == ItemPhysicalState::WorldSettled);
    item.softContactFrames = 0;

    worldPosToChunkCoords(position, item.chunkX, item.chunkZ);

    items_[guid] = item;
    uint64_t chunkKey = packChunkKey(item.chunkX, item.chunkZ);
    chunkDeltas_[chunkKey].push_back(guid);

    if (tier == PersistenceTier::Tier2_ClutterLRU) {
        enforceClutterCap(item.chunkX, item.chunkZ);
    }

    return guid;
}

void ItemPersistenceSystem::enforceClutterCap(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    auto it = chunkDeltas_.find(chunkKey);
    if (it == chunkDeltas_.end()) return;

    std::vector<uint64_t> clutterGuids;
    for (uint64_t guid : it->second) {
        auto itemIt = items_.find(guid);
        if (itemIt != items_.end() && itemIt->second.tier == PersistenceTier::Tier2_ClutterLRU) {
            clutterGuids.push_back(guid);
        }
    }

    if (clutterGuids.size() > MAX_CLUTTER_PER_CHUNK) {
        size_t toRemove = clutterGuids.size() - MAX_CLUTTER_PER_CHUNK;
        for (size_t i = 0; i < toRemove; ++i) {
            uint64_t purgeGuid = clutterGuids[i];
            items_.erase(purgeGuid);
            it->second.erase(std::remove(it->second.begin(), it->second.end(), purgeGuid), it->second.end());
        }
    }
}

void ItemPersistenceSystem::wakeItem(uint64_t itemGuid) {
    auto it = items_.find(itemGuid);
    if (it == items_.end()) return;

    it->second.state = ItemPhysicalState::WorldDynamic;
    it->second.isKinematicSleeper = false;
    it->second.settleTimer = 0.0f;
    it->second.softContactFrames = 3; // 3 frames soft contact relaxation
}

bool ItemPersistenceSystem::pickUpItem(uint64_t itemGuid, uint64_t inventoryOwnerId) {
    auto it = items_.find(itemGuid);
    if (it == items_.end()) return false;

    if (grabConstraint_.active && grabConstraint_.grabbedItemGuid == itemGuid) {
        releasePhysicsGrab();
    }

    it->second.state = ItemPhysicalState::PlayerInventory;
    it->second.linearVelocity = glm::vec3(0.0f);
    it->second.angularVelocity = glm::vec3(0.0f);
    it->second.containerGuid = inventoryOwnerId;

    uint64_t chunkKey = packChunkKey(it->second.chunkX, it->second.chunkZ);
    auto chunkIt = chunkDeltas_.find(chunkKey);
    if (chunkIt != chunkDeltas_.end()) {
        auto& list = chunkIt->second;
        list.erase(std::remove(list.begin(), list.end(), itemGuid), list.end());
    }

    return true;
}

uint64_t ItemPersistenceSystem::dropItemFromInventory(uint64_t itemGuid, const glm::vec3& eyePos, const glm::vec3& throwVelocity) {
    auto it = items_.find(itemGuid);
    if (it == items_.end()) return 0;

    it->second.position = eyePos;
    it->second.linearVelocity = throwVelocity;
    it->second.angularVelocity = glm::vec3(0.5f, 1.0f, 0.2f);
    it->second.state = ItemPhysicalState::WorldDynamic;
    it->second.containerGuid = 0;
    it->second.settleTimer = 0.0f;

    worldPosToChunkCoords(it->second.position, it->second.chunkX, it->second.chunkZ);

    uint64_t chunkKey = packChunkKey(it->second.chunkX, it->second.chunkZ);
    chunkDeltas_[chunkKey].push_back(itemGuid);

    return itemGuid;
}

bool ItemPersistenceSystem::startPhysicsGrab(uint64_t itemGuid, const glm::vec3& eyePos, const glm::vec3& forwardDir) {
    auto it = items_.find(itemGuid);
    if (it == items_.end()) return false;

    float dist = glm::distance(eyePos, it->second.position);
    if (dist > grabConstraint_.maxReachDistance) return false;

    if (grabConstraint_.active) {
        releasePhysicsGrab();
    }

    grabConstraint_.active = true;
    grabConstraint_.grabbedItemGuid = itemGuid;
    grabConstraint_.targetAnchor = eyePos + forwardDir * grabConstraint_.holdDistance;
    grabConstraint_.targetOrientation = it->second.rotation;

    it->second.state = ItemPhysicalState::GrabbedPhysics;
    it->second.settleTimer = 0.0f;

    return true;
}

void ItemPersistenceSystem::updatePhysicsGrab(const glm::vec3& eyePos, const glm::vec3& forwardDir, float dt) {
    if (!grabConstraint_.active) return;

    auto it = items_.find(grabConstraint_.grabbedItemGuid);
    if (it == items_.end()) {
        grabConstraint_.active = false;
        return;
    }

    grabConstraint_.targetAnchor = eyePos + forwardDir * grabConstraint_.holdDistance;

    glm::vec3 posError = grabConstraint_.targetAnchor - it->second.position;
    glm::vec3 springForce = posError * grabConstraint_.springStiffness;
    glm::vec3 dampingForce = -it->second.linearVelocity * grabConstraint_.damping;
    glm::vec3 accel = springForce + dampingForce;

    it->second.linearVelocity += accel * dt;
    it->second.position += it->second.linearVelocity * dt;

    it->second.rotation = glm::slerp(it->second.rotation, grabConstraint_.targetOrientation, std::clamp(dt * 12.0f, 0.0f, 1.0f));

    uint32_t newChunkX, newChunkZ;
    worldPosToChunkCoords(it->second.position, newChunkX, newChunkZ);
    if (newChunkX != it->second.chunkX || newChunkZ != it->second.chunkZ) {
        uint64_t oldKey = packChunkKey(it->second.chunkX, it->second.chunkZ);
        auto oldIt = chunkDeltas_.find(oldKey);
        if (oldIt != chunkDeltas_.end()) {
            oldIt->second.erase(std::remove(oldIt->second.begin(), oldIt->second.end(), it->first), oldIt->second.end());
        }
        it->second.chunkX = newChunkX;
        it->second.chunkZ = newChunkZ;
        chunkDeltas_[packChunkKey(newChunkX, newChunkZ)].push_back(it->first);
    }
}

void ItemPersistenceSystem::rotateGrabbedItem(float yawDeltaDegrees, float pitchDeltaDegrees) {
    if (!grabConstraint_.active) return;

    float yawRad = glm::radians(yawDeltaDegrees);
    float pitchRad = glm::radians(pitchDeltaDegrees);

    glm::quat qYaw = glm::angleAxis(yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));

    grabConstraint_.targetOrientation = glm::normalize(qYaw * qPitch * grabConstraint_.targetOrientation);
}

void ItemPersistenceSystem::releasePhysicsGrab() {
    if (!grabConstraint_.active) return;

    auto it = items_.find(grabConstraint_.grabbedItemGuid);
    if (it != items_.end()) {
        it->second.state = ItemPhysicalState::WorldDynamic;
        it->second.settleTimer = 0.0f;
    }

    grabConstraint_.active = false;
    grabConstraint_.grabbedItemGuid = 0;
}

void ItemPersistenceSystem::update(float dt) {
    for (auto& [guid, item] : items_) {
        if (item.state == ItemPhysicalState::WorldDynamic) {
            // Soft contact relaxation filter: damp separation impulses for first 3 frames upon waking
            if (item.softContactFrames > 0) {
                item.softContactFrames--;
                float curSpeed = glm::length(item.linearVelocity);
                if (curSpeed > 1.5f) {
                    item.linearVelocity = (item.linearVelocity / curSpeed) * 1.5f;
                }
            }

            item.linearVelocity *= std::max(0.0f, 1.0f - dt * 2.5f);
            item.angularVelocity *= std::max(0.0f, 1.0f - dt * 4.0f);
            item.position += item.linearVelocity * dt;

            float speed = glm::length(item.linearVelocity);
            float angSpeed = glm::length(item.angularVelocity);

            if (speed < VELOCITY_SETTLE_EPSILON && angSpeed < ANGULAR_SETTLE_EPSILON) {
                item.settleTimer += dt;
                if (item.settleTimer >= REQUIRED_SETTLE_DURATION) {
                    item.state = ItemPhysicalState::WorldSettled;
                    item.isKinematicSleeper = true;
                    item.linearVelocity = glm::vec3(0.0f);
                    item.angularVelocity = glm::vec3(0.0f);
                }
            } else {
                item.settleTimer = 0.0f;
            }
        }
    }
}

void ItemPersistenceSystem::onChunkLoaded(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    auto it = chunkDeltas_.find(chunkKey);
    if (it == chunkDeltas_.end()) return;

    // Initialize all settled items as kinematic sleepers to eliminate penetration explosions
    for (uint64_t guid : it->second) {
        auto itemIt = items_.find(guid);
        if (itemIt != items_.end() && itemIt->second.state == ItemPhysicalState::WorldSettled) {
            itemIt->second.isKinematicSleeper = true;
            itemIt->second.linearVelocity = glm::vec3(0.0f);
            itemIt->second.angularVelocity = glm::vec3(0.0f);
        }
    }
}

void ItemPersistenceSystem::onChunkUnloaded(uint32_t chunkX, uint32_t chunkZ) {
    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    auto it = chunkDeltas_.find(chunkKey);
    if (it == chunkDeltas_.end()) return;

    // Purge Tier 3 Ephemeral items (spent brass, decals, gore)
    std::vector<uint64_t> toErase;
    for (uint64_t guid : it->second) {
        auto itemIt = items_.find(guid);
        if (itemIt != items_.end() && itemIt->second.tier == PersistenceTier::Tier3_Ephemeral) {
            toErase.push_back(guid);
        }
    }

    for (uint64_t guid : toErase) {
        items_.erase(guid);
        it->second.erase(std::remove(it->second.begin(), it->second.end(), guid), it->second.end());
    }
}

const PersistentItemData* ItemPersistenceSystem::getItem(uint64_t guid) const {
    auto it = items_.find(guid);
    return (it != items_.end()) ? &it->second : nullptr;
}

PersistentItemData* ItemPersistenceSystem::getMutableItem(uint64_t guid) {
    auto it = items_.find(guid);
    return (it != items_.end()) ? &it->second : nullptr;
}

size_t ItemPersistenceSystem::getSettledItemCount() const {
    size_t count = 0;
    for (const auto& [_, item] : items_) {
        if (item.state == ItemPhysicalState::WorldSettled) ++count;
    }
    return count;
}

size_t ItemPersistenceSystem::getDynamicItemCount() const {
    size_t count = 0;
    for (const auto& [_, item] : items_) {
        if (item.state == ItemPhysicalState::WorldDynamic) ++count;
    }
    return count;
}

std::vector<uint64_t> ItemPersistenceSystem::getItemsInChunk(uint32_t chunkX, uint32_t chunkZ) const {
    uint64_t key = packChunkKey(chunkX, chunkZ);
    auto it = chunkDeltas_.find(key);
    return (it != chunkDeltas_.end()) ? it->second : std::vector<uint64_t>{};
}

#pragma pack(push, 1)
struct BinaryItemHeader {
    char magic[8]{ 'Z', 'E', 'I', 'T', 'E', 'M', '0', '1' };
    uint32_t version{ 1 };
    uint32_t itemCount{ 0 };
    uint64_t worldSeed{ 0 };
};

struct BinaryItemRecord {
    uint64_t guid;
    uint32_t archetypeId;
    float posX, posY, posZ;
    float rotW, rotX, rotY, rotZ;
    float velX, velY, velZ;
    uint8_t state;
    uint32_t chunkX, chunkZ;
    float condition;
    uint32_t stackCount;
    uint64_t containerGuid;
};
#pragma pack(pop)

bool ItemPersistenceSystem::serializeToSnapshot(std::vector<uint8_t>& outBinary) const {
    BinaryItemHeader header;
    header.version = 1;
    header.itemCount = static_cast<uint32_t>(items_.size());
    header.worldSeed = worldSeed_;

    size_t totalBytes = sizeof(BinaryItemHeader) + items_.size() * sizeof(BinaryItemRecord);
    outBinary.resize(totalBytes);

    std::memcpy(outBinary.data(), &header, sizeof(BinaryItemHeader));
    uint8_t* writePtr = outBinary.data() + sizeof(BinaryItemHeader);

    for (const auto& [guid, item] : items_) {
        BinaryItemRecord rec;
        rec.guid = item.persistentGuid;
        rec.archetypeId = item.archetypeId;
        rec.posX = item.position.x; rec.posY = item.position.y; rec.posZ = item.position.z;
        rec.rotW = item.rotation.w; rec.rotX = item.rotation.x; rec.rotY = item.rotation.y; rec.rotZ = item.rotation.z;
        rec.velX = item.linearVelocity.x; rec.velY = item.linearVelocity.y; rec.velZ = item.linearVelocity.z;
        rec.state = static_cast<uint8_t>(item.state);
        rec.chunkX = item.chunkX; rec.chunkZ = item.chunkZ;
        rec.condition = item.condition;
        rec.stackCount = item.stackCount;
        rec.containerGuid = item.containerGuid;

        std::memcpy(writePtr, &rec, sizeof(BinaryItemRecord));
        writePtr += sizeof(BinaryItemRecord);
    }

    return true;
}

bool ItemPersistenceSystem::deserializeFromSnapshot(const uint8_t* data, size_t size) {
    if (!data || size < sizeof(BinaryItemHeader)) return false;

    BinaryItemHeader header;
    std::memcpy(&header, data, sizeof(BinaryItemHeader));

    if (std::memcmp(header.magic, "ZEITEM01", 8) != 0) return false;

    size_t expectedSize = sizeof(BinaryItemHeader) + header.itemCount * sizeof(BinaryItemRecord);
    if (size < expectedSize) return false;

    reset();
    worldSeed_ = header.worldSeed;

    const uint8_t* readPtr = data + sizeof(BinaryItemHeader);
    for (uint32_t i = 0; i < header.itemCount; ++i) {
        BinaryItemRecord rec;
        std::memcpy(&rec, readPtr, sizeof(BinaryItemRecord));
        readPtr += sizeof(BinaryItemRecord);

        PersistentItemData item;
        item.persistentGuid = rec.guid;
        item.archetypeId = rec.archetypeId;
        item.position = glm::vec3(rec.posX, rec.posY, rec.posZ);
        item.rotation = glm::quat(rec.rotW, rec.rotX, rec.rotY, rec.rotZ);
        item.linearVelocity = glm::vec3(rec.velX, rec.velY, rec.velZ);
        item.angularVelocity = glm::vec3(0.0f);
        item.state = static_cast<ItemPhysicalState>(rec.state);
        item.chunkX = rec.chunkX;
        item.chunkZ = rec.chunkZ;
        item.condition = rec.condition;
        item.stackCount = rec.stackCount;
        item.containerGuid = rec.containerGuid;
        item.settleTimer = (item.state == ItemPhysicalState::WorldSettled) ? REQUIRED_SETTLE_DURATION : 0.0f;

        items_[item.persistentGuid] = item;
        if (item.state == ItemPhysicalState::WorldSettled || item.state == ItemPhysicalState::WorldDynamic) {
            uint64_t chunkKey = packChunkKey(item.chunkX, item.chunkZ);
            chunkDeltas_[chunkKey].push_back(item.persistentGuid);
        }

        if (item.persistentGuid >= nextGuidCounter_) {
            nextGuidCounter_ = (item.persistentGuid & 0xFFFFFFFFULL) + 1ULL;
        }
    }

    return true;
}

} // namespace ze::world
