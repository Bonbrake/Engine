#pragma once

#include "ze/ai/L4D2Director.h"
#include "ze/slm/SLMClient.h"
#include <memory>
#include <functional>
#include <string>

namespace ze::ai {

// Output narrative payload dispatched to audio/subtitles/HUD
struct DirectorNarrativeOutput {
    std::string category;     // "BARK", "RADIO", "CRESCENDO_OBJECTIVE", "LORE"
    std::string speaker;      // e.g. "Coach", "Ellis", "Station-Alpha"
    std::string text;         // The generated dialogue or transmission
    float intensityAtTrigger; // Intensity score when generated
    PacingPhase phase;        // Associated L4D2 phase
};

class L4D2SLMBridge {
public:
    L4D2SLMBridge(L4D2Director& director, slm::SLMClient& slmClient);
    ~L4D2SLMBridge();

    // Polled on Main Thread (e.g. at frame start) to process completed SLM results
    void update();

    // Register callback for when the neural SLM produces narrative output
    void setNarrativeListener(std::function<void(const DirectorNarrativeOutput&)> listener) {
        narrativeListener_ = std::move(listener);
    }

    // Direct trigger for contextual survivor dialogue based on L4D2 state
    void requestSurvivorBark(uint64_t survivorEntityId, const std::string& situationContext);

    // Direct trigger for emergency radio transmission during combat lull or crescendo
    void requestRadioBroadcast(const std::string& tacticalSituation);

private:
    void handleDirectorEvent(const DirectorEvent& evt);

    L4D2Director& director_;
    slm::SLMClient& slmClient_;
    std::function<void(const DirectorNarrativeOutput&)> narrativeListener_;
};

} // namespace ze::ai
