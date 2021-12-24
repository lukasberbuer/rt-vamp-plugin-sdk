# add_executable(
#     tests_host
#     Plugin.cpp
#     PluginAdapter.cpp
#     VampFeatureWrapper.cpp
#     VampOutputDescriptorWrapper.cpp
# )

# target_link_libraries(
#     tests_plugin
#     PRIVATE
#         ${PROJECT_OPTIONS}
#         ${LIBRARY_NAME}
#         Catch2::Catch2WithMain
# )
# target_include_directories(tests_plugin PRIVATE ../src)

# discover_and_run_tests(tests_plugin)
