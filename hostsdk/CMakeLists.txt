set(library_name rtvamp-hostsdk)
set(alias_name "${PROJECT_NAME}::hostsdk")

add_library(
    ${library_name}
    src/PluginHostAdapter.cpp
    src/PluginKey.cpp
    src/PluginLibrary.cpp
    $<IF:$<PLATFORM_ID:Windows>, src/DynamicLibrary_Windows.cpp, src/DynamicLibrary_Unix.cpp>
    src/PluginLoader.cpp
)
add_library(${alias_name} ALIAS ${library_name})

target_link_libraries(
    ${library_name}
    PRIVATE
        ${project_options}
        ${CMAKE_DL_LIBS}
)
target_include_directories(${library_name} PUBLIC include)

if(RTVAMP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
