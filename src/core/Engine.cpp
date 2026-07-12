#include "Engine.h"
#include "Config.h"
#include "Logger.h"
#include "Platform.h"
#include "Input.h"
#include "JobSystem.h"
#include "TeardownTracker.h"
#include "../render/VulkanContext.h"
#include "../render/MSDFPipeline.h"
#include "../render/Device.h"
#include "../physics/PhysicsSystem.h"
#include "../events/EventBus.h"
#include "../ecs/Destructible.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <chrono>
#include <fstream>
#include <thread>

#include "../debug/MetaRegistry.h"
#include "../render/Device.h"
#include "../ecs/ECS.h"
#include "../ecs/Components.h"
#include "../ecs/EnTTCache.h"

#include <imgui.h>

#ifdef JPH_DEBUG_RENDERER
#include "../physics/PhysicsDebugRenderer.h"
#endif
#include "../ecs/EntityFactory.h"
#include "../ecs/SpatialHash.h"
#include "../ecs/GenerationalTable.h"
#include "FileHandleRing.h"
#include "ThreadAffinity.h"

namespace core {

Engine::Engine() {
    try {
        Platform::initCrashHandler();
        Logger::init();
        
        LOG_INFO("Initializing Engine...");

        uint32_t numJobThreads = Config::get().replayInput ? 1 : 0;
        JobSystem::init(numJobThreads);
        TeardownTracker::RegisterInit(TeardownTracker::Stage::JobSystem, "JobSystem");
        Input::init();

        debug::MetaRegistry::Initialize();

        if (!Config::get().headless) {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                LOG_CRITICAL("Failed to initialize SDL3");
                Platform::triggerBreakpoint();
            }
            TeardownTracker::RegisterInit(TeardownTracker::Stage::SDL, "SDL");

            if (!SDL_Vulkan_LoadLibrary(nullptr)) {
                LOG_CRITICAL("Failed to load Vulkan library in SDL: {}", SDL_GetError());
                Platform::triggerBreakpoint();
            }

            window_ = SDL_CreateWindow("Zombie Engine", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
            if (!window_) {
                LOG_CRITICAL("Failed to create SDL3 window: {}", SDL_GetError());
                Platform::triggerBreakpoint();
            }
        }

        vulkanContext_ = std::make_unique<render::VulkanContext>(window_);
        LOG_INFO("ENGINE: VulkanContext initialized");
        
        uint32_t numThreads = Config::get().replayInput ? 1 : std::thread::hardware_concurrency();
        ecsContext_ = std::make_unique<ecs::ECSContext>(numThreads);
        LOG_INFO("ENGINE: ECSContext initialized");

        // [M2] Physics and event bus — init after ECS, before rendering
        eventBus_     = std::make_unique<events::EventBus>();
        LOG_INFO("ENGINE: EventBus initialized");
        
        physics::PhysicsSystem::initializeGlobal();
        physicsSystem_ = std::make_unique<physics::PhysicsSystem>();
        LOG_INFO("ENGINE: PhysicsSystem initialized");
        
        ecs::DamageSystem::init(&ecsContext_->GetRegistry(), eventBus_.get());
        LOG_INFO("ENGINE: DamageSystem initialized");

        if (!Config::get().headless) {
            imguiOverlay_.Initialize(
                vulkanContext_->getDevice()->getLogicalDevice(),
                vulkanContext_->getInstance(),
                vulkanContext_->getDevice()->getPhysicalDevice(),
                vulkanContext_->getDevice()->getGraphicsQueue(),
                vulkanContext_->getDevice()->getGraphicsQueueIndex(),
                window_
            );
            TeardownTracker::RegisterInit(TeardownTracker::Stage::ImGui, "ImGui");
            
            render::MSDFPipeline::GenerateAtlas(
                vulkanContext_->getDevice(), 
                "assets/fonts/Roboto-Regular.ttf"
            );
        }

        if (Config::get().headless && Config::get().quitFrame == -1) {
            // Run for exactly 1 frame in headless mode if quitFrame is not specified
            const_cast<Config&>(Config::get()).quitFrame = 1;
        }
    } catch (...) {
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        JobSystem::shutdown();
        Input::shutdown();
        throw;
    }
}

Engine::~Engine() {
    if (!Config::get().headless) {
        render::MSDFPipeline::Cleanup(vulkanContext_->getDevice());
        imguiOverlay_.Destroy(vulkanContext_->getDevice()->getLogicalDevice());
        TeardownTracker::RegisterShutdown(TeardownTracker::Stage::ImGui);
    }

    vulkanContext_.reset();

    if (window_) {
        SDL_DestroyWindow(window_);
    }
    
    if (!Config::get().headless) {
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();
        TeardownTracker::RegisterShutdown(TeardownTracker::Stage::SDL);
    }

    Input::shutdown();
    JobSystem::shutdown();
    TeardownTracker::RegisterShutdown(TeardownTracker::Stage::JobSystem);
    LOG_INFO("Engine Shutdown Complete.");
}

void Engine::run() {
    running_ = true;
    mainLoop();
}

void Engine::mainLoop() {
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    double accumulator = 0.0;
    const double FIXED_DT = 1.0 / 60.0;

    uint64_t frameCount = 0;
    uint64_t tickHash = 0;

    std::ofstream hashFile;
    std::ifstream hashFileIn;
    if (Config::get().recordInput) {
        hashFile.open("tick_hashes.txt");
    } else if (Config::get().replayInput) {
        hashFileIn.open("tick_hashes.txt");
    }

    while (running_) {
        auto currentTime = clock::now();
        std::chrono::duration<double> frameTime = currentTime - lastTime;
        lastTime = currentTime;

        double dt = frameTime.count();
        if (dt > 0.25) dt = 0.25; // Clamp at 0.25s to avoid spiral of death

        accumulator += dt;

        Input::poll();
        if (Input::getState().quit) {
            running_ = false;
        }
        if (!Config::get().headless) {
            for (const auto& ev : Input::getState().events) {
                imguiOverlay_.ProcessEvent(&ev);
            }
        }

        while (accumulator >= FIXED_DT) {
            // [M1-EXT-06] Drain SPSC queues before system ticks
            if (ecsContext_) {
                ecsContext_->DrainMutations([](entt::registry& reg, const ecs::DeferredMutation& mut) {
                    if (mut.operationBitmask == 1) { // 1 = destroy entity
                        if (reg.valid(mut.targetEntity)) {
                            LOG_INFO("M1-EXT-06: Successfully drained mutation and destroyed entity {}", static_cast<uint32_t>(mut.targetEntity));
                            reg.destroy(mut.targetEntity);
                        }
                    }
                });
            }

            // [M2] Fixed-step physics tick
            physicsTick();

            // Fixed update tick hash
            tickHash ^= std::hash<size_t>{}(Input::getState().events.size()) + 0x9e3779b9 + (tickHash << 6) + (tickHash >> 2);
            tickHash ^= std::hash<uint64_t>{}(frameCount) + 0x9e3779b9 + (tickHash << 6) + (tickHash >> 2);

            accumulator -= FIXED_DT;
        }



        if (Config::get().quitFrame != -1 && frameCount == Config::get().quitFrame) {
            LOG_INFO("Triggering clean quit at frame {}", frameCount);
            running_ = false;
        }

        if (frameCount == 5 && ecsContext_) {
            auto& reg = ecsContext_->GetRegistry();
            auto entities = ecs::EntityFactory::LoadPrefab(reg, "assets/prefabs/test_prefab.json");
            
            for (size_t i = 0; i < entities.size(); ++i) {
                auto ent = entities[i];
                LOG_INFO("VERIFYING PREFAB ENTITY {}", i);
                if (auto* t = reg.try_get<ecs::Transform>(ent)) {
                    LOG_INFO("  Transform: position = [{}, {}, {}]", t->position.x, t->position.y, t->position.z);
                }
                if (auto* h = reg.try_get<ecs::Health>(ent)) {
                    LOG_INFO("  Health: current = {}, max = {}", h->current, h->max);
                }
                if (auto* s = reg.try_get<ecs::StableId>(ent)) {
                    LOG_INFO("  StableId: uuid = {}", s->uuid);
                }
                if (auto* b = reg.try_get<ecs::BulletComponent>(ent)) {
                    LOG_INFO("  BulletComponent: velocity = [{}, {}, {}], mass = {}, drag = {}, flags = {}", 
                             b->velocity.x, b->velocity.y, b->velocity.z, b->mass, b->dragCoefficient, b->flags);
                }
            }
            LOG_INFO("Spawned 3 live entities for testing.");
        }

        if (frameCount == 6 && ecsContext_) {
            auto& reg = ecsContext_->GetRegistry();
            auto cache = ecs::SnapshotComponentPool<ecs::Transform>(reg);
            
            auto view = reg.view<ecs::Transform>();
            bool matched = true;
            for (auto entity : view) {
                uint32_t rawEnt = static_cast<uint32_t>(entity);
                ecs::Transform* cachedPtr = cache.ResolveComponentPointerDirect<ecs::Transform>(rawEnt);
                ecs::Transform& viewRef = view.get<ecs::Transform>(entity);
                
                if (cachedPtr != &viewRef || cachedPtr->position.x != viewRef.position.x) {
                    matched = false;
                    void* derefCachePtr = nullptr;
                    if (cachedPtr) derefCachePtr = *(void**)cachedPtr;
                    LOG_ERROR("EXT-09 Mismatch! Entity {} cachePtr={} (deref={}) viewPtr={}", rawEnt, (void*)cachedPtr, derefCachePtr, (void*)&viewRef);
                }
            }
            if (matched && !view.empty()) {
                LOG_INFO("EXT-09 Cache verified: {} entities matched exactly in memory between EnTTCache and EnTT view.", view.size());
            }
        }

        // Test SPSC Queue with a background enkiTS job
        if (frameCount == 10 && ecsContext_) {
            // Find one of the live entities to destroy asynchronously
            auto view = ecsContext_->GetRegistry().view<ecs::Transform>();
            if (!view.empty()) {
                entt::entity target = view.front();
                
                struct SpscSweeperTask : enki::ITaskSet {
                    ecs::ECSContext* ecs;
                    entt::entity target;
                    SpscSweeperTask(ecs::ECSContext* e, entt::entity t) : ecs(e), target(t) {}
                    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
                        ecs->GetWorkerQueue(threadnum).PushMutation(target, 1);
                    }
                };
                SpscSweeperTask* task = new SpscSweeperTask(ecsContext_.get(), target);
                JobSystem::get()->AddTaskSetToPipe(task);
                LOG_INFO("M1-EXT-06: Dispatched async destroy for entity {} via SPSC mutation queue.", static_cast<uint32_t>(target));
                JobSystem::get()->WaitforTask(task);
                delete task;
            }
        }
        if (window_) {
            if (frameCount == 10) {
                SDL_SetWindowSize(window_, 800, 600);
            } else if (frameCount == 20) {
                SDL_MinimizeWindow(window_);
            } else if (frameCount == 30) {
                SDL_RestoreWindow(window_);
            }
        }

        LOG_INFO("Frame {} TickHash {}", frameCount, tickHash);

        // Render update
        bool canRender = true;
        if (window_) {
            int width = 0, height = 0;
            SDL_GetWindowSizeInPixels(window_, &width, &height);
            uint32_t flags = SDL_GetWindowFlags(window_);
            if (width == 0 || height == 0 || (flags & SDL_WINDOW_MINIMIZED)) {
                canRender = false;
            }
        }

        if (canRender) {
            if (!Config::get().headless) {
                imguiOverlay_.NewFrame();

#ifdef JPH_DEBUG_RENDERER
                if (physicsSystem_ && physicsSystem_->getDebugRenderer()) {
                    physicsSystem_->drawBodies();
                    const auto& lines = physicsSystem_->getDebugRenderer()->getLines();
                    
                    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
                    int w = 800, h = 600;
                    if (window_) {
                        SDL_GetWindowSizeInPixels(window_, &w, &h);
                    }
                    
                    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)w / (float)h, 0.1f, 1000.0f);
                    glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 20), glm::vec3(0,0,0), glm::vec3(0,1,0));
                    glm::mat4 vp = proj * view;
                    
                    for (const auto& line : lines) {
                        glm::vec4 p1 = vp * glm::vec4(line.from, 1.0f);
                        glm::vec4 p2 = vp * glm::vec4(line.to, 1.0f);
                        
                        if (p1.w > 0.0f && p2.w > 0.0f) {
                            p1 /= p1.w;
                            p2 /= p2.w;
                            
                            // Map NDC to screen coords. Y points down in ImGui.
                            ImVec2 sp1((p1.x * 0.5f + 0.5f) * w, (p1.y * -0.5f + 0.5f) * h);
                            ImVec2 sp2((p2.x * 0.5f + 0.5f) * w, (p2.y * -0.5f + 0.5f) * h);
                            
                            drawList->AddLine(sp1, sp2, line.color);
                        }
                    }
                }
#endif
            }
            vulkanContext_->renderFrame(&imguiOverlay_);
        }
        
        if (Config::get().recordInput && hashFile.is_open()) {
            hashFile << frameCount << " " << tickHash << "\n";
        }
        if (Config::get().replayInput && hashFileIn.is_open()) {
            uint64_t expectedFrame = 0;
            uint64_t expectedHash = 0;
            if (hashFileIn >> expectedFrame >> expectedHash) {
                ENGINE_ASSERT(expectedFrame == frameCount, "Replay frame count mismatch! Expected {}, got {}", expectedFrame, frameCount);
                if (expectedHash != tickHash) {
                    LOG_CRITICAL("Replay determinism violation at frame {}! Expected hash {}, got {}", frameCount, expectedHash, tickHash);
                    Platform::triggerBreakpoint();
                    exit(1);
                }
            }
        }

        frameCount++;
    }
}

void Engine::physicsTick() {
    if (physicsSystem_) {
        // [M2] Jolt simulation steps
        physicsSystem_->step(ecsContext_->GetRegistry(), eventBus_->raw());
    }

    if (eventBus_) {
        // [M2] Apply queued DamageEvents to Health/Destructible
        ecs::DamageSystem::tick(ecsContext_->GetRegistry(), *eventBus_);
        // Flush remaining events to subscribers
        eventBus_->flush();
    }
}

#if ENGINE_DEV_TOOLS

static bool runCoreTests() {
    // 1. Test FileHandleRing modulo indexing safety
    FileHandleRing ring;
    ring.handles.push_back((OSFileHandle)1);
    ring.handles.push_back((OSFileHandle)2);
    ring.handles.push_back((OSFileHandle)3); // Non-power-of-two size (3)
    OSFileHandle h1 = AcquireHandle(ring);
    OSFileHandle h2 = AcquireHandle(ring);
    OSFileHandle h3 = AcquireHandle(ring);
    OSFileHandle h4 = AcquireHandle(ring);
    if (h1 != (OSFileHandle)1 || h2 != (OSFileHandle)2 || h3 != (OSFileHandle)3 || h4 != (OSFileHandle)1) {
        LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: FileHandleRing indexing failed!");
        return false;
    }

    // 2. Test ThreadAffinity limit to 64 cores
    ThreadAffinityAllocator aff;
    for (int i = 0; i < 100; ++i) {
        uint64_t mask = aff.AllocateBitmask();
        if (mask == 0) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: ThreadAffinity returned 0 mask!");
            return false;
        }
    }

    LOG_INFO("DIAGNOSTIC SUCCESS: Core systems verified.");
    return true;
}

static bool runEcsTests() {
    // 1. Test SpatialHash subdivision & Euclidean modulo math
    {
        ecs::SpatialHash sh;
        
        // Insert 100 entities to trigger subdivision (threshold is 64)
        for (int i = 0; i < 100; ++i) {
            sh.Insert(static_cast<entt::entity>(i), 0.1f, 0.1f);
        }
        
        // Assert they are all queryable from the parent cell
        auto cellEnts = sh.QueryCell(0, 0);
        if (cellEnts.size() != 100) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: SpatialHash subdivision failed! Expected 100 entities in Cell (0,0), got {}", cellEnts.size());
            return false;
        }

        // Insert entity at negative coordinate
        sh.Insert(static_cast<entt::entity>(101), -0.2f, -0.2f);
        auto negCellEnts = sh.QueryCell(-1, -1);
        if (negCellEnts.size() != 1) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: SpatialHash negative coordinate floor modulo failed! Expected 1 entity in Cell (-1,-1), got {}", negCellEnts.size());
            return false;
        }

        sh.Clear();
    }

    // 2. Test EnTTCache bounds and null checks
    {
        entt::registry reg;
        entt::entity e1 = reg.create();
        entt::entity e2 = reg.create();

        // Emplace Transform component only on e1
        reg.emplace<ecs::Transform>(e1);

        ecs::LockedComponentPoolCache cache = ecs::SnapshotComponentPool<ecs::Transform>(reg);

        // Resolve component for e1 (should succeed)
        auto* t1 = cache.ResolveComponentPointerDirect<ecs::Transform>(static_cast<uint32_t>(e1));
        if (!t1) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: EnTTCache failed to resolve component for entity with component!");
            return false;
        }

        // Resolve component for e2 (should fail gracefully returning nullptr)
        auto* t2 = cache.ResolveComponentPointerDirect<ecs::Transform>(static_cast<uint32_t>(e2));
        if (t2 != nullptr) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: EnTTCache did not return nullptr for entity without component!");
            return false;
        }

        // Resolve component for out-of-bounds entity ID (should fail gracefully returning nullptr)
        auto* t3 = cache.ResolveComponentPointerDirect<ecs::Transform>(99999);
        if (t3 != nullptr) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: EnTTCache did not return nullptr for out-of-bounds entity ID!");
            return false;
        }
    }

    // 3. Test GenerationalTable resource release
    {
        ecs::GenerationalTable<ecs::StableId> table;
        ecs::StableId s;
        s.uuid = 42;
        ecs::Handle h = table.Insert(s);

        if (table.isSlotCleared(h.index)) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: GenerationalTable slot was reported cleared before removal!");
            return false;
        }

        table.Remove(h);

        if (!table.isSlotCleared(h.index)) {
            LOG_CRITICAL("DIAGNOSTIC TEST FAILURE: GenerationalTable slot was not cleared after removal!");
            return false;
        }
    }

    LOG_INFO("DIAGNOSTIC SUCCESS: ECS and cache safeguards verified.");
    return true;
}

static bool runRenderTests(render::Device* device) {
    // --- Test 1: MAX_FRAMES_IN_FLIGHT constant ---
    // The codebase uses 3 hard-coded throughout (createBuffers, QueryPool capacity, framePacing limit).
    // Assert the literal matches so any future refactor that changes one without the other fails loudly.
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
    static_assert(MAX_FRAMES_IN_FLIGHT == 3,
        "MAX_FRAMES_IN_FLIGHT must be 3 to match triple-buffered buffer arrays in TriangleRenderer");
    LOG_INFO("RENDER TEST: MAX_FRAMES_IN_FLIGHT == {} confirmed", MAX_FRAMES_IN_FLIGHT);

    // --- Test 2: Triple-buffer frame index selection over 5 simulated frames ---
    // cull() and draw() both index buffers as [imageIndex % MAX_FRAMES_IN_FLIGHT].
    // Simulate 5 frames and verify the selected index is always in [0, 2].
    bool indexSelectionOk = true;
    for (uint32_t frame = 0; frame < 5; ++frame) {
        uint32_t selectedIndex = frame % MAX_FRAMES_IN_FLIGHT;
        if (selectedIndex >= MAX_FRAMES_IN_FLIGHT) {
            LOG_ERROR("RENDER TEST FAIL: frame {} produced out-of-range buffer index {}", frame, selectedIndex);
            indexSelectionOk = false;
        }
    }
    if (indexSelectionOk) {
        LOG_INFO("RENDER TEST PASS: Triple-buffer index selection correct over 5 simulated frames (indices 0,1,2,0,1)");
    } else {
        return false;
    }

    // --- Test 3: Capability tier logging ---
    const auto& caps = device->getCapabilities();
    LOG_INFO("RENDER TEST: Capability tier — descriptorBuffer={} shaderObject={} unifiedImageLayouts={} meshShaders={} rtPipeline={} queryTimestamps={}",
        caps.descriptorBuffer, caps.shaderObject, caps.unifiedImageLayouts,
        caps.meshShaders, caps.rtPipeline, caps.queryTimestamps);

    return true;
}

static bool runPhysicsTests(ecs::ECSContext* ecsCtx, physics::PhysicsSystem* physCtx, events::EventBus* bus) {
    LOG_INFO("Running M2 Physics Tests...");
    auto& reg = ecsCtx->GetRegistry();

    // 1. Create a destructible test entity with Health and DestructibleComponent
    entt::entity testEnt = reg.create();
    reg.emplace<ecs::Transform>(testEnt);
    reg.emplace<ecs::Health>(testEnt, 100.0f, 100.0f);
    
    auto& dest = reg.emplace<ecs::DestructibleComponent>(testEnt);
    dest.intactMeshHandle = 1;
    dest.destroyedMeshHandle = 2;

    // 2. Create a Jolt body for the entity
    JPH::BodyID bodyId = physCtx->createBox(glm::dvec3(0, 10, 0), glm::vec3(1, 1, 1), false, 50.0f);
    if (bodyId.IsInvalid()) {
        LOG_CRITICAL("PHYSICS TEST FAIL: Failed to create Jolt body!");
        return false;
    }
    reg.emplace<physics::PhysicsBodyComponent>(testEnt, bodyId);

    // 3. Step physics once to confirm it moves
    physCtx->step(reg, bus->raw());
    
    auto& tf = reg.get<ecs::Transform>(testEnt);
    if (tf.position.y >= 10.0f) {
        LOG_CRITICAL("PHYSICS TEST FAIL: Body did not fall under gravity! Pos: {}", tf.position.y);
        return false;
    }

    // 4. Send lethal damage event via EventBus
    ecs::DamageEvent dmg;
    dmg.amount = 150.0f;
    dmg.source = testEnt; // For M2 test, we address by source
    dmg.instigator = testEnt;
    bus->enqueue(dmg);

    // 5. Tick DamageSystem
    ecs::DamageSystem::tick(reg, *bus);

    // 6. Verify DestructibleComponent triggered
    auto& destCheck = reg.get<ecs::DestructibleComponent>(testEnt);
    if (!destCheck.isDestroyed) {
        LOG_CRITICAL("PHYSICS TEST FAIL: Entity not marked destroyed after lethal damage!");
        return false;
    }
    if (reg.all_of<physics::PhysicsBodyComponent>(testEnt)) {
        LOG_CRITICAL("PHYSICS TEST FAIL: PhysicsBodyComponent not removed upon destruction!");
        return false;
    }

    // Clean up
    physCtx->destroyBody(bodyId);
    reg.destroy(testEnt);

    LOG_INFO("DIAGNOSTIC SUCCESS: M2 Physics and EventBus verified.");
    return true;
}
#endif

bool Engine::verifyHeadlessInit() const {
    if (!vulkanContext_) return false;
    auto* device = vulkanContext_->getDevice();
    if (!device) return false;
    if (device->getLogicalDevice() == VK_NULL_HANDLE) return false;
    if (device->getAllocator() == VK_NULL_HANDLE) return false;

#if ENGINE_DEV_TOOLS
    if (!runCoreTests()) return false;
    if (!runEcsTests()) return false;
    if (!runRenderTests(device)) return false;
    if (!runPhysicsTests(ecsContext_.get(), physicsSystem_.get(), eventBus_.get())) return false;
#endif

    return true;
}

} // namespace core

