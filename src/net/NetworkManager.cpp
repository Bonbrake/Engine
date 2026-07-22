#include "ze/net/NetworkManager.h"
#include <algorithm>
#include <cstring>

namespace net {

NetworkManager::~NetworkManager() { shutdown(); }

bool NetworkManager::initialize(uint16_t port, bool isServer, uint32_t maxClients) {
    port_ = port;
    isServer_ = isServer;
    state_ = ConnectionState::Disconnected;
    return true;
}

void NetworkManager::shutdown() {
    disconnect();
}

bool NetworkManager::connect(const std::string& host, uint16_t port) {
    state_ = ConnectionState::Connecting;
    state_ = ConnectionState::Connected;
    if (onConnect_) onConnect_(localId_);
    return true;
}

void NetworkManager::disconnect() {
    if (state_ == ConnectionState::Connected && onDisconnect_) {
        onDisconnect_(localId_);
    }
    state_ = ConnectionState::Disconnected;
}

void NetworkManager::tick(float dt) {
    // Placeholder: process incoming packets, timeouts, keepalive
}

bool NetworkManager::sendPacket(uint32_t clientId, const uint8_t* data, size_t size) {
    // Placeholder: serialize + transmit via UDP
    return true;
}

bool NetworkManager::pollPacket(uint32_t& clientId, std::vector<uint8_t>& data) {
    // Placeholder: dequeue next packet from receive buffer
    return false;
}

bool NetworkManager::validateReplay(const std::vector<uint8_t>& inputStream,
                                     const std::vector<uint32_t>& expectedHashes) {
    // Anti-cheat: compare input hash stream against recorded (M2.8-EXT-09)
    if (inputStream.empty() || expectedHashes.empty()) return false;
    for (size_t i = 0; i < expectedHashes.size(); ++i) {
        uint32_t hash = 2166136261u;
        for (size_t j = i * 64; j < (i + 1) * 64 && j < inputStream.size(); ++j) {
            hash ^= inputStream[j];
            hash *= 16777619u;
        }
        if (hash != expectedHashes[i]) return false;
    }
    return true;
}

} // namespace net
