add_executable(benchmark_hostsdks main.cpp)
target_link_libraries(
    benchmark_hostsdks
    PRIVATE
        ${PROJECT_OPTIONS}
        benchmark::benchmark
        rtvamp-hostsdk
        vamp-plugin-sdk::vamp-hostsdk
)
