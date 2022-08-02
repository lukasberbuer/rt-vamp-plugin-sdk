#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginKey.hpp"

#include "DynamicLibrary.hpp"

// forward declarations
struct _VampPluginDescriptor;
typedef _VampPluginDescriptor VampPluginDescriptor;

namespace rtvamp::hostsdk {

class PluginLibrary {
public:
    PluginLibrary(const std::filesystem::path& libraryPath);

    std::filesystem::path   getLibraryPath() const noexcept;
    std::string             getLibraryName() const;

    size_t                  getPluginCount() const noexcept;

    std::vector<PluginKey>  listPlugins() const;

    std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate) const;

private:
    DynamicLibrary                           dl_;
    std::vector<const VampPluginDescriptor*> descriptors_;
};

}  // namespace rtvamp::hostsdk
