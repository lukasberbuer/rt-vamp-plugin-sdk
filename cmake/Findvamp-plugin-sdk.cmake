# Find vamp-plugin-sdk.
#
# This module defines:
#  vamp-plugin-sdk_INCLUDE_DIRS
#  vamp-plugin-sdk_LIBRARIES
#  vamp-plugin-sdk_FOUND
#
# If you have vamp-plugin-sdk installed in a non-standard place, you can define
# vamp-plugin-sdk_PREFIX to tell cmake where it is.

find_path(
    vamp_INCLUDE_DIR
    NAMES vamp.h
    HINTS /usr/include /usr/local/include ${vamp-sdk_PREFIX}/include
    PATH_SUFFIXES vamp
)

find_path(
    vamp-sdk_INCLUDE_DIR
    NAMES
        FFT.h
        Plugin.h
        PluginAdapter.h
        PluginBase.h
        RealTime.h
        plugguard.h
        vamp-sdk.h
    HINTS /usr/include /usr/local/include ${vamp-sdk_PREFIX}/include
    PATH_SUFFIXES vamp-sdk
)

find_library(
    vamp-sdk_LIBRARY
    NAMES vamp-sdk
    HINTS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu/lib ${vamp-sdk_PREFIX}/lib
)

find_path(
    vamp-hostsdk_INCLUDE_DIR
    NAMES
        Plugin.h
        PluginBase.h
        PluginBufferingAdapter.h
        PluginChannelAdapter.h
        PluginHostAdapter.h
        PluginInputDomainAdapter.h
        PluginLoader.h
        PluginSummarisingAdapter.h
        PluginWrapper.h
        RealTime.h
        host-c.h
        hostguard.h
        vamp-hostsdk.h
    HINTS /usr/include /usr/local/include ${vamp-sdk_PREFIX}/include
    PATH_SUFFIXES vamp-hostsdk
)

find_library(
    vamp-hostsdk_LIBRARY
    NAMES vamp-hostsdk
    HINTS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu/lib ${vamp-sdk_PREFIX}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    vamp-plugin-sdk
    FOUND_VAR vamp-plugin-sdk_FOUND
    REQUIRED_VARS
        vamp_INCLUDE_DIR
        vamp-sdk_INCLUDE_DIR
        vamp-sdk_LIBRARY
        vamp-hostsdk_INCLUDE_DIR
        vamp-hostsdk_LIBRARY
)

mark_as_advanced(
    vamp_INCLUDE_DIR
    vamp-sdk_INCLUDE_DIR
    vamp-sdk_LIBRARY
    vamp-hostsdk_INCLUDE_DIR
    vamp-hostsdk_LIBRARY
)

if(vamp-plugin-sdk_FOUND)
    message(STATUS "Found vamp: ${vamp_INCLUDE_DIR}")
    message(STATUS "Found vamp-sdk: ${vamp-sdk_LIBRARY}, ${vamp-sdk_INCLUDE_DIR}")
    message(STATUS "Found vamp-hostsdk: ${vamp-hostsdk_LIBRARY}, ${vamp-hostsdk_INCLUDE_DIR}")

    set(
        vamp-plugin-sdk_INCLUDE_DIRS
        ${vamp-sdk_INCLUDE_DIR}
        ${vamp-plugin-sdk_INCLUDE_DIR}
        ${vamp-plugin-hostsdk_INCLUDE_DIR}
    )
    set(
        vamp-plugin-sdk_LIBRARIES
        ${vamp-plugin-sdk_LIBRARY}
        ${vamp-plugin-hostsdk_LIBRARY}
    )

    if (NOT TARGET vamp-plugin-sdk::vamp)
        add_library(vamp-plugin-sdk::vamp UNKNOWN IMPORTED)
        set_target_properties(
            vamp-plugin-sdk::vamp
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${vamp_INCLUDE_DIR}"
        )
    endif()

    if (NOT TARGET vamp-plugin-sdk::vamp-sdk)
        add_library(vamp-plugin-sdk::vamp-sdk UNKNOWN IMPORTED)
        set_target_properties(
            vamp-plugin-sdk::vamp-sdk
            PROPERTIES
                IMPORTED_LOCATION "${vamp-sdk_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${vamp-sdk_INCLUDE_DIR}"
        )
    endif()

    if (NOT TARGET vamp-plugin-sdk::vamp-hostsdk)
        add_library(vamp-plugin-sdk::vamp-hostsdk UNKNOWN IMPORTED)
        set_target_properties(
            vamp-plugin-sdk::vamp-hostsdk
            PROPERTIES
                IMPORTED_LOCATION "${vamp-hostsdk_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${vamp-hostsdk_INCLUDE_DIR}"
        )
    endif()
endif()
