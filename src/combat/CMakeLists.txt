set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/DamageTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParrySystem.cpp
)

add_library(ze-combat OBJECT ${SOURCE_FILES})
target_link_libraries(ze-combat PUBLIC ze-deps)
target_include_directories(ze-combat PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-combat PUBLIC ENGINE_DEV_TOOLS)
