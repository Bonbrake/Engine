#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace ze::ai {

// Pacing state machine phases (Valve L4D2 AI Director 2.0 standard)
enum class PacingPhase : uint8_t {
    BuildUp = 0,     // Spawning ambient mobs; intensity builds toward peak threshold
    SustainPeak = 1, // Horde assault / crescendo in progress; maximum combat intensity
    PeakFade = 2,    // Spawns halted; survivors clearing remaining engaged infected
    Relax = 3        // Mandatory calm period; zero ambient spawns, allows scavenging/healing
};

// Roles for special infected synergy coordination (L4D2 tactical pairing)
enum class SpecialInfectedRole : uint8_t {
    Disabler = 0,    // Hunter / Charger / Jockey (pins/disables single survivor)
    AreaDenial = 1,  // Spitter (forces team to displace out of choke points)
    CrowdControl = 2,// Boomer (horde beacon, blinds and disorients)
    Stalker = 3      // Smoker (drags stragglers/lone-wolves from high perches)
};

// Telemetry fed on every simulation tick (60Hz) per active survivor
struct SurvivorTelemetry {
    uint64_t entityId{ 0 };
    float health{ 100.0f };
    float maxHealth{ 100.0f };
    float ammoFraction{ 1.0f }; // 0.0 (empty) to 1.0 (full reserve)
    float flowDistance{ 0.0f };  // Meters progressed along navmesh flow path
    float posX{ 0.0f }, posY{ 0.0f }, posZ{ 0.0f }; // World coordinates
    bool isIncapacitated{ false };
    bool isPinned{ false };      // Pinned by special infected
    float recentDamage{ 0.0f };  // Accumulated damage in current evaluation window
    bool isLoneWolf{ false };    // Computed by Director: separated from pack (>25m)
};

// Spawning budget dictated by the mathematical director
struct DirectorBudget {
    uint32_t maxWanderingZombies{ 15 };
    uint32_t maxHordeZombies{ 0 };
    uint32_t specialInfectedSlots{ 0 };
    float spawnIntervalSeconds{ 3.0f };
    bool allowMobSpawns{ true };
    SpecialInfectedRole prioritizedSpecialRole{ SpecialInfectedRole::Disabler };
};

// Dynamic resource & scavenging director budget (L4D2 dynamic item seeding)
struct ResourceScarcityBudget {
    float medicalDropProbability{ 0.35f }; // Spawns first aid / pills if team is struggling
    float ammoCacheProbability{ 0.50f };   // Spawns ammo piles if reserves are depleted
    float tier2WeaponChance{ 0.20f };      // Higher tier weapons seeded before crescendos
};

// Environmental & audio atmosphere modulation (L4D2 Hard Rain dynamic weather)
struct EnvironmentalAtmosphere {
    float volumetricFogMultiplier{ 1.0f }; // Scales Vulkan 1.4 fog density (1.0x to 2.5x)
    float stormIntensity{ 0.0f };          // 0.0 = clear, 1.0 = torrential squall + lightning
    float acousticMuffleFactor{ 0.0f };    // 0.0 = normal, 0.6 = adrenaline tunnel vision
};

// Pacing event dispatched to audio, HUD, and the Neural SLM Co-Processor
struct DirectorEvent {
    enum class Type : uint8_t {
        PhaseChanged,
        CrescendoTriggered,
        SurvivorDown,
        SurvivorRecovered,
        LoneWolfIsolated,
        IntensitySpike,
        RelaxEntered
    } type{ Type::PhaseChanged };

    PacingPhase oldPhase{ PacingPhase::BuildUp };
    PacingPhase newPhase{ PacingPhase::BuildUp };
    float intensityScore{ 0.0f };
    uint64_t subjectEntityId{ 0 };
    std::string contextMessage;
};

// Left 4 Dead 2 Style Mathematical Pacing Engine (Valve Director 2.0 Standard)
class L4D2Director {
public:
    L4D2Director();
    ~L4D2Director() = default;

    // Simulation tick update (called from Main Thread at 60Hz)
    void update(float dt, std::vector<SurvivorTelemetry>& team, uint32_t activeHostileCount);

    // Manual event triggers (e.g. car alarm tripped, crescendo button pressed)
    void triggerCrescendo(const std::string& reason);

    // Queries
    PacingPhase getCurrentPhase() const { return currentPhase_; }
    float getIntensity() const { return teamIntensity_; }
    float getPhaseTime() const { return phaseTimer_; }
    DirectorBudget getSpawnBudget() const { return currentBudget_; }
    ResourceScarcityBudget getResourceBudget() const { return resourceBudget_; }
    EnvironmentalAtmosphere getAtmosphere() const { return atmosphere_; }
    float getMaxSeparationDistance() const { return maxSeparationDistance_; }
    uint64_t getMostIsolatedSurvivorId() const { return isolatedSurvivorId_; }

    float getMaxSurvivorStress() const { return maxSurvivorStress_; }
    uint32_t getSwarmHordeTarget() const { return currentBudget_.maxHordeZombies; }

    // Event listener registration (for SLM Bridge and Game Systems)
    void setEventListener(std::function<void(const DirectorEvent&)> callback) {
        eventCallback_ = std::move(callback);
    }

    // Tuning parameters
    struct TuningConfig {
        float peakThreshold{ 0.75f };       // Intensity required to trigger SustainPeak
        float sustainPeakDuration{ 25.0f }; // Seconds peak assault lasts
        float peakFadeDuration{ 12.0f };    // Seconds to clear remaining hostiles
        float relaxDuration{ 30.0f };       // Seconds calm period before BuildUp resumes
        float intensityDecayRate{ 0.05f };  // Natural intensity bleed-off per second
        float loneWolfDistance{ 25.0f };    // Meters from team center to trigger lone wolf flag
        
        // Weighting factors for intensity calculation
        float weightDamage{ 0.45f };
        float weightAmmoDeficit{ 0.20f };
        float weightMobDensity{ 0.15f };
        float weightIncap{ 0.15f };
        float weightSeparation{ 0.05f };
    } config;

private:
    void calculateIntensity(float dt, std::vector<SurvivorTelemetry>& team, uint32_t activeHostileCount);
    void updateStateMachine(float dt, uint32_t activeHostileCount);
    void updateBudgets();
    void updateAtmosphere(float dt);
    void dispatchEvent(const DirectorEvent& evt);

    PacingPhase currentPhase_{ PacingPhase::BuildUp };
    float phaseTimer_{ 0.0f };
    float teamIntensity_{ 0.0f };
    float maxSurvivorStress_{ 0.0f };
    float maxSeparationDistance_{ 0.0f };
    uint64_t isolatedSurvivorId_{ 0 };

    DirectorBudget currentBudget_;
    ResourceScarcityBudget resourceBudget_;
    EnvironmentalAtmosphere atmosphere_;
    std::function<void(const DirectorEvent&)> eventCallback_;
};

} // namespace ze::ai
