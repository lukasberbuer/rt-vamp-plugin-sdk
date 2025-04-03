add_library(
    example-plugin SHARED
    plugin.cpp
    RMS.cpp
    SpectralRolloff.cpp
)
target_link_libraries(
    example-plugin
    PRIVATE
        rtvamp_project_options
        rtvamp::pluginsdk
)
set_target_properties(example-plugin PROPERTIES PREFIX "")
