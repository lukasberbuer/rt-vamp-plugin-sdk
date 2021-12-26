add_executable(
    tests_plugin
    EntryPoint.cpp
    PluginAdapter.cpp
    PluginDefinition.cpp
    VampWrapper.cpp
)

target_link_libraries(
    tests_plugin
    PRIVATE
        ${PROJECT_OPTIONS}
        ${LIBRARY_NAME}
        Catch2::Catch2WithMain
)

discover_and_run_tests(tests_plugin)
