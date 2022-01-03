#include "RMS.hpp"

#include <vamp-sdk/PluginAdapter.h>

#include "rtvamp/pluginsdk/PluginAdapter.hpp"

const VampPluginDescriptor* getVampDescriptor() {
    static Vamp::PluginAdapter<RMSvamp> adapter;
    return adapter.getDescriptor();
}

const VampPluginDescriptor* getRtvampDescriptor() {
    return rtvamp::pluginsdk::PluginAdapter<RMS>::getDescriptor();
}
