#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ze::core {

enum class SteamActionSet : uint8_t {
    InGame = 0,
    Menu,
    TacticalWheel
};

enum class DigitalAction : uint32_t {
    Fire = 0,
    Reload,
    PointShootTuck,
    PhysicsGrab,
    TacticalWheel,
    Jump,
    Crouch,
    Interact,
    Flashlight,
    MeleeBash,
    Count
};

enum class AnalogAction : uint32_t {
    Move = 0,
    Aim,
    Count
};

struct SteamDeckProfile {
    bool isDeck = false;
    uint32_t displayWidth = 1280;
    uint32_t displayHeight = 800;
    float aspectRatio = 16.0f / 10.0f;
    float uiScaleMultiplier = 1.25f;
    bool gyroAdsEnabled = true;
    uint32_t targetFrameRate = 60;
};

// Compression header for .zesave cloud chunks
#pragma pack(push, 1)
struct ZeSaveChunkHeader {
    uint32_t magic = 0x53455A58; // 'XZES' (Zombie Engine Save)
    uint32_t version = 1;
    uint32_t uncompressedSize = 0;
    uint32_t compressedSize = 0;
    uint32_t crc32 = 0;
    uint32_t chunkId = 0;
};
#pragma pack(pop)

class SteamworksManager {
public:
    SteamworksManager();
    ~SteamworksManager();

    bool initialize(bool forceSteamDeckEmulation = false);
    void shutdown();

    void update();

    // Action Set Management
    void setActiveActionSet(SteamActionSet set);
    SteamActionSet getActiveActionSet() const { return m_activeActionSet; }

    // Digital & Analog State Query
    bool isActionActive(DigitalAction action) const;
    void setActionState(DigitalAction action, bool pressed);

    void getAnalogAction(AnalogAction action, float& outX, float& outY) const;
    void setAnalogAction(AnalogAction action, float x, float y);

    // Steam Deck Profile & Display
    const SteamDeckProfile& getDeckProfile() const { return m_deckProfile; }
    bool isRunningOnSteamDeck() const { return m_deckProfile.isDeck; }

    // Cloud Persistence Serialization (.zesave)
    static bool serializeSaveChunk(uint32_t chunkId, const std::vector<uint8_t>& rawData, std::vector<uint8_t>& outChunk);
    static bool deserializeSaveChunk(const std::vector<uint8_t>& chunk, uint32_t& outChunkId, std::vector<uint8_t>& outRawData);

    static uint32_t computeCRC32(const uint8_t* data, size_t size);

private:
    bool m_initialized = false;
    SteamActionSet m_activeActionSet = SteamActionSet::InGame;
    SteamDeckProfile m_deckProfile;

    bool m_digitalStates[static_cast<size_t>(DigitalAction::Count)] = {};
    float m_analogStates[static_cast<size_t>(AnalogAction::Count)][2] = {};
};

} // namespace ze::core
