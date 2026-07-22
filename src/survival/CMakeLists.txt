set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/BodyTemp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StaminaSystem.cpp
)

add_library(ze-survival OBJECT ${SOURCE_FILES})
target_link_libraries(ze-survival PUBLIC ze-deps)
target_include_directories(ze-survival PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-survival PUBLIC ENGINE_DEV_TOOLS)
