#include "ze/core/GamepadController.h"
#include <SDL3/SDL_gamepad.h>
#include <cmath>
#include <algorithm>

namespace ze::core {

GamepadController::GamepadController() {
    activeDevice_ = InputDevice::Gamepad;
    hardwareType_ = ControllerHardwareType::Xbox;
}

void GamepadController::setHardwareType(ControllerHardwareType type) {
    hardwareType_ = type;
}

ControllerGlyphs GamepadController::getActiveGlyphs() const {
    ControllerGlyphs g;
    switch (hardwareType_) {
    case ControllerHardwareType::PlayStationDualSense:
        g.buttonBottom = "Cross (X)";
        g.buttonRight = "Circle (O)";
        g.buttonLeft = "Square ([])";
        g.buttonTop = "Triangle (/\\)";
        g.triggerLeft = "L2";
        g.triggerRight = "R2";
        break;
    case ControllerHardwareType::SteamDeck:
        g.buttonBottom = "A";
        g.buttonRight = "B";
        g.buttonLeft = "X";
        g.buttonTop = "Y";
        g.triggerLeft = "L2";
        g.triggerRight = "R2";
        break;
    case ControllerHardwareType::NintendoSwitchPro:
        g.buttonBottom = "B";
        g.buttonRight = "A";
        g.buttonLeft = "Y";
        g.buttonTop = "X";
        g.triggerLeft = "ZL";
        g.triggerRight = "ZR";
        break;
    case ControllerHardwareType::Xbox:
    default:
        g.buttonBottom = "A";
        g.buttonRight = "B";
        g.buttonLeft = "X";
        g.buttonTop = "Y";
        g.triggerLeft = "LT";
        g.triggerRight = "RT";
        break;
    }
    return g;
}

void GamepadController::injectGyroMotion(float pitchRateDeg, float yawRateDeg, float rollRateDeg) {
    if (!gyroConfig.enabled) return;
    (void)rollRateDeg;

    if (std::abs(yawRateDeg) > gyroConfig.gyroDeadzoneDegPerSec) {
        gyroYawDelta_ += yawRateDeg * gyroConfig.sensitivity;
    }
    if (std::abs(pitchRateDeg) > gyroConfig.gyroDeadzoneDegPerSec) {
        gyroPitchDelta_ += pitchRateDeg * gyroConfig.sensitivity;
    }
}

void GamepadController::update(float dt, const SDL_Event* events, size_t eventCount,
                               const glm::vec3& eyePos, const glm::vec3& aimDir,
                               const std::vector<AimFrictionTarget>& nearbyTargets) {
    // Reset single-frame action pulses
    combatReloadTriggered_ = false;
    magCheckTriggered_ = false;
    interactTriggered_ = false;
    physicsGrabTriggered_ = false;
    fireTriggered_ = false;
    pointAimHeld_ = false;
    frameYawDelta_ = 0.0f;
    framePitchDelta_ = 0.0f;

    // Incorporate injected Gyro Motion
    frameYawDelta_ += gyroYawDelta_ * dt;
    framePitchDelta_ += gyroPitchDelta_ * dt;
    gyroYawDelta_ = 0.0f;
    gyroPitchDelta_ = 0.0f;

    // Process incoming SDL events
    for (size_t i = 0; i < eventCount; ++i) {
        const auto& ev = events[i];

        if (ev.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
            activeDevice_ = InputDevice::Gamepad;
            if (ev.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTX) {
                rawRightStickX_ = ev.gaxis.value / 32767.0f;
            } else if (ev.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTY) {
                rawRightStickY_ = ev.gaxis.value / 32767.0f;
            } else if (ev.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
                if (ev.gaxis.value > 16000) fireTriggered_ = true;
            } else if (ev.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
                if (ev.gaxis.value > 16000) pointAimHeld_ = true;
            }
        } else if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            activeDevice_ = InputDevice::Gamepad;
            if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_WEST) { // Reload
                reloadButtonDown_ = true;
                reloadHoldTimer_ = 0.0f;
            } else if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) { // Interact / Grab
                interactButtonDown_ = true;
                interactHoldTimer_ = 0.0f;
            } else if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
                fireTriggered_ = true;
            } else if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
                pointAimHeld_ = true;
            }
        } else if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_WEST) {
                if (reloadButtonDown_ && reloadHoldTimer_ < MAG_CHECK_HOLD_DURATION) {
                    combatReloadTriggered_ = true;
                }
                reloadButtonDown_ = false;
            } else if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                if (interactButtonDown_ && interactHoldTimer_ < GRAB_HOLD_DURATION) {
                    interactTriggered_ = true;
                }
                interactButtonDown_ = false;
            }
        } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
            if (std::abs(ev.motion.xrel) > 1 || std::abs(ev.motion.yrel) > 1) {
                activeDevice_ = InputDevice::KeyboardMouse;
                frameYawDelta_ = ev.motion.xrel * 0.08f;
                framePitchDelta_ = -ev.motion.yrel * 0.08f;
            }
        } else if (ev.type == SDL_EVENT_KEY_DOWN) {
            activeDevice_ = InputDevice::KeyboardMouse;
            if (ev.key.scancode == SDL_SCANCODE_R) {
                reloadButtonDown_ = true;
                reloadHoldTimer_ = 0.0f;
            } else if (ev.key.scancode == SDL_SCANCODE_E) {
                interactButtonDown_ = true;
                interactHoldTimer_ = 0.0f;
            }
        } else if (ev.type == SDL_EVENT_KEY_UP) {
            if (ev.key.scancode == SDL_SCANCODE_R) {
                if (reloadButtonDown_ && reloadHoldTimer_ < MAG_CHECK_HOLD_DURATION) {
                    combatReloadTriggered_ = true;
                }
                reloadButtonDown_ = false;
            } else if (ev.key.scancode == SDL_SCANCODE_E) {
                if (interactButtonDown_ && interactHoldTimer_ < GRAB_HOLD_DURATION) {
                    interactTriggered_ = true;
                }
                interactButtonDown_ = false;
            }
        }
    }

    // Process Reload Hold Timer (Physical Magazine Inspection)
    if (reloadButtonDown_) {
        reloadHoldTimer_ += dt;
        if (reloadHoldTimer_ >= MAG_CHECK_HOLD_DURATION && !magCheckTriggered_) {
            magCheckTriggered_ = true;
        }
    }

    // Process Interact Hold Timer (Skyrim/Fallout Physics Grab)
    if (interactButtonDown_) {
        interactHoldTimer_ += dt;
        if (interactHoldTimer_ >= GRAB_HOLD_DURATION && !physicsGrabTriggered_) {
            physicsGrabTriggered_ = true;
        }
    }

    // If active device is Gamepad, process thumbstick aim response curve
    if (activeDevice_ == InputDevice::Gamepad) {
        processStickAim(rawRightStickX_, rawRightStickY_, dt, eyePos, aimDir, nearbyTargets);
    }
}

void GamepadController::processStickAim(float rawStickX, float rawStickY, float dt,
                                        const glm::vec3& eyePos, const glm::vec3& aimDir,
                                        const std::vector<AimFrictionTarget>& targets) {
    float r = std::sqrt(rawStickX * rawStickX + rawStickY * rawStickY);
    if (r < config.radialDeadzone) {
        torsoTurnRampTimer_ = 0.0f;
        aimFrictionActive_ = false;
        return;
    }

    // Radial deadzone normalization
    float rHat = std::clamp((r - config.radialDeadzone) / (1.0f - config.radialDeadzone), 0.0f, 1.0f);
    float normX = rawStickX / r;
    float normY = rawStickY / r;

    // Tactical Aim Friction Check (Target Slowdown Box)
    aimFrictionActive_ = false;
    float cosFrictionCone = std::cos(glm::radians(config.aimFrictionConeAngleDeg));
    for (const auto& target : targets) {
        glm::vec3 toTarget = target.worldPosition - eyePos;
        float dist = glm::length(toTarget);
        if (dist > 0.1f && dist < 40.0f) {
            glm::vec3 dirToTarget = toTarget / dist;
            if (glm::dot(aimDir, dirToTarget) >= cosFrictionCone) {
                aimFrictionActive_ = true;
                break;
            }
        }
    }

    float frictionMod = aimFrictionActive_ ? config.aimFrictionMultiplier : 1.0f;

    // Dual-Zone Stick Response Curve
    float speedDegreesPerSec = 0.0f;
    if (rHat <= config.freeAimThreshold) {
        // Zone 1: Cubic / Exponential Free-Aim Precision within Bodycam Viewport
        float u = rHat / config.freeAimThreshold;
        speedDegreesPerSec = config.fineAimSensitivity * std::pow(u, config.fineAimGamma);
        torsoTurnRampTimer_ = 0.0f;
    } else {
        // Zone 2: Torso Turn Zone with Dynamic Acceleration Ramp
        float w = (rHat - config.freeAimThreshold) / (1.0f - config.freeAimThreshold);
        torsoTurnRampTimer_ = std::min(config.turnAccelerationRampTime, torsoTurnRampTimer_ + dt);
        float alpha = torsoTurnRampTimer_ / config.turnAccelerationRampTime;
        speedDegreesPerSec = config.torsoTurnBaseSensitivity + 
                             (config.torsoTurnMaxSensitivity - config.torsoTurnBaseSensitivity) * w * alpha;
    }

    float effectiveSpeed = speedDegreesPerSec * frictionMod;
    frameYawDelta_ = normX * effectiveSpeed * dt;
    framePitchDelta_ = -normY * effectiveSpeed * dt;
}

void GamepadController::dispatchTriggerHaptics(SDL_Gamepad* gamepad, const HapticTriggerProfile& profile) {
    if (!gamepad) return;

    if (profile.emptyMagazineSlack) {
        // Loose empty trigger: zero resistance
        SDL_RumbleGamepadTriggers(gamepad, 0, 0, 100);
    } else if (profile.firePulseAmplitude > 0.05f) {
        // High-impulse firing punch
        uint16_t pulseVal = static_cast<uint16_t>(profile.firePulseAmplitude * 65535.0f);
        SDL_RumbleGamepadTriggers(gamepad, pulseVal / 2, pulseVal, 40);
    } else {
        // Normal resistance take-up
        uint16_t resistance = static_cast<uint16_t>(profile.takeUpResistance * 32767.0f);
        SDL_RumbleGamepadTriggers(gamepad, resistance, 0, 50);
    }
}

} // namespace ze::core
