#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace vehicle {

// Vehicle system (M8.7)
struct VehicleState {
    glm::vec3 position;
    glm::vec3 velocity;
    float yaw = 0.0f;
    float speed = 0.0f;       // m/s
    float engineRPM = 0.0f;
    uint32_t gear = 0;
    float fuel = 100.0f;      // 0..100
    float health = 100.0f;    // 0..100
    bool engineRunning = false;
};

class VehicleSystem {
public:
    VehicleSystem() = default;

    void update(float dt, float throttle, float brake, float steering);

    VehicleState& state() { return state_; }
    const VehicleState& state() const { return state_; }

    void startEngine() { state_.engineRunning = true; }
    void stopEngine() { state_.engineRunning = false; }
    void refuel(float amount) { state_.fuel = std::min(100.0f, state_.fuel + amount); }
    void applyDamage(float dmg) { state_.health = std::max(0.0f, state_.health - dmg); }

private:
    VehicleState state_;
    void updateEngine(float dt, float throttle);
    void updateTransmission(float dt);
};

} // namespace vehicle
