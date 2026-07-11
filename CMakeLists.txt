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
find_path(STB_INCLUDE_DIR "stb_image.h")

find_program(GLSLC glslc REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INC_FILES "src/*.h")
list(APPEND SRC_FILES "src/debug/imgui_impl_vulkan.cpp")

# Compile shaders
set(SHADERS simple.vert simple.frag cull.comp depth_pyramid.comp)
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
)

target_include_directories(ZombieEngine PRIVATE ${STB_INCLUDE_DIR})

# Vulkan validation layers ON in Debug and OFF in Release
target_compile_definitions(ZombieEngine PRIVATE
    IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING=1
    IMGUI_IMPL_VULKAN_USE_VOLK=1
    $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
    $<$<CONFIG:Debug>:ENGINE_DEV_TOOLS=1>
    $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
    SHADER_DIR=\"${CMAKE_BINARY_DIR}/shaders\"
)