add_executable(example-host host.cpp)
target_link_libraries(
    example-host
    PRIVATE
        ${PROJECT_OPTIONS}
        ${CMAKE_DL_LIBS}
)
