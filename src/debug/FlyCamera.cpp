#include "debug/FlyCamera.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace debug {

glm::dvec3 FlyCamera::computeForward() const {
    glm::dvec3 f;
    f.x = std::cos(pitch_) * std::sin(yaw_);
    f.y = std::sin(pitch_);
    f.z = -std::cos(pitch_) * std::cos(yaw_);
    return f;
}

void FlyCamera::update(float dt) {
    const auto& s = core::Input::getState();

    // --- Look (mouse) ---
    // NOTE: yaw/pitch sign is an OPEN ITEM — must be confirmed by looking at
    // the actual rendered view on local GPU (mouse-right should turn right,
    // W should move where you look). A one-line sign flip fixes inversion.
    yaw_   -= s.mouseDX * kMouseSens;
    pitch_ -= s.mouseDY * kMouseSens;
    if (pitch_ >  kPitchLimit) pitch_ =  kPitchLimit;
    if (pitch_ < -kPitchLimit) pitch_ = -kPitchLimit;

    // --- Basis from yaw/pitch ---
    glm::dvec3 forward = computeForward();
    glm::dvec3 worldUp{0.0, 1.0, 0.0};
    glm::dvec3 right = glm::normalize(glm::cross(forward, worldUp));

    // --- Move (keyboard) ---
    const double speed = kMoveSpeed * (s.keyboardState[SDL_SCANCODE_LSHIFT] ? kSprintMult : 1.0);
    const double step = speed * (double)dt;
    if (s.keyboardState[SDL_SCANCODE_W])     position_ += forward * step;
    if (s.keyboardState[SDL_SCANCODE_S])     position_ -= forward * step;
    if (s.keyboardState[SDL_SCANCODE_D])     position_ += right   * step;
    if (s.keyboardState[SDL_SCANCODE_A])     position_ -= right   * step;
    if (s.keyboardState[SDL_SCANCODE_SPACE]) position_ += worldUp * step;
    if (s.keyboardState[SDL_SCANCODE_LCTRL]) position_ -= worldUp * step;
}

glm::mat4 FlyCamera::getViewMatrix() const {
    // Single-precision cast ONLY at the final step (precision rule).
    glm::dvec3 forward = computeForward();
    glm::dvec3 center  = position_ + forward;   // dvec3 + dvec3
    return glm::lookAt(
        glm::vec3(position_),                   // dvec3 -> vec3 here, once
        glm::vec3(center),
        glm::vec3(0.0, 1.0, 0.0));
}

} // namespace debug
