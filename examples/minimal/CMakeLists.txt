add_library(minimal-plugin SHARED plugin.cpp)
target_link_libraries(
    minimal-plugin
    PRIVATE
        ${project_options}
        rtvamp-pluginsdk
)
set_target_properties(minimal-plugin PROPERTIES PREFIX "")

add_executable(minimal-host host.cpp)
target_link_libraries(
    minimal-host
    PRIVATE
        ${project_options}
        rtvamp-hostsdk
)
