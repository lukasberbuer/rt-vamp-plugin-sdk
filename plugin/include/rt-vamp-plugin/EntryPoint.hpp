#pragma once

#include "vamp/vamp.h"

#include "PluginAdapter.hpp"

namespace rtvamp {

template <typename... Plugins>
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

}  // namespace rtvamp

/* -------------------------------------------- Macro ------------------------------------------- */

#define RTVAMP_ENTRY_POINT(...)                                                  \
    extern "C" const VampPluginDescriptor* vampGetPluginDescriptor(              \
        unsigned int version,                                                    \
        unsigned int index                                                       \
    ) {                                                                          \
        return ::rtvamp::EntryPoint<__VA_ARGS__>::getDescriptor(version, index); \
    }                                                                            \
