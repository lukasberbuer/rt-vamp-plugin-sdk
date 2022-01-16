set(library_name rtvamp-pluginsdk)
set(alias_name "${PROJECT_NAME}::pluginsdk")

add_library(${library_name} INTERFACE)  # header-only
add_library(${alias_name} ALIAS ${library_name})

target_link_libraries(
    ${library_name}
    INTERFACE
        ${project_options}
)

if(RTVAMP_ENABLE_AMALGAMATION)
    find_program(QUOM_EXE quom REQUIRED)

    set(amalgamation_include_dir ${PROJECT_BINARY_DIR}/amalgamation/pluginsdk/include)
    file(MAKE_DIRECTORY ${amalgamation_include_dir}/rtvamp)

    add_custom_target(
        rtvamp-pluginsdk-amalgamation
        COMMAND
            ${QUOM_EXE}
            ${CMAKE_CURRENT_SOURCE_DIR}/include/rtvamp/pluginsdk.hpp
            ${amalgamation_include_dir}/rtvamp/pluginsdk.hpp
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            -I ${PROJECT_SOURCE_DIR}/3rdparty
        COMMENT "Generate single header pluginsdk.hpp"
    )

    add_dependencies(${library_name} rtvamp-pluginsdk-amalgamation)
    target_include_directories(${library_name} INTERFACE ${amalgamation_include_dir})
else()
    target_include_directories(${library_name} INTERFACE include)
endif()

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
