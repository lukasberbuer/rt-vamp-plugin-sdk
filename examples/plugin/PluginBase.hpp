#include "rt-vamp-plugin/Plugin.hpp"

class PluginBase : public rtvamp::Plugin {
public:
    using rtvamp::Plugin::Plugin;

    const char* getMaker() const override {
        return "Lukas Berbuer";
    }
    const char* getCopyright() const override {
        return "MIT";
    }
};
