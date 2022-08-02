#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rtvamp/hostsdk/PluginKey.hpp"

#include "DynamicLibrary.hpp"

namespace rtvamp::hostsdk {

class PluginLibrary {
public:
    PluginLibrary(const std::filesystem::path& libraryPath);

    std::filesystem::path getLibraryPath() const noexcept;
    std::string           getLibraryName() const;

    std::vector<PluginKey> listPlugins() const;

    std::vector<const VampPluginDescriptor*> getDescriptors() const;

private:
    DynamicLibrary dl_;
    std::vector<const VampPluginDescriptor*> descriptors_;
};

}  // namespace rtvamp::hostsdk
