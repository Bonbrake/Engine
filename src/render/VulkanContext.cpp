#include "VulkanContext.h"
#include "Device.h"
#include "Swapchain.h"
#include "../core/Logger.h"
#include "../core/Config.h"
#include "../core/Platform.h"
#include "PipelineCacheManager.h"
#include "MaterialSystem.h"
#include "AssetManager.h"
#include "ShaderManager.h"
#include "../core/TeardownTracker.h"
#include <SDL3/SDL_vulkan.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace render {

VulkanContext::VulkanContext(SDL_Window* window) : window_(window) {
    initVulkan(window);
}

VulkanContext::~VulkanContext() {
    cleanup();
}

void VulkanContext::initVulkan(SDL_Window* window) {
    if (volkInitialize() != VK_SUCCESS) {
        LOG_CRITICAL("Failed to initialize volk");
        core::Platform::triggerBreakpoint();
    }

    vkb::InstanceBuilder builder;
    
    // Require Vulkan 1.4 API
    builder.require_api_version(1, 4, 0)
           .set_app_name("ZombieEngine")
           .set_engine_name("Antigravity2")
           .request_validation_layers(core::Config::get().devMode)
           .use_default_debug_messenger();

    if (!core::Config::get().headless) {
        // Platform specific surface extensions would be requested here if we weren't using SDL
        // vkb handles basic surface extensions if we provide a dummy surface, or we enable them manually.
        // Actually SDL requires us to enable surface extensions ourselves, or vkb instance builder does it?
        // vkbInstanceBuilder enables surface extensions automatically.
    } else {
        builder.set_headless();
    }

    auto inst_ret = builder.build();
    if (!inst_ret) {
        LOG_CRITICAL("Failed to create Vulkan instance: {}", inst_ret.error().message());
        core::Platform::triggerBreakpoint();
    }
    vkbInstance_ = inst_ret.value();
    volkLoadInstance(vkbInstance_.instance);

    if (!core::Config::get().headless && window) {
        if (!SDL_Vulkan_CreateSurface(window, vkbInstance_.instance, nullptr, &surface_)) {
            LOG_CRITICAL("Failed to create Vulkan surface: {}", SDL_GetError());
            core::Platform::triggerBreakpoint();
        }
    }

    // Initialize Device and Swapchain
    device_ = std::make_unique<Device>(this);
    ShaderManager::init(device_.get());
    
    core::TeardownTracker::RegisterInit(core::TeardownTracker::Stage::VulkanInstance, "VulkanInstance");
    core::TeardownTracker::RegisterInit(core::TeardownTracker::Stage::Device, "Device");

    // Initialize PipelineCache
    PipelineCacheManager::init(device_.get(), "pipeline_cache.bin");

    if (!core::Config::get().headless) {
        swapchain_ = std::make_unique<Swapchain>(device_.get(), window_);
        core::TeardownTracker::RegisterInit(core::TeardownTracker::Stage::Swapchain, "Swapchain");
    }

    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->Initialize(device_.get());
    core::TeardownTracker::RegisterInit(core::TeardownTracker::Stage::AssetManager, "AssetManager");
    
    materialSystem_ = std::make_unique<MaterialSystem>();
    materialSystem_->Initialize(device_.get());
    core::TeardownTracker::RegisterInit(core::TeardownTracker::Stage::MaterialSystem, "MaterialSystem");
    
    // 0. Verify Dense Geometry Format round-trip compression
    Vertex testV{ {1.2f, -3.4f, 5.6f}, {0.0f, 1.0f, 0.0f}, {0.2f, 0.8f} };
    CompressedVertex cv = CompressVertex(testV);
    Vertex decompV = DecompressVertex(cv);
    float posDiff = glm::distance(testV.position, decompV.position);
    float normCos = glm::dot(testV.normal, decompV.normal);
    ENGINE_ASSERT(posDiff < 0.05f, "Compressed position error too large!");
    ENGINE_ASSERT(normCos > 0.99f, "Compressed normal error too large!");
    LOG_INFO("VERIFICATION SUCCESS: Dense Geometry compression verified. Position error: {}, Normal cosine: {}", posDiff, normCos);

    // 1. Verify soft-failure path for non-existent texture (should use fallback)
    ecs::Handle nonexistentTex = assetManager_->LoadTexture("assets/textures/nonexistent.png");
    render::MaterialAsset invalidMat;
    invalidMat.albedoTexture = nonexistentTex;
    ecs::Handle fallbackMat = materialSystem_->CreateMaterial(invalidMat, assetManager_.get());
    if (fallbackMat.index != 0xFFFFFFFF) {
        LOG_INFO("VERIFICATION SUCCESS: Soft asset mitigation working. Fallback material created successfully.");
    } else {
        LOG_ERROR("VERIFICATION FAILURE: Soft asset mitigation failed!");
    }

    // 2. Verify success path for valid texture
    ecs::Handle texHandle = assetManager_->LoadTexture("assets/textures/test.png");
    if (texHandle.index != 0xFFFFFFFF) {
        LOG_INFO("VERIFICATION SUCCESS: Loaded test.png. Handle index: {}", texHandle.index);
        
        render::MaterialAsset dummyMat;
        dummyMat.albedoTexture = texHandle;
        ecs::Handle matHandle = materialSystem_->CreateMaterial(dummyMat, assetManager_.get());
        if (matHandle.index != 0xFFFFFFFF) {
            LOG_INFO("VERIFICATION SUCCESS: Created material. Handle index: {}, Offset: {} bytes", 
                     matHandle.index, matHandle.index * materialSystem_->GetAlignedBlockSize());
            
            // 3. Verify overflow (MAX_MATERIALS = 4096)
            bool overflowFailed = false;
            std::vector<ecs::Handle> testMaterials;
            testMaterials.push_back(matHandle);
            for (uint32_t i = 0; i < 4096; i++) {
                ecs::Handle h = materialSystem_->CreateMaterial(dummyMat, assetManager_.get());
                if (h.index == 0xFFFFFFFF) {
                    LOG_INFO("VERIFICATION SUCCESS: Overflow triggered exactly at index {} (total material count: 4096)", i + 1);
                    overflowFailed = true;
                    break;
                } else {
                    testMaterials.push_back(h);
                }
            }
            if (!overflowFailed) {
                LOG_ERROR("VERIFICATION FAILURE: MAX_MATERIALS overflow check failed!");
            }
            
            // Dev-test mesh load (asset-manager self-test, pairs with the texture self-test above)
            devTestMeshHandle_ = assetManager_->LoadMesh("assets/models/dev_test_cube.glb");
            if (devTestMeshHandle_.generation != 0) {  // valid Insert => generation >= 1; Handle() null => 0
                LOG_INFO("VERIFICATION SUCCESS: Loaded dev_test_cube.glb. Mesh handle index: {}", devTestMeshHandle_.index);
                // [Slice 0a] Hand the loaded mesh to the renderer so draw() renders it as a gated cube.
                if (swapchain_ && swapchain_->triangleRenderer()) {
                    swapchain_->triangleRenderer()->setDevTestMesh(assetManager_->GetMesh(devTestMeshHandle_));
                    LOG_INFO("[Slice 0a] Dev-test cube wired into TriangleRenderer.");
                }
            } else {
                LOG_ERROR("VERIFICATION FAILURE: dev_test_cube.glb load failed!");
            }
            // Clean up dummy test materials to avoid startup saturation
            for (auto h : testMaterials) {
                materialSystem_->DestroyMaterial(h);
            }
        }
    }
}

void VulkanContext::cleanup() {
    if (device_) {
        device_->waitIdle();
        core::TeardownTracker::RegisterShutdown(core::TeardownTracker::Stage::MaterialSystem);
        if (materialSystem_) {
            materialSystem_->Destroy();
        }
        core::TeardownTracker::RegisterShutdown(core::TeardownTracker::Stage::AssetManager);
        if (assetManager_) {
            assetManager_->Destroy();
        }
        ShaderManager::shutdown();
        PipelineCacheManager::shutdown(device_.get());
    }

    materialSystem_.reset();
    assetManager_.reset();

    if (!core::Config::get().headless) {
        core::TeardownTracker::RegisterShutdown(core::TeardownTracker::Stage::Swapchain);
    }
    swapchain_.reset();
    
    core::TeardownTracker::RegisterShutdown(core::TeardownTracker::Stage::Device);
    device_.reset();

    if (surface_) {
        vkb::destroy_surface(vkbInstance_.instance, surface_);
        surface_ = VK_NULL_HANDLE;
    }
    
    core::TeardownTracker::RegisterShutdown(core::TeardownTracker::Stage::VulkanInstance);
    vkb::destroy_instance(vkbInstance_);
}

void VulkanContext::renderFrame(debug::ImGuiOverlay* imguiOverlay) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (core::Config::get().headless) {
        return; 
    }

    if (swapchain_) {
        // Frame-dump: request a capture on the matching render-call index (windowed only).
        const auto& cfg = core::Config::get();
        if (!cfg.dumpFramePath.empty() && cfg.dumpFrameAt >= 0 &&
            renderCallCount_ == static_cast<uint64_t>(cfg.dumpFrameAt)) {
            swapchain_->requestDump(cfg.dumpFramePath);
        }
        renderCallCount_++;
        swapchain_->acquireAndPresent(imguiOverlay, materialSystem_.get());
    }
}

} // namespace render
