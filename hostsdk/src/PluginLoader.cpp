#include "rtvamp/hostsdk/PluginLoader.hpp"

#include <utility>  // pair

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "PluginLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

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
        throw std::invalid_argument(helper::concat("Invalid plugin key ", key_));
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

std::vector<std::filesystem::path> PluginLoader::getPluginPaths() {
    using std::filesystem::path;

    std::vector<path> result;

    if (const char* vampPath = std::getenv("VAMP_PATH")) {
        result.emplace_back(vampPath);
    }

    const std::vector<path> staticPaths{
#ifdef _WIN32
        path(getenv("PROGRAMFILES")).append("Vamp Plugins"),
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

    result.insert(result.end(), staticPaths.begin(), staticPaths.end());
    return result;
}

static constexpr std::string_view getPluginExtension() {
#ifdef _WIN32
    return ".dll";
#elif __APPLE__
    return ".dylib";
#else
    return ".so";
#endif
}

std::vector<std::filesystem::path> PluginLoader::listLibraries() {
    std::vector<std::filesystem::path> result;

    for (const auto& path : getPluginPaths()) {
        if (!std::filesystem::is_directory(path)) continue;

        for (const auto& file : std::filesystem::directory_iterator(path)) {
            if (file.is_regular_file() && file.path().extension() == getPluginExtension()) {
                result.push_back(file.path());
            }
        }
    }

    return result;
}

std::vector<PluginKey> PluginLoader::listPlugins() {
    std::vector<PluginKey> result;

    for (const auto& path : listLibraries()) {
        try {
            PluginLibrary library(path);

            for (const auto* descriptor : library.getDescriptors()) {
                result.emplace_back(path.stem().string(), descriptor->identifier);
            }
        } catch(...) {}
    }

    return result;
}

class PluginHostAdapterLibraryWrapper : public PluginHostAdapter {
public:
    PluginHostAdapterLibraryWrapper(
        PluginLibrary library, const VampPluginDescriptor& descriptor, float inputSampleRate
    )
        : PluginHostAdapter(descriptor, inputSampleRate),
          library_(std::move(library)) {}
private:
    PluginLibrary library_;
};

std::unique_ptr<Plugin> PluginLoader::loadPlugin(const PluginKey& key, float inputSampleRate) {
    const auto libraryPath = [&] {
        for (auto&& path : listLibraries()) {
            if (path.stem() == key.getLibrary()) return path;
        }
        throw std::invalid_argument("Plugin library not found");
    }();

    PluginLibrary library(libraryPath);
    const auto* descriptor = [&] {
        for (const auto* d : library.getDescriptors()) {
            if (d->identifier == key.getIdentifier()) return d;
        }
        throw std::invalid_argument("Plugin identifier not found in descriptors");
    }();

    return std::make_unique<PluginHostAdapterLibraryWrapper>(library, *descriptor, inputSampleRate);
}

}  // namespace rtvamp::hostsdk
