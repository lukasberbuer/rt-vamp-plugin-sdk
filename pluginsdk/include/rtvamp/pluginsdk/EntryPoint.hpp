#pragma once

#include "vamp/vamp.h"

#include "rtvamp/pluginsdk/PluginAdapter.hpp"
#include "rtvamp/pluginsdk/PluginDefinition.hpp"

namespace rtvamp::pluginsdk {

template <IsPluginDefinition... Plugins>
class EntryPoint {
public:
    static constexpr const VampPluginDescriptor* getDescriptor(
        unsigned int version,
        unsigned int index
    ) {
        if (version < 1) return {};
        if (index >= pluginCount) return {};
        return descriptors[index];
    }

private:
    static constexpr auto pluginCount = sizeof...(Plugins);

    static constexpr std::array<const VampPluginDescriptor*, pluginCount> descriptors{
        {PluginAdapter<Plugins>::getDescriptor()...}
    };
};

}  // namespace rtvamp::pluginsdk

/* -------------------------------------------- Macro ------------------------------------------- */

#define RTVAMP_ENTRY_POINT(...)                                                             \
    extern "C" const VampPluginDescriptor* vampGetPluginDescriptor(                         \
        unsigned int version,                                                               \
        unsigned int index                                                                  \
    ) {                                                                                     \
        return ::rtvamp::pluginsdk::EntryPoint<__VA_ARGS__>::getDescriptor(version, index); \
    }
