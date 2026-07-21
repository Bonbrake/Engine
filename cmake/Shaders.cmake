set(SHADERS simple.vert simple.frag simple_mesh.vert cull.comp depth_pyramid.comp tonemap.vert tonemap.frag)
set(SHADER_SPV_FILES "")
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
    list(APPEND SHADER_SPV_FILES ${OUTPUT})
endforeach()
