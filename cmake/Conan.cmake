option(RTVAMP_ENABLE_CONAN "Use Conan for dependency management" OFF)
if(RTVAMP_ENABLE_CONAN)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(
        DOWNLOAD
            "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
        TLS_VERIFY ON
    )
    endif()

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    # for multi configuration generators, like VS and XCode
    if(NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Single configuration build")
        set(list_of_build_types ${CMAKE_BUILD_TYPE})
    else()
        message(STATUS "Multi-configuration build: '${CMAKE_CONFIGURATION_TYPES}'")
        # avoid problems with both cmake_find_package and cmake_find_package_multi generators
        set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE CACHE BOOL "" FORCE)
        set(list_of_build_types ${CMAKE_CONFIGURATION_TYPES})
    endif()

    foreach(type ${list_of_build_types})
        message(STATUS "Running Conan for build type '${type}'")
        conan_cmake_autodetect(settings BUILD_TYPE ${type})

        conan_cmake_install(
            PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}
            BUILD missing
            ENV "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}"
            SETTINGS ${settings}
        )
    endforeach()
endif()
