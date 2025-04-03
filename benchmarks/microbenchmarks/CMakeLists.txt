file(GLOB sources "*.cpp")
foreach(source ${sources})
    get_filename_component(name ${source} NAME_WE)
    set(name "benchmark_${name}")
    add_executable(${name} ${source})
    target_link_libraries(
        ${name}
        PRIVATE
            rtvamp_project_options
            rtvamp::pluginsdk
            rtvamp::hostsdk
            benchmark::benchmark
    )
endforeach()
