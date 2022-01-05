#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <memory>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"

namespace rtvamp::hostsdk {

class PluginKey {
public:
    explicit PluginKey(std::string key);
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
    static std::vector<std::filesystem::path> getPluginPaths();

    static std::vector<std::filesystem::path> listLibraries();

    static std::vector<PluginKey> listPlugins();

    using PluginDeleter = std::function<void(Plugin*)>;
    using PluginPtr     = std::unique_ptr<Plugin, PluginDeleter>;

    static PluginPtr loadPlugin(const PluginKey& key, float inputSampleRate);
};

}  // namespace rtvamp::hostsdk
