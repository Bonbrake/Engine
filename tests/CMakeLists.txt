enable_testing()
file(GLOB_RECURSE TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/unit/*.cpp")
if(TEST_SRC_FILES)
    add_executable(ZombieEngineTests ${TEST_SRC_FILES})
    
    # Link game sources
    set(ZE_SUBSYSTEMS ai audio core debug ecs events modding net physics render save slm ui world)
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

    # Ensure vcpkg release runtime DLLs are co-located for test execution.
    set(VCPKG_RELEASE_BIN_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/bin")
    if(EXISTS "${VCPKG_RELEASE_BIN_DIR}")
        add_custom_command(TARGET ZombieEngineTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VCPKG_RELEASE_BIN_DIR}/SDL3.dll"
                "$<TARGET_FILE_DIR:ZombieEngineTests>")
        add_custom_command(TARGET ZombieEngineTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VCPKG_RELEASE_BIN_DIR}/enkiTS.dll"
                "${VCPKG_RELEASE_BIN_DIR}/spdlog.dll"
                "${VCPKG_RELEASE_BIN_DIR}/fmt.dll"
                "${VCPKG_RELEASE_BIN_DIR}/msdfgen-core.dll"
                "${VCPKG_RELEASE_BIN_DIR}/msdfgen-ext.dll"
                "${VCPKG_RELEASE_BIN_DIR}/fastgltf.dll"
                "${VCPKG_RELEASE_BIN_DIR}/vulkan-1.dll"
                "$<TARGET_FILE_DIR:ZombieEngineTests>"
            COMMENT "Copying vcpkg release runtime DLLs for test binary")
    endif()

    # Ensure vcpkg debug runtime DLLs are co-located for test discovery/run.
    set(VCPKG_DEBUG_BIN_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/debug/bin")
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        set_target_properties(ZombieEngineTests PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_DIR};%PATH%"
        )
        add_custom_command(TARGET ZombieEngineTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VCPKG_DEBUG_BIN_DIR}/SDL3.dll"
                "$<TARGET_FILE_DIR:ZombieEngineTests>")
        add_custom_command(TARGET ZombieEngineTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VCPKG_DEBUG_BIN_DIR}/Jolt.lib"
                "${VCPKG_DEBUG_BIN_DIR}/Jolt.dll"
                "${VCPKG_DEBUG_BIN_DIR}/spdlogd.dll"
                "${VCPKG_DEBUG_BIN_DIR}/fmtd.dll"
                "${VCPKG_DEBUG_BIN_DIR}/enkiTS.dll"
                "${VCPKG_DEBUG_BIN_DIR}/msdfgen-core.dll"
                "${VCPKG_DEBUG_BIN_DIR}/msdfgen-ext.dll"
                "${VCPKG_DEBUG_BIN_DIR}/fastgltf.dll"
                "${VCPKG_DEBUG_BIN_DIR}/vulkan-1.dll"
                "$<TARGET_FILE_DIR:ZombieEngineTests>"
            COMMENT "Copying vcpkg debug runtime DLLs for test binary")
    endif()

    include(Catch)

    # Test discovery via catch_discover_tests needs the executable to run.
    # That can fail before runtime DLLs are staged, so make discovery best-effort.
    get_target_property(_ze_test_bin_dir ZombieEngineTests RUNTIME_OUTPUT_DIRECTORY)
    if(NOT _ze_test_bin_dir)
        set(_ze_test_bin_dir $<TARGET_FILE_DIR:ZombieEngineTests>)
    endif()
    file(TO_NATIVE_PATH "${_ze_test_bin_dir}/ZombieEngineTests.exe" _ze_test_exe)
    execute_process(
        COMMAND "${_ze_test_exe}" -?
        RESULT_VARIABLE _ze_test_discover_result
        OUTPUT_QUIET ERROR_QUIET
    )
    if(_ze_test_discover_result EQUAL 0)
        catch_discover_tests(ZombieEngineTests)
    else()
        message(STATUS "Catch2 test discovery skipped: test binary failed to launch (result=${_ze_test_discover_result}). Run tests manually after staging debug runtime DLLs.")
    endif()

    add_test(NAME ZombieEngineTests COMMAND ZombieEngineTests)
endif()
