cmake_minimum_required(VERSION 3.20)
project(ZombieEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(enkiTS CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(msdfgen CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIR "stb_image.h")
find_package(Tracy CONFIG REQUIRED)

option(ENGINE_ENABLE_PROFILING "Enable Tracy Profiling" OFF)

# Sanitizer strategy (toolchain revision 2026-07-12):
#   Windows local  -> MSVC native ASan only (fast smoke check, no ClangCL needed).
#   WSL2 / Linux CI -> clang + ASan/UBSan/TSan (full matrix, authoritative results).
# The Windows ClangCL toolchain requirement was dropped: host OS build 10.0.26220 is
# outside VS2022's supported range, so ClangCL cannot be installed here. WSL2 sidesteps
# the OS/VS wall entirely and mirrors the Linux CI runner. TSan was never viable on
# Windows; it runs under the same WSL2/Linux path as UBSan.
set(ENGINE_SANITIZE "NONE" CACHE STRING "Sanitizer to enable (NONE, ASAN, UBSAN, TSAN)")
set_property(CACHE ENGINE_SANITIZE PROPERTY STRINGS "NONE" "ASAN" "UBSAN" "TSAN")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows only supports ASan locally, and MSVC provides it natively (-fsanitize=address
    # is accepted by cl.exe). No ClangCL requirement. UBSan/TSan are not available on the
    # MSVC toolchain — route them to WSL2/Linux instead of hard-failing here.
    if(ENGINE_SANITIZE STREQUAL "UBSAN" OR ENGINE_SANITIZE STREQUAL "TSAN")
        message(FATAL_ERROR "${ENGINE_SANITIZE} is not available on the Windows/MSVC toolchain. "
            "Run UBSAN/TSAN builds under WSL2 or Linux CI with clang (e.g. `cmake -B build-wsl -S . "
            "-DCMAKE_CXX_COMPILER=clang++ -DENGINE_SANITIZE=${ENGINE_SANITIZE}`).")
    endif()
endif()

if(NOT ENGINE_SANITIZE STREQUAL "NONE")
    if(ENGINE_SANITIZE STREQUAL "ASAN")
        set(SANITIZE_FLAGS "-fsanitize=address")
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            # ASan on Windows (MSVC) does not support debug runtime libraries (-MDd/-MTd)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
            # Workaround for STL container annotation ABI mismatch with uninstrumented vcpkg libs
            add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
        endif()
    elseif(ENGINE_SANITIZE STREQUAL "UBSAN")
        set(SANITIZE_FLAGS "-fsanitize=undefined")
    elseif(ENGINE_SANITIZE STREQUAL "TSAN")
        set(SANITIZE_FLAGS "-fsanitize=thread")
    endif()

    add_compile_options(${SANITIZE_FLAGS})
    add_link_options(${SANITIZE_FLAGS})
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_program(GLSLC glslc REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INC_FILES "src/*.h")
list(APPEND SRC_FILES "src/debug/imgui_impl_vulkan.cpp")

# Compile shaders
set(SHADERS simple.vert simple.frag simple_mesh.vert cull.comp depth_pyramid.comp)
set(SPV_FILES "")
foreach(SHADER ${SHADERS})
    set(INPUT "${CMAKE_SOURCE_DIR}/shaders/${SHADER}")
    set(OUTPUT "${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv")
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${GLSLC} -O ${INPUT} -o ${OUTPUT}
        DEPENDS ${INPUT}
        COMMENT "Compiling shader ${SHADER}"
    )
    list(APPEND SPV_FILES ${OUTPUT})
endforeach()

add_executable(ZombieEngine ${SRC_FILES} ${INC_FILES} ${SPV_FILES})
target_include_directories(ZombieEngine PRIVATE src)
target_compile_definitions(ZombieEngine PRIVATE ENGINE_DEV_TOOLS)

target_link_libraries(ZombieEngine PRIVATE
    SDL3::SDL3
    Vulkan::Headers
    vk-bootstrap::vk-bootstrap
    glm::glm
    volk::volk
    spdlog::spdlog
    enkiTS::enkiTS
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
    Catch2::Catch2
    GPUOpen::VulkanMemoryAllocator
    EnTT::EnTT
    imgui::imgui
    msdfgen::msdfgen-core
    msdfgen::msdfgen-ext
    unofficial::spirv-reflect
    fastgltf::fastgltf
    Jolt::Jolt
)

# [M0-EXT-15] Standalone focus-loss probe: windowed SDL3 only, no engine deps.
# Exercises the production SDL_GetKeyboardState path across a minimize/restore
# cycle — the alt-tab bug class unit tests can't cover.
add_executable(focus_probe phase1_verify/focus_probe.cpp)
target_include_directories(focus_probe PRIVATE src)
target_link_libraries(focus_probe PRIVATE
    SDL3::SDL3
)

target_include_directories(ZombieEngine PRIVATE ${STB_INCLUDE_DIR})

if(ENGINE_ENABLE_PROFILING)
    target_compile_definitions(ZombieEngine PRIVATE TRACY_ENABLE)
    target_link_libraries(ZombieEngine PRIVATE Tracy::TracyClient)
endif()

# Tests
enable_testing()
file(GLOB_RECURSE TEST_SRC_FILES "src/tests/*.cpp")
if(TEST_SRC_FILES)
    add_executable(ZombieEngineTests ${TEST_SRC_FILES})
    
    # We must also link the game sources to the test executable, except main.cpp
    # so we filter it out of SRC_FILES.
    list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")
    
    target_sources(ZombieEngineTests PRIVATE ${SRC_FILES})
    target_include_directories(ZombieEngineTests PRIVATE src ${STB_INCLUDE_DIR})
    
    target_link_libraries(ZombieEngineTests PRIVATE
        SDL3::SDL3
        Vulkan::Headers
        vk-bootstrap::vk-bootstrap
        glm::glm
        volk::volk
        spdlog::spdlog
        enkiTS::enkiTS
        nlohmann_json::nlohmann_json
        cxxopts::cxxopts
        Catch2::Catch2WithMain
        GPUOpen::VulkanMemoryAllocator
        EnTT::EnTT
        imgui::imgui
        msdfgen::msdfgen-core
        msdfgen::msdfgen-ext
        unofficial::spirv-reflect
        fastgltf::fastgltf
        Jolt::Jolt
    )
    
    target_compile_definitions(ZombieEngineTests PRIVATE
        JPH_CROSS_PLATFORM_DETERMINISTIC
        JPH_DOUBLE_PRECISION
    )
    
    include(Catch)
    catch_discover_tests(ZombieEngineTests)
endif()

# Vulkan validation layers ON in Debug and OFF in Release
target_compile_definitions(ZombieEngine PRIVATE
    IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING=1
    IMGUI_IMPL_VULKAN_USE_VOLK=1
    $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
    $<$<CONFIG:Debug>:ENGINE_DEV_TOOLS=1>
    $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
    SHADER_DIR=\"${CMAKE_BINARY_DIR}/shaders\"
    # M2 spec: Jolt must be built with these defines for bit-identical physics across compiler/CPU
    JPH_CROSS_PLATFORM_DETERMINISTIC
    JPH_DOUBLE_PRECISION
    # M2-EXIT debughull: enable Jolt collision-hull overlay. Jolt lib is already built with
    # DEBUG_RENDERER_IN_DEBUG_AND_RELEASE (root vcpkg.json requests the "debugrenderer" feature),
    # so this only flips the engine-side #ifdef blocks (drawBodies + ImGui projection).
    JPH_DEBUG_RENDERER
)

# Static Analysis Target
# We use a custom target so it doesn't block the standard build per the directive
add_custom_target(run_clang_tidy
    COMMAND clang-tidy -p ${CMAKE_BINARY_DIR} ${SRC_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-tidy static analysis"
)