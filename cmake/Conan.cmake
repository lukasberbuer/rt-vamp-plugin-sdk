option(RTVAMP_ENABLE_CONAN "Use Conan for dependency management" OFF)
if(RTVAMP_ENABLE_CONAN)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(
            DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
            TLS_VERIFY ON
        )
    endif()

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    # for multi configuration generators, like VS and XCode
    if(NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Single configuration build")
        set(LIST_OF_BUILD_TYPES ${CMAKE_BUILD_TYPE})
    else()
        message(STATUS "Multi-configuration build: '${CMAKE_CONFIGURATION_TYPES}'")
        set(LIST_OF_BUILD_TYPES ${CMAKE_CONFIGURATION_TYPES})
    endif()

    foreach(TYPE ${LIST_OF_BUILD_TYPES})
        message(STATUS "Running Conan for build type '${TYPE}'")
        conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})

        conan_cmake_install(
            PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}
            BUILD missing
            ENV "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}"
            SETTINGS ${settings}
        )
    endforeach()
endif()
