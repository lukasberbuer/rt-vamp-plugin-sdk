find_package(SndFile REQUIRED)

add_executable(example-host host.cpp)
target_link_libraries(
    example-host
    PRIVATE
        ${PROJECT_OPTIONS}
        rtvamp-hostsdk
        SndFile::sndfile
)
