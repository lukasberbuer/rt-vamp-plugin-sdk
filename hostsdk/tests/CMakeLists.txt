add_executable(
    tests_hostsdk
    PluginHostAdapter.cpp
)

target_link_libraries(
    tests_hostsdk
    PRIVATE
        ${PROJECT_OPTIONS}
        ${LIBRARY_NAME}
        Catch2::Catch2WithMain
)
target_include_directories(tests_hostsdk PRIVATE ../src)

discover_and_run_tests(tests_hostsdk)
