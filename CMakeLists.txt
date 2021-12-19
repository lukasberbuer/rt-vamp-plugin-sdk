cmake_minimum_required(VERSION 3.10)

project(rt-vamp-plugin-sdk CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PROJECT_OPTIONS "${PROJECT_NAME}_project_options")
add_library(${PROJECT_OPTIONS} INTERFACE)

target_compile_features(${PROJECT_OPTIONS} INTERFACE cxx_std_20)
target_include_directories(${PROJECT_OPTIONS} INTERFACE 3rdparty)

if(MSVC)
    target_compile_options(
        ${PROJECT_OPTIONS}
        INTERFACE
            /permissive-
            /W4
            /w14640
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
    )
endif()

include(cmake/Sanitizers.cmake)

include_directories(3rdparty)

option(RTVAMP_ENABLE_TESTS "Enable tests" OFF)
if(RTVAMP_ENABLE_TESTS)
    message(STATUS "Tests enabled")
    find_package(Catch2 REQUIRED)  # target Catch2::Catch2 or Catch2::Catch2WithMain
    include(cmake/Tests.cmake)
    enable_testing()
endif()
    
option(RTVAMP_ENABLE_EXAMPLES "Build examples" OFF)
if(RTVAMP_ENABLE_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()

add_subdirectory(host)
add_subdirectory(plugin)
