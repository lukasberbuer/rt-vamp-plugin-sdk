add_executable(benchmark_plugin-sdk main.cpp)
target_link_libraries(
    benchmark_plugin-sdk
    PRIVATE
        ${PROJECT_OPTIONS}
        rt-vamp-plugin
        benchmark::benchmark
        vamp-plugin-sdk::vamp-sdk
)
