set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/PromptTemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SLMClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesPromptFormatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesToolParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesGrammar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesContext.cpp
)

add_library(ze-slm OBJECT ${SOURCE_FILES})
target_link_libraries(ze-slm PUBLIC ze-deps)
target_include_directories(ze-slm PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(ze-slm PUBLIC ENGINE_DEV_TOOLS)
