add_library(minimal-plugin SHARED plugin.cpp)
target_link_libraries(
    minimal-plugin
    PRIVATE
        ${PROJECT_OPTIONS}
        rtvamp-pluginsdk
)
set_target_properties(
    minimal-plugin
    PROPERTIES
        PREFIX ""
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

add_executable(minimal-host host.cpp)
target_link_libraries(
    minimal-host
    PRIVATE
        ${PROJECT_OPTIONS}
        rtvamp-hostsdk
)
