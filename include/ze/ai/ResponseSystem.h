#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <glm/glm.hpp>

namespace ze::ai {

// Predefined dialogue concepts (Valve RR1/RR2 standard)
enum class SpeechConcept : uint8_t {
    Reloading = 0,
    TakingFire = 1,
    SpecialSpotted = 2,
    HordeIncoming = 3,
    HealOther = 4,
    CoverMe = 5,
    RadioCheckIn = 6,
    SafehouseWelcome = 7,
    PainGruntBiological = 8, // Permitted for player (physical audio only)
    SquadCommandAcknowledged = 9
};

// Response criteria query payload
struct CriteriaQuery {
    uint64_t speakerEntityId{ 0 };
    bool isNpc{ true };
    SpeechConcept speechConcept{ SpeechConcept::Reloading };
    float healthFraction{ 1.0f }; // 0.0 to 1.0
    float directorIntensity{ 0.0f }; // 0.0 to 1.0
    float distanceToPlayer{ 5.0f }; // Meters
    uint32_t nearbyEnemyType{ 0 };
    glm::vec3 speakerWorldPos{ 0.0f };
};

// Dispatched audio & subtitle response
struct DispatchedSpeechResponse {
    bool valid{ false };
    uint64_t speakerEntityId{ 0 };
    std::string audioCueName;
    std::string subtitleText;
    float durationSeconds{ 2.0f };
    bool isSpatial3D{ true };
    glm::vec3 worldPos{ 0.0f };
};

// Sub-millisecond Criteria Response System for NPC Squadmates and Radio
class ResponseSystem {
public:
    static constexpr uint64_t PLAYER_ENTITY_ID = 1ULL; // Authoritative human player ID

    ResponseSystem();
    ~ResponseSystem() = default;

    void initialize();
    void reset();

    // Simulation tick update (evaluates speech cooldowns)
    void update(float dt);

    // Query response: Returns true if criteria matched and spoken audio queued.
    // Invariant: Returns FALSE immediately for PLAYER_ENTITY_ID (Silent Protagonist).
    bool queryResponse(const CriteriaQuery& query, DispatchedSpeechResponse& outResponse);

    // Biological player audio queries (breathing, pain grunts, heartbeat - NOT spoken barks)
    bool queryPlayerBiologicalExertion(float staminaFraction, float healthFraction, 
                                       float directorIntensity, DispatchedSpeechResponse& outResponse);

    // Queries
    size_t getRegisteredRuleCount() const { return rules_.size(); }
    float getConceptCooldownRemaining(SpeechConcept speechConcept, uint64_t speakerId) const;

private:
    struct ResponseRule {
        SpeechConcept speechConcept{ SpeechConcept::Reloading };
        bool requireNpcOnly{ true };
        float minIntensity{ 0.0f };
        float maxIntensity{ 1.0f };
        float cooldownSeconds{ 5.0f };
        std::vector<std::string> audioCues;
        std::vector<std::string> subtitles;
    };

    std::vector<ResponseRule> rules_;
    
    // Speaker + Concept -> Cooldown timestamp
    std::unordered_map<uint64_t, float> cooldownMap_;
    float globalTime_{ 0.0f };
};

} // namespace ze::ai
