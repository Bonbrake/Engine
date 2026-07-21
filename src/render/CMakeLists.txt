set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/AssetManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DescriptorSlotAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MSDFPipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MaterialSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineCacheManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineCompatValidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TriangleRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanContext.cpp
)

add_library(ze-render OBJECT ${SOURCE_FILES})
target_link_libraries(ze-render PUBLIC ze-deps)
target_include_directories(ze-render PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-render PUBLIC ENGINE_DEV_TOOLS)
