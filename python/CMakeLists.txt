find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

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
    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/rtvamp"
        DESTINATION .
        COMPONENT python
    )
    install(
        TARGETS rtvamp_python_bindings
        DESTINATION rtvamp
        COMPONENT python
    )
    install(
        TARGETS example-plugin
        DESTINATION rtvamp/plugins
        COMPONENT python
    )
endif()
