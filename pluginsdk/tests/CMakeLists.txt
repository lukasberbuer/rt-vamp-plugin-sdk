find_package(Threads REQUIRED)

add_executable(
    tests_pluginsdk
    EntryPoint.cpp
    Plugin.cpp
    PluginAdapter.cpp
    PluginExt.cpp
    VampWrapper.cpp
)

target_link_libraries(
    tests_pluginsdk
    PRIVATE
        ${PROJECT_OPTIONS}
        ${LIBRARY_NAME}
        Catch2::Catch2WithMain
        Threads::Threads
)

discover_and_run_tests(tests_pluginsdk)
