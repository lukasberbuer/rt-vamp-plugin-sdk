#include "rtvamp/hostsdk/PluginLoader.hpp"

#include <array>
#include <cstdlib>  // getenv
#include <set>
#include <stdexcept>

#include "PluginLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

static constexpr std::string_view getPluginExtension() {
#if defined(__APPLE__)
    return ".dylib";
#elif (defined(unix) || defined(__unix) || defined(__unix__)) && !defined(__CYGWIN__)
    return ".so";
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    return ".dll";
#else
    return "";
#endif
}

PluginLoader::PluginLoader() {
    // discover all available plugins
    const auto findPluginsInLibrary = [&](const std::filesystem::path& libraryPath) {
        try {
            PluginLibrary library(libraryPath);

            for (auto&& key : library.listPlugins()) {
                // don't overwrite existing entry with same plugin key
                plugins_.try_emplace(key, libraryPath);
            }
        } catch(...) {}
    };

    for (auto&& path : getPluginPaths()) {
        if (!std::filesystem::is_directory(path)) continue;

        for (auto&& file : std::filesystem::directory_iterator(path)) {
            if (file.is_regular_file() && file.path().extension() == getPluginExtension()) {
                findPluginsInLibrary(file.path());
            }
        }
    }
}

std::vector<std::filesystem::path> PluginLoader::getPluginPaths() {
    using std::filesystem::path;

    const std::array staticPaths{
#ifdef _WIN32
        path("C:\\Program Files\\Vamp Plugins"),
#elif __APPLE__
        path(std::getenv("HOME")).append("Library/Audio/Plug-Ins/Vamp"),
        path("/Library/Audio/Plug-Ins/Vamp"),
#elif __linux__
        path(std::getenv("HOME")).append("vamp"),
        path(std::getenv("HOME")).append(".vamp"),
        path("/usr/lib/vamp"),
        path("/usr/lib/x86_64-linux-gnu/vamp"),
        path("/usr/local/lib/vamp"),
#endif
    };

    std::vector<path> result;
    result.reserve(staticPaths.size() + 1);

    if (const char* vampPath = std::getenv("VAMP_PATH")) {
        result.emplace_back(vampPath);
    }

    result.insert(result.end(), staticPaths.begin(), staticPaths.end());
    return result;
}

std::vector<std::filesystem::path> PluginLoader::listLibraries() const {
    std::set<std::filesystem::path> libraries;
    for (const auto& [key, path] : plugins_) {
        libraries.insert(path);
    }
    return {libraries.begin(), libraries.end()};
}

std::vector<PluginKey> PluginLoader::listPlugins() const {
    std::vector<PluginKey> result;
    for (const auto& [key, _] : plugins_) {
        result.push_back(key);
    }
    return result;
}

std::vector<PluginKey> PluginLoader::listPluginsInLibrary(
    const std::filesystem::path& libraryPath
) const {
    std::vector<PluginKey> result;
    for (const auto& [key, path] : plugins_) {
        if (path == libraryPath) {
            result.push_back(key);
        }
    }
    return result;
}

std::unique_ptr<Plugin> PluginLoader::loadPlugin(
    const PluginKey& key, float inputSampleRate
) const {
    const auto getLibraryPath = [&] {
        if (!plugins_.contains(key)) {
            throw std::invalid_argument(helper::concat("Plugin not found: ", key.get()));
        }
        return plugins_.at(key);
    };

    PluginLibrary library(getLibraryPath());
    return library.loadPlugin(key, inputSampleRate);
}

}  // namespace rtvamp::hostsdk
