#include "ze/vehicle/VehicleSystem.h"
#include "ze/vehicle/WheelSim.h"
#include <algorithm>
#include <cmath>

namespace vehicle {

void VehicleSystem::update(float dt, float throttle, float brake, float steering) {
    if (!state_.engineRunning) {
        state_.velocity *= 0.99f; // rolling resistance
        return;
    }

    updateEngine(dt, throttle);
    updateTransmission(dt);

    // Simple drag + rolling resistance
    float drag = state_.velocity.length() * 0.5f;
    state_.velocity *= (1.0f - drag * dt);

    // Steering (simplified)
    float turnRate = steering * 1.5f;
    state_.yaw += turnRate * dt;

    // Forward thrust
    float forwardForce = throttle * state_.engineRPM * 0.01f;
    glm::vec3 forwardDir = glm::vec3(std::sin(state_.yaw), 0, std::cos(state_.yaw));
    state_.velocity += forwardDir * forwardForce * dt;

    // Fuel consumption
    state_.fuel -= throttle * 0.1f * dt;
    if (state_.fuel <= 0.0f) {
        state_.fuel = 0.0f;
        stopEngine();
    }
}

void VehicleSystem::updateEngine(float dt, float throttle) {
    float targetRPM = 800.0f + throttle * 6000.0f;
    state_.engineRPM += (targetRPM - state_.engineRPM) * dt * 3.0f;
    state_.engineRPM = std::max(0.0f, std::min(8000.0f, state_.engineRPM));
}

void VehicleSystem::updateTransmission(float dt) {
    // Simple auto gearbox
    if (state_.engineRPM > 5500.0f && state_.gear < 6) {
        state_.gear++;
    } else if (state_.engineRPM < 2000.0f && state_.gear > 0) {
        state_.gear--;
    }
    state_.speed = state_.velocity.length();
}

} // namespace vehicle
