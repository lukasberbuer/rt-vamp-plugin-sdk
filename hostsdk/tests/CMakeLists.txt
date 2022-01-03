add_library(invalid-plugin SHARED InvalidPlugin.cpp)
set_target_properties(invalid-plugin PROPERTIES PREFIX "")

add_executable(
    tests_hostsdk
    Common.cpp
    PluginHostAdapter.cpp
    PluginLibrary.cpp
    PluginLoader.cpp
)

target_link_libraries(
    tests_hostsdk
    PRIVATE
        ${PROJECT_OPTIONS}
        ${LIBRARY_NAME}
        Catch2::Catch2WithMain
)
target_include_directories(tests_hostsdk PRIVATE ../src)

if(NOT RTVAMP_ENABLE_EXAMPLES)
    message(
        FATAL_ERROR
        "Examples must be enabled for hostsdk tests (activate RTVAMP_ENABLE_EXAMPLES option)"
    )
endif()
add_dependencies(tests_hostsdk example-plugin)

discover_and_run_tests(tests_hostsdk)
