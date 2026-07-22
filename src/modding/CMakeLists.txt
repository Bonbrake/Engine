set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Modding.cpp
)

add_library(ze-modding OBJECT ${SOURCE_FILES})
target_link_libraries(ze-modding PUBLIC ze-deps)
target_include_directories(ze-modding PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-modding PUBLIC ENGINE_DEV_TOOLS)
