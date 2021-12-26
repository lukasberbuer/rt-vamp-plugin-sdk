add_executable(benchmark_pluginsdks main.cpp)
target_link_libraries(
    benchmark_pluginsdks
    PRIVATE
        ${PROJECT_OPTIONS}
        rtvamp-pluginsdk
        benchmark::benchmark
        vamp-plugin-sdk::vamp-sdk
)
