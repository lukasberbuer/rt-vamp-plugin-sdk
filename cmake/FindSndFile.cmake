# Find libsndfile.
#
# This module defines:
#  SndFile_INCLUDE_DIRS
#  SndFile_LIBRARIES
#  SndFile_FOUND

find_path(
    SndFile_INCLUDE_DIR
    NAMES
        sndfile.h
        sndfile.hh
    HINTS /usr/include /usr/local/include
)

find_library(
    SndFile_LIBRARY
    NAMES sndfile
    HINTS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    SndFile
    FOUND_VAR SndFile_FOUND
    REQUIRED_VARS
        SndFile_INCLUDE_DIR
        SndFile_LIBRARY
)

mark_as_advanced(
    SndFile_INCLUDE_DIR
    SndFile_LIBRARY
)

if(SndFile_FOUND)
    message(STATUS "Found libsndfile: ${SndFile_LIBRARY}, ${SndFile_INCLUDE_DIR}")

    set(SndFile_INCLUDE_DIRS ${SndFile_INCLUDE_DIR})
    set(SndFile_LIBRARIES ${SndFile_LIBRARY})

    if (NOT TARGET SndFile::sndfile)
        add_library(SndFile::sndfile UNKNOWN IMPORTED)
        set_target_properties(
            SndFile::sndfile
            PROPERTIES
                IMPORTED_LOCATION "${SndFile_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIR}"
        )
    endif()
endif()
