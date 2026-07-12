#define VMA_IMPLEMENTATION
#include "Device.h"
#include "VulkanContext.h"
#include "MemoryBudget.h"
#include "DriverBlocklist.h"
#include "../core/Logger.h"
#include "../core/Config.h"
#include "../core/Platform.h"

namespace render {

Device::Device(VulkanContext* context) {
    selectPhysicalDevice(context);
    createLogicalDevice();
    initVMA(context);
}

Device::~Device() {
    if (allocator_) {
        vmaDestroyAllocator(allocator_);
        allocator_ = VK_NULL_HANDLE;
    }
    if (queryPool_) {
        vkDestroyQueryPool(vkbDevice_.device, queryPool_, nullptr);
        queryPool_ = VK_NULL_HANDLE;
    }
    if (vkbDevice_.device) {
        vkb::destroy_device(vkbDevice_);
    }
}

void Device::waitIdle() {
    if (vkbDevice_.device) {
        vkDeviceWaitIdle(vkbDevice_.device);
    }
}

void Device::selectPhysicalDevice(VulkanContext* context) {
    vkb::PhysicalDeviceSelector selector(context->getInstance());
    
    selector.set_minimum_version(1, 4)
            .defer_surface_initialization();

    if (context->getSurface()) {
        selector.set_surface(context->getSurface());
    }

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan14Features features14{};
    features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    features14.pipelineRobustness = VK_TRUE;
    features14.hostImageCopy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.timelineSemaphore = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.drawIndirectCount = VK_TRUE;

    selector.set_required_features_12(features12)
            .set_required_features_13(features13)
            .set_required_features_14(features14)
            .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    selector.add_required_extension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    // mesh shaders, rt pipeline, descriptor buffer, unified image layouts, shader object
    
    auto phys_ret = selector.select();
    if (!phys_ret) {
        LOG_CRITICAL("Failed to select physical device: {}", phys_ret.error().message());
        core::Platform::triggerBreakpoint();
    }
    vkb::PhysicalDevice physDevice = phys_ret.value();

    // Check blocklist
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physDevice.physical_device, &props);
    if (DriverBlocklist::IsDeviceBlocklisted(props.vendorID, props.deviceID, props.deviceName)) {
        LOG_CRITICAL("Selected GPU is blocklisted via DriverBlocklist!");
        core::Platform::triggerBreakpoint();
    }
    
    // Optional extensions for M1 capabilities — conditionally enabled based on physical device support.
    // Extensions listed here are requested only if present; absence is not fatal.
    // forceTier0 callers will still build but checkCapabilities() will zero caps_ at runtime.
    physDevice.enable_extension_if_present(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    physDevice.enable_extension_if_present(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    physDevice.enable_extension_if_present(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    physDevice.enable_extension_if_present(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    physDevice.enable_extension_if_present(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
    
    LOG_INFO("Selected GPU: {}", physDevice.name);
    
    vkbDevice_.physical_device = physDevice;
}

void Device::createLogicalDevice() {
    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.timelineSemaphore = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.drawIndirectCount = VK_TRUE;

    features13.pNext = &features12;
    void** pNextChain = &features12.pNext;

    vkb::DeviceBuilder deviceBuilder{vkbDevice_.physical_device};

    auto available_extensions = vkbDevice_.physical_device.get_extensions();

    // 1. Descriptor Buffer Extension Gating
    bool supportsDescBuffer = false;
    for (const auto& ext : available_extensions) {
        if (ext == VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) {
            supportsDescBuffer = true;
            break;
        }
    }
    // Extensions were already conditionally enabled in selectPhysicalDevice via enable_extension_if_present.
    // DeviceBuilder has no add_extension API — extensions flow through the PhysicalDevice selection.
    // We only need to chain the feature structs here if the extension was actually enabled.
    VkPhysicalDeviceDescriptorBufferFeaturesEXT descBufferFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT};
    if (supportsDescBuffer && !core::Config::get().forceTier0) {
        descBufferFeatures.descriptorBuffer = VK_TRUE;
        *pNextChain = &descBufferFeatures;
        pNextChain = &descBufferFeatures.pNext;
    }

    // 2. Shader Object Extension Gating
    bool supportsShaderObject = false;
    for (const auto& ext : available_extensions) {
        if (ext == VK_EXT_SHADER_OBJECT_EXTENSION_NAME) {
            supportsShaderObject = true;
            break;
        }
    }
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT};
    if (supportsShaderObject && !core::Config::get().forceTier0) {
        shaderObjectFeatures.shaderObject = VK_TRUE;
        *pNextChain = &shaderObjectFeatures;
        pNextChain = &shaderObjectFeatures.pNext;
    }

    // 3. Unified Image Layouts KHR Extension Gating
    bool supportsUnifiedLayouts = false;
    for (const auto& ext : available_extensions) {
        if (ext == VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) {
            supportsUnifiedLayouts = true;
            break;
        }
    }
    VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedLayoutFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR};
    if (supportsUnifiedLayouts && !core::Config::get().forceTier0) {
        unifiedLayoutFeatures.unifiedImageLayouts = VK_TRUE;
        *pNextChain = &unifiedLayoutFeatures;
        pNextChain = &unifiedLayoutFeatures.pNext;
    }

    deviceBuilder.add_pNext(&features13);
    
    // Check capabilities here and enable features
    checkCapabilities();
    
    auto dev_ret = deviceBuilder.build();
    if (!dev_ret) {
        LOG_CRITICAL("Failed to create logical device: {}", dev_ret.error().message());
        core::Platform::triggerBreakpoint();
    }
    
    vkbDevice_ = dev_ret.value();
    volkLoadDevice(vkbDevice_.device);
    
    auto graphicsQueueRet = vkbDevice_.get_queue(vkb::QueueType::graphics);
    if (graphicsQueueRet) {
        graphicsQueue_ = graphicsQueueRet.value();
        graphicsQueueIndex_ = vkbDevice_.get_queue_index(vkb::QueueType::graphics).value();
    }
    
    // [M0-EXT-14] Async Compute Queue Separation
    auto computeQueueRet = vkbDevice_.get_queue(vkb::QueueType::compute);
    if (computeQueueRet) {
        computeQueue_ = computeQueueRet.value();
        computeQueueIndex_ = vkbDevice_.get_queue_index(vkb::QueueType::compute).value();
        LOG_INFO("Async Compute queue established at family index {}", computeQueueIndex_);
    } else {
        LOG_WARN("Dedicated compute queue not found, falling back to graphics queue");
        computeQueue_ = graphicsQueue_;
        computeQueueIndex_ = graphicsQueueIndex_;
    }

    // Check timestamp support now that graphicsQueueIndex_ is known
    VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    props2.pNext = &descriptorBufferProperties_;
    vkGetPhysicalDeviceProperties2(vkbDevice_.physical_device.physical_device, &props2);
    
    float timestampPeriod = props2.properties.limits.timestampPeriod;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkbDevice_.physical_device.physical_device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkbDevice_.physical_device.physical_device, &queueFamilyCount, queueFamilyProps.data());
    
    uint32_t timestampValidBits = 0;
    if (graphicsQueueIndex_ < queueFamilyProps.size()) {
        timestampValidBits = queueFamilyProps[graphicsQueueIndex_].timestampValidBits;
    }

    if (!core::Config::get().forceTier0 && props2.properties.limits.timestampPeriod > 0.0f && timestampValidBits > 0) {
        caps_.queryTimestamps = true;
        timestampPeriod_ = props2.properties.limits.timestampPeriod;
    } else {
        caps_.queryTimestamps = false;
        if (!core::Config::get().forceTier0) {
            LOG_WARN("GPU timestamps not supported: timestampPeriod = {}, timestampValidBits = {}", props2.properties.limits.timestampPeriod, timestampValidBits);
        }
    }

    if (caps_.queryTimestamps) {
        VkQueryPoolCreateInfo queryPoolInfo = {};
        queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        // RenderGraph::MAX_PASSES (16) * 2 queries per pass * 3 max frames in flight = 96
        queryPoolInfo.queryCount = 16 * 2 * 3; 

        if (vkCreateQueryPool(vkbDevice_.device, &queryPoolInfo, nullptr, &queryPool_) != VK_SUCCESS) {
            LOG_WARN("Failed to create timestamp query pool despite capability being reported.");
            caps_.queryTimestamps = false;
        } else {
            LOG_INFO("Created timestamp query pool with capacity {}", queryPoolInfo.queryCount);
            setDebugObjectName(VK_OBJECT_TYPE_QUERY_POOL, (uint64_t)queryPool_, "Timestamp Query Pool");
        }
    }

    if (graphicsQueue_) {
        setDebugObjectName(VK_OBJECT_TYPE_QUEUE, (uint64_t)graphicsQueue_, "Graphics Queue");
    }
    if (computeQueue_) {
        setDebugObjectName(VK_OBJECT_TYPE_QUEUE, (uint64_t)computeQueue_, "Compute Queue");
    }
}

void Device::initVMA(VulkanContext* context) {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = vkbDevice_.physical_device.physical_device;
    allocatorInfo.device = vkbDevice_.device;
    allocatorInfo.instance = context->getInstance().instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    
    if (vmaCreateAllocator(&allocatorInfo, &allocator_) != VK_SUCCESS) {
        LOG_CRITICAL("Failed to create VMA allocator");
        core::Platform::triggerBreakpoint();
    }
    LOG_INFO("VMA Allocator initialized");
    
    // Log Memory Budget using [M0-EXT-05]
    VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps{};
    budgetProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
    
    VkPhysicalDeviceMemoryProperties2 memProps2{};
    memProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    memProps2.pNext = &budgetProps;
    
    vkGetPhysicalDeviceMemoryProperties2(vkbDevice_.physical_device.physical_device, &memProps2);
    
    for (uint32_t i = 0; i < memProps2.memoryProperties.memoryHeapCount; ++i) {
        if (memProps2.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            float headroom = GetHeapHeadroom(budgetProps, i);
            LOG_INFO("Device Local Heap {} Headroom: {} MB", i, headroom / (1024.0f * 1024.0f));
        }
    }
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vkbDevice_.physical_device.physical_device, &props);
}

void Device::checkCapabilities() {
    if (core::Config::get().forceTier0) {
        LOG_INFO("Forcing Tier-0 Capabilities");
        caps_ = CapabilityTier{}; // all false
        return;
    }
    
    caps_.isTier0 = false; 
    
    auto available_extensions = vkbDevice_.physical_device.get_extensions();
    bool hasUnifiedExtension = false;
    for (const auto& ext : available_extensions) {
        if (ext == VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME) caps_.graphicsPipelineLibrary = true;
        if (ext == VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) caps_.descriptorBuffer = true;
        if (ext == VK_EXT_MESH_SHADER_EXTENSION_NAME) caps_.meshShaders = true;
        if (ext == VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME) caps_.attachmentFeedbackLoop = true;
        if (ext == VK_EXT_SHADER_OBJECT_EXTENSION_NAME) caps_.shaderObject = true;
        if (ext == VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) hasUnifiedExtension = true;
    }
    
    if (hasUnifiedExtension) {
        VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedLayoutFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR};
        VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        features2.pNext = &unifiedLayoutFeatures;
        vkGetPhysicalDeviceFeatures2(vkbDevice_.physical_device.physical_device, &features2);
        caps_.unifiedImageLayouts = (unifiedLayoutFeatures.unifiedImageLayouts == VK_TRUE);
    } else {
        caps_.unifiedImageLayouts = false;
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vkbDevice_.physical_device.physical_device, &props);

    // [M0] 8K Texture / Multi-RT Guarantee Checks
    ENGINE_ASSERT(props.limits.maxImageDimension2D >= 8192, "GPU maxImageDimension2D limit ({}) is less than 8K (8192)!", props.limits.maxImageDimension2D);
    ENGINE_ASSERT(props.limits.maxColorAttachments >= 8, "GPU maxColorAttachments limit ({}) is less than 8!", props.limits.maxColorAttachments);
    LOG_INFO("VERIFICATION SUCCESS: 8K maxImageDimension2D limit and multi-RT (maxColorAttachments >= 8) verified.");

    if (vkbDevice_.physical_device.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        LOG_WARN("Discrete GPU topology not found! Performance may be degraded.");
    }
}

void Device::setDebugObjectName(VkObjectType objectType, uint64_t objectHandle, const char* name) {
    if (!name) return;
    
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = objectHandle;
    nameInfo.pObjectName = name;
    
    if (vkSetDebugUtilsObjectNameEXT) {
        vkSetDebugUtilsObjectNameEXT(vkbDevice_.device, &nameInfo);
    }
}

} // namespace render
