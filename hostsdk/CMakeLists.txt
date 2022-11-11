set(library_name rtvamp-hostsdk)
set(alias_name "${PROJECT_NAME}::hostsdk")

add_library(
    ${library_name}
    $<IF:$<PLATFORM_ID:Windows>, src/DynamicLibrary_Windows.cpp, src/DynamicLibrary_Unix.cpp>
    src/hostsdk.cpp
    src/PluginHostAdapter.cpp
    src/PluginKey.cpp
    src/PluginLibrary.cpp
)
add_library(${alias_name} ALIAS ${library_name})

target_link_libraries(
    ${library_name}
    PRIVATE
        ${project_options}
        ${CMAKE_DL_LIBS}
)
target_include_directories(${library_name} PUBLIC include)

option(RTVAMP_VALIDATE "Validate input data and method call order in hostsdk" OFF)
if(RTVAMP_VALIDATE)
    target_compile_definitions(${library_name} PUBLIC RTVAMP_VALIDATE)
endif()

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
