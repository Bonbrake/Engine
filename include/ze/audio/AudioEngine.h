#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <span>
#include <functional>

namespace audio {
namespace constants {
    constexpr float SOUND_SPEED = 343.0f; // m/s at 20°C
    constexpr float AIR_DENSITY = 1.2041f; // kg/m³ at 20°C
    constexpr uint32_t MAX_VOICES = 256;
    constexpr uint32_t VELVET_PULSE_DENSITY = 16; // pulses per octave
}

// Material-indexed acoustic properties
enum class AcousticMaterial : uint8_t {
    Concrete, Masonry, Wood, Metal, Glass,
    Carpet, Drywall, Foliage, Water, Earth, Snow, Flesh
};

// Describes a single sound source for the priority mix
struct SoundSource {
    glm::vec3 worldPos;
    glm::vec3 worldVelocity;
    float baseFrequency = 1.0f;    // 1.0 = natural
    float volumeBase = 1.0f;
    uint32_t sourceId = 0;
    glm::vec3 direction; // facing direction for cone
    float occlusion = 0.0f; // 0=clear, 1=fully occluded
    AcousticMaterial material = AcousticMaterial::Concrete;
};

// Output audio handle for tracking spatial mix state
struct AudioVoiceState {
    uint32_t voiceIndex;
    float priorityScore;
    float cadenceInterval;
    uint32_t lastTick;
};

// Material-indexed reverb properties
struct ReverbProfile {
    float tau = 0.5f;       // Decay time constant (seconds)
    float density = 0.06f;  // Velvet-noise pulse density (pulses/sample)
};

// Doppler shift (M6-EXT-01)
// f_effective = f_base * (c - v_source · u) / (c - v_observer · u)
inline float calcDopplerShift(float baseFreq, float speedSound,
                              glm::vec3 sourceVel, glm::vec3 observerVel,
                              glm::vec3 propagationDir) {
    float sourceTerm = speedSound - glm::dot(sourceVel, propagationDir);
    float observerTerm = speedSound - glm::dot(observerVel, propagationDir);
    if (observerTerm < 0.001f) observerTerm = 0.001f;
    return baseFreq * std::max(sourceTerm / observerTerm, 0.1f);
}

// Footstep SPL (M6-EXT-02)
// SPL_step = κ * m_total * ||v_foot||² * Z_surface
inline float calcFootstepSPL(float mass, float footVelocity, float surfaceImpedance) {
    constexpr float kappa = 0.02f;
    return kappa * mass * (footVelocity * footVelocity) * surfaceImpedance;
}

// Acoustic echo reflection (M6-EXT-03)
// I_echo = I_source * cos(θ) * R_material_hardness
inline float calcEchoIntensity(float sourceIntensity, float incidenceAngle,
                               float materialHardness) {
    return sourceIntensity * std::max(std::cos(incidenceAngle), 0.0f) * materialHardness;
}

// Portal attenuation (M6-EXT-04)
// Attenuation = aperture_area * dot(SoundVector, FaceNormal) / Distance²
inline float calcPortalAttenuation(float apertureArea, glm::vec3 soundDir,
                                   glm::vec3 faceNormal, float distance) {
    float dist2 = std::max(distance * distance, 0.01f);
    return apertureArea * std::max(glm::dot(soundDir, faceNormal), 0.0f) / dist2;
}

// Voice priority (M6-EXT-05)
// Priority = Volume_base / (d_distance² + ε) * cos(θ_viewAngle)
inline float calcVoicePriority(float volumeBase, float distance,
                               float viewAngleCosine) {
    constexpr float eps = 0.001f;
    return (volumeBase / ((distance * distance) + eps)) * viewAngleCosine;
}

// Audio cadence (M6-EXT-06)
// I_cadence = Clamp(floor(d_distance / 10.0), 1, 8)
inline uint32_t calcCadenceInterval(float distance) {
    uint32_t cadence = static_cast<uint32_t>(distance / 10.0f);
    if (cadence < 1) cadence = 1;
    if (cadence > 8) cadence = 8;
    return cadence;
}

// Velvet-noise pulse position (M6-EXT-07)
// t_k = (k + rand[0,1)) * T_avg
inline float calcVelvetPulse(uint32_t k, float avgSpacing, float randFrac) {
    return (static_cast<float>(k) + randFrac) * avgSpacing;
}

// Material-indexed reverb envelope
// EnvelopeDecay(t) = e^{-t / τ_material}
inline float calcReverbEnvelope(float time, float tau) {
    return std::exp(-time / tau);
}

// Core audio engine
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize(uint32_t sampleRate = 48000, uint32_t bufferFrames = 1024);
    void shutdown();
    bool isInitialized() const { return initialized_; }

    // Source management
    uint32_t registerSource(const SoundSource& src);
    void updateSource(uint32_t id, const SoundSource& src);
    void removeSource(uint32_t id);

    // Per-tick audio processing
    void tick(float dt, glm::vec3 listenerPos, glm::vec3 listenerVel,
              glm::quat listenerOrientation, uint64_t frameCount);

    // Output to miniaudio or callback
    void mixOutput(float* outputBuffer, uint32_t frames);

    // Voice state inspection
    std::span<const AudioVoiceState> activeVoices() const { return activeVoices_; }

    // Reverb profiles
    void setReverbProfile(AcousticMaterial material, const ReverbProfile& profile);
    ReverbProfile getReverbProfile(AcousticMaterial material) const;

    // Doppler enable
    void setDopplerEnabled(bool enabled) { dopplerEnabled_ = enabled; }
    bool isDopplerEnabled() const { return dopplerEnabled_; }

private:
    struct Voice {
        uint32_t sourceId;
        SoundSource source;
        float priority;
        uint32_t cadence;
        uint64_t lastTick;
        bool active = false;
        float distance = 0.0f;
        float reverbAccumulator[2] = {0.0f, 0.0f}; // stereo velvet-noise reverb tail
        uint32_t velvetPhase = 0;
    };

    bool initialized_ = false;
    uint32_t sampleRate_ = 48000;
    uint32_t bufferFrames_ = 1024;
    bool dopplerEnabled_ = true;

    std::vector<SoundSource> sources_;
    std::vector<Voice> voices_;
    std::vector<AudioVoiceState> activeVoices_;
    ReverbProfile reverbProfiles_[16];

    void updatePriority(Voice& v, glm::vec3 listenerPos, glm::vec3 listenerVel,
                        glm::quat listenerOrientation);
    void processVoice(Voice& v, float dt, uint64_t frameCount,
                      float* reverbBufferL, float* reverbBufferR,
                      uint32_t frames);
};

} // namespace audio
