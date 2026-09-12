#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <entt/entt.hpp>
#include "ze/debug/ImGuiOverlay.h"

namespace render { class VulkanContext; }
namespace ecs { class ECSContext; }
namespace physics { class PhysicsSystem; }
namespace events { class EventBus; }
namespace debug { class FlyCamera; }
namespace audio { class AudioEngine; }
namespace ai { class AIDirector; }
namespace world { class BiomeGraph; class ChunkStreamer; }
namespace survival { class BodyTempSystem; class StaminaSystem; }
namespace save { class SaveSystem; }
namespace modding { class Modding; }
namespace net { class NetworkManager; }
namespace slm { class SLMClient; }
namespace ui { class HUD; }
namespace combat { class DamageSystem; }
namespace vehicle { class VehicleSystem; }

namespace ze::ai { class L4D2Director; class SwarmEngine; class L4D2SLMBridge; class ResponseSystem; class SquadCommandWheel; }
namespace ze::world { class ItemPersistenceSystem; }
namespace ze::core { class BodycamCamera; class GamepadController; class SteamworksManager; }
namespace ze::audio { class BodycamAcousticsSystem; }
namespace ze::render { class PipelineWarmup; class ProceduralWeaponAnim; }

namespace core {

class Engine {
public:
    Engine();
    ~Engine();

    void run();
    bool verifyHeadlessInit() const;

    ze::ai::L4D2Director* getL4D2Director() const { return l4d2Director_.get(); }
    ze::ai::SwarmEngine* getSwarmEngine() const { return swarmEngine_.get(); }
    ze::world::ItemPersistenceSystem* getItemPersistence() const { return itemPersistence_.get(); }
    ze::core::BodycamCamera* getBodycamCamera() const { return bodycamCamera_.get(); }

private:
    void mainLoop();
    void physicsTick(); // fixed-timestep physics step
    void dumpVendorCheckpoints();

#if ENGINE_DEV_TOOLS
    // [M2-#4] Dev-only scaffolding: spawn a Jolt body in the windowed --dev path so
    // drawBodies() has non-empty input for the debug-draw overlay eyes-on check.
    // Gated by devMode && !headless; never touches the headless CI/audit path.
    void spawnDevTestBody_();
#endif


    bool running_ = false;
    float physicsAccumulator_ = 0.0f; // [M2] fixed-timestep accumulator

    SDL_Window* window_ = nullptr;
    std::unique_ptr<render::VulkanContext>  vulkanContext_;
    std::unique_ptr<ecs::ECSContext>        ecsContext_;
    std::unique_ptr<physics::PhysicsSystem> physicsSystem_; // [M2] Jolt
    std::unique_ptr<events::EventBus>       eventBus_;      // [M2] entt::dispatcher
    debug::ImGuiOverlay imguiOverlay_;

    // Core & Gameplay Subsystems
    std::unique_ptr<audio::AudioEngine>          audioEngine_;
    std::unique_ptr<ai::AIDirector>              aiDirector_;
    std::unique_ptr<world::BiomeGraph>           biomeGraph_;
    std::unique_ptr<world::ChunkStreamer>        chunkStreamer_;
    std::unique_ptr<survival::BodyTempSystem>    bodyTempSystem_;
    std::unique_ptr<survival::StaminaSystem>     staminaSystem_;
    std::unique_ptr<save::SaveSystem>            saveSystem_;
    std::unique_ptr<modding::Modding>            modding_;
    std::unique_ptr<net::NetworkManager>         networkManager_;
    std::unique_ptr<slm::SLMClient>              slmClient_;
    std::unique_ptr<ui::HUD>                     hud_;
    std::unique_ptr<vehicle::VehicleSystem>      vehicleSystem_;

    // Master Plan v8.0 Architecture Subsystems (Stages 1-3)
    std::unique_ptr<ze::ai::L4D2Director>            l4d2Director_;
    std::unique_ptr<ze::ai::SwarmEngine>             swarmEngine_;
    std::unique_ptr<ze::ai::L4D2SLMBridge>           l4d2SlmBridge_;
    std::unique_ptr<ze::ai::ResponseSystem>          responseSystem_;
    std::unique_ptr<ze::ai::SquadCommandWheel>       squadWheel_;
    std::unique_ptr<ze::world::ItemPersistenceSystem> itemPersistence_;
    std::unique_ptr<ze::core::BodycamCamera>         bodycamCamera_;
    std::unique_ptr<ze::core::GamepadController>     gamepadController_;
    std::unique_ptr<ze::core::SteamworksManager>     steamworksManager_;
    std::unique_ptr<ze::audio::BodycamAcousticsSystem> acoustics_;
    std::unique_ptr<ze::render::PipelineWarmup>      pipelineWarmup_;
    std::unique_ptr<ze::render::ProceduralWeaponAnim> weaponAnim_;

#if ENGINE_DEV_TOOLS
    // [M2-#4] Dev-test hook state (devMode && !headless only)
    bool devTestBodySpawned_ = false;
    bool devTestDamageFired_ = false;
    entt::entity devTestEntity_ = entt::null;

    // [M2.6 Phase 2] Debug fly-camera owner (dev-gated; only allocated when --fly-camera).
    std::unique_ptr<debug::FlyCamera> flyCamera_;
#endif
};

} // namespace core
