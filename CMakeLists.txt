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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
# find packages installed with Conan
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

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

include(cmake/Sanitizers.cmake)

option(RTVAMP_ENABLE_TESTS "Enable tests" OFF)
if(RTVAMP_ENABLE_TESTS)
    message(STATUS "Tests enabled")

    find_package(Catch2)  # target Catch2::Catch2 or Catch2::Catch2WithMain
    if(NOT Catch2_FOUND)
        message(STATUS "Install catch2")
        set(CATCH_BUILD_STATIC_LIBRARY ON CACHE INTERNAL "")  # enable Catch2WithMain target
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v2.13.7
        )
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
    endif()

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
