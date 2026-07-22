set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/NetworkManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StateSync.cpp
)

add_library(ze-net OBJECT ${SOURCE_FILES})
target_link_libraries(ze-net PUBLIC ze-deps)
target_include_directories(ze-net PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-net PUBLIC ENGINE_DEV_TOOLS)
