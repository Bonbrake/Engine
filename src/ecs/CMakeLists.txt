set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Destructible.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EntityFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SpatialHash.cpp
)

add_library(ze-ecs OBJECT ${SOURCE_FILES})
target_link_libraries(ze-ecs PUBLIC ze-deps)
target_include_directories(ze-ecs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-ecs PUBLIC ENGINE_DEV_TOOLS)
