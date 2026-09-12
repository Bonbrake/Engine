#include "ze/ai/L4D2Director.h"
#include <algorithm>
#include <cmath>

namespace ze::ai {

L4D2Director::L4D2Director() {
    updateBudgets();
}

void L4D2Director::update(float dt, std::vector<SurvivorTelemetry>& team, uint32_t activeHostileCount) {
    if (dt <= 0.0f) return;

    calculateIntensity(dt, team, activeHostileCount);
    updateStateMachine(dt, activeHostileCount);
    updateBudgets();
    updateAtmosphere(dt);
}

void L4D2Director::triggerCrescendo(const std::string& reason) {
    DirectorEvent evt;
    evt.type = DirectorEvent::Type::CrescendoTriggered;
    evt.oldPhase = currentPhase_;
    evt.newPhase = PacingPhase::SustainPeak;
    evt.intensityScore = teamIntensity_;
    evt.contextMessage = reason.empty() ? "Crescendo event initiated" : reason;

    currentPhase_ = PacingPhase::SustainPeak;
    phaseTimer_ = 0.0f;
    teamIntensity_ = std::max(teamIntensity_, config.peakThreshold);

    dispatchEvent(evt);
}

void L4D2Director::calculateIntensity(float dt, std::vector<SurvivorTelemetry>& team, uint32_t activeHostileCount) {
    if (team.empty()) {
        teamIntensity_ = std::max(0.0f, teamIntensity_ - config.intensityDecayRate * dt);
        maxSurvivorStress_ = 0.0f;
        maxSeparationDistance_ = 0.0f;
        isolatedSurvivorId_ = 0;
        return;
    }

    float totalDamageFactor = 0.0f;
    float totalAmmoDeficit = 0.0f;
    float incapPenalty = 0.0f;
    float maxIndivStress = 0.0f;

    // 1. Calculate centroid (geometric center of team) for lone-wolf analysis
    float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
    for (const auto& s : team) {
        centerX += s.posX;
        centerY += s.posY;
        centerZ += s.posZ;
    }
    float n = static_cast<float>(team.size());
    centerX /= n; centerY /= n; centerZ /= n;

    maxSeparationDistance_ = 0.0f;
    isolatedSurvivorId_ = 0;

    // 2. Evaluate individual survivor metrics and dispersion
    for (auto& s : team) {
        float maxH = (s.maxHealth > 0.0f) ? s.maxHealth : 100.0f;
        float healthPct = std::clamp(s.health / maxH, 0.0f, 1.0f);
        totalDamageFactor += (1.0f - healthPct);

        float ammoDef = (1.0f - std::clamp(s.ammoFraction, 0.0f, 1.0f));
        totalAmmoDeficit += ammoDef;

        float indivStress = (1.0f - healthPct) * 0.45f + ammoDef * 0.20f;

        if (s.isIncapacitated || s.isPinned) {
            incapPenalty += 0.5f;
            indivStress += 0.55f;
        }

        if (s.recentDamage > 0.0f) {
            float dmgFactor = std::min(1.0f, s.recentDamage / 50.0f);
            totalDamageFactor += dmgFactor;
            indivStress += dmgFactor * 0.40f;
        }

        if (indivStress > maxIndivStress) {
            maxIndivStress = indivStress;
        }

        // Distance from team centroid
        float dx = s.posX - centerX;
        float dy = s.posY - centerY;
        float dz = s.posZ - centerZ;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (dist > config.loneWolfDistance) {
            s.isLoneWolf = true;
            if (dist > maxSeparationDistance_) {
                maxSeparationDistance_ = dist;
                isolatedSurvivorId_ = s.entityId;
            }
        } else {
            s.isLoneWolf = false;
        }
    }

    maxSurvivorStress_ = std::clamp(maxIndivStress, 0.0f, 1.0f);

    float avgDamage = totalDamageFactor / n;
    float avgAmmoDeficit = totalAmmoDeficit / n;
    float mobFactor = std::clamp(static_cast<float>(activeHostileCount) / 100.0f, 0.0f, 1.0f);
    float separationFactor = std::clamp(maxSeparationDistance_ / (config.loneWolfDistance * 2.0f), 0.0f, 1.0f);

    // Weighted composite intensity formula (Valve Dramatic Pacing Standard)
    float targetIntensity = 
        (config.weightDamage * avgDamage) +
        (config.weightAmmoDeficit * avgAmmoDeficit) +
        (config.weightMobDensity * mobFactor) +
        (config.weightIncap * std::clamp(incapPenalty, 0.0f, 1.0f)) +
        (config.weightSeparation * separationFactor);

    // Exponential moving average filter for smooth transitions
    float alpha = 1.0f - std::exp(-dt * 2.0f);
    teamIntensity_ = teamIntensity_ + alpha * (targetIntensity - teamIntensity_);
    teamIntensity_ = std::clamp(teamIntensity_, 0.0f, 1.0f);

    // Dynamic Resource Scarcity Calibration (L4D2 Dynamic Item Placement)
    resourceBudget_.medicalDropProbability = std::clamp(0.10f + 0.60f * avgDamage, 0.10f, 0.70f);
    resourceBudget_.ammoCacheProbability = std::clamp(0.20f + 0.50f * avgAmmoDeficit, 0.20f, 0.80f);
    resourceBudget_.tier2WeaponChance = (currentPhase_ == PacingPhase::Relax) ? 0.40f : 0.15f;
}

void L4D2Director::updateStateMachine(float dt, uint32_t activeHostileCount) {
    switch (currentPhase_) {
    case PacingPhase::BuildUp:
        phaseTimer_ += dt;
        // Valve Booth standard: transition to peak if team intensity OR max survivor stress crosses threshold
        if (teamIntensity_ >= config.peakThreshold || maxSurvivorStress_ >= config.peakThreshold) {
            PacingPhase old = currentPhase_;
            currentPhase_ = PacingPhase::SustainPeak;
            phaseTimer_ = 0.0f;

            DirectorEvent evt;
            evt.type = DirectorEvent::Type::PhaseChanged;
            evt.oldPhase = old;
            evt.newPhase = currentPhase_;
            evt.intensityScore = std::max(teamIntensity_, maxSurvivorStress_);
            evt.contextMessage = "Intensity threshold breached: WWZ Swarm assault triggered";
            dispatchEvent(evt);
        }
        break;

    case PacingPhase::SustainPeak:
        phaseTimer_ += dt;
        // Peak assault lasts for the calibrated duration
        if (phaseTimer_ >= config.sustainPeakDuration) {
            PacingPhase old = currentPhase_;
            currentPhase_ = PacingPhase::PeakFade;
            phaseTimer_ = 0.0f;

            DirectorEvent evt;
            evt.type = DirectorEvent::Type::PhaseChanged;
            evt.oldPhase = old;
            evt.newPhase = currentPhase_;
            evt.intensityScore = teamIntensity_;
            evt.contextMessage = "Peak duration elapsed: Halting spawns, entering Peak Fade";
            dispatchEvent(evt);
        }
        break;

    case PacingPhase::PeakFade:
        phaseTimer_ += dt;
        // Spawns are halted while players mop up. Transition to Relax after fade duration.
        if (phaseTimer_ >= config.peakFadeDuration) {
            PacingPhase old = currentPhase_;
            currentPhase_ = PacingPhase::Relax;
            phaseTimer_ = 0.0f;

            DirectorEvent evt;
            evt.type = DirectorEvent::Type::RelaxEntered;
            evt.oldPhase = old;
            evt.newPhase = currentPhase_;
            evt.intensityScore = teamIntensity_;
            evt.contextMessage = "Combat cleared: Mandatory Relax period initiated";
            dispatchEvent(evt);
        }
        break;

    case PacingPhase::Relax:
        // Bleed off intensity during mandatory lull
        teamIntensity_ = std::max(0.0f, teamIntensity_ - config.intensityDecayRate * dt);
        maxSurvivorStress_ = std::max(0.0f, maxSurvivorStress_ - config.intensityDecayRate * dt);

        // Booth Break Condition: Relax timer ONLY counts down if engaged hostiles are cleared (<= 5)
        if (activeHostileCount <= 5) {
            phaseTimer_ += dt;
        }

        if (phaseTimer_ >= config.relaxDuration) {
            PacingPhase old = currentPhase_;
            currentPhase_ = PacingPhase::BuildUp;
            phaseTimer_ = 0.0f;

            DirectorEvent evt;
            evt.type = DirectorEvent::Type::PhaseChanged;
            evt.oldPhase = old;
            evt.newPhase = currentPhase_;
            evt.intensityScore = teamIntensity_;
            evt.contextMessage = "Relax period ended: Entering BuildUp phase";
            dispatchEvent(evt);
        }
        break;
    }
}

void L4D2Director::updateBudgets() {
    switch (currentPhase_) {
    case PacingPhase::BuildUp:
        currentBudget_.allowMobSpawns = true;
        currentBudget_.maxWanderingZombies = 35;
        currentBudget_.maxHordeZombies = 0;
        currentBudget_.specialInfectedSlots = 1;
        currentBudget_.spawnIntervalSeconds = 3.0f;
        currentBudget_.prioritizedSpecialRole = (maxSeparationDistance_ > config.loneWolfDistance)
            ? SpecialInfectedRole::Stalker
            : SpecialInfectedRole::Disabler;
        break;

    case PacingPhase::SustainPeak:
        currentBudget_.allowMobSpawns = true;
        currentBudget_.maxWanderingZombies = 60;
        currentBudget_.maxHordeZombies = 650; // World War Z Swarm Scale!
        currentBudget_.specialInfectedSlots = 4;
        currentBudget_.spawnIntervalSeconds = 0.25f;
        currentBudget_.prioritizedSpecialRole = SpecialInfectedRole::CrowdControl;
        break;

    case PacingPhase::PeakFade:
        currentBudget_.allowMobSpawns = false;
        currentBudget_.maxWanderingZombies = 0;
        currentBudget_.maxHordeZombies = 0;
        currentBudget_.specialInfectedSlots = 0;
        currentBudget_.spawnIntervalSeconds = 999.0f;
        break;

    case PacingPhase::Relax:
        currentBudget_.allowMobSpawns = false;
        currentBudget_.maxWanderingZombies = 0;
        currentBudget_.maxHordeZombies = 0;
        currentBudget_.specialInfectedSlots = 0;
        currentBudget_.spawnIntervalSeconds = 999.0f;
        break;
    }
}

void L4D2Director::updateAtmosphere(float dt) {
    float targetFog = 1.0f;
    float targetStorm = 0.0f;
    float targetMuffle = 0.0f;

    if (currentPhase_ == PacingPhase::SustainPeak) {
        targetFog = 1.8f + (teamIntensity_ * 0.7f); // Up to 2.5x volumetric fog
        targetStorm = teamIntensity_;
        targetMuffle = 0.40f + (teamIntensity_ * 0.20f); // Auditory exclusion under fire
    } else if (currentPhase_ == PacingPhase::BuildUp) {
        targetFog = 1.0f + (teamIntensity_ * 0.3f);
        targetStorm = 0.1f;
        targetMuffle = 0.0f;
    }

    float rate = 1.0f - std::exp(-dt * 1.5f);
    atmosphere_.volumetricFogMultiplier += rate * (targetFog - atmosphere_.volumetricFogMultiplier);
    atmosphere_.stormIntensity += rate * (targetStorm - atmosphere_.stormIntensity);
    atmosphere_.acousticMuffleFactor += rate * (targetMuffle - atmosphere_.acousticMuffleFactor);
}

void L4D2Director::dispatchEvent(const DirectorEvent& evt) {
    if (eventCallback_) {
        eventCallback_(evt);
    }
}

} // namespace ze::ai
