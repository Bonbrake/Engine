set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/BiomeGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ChunkStreamer.cpp
)

add_library(ze-world OBJECT ${SOURCE_FILES})
target_link_libraries(ze-world PUBLIC ze-deps)
target_include_directories(ze-world PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-world PUBLIC ENGINE_DEV_TOOLS GLM_ENABLE_EXPERIMENTAL)
