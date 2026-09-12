#define _CRT_SECURE_NO_WARNINGS
#include "ze/core/SteamworksManager.h"
#include <cstring>
#include <cstdlib>

namespace ze::core {

uint32_t SteamworksManager::computeCRC32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320u & (-(int32_t)(crc & 1)));
        }
    }
    return ~crc;
}

SteamworksManager::SteamworksManager() {
    std::memset(m_digitalStates, 0, sizeof(m_digitalStates));
    std::memset(m_analogStates, 0, sizeof(m_analogStates));
}

SteamworksManager::~SteamworksManager() {
    shutdown();
}

bool SteamworksManager::initialize(bool forceSteamDeckEmulation) {
    m_initialized = true;

    // Check SteamOS / Gamescope / SteamCube environment variables or hardware emulation
    const char* deckEnv = std::getenv("SteamDeck");
    const char* gamescopeEnv = std::getenv("GAMESCOPE_WAYLAND_DISPLAY");
    const char* steamCubeEnv = std::getenv("SteamCube");
    const char* steamMachineEnv = std::getenv("SteamMachine");
    const char* steamOSEnv = std::getenv("SteamOS");

    if (forceSteamDeckEmulation || (deckEnv && std::strcmp(deckEnv, "1") == 0)) {
        // Handheld emulation mode (for sanity harness / legacy test compatibility)
        m_deckProfile.isDeck = true;
        m_deckProfile.isSteamOSConsole = false;
        m_deckProfile.isGamescope = (gamescopeEnv != nullptr);
        m_deckProfile.isLivingRoomTV = false;
        m_deckProfile.hdr10Enabled = false;
        m_deckProfile.displayWidth = 1280;
        m_deckProfile.displayHeight = 800;
        m_deckProfile.aspectRatio = 16.0f / 10.0f;
        m_deckProfile.uiScaleMultiplier = 1.25f;
        m_deckProfile.gyroAdsEnabled = true;
        m_deckProfile.targetFrameRate = 60;
    } else if ((steamCubeEnv && std::strcmp(steamCubeEnv, "1") == 0) ||
               (steamMachineEnv && std::strcmp(steamMachineEnv, "1") == 0) ||
               (steamOSEnv && std::strcmp(steamOSEnv, "1") == 0) ||
               (gamescopeEnv != nullptr)) {
        // SteamOS Desktop / Living Room Console ("Steam Cube" / Steam Machine)
        m_deckProfile.isDeck = false;
        m_deckProfile.isSteamOSConsole = true;
        m_deckProfile.isGamescope = true;
        m_deckProfile.isLivingRoomTV = true;
        m_deckProfile.hdr10Enabled = true;
        m_deckProfile.displayWidth = 2560;
        m_deckProfile.displayHeight = 1440;
        m_deckProfile.aspectRatio = 16.0f / 9.0f;
        m_deckProfile.uiScaleMultiplier = 1.75f;
        m_deckProfile.gyroAdsEnabled = true;
        m_deckProfile.targetFrameRate = 60;
    } else {
        // Standard Desktop PC
        m_deckProfile.isDeck = false;
        m_deckProfile.isSteamOSConsole = false;
        m_deckProfile.isGamescope = false;
        m_deckProfile.isLivingRoomTV = false;
        m_deckProfile.hdr10Enabled = false;
        m_deckProfile.displayWidth = 1920;
        m_deckProfile.displayHeight = 1080;
        m_deckProfile.aspectRatio = 16.0f / 9.0f;
        m_deckProfile.uiScaleMultiplier = 1.0f;
        m_deckProfile.gyroAdsEnabled = false;
        m_deckProfile.targetFrameRate = 120;
    }

    m_activeActionSet = SteamActionSet::InGame;
    return true;
}

void SteamworksManager::shutdown() {
    m_initialized = false;
}

void SteamworksManager::update() {
    // In live Steamworks client mode, SteamInput()->RunFrame() is called here.
}

void SteamworksManager::setActiveActionSet(SteamActionSet set) {
    m_activeActionSet = set;
}

bool SteamworksManager::isActionActive(DigitalAction action) const {
    size_t idx = static_cast<size_t>(action);
    if (idx < static_cast<size_t>(DigitalAction::Count)) {
        return m_digitalStates[idx];
    }
    return false;
}

void SteamworksManager::setActionState(DigitalAction action, bool pressed) {
    size_t idx = static_cast<size_t>(action);
    if (idx < static_cast<size_t>(DigitalAction::Count)) {
        m_digitalStates[idx] = pressed;
    }
}

void SteamworksManager::getAnalogAction(AnalogAction action, float& outX, float& outY) const {
    size_t idx = static_cast<size_t>(action);
    if (idx < static_cast<size_t>(AnalogAction::Count)) {
        outX = m_analogStates[idx][0];
        outY = m_analogStates[idx][1];
    } else {
        outX = 0.0f;
        outY = 0.0f;
    }
}

void SteamworksManager::setAnalogAction(AnalogAction action, float x, float y) {
    size_t idx = static_cast<size_t>(action);
    if (idx < static_cast<size_t>(AnalogAction::Count)) {
        m_analogStates[idx][0] = x;
        m_analogStates[idx][1] = y;
    }
}

bool SteamworksManager::serializeSaveChunk(uint32_t chunkId, const std::vector<uint8_t>& rawData, std::vector<uint8_t>& outChunk) {
    if (rawData.empty()) {
        return false;
    }

    ZeSaveChunkHeader header;
    header.magic = 0x53455A58; // 'XZES'
    header.version = 1;
    header.chunkId = chunkId;
    header.uncompressedSize = static_cast<uint32_t>(rawData.size());

    // Simple, deterministic RLE/byte-pack compression for safehouse and chunk delta snapshots
    std::vector<uint8_t> compressedData;
    compressedData.reserve(rawData.size());

    size_t i = 0;
    while (i < rawData.size()) {
        uint8_t byte = rawData[i];
        uint8_t runLen = 1;
        while (i + 1 < rawData.size() && rawData[i + 1] == byte && runLen < 255) {
            ++runLen;
            ++i;
        }
        if (runLen > 3) {
            compressedData.push_back(0xFF); // Escape byte for run
            compressedData.push_back(runLen);
            compressedData.push_back(byte);
        } else {
            for (uint8_t r = 0; r < runLen; ++r) {
                if (byte == 0xFF) {
                    compressedData.push_back(0xFF);
                    compressedData.push_back(0x00); // Escaped literal 0xFF
                } else {
                    compressedData.push_back(byte);
                }
            }
        }
        ++i;
    }

    header.compressedSize = static_cast<uint32_t>(compressedData.size());
    header.crc32 = computeCRC32(rawData.data(), rawData.size());

    outChunk.resize(sizeof(ZeSaveChunkHeader) + compressedData.size());
    std::memcpy(outChunk.data(), &header, sizeof(ZeSaveChunkHeader));
    std::memcpy(outChunk.data() + sizeof(ZeSaveChunkHeader), compressedData.data(), compressedData.size());

    return true;
}

bool SteamworksManager::deserializeSaveChunk(const std::vector<uint8_t>& chunk, uint32_t& outChunkId, std::vector<uint8_t>& outRawData) {
    if (chunk.size() < sizeof(ZeSaveChunkHeader)) {
        return false;
    }

    ZeSaveChunkHeader header;
    std::memcpy(&header, chunk.data(), sizeof(ZeSaveChunkHeader));

    if (header.magic != 0x53455A58 || header.version != 1) {
        return false;
    }

    if (chunk.size() != sizeof(ZeSaveChunkHeader) + header.compressedSize) {
        return false;
    }

    const uint8_t* compPtr = chunk.data() + sizeof(ZeSaveChunkHeader);
    outRawData.clear();
    outRawData.reserve(header.uncompressedSize);

    size_t cIdx = 0;
    while (cIdx < header.compressedSize) {
        uint8_t b = compPtr[cIdx++];
        if (b == 0xFF) {
            if (cIdx >= header.compressedSize) return false;
            uint8_t code = compPtr[cIdx++];
            if (code == 0x00) {
                outRawData.push_back(0xFF);
            } else {
                if (cIdx >= header.compressedSize) return false;
                uint8_t runByte = compPtr[cIdx++];
                for (uint8_t r = 0; r < code; ++r) {
                    outRawData.push_back(runByte);
                }
            }
        } else {
            outRawData.push_back(b);
        }
    }

    if (outRawData.size() != header.uncompressedSize) {
        return false;
    }

    uint32_t crc = computeCRC32(outRawData.data(), outRawData.size());
    if (crc != header.crc32) {
        return false;
    }

    outChunkId = header.chunkId;
    return true;
}

} // namespace ze::core
