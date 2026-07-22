#include "ze/ui/HapticManager.h"

namespace ui {

void HapticManager::trigger(const HapticEvent& ev) {
    currentAmplitude_ = std::min(1.0f, currentAmplitude_ + ev.intensity);
    currentDuration_ = std::max(currentDuration_, ev.duration);
}

void HapticManager::update(float dt) {
    if (currentDuration_ > 0.0f) {
        currentDuration_ -= dt;
        if (outputFn_) {
            outputFn_(currentAmplitude_, currentDuration_);
        }
        if (currentDuration_ <= 0.0f) {
            currentAmplitude_ = 0.0f;
        }
    }
}

} // namespace ui
