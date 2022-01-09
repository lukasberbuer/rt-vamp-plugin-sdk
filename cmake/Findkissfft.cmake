# Find kissfft.
#
# This module defines:
#  kissfft_INCLUDE_DIRS
#  kissfft_LIBRARIES
#  kissfft_FOUND

find_path(
    kissfft_INCLUDE_DIR
    NAMES
        kiss_fft.h
        kiss_fftr.h
    HINTS /usr/include /usr/local/include
    PATH_SUFFIXES kissfft
)

find_library(
    kissfft_LIBRARY
    NAMES kissfft-float
    HINTS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    kissfft
    FOUND_VAR kissfft_FOUND
    REQUIRED_VARS
        kissfft_INCLUDE_DIR
        kissfft_LIBRARY
)

mark_as_advanced(
    kissfft_INCLUDE_DIR
    kissfft_LIBRARY
)

if(kissfft_FOUND)
    message(STATUS "Found kissfft: ${kissfft_LIBRARY}, ${kissfft_INCLUDE_DIR}")

    set(kissfft_INCLUDE_DIRS ${kissfft_INCLUDE_DIR})
    set(kissfft_LIBRARIES ${kissfft_LIBRARY})

    if (NOT TARGET kissfft::kissfft)
        add_library(kissfft::kissfft UNKNOWN IMPORTED)
        set_target_properties(
            kissfft::kissfft
            PROPERTIES
                IMPORTED_LOCATION "${kissfft_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${kissfft_INCLUDE_DIR}"
        )
    endif()
endif()
