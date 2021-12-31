add_executable(benchmark_pluginsdks main.cpp)
target_link_libraries(
    benchmark_pluginsdks
    PRIVATE
        ${PROJECT_OPTIONS}
        benchmark::benchmark
        rtvamp-pluginsdk
        vamp-plugin-sdk::vamp-sdk
)
