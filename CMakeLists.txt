cmake_minimum_required(VERSION 3.20)
project(ZombieEngine CXX)

# Enforce modern C++ for cleaner syntax
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find vcpkg dependencies
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(joltphysics CONFIG REQUIRED)

# M0 Target
add_executable(ZombieEngine src/core/Application.cpp)

# Link everything 
target_link_libraries(ZombieEngine PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    glm::glm
    EnTT::EnTT
    joltphysics::joltphysics
)