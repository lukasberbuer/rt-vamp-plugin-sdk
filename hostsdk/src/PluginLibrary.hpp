#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "DynamicLibrary.hpp"

namespace rtvamp::hostsdk {

class PluginLibrary {
public:
    PluginLibrary(const std::filesystem::path& libraryPath);

    std::string getLibraryName() const;

    std::vector<const VampPluginDescriptor*> getDescriptors() const;

private:
    DynamicLibrary dl_;
    std::vector<const VampPluginDescriptor*> descriptors_;
};

}  // namespace rtvamp::hostsdk
