#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <map>
#include <memory>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"

namespace rtvamp::hostsdk {

class PluginKey {
public:
    PluginKey(const char* key);
    PluginKey(std::string key);
    PluginKey(std::string_view library, std::string_view identifier);

    std::string_view get()           const noexcept { return key_; }
    std::string_view getLibrary()    const noexcept;
    std::string_view getIdentifier() const noexcept;

    auto operator<=>(const PluginKey&) const = default;  

private:
    std::string key_;
    size_t      pos_;
};

class PluginLoader {
public:
    PluginLoader();

    static std::vector<std::filesystem::path> getPluginPaths();

    std::vector<std::filesystem::path> listLibraries() const;

    std::vector<PluginKey> listPlugins() const;
    std::vector<PluginKey> listPluginsInLibrary(const std::filesystem::path& libraryPath) const;

    using PluginDeleter = std::function<void(Plugin*)>;
    using PluginPtr     = std::unique_ptr<Plugin, PluginDeleter>;

    PluginPtr loadPlugin(const PluginKey& key, float inputSampleRate) const;

private:
    std::map<PluginKey, std::filesystem::path> plugins_;
};

}  // namespace rtvamp::hostsdk
