set(LIBRARY_NAME rtvamp-hostsdk)
set(ALIAS_NAME "${PROJECT_NAME}::hostsdk")

add_library(
    ${LIBRARY_NAME}
    src/PluginAdapter.cpp
)
add_library(${ALIAS_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(
    ${LIBRARY_NAME}
    PUBLIC
        rtvamp-pluginsdk
    PRIVATE
        ${PROJECT_OPTIONS}
)
target_include_directories(${LIBRARY_NAME} PUBLIC include)

if(RTVAMP_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
