find_package(vamp-plugin-sdk)

if(vamp-plugin-sdk_FOUND)
    add_executable(benchmark_hostsdks benchmark_hostsdks.cpp)
    target_link_libraries(
        benchmark_hostsdks
        PRIVATE
            ${project_options}
            benchmark::benchmark
            rtvamp-hostsdk
            vamp-plugin-sdk::vamp-hostsdk
    )

    add_executable(benchmark_pluginsdks benchmark_pluginsdks.cpp)
    target_link_libraries(
        benchmark_pluginsdks
        PRIVATE
            ${project_options}
            benchmark::benchmark
            rtvamp-pluginsdk
            vamp-plugin-sdk::vamp-sdk
    )

    add_executable(benchmark_sdks benchmark_sdks.cpp RMS.cpp)
    target_link_libraries(
        benchmark_sdks
        PRIVATE
            ${project_options}
            benchmark::benchmark
            rtvamp-pluginsdk
            rtvamp-hostsdk
            vamp-plugin-sdk::vamp-sdk
            vamp-plugin-sdk::vamp-hostsdk
    )
else()
    message(WARNING "SDK benchmarks won't be built because vamp-plugin-sdk was not found")
endif()
