#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginKey.hpp"

namespace rtvamp::hostsdk {

class PluginLoader {
public:
    PluginLoader();

    static std::vector<std::filesystem::path> getPluginPaths();

    std::vector<std::filesystem::path> listLibraries() const;

    std::vector<PluginKey> listPlugins() const;
    std::vector<PluginKey> listPluginsInLibrary(const std::filesystem::path& libraryPath) const;

    std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate) const;

private:
    std::map<PluginKey, std::filesystem::path> plugins_;
};

}  // namespace rtvamp::hostsdk
