add_executable(
    tests_plugin
    Plugin.cpp
    PluginAdapter.cpp
    VampFeatureWrapper.cpp
    VampOutputDescriptorWrapper.cpp
)

target_link_libraries(
    tests_plugin
    PRIVATE
        ${LIBRARY_NAME}
        ${PROJECT_OPTIONS}
        Catch2::Catch2WithMain
)
target_include_directories(tests_plugin PRIVATE ../src)

discover_and_run_tests(tests_plugin)
