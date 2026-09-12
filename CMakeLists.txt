cmake_minimum_required(VERSION 3.28)
project(EndlessQuarantine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- Target Architecture & vcpkg Triplet Resolution ---
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|ARM64|arm64)" OR CMAKE_GENERATOR_PLATFORM MATCHES "^(ARM64|arm64)")
    set(ZE_TARGET_ARCH "arm64")
else()
    set(ZE_TARGET_ARCH "x64")
endif()

if(WIN32)
    set(VCPKG_TARGET_TRIPLET "${ZE_TARGET_ARCH}-windows")
elseif(APPLE)
    set(VCPKG_TARGET_TRIPLET "${ZE_TARGET_ARCH}-osx")
else()
    set(VCPKG_TARGET_TRIPLET "${ZE_TARGET_ARCH}-linux")
endif()

if(NOT DEFINED ENV{VCPKG_INSTALLED_DIR})
    set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed")
endif()

# Check for target-specific installed directory; fallback to x64-windows for host builds if present
if(EXISTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    set(VCPKG_RESOLVED_TRIPLET "${VCPKG_TARGET_TRIPLET}")
else()
    set(VCPKG_RESOLVED_TRIPLET "x64-windows")
endif()

list(PREPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_RESOLVED_TRIPLET}")

include(cmake/CompilerFlags.cmake)
include(cmake/Dependencies.cmake)
include(cmake/Shaders.cmake)

# Subsystems
add_subdirectory(src)

set(ZE_ALL_SUBSYSTEMS
    ze-ai
    ze-audio
    ze-combat
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
    ze-survival
    ze-ui
    ze-vehicle
    ze-world
)

add_executable(EndlessQuarantine
    src/main.cpp
    ${SHADER_SPV_FILES}
)
target_include_directories(EndlessQuarantine PRIVATE
    include
    ${STB_INCLUDE_DIR}
)
target_link_libraries(EndlessQuarantine PRIVATE
    ${ZE_ALL_SUBSYSTEMS}
    ze-deps
)
apply_engine_definitions(EndlessQuarantine)

# Alias ZombieEngine -> EndlessQuarantine
add_executable(ZombieEngine ALIAS EndlessQuarantine)

# Copy DLLs post-build
if(EXISTS "${VCPKG_INSTALLED_DIR}/${VCPKG_RESOLVED_TRIPLET}/bin")
    add_custom_command(TARGET EndlessQuarantine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VCPKG_INSTALLED_DIR}/${VCPKG_RESOLVED_TRIPLET}/bin"
            "$<TARGET_FILE_DIR:EndlessQuarantine>"
    )
endif()

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sanity_suite_harness/sanity.cpp")
    add_executable(sanity_suite sanity_suite_harness/sanity.cpp)
    target_link_libraries(sanity_suite PRIVATE
        ${ZE_ALL_SUBSYSTEMS}
        ze-deps
    )
    target_include_directories(sanity_suite PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${STB_INCLUDE_DIR}
    )
    apply_engine_definitions(sanity_suite)
    add_test(NAME sanity_suite COMMAND sanity_suite)
    # Runtime DLLs for the build directory are already staged by EndlessQuarantine POST_BUILD
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

