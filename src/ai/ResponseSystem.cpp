#include "ze/ai/ResponseSystem.h"

namespace ze::ai {

ResponseSystem::ResponseSystem() {
    initialize();
}

void ResponseSystem::initialize() {
    rules_.clear();
    cooldownMap_.clear();
    globalTime_ = 0.0f;

    // Rule 1: NPC Squadmate Reloading callout
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::Reloading;
        r.requireNpcOnly = true;
        r.minIntensity = 0.0f;
        r.maxIntensity = 1.0f;
        r.cooldownSeconds = 6.0f;
        r.audioCues = { "npc_survivor_reloading_01", "npc_survivor_reloading_02" };
        r.subtitles = { "Cover me, reloading!", "Changing mag!" };
        rules_.push_back(r);
    }

    // Rule 2: NPC Squadmate Taking Fire / Under Attack
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::TakingFire;
        r.requireNpcOnly = true;
        r.minIntensity = 0.3f;
        r.maxIntensity = 1.0f;
        r.cooldownSeconds = 5.0f;
        r.audioCues = { "npc_survivor_takingfire_01", "npc_survivor_takingfire_02" };
        r.subtitles = { "I'm hit! Get them off me!", "They're swarming me!" };
        rules_.push_back(r);
    }

    // Rule 3: Special Infected Spotted
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::SpecialSpotted;
        r.requireNpcOnly = true;
        r.minIntensity = 0.0f;
        r.maxIntensity = 1.0f;
        r.cooldownSeconds = 8.0f;
        r.audioCues = { "npc_survivor_spotted_special_01", "npc_survivor_spotted_special_02" };
        r.subtitles = { "Watch out! Special infected spotted!", "Heads up, mutator in the area!" };
        rules_.push_back(r);
    }

    // Rule 4: WWZ Swarm / Horde Incoming Warning
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::HordeIncoming;
        r.requireNpcOnly = true;
        r.minIntensity = 0.5f;
        r.maxIntensity = 1.0f;
        r.cooldownSeconds = 15.0f;
        r.audioCues = { "npc_survivor_horde_roar_01", "npc_survivor_horde_roar_02" };
        r.subtitles = { "Swarm incoming! Hundreds of them!", "They're climbing the walls! Fall back!" };
        rules_.push_back(r);
    }

    // Rule 5: Heal Other
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::HealOther;
        r.requireNpcOnly = true;
        r.minIntensity = 0.0f;
        r.maxIntensity = 0.8f;
        r.cooldownSeconds = 10.0f;
        r.audioCues = { "npc_survivor_heal_01" };
        r.subtitles = { "Hold still, patching you up." };
        rules_.push_back(r);
    }

    // Rule 6: Emergency Radio Check-In (Military / Outpost Broadcast)
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::RadioCheckIn;
        r.requireNpcOnly = true;
        r.minIntensity = 0.0f;
        r.maxIntensity = 0.6f;
        r.cooldownSeconds = 25.0f;
        r.audioCues = { "radio_outpost_dispatch_01", "radio_outpost_dispatch_02" };
        r.subtitles = { "[Radio static] Outpost 4 to any survivors, bridge checkpoint has fallen.",
                        "[Radio static] Air recon confirms massive horde moving along sector 7." };
        rules_.push_back(r);
    }

    // Rule 7: Squad Command Acknowledged (Tactical Wheel Order Response)
    {
        ResponseRule r;
        r.speechConcept = SpeechConcept::SquadCommandAcknowledged;
        r.requireNpcOnly = true;
        r.minIntensity = 0.0f;
        r.maxIntensity = 1.0f;
        r.cooldownSeconds = 1.0f;
        r.audioCues = { "npc_survivor_ack_01", "npc_survivor_ack_02", "npc_survivor_ack_03" };
        r.subtitles = { "Copy that, moving on it!", "On my way, covering that sector!", "Understood, holding here!" };
        rules_.push_back(r);
    }
}

void ResponseSystem::reset() {
    cooldownMap_.clear();
    globalTime_ = 0.0f;
}

void ResponseSystem::update(float dt) {
    globalTime_ += dt;
}

bool ResponseSystem::queryResponse(const CriteriaQuery& query, DispatchedSpeechResponse& outResponse) {
    outResponse.valid = false;

    // SILENT PROTAGONIST INVARIANT: Human player never speaks unsolicited barks
    if (query.speakerEntityId == PLAYER_ENTITY_ID || !query.isNpc) {
        return false;
    }

    // Look up cooldown
    uint64_t cooldownKey = (query.speakerEntityId << 8) | static_cast<uint64_t>(query.speechConcept);
    auto it = cooldownMap_.find(cooldownKey);
    if (it != cooldownMap_.end() && globalTime_ < it->second) {
        return false; // Still on cooldown
    }

    // Find matching rule
    for (const auto& rule : rules_) {
        if (rule.speechConcept == query.speechConcept) {
            if (rule.requireNpcOnly && !query.isNpc) continue;
            if (query.directorIntensity < rule.minIntensity || query.directorIntensity > rule.maxIntensity) continue;

            // Matched!
            outResponse.valid = true;
            outResponse.speakerEntityId = query.speakerEntityId;
            outResponse.audioCueName = rule.audioCues.empty() ? "" : rule.audioCues[0];
            outResponse.subtitleText = rule.subtitles.empty() ? "" : rule.subtitles[0];
            outResponse.durationSeconds = 2.2f;
            outResponse.isSpatial3D = true;
            outResponse.worldPos = query.speakerWorldPos;

            // Set cooldown
            cooldownMap_[cooldownKey] = globalTime_ + rule.cooldownSeconds;
            return true;
        }
    }

    return false;
}

bool ResponseSystem::queryPlayerBiologicalExertion(float staminaFraction, float healthFraction, 
                                                  float directorIntensity, DispatchedSpeechResponse& outResponse) {
    outResponse.valid = false;
    outResponse.speakerEntityId = PLAYER_ENTITY_ID;
    outResponse.isSpatial3D = false; // Player head audio (stereo)

    if (staminaFraction < 0.25f) {
        outResponse.valid = true;
        outResponse.audioCueName = "player_biological_heavy_panting";
        outResponse.subtitleText = ""; // Zero arcade text for biological breathing
        outResponse.durationSeconds = 3.0f;
        return true;
    }

    if (healthFraction < 0.35f) {
        outResponse.valid = true;
        outResponse.audioCueName = "player_biological_tachycardia_heartbeat";
        outResponse.subtitleText = "";
        outResponse.durationSeconds = 4.0f;
        return true;
    }

    if (directorIntensity > 0.85f) {
        outResponse.valid = true;
        outResponse.audioCueName = "player_biological_adrenaline_breath";
        outResponse.subtitleText = "";
        outResponse.durationSeconds = 2.5f;
        return true;
    }

    return false;
}

float ResponseSystem::getConceptCooldownRemaining(SpeechConcept speechConcept, uint64_t speakerId) const {
    uint64_t key = (speakerId << 8) | static_cast<uint64_t>(speechConcept);
    auto it = cooldownMap_.find(key);
    if (it == cooldownMap_.end()) return 0.0f;
    return std::max(0.0f, it->second - globalTime_);
}

} // namespace ze::ai
