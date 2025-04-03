configure_file(
    "config.hpp.in"
    "${PROJECT_BINARY_DIR}/hostsdk/tests/config.hpp"
    @ONLY
)

add_library(invalid-plugin SHARED InvalidPlugin.cpp)
set_target_properties(invalid-plugin PROPERTIES PREFIX "")

add_executable(
    tests_hostsdk
    DynamicLibrary.cpp
    hostsdk.cpp
    PluginHostAdapter.cpp
    PluginKey.cpp
    PluginLibrary.cpp
)
target_link_libraries(
    tests_hostsdk
    PRIVATE
        rtvamp_project_options
        rtvamp::hostsdk
        Catch2::Catch2WithMain
)
target_include_directories(
    tests_hostsdk
    PRIVATE
        ../src
        "${PROJECT_BINARY_DIR}/hostsdk/tests"
)
set_target_properties(tests_hostsdk PROPERTIES CXX_CLANG_TIDY "")

if(NOT RTVAMP_BUILD_EXAMPLES)
    message(
        FATAL_ERROR
        "Examples must be enabled for hostsdk tests (activate RTVAMP_BUILD_EXAMPLES option)"
    )
endif()
add_dependencies(tests_hostsdk example-plugin invalid-plugin)

catch_discover_tests(tests_hostsdk)
