cmake_minimum_required(VERSION 3.20)
project(ZombieEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(vulkan_memory_allocator CONFIG REQUIRED)

# Find glslc for shader compilation
find_program(GLSLC glslc REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Compile shaders
set(SHADERS simple.vert simple.frag)
set(SPV_FILES "")
foreach(SHADER ${SHADERS})
    set(INPUT "${CMAKE_SOURCE_DIR}/shaders/${SHADER}")
    set(OUTPUT "${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv")
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${GLSLC} ${INPUT} -o ${OUTPUT}
        DEPENDS ${INPUT}
        COMMENT "Compiling shader ${SHADER}"
    )
    list(APPEND SPV_FILES ${OUTPUT})
endforeach()

add_executable(ZombieEngine ${SRC_FILES} ${SPV_FILES})
target_include_directories(ZombieEngine PRIVATE src)

target_link_libraries(ZombieEngine PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    glm::glm
    EnTT::EnTT
    GPUOpen::vulkan_memory_allocator
)

# Vulkan validation layers ON in Debug and OFF in Release
target_compile_definitions(ZombieEngine PRIVATE
    $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
    $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
    SHADER_DIR=\"${CMAKE_BINARY_DIR}/shaders\"
)