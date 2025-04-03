option(RTVAMP_ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
if(RTVAMP_ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --suppress=missingInclude
            --suppress=missingIncludeSystem
            --enable=all
            --inline-suppr
            --inconclusive
        )
    else()
        message(WARNING "cppcheck requested but executable not found")
    endif()
endif()

option(RTVAMP_ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(RTVAMP_ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else()
        message(WARNING "clang-tidy requested but executable not found")
    endif()
endif()

option(RTVAMP_ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
if(RTVAMP_ENABLE_INCLUDE_WHAT_YOU_USE)
    find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
    if(INCLUDE_WHAT_YOU_USE)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
    else()
        message(WARNING "include-what-you-use requested but executable not found")
    endif()
endif()
