#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

namespace ze::render {

enum class ReadyStance : uint8_t {
    HipReady = 0,
    PointAimADS = 1,
    HighReadyCompressed = 2,
    LowReadyCompressed = 3
};

enum class MagCheckState : uint8_t {
    Idle = 0,
    DroppingToPalm = 1,
    InspectingWitnessHoles = 2,
    ReseatingMagazine = 3
};

enum class EstimatedAmmoLevel : uint8_t {
    Full = 0,             // 80% - 100% capacity
    ApproximatelyHalf = 1,// 40% - 79% capacity
    Low = 2,              // 1% - 39% capacity
    Empty = 3             // 0 rounds
};

struct WeaponInertiaConfig {
    float barrelLengthMeters{ 0.75f };    // Physical weapon barrel length
    float springStiffness{ 14.0f };       // Angular recovery spring (rad/s)
    float dampingRatio{ 0.82f };          // Damping factor
    float rotationalDragFactor{ 0.10f };  // Inertial drag per rad/s turn
    float maxPushbackMeters{ 0.45f };     // Maximum compression toward armpit
    float highReadyPitchDegrees{ 28.0f }; // High-ready upward tilt when compressed
};

class ProceduralWeaponAnim {
public:
    ProceduralWeaponAnim();
    ~ProceduralWeaponAnim() = default;

    // Simulation update (60Hz tick)
    // forwardClearanceMeters: raycast distance to wall, doorframe, or zombie
    // cameraAngularVel: rad/s turning speed of camera (yaw, pitch, roll)
    // directorIntensity: 0.0 (Relax) to 1.0 (SustainPeak)
    void update(float dt, float forwardClearanceMeters, const glm::vec3& cameraAngularVel, float directorIntensity);

    // Physical Magazine Inspection trigger
    void startMagazineInspection(uint32_t currentRounds, uint32_t maxCapacity);

    // Queries
    bool isShootingBlocked() const { return shootingBlocked_; }
    bool isPointShootingStance() const { return currentStance_ == ReadyStance::HighReadyCompressed && !shootingBlocked_; }
    ReadyStance getStance() const { return currentStance_; }
    MagCheckState getMagCheckState() const { return magCheckState_; }
    EstimatedAmmoLevel getInspectedAmmoLevel() const { return estimatedAmmo_; }

    glm::vec3 getTranslationOffset() const { return translationOffset_; }
    glm::vec3 getRotationOffsetDegrees() const { return rotationOffsetDegrees_; }

    WeaponInertiaConfig config;

private:
    ReadyStance currentStance_{ ReadyStance::HipReady };
    MagCheckState magCheckState_{ MagCheckState::Idle };
    EstimatedAmmoLevel estimatedAmmo_{ EstimatedAmmoLevel::Full };
    float magCheckTimer_{ 0.0f };
    bool shootingBlocked_{ false };

    glm::vec3 currentInertiaRot_{ 0.0f };
    glm::vec3 inertiaVelocity_{ 0.0f };
    glm::vec3 translationOffset_{ 0.0f };
    glm::vec3 rotationOffsetDegrees_{ 0.0f };

    float breathPhase_{ 0.0f };
};

} // namespace ze::render
