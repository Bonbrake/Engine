#pragma once
#include <cstdint>
#include <functional>

namespace ui {

// Haptic feedback manager (M7)
struct HapticEvent {
    enum Type : uint8_t { Damage, Impact, Sprint, Footstep, WeaponFire, Explosion, Interaction };
    Type type;
    float intensity;   // 0..1
    float duration;    // seconds
};

class HapticManager {
public:
    HapticManager() = default;

    void trigger(const HapticEvent& ev);

    // Platform-specific haptic output
    using HapticOutputFn = std::function<void(float amplitude, float duration)>;
    void setOutputFn(HapticOutputFn fn) { outputFn_ = fn; }

    void update(float dt);

private:
    HapticOutputFn outputFn_;
    float currentAmplitude_ = 0.0f;
    float currentDuration_ = 0.0f;
};

} // namespace ui
