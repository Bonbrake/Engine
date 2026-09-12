#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace ze::core {

// Optical parameters: 0.0f by default for pure human eyesight (zero video artifacts)
struct BodycamOpticalConfig {
    float barrelDistortionK1{ 0.0f };      // Pure human eye: 0.0 distortion
    float barrelDistortionK2{ 0.0f };      // Pure human eye: 0.0 secondary distortion
    float chromaticAberration{ 0.0f };     // Pure human eye: zero RGB lens split
    float vignetteFalloff{ 0.0f };         // Pure human eye: zero lens shading
    float sensorNoiseIso{ 0.0f };          // Pure human eye: zero CMOS digital noise
    float autoExposureSpeed{ 4.0f };       // Dynamic biological pupil adaptation rate
};

// True First-Person Tactical Movement, Decoupled Free-Aim, and Spring-Damper Inertial Camera
class BodycamCamera {
public:
    BodycamCamera();
    ~BodycamCamera() = default;

    // Tick update (60Hz / variable dt)
    // mouseDeltaX/Y: raw mouse inputs
    // playerVelocity: magnitude of horizontal movement in m/s
    // directorIntensity: 0.0 (Relax) to 1.0 (SustainPeak) fed from L4D2Director
    void update(float dt, float mouseDeltaX, float mouseDeltaY, float playerVelocity, float directorIntensity);

    // Trigger footstep impact impulse (called on each physical footfall)
    void onFootstepImpact(float groundVelocity);

    // Queries
    float getCameraYaw() const { return currentCameraYaw_; }
    float getCameraPitch() const { return currentCameraPitch_; }
    float getCameraRoll() const { return currentCameraRoll_; }

    float getWeaponYawOffset() const { return weaponYawOffset_; }
    float getWeaponPitchOffset() const { return weaponPitchOffset_; }
    float getWeaponTremor() const { return weaponTremor_; }

    const BodycamOpticalConfig& getOpticalConfig() const { return opticalConfig_; }

    // Deadzone and physical tuning parameters
    struct Config {
        float deadzoneRadiusX{ 12.0f };    // Max horizontal free-aim degrees before camera turns
        float deadzoneRadiusY{ 8.0f };     // Max vertical free-aim degrees before camera turns
        float mouseSensitivity{ 0.08f };   // Degrees per raw mouse count
        float springFrequency{ 12.0f };    // Natural oscillation frequency (rad/s)
        float dampingRatio{ 0.85f };       // Critical damping factor (0.85 = slightly underdamped for visceral lag)
        float footstepBobAmplitude{ 0.035f };// Vertical displacement on footfalls
        float footstepRollAmplitude{ 0.80f };// Degrees roll tilt on footfalls
        float maxStressTremor{ 0.45f };    // Angular degrees tremor at maximum intensity (SustainPeak)
        float eyeHeight{ 1.68f };          // Natural human eye-level height in meters
    } config;

private:
    void updateDeadzoneAiming(float mouseDeltaX, float mouseDeltaY);
    void updateSpringDamperInertia(float dt);
    void updateStressTremor(float dt, float directorIntensity);
    void updateFootstepBob(float dt, float playerVelocity);

    float targetCameraYaw_{ 0.0f };
    float targetCameraPitch_{ 0.0f };
    float currentCameraYaw_{ 0.0f };
    float currentCameraPitch_{ 0.0f };
    float currentCameraRoll_{ 0.0f };

    float cameraYawVelocity_{ 0.0f };
    float cameraPitchVelocity_{ 0.0f };
    float cameraRollVelocity_{ 0.0f };

    float weaponYawOffset_{ 0.0f };
    float weaponPitchOffset_{ 0.0f };
    float weaponTremor_{ 0.0f };

    float footstepPhase_{ 0.0f };
    float footstepImpactVelocity_{ 0.0f };
    float tremorTimer_{ 0.0f };

    BodycamOpticalConfig opticalConfig_;
};

// Authoritative alias for True First-Person Tactical Camera
using TacticalFirstPersonCamera = BodycamCamera;

} // namespace ze::core
