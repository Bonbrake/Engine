#pragma once

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

namespace ze::audio {

enum class SurfaceMaterial : uint8_t {
    Concrete = 0,
    Wood = 1,
    Metal = 2,
    Carpet = 3,
    Earth = 4
};

enum class WeaponFoleyStage : uint8_t {
    TriggerSearRelease = 0,
    ChamberExplosion = 1,
    BoltSlideRearward = 2,
    SpentCasingEject = 3,
    BoltSlamForward = 4,
    DryFireClick = 5,
    MagRelease = 6,
    MagInsertWell = 7,
    MagPalmSlap = 8
};

struct BallisticShockwaveResult {
    bool isSupersonic{ false };
    float crackDelaySeconds{ 0.0f };   // Arrival time of Mach cone shockwave crack
    float muzzleDelaySeconds{ 0.0f };  // Arrival time of delayed muzzle report blast
    glm::vec3 crackOriginWorld{ 0.0f };// Spatial location where Mach cone radiated toward listener
    float shockwaveIntensitySPL{ 0.0f };// Sound pressure level (dB / arbitrary scalar)
    float subsonicDopplerPitch{ 1.0f }; // Doppler pitch multiplier if subsonic whiz-by
};

struct CasingBounceAcoustic {
    SurfaceMaterial surface{ SurfaceMaterial::Concrete };
    float frequencyPitchHz{ 3200.0f }; // Resonant ring frequency
    float highFreqCutoffHz{ 16000.0f };// Low-pass wall absorption
    float bounceDecayDamping{ 0.65f }; // Energy loss per ground bounce
    uint32_t bounceCount{ 3 };
};

class BodycamAcousticsSystem {
public:
    BodycamAcousticsSystem() = default;
    ~BodycamAcousticsSystem() = default;

    // Ballistic acoustics calculation (supersonic Mach cone vs subsonic Doppler whiz-by)
    BallisticShockwaveResult calculateBallistics(const glm::vec3& bulletOrigin, 
                                                 const glm::vec3& bulletVelocity, 
                                                 float bulletCaliberMm,
                                                 const glm::vec3& listenerPos);

    // Multi-stage weapon handling Foley
    void triggerWeaponFoley(WeaponFoleyStage stage, const glm::vec3& weaponPos, float loudness = 1.0f);
    WeaponFoleyStage getLastFoleyStage() const { return lastFoleyStage_; }

    // Surface-dependent casing bounce acoustics
    CasingBounceAcoustic getCasingBounceAcoustic(SurfaceMaterial material) const;

    // Kinetic gear rattle clatter tied to linear acceleration and angular jerk
    float calculateGearRattleSPL(float linearAccelerationMagnitude, float torsoAngularVelocityMagnitude) const;

    static constexpr float SOUND_SPEED = 343.0f; // m/s in dry air at 20°C

private:
    WeaponFoleyStage lastFoleyStage_{ WeaponFoleyStage::TriggerSearRelease };
};

} // namespace ze::audio
