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
find_program(GLSLC glslc REQUIRED)

add_library(ze-deps INTERFACE)
target_include_directories(ze-deps INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${STB_INCLUDE_DIR}
)
target_compile_definitions(ze-deps INTERFACE ENGINE_DEV_TOOLS)
target_link_libraries(ze-deps INTERFACE
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
