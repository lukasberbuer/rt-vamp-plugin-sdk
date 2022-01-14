cmake_minimum_required(VERSION 3.10)

project(rtvamp CXX)
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

set(PROJECT_OPTIONS "${PROJECT_NAME}_project_options")
add_library(${PROJECT_OPTIONS} INTERFACE)

target_compile_features(${PROJECT_OPTIONS} INTERFACE cxx_std_20)
target_include_directories(${PROJECT_OPTIONS} INTERFACE 3rdparty)

if(MSVC)
    target_compile_options(
        ${PROJECT_OPTIONS}
        INTERFACE
            /W4
            /permissive-
            # disabled warnings:
            /wd4100  # unreferenced parameter
            /wd4996  # use _dupenv_s instead of std::getenv
    )
else()
    target_compile_options(
        ${PROJECT_OPTIONS}
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

option(RTVAMP_ENABLE_COVERAGE ON)
if(RTVAMP_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${PROJECT_OPTIONS} INTERFACE --coverage -O0 -g)
        target_link_libraries(${PROJECT_OPTIONS} INTERFACE --coverage)
    endif()
endif()

include(cmake/Conan.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)  # for find_package if Conan disabled

option(RTVAMP_ENABLE_TESTS "Enable tests" OFF)
if(RTVAMP_ENABLE_TESTS)
    message(STATUS "Tests enabled")
    find_package(Catch2 REQUIRED)
    include(cmake/Tests.cmake)
    enable_testing()
endif()

add_subdirectory(hostsdk)
add_subdirectory(pluginsdk)
    
option(RTVAMP_ENABLE_BENCHMARKS "Build benchmarks" OFF)
if(RTVAMP_ENABLE_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif()
    
option(RTVAMP_ENABLE_EXAMPLES "Build examples" OFF)
if(RTVAMP_ENABLE_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()
    
option(RTVAMP_BUILD_DOCUMENTATION "Build documentation" OFF)
if(RTVAMP_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()
