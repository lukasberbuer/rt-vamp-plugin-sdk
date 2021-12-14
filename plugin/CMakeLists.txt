set(LIBRARY_NAME rt-vamp-plugin)
set(ALIAS_NAME "${PROJECT_NAME}::plugin")

add_library(
    ${LIBRARY_NAME}
    src/Plugin.cpp
    src/PluginAdapter.cpp
)
add_library(${ALIAS_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(
    ${LIBRARY_NAME}
    PRIVATE
        ${PROJECT_OPTIONS}
)
target_include_directories(${LIBRARY_NAME} PUBLIC include)

if(RTVAMP_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
