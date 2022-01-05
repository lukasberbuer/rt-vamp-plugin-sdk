#include "rtvamp/hostsdk/PluginLoader.hpp"

#include <array>
#include <set>
#include <utility>  // pair

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "PluginLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

PluginKey::PluginKey(const char* key) : PluginKey(std::string(key)) {}

PluginKey::PluginKey(std::string key)
    : key_(std::move(key)) {
    if (key_.empty()) {
        throw std::invalid_argument("Plugin key empty");
    }

    pos_ = key_.find(':');
    if (
        pos_ == std::string::npos ||  // not found
        pos_ == 0 ||                  // first character -> empty library name
        pos_ == key_.size() - 1       // last character -> empty identifier
    ) {
        throw std::invalid_argument(helper::concat("Invalid plugin key: ", key_));
    }
}

PluginKey::PluginKey(std::string_view library, std::string_view identifier)
    : PluginKey(std::string(library).append(":").append(identifier)) {}

std::string_view PluginKey::getLibrary() const noexcept {
    return std::string_view(key_).substr(0, pos_);
}

std::string_view PluginKey::getIdentifier() const noexcept {
    return std::string_view(key_).substr(pos_ + 1);
}

/* ---------------------------------------------------------------------------------------------- */

static constexpr std::string_view getPluginExtension() {
#ifdef _WIN32
    return ".dll";
#elif __APPLE__
    return ".dylib";
#else
    return ".so";
#endif
}

PluginLoader::PluginLoader() {
    // discover all available plugins
    const auto findPluginsInLibrary = [&](const std::filesystem::path& libraryPath) {
        try {
            PluginLibrary library(libraryPath);

            for (auto&& descriptor : library.getDescriptors()) {
                // don't overwrite existing entry with same plugin key
                plugins_.try_emplace(
                    PluginKey(library.getLibraryName(), descriptor->identifier),
                    libraryPath
                );
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
        path(getenv("HOME")).append("Library/Audio/Plug-Ins/Vamp"),
        path("/Library/Audio/Plug-Ins/Vamp"),
#elif __linux__
        path(getenv("HOME")).append("vamp"),
        path(getenv("HOME")).append(".vamp"),
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

PluginLoader::PluginPtr PluginLoader::loadPlugin(
    const PluginKey& key, float inputSampleRate
) const {
    if (!plugins_.contains(key)) {
        throw std::invalid_argument(helper::concat("Plugin not found: ", key.get()));
    }
    const auto libraryPath = plugins_.at(key);

    PluginLibrary library(libraryPath);
    const auto* descriptor = [&] {
        for (const auto* d : library.getDescriptors()) {
            if (d->identifier == key.getIdentifier()) return d;
        }
        throw std::invalid_argument(helper::concat("Plugin identifier not found in descriptors: ", key.get()));
    }();

    return std::unique_ptr<PluginHostAdapter, PluginDeleter>(
        new PluginHostAdapter(*descriptor, inputSampleRate),
        [dl = std::move(library)](Plugin* p) {  // capture library to be deleted after plugin
            delete p;
        }
    );
}

}  // namespace rtvamp::hostsdk
