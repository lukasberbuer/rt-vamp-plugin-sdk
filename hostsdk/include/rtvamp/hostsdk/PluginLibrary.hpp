#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginKey.hpp"

// forward declarations
struct _VampPluginDescriptor;
typedef _VampPluginDescriptor VampPluginDescriptor;

namespace rtvamp::hostsdk {

class DynamicLibrary;

class PluginLibrary {
public:
    explicit PluginLibrary(const std::filesystem::path& libraryPath);

    std::filesystem::path   getLibraryPath() const noexcept;
    std::string             getLibraryName() const;

    size_t                  getPluginCount() const noexcept;

    std::vector<PluginKey>  listPlugins() const;

    std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate) const;
    std::unique_ptr<Plugin> loadPlugin(size_t index, float inputSampleRate) const;

private:
    std::shared_ptr<DynamicLibrary>          dl_;
    std::vector<const VampPluginDescriptor*> descriptors_;
};

}  // namespace rtvamp::hostsdk
