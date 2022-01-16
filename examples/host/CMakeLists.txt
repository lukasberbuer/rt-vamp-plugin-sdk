find_package(SndFile)

if(SndFile_FOUND)
    find_package(kissfft)

    add_executable(
        example-host
        main.cpp
        FFT.cpp
        $<IF:$<BOOL:${kissfft_FOUND}>, FFT_Kiss.cpp, FFT_NotFound.cpp>
    )

    target_link_libraries(
        example-host
        PRIVATE
            ${project_options}
            rtvamp-hostsdk
            SndFile::sndfile
            $<$<BOOL:${kissfft_FOUND}>:kissfft::kissfft>
    )
else()
    message(WARNING "example-host won't be built because libsndfile was not found")
endif()
