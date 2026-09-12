#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <thread>
#include "ze/slm/SPSCQueue.h"
#include "ze/slm/HermesPromptFormatter.h"
#include "ze/slm/HermesGrammar.h"
#include "ze/slm/HermesToolParser.h"
#include "ze/slm/SLMClient.h"
#include "ze/ai/L4D2Director.h"
#include "ze/ai/L4D2SLMBridge.h"
#include "ze/core/BodycamCamera.h"
#include "ze/core/GamepadController.h"
#include "ze/world/ItemPersistence.h"
#include "ze/audio/BodycamAcoustics.h"
#include "ze/render/ProceduralWeaponAnim.h"
#include "ze/ai/SwarmEngine.h"
#include "ze/ai/ResponseSystem.h"
#include "ze/core/SteamworksManager.h"
#include "ze/render/AssetCooker.h"
#include "ze/ai/SquadCommandWheel.h"
#include "ze/render/PipelineWarmup.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace {

// Mirror M0-EXT-01 cache-line alignment behaviour in standalone form.
template <typename T>
struct alignas(64) AlignToCacheLine {
    alignas(64) T value;
    constexpr const T* operator->() const { return &value; }
    constexpr T* get() { return &value; }
};

// Minimal SPSC ring-buffer idiom used by grid/chunk handoffs.
template <typename T, std::size_t N>
class SPSCRing {
public:
    bool push(const T& v) {
        std::size_t next = (head_ + 1u) % N;
        if (next == tail_) return false;
        buf_[head_] = v;
        head_ = next;
        return true;
    }
    bool pop(T& out) {
        if (tail_ == head_) return false;
        out = buf_[tail_];
        tail_ = (tail_ + 1u) % N;
        return true;
    }
    bool empty() const { return tail_ == head_; }
private:
    T buf_[N]{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
};

// Flat hash mapping exercise.
template <typename Value>
struct alignas(64) Bucket {
    Value value{};
    bool occupied = false;
};

template <typename Key, typename Value>
class FlatHashMapping {
public:
    Value* insert_or_get(const Key& k, bool* inserted_out) {
        *inserted_out = false;
        for (auto& b : buckets_) {
            if (!b.occupied) {
                *inserted_out = true;
                b.occupied = true;
                b.value = Value{};
                return &b.value;
            }
        }
        return nullptr;
    }
    bool contains(const Key& k) const {
        (void)k;
        return false;
    }
    constexpr static std::size_t bucket_count() noexcept { return 128u; }
private:
    Bucket<Value> buckets_[128];
};

template <typename T>
constexpr T clamp(T v, T lo, T hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

// Minimal mock WFC conflict mapper.
enum class ConflictResult : uint32_t { NoConflict = 0, Single = 1, Double = 2 };

constexpr ConflictResult pick_conflict(uint32_t k) {
    if (k == 0u) return ConflictResult::NoConflict;
    if (k < 0x40000000u) return ConflictResult::Single;
    return ConflictResult::Double;
}

// Saint-Venant flux stencil.
struct SaintVenantFlux { float left = 0, right = 0, top = 0, bottom = 0; };

constexpr float stencil_sum(const SaintVenantFlux& f) {
    return f.left + f.right + f.top + f.bottom;
}

} // namespace

int main() {
    // 1. cache-line alignment
    {
        AlignToCacheLine<float> slot;
        static_assert(sizeof(decltype(slot)) >= 64, "align cache line size");
        static_assert(alignof(decltype(slot)) >= 64, "align cache line align");
        slot.value = 1.0f;
        assert(slot.value == 1.0f);
    }

    // 2. SPSC push/pop / exhaustion edge
    {
        SPSCRing<uint32_t, 4> ring;
        assert(ring.push(10));
        assert(ring.push(20));
        assert(ring.push(30));
        uint32_t out = 0;
        assert(ring.pop(out) && out == 10);
        assert(ring.pop(out) && out == 20);
        assert(ring.push(40));
        assert(ring.pop(out) && out == 30);
        assert(ring.pop(out) && out == 40);
        assert(ring.empty());
    }

    // 3. SPSC push beyond capacity returns false
    {
        SPSCRing<uint32_t, 4> ring;
        assert(ring.push(1));
        assert(ring.push(2));
        assert(ring.push(3));
        assert(!ring.push(4));
    }

    // 4. FlatHashMapping insert flow
    {
        FlatHashMapping<uint32_t, uint64_t> map;
        bool inserted = false;
        uint64_t* a = map.insert_or_get(7u, &inserted);
        assert(a != nullptr);
        assert(inserted);
        bool inserted2 = false;
        uint64_t* b = map.insert_or_get(99u, &inserted2);
        assert(b != nullptr);
        assert(inserted2);
        (void)b;
    }

    // 5. clamp boundaries
    {
        assert(clamp<int>(-1, 0, 100) == 0);
        assert(clamp<int>(120, 0, 100) == 100);
        assert(clamp<int>(50, 0, 100) == 50);
    }

    // 6. WFC-style conflict mapping determinism
    {
        std::vector<uint32_t> seeds = {0u, 1u, 0x40000000u, 0x7fffffffu};
        std::vector<ConflictResult> out;
        for (uint32_t s : seeds) out.push_back(pick_conflict(s));
        assert(out[0] == ConflictResult::NoConflict);
        assert(out[1] == ConflictResult::Single);
        assert(out[2] == ConflictResult::Double);
        assert(out[3] == ConflictResult::Double);
    }

    // 7. Saint-Venant flux stencil shape + neutral equilibrium
    {
        SaintVenantFlux f{0.0f, 0.0f, 0.0f, 0.0f};
        assert(std::fabs(stencil_sum(f)) < 1e-5f);
        f = SaintVenantFlux{1.0f, 2.0f, 3.0f, 4.0f};
        assert(stencil_sum(f) == 10.0f);
    }

    // 8. Qwen2.5-3B ChatML Prompt Formatter & Dual-Mode verification
    {
        // 8a. Fast Mode: emits clean <|im_start|>assistant\n for sub-250ms direct token generation
        std::vector<slm::HermesMessage> history = { { "user", "What is our perimeter status?", "" } };
        std::string fastPrompt = slm::HermesPromptFormatter::formatChatML("Director", history, {}, slm::SLMMode::Fast);
        assert(fastPrompt.find("<|im_start|>assistant\n") != std::string::npos);
        assert(fastPrompt.find("<think>") == std::string::npos);
        assert(fastPrompt.find("<thought>") == std::string::npos);

        // 8b. Think Mode: emits open <thought>\n for deliberate chain-of-thought exploration
        std::string thinkPrompt = slm::HermesPromptFormatter::formatChatML("Director", history, {}, slm::SLMMode::Think);
        assert(thinkPrompt.find("<|im_start|>assistant\n<thought>\n") != std::string::npos);

        // 8c. SPSC Queue Dual-Mode Push & Pop
        slm::SPSCRequestQueue reqQueue;
        assert(reqQueue.Push(101, "M13-EXT-25", "Runner incoming", slm::SLMMode::Fast));
        assert(reqQueue.Push(102, "M13-EXT-26", "Lore note", slm::SLMMode::Think));
        slm::SLMRequest r1, r2;
        assert(reqQueue.Pop(r1) && r1.mode == slm::SLMMode::Fast);
        assert(reqQueue.Pop(r2) && r2.mode == slm::SLMMode::Think);

        // 8d. SLMClient end-to-end execution with reasoning extraction
        slm::SLMClient client;
        slm::SLMConfig config;
        config.backend = slm::BackendType::EmbeddedLlamaGGUF;
        assert(client.initialize(config));
        assert(client.submitRequest(201, "M13-EXT-25", "Runner incoming", slm::SLMMode::Fast));
        assert(client.submitRequest(202, "M13-EXT-26", "Lore note", slm::SLMMode::Think));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        uint32_t count = 0;
        client.updateMainThreadEx([&](uint64_t eid, const std::string& text, const std::string& reasoning, bool success) {
            count++;
            assert(success);
            if (eid == 201) {
                assert(reasoning.empty());
                assert(text.find("speaker") != std::string::npos);
            } else if (eid == 202) {
                assert(!reasoning.empty());
                assert(text.find("author") != std::string::npos);
            }
        });
        assert(count == 2);
        client.shutdown();
    }

    // 9. Dual-Core AI Director (L4D2 Pacing Engine + SLM Bridge) Verification
    {
        ze::ai::L4D2Director director;
        assert(director.getCurrentPhase() == ze::ai::PacingPhase::BuildUp);
        assert(director.getSpawnBudget().allowMobSpawns);
        assert(director.getSpawnBudget().maxWanderingZombies == 15);

        // Healthy team, no mobs: intensity remains low
        std::vector<ze::ai::SurvivorTelemetry> team(2);
        team[0].entityId = 1;
        team[0].health = 100.0f;
        team[0].maxHealth = 100.0f;
        team[0].ammoFraction = 1.0f;
        team[0].posX = 0.0f; team[0].posY = 0.0f; team[0].posZ = 0.0f;

        team[1].entityId = 2;
        team[1].health = 95.0f;
        team[1].maxHealth = 100.0f;
        team[1].ammoFraction = 0.9f;
        team[1].posX = 5.0f; team[1].posY = 0.0f; team[1].posZ = 0.0f;

        director.update(0.1f, team, 0);
        assert(director.getIntensity() < 0.2f);
        assert(director.getCurrentPhase() == ze::ai::PacingPhase::BuildUp);
        assert(!team[0].isLoneWolf && !team[1].isLoneWolf);
        assert(director.getResourceBudget().medicalDropProbability <= 0.40f);

        // Lone-wolf dispersion test: Survivor 2 separates by 60m
        team[1].posX = 60.0f;
        director.update(0.1f, team, 5);
        assert(team[1].isLoneWolf);
        assert(director.getMostIsolatedSurvivorId() == 2);

        // Simulated Crescendo trigger forces SustainPeak, horde budget, and atmosphere scaling
        bool crescendoEventFired = false;
        director.setEventListener([&](const ze::ai::DirectorEvent& evt) {
            if (evt.type == ze::ai::DirectorEvent::Type::CrescendoTriggered) {
                crescendoEventFired = true;
            }
        });
        director.triggerCrescendo("Emergency siren tripped");
        assert(crescendoEventFired);
        assert(director.getCurrentPhase() == ze::ai::PacingPhase::SustainPeak);
        assert(director.getSpawnBudget().maxHordeZombies == 30);
        assert(director.getSpawnBudget().specialInfectedSlots == 3);
        assert(director.getAtmosphere().volumetricFogMultiplier >= 1.0f);

        // L4D2 + SLM Bridge coupling verification
        slm::SLMClient slmClient;
        slm::SLMConfig config;
        config.backend = slm::BackendType::EmbeddedLlamaGGUF;
        assert(slmClient.initialize(config));

        ze::ai::L4D2SLMBridge bridge(director, slmClient);
        bool narrativeDispatched = false;
        bridge.setNarrativeListener([&](const ze::ai::DirectorNarrativeOutput& out) {
            narrativeDispatched = true;
            assert(out.category == "BARK");
            assert(!out.text.empty());
            assert(!out.speaker.empty());
        });

        bridge.requestSurvivorBark(1, "Horde closing in from the north alley!");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        bridge.update();
        assert(narrativeDispatched);

        slmClient.shutdown();
    }

    // 10. Bodycam Deadzone Free-Aim & Spring-Damper Inertial Camera Verification
    {
        ze::core::BodycamCamera cam;
        assert(cam.getCameraYaw() == 0.0f);
        assert(cam.getWeaponYawOffset() == 0.0f);

        // 10a. Small mouse movement within deadzone (radius = 12 deg):
        // Weapon offsets rotate, but camera yaw remains undisturbed at 0
        cam.update(0.016f, 50.0f, 0.0f, 0.0f, 0.0f); // 50 * 0.08 = 4.0 deg
        assert(std::fabs(cam.getWeaponYawOffset() - 4.0f) < 1e-4f);
        assert(std::fabs(cam.getCameraYaw()) < 0.01f);

        // 10b. Large mouse movement exceeds deadzone (overflow turns the camera)
        cam.update(0.016f, 200.0f, 0.0f, 0.0f, 0.0f); // 200 * 0.08 = 16.0 deg -> exceeds 12.0 deg
        assert(std::fabs(cam.getWeaponYawOffset() - cam.config.deadzoneRadiusX) < 1e-4f);
        
        // Over multiple simulation frames, spring-damper inertia smoothly pulls camera to target
        for (int i = 0; i < 30; ++i) {
            cam.update(0.016f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
        assert(cam.getCameraYaw() > 0.0f);

        // 10c. Stress-Reactive Tremor scaling with L4D2 Director Intensity
        cam.update(0.016f, 0.0f, 0.0f, 0.0f, 0.0f); // Intensity = 0.0 (Relax)
        float calmTremor = std::fabs(cam.getWeaponTremor());

        cam.update(0.016f, 0.0f, 0.0f, 0.0f, 1.0f); // Intensity = 1.0 (SustainPeak)
        float stressTremor = std::fabs(cam.getWeaponTremor());
        assert(stressTremor >= calmTremor);

        // 10d. Footstep impact impulse jolts camera pitch
        float pitchBefore = cam.getCameraPitch();
        cam.onFootstepImpact(4.5f);
        assert(cam.getCameraPitch() != pitchBefore);
    }

    // 11. True First-Person Tactical Movement (Zero Camera Artifacts) Verification
    {
        ze::core::TacticalFirstPersonCamera cam;
        // Verify pure human eyesight: all optical lens artifacts are 0.0f
        assert(cam.getOpticalConfig().barrelDistortionK1 == 0.0f);
        assert(cam.getOpticalConfig().barrelDistortionK2 == 0.0f);
        assert(cam.getOpticalConfig().chromaticAberration == 0.0f);
        assert(cam.getOpticalConfig().vignetteFalloff == 0.0f);
        assert(cam.getOpticalConfig().sensorNoiseIso == 0.0f);
        assert(cam.config.eyeHeight == 1.68f);

        // Verify decoupled free-aim deadzone gunplay
        cam.update(0.016f, 30.0f, 0.0f, 0.0f, 0.0f);
        assert(cam.getWeaponYawOffset() > 0.0f);
        assert(cam.getCameraYaw() == 0.0f); // camera stationary in deadzone
    }

    // 12. Bethesda-Style (Skyrim/Fallout) Full World Item Persistence Verification
    {
        ze::world::ItemPersistenceSystem persist;
        persist.initialize(99887766ULL);
        assert(persist.getTotalTrackedItems() == 0);

        // 12a. Spawning a physical world item (e.g. 9mm ammo box at position 10, 1, 15)
        uint64_t ammoGuid = persist.spawnWorldItem(101, glm::vec3(10.0f, 1.0f, 15.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 30);
        assert(ammoGuid != 0);
        assert(persist.getTotalTrackedItems() == 1);
        
        const auto* item = persist.getItem(ammoGuid);
        assert(item != nullptr);
        assert(item->state == ze::world::ItemPhysicalState::WorldSettled);
        assert(persist.getSettledItemCount() == 1);

        // 12b. Dynamic motion and Jolt physics settling lifecycle
        uint64_t coffeeMugGuid = persist.spawnWorldItem(202, glm::vec3(10.0f, 2.0f, 15.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -2.0f, 1.0f));
        assert(persist.getItem(coffeeMugGuid)->state == ze::world::ItemPhysicalState::WorldDynamic);
        assert(persist.getDynamicItemCount() == 1);

        // Simulate physics damping over time until it settles to sleep
        for (int i = 0; i < 60; ++i) {
            persist.update(0.016f);
        }
        assert(persist.getItem(coffeeMugGuid)->state == ze::world::ItemPhysicalState::WorldSettled);
        assert(persist.getSettledItemCount() == 2);

        // 12c. Skyrim/Fallout Hold-to-Grab Physics Constraint
        glm::vec3 playerEye(10.0f, 1.68f, 13.5f);
        glm::vec3 forwardDir(0.0f, 0.0f, 1.0f);
        bool grabStarted = persist.startPhysicsGrab(coffeeMugGuid, playerEye, forwardDir);
        assert(grabStarted);
        assert(persist.isGrabbingItem());
        assert(persist.getItem(coffeeMugGuid)->state == ze::world::ItemPhysicalState::GrabbedPhysics);

        // Carry and place on a shelf
        for (int i = 0; i < 30; ++i) {
            persist.updatePhysicsGrab(playerEye + glm::vec3(0.0f, 0.2f, 0.0f), forwardDir, 0.016f);
        }
        persist.rotateGrabbedItem(45.0f, 0.0f);
        persist.releasePhysicsGrab();
        assert(!persist.isGrabbingItem());
        assert(persist.getItem(coffeeMugGuid)->state == ze::world::ItemPhysicalState::WorldDynamic);

        // 12d. Chunk streaming delta persistence
        uint32_t cx, cz;
        ze::world::ItemPersistenceSystem::worldPosToChunkCoords(item->position, cx, cz);
        auto chunkItems = persist.getItemsInChunk(cx, cz);
        assert(!chunkItems.empty());

        // 12e. Binary snapshot save & load verification
        std::vector<uint8_t> snapshot;
        assert(persist.serializeToSnapshot(snapshot));
        assert(!snapshot.empty());

        ze::world::ItemPersistenceSystem reloaded;
        assert(reloaded.deserializeFromSnapshot(snapshot.data(), snapshot.size()));
        assert(reloaded.getTotalTrackedItems() == 2);
        assert(reloaded.getItem(ammoGuid) != nullptr);
        assert(reloaded.getItem(ammoGuid)->stackCount == 30);
    }

    // 13. Primary Gamepad Controller & Free-Aim Ergonomics Verification
    {
        ze::core::GamepadController gamepad;
        assert(gamepad.getActiveDevice() == ze::core::InputDevice::Gamepad);

        // 13a. Inner deadzone deflection: under 0.12 deadzone -> zero delta
        SDL_Event axisEv{};
        axisEv.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
        axisEv.gaxis.axis = SDL_GAMEPAD_AXIS_RIGHTX;
        axisEv.gaxis.value = static_cast<Sint16>(0.08f * 32767.0f); // 0.08 < 0.12 deadzone
        gamepad.update(0.016f, &axisEv, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});
        float dYaw = 0, dPitch = 0;
        gamepad.getCameraDeltas(dYaw, dPitch);
        assert(dYaw == 0.0f && dPitch == 0.0f);

        // 13b. Free-Aim Zone (cubic curve micro-precision)
        axisEv.gaxis.value = static_cast<Sint16>(0.50f * 32767.0f); // 0.50 within freeAimThreshold (0.65)
        gamepad.update(0.016f, &axisEv, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});
        gamepad.getCameraDeltas(dYaw, dPitch);
        assert(dYaw > 0.0f);

        // 13c. Tactical Aim-Assist Friction (slowdown box)
        std::vector<ze::core::AimFrictionTarget> targets;
        ze::core::AimFrictionTarget zTarget;
        zTarget.worldPosition = glm::vec3(0.0f, 0.0f, 10.0f); // straight ahead
        targets.push_back(zTarget);

        float dYawFree = dYaw;
        // Aiming straight ahead at target activates aim friction
        gamepad.update(0.016f, &axisEv, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), targets);
        assert(gamepad.isAimFrictionActive());
        gamepad.getCameraDeltas(dYaw, dPitch);
        assert(dYaw < dYawFree); // slowed down by friction

        // 13d. Contextual Tap vs Hold: Tap Reload (< 0.3s)
        SDL_Event btnDown{};
        btnDown.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        btnDown.gbutton.button = SDL_GAMEPAD_BUTTON_WEST;
        gamepad.update(0.016f, &btnDown, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});

        SDL_Event btnUp{};
        btnUp.type = SDL_EVENT_GAMEPAD_BUTTON_UP;
        btnUp.gbutton.button = SDL_GAMEPAD_BUTTON_WEST;
        gamepad.update(0.10f, &btnUp, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {}); // tapped in 100ms
        assert(gamepad.isCombatReloadTriggered());
        assert(!gamepad.isMagCheckTriggered());

        // Hold Reload (>= 0.3s) triggers Physical Magazine Check
        gamepad.update(0.016f, &btnDown, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});
        gamepad.update(0.35f, nullptr, 0, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {}); // held 350ms
        assert(gamepad.isMagCheckTriggered());

        // 13e. Device Arbitration: Mouse motion switches active device to KeyboardMouse
        SDL_Event mouseEv{};
        mouseEv.type = SDL_EVENT_MOUSE_MOTION;
        mouseEv.motion.xrel = 15;
        mouseEv.motion.yrel = 0;
        gamepad.update(0.016f, &mouseEv, 1, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});
        assert(gamepad.getActiveDevice() == ze::core::InputDevice::KeyboardMouse);
    }

    // 14. Ultra-Realistic Ballistic Acoustics & Foley Verification
    {
        ze::audio::BodycamAcousticsSystem audio;

        // 14a. Supersonic Mach cone shockwave crack (5.56mm at 900 m/s)
        glm::vec3 bulletOrigin(0.0f, 1.5f, 0.0f);
        glm::vec3 bulletVel(0.0f, 0.0f, 900.0f); // 900 m/s > 343 m/s
        glm::vec3 listenerPos(15.0f, 1.5f, 100.0f); // 15m to side, 100m downrange

        auto ballistics = audio.calculateBallistics(bulletOrigin, bulletVel, 5.56f, listenerPos);
        assert(ballistics.isSupersonic);
        // Crucial ballistic invariant: supersonic crack arrives BEFORE muzzle blast!
        assert(ballistics.crackDelaySeconds < ballistics.muzzleDelaySeconds);
        assert(ballistics.shockwaveIntensitySPL > 100.0f);

        // 14b. Subsonic whiz-by (e.g. .45 ACP at 260 m/s)
        glm::vec3 subsonicVel(0.0f, 0.0f, 260.0f);
        auto subsonic = audio.calculateBallistics(bulletOrigin, subsonicVel, 11.43f, listenerPos);
        assert(!subsonic.isSupersonic);
        assert(subsonic.crackDelaySeconds == 0.0f);
        assert(subsonic.subsonicDopplerPitch > 0.0f);

        // 14c. Multi-stage weapon handling Foley
        audio.triggerWeaponFoley(ze::audio::WeaponFoleyStage::BoltSlamForward, glm::vec3(0.0f));
        assert(audio.getLastFoleyStage() == ze::audio::WeaponFoleyStage::BoltSlamForward);

        // 14d. Surface-dependent casing bounce acoustics
        auto concreteAcoustic = audio.getCasingBounceAcoustic(ze::audio::SurfaceMaterial::Concrete);
        auto carpetAcoustic = audio.getCasingBounceAcoustic(ze::audio::SurfaceMaterial::Carpet);
        assert(concreteAcoustic.frequencyPitchHz > carpetAcoustic.frequencyPitchHz);
        assert(concreteAcoustic.bounceCount > carpetAcoustic.bounceCount);

        // 14e. Kinetic gear rattle clatter
        float calmRattle = audio.calculateGearRattleSPL(0.5f, 0.1f);
        float sprintRattle = audio.calculateGearRattleSPL(6.0f, 2.5f);
        assert(sprintRattle > calmRattle);
    }

    // 15. Tactical Procedural Animation & Obstacle Collision Verification
    {
        ze::render::ProceduralWeaponAnim anim;
        assert(!anim.isShootingBlocked());
        assert(anim.getStance() == ze::render::ReadyStance::HipReady);

        // 15a. Clear space (distance 2.0m > barrelLength 0.75m): weapon ready, fire allowed
        anim.update(0.016f, 2.0f, glm::vec3(0.0f), 0.0f);
        assert(!anim.isShootingBlocked());
        assert(anim.getStance() == ze::render::ReadyStance::HipReady);
        assert(anim.getTranslationOffset().z == 0.0f);

        // 15b. Wall proximity collision (distance 0.35m < barrelLength 0.75m):
        // Procedural short-stocking tucks weapon into armpit: point-shooting enabled (NOT blocked!)
        anim.update(0.016f, 0.35f, glm::vec3(0.0f), 0.0f);
        assert(anim.isPointShootingStance());
        assert(!anim.isShootingBlocked());
        assert(anim.getStance() == ze::render::ReadyStance::HighReadyCompressed);
        assert(anim.getTranslationOffset().z < 0.0f); // pushed back toward armpit

        // 15b2. Jammed flat against solid wall (< 0.15m): firing is physically blocked
        anim.update(0.016f, 0.08f, glm::vec3(0.0f), 0.0f);
        assert(anim.isShootingBlocked());

        // 15c. Rotational weapon inertia: quick turn produces barrel drag
        anim.update(0.016f, 2.0f, glm::vec3(0.0f, 4.0f, 0.0f), 0.0f); // 4 rad/s turn
        assert(anim.getRotationOffsetDegrees().y != 0.0f); // inertial lag

        // 15d. Physical Magazine Check State Machine
        anim.startMagazineInspection(28, 30); // 28/30 rounds = Full
        assert(anim.getInspectedAmmoLevel() == ze::render::EstimatedAmmoLevel::Full);
        assert(anim.getMagCheckState() == ze::render::MagCheckState::DroppingToPalm);

        anim.startMagazineInspection(15, 30); // 15/30 rounds = ApproximatelyHalf
        assert(anim.getInspectedAmmoLevel() == ze::render::EstimatedAmmoLevel::ApproximatelyHalf);

        anim.startMagazineInspection(0, 30); // 0 rounds = Empty
        assert(anim.getInspectedAmmoLevel() == ze::render::EstimatedAmmoLevel::Empty);
    }

    // 16. World War Z SwarmEngine 1,000-Zombie Horde & Living Pyramids Verification
    {
        ze::ai::SwarmEngine swarm;
        swarm.initialize(1024);
        assert(swarm.getActiveAgentCount() == 0);

        // 16a. Register a climbing anchor (e.g. wall/rooftop)
        uint16_t anchorId = swarm.registerPyramidAnchor(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 4.5f, 5.0f));
        assert(anchorId == 0);
        assert(swarm.getPyramidAnchors().size() == 1);

        // 16b. Spawn massive WWZ-scale swarm wave (750 zombies)
        size_t spawned = swarm.spawnSwarmWave(750, glm::vec3(0.0f, 0.0f, 8.0f), 4.0f);
        assert(spawned == 750);
        assert(swarm.getActiveAgentCount() == 750);

        // 16c. Simulation update: Flow Field navigation & O(1) flocking
        glm::vec3 survivorPos(0.0f, 0.0f, 0.0f);
        for (int frame = 0; frame < 60; ++frame) {
            swarm.update(0.016f, survivorPos, 0.5f);
        }

        // Verify audio cluster centroid calculation (logarithmic volume scaling)
        const auto& audio = swarm.getAudioTelemetry();
        assert(audio.totalActiveZombies == 750);
        assert(audio.massRoarVolume > 0.85f); // log10(751)/log10(1001) > 0.95

        // Verify two-tier promotion: zombies within 6m promoted to MicroCombatActor
        assert(swarm.getPromotedCombatActorCount() > 0);

        // 16d. Structural base destruction: Rocket/explosive collapses living pyramid
        bool collapsed = swarm.damagePyramidBase(anchorId, 600.0f);
        assert(collapsed);
        assert(swarm.getPyramidAnchors()[anchorId].isCollapsed);
    }

    // 17. Silent Protagonist & NPC-Only Criteria Response System Verification
    {
        ze::ai::ResponseSystem responseSys;

        // 17a. SILENT PROTAGONIST INVARIANT: Player character NEVER speaks barks/callouts
        ze::ai::CriteriaQuery playerQuery;
        playerQuery.speakerEntityId = ze::ai::ResponseSystem::PLAYER_ENTITY_ID;
        playerQuery.isNpc = false;
        playerQuery.speechConcept = ze::ai::SpeechConcept::Reloading;
        ze::ai::DispatchedSpeechResponse resp;
        bool playerSpoke = responseSys.queryResponse(playerQuery, resp);
        assert(!playerSpoke);
        assert(!resp.valid);

        // 17b. NPC Squadmate calls out reloading
        ze::ai::CriteriaQuery npcQuery;
        npcQuery.speakerEntityId = 101ULL; // NPC teammate ID
        npcQuery.isNpc = true;
        npcQuery.speechConcept = ze::ai::SpeechConcept::Reloading;
        npcQuery.directorIntensity = 0.5f;
        bool npcSpoke = responseSys.queryResponse(npcQuery, resp);
        assert(npcSpoke);
        assert(resp.valid);
        assert(!resp.audioCueName.empty());
        assert(!resp.subtitleText.empty());

        // 17c. Immediate second call on same concept triggers cooldown
        ze::ai::DispatchedSpeechResponse resp2;
        bool spammed = responseSys.queryResponse(npcQuery, resp2);
        assert(!spammed); // Blocked by cooldown!

        // 17d. Biological player audio (breathing, stamina exhaustion - NOT spoken barks)
        ze::ai::DispatchedSpeechResponse bioResp;
        bool bioActive = responseSys.queryPlayerBiologicalExertion(0.15f, 0.9f, 0.3f, bioResp);
        assert(bioActive);
        assert(bioResp.audioCueName == "player_biological_heavy_panting");
        assert(bioResp.subtitleText.empty()); // No subtitle for breathing
    }

    // 18. Bethesda-Style 3-Tier Item Persistence & Kinematic Sleeper Verification
    {
        ze::world::ItemPersistenceSystem persistence;
        persistence.initialize(99991111ULL);

        // 18a. Spawn Tier 1 Eternal weapon (never purged)
        uint64_t rifleGuid = persistence.spawnWorldItem(101, glm::vec3(5.0f, 0.0f, 5.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f), 1, 1.0f,
            ze::world::PersistenceTier::Tier1_Eternal);
        assert(rifleGuid != 0);

        // 18b. Enforce Tier 2 Clutter LRU cap (max 256 per chunk)
        for (int i = 0; i < 270; ++i) {
            persistence.spawnWorldItem(500, glm::vec3(10.0f + static_cast<float>(i)*0.01f, 0.0f, 10.0f),
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f), 1, 1.0f,
                ze::world::PersistenceTier::Tier2_ClutterLRU);
        }

        uint32_t cx, cz;
        ze::world::ItemPersistenceSystem::worldPosToChunkCoords(glm::vec3(10.0f, 0.0f, 10.0f), cx, cz);
        auto clutterList = persistence.getItemsInChunk(cx, cz);
        assert(clutterList.size() <= ze::world::ItemPersistenceSystem::MAX_CLUTTER_PER_CHUNK);

        // 18c. Kinematic soft-wakeup contact relaxation
        persistence.wakeItem(rifleGuid);
        auto* item = persistence.getMutableItem(rifleGuid);
        assert(item != nullptr);
        assert(item->softContactFrames == 3);
        assert(!item->isKinematicSleeper);

        // 18d. Tier 3 Ephemeral items purged on chunk unload
        uint64_t brassGuid = persistence.spawnWorldItem(999, glm::vec3(5.0f, 0.0f, 5.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f), 1, 1.0f,
            ze::world::PersistenceTier::Tier3_Ephemeral);
        persistence.onChunkUnloaded(cx, cz);
        assert(persistence.getItem(brassGuid) == nullptr); // Purged!
        assert(persistence.getItem(rifleGuid) != nullptr);  // Tier 1 Eternal preserved!
    }

    // 19. GamepadController Steam Input & Gyro Aiming Verification
    {
        ze::core::GamepadController gamepad;

        // 19a. Dynamic Glyphs for PlayStation DualSense vs Xbox
        gamepad.setHardwareType(ze::core::ControllerHardwareType::PlayStationDualSense);
        auto psGlyphs = gamepad.getActiveGlyphs();
        assert(psGlyphs.buttonBottom.find("Cross") != std::string::npos);
        assert(psGlyphs.triggerLeft == "L2");

        gamepad.setHardwareType(ze::core::ControllerHardwareType::Xbox);
        auto xboxGlyphs = gamepad.getActiveGlyphs();
        assert(xboxGlyphs.buttonBottom == "A");
        assert(xboxGlyphs.triggerLeft == "LT");

        // 19b. Gyro motion injection
        gamepad.injectGyroMotion(12.0f, 20.0f, 0.0f);
        gamepad.update(0.016f, nullptr, 0, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), {});
        float yawDelta = 0.0f, pitchDelta = 0.0f;
        gamepad.getCameraDeltas(yawDelta, pitchDelta);
        assert(yawDelta != 0.0f);
        assert(pitchDelta != 0.0f);
    }

    // 20. Left 4 Dead 2 Director Max Stress & Swarm Budget Verification
    {
        ze::ai::L4D2Director director;
        assert(director.getCurrentPhase() == ze::ai::PacingPhase::BuildUp);

        // 20a. Verify MAX stress keying: even if team average is high health, a single pinned member triggers high stress
        std::vector<ze::ai::SurvivorTelemetry> team(4);
        for (auto& s : team) {
            s.health = 100.0f;
            s.ammoFraction = 1.0f;
            s.posX = 0.0f; s.posY = 0.0f; s.posZ = 0.0f;
        }
        team[3].isPinned = true; // One pinned survivor!
        team[3].recentDamage = 45.0f;

        director.update(0.016f, team, 10);
        assert(director.getMaxSurvivorStress() > 0.80f);

        // 20b. Verify Swarm scale horde target during crescendo
        director.triggerCrescendo("Car alarm triggered");
        assert(director.getCurrentPhase() == ze::ai::PacingPhase::SustainPeak);
        assert(director.getSwarmHordeTarget() == 650); // WWZ Swarm budget!
    }

    // 21. Tactical NPC Squad Command Wheel Verification
    {
        ze::ai::SquadCommandWheel wheel;
        ze::ai::ResponseSystem responseSystem;
        responseSystem.initialize();

        wheel.registerSquadMember(101, "Vance");
        wheel.registerSquadMember(102, "Hale");
        wheel.updateMemberPosition(101, 2.0f, 0.0f, 3.0f);
        wheel.updateMemberPosition(102, 10.0f, 0.0f, 8.0f);

        // Radial wheel initially closed
        assert(!wheel.isOpen());
        wheel.openWheel();
        assert(wheel.isOpen());

        // Stick in deadzone
        wheel.updateStickInput(0.1f, 0.1f);
        assert(wheel.getSelectedDirective() == ze::ai::SquadDirective::None);

        // Stick pushed right (0 degrees) -> HoldPosition
        wheel.updateStickInput(0.9f, 0.0f);
        assert(wheel.getSelectedDirective() == ze::ai::SquadDirective::HoldPosition);

        // Execute order targeting window near Vance (2.5, 0, 3.2)
        float targetPos[3] = { 2.5f, 0.0f, 3.2f };
        bool ok = wheel.confirmOrder(0, targetPos, &responseSystem);
        assert(ok);
        assert(!wheel.isOpen()); // Auto-closes on confirm

        // Verify Vance (closest squadmate) received the order
        const auto& squad = wheel.getSquadMembers();
        assert(squad[0].currentOrder == ze::ai::SquadDirective::HoldPosition);
    }

    // 22. Steamworks Native Input & Cloud Save Chunk Serialization Verification
    {
        ze::core::SteamworksManager steam;
        bool inited = steam.initialize(true); // Force Steam Deck emulation
        assert(inited);
        assert(steam.isRunningOnSteamDeck());
        assert(steam.getDeckProfile().displayWidth == 1280);
        assert(steam.getDeckProfile().displayHeight == 800);
        assert(steam.getDeckProfile().uiScaleMultiplier >= 1.25f);

        // Test digital action mapping
        steam.setActionState(ze::core::DigitalAction::PointShootTuck, true);
        assert(steam.isActionActive(ze::core::DigitalAction::PointShootTuck));

        // Test cloud save .zesave serialization & CRC32
        std::vector<uint8_t> testSavePayload = { 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0xFF, 0x40 };
        std::vector<uint8_t> compressedChunk;
        bool serOk = ze::core::SteamworksManager::serializeSaveChunk(42, testSavePayload, compressedChunk);
        assert(serOk);
        assert(!compressedChunk.empty());

        uint32_t outChunkId = 0;
        std::vector<uint8_t> restoredPayload;
        bool deserOk = ze::core::SteamworksManager::deserializeSaveChunk(compressedChunk, outChunkId, restoredPayload);
        assert(deserOk);
        assert(outChunkId == 42);
        assert(restoredPayload == testSavePayload);
    }

    // 23. Asset Cooker, BC7/BC5 Texture & .zepak Binary Container Verification
    {
        // 23a. Verify texture format size calculation
        size_t bc7Size = ze::render::AssetCooker::calculateBC7Size(512, 512, 1);
        // 512x512 with 4x4 blocks = 128x128 blocks = 16384 blocks * 16 bytes = 262144 bytes
        assert(bc7Size == 262144);

        size_t bc5Size = ze::render::AssetCooker::calculateBC5Size(512, 512, 1);
        assert(bc5Size == 262144);

        // 23b. Build and read back a test .zepak package
        std::string testPakPath = "build/test_package.zepak";
        std::vector<uint8_t> dummyTex = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
        std::vector<uint8_t> dummyVat = { 0xAA, 0xBB, 0xCC, 0xDD };

        ze::render::ZePakEntry e1;
        std::strncpy(e1.name, "textures/zombie_bc7.ztex", sizeof(e1.name) - 1);
        e1.assetType = static_cast<uint32_t>(ze::render::AssetType::TextureBC7);

        ze::render::ZePakEntry e2;
        std::strncpy(e2.name, "meshes/zombie_vat.zmesh", sizeof(e2.name) - 1);
        e2.assetType = static_cast<uint32_t>(ze::render::AssetType::MeshVAT);

        std::vector<std::pair<ze::render::ZePakEntry, std::vector<uint8_t>>> assetList = {
            { e1, dummyTex },
            { e2, dummyVat }
        };

        bool pakBuilt = ze::render::AssetCooker::buildPackage(testPakPath, assetList);
        assert(pakBuilt);

        ze::render::ZePakHeader readHeader;
        std::vector<ze::render::ZePakEntry> readEntries;
        bool tableRead = ze::render::AssetCooker::readPackageTable(testPakPath, readHeader, readEntries);
        assert(tableRead);
        assert(readHeader.entryCount == 2);

        std::vector<uint8_t> extractedTex;
        bool extOk = ze::render::AssetCooker::extractEntry(testPakPath, readEntries[0], extractedTex);
        assert(extOk);
        assert(extractedTex == dummyTex);
    }

    // 24. Vulkan 1.4 Pipeline Pre-Warming & Zero-Stutter Cache Verification
    {
        ze::render::PipelineWarmup warmup;
        ze::render::WarmupPipelineDesc p1;
        p1.pipelineName = "zombie_flocker_vat";
        p1.shaderStageBits = 3;
        warmup.registerPipelineDesc(p1);

        ze::render::WarmupPipelineDesc p2;
        p2.pipelineName = "player_weapon_fp";
        p2.shaderStageBits = 3;
        warmup.registerPipelineDesc(p2);

        assert(warmup.getRegisteredPipelineCount() == 2);
        bool warmed = warmup.executeWarmup();
        assert(warmed);
        assert(warmup.isWarmupComplete());
        assert(warmup.getWarmedPipelineCount() == 2);

        // Test pipeline cache serialization with driver UUID
        std::string cacheFile = "build/test_pipeline_cache.bin";
        uint8_t uuid[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        std::vector<uint8_t> cacheBytes = { 0xDE, 0xAD, 0xBE, 0xEF };

        bool saved = ze::render::PipelineWarmup::saveCacheToFile(cacheFile, 0x10DE, 0x1F08, uuid, cacheBytes);
        assert(saved);

        std::vector<uint8_t> loadedBytes;
        bool loaded = ze::render::PipelineWarmup::loadCacheFromFile(cacheFile, 0x10DE, 0x1F08, uuid, loadedBytes);
        assert(loaded);
        assert(loadedBytes == cacheBytes);

        // Mismatched UUID (driver update simulated) -> must return false to invalidate cache
        uint8_t wrongUuid[16] = { 0xFF };
        std::vector<uint8_t> staleBytes;
        bool shouldFail = ze::render::PipelineWarmup::loadCacheFromFile(cacheFile, 0x10DE, 0x1F08, wrongUuid, staleBytes);
        assert(!shouldFail);
    }

    return 0;
}
