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
    ) noexcept {
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
