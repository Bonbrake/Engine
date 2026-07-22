#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace net {

// Reliable UDP client/server (M10)
enum class ConnectionState : uint8_t {
    Disconnected, Connecting, Connected, Timeout
};

struct NetPacket {
    uint32_t sequence;
    uint32_t ack;
    std::vector<uint8_t> data;
    uint64_t timestamp;
};

class NetworkManager {
public:
    NetworkManager() = default;
    ~NetworkManager();

    bool initialize(uint16_t port, bool isServer, uint32_t maxClients = 32);
    void shutdown();

    bool connect(const std::string& host, uint16_t port);
    void disconnect();
    void tick(float dt);

    // Send/receive
    bool sendPacket(uint32_t clientId, const uint8_t* data, size_t size);
    bool pollPacket(uint32_t& clientId, std::vector<uint8_t>& data);

    // State
    ConnectionState state() const { return state_; }
    uint32_t localClientId() const { return localId_; }
    float roundTripTime() const { return rtt_; }
    uint32_t packetLoss() const { return packetLoss_; }

    // Events
    using ConnectCallback = std::function<void(uint32_t clientId)>;
    using DisconnectCallback = std::function<void(uint32_t clientId)>;
    void onConnect(ConnectCallback cb) { onConnect_ = cb; }
    void onDisconnect(DisconnectCallback cb) { onDisconnect_ = cb; }

    // Anti-cheat (M2.8-EXT-09)
    bool validateReplay(const std::vector<uint8_t>& inputStream,
                        const std::vector<uint32_t>& expectedHashes);

private:
    bool isServer_ = false;
    uint16_t port_ = 0;
    ConnectionState state_ = ConnectionState::Disconnected;
    uint32_t localId_ = 0;
    float rtt_ = 0.0f;
    uint32_t packetLoss_ = 0;
    ConnectCallback onConnect_;
    DisconnectCallback onDisconnect_;
};

} // namespace net
