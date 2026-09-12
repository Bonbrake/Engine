#include "ze/core/BodycamCamera.h"
#include <cmath>
#include <algorithm>

namespace ze::core {

BodycamCamera::BodycamCamera() = default;

void BodycamCamera::update(float dt, float mouseDeltaX, float mouseDeltaY, float playerVelocity, float directorIntensity) {
    if (dt <= 0.0f) return;

    updateDeadzoneAiming(mouseDeltaX, mouseDeltaY);
    updateSpringDamperInertia(dt);
    updateStressTremor(dt, directorIntensity);
    updateFootstepBob(dt, playerVelocity);

    // Pure human eyesight (zero video artifacts)
    opticalConfig_.sensorNoiseIso = 0.0f;
    opticalConfig_.chromaticAberration = 0.0f;
}

void BodycamCamera::onFootstepImpact(float groundVelocity) {
    footstepImpactVelocity_ = std::min(1.0f, groundVelocity / 6.0f);
    // Footfall jolts camera pitch down and rolls slightly opposite to foot
    cameraPitchVelocity_ -= (config.footstepBobAmplitude * 40.0f * footstepImpactVelocity_);
    cameraRollVelocity_ += (config.footstepRollAmplitude * 8.0f * footstepImpactVelocity_);
}

void BodycamCamera::updateDeadzoneAiming(float mouseDeltaX, float mouseDeltaY) {
    float deltaYaw = mouseDeltaX * config.mouseSensitivity;
    float deltaPitch = -mouseDeltaY * config.mouseSensitivity;

    // Soft Proportional Free-Aim: 75% drives weapon within deadzone, 25% smooth camera rotation
    // Eliminates perceived input lag while maintaining tactical loose-barrel gunplay
    float weaponFactor = 0.75f;
    float cameraFactor = 0.25f;

    weaponYawOffset_ += deltaYaw * weaponFactor;
    weaponPitchOffset_ += deltaPitch * weaponFactor;
    targetCameraYaw_ += deltaYaw * cameraFactor;
    targetCameraPitch_ += deltaPitch * cameraFactor;

    // Overflow check for hard boundaries
    if (std::fabs(weaponYawOffset_) > config.deadzoneRadiusX) {
        float clamped = std::clamp(weaponYawOffset_, -config.deadzoneRadiusX, config.deadzoneRadiusX);
        float overflow = weaponYawOffset_ - clamped;
        targetCameraYaw_ += overflow;
        weaponYawOffset_ = clamped;
    }

    if (std::fabs(weaponPitchOffset_) > config.deadzoneRadiusY) {
        float clamped = std::clamp(weaponPitchOffset_, -config.deadzoneRadiusY, config.deadzoneRadiusY);
        float overflow = weaponPitchOffset_ - clamped;
        targetCameraPitch_ += overflow;
        weaponPitchOffset_ = clamped;
    }

    // Clamp absolute pitch to prevent camera flipping
    targetCameraPitch_ = std::clamp(targetCameraPitch_, -85.0f, 85.0f);
}

void BodycamCamera::updateSpringDamperInertia(float dt) {
    float omega = config.springFrequency;
    float zeta = config.dampingRatio;

    // 1. Yaw Spring-Damper
    float yawError = targetCameraYaw_ - currentCameraYaw_;
    float yawAccel = (omega * omega * yawError) - (2.0f * zeta * omega * cameraYawVelocity_);
    cameraYawVelocity_ += yawAccel * dt;
    currentCameraYaw_ += cameraYawVelocity_ * dt;

    // 2. Pitch Spring-Damper
    float pitchError = targetCameraPitch_ - currentCameraPitch_;
    float pitchAccel = (omega * omega * pitchError) - (2.0f * zeta * omega * cameraPitchVelocity_);
    cameraPitchVelocity_ += pitchAccel * dt;
    currentCameraPitch_ += cameraPitchVelocity_ * dt;

    // 3. Roll Spring-Damper (returns to 0 baseline after footstep jolts)
    float rollError = -currentCameraRoll_;
    float rollAccel = (omega * omega * rollError) - (2.0f * zeta * omega * cameraRollVelocity_);
    cameraRollVelocity_ += rollAccel * dt;
    currentCameraRoll_ += cameraRollVelocity_ * dt;
}

void BodycamCamera::updateStressTremor(float dt, float directorIntensity) {
    tremorTimer_ += dt;

    // Dual-frequency tremor representing micro-muscular stress under fire
    float f1 = std::sin(tremorTimer_ * 19.5f);
    float f2 = std::cos(tremorTimer_ * 31.2f);
    float compositeTremor = (0.65f * f1) + (0.35f * f2);

    weaponTremor_ = directorIntensity * config.maxStressTremor * compositeTremor;
}

void BodycamCamera::updateFootstepBob(float dt, float playerVelocity) {
    if (playerVelocity > 0.1f) {
        // Step cadence scales with locomotion speed
        float stepRate = 2.0f * std::sqrt(playerVelocity);
        footstepPhase_ += dt * stepRate * 3.14159f;

        float verticalBob = std::sin(2.0f * footstepPhase_) * config.footstepBobAmplitude * (playerVelocity / 5.0f);
        float lateralRoll = std::cos(footstepPhase_) * config.footstepRollAmplitude * (playerVelocity / 5.0f);

        currentCameraPitch_ += verticalBob;
        currentCameraRoll_ += lateralRoll;
    }

    // Decay footstep impact impulse
    footstepImpactVelocity_ = std::max(0.0f, footstepImpactVelocity_ - dt * 4.0f);
}

} // namespace ze::core
