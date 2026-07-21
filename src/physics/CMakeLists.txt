set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/PhysicsDebugRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PhysicsSystem.cpp
)

add_library(ze-physics OBJECT ${SOURCE_FILES})
target_link_libraries(ze-physics PUBLIC ze-deps)
target_include_directories(ze-physics PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-physics PUBLIC ENGINE_DEV_TOOLS)
