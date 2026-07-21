function(apply_engine_definitions target)
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
endfunction()

function(apply_sanitizer target)
    if(ENGINE_SANITIZE STREQUAL "NONE")
        return()
    endif()
    if(ENGINE_SANITIZE STREQUAL "ASAN")
        set(SANITIZE_FLAGS "-fsanitize=address")
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
            add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
        endif()
    elseif(ENGINE_SANITIZE STREQUAL "UBSAN")
        set(SANITIZE_FLAGS "-fsanitize=undefined")
    elseif(ENGINE_SANITIZE STREQUAL "TSAN")
        set(SANITIZE_FLAGS "-fsanitize=thread")
    endif()
    add_compile_options(${SANITIZE_FLAGS})
    add_link_options(${SANITIZE_FLAGS})
endfunction()
