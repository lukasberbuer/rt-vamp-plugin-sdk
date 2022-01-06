find_package(SndFile)

if(SndFile_FOUND)
    add_executable(example-host host.cpp)
    target_link_libraries(
        example-host
        PRIVATE
            ${PROJECT_OPTIONS}
            rtvamp-hostsdk
            SndFile::sndfile
    )
else()
    message(WARNING "example-host won't be built because libsndfile was not found")
endif()
