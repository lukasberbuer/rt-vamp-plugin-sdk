add_library(
    rtvamp_hostsdk
    $<IF:$<PLATFORM_ID:Windows>, src/DynamicLibrary_Windows.cpp, src/DynamicLibrary_Unix.cpp>
    src/hostsdk.cpp
    src/PluginHostAdapter.cpp
    src/PluginKey.cpp
    src/PluginLibrary.cpp
)
add_library(rtvamp::hostsdk ALIAS rtvamp_hostsdk)

target_link_libraries(
    rtvamp_hostsdk
    PRIVATE
        rtvamp_project_options
        ${CMAKE_DL_LIBS}
)
target_include_directories(rtvamp_hostsdk PUBLIC include)

option(RTVAMP_VALIDATE "Validate input data and method call order in hostsdk" OFF)
if(RTVAMP_VALIDATE)
    target_compile_definitions(rtvamp_hostsdk PUBLIC RTVAMP_VALIDATE)
endif()

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
