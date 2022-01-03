#include "rtvamp/hostsdk/PluginLoader.hpp"

#include "PluginLibrary.hpp"

namespace rtvamp::hostsdk {

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
                result.push_back(file.path().string());
            }
        }
    }

    return result;
}

std::vector<std::string> PluginLoader::listPlugins() {
    std::vector<std::string> result;

    for (const auto& path : listLibraries()) {
        try {
            PluginLibrary library(path);

            for (const auto* descriptor : library.getDescriptors()) {
                std::string key = path.stem().string();
                key += ":";
                key += descriptor->identifier;
                result.push_back(key);
            }
        } catch(...) {}
    }

    return result;
}

}  // namespace rtvamp::hostsdk
