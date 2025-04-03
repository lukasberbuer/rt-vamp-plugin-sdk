find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

if(SKBUILD)
    # scikit-build does not add your site-packages to the search path automatically,
    # so we need to add it _or_ the pybind11 specific directory here
    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -c "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE tmp_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT
    )
    list(APPEND CMAKE_PREFIX_PATH "${tmp_dir}")
endif()

include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY    https://github.com/pybind/pybind11.git
    GIT_TAG           v2.13.6
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 2.13.6
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(
    rtvamp_python_bindings
    src/bindings.cpp
)

target_link_libraries(rtvamp_python_bindings
    PRIVATE
        rtvamp_project_options
        rtvamp::hostsdk
)

target_compile_definitions(rtvamp_python_bindings PRIVATE VERSION_INFO=${PROJECT_VERSION})
set_target_properties(rtvamp_python_bindings PROPERTIES OUTPUT_NAME "_bindings")

if(SKBUILD)
    install(TARGETS rtvamp_python_bindings DESTINATION .)
    install(TARGETS example-plugin DESTINATION .)  # for tests
endif()
