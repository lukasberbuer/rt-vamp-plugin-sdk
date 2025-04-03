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
        ${project_options}
        ${library_name}
        Catch2::Catch2WithMain
        Threads::Threads
)
set_target_properties(tests_pluginsdk PROPERTIES CXX_CLANG_TIDY "")

catch_discover_tests(tests_pluginsdk)
