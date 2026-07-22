set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioPropagation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioVoice.cpp
)

add_library(ze-audio OBJECT ${SOURCE_FILES})
target_link_libraries(ze-audio PUBLIC ze-deps)
target_include_directories(ze-audio PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-audio PUBLIC ENGINE_DEV_TOOLS)
