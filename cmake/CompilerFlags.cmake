function(apply_engine_definitions target)
    if(ZE_TARGET_ARCH STREQUAL "arm64")
        target_compile_definitions(${target} PRIVATE
            ZE_ARCH_ARM64=1
            GLM_FORCE_INTRINSICS=1
        )
    else()
        target_compile_definitions(${target} PRIVATE
            ZE_ARCH_X64=1
            GLM_FORCE_AVX2=1
        )
    endif()

    target_compile_definitions(${target} PRIVATE
        IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING=1
        IMGUI_IMPL_VULKAN_USE_VOLK=1
        $<$<CONFIG:Debug>:ENABLE_VULKAN_VALIDATION_LAYERS=1>
        $<$<CONFIG:Debug>:ENGINE_DEV_TOOLS=1>
        $<$<NOT:$<CONFIG:Debug>>:ENABLE_VULKAN_VALIDATION_LAYERS=0>
        SHADER_DIR=\"${CMAKE_BINARY_DIR}/shaders\"
        JPH_CROSS_PLATFORM_DETERMINISTIC
        JPH_DOUBLE_PRECISION
        JPH_DEBUG_RENDERER
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /fp:precise)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -ffp-contract=off)
    endif()
endfunction()

function(apply_sanitizer target)
    if(ENGINE_SANITIZE STREQUAL "NONE")
        return()
    endif()
    if(ENGINE_SANITIZE STREQUAL "ASAN")
        set(SANITIZE_FLAGS "-fsanitize=address")
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
            target_compile_definitions(${target} PRIVATE
                _DISABLE_STRING_ANNOTATION
                _DISABLE_VECTOR_ANNOTATION
            )
        endif()
    elseif(ENGINE_SANITIZE STREQUAL "UBSAN")
        set(SANITIZE_FLAGS "-fsanitize=undefined")
    elseif(ENGINE_SANITIZE STREQUAL "TSAN")
        set(SANITIZE_FLAGS "-fsanitize=thread")
    endif()
    target_compile_options(${target} PRIVATE ${SANITIZE_FLAGS})
    target_link_options(${target} PRIVATE ${SANITIZE_FLAGS})
endfunction()
