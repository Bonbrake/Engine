#include "ze/ai/L4D2SLMBridge.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace ze::ai {

L4D2SLMBridge::L4D2SLMBridge(L4D2Director& director, slm::SLMClient& slmClient)
    : director_(director), slmClient_(slmClient) {
    
    // Subscribe to L4D2 director events
    director_.setEventListener([this](const DirectorEvent& evt) {
        handleDirectorEvent(evt);
    });

    spdlog::info("L4D2SLMBridge initialized | Coupling L4D2 Pacing Engine with Neural SLM");
}

L4D2SLMBridge::~L4D2SLMBridge() {
    director_.setEventListener(nullptr);
}

void L4D2SLMBridge::update() {
    // Drain completed SLM inferences back on the main thread
    slmClient_.updateMainThread([this](uint64_t /*entityId*/, const std::string& text, bool success) {
        if (!success || text.empty()) return;

        DirectorNarrativeOutput out;
        out.phase = director_.getCurrentPhase();
        out.intensityAtTrigger = director_.getIntensity();

        // Parse structured JSON response from Qwen2.5-3B
        auto parsed = json::parse(text, nullptr, false);
        if (!parsed.is_discarded()) {
            if (parsed.contains("speaker") && parsed.contains("text")) {
                out.category = "BARK";
                out.speaker = parsed["speaker"].get<std::string>();
                out.text = parsed["text"].get<std::string>();
            } else if (parsed.contains("callsign") && parsed.contains("transmission")) {
                out.category = "RADIO";
                out.speaker = parsed["callsign"].get<std::string>();
                out.text = parsed["transmission"].get<std::string>();
            } else if (parsed.contains("content")) {
                out.category = "LORE";
                out.speaker = parsed.value("author", "Narrative");
                out.text = parsed["content"].get<std::string>();
            } else {
                out.category = "GENERIC";
                out.speaker = "Director";
                out.text = text;
            }
        } else {
            out.category = "RAW";
            out.speaker = "Survivor";
            out.text = text;
        }

        if (narrativeListener_) {
            narrativeListener_(out);
        }
    });
}

void L4D2SLMBridge::requestSurvivorBark(uint64_t survivorEntityId, const std::string& situationContext) {
    std::ostringstream ss;
    ss << "Survivor ID: " << survivorEntityId << " | "
       << "Phase: " << static_cast<int>(director_.getCurrentPhase()) << " | "
       << "Intensity: " << director_.getIntensity() << " | "
       << "Context: " << situationContext;

    // Direct fast bark (sub-250ms target)
    slmClient_.submitRequest(survivorEntityId, "DIALOGUE", ss.str(), slm::SLMMode::Fast);
}

void L4D2SLMBridge::requestRadioBroadcast(const std::string& tacticalSituation) {
    std::ostringstream ss;
    ss << "Emergency Radio Dispatch | "
       << "Phase: " << static_cast<int>(director_.getCurrentPhase()) << " | "
       << "Intensity: " << director_.getIntensity() << " | "
       << "Situation: " << tacticalSituation;

    // Radio transmissions can use Fast mode during crescendo or Think mode during relax
    slm::SLMMode mode = (director_.getCurrentPhase() == PacingPhase::Relax) 
        ? slm::SLMMode::Think 
        : slm::SLMMode::Fast;

    slmClient_.submitRequest(0, "RADIO", ss.str(), mode);
}

void L4D2SLMBridge::handleDirectorEvent(const DirectorEvent& evt) {
    switch (evt.type) {
    case DirectorEvent::Type::PhaseChanged:
        if (evt.newPhase == PacingPhase::SustainPeak) {
            requestRadioBroadcast("Military broadcast warning: Major infected horde incoming!");
        } else if (evt.newPhase == PacingPhase::Relax) {
            requestRadioBroadcast("Civil defense update: Sector clear, scavenge supplies before next wave.");
        }
        break;

    case DirectorEvent::Type::CrescendoTriggered:
        requestRadioBroadcast("ALERT: Alarm triggered! Defend the perimeter!");
        break;

    case DirectorEvent::Type::SurvivorDown:
        requestSurvivorBark(evt.subjectEntityId, "Survivor down! Requesting immediate rescue cover!");
        break;

    case DirectorEvent::Type::LoneWolfIsolated:
        requestSurvivorBark(evt.subjectEntityId, "Don't wander off alone! Regroup with the team!");
        break;

    default:
        break;
    }
}

} // namespace ze::ai
