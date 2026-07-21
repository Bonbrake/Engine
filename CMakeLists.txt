cmake_minimum_required(VERSION 3.20)
project(ZombieEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerFlags.cmake)
include(cmake/Dependencies.cmake)
include(cmake/Shaders.cmake)

# Subsystems
add_subdirectory(src/core)
add_subdirectory(src/debug)
add_subdirectory(src/ecs)
add_subdirectory(src/events)
add_subdirectory(src/modding)
add_subdirectory(src/net)
add_subdirectory(src/physics)
add_subdirectory(src/render)
add_subdirectory(src/save)
add_subdirectory(src/slm)
add_subdirectory(src/ui)
add_subdirectory(src/world)

# Main executable
add_executable(ZombieEngine
    src/main.cpp
    ${SHADER_SPV_FILES}
)
target_include_directories(ZombieEngine PRIVATE
    include
    ${STB_INCLUDE_DIR}
)
target_link_libraries(ZombieEngine PRIVATE
    ze-core
    ze-debug
    ze-ecs
    ze-events
    ze-modding
    ze-net
    ze-physics
    ze-render
    ze-save
    ze-slm
    ze-ui
    ze-world
    SDL3::SDL3
    Vulkan::Headers
    vk-bootstrap::vk-bootstrap
    glm::glm
    volk::volk
    spdlog::spdlog
    enkiTS::enkiTS
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
    GPUOpen::VulkanMemoryAllocator
    EnTT::EnTT
    imgui::imgui
    msdfgen::msdfgen-core
    msdfgen::msdfgen-ext
    unofficial::spirv-reflect
    fastgltf::fastgltf
    Jolt::Jolt
)
apply_engine_definitions(ZombieEngine)

# Focus probe
add_executable(focus_probe archive/phase1_verify/focus_probe.cpp)
target_link_libraries(focus_probe PRIVATE SDL3::SDL3)

# Tests
enable_testing()
add_subdirectory(tests)

# Profiling
if(ENGINE_ENABLE_PROFILING)
    target_link_libraries(ZombieEngine PRIVATE Tracy::TracyClient)
endif()

# Install rules
include(GNUInstallDirs)
install(TARGETS ZombieEngine DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS ZombieEngineTests DESTINATION ${CMAKE_INSTALL_BINDIR})

# Register top-level test
add_test(NAME ZombieEngineTests COMMAND ZombieEngineTests)
