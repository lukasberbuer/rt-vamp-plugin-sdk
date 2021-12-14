#pragma once

#include <memory>
#include <type_traits>

#include "Plugin.h"

// forward declarations
struct _VampPluginDescriptor;
typedef _VampPluginDescriptor VampPluginDescriptor;

namespace rtvamp {

class PluginAdapterBase {
public:
    PluginAdapterBase();
    virtual ~PluginAdapterBase();

    const VampPluginDescriptor* getDescriptor();

protected:
    virtual std::unique_ptr<Plugin> createPlugin(float inputSampleRate) const = 0;

    class Impl;
    std::unique_ptr<Impl> impl_;
};


template <typename PluginType>
class PluginAdapter : public PluginAdapterBase {
    static_assert(std::is_base_of_v<Plugin, PluginType>, "PluginType must inherit from Plugin");
protected:
    std::unique_ptr<Plugin> createPlugin(float inputSampleRate) const override {
        return std::make_unique<PluginType>(inputSampleRate);
    }
};

}  // namespace rtvamp
