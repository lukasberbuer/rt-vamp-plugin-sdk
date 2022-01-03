#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "vamp/vamp.h"

namespace rtvamp::hostsdk {

class PluginLibrary {
public:
    PluginLibrary(const std::filesystem::path& libraryPath);
    ~PluginLibrary();

    std::string getLibraryName() const;

    std::vector<std::string> getPlugins() const;

    std::vector<const VampPluginDescriptor*> getDescriptors() const;

private:
    const std::filesystem::path libraryPath_;
    void* handle_{nullptr};
    std::vector<const VampPluginDescriptor*> descriptors_;
};

}  // namespace rtvamp::hostsdk
