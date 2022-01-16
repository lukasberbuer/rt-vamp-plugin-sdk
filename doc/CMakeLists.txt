find_package(Doxygen REQUIRED dot)

set(doxygen_output_dir "${PROJECT_BINARY_DIR}/doc")

configure_file(
    Doxyfile.in
    ${doxygen_output_dir}/Doxyfile
)

add_custom_target(
    rtvamp-doc
    ALL  # build by default
    COMMAND ${DOXYGEN_EXECUTABLE}
    COMMENT "Build Doxygen documentation to ${doxygen_output_dir}"
    WORKING_DIRECTORY ${doxygen_output_dir}
)
