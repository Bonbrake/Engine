#pragma once

#include "ze/core/Input.h"
#include <glm/glm.hpp>

namespace debug {

// Free-fly / noclip debug camera. Double-precision position; feeds the
// renderer a single-precision view matrix derived at the last step only
// (dvec3 -> vec3 cast happens exactly once, in getViewMatrix()).
//
// Dev-gated at the Engine call site (#if ENGINE_DEV_TOOLS), not here.
class FlyCamera {
public:
    FlyCamera() = default;

    // Advance position/orientation from this frame's Input state.
    void update(float dt);

    // Single-precision view matrix for the renderer.
    glm::mat4 getViewMatrix() const;

    const glm::dvec3& position() const { return position_; }
    void setPosition(const glm::dvec3& p) { position_ = p; }
    void setYawPitch(double yaw, double pitch) { yaw_ = yaw; pitch_ = pitch; }
    glm::dvec3 getForward() const { return computeForward(); }  // [M2.6 Phase 2] for state-dump

private:
    // Shared forward-vector math, called from update() and getViewMatrix()
    // so the two copies can never drift (yaw-sign correctness depends on it).
    glm::dvec3 computeForward() const;

    glm::dvec3 position_{0.0, 0.0, 0.0};
    double yaw_   = 0.0;   // radians, around +Y
    double pitch_ = 0.0;   // radians, clamped to (-kPitchLimit, kPitchLimit)

    static constexpr double kMoveSpeed   = 12.0;    // units/sec (normal WASD)
    static constexpr double kSprintMult  = 3.0;     // Shift sprint multiplier (fast far-cube traversal)
    static constexpr double kMouseSens   = 0.0006;  // rad per pixel (tuned down ~3.7x)
    static constexpr double kPitchLimit  = 1.5533;  // ~89 deg, avoids gimbal flip
};

} // namespace debug
