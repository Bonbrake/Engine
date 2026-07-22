set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/SaveSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SchemaVersion.cpp
)

add_library(ze-save OBJECT ${SOURCE_FILES})
target_link_libraries(ze-save PUBLIC ze-deps)
target_include_directories(ze-save PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-save PUBLIC ENGINE_DEV_TOOLS)
