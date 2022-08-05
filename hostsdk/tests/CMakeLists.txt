add_library(invalid-plugin SHARED InvalidPlugin.cpp)
set_target_properties(invalid-plugin PROPERTIES PREFIX "")

add_executable(
    tests_hostsdk
    Common.cpp
    DynamicLibrary.cpp
    hostsdk.cpp
    PluginHostAdapter.cpp
    PluginKey.cpp
    PluginLibrary.cpp
)

target_link_libraries(
    tests_hostsdk
    PRIVATE
        ${project_options}
        ${library_name}
        Catch2::Catch2WithMain
)
target_include_directories(tests_hostsdk PRIVATE ../src)

if(NOT RTVAMP_BUILD_EXAMPLES)
    message(
        FATAL_ERROR
        "Examples must be enabled for hostsdk tests (activate RTVAMP_BUILD_EXAMPLES option)"
    )
endif()
add_dependencies(tests_hostsdk example-plugin invalid-plugin)

discover_and_run_tests(tests_hostsdk)
