#include "ze/audio/BodycamAcoustics.h"
#include <cmath>
#include <algorithm>

namespace ze::audio {

BallisticShockwaveResult BodycamAcousticsSystem::calculateBallistics(
    const glm::vec3& bulletOrigin, 
    const glm::vec3& bulletVelocity, 
    float bulletCaliberMm,
    const glm::vec3& listenerPos) 
{
    BallisticShockwaveResult result;
    float bulletSpeed = glm::length(bulletVelocity);
    if (bulletSpeed < 1.0f) return result;

    glm::vec3 bulletDir = bulletVelocity / bulletSpeed;
    glm::vec3 toListener = listenerPos - bulletOrigin;
    float distanceDirect = glm::length(toListener);

    // Muzzle blast arrival time (spherical wave propagating at speed of sound)
    result.muzzleDelaySeconds = distanceDirect / SOUND_SPEED;

    // Vector projection along bullet ray
    float dParallel = glm::dot(toListener, bulletDir);
    glm::vec3 closestPoint = bulletOrigin + bulletDir * dParallel;
    float dPerpendicular = glm::length(listenerPos - closestPoint);

    if (bulletSpeed > SOUND_SPEED) {
        result.isSupersonic = true;

        // Mach cone geometry: sin(theta_M) = c / v
        float sinThetaM = SOUND_SPEED / bulletSpeed;
        float cosThetaM = std::sqrt(std::max(0.0f, 1.0f - sinThetaM * sinThetaM));
        float tanThetaM = sinThetaM / std::max(cosThetaM, 0.001f);

        // Setback along trajectory where the Mach cone ray was emitted toward listener
        float setback = dPerpendicular / tanThetaM;
        float emitDistance = dParallel - setback;

        if (emitDistance > 0.0f) {
            result.crackOriginWorld = bulletOrigin + bulletDir * emitDistance;
            float tBulletTravel = emitDistance / bulletSpeed;
            float soundTravelDist = std::sqrt(dPerpendicular * dPerpendicular + setback * setback);
            float tSoundTravel = soundTravelDist / SOUND_SPEED;

            result.crackDelaySeconds = tBulletTravel + tSoundTravel;
        } else {
            // Emitted right at muzzle exit
            result.crackOriginWorld = bulletOrigin;
            result.crackDelaySeconds = distanceDirect / SOUND_SPEED;
        }

        // Cylindrical shockwave amplitude decay: SPL ~ 1 / sqrt(r)
        float caliberScalar = std::max(bulletCaliberMm, 4.0f) / 5.56f;
        float distanceDecay = 1.0f / std::sqrt(std::max(dPerpendicular, 0.5f));
        result.shockwaveIntensitySPL = 135.0f * caliberScalar * distanceDecay;
    } else {
        result.isSupersonic = false;
        result.crackDelaySeconds = 0.0f;

        // Subsonic whiz-by Doppler pitch plunge: f' = f * c / (c - v * cos(theta))
        glm::vec3 dirDirect = (distanceDirect > 0.01f) ? (toListener / distanceDirect) : bulletDir;
        float vObsApproach = glm::dot(bulletVelocity, dirDirect);
        float denom = std::max(SOUND_SPEED - vObsApproach, 10.0f);
        result.subsonicDopplerPitch = std::clamp(SOUND_SPEED / denom, 0.4f, 2.5f);
    }

    return result;
}

void BodycamAcousticsSystem::triggerWeaponFoley(WeaponFoleyStage stage, const glm::vec3& weaponPos, float loudness) {
    (void)weaponPos;
    (void)loudness;
    lastFoleyStage_ = stage;
}

CasingBounceAcoustic BodycamAcousticsSystem::getCasingBounceAcoustic(SurfaceMaterial material) const {
    CasingBounceAcoustic acoustic;
    acoustic.surface = material;

    switch (material) {
        case SurfaceMaterial::Concrete:
            acoustic.frequencyPitchHz = 3400.0f;
            acoustic.highFreqCutoffHz = 18000.0f;
            acoustic.bounceDecayDamping = 0.65f;
            acoustic.bounceCount = 3;
            break;
        case SurfaceMaterial::Wood:
            acoustic.frequencyPitchHz = 2100.0f;
            acoustic.highFreqCutoffHz = 9000.0f;
            acoustic.bounceDecayDamping = 0.50f;
            acoustic.bounceCount = 2;
            break;
        case SurfaceMaterial::Metal:
            acoustic.frequencyPitchHz = 4800.0f;
            acoustic.highFreqCutoffHz = 20000.0f;
            acoustic.bounceDecayDamping = 0.80f;
            acoustic.bounceCount = 4;
            break;
        case SurfaceMaterial::Carpet:
            acoustic.frequencyPitchHz = 850.0f;
            acoustic.highFreqCutoffHz = 2500.0f;
            acoustic.bounceDecayDamping = 0.20f;
            acoustic.bounceCount = 1;
            break;
        case SurfaceMaterial::Earth:
            acoustic.frequencyPitchHz = 1100.0f;
            acoustic.highFreqCutoffHz = 3500.0f;
            acoustic.bounceDecayDamping = 0.30f;
            acoustic.bounceCount = 1;
            break;
    }

    return acoustic;
}

float BodycamAcousticsSystem::calculateGearRattleSPL(float linearAccelerationMagnitude, float torsoAngularVelocityMagnitude) const {
    float rattle = 0.12f * linearAccelerationMagnitude + 0.08f * (torsoAngularVelocityMagnitude * torsoAngularVelocityMagnitude);
    return std::clamp(rattle, 0.0f, 1.0f);
}

} // namespace ze::audio
