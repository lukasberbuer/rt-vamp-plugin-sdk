set(library_name rtvamp-pluginsdk)
set(alias_name "${PROJECT_NAME}::pluginsdk")

add_library(${library_name} INTERFACE)  # header-only
add_library(${alias_name} ALIAS ${library_name})

target_link_libraries(
    ${library_name}
    INTERFACE
        ${project_options}
)
target_include_directories(${library_name} INTERFACE include)

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
