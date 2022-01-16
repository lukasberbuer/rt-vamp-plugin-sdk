#include "RMS.hpp"

#include <vamp-sdk/PluginAdapter.h>

#include "rtvamp/pluginsdk.hpp"

const VampPluginDescriptor* getVampDescriptor() {
    static Vamp::PluginAdapter<RMSvamp> adapter;
    return adapter.getDescriptor();
}

const VampPluginDescriptor* getRtvampDescriptor() {
    return rtvamp::pluginsdk::detail::PluginAdapter<RMS>::getDescriptor();
}
