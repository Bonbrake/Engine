set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ActionState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BodycamCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CVarSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GamepadController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JobSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksManager.cpp
)

add_library(ze-core OBJECT ${SOURCE_FILES})
target_link_libraries(ze-core PUBLIC ze-deps)
target_include_directories(ze-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-core PUBLIC ENGINE_DEV_TOOLS GLM_ENABLE_EXPERIMENTAL)
