set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/EventBus.cpp
)

add_library(ze-events OBJECT ${SOURCE_FILES})
target_link_libraries(ze-events PUBLIC ze-deps)
target_include_directories(ze-events PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-events PUBLIC ENGINE_DEV_TOOLS)
