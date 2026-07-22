#include "ze/vehicle/WheelSim.h"
#include <algorithm>
#include <cmath>

namespace vehicle {

void WheelSim::update(float dt, const glm::vec3& chassisVel, float steeringAngle,
                      float throttle, float brakeForce) {
    // Suspension model: spring + damper
    float compression = state_.suspensionTravel - 0.5f;
    state_.springForce = -stiffness_ * compression;
    state_.dampForce = -damping_ * (chassisVel.y * 0.1f);

    // Ground contact check
    state_.onGround = state_.suspensionTravel > 0.1f && state_.suspensionTravel < 0.9f;

    if (state_.onGround) {
        // Wheel speed from chassis velocity + throttle
        float forwardSpeed = chassisVel.length() * 0.1f;
        state_.wheelSpeed += (throttle * 100.0f - state_.wheelSpeed) * dt * 5.0f;
        state_.wheelSpeed = std::max(0.0f, state_.wheelSpeed);

        // Slip angle
        float lateralVel = chassisVel.x * std::sin(steeringAngle) + chassisVel.z * std::cos(steeringAngle);
        state_.slipAngle = std::atan2(lateralVel, forwardSpeed + 0.01f);

        // Traction force (simplified friction circle)
        state_.tractionForce = 5000.0f * (1.0f - std::abs(state_.slipAngle) / 0.5f);
        if (state_.tractionForce < 0.0f) state_.tractionForce = 0.0f;
    } else {
        state_.wheelSpeed *= 0.95f; // wheel free-spinning decay
        state_.tractionForce = 0.0f;
    }

    // Brake
    if (brakeForce > 0.0f) {
        state_.wheelSpeed -= brakeForce * dt * 2.0f;
        if (state_.wheelSpeed < 0.0f) state_.wheelSpeed = 0.0f;
    }
}

} // namespace vehicle
