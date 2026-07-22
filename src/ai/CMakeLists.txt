set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/AIDirector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScentGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZombieFSM.cpp
)

add_library(ze-ai OBJECT ${SOURCE_FILES})
target_link_libraries(ze-ai PUBLIC ze-deps)
target_include_directories(ze-ai PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-ai PUBLIC ENGINE_DEV_TOOLS)
