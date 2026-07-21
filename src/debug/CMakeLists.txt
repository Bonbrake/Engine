set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/FlyCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiOverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MetaRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_impl_vulkan.cpp
)

add_library(ze-debug OBJECT ${SOURCE_FILES})
target_link_libraries(ze-debug PUBLIC ze-deps)
target_include_directories(ze-debug PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-debug PUBLIC ENGINE_DEV_TOOLS)
