include(FetchContent)
option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
option(BENCHMARK_INSTALL_DOCS "Enable installation of documentation." OFF)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY    https://github.com/google/benchmark.git
    GIT_TAG           v1.9.1
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 1.9.1
)
FetchContent_MakeAvailable(benchmark)
if(TARGET benchmark)
    set_target_properties(benchmark benchmark_main PROPERTIES CXX_CLANG_TIDY "")
endif()

add_subdirectory(microbenchmarks)
add_subdirectory(sdks)
