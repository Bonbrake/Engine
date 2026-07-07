cmake_minimum_required(VERSION 3.20)
project(ZombieEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(joltphysics CONFIG REQUIRED)

add_executable(ZombieEngine src/core/Application.cpp)

target_link_libraries(ZombieEngine PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    glm::glm
    EnTT::EnTT
    JoltPhysics::JoltPhysics
)

# Vulkan validation layers ON in Debug and OFF in Release
target_compile_definitions(ZombieEngine PRIVATE
    $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
    $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
)
