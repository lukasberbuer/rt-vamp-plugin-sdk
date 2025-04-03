add_library(rtvamp_pluginsdk INTERFACE)  # header-only
add_library(rtvamp::pluginsdk ALIAS rtvamp_pluginsdk)

target_link_libraries(
    rtvamp_pluginsdk
    INTERFACE
        rtvamp_project_options
)

if(RTVAMP_ENABLE_AMALGAMATION)
    find_program(QUOM_EXE quom REQUIRED)

    set(amalgamation_include_dir ${PROJECT_BINARY_DIR}/amalgamation/pluginsdk/include)
    file(MAKE_DIRECTORY ${amalgamation_include_dir}/rtvamp)

    add_custom_target(
        rtvamp_pluginsdk_amalgamation
        COMMAND
            ${QUOM_EXE}
            ${CMAKE_CURRENT_SOURCE_DIR}/include/rtvamp/pluginsdk.hpp
            ${amalgamation_include_dir}/rtvamp/pluginsdk.hpp
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            -I ${PROJECT_SOURCE_DIR}/3rdparty
        COMMENT "Generate single header pluginsdk.hpp"
    )

    add_dependencies(rtvamp_pluginsdk rtvamp_pluginsdk_amalgamation)
    target_include_directories(rtvamp_pluginsdk INTERFACE ${amalgamation_include_dir})
else()
    target_include_directories(rtvamp_pluginsdk INTERFACE include)
endif()

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
