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

find_package(pybind11 REQUIRED)

pybind11_add_module(
    rtvamp-python-bindings
    src/bindings.cpp
)

target_link_libraries(rtvamp-python-bindings
    PRIVATE
        ${project_options}
        rtvamp-hostsdk
)

target_compile_definitions(rtvamp-python-bindings PRIVATE VERSION_INFO=${PROJECT_VERSION})
set_target_properties(rtvamp-python-bindings PROPERTIES OUTPUT_NAME "_bindings")

if(SKBUILD)
    install(TARGETS rtvamp-python-bindings DESTINATION .)
    install(TARGETS example-plugin DESTINATION .)  # for tests
endif()
