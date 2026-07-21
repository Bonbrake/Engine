enable_testing()
file(GLOB_RECURSE TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/unit/*.cpp")
if(TEST_SRC_FILES)
    add_executable(ZombieEngineTests ${TEST_SRC_FILES})
    
    # Link game sources
    set(ZE_SUBSYSTEMS core debug ecs events modding net physics render save slm ui world)
    foreach(subsystem IN LISTS ZE_SUBSYSTEMS)
        target_link_libraries(ZombieEngineTests PRIVATE ze-${subsystem})
    endforeach()
    
    target_include_directories(ZombieEngineTests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${STB_INCLUDE_DIR}
    )
    
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
    
    add_test(NAME ZombieEngineTests COMMAND ZombieEngineTests)
endif()
