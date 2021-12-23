#include "rt-vamp-plugin/Plugin.hpp"

class PluginBase : public rtvamp::Plugin {
public:
    using rtvamp::Plugin::Plugin;

    constexpr const char* getMaker()     const override { return "Lukas Berbuer"; }
    constexpr const char* getCopyright() const override { return "MIT"; }
};
