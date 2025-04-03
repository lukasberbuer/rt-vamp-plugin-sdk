cmake_minimum_required(VERSION 3.12)

project(
    rtvamp
    VERSION 0.3.1
    DESCRIPTION "Real-time Vamp plugin SDK for C++20"
    HOMEPAGE_URL "https://github.com/lukasberbuer/rt-vamp-plugin-sdk"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_library(rtvamp_project_options INTERFACE)
target_compile_features(rtvamp_project_options INTERFACE cxx_std_20)
target_include_directories(rtvamp_project_options INTERFACE 3rdparty)

if(MSVC)
    target_compile_options(
        rtvamp_project_options
        INTERFACE
            /W4
            /permissive-
            # disabled warnings:
            /wd4100  # unreferenced parameter
            /wd4996  # use _dupenv_s instead of std::getenv
    )
else()
    target_compile_options(
        rtvamp_project_options
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

option(RTVAMP_ENABLE_COVERAGE "Enable coverage reporting for GCC/Clang" OFF)
if(RTVAMP_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(rtvamp_project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(rtvamp_project_options INTERFACE --coverage)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(sanitizers "")
    foreach(san IN ITEMS address leak undefined thread)
        string(TOUPPER ${san} san_upper)
        option(RTVAMP_ENABLE_SANITIZER_${san_upper} "Enable ${san} sanitizer" OFF)
        if(RTVAMP_ENABLE_SANITIZER_${san_upper})
            list(APPEND sanitizers ${san})
        endif()
    endforeach()

    list(JOIN sanitizers "," sanitizers_joined)
    if(sanitizers_joined)
        target_compile_options(rtvamp_project_options INTERFACE -fsanitize=${sanitizers_joined})
        target_link_options(rtvamp_project_options INTERFACE -fsanitize=${sanitizers_joined})
    endif()
endif()

option(RTVAMP_ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(RTVAMP_ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else()
        message(WARNING "clang-tidy requested but executable not found")
    endif()
endif()

option(RTVAMP_ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
if(RTVAMP_ENABLE_INCLUDE_WHAT_YOU_USE)
    find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
    if(INCLUDE_WHAT_YOU_USE)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
    else()
        message(WARNING "include-what-you-use requested but executable not found")
    endif()
endif()

option(RTVAMP_BUILD_TESTS "Build and run tests" OFF)
if(RTVAMP_BUILD_TESTS)
    message(STATUS "Tests enabled")

    include(FetchContent)
    option(CATCH_INSTALL_DOCS "Install documentation alongside library" OFF)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
        GIT_TAG           v3.8.0
        EXCLUDE_FROM_ALL
        SYSTEM
        FIND_PACKAGE_ARGS 3.8.0
    )
    FetchContent_MakeAvailable(Catch2)
    if(TARGET Catch2)
        set_target_properties(Catch2 Catch2WithMain PROPERTIES CXX_CLANG_TIDY "")
    endif()

    enable_testing()
    include(CTest)
    include(Catch)
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
