#include "rt-vamp-plugin/EntryPoint.hpp"

#include "RMS.hpp"

extern "C" const VampPluginDescriptor* vampGetPluginDescriptor(
    unsigned int version,
    unsigned int index
) {
    return EntryPoint<RMS>::getDescriptor(version, index);
}
