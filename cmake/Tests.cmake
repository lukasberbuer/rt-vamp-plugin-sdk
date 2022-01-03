include(CTest)
include(Catch)

function(run_tests TEST_TARGET)
    add_custom_command(
        TARGET ${TEST_TARGET}
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E env VAMP_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            ${CMAKE_CTEST_COMMAND} "--output-on-failure"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

function(discover_and_run_tests TEST_TARGET)
    catch_discover_tests(${TEST_TARGET})
    run_tests(${TEST_TARGET})
endfunction()
