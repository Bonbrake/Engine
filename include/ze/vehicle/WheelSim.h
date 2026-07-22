#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace vehicle {

// Wheel simulation: suspension, traction, slip (M8.7)
struct WheelState {
    glm::vec3 position;       // local position on chassis
    float suspensionTravel = 0.0f;  // 0..1
    float springForce = 0.0f;
    float dampForce = 0.0f;
    float slipAngle = 0.0f;
    float tractionForce = 0.0f;
    float wheelSpeed = 0.0f;  // rad/s
    bool onGround = false;
};

class WheelSim {
public:
    WheelSim() = default;

    void update(float dt, const glm::vec3& chassisVel, float steeringAngle,
                float throttle, float brakeForce);

    const WheelState& state() const { return state_; }
    float loadForce() const { return state_.springForce + state_.dampForce; }

    void setSuspension(float stiffness, float damping, float maxTravel) {
        stiffness_ = stiffness;
        damping_ = damping;
        maxTravel_ = maxTravel;
    }

private:
    WheelState state_;
    float stiffness_ = 30000.0f;   // spring constant
    float damping_ = 4000.0f;      // damping coefficient
    float maxTravel_ = 0.3f;       // meters
    float wheelRadius_ = 0.35f;    // meters
};

} // namespace vehicle
