add_library(
    example-plugin SHARED
    plugin.cpp
    RMS.cpp
    SpectralRolloff.cpp
)
target_link_libraries(
    example-plugin
    PRIVATE
        ${PROJECT_OPTIONS}
        rtvamp-pluginsdk
)
set_target_properties(example-plugin PROPERTIES PREFIX "")
