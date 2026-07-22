set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/PromptTemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SLMClient.cpp
)

add_library(ze-slm OBJECT ${SOURCE_FILES})
target_link_libraries(ze-slm PUBLIC ze-deps)
target_include_directories(ze-slm PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-slm PUBLIC ENGINE_DEV_TOOLS)
