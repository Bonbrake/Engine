#include "ze/audio/AudioEngine.h"
#include "ze/core/Logger.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace audio {

// Default reverb profiles indexed by AcousticMaterial
static constexpr ReverbProfile kDefaultProfiles[16] = {
    {0.8f, 0.04f},   // Concrete
    {0.7f, 0.05f},   // Masonry
    {1.2f, 0.03f},   // Wood
    {1.5f, 0.02f},   // Metal
    {0.5f, 0.06f},   // Glass
    {0.3f, 0.08f},   // Carpet
    {0.6f, 0.05f},   // Drywall
    {0.2f, 0.10f},   // Foliage
    {0.4f, 0.07f},   // Water
    {0.4f, 0.07f},   // Earth
    {0.2f, 0.10f},   // Snow
    {0.3f, 0.08f},   // Flesh
};

AudioEngine::AudioEngine() {
    for (int i = 0; i < 16; ++i) {
        reverbProfiles_[i] = kDefaultProfiles[i];
    }
    voices_.reserve(constants::MAX_VOICES);
    sources_.reserve(constants::MAX_VOICES);
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(uint32_t sampleRate, uint32_t bufferFrames) {
    sampleRate_ = sampleRate;
    bufferFrames_ = bufferFrames;
    initialized_ = true;
    // Miniaudio backend init deferred to integration step
    // TODO: plug into Miniaudio device callback
    return true;
}

void AudioEngine::shutdown() {
    sources_.clear();
    voices_.clear();
    activeVoices_.clear();
    initialized_ = false;
}

uint32_t AudioEngine::registerSource(const SoundSource& src) {
    uint32_t id = static_cast<uint32_t>(sources_.size());
    sources_.push_back(src);

    Voice v;
    v.sourceId = id;
    v.source = src;
    v.active = true;
    voices_.push_back(v);

    return id;
}

void AudioEngine::updateSource(uint32_t id, const SoundSource& src) {
    if (id < sources_.size()) {
        sources_[id] = src;
    }
    for (auto& v : voices_) {
        if (v.sourceId == id) {
            v.source = src;
            break;
        }
    }
}

void AudioEngine::removeSource(uint32_t id) {
    if (id < sources_.size()) {
        sources_[id] = SoundSource{};
        sources_[id].sourceId = id + 1; // invalidated
    }
    for (auto& v : voices_) {
        if (v.sourceId == id) {
            v.active = false;
            break;
        }
    }
}

void AudioEngine::updatePriority(Voice& v, glm::vec3 listenerPos,
                                  glm::vec3 listenerVel,
                                  glm::quat listenerOrientation) {
    glm::vec3 toListener = listenerPos - v.source.worldPos;
    float dist = glm::length(toListener);
    v.distance = dist;

    // View angle: dot of listener forward with direction to source
    glm::vec3 listenerForward = listenerOrientation * glm::vec3(0, 0, -1);
    glm::vec3 toSource = glm::normalize(toListener);
    float viewCos = std::max(glm::dot(listenerForward, toSource), 0.0f);

    v.priority = calcVoicePriority(v.source.volumeBase, dist, viewCos);
    v.cadence = calcCadenceInterval(dist);
}

void AudioEngine::processVoice(Voice& v, float dt, uint64_t frameCount,
                                float* reverbBufferL, float* reverbBufferR,
                                uint32_t frames) {
    // Cadence gate: skip processing on non-cadence ticks (M6-EXT-06)
    uint32_t cadence = v.cadence;
    if ((frameCount + v.sourceId) % cadence != 0) {
        return;
    }

    // Compute Doppler if enabled (M6-EXT-01)
    float effectiveFreq = 1.0f;
    if (dopplerEnabled_) {
        glm::vec3 toListener = glm::vec3(0, 0, -1) - v.source.worldPos;
        glm::vec3 dir = glm::normalize(toListener);
        effectiveFreq = calcDopplerShift(
            v.source.baseFrequency, constants::SOUND_SPEED,
            v.source.worldVelocity, glm::vec3(0), dir);
    }

    // Compute echo intensity with distance falloff (M6-EXT-03)
    float dist = v.distance;
    float sourceIntensity = v.source.volumeBase / (dist * dist + 0.01f);

    // Portal spread (M6-EXT-04)
    float portalFactor = 1.0f;
    if (v.source.occlusion > 0.0f) {
        portalFactor = (1.0f - v.source.occlusion * 0.5f);
    }

    // Characterize distance + material + velocity → SPL proxy (M6-EXT-02)
    AcousticMaterial mat = v.source.material;
    const ReverbProfile& prof = reverbProfiles_[static_cast<int>(mat)];

    // Velvet-noise late reverb tail accumulation (M6-EXT-07)
    if (prof.tau > 0.01f) {
        float avgSpacing = 1.0f / (prof.density * constants::SOUND_SPEED);
        for (uint32_t p = 0; p < constants::VELVET_PULSE_DENSITY; ++p) {
            float t = calcVelvetPulse(v.velvetPhase + p, avgSpacing,
                                      static_cast<float>(p) / constants::VELVET_PULSE_DENSITY);
            float env = calcReverbEnvelope(t, prof.tau);
            float gain = env * portalFactor * effectiveFreq;
            int sampleIdx = static_cast<int>(t * sampleRate_);
            if (sampleIdx < static_cast<int>(frames)) {
                reverbBufferL[sampleIdx] += gain * 0.5f;
                reverbBufferR[sampleIdx] += gain * 0.5f;
            }
        }
        v.velvetPhase += constants::VELVET_PULSE_DENSITY;
    }

    // Mark active for mixdown
    v.lastTick = frameCount;
}

void AudioEngine::tick(float dt, glm::vec3 listenerPos, glm::vec3 listenerVel,
                        glm::quat listenerOrientation, uint64_t frameCount) {
    activeVoices_.clear();

    for (auto& v : voices_) {
        if (!v.active) continue;

        updatePriority(v, listenerPos, listenerVel, listenerOrientation);

        AudioVoiceState state;
        state.voiceIndex = static_cast<uint32_t>(&v - voices_.data());
        state.priorityScore = v.priority;
        state.cadenceInterval = static_cast<float>(v.cadence);
        state.lastTick = static_cast<uint32_t>(v.lastTick);
        activeVoices_.push_back(state);
    }

    // Sort by priority descending
    std::sort(activeVoices_.begin(), activeVoices_.end(),
              [](const AudioVoiceState& a, const AudioVoiceState& b) {
                  return a.priorityScore > b.priorityScore;
              });

    // Cull lowest-priority voices if over budget
    if (activeVoices_.size() > constants::MAX_VOICES) {
        activeVoices_.resize(constants::MAX_VOICES);
    }
}

void AudioEngine::mixOutput(float* outputBuffer, uint32_t frames) {
    // Placeholder: sample-level mix with velvet-noise reverb
    // Full implementation will use Miniaudio mixer
    std::memset(outputBuffer, 0, frames * 2 * sizeof(float));

    // Per-voice mix
    for (const auto& state : activeVoices_) {
        if (state.voiceIndex < voices_.size()) {
            Voice& v = voices_[state.voiceIndex];
            processVoice(v, 0.0f, 0, outputBuffer, outputBuffer + 1, frames);
        }
    }
}

void AudioEngine::setReverbProfile(AcousticMaterial material, const ReverbProfile& profile) {
    int idx = static_cast<int>(material);
    if (idx >= 0 && idx < 16) {
        reverbProfiles_[idx] = profile;
    }
}

ReverbProfile AudioEngine::getReverbProfile(AcousticMaterial material) const {
    int idx = static_cast<int>(material);
    if (idx >= 0 && idx < 16) {
        return reverbProfiles_[idx];
    }
    return ReverbProfile{0.5f, 0.05f};
}

} // namespace audio
