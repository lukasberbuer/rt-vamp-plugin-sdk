cmake_minimum_required(VERSION 3.12)

project(
    rtvamp
    VERSION 0.2.0
    DESCRIPTION "Real-time Vamp plugin SDK for C++20"
    HOMEPAGE_URL "https://github.com/lukasberbuer/rt-vamp-plugin-sdk"
    LANGUAGES CXX
)

include(cmake/StandardProjectSettings.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set same directories for multi-configuration build systems (e.g. MSVC)
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach()

set(project_options "${PROJECT_NAME}_project_options")
add_library(${project_options} INTERFACE)

target_compile_features(${project_options} INTERFACE cxx_std_20)
target_include_directories(${project_options} INTERFACE 3rdparty)

if(MSVC)
    target_compile_options(
        ${project_options}
        INTERFACE
            /W4
            /permissive-
            # disabled warnings:
            /wd4100  # unreferenced parameter
            /wd4996  # use _dupenv_s instead of std::getenv
    )
else()
    target_compile_options(
        ${project_options}
        INTERFACE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wpedantic
            # disabled warnings:
            -Wno-unused-parameter
    )
endif()

option(RTVAMP_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
if(RTVAMP_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${project_options} INTERFACE --coverage -O0 -g)
        target_link_libraries(${project_options} INTERFACE --coverage)
    endif()
endif()

include(cmake/Conan.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)  # for find_package if Conan disabled

option(RTVAMP_BUILD_TESTS "Build and run tests" OFF)
if(RTVAMP_BUILD_TESTS)
    message(STATUS "Tests enabled")
    find_package(Catch2 REQUIRED)
    include(cmake/Tests.cmake)
    enable_testing()
endif()

add_subdirectory(hostsdk)
add_subdirectory(pluginsdk)

option(RTVAMP_ENABLE_AMALGAMATION "Create amalgamated header/source files" OFF)

option(RTVAMP_BUILD_BENCHMARKS "Build benchmarks" OFF)
if(RTVAMP_BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif()
    
option(RTVAMP_BUILD_EXAMPLES "Build examples" OFF)
if(RTVAMP_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()
    
option(RTVAMP_BUILD_DOCUMENTATION "Build documentation" OFF)
if(RTVAMP_BUILD_DOCUMENTATION)
    message(STATUS "Documentation enabled")
    add_subdirectory(doc)
endif()

option(RTVAMP_BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
if(RTVAMP_BUILD_PYTHON_BINDINGS)
    message(STATUS "Python bindings enabled")
    add_subdirectory(python)
endif()
