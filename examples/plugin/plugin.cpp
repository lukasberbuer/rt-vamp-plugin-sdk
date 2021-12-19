#include <iostream>
#include <stdexcept>
#include <vector>

#include "rt-vamp-plugin/PluginAdapter.hpp"

#include "RMS.hpp"

extern "C" const VampPluginDescriptor* vampGetPluginDescriptor(
    unsigned int version,
    unsigned int index
) {
    static rtvamp::PluginAdapter<RMS> rmsAdapter;

    if (version < 1) return {};

    switch (index) {
    case  0: return rmsAdapter.getDescriptor();
    default: return {};
    }
}
