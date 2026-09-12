#include "ze/render/ProceduralWeaponAnim.h"
#include <cmath>
#include <algorithm>

namespace ze::render {

ProceduralWeaponAnim::ProceduralWeaponAnim() = default;

void ProceduralWeaponAnim::startMagazineInspection(uint32_t currentRounds, uint32_t maxCapacity) {
    float fraction = (maxCapacity > 0) ? (static_cast<float>(currentRounds) / static_cast<float>(maxCapacity)) : 0.0f;

    if (currentRounds == 0) {
        estimatedAmmo_ = EstimatedAmmoLevel::Empty;
    } else if (fraction >= 0.80f) {
        estimatedAmmo_ = EstimatedAmmoLevel::Full;
    } else if (fraction >= 0.40f) {
        estimatedAmmo_ = EstimatedAmmoLevel::ApproximatelyHalf;
    } else {
        estimatedAmmo_ = EstimatedAmmoLevel::Low;
    }

    magCheckState_ = MagCheckState::DroppingToPalm;
    magCheckTimer_ = 0.0f;
}

void ProceduralWeaponAnim::update(float dt, float forwardClearanceMeters, 
                                  const glm::vec3& cameraAngularVel, float directorIntensity) {
    // 1. Obstacle Collision & Procedural Short-Stocking
    float pushbackZ = 0.0f;
    float highReadyPitch = 0.0f;

    if (forwardClearanceMeters < config.barrelLengthMeters) {
        float penetration = config.barrelLengthMeters - forwardClearanceMeters;
        pushbackZ = -std::min(config.maxPushbackMeters, penetration * 0.85f);
        // Tucked Compressed Ready: pitch up slightly (max 14 degrees), tucking stock under armpit
        highReadyPitch = std::min(14.0f, config.highReadyPitchDegrees * (penetration / 0.5f));
        currentStance_ = ReadyStance::HighReadyCompressed;

        // Point-Shooting Rule: Do NOT block shooting unless jammed flat against a solid wall (<0.15m)
        // Player can panic point-shoot with higher spread/recoil penalty
        shootingBlocked_ = (forwardClearanceMeters < 0.15f);
    } else {
        currentStance_ = ReadyStance::HipReady;
        shootingBlocked_ = false;
    }

    // 2. Weapon Rotational Inertia & Mass Drag (Second-Order Spring-Damper)
    // Target lag angle in degrees proportional to turning speed
    glm::vec3 targetLagDegrees = -cameraAngularVel * (config.rotationalDragFactor * (180.0f / 3.14159265f));

    glm::vec3 springForce = (targetLagDegrees - currentInertiaRot_) * (config.springStiffness * config.springStiffness);
    glm::vec3 dampingForce = -inertiaVelocity_ * (2.0f * config.dampingRatio * config.springStiffness);
    glm::vec3 accel = springForce + dampingForce;

    inertiaVelocity_ += accel * dt;
    currentInertiaRot_ += inertiaVelocity_ * dt;

    // 3. Stress-Coupled Breathing Sway (L4D2 Director Intensity)
    float breathFreq = 1.3f + 2.2f * directorIntensity;
    float breathAmp = 0.25f + 0.65f * directorIntensity;
    breathPhase_ += breathFreq * dt;

    float swayYaw = std::sin(breathPhase_) * breathAmp;
    float swayPitch = std::cos(breathPhase_ * 0.5f) * (breathAmp * 0.7f);

    // 4. Physical Magazine Check Animation Phases
    if (magCheckState_ == MagCheckState::DroppingToPalm) {
        magCheckTimer_ += dt;
        if (magCheckTimer_ >= 0.35f) {
            magCheckState_ = MagCheckState::InspectingWitnessHoles;
        }
    } else if (magCheckState_ == MagCheckState::InspectingWitnessHoles) {
        magCheckTimer_ += dt;
        if (magCheckTimer_ >= 0.95f) {
            magCheckState_ = MagCheckState::ReseatingMagazine;
        }
    } else if (magCheckState_ == MagCheckState::ReseatingMagazine) {
        magCheckTimer_ += dt;
        if (magCheckTimer_ >= 1.35f) {
            magCheckState_ = MagCheckState::Idle;
        }
    }

    // Combine final procedural translation and rotation offsets
    translationOffset_ = glm::vec3(swayYaw * 0.004f, swayPitch * 0.004f, pushbackZ);
    rotationOffsetDegrees_ = currentInertiaRot_ + glm::vec3(highReadyPitch + swayPitch, swayYaw, 0.0f);
}

} // namespace ze::render
