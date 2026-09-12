#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

namespace ze::core {

enum class InputDevice : uint8_t {
    Gamepad = 0,       // Primary authoritative device
    KeyboardMouse = 1  // Secondary desktop device
};

enum class ControllerHardwareType : uint8_t {
    Xbox = 0,
    PlayStationDualSense = 1,
    SteamDeck = 2,
    NintendoSwitchPro = 3
};

struct ControllerGlyphs {
    std::string buttonBottom; // Xbox A / DualSense Cross / Switch B
    std::string buttonRight;  // Xbox B / DualSense Circle / Switch A
    std::string buttonLeft;   // Xbox X / DualSense Square / Switch Y
    std::string buttonTop;    // Xbox Y / DualSense Triangle / Switch X
    std::string triggerLeft;  // LT / L2 / ZL
    std::string triggerRight; // RT / R2 / ZR
};

struct GyroAimConfig {
    bool enabled{ true };
    float sensitivity{ 1.8f };           // Angular degrees per rad/s gyroscope velocity
    float gyroDeadzoneDegPerSec{ 1.2f }; // Rejection threshold for sensor noise
    bool flickStickEnabled{ false };      // Instant 180° flick stick option
};

struct GamepadStickConfig {
    float radialDeadzone{ 0.12f };          // Inner deadzone threshold
    float freeAimThreshold{ 0.65f };        // Transition point between free-aim and torso turn
    float fineAimGamma{ 2.4f };             // Exponential response curve for micro-precision free-aim
    float fineAimSensitivity{ 22.0f };      // Degrees/second for free-aim within deadzone
    float torsoTurnBaseSensitivity{ 90.0f };// Degrees/second initial torso turn speed
    float torsoTurnMaxSensitivity{ 260.0f };// Degrees/second maximum torso turn speed
    float turnAccelerationRampTime{ 0.15f };// Seconds to ramp from base to max turn speed
    float aimFrictionMultiplier{ 0.42f };   // Slowdown multiplier when crosshair sweeps over target
    float aimFrictionConeAngleDeg{ 3.5f };  // Angular cone for target friction detection
};

struct AimFrictionTarget {
    glm::vec3 worldPosition{ 0.0f };
    float boundingRadius{ 0.5f };
};

struct HapticTriggerProfile {
    float takeUpResistance{ 0.25f };   // Soft spring take-up (0.0 to 1.0)
    float breakWallPosition{ 0.75f };  // Trigger break position threshold (0.0 to 1.0)
    float firePulseAmplitude{ 0.85f }; // Firing impulse shockwave (0.0 to 1.0)
    bool emptyMagazineSlack{ false };  // Loose, zero-resistance dead trigger on empty magazine
};

class GamepadController {
public:
    GamepadController();
    ~GamepadController() = default;

    void setHardwareType(ControllerHardwareType type);
    ControllerHardwareType getHardwareType() const { return hardwareType_; }
    ControllerGlyphs getActiveGlyphs() const;

    // Steam Input / SDL3 Gyro motion injection (degrees/second)
    void injectGyroMotion(float pitchRateDeg, float yawRateDeg, float rollRateDeg);

    void update(float dt, const SDL_Event* events, size_t eventCount, 
                const glm::vec3& eyePos, const glm::vec3& aimDir,
                const std::vector<AimFrictionTarget>& nearbyTargets);

    // Compute angular deltas for BodycamCamera (yaw and pitch in degrees)
    void getCameraDeltas(float& outYawDelta, float& outPitchDelta) const {
        outYawDelta = frameYawDelta_;
        outPitchDelta = framePitchDelta_;
    }

    // Contextual button actions
    bool isCombatReloadTriggered() const { return combatReloadTriggered_; }
    bool isMagCheckTriggered() const { return magCheckTriggered_; }
    bool isPhysicsGrabTriggered() const { return physicsGrabTriggered_; }
    bool isInteractTriggered() const { return interactTriggered_; }
    bool isFireTriggered() const { return fireTriggered_; }
    bool isPointAimHeld() const { return pointAimHeld_; }

    InputDevice getActiveDevice() const { return activeDevice_; }
    bool isAimFrictionActive() const { return aimFrictionActive_; }

    void dispatchTriggerHaptics(SDL_Gamepad* gamepad, const HapticTriggerProfile& profile);

    GamepadStickConfig config;
    GyroAimConfig gyroConfig;

private:
    void processStickAim(float rawStickX, float rawStickY, float dt,
                         const glm::vec3& eyePos, const glm::vec3& aimDir,
                         const std::vector<AimFrictionTarget>& targets);

    InputDevice activeDevice_{ InputDevice::Gamepad };
    ControllerHardwareType hardwareType_{ ControllerHardwareType::Xbox };

    float rawRightStickX_{ 0.0f };
    float rawRightStickY_{ 0.0f };
    float torsoTurnRampTimer_{ 0.0f };

    float frameYawDelta_{ 0.0f };
    float framePitchDelta_{ 0.0f };
    float gyroYawDelta_{ 0.0f };
    float gyroPitchDelta_{ 0.0f };

    bool reloadButtonDown_{ false };
    float reloadHoldTimer_{ 0.0f };
    bool combatReloadTriggered_{ false };
    bool magCheckTriggered_{ false };

    bool interactButtonDown_{ false };
    float interactHoldTimer_{ 0.0f };
    bool interactTriggered_{ false };
    bool physicsGrabTriggered_{ false };

    bool fireTriggered_{ false };
    bool pointAimHeld_{ false };
    bool aimFrictionActive_{ false };

    static constexpr float MAG_CHECK_HOLD_DURATION = 0.30f;
    static constexpr float GRAB_HOLD_DURATION = 0.35f;
};

} // namespace ze::core
