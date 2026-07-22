#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <array>
#include <functional>

namespace ai {

// Director state machine (M5.4-EXT-01)
enum class DirectorPhase : uint8_t {
    Calm, Tension, Chase, Siege, Recovery
};

// Personality preset (M5.4-EXT-02)
enum class DirectorPersonality : uint8_t {
    Cassandra,  // escalates steadily
    Phoebe,     // calm with sudden bursts
    Randy       // chaotic, high variance
};

// Event grammar slot filler (M5.4-EXT-04)
struct EventTemplate {
    uint32_t id;
    float weight;
    float cooldown;
    float minDistanceToPlayer;
    struct Slot {
        int type; // 0=faction, 1=reward, 2=risk, 3=moralChoice
    };
    std::vector<Slot> slots;
};

// Director core (M5.4-EXT-01..17)
class AIDirector {
public:
    AIDirector();
    ~AIDirector() = default;

    // Per-tick update
    void tick(float dt, const glm::vec3& playerPos,
              float health, float noiseLevel, float settlementWealth,
              float timeOfDay, uint32_t population);

    // Event query — returns the event template id if director fires one this tick
    uint32_t pollEvent();

    // Configuration
    void setPersonality(DirectorPersonality p) { personality_ = p; }
    DirectorPersonality personality() const { return personality_; }
    DirectorPhase phase() const { return phase_; }

    // Event template registration (M5.4-EXT-17)
    void registerEventTemplate(const EventTemplate& tmpl);

    // Event cooldown management (M5.4-EXT-16)
    void resetCooldowns();

private:
    DirectorPersonality personality_ = DirectorPersonality::Cassandra;
    DirectorPhase phase_ = DirectorPhase::Calm;

    // State weights derived from M5.4-EXT-01
    std::array<float, 5> stateWeights_{};
    std::array<float, 5> stateScores_{};

    // Event queue
    std::vector<EventTemplate> templates_;
    std::vector<float> cooldowns_;    // remaining cooldown per template
    uint32_t pendingEvent_ = 0;

    // Pacing parameters
    float phaseTimer_ = 0.0f;
    float escalationRate_ = 1.4f;
    float calmBias_ = 1.0f;
    float entropy_ = 0.5f;

    // Internal state evaluation
    void evaluateStateWeights(float noiseLevel, float settlementWealth, float timeOfDay, float health);
    void transitionPhase();
    void applyPersonality();
    uint32_t selectEvent();
    void decayCooldowns(float dt);
};

} // namespace ai
