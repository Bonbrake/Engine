set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/HapticManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HUD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MSDFText.cpp
)

add_library(ze-ui OBJECT ${SOURCE_FILES})
target_link_libraries(ze-ui PUBLIC ze-deps)
target_include_directories(ze-ui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-ui PUBLIC ENGINE_DEV_TOOLS)
