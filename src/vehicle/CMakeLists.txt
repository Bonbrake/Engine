set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/VehicleSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WheelSim.cpp
)

add_library(ze-vehicle OBJECT ${SOURCE_FILES})
target_link_libraries(ze-vehicle PUBLIC ze-deps)
target_include_directories(ze-vehicle PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-vehicle PUBLIC ENGINE_DEV_TOOLS)
