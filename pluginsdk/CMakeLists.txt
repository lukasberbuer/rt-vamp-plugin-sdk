set(LIBRARY_NAME rtvamp-pluginsdk)
set(ALIAS_NAME "${PROJECT_NAME}::pluginsdk")

add_library(${LIBRARY_NAME} INTERFACE)  # header-only
add_library(${ALIAS_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(
    ${LIBRARY_NAME}
    INTERFACE
        ${PROJECT_OPTIONS}
)
target_include_directories(${LIBRARY_NAME} INTERFACE include)

if(RTVAMP_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
