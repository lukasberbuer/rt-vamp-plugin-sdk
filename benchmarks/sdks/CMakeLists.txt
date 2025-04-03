find_package(vamp-plugin-sdk)

if(vamp-plugin-sdk_FOUND)
    add_executable(benchmark_hostsdks benchmark_hostsdks.cpp)
    target_link_libraries(
        benchmark_hostsdks
        PRIVATE
            rtvamp_project_options
            rtvamp::hostsdk
            benchmark::benchmark
            vamp-plugin-sdk::vamp-hostsdk
    )

    add_executable(benchmark_pluginsdks benchmark_pluginsdks.cpp)
    target_link_libraries(
        benchmark_pluginsdks
        PRIVATE
            rtvamp_project_options
            rtvamp::pluginsdk
            benchmark::benchmark
            vamp-plugin-sdk::vamp-sdk
    )

    add_executable(benchmark_sdks benchmark_sdks.cpp RMS.cpp)
    target_link_libraries(
        benchmark_sdks
        PRIVATE
            rtvamp_project_options
            rtvamp::pluginsdk
            rtvamp::hostsdk
            benchmark::benchmark
            vamp-plugin-sdk::vamp-sdk
            vamp-plugin-sdk::vamp-hostsdk
    )
else()
    message(WARNING "SDK benchmarks won't be built because vamp-plugin-sdk was not found")
endif()
