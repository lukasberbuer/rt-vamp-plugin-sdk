#include "rtvamp/hostsdk.hpp"

#include <cassert>
#include <optional>
#include <set>

#include "vamp/vamp.h"

#include "DynamicLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

static constexpr std::string_view getPluginExtension() noexcept {
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

PathList getVampPaths() {
    using std::filesystem::path;
    std::vector<path> result;

    if (const char* customPath = std::getenv("VAMP_PATH")) {
        result.emplace_back(customPath);
    }

#ifdef _WIN32
    result.emplace_back("C:\\Program Files\\Vamp Plugins");
#elif __APPLE__
    if (const char* home = std::getenv("HOME")) {
        result.emplace_back(path(home).append("Library/Audio/Plug-Ins/Vamp"));
    }
    result.emplace_back("/Library/Audio/Plug-Ins/Vamp");
#elif __linux__
    if (const char* home = std::getenv("HOME")) {
        result.emplace_back(path(home).append("vamp"));
        result.emplace_back(path(home).append(".vamp"));
    }
    result.emplace_back("/usr/lib/vamp");
    result.emplace_back("/usr/lib/x86_64-linux-gnu/vamp");
    result.emplace_back("/usr/local/lib/vamp");
#endif

    return result;
}

bool isVampLibrary(const std::filesystem::path& libraryPath) {
    DynamicLibrary dl;
    if (!dl.load(libraryPath)) {
        return false;
    }
    if (!dl.getFunction<VampGetPluginDescriptorFunction>("vampGetPluginDescriptor")) {
        return false;
    }
    return true;
}

static bool isVampLibrary(const std::filesystem::directory_entry& entry) {
    std::error_code ec;
    return (
        entry.is_regular_file(ec) &&
        entry.path().extension() == getPluginExtension() &&
        isVampLibrary(entry.path())
    );
}

PathList listLibraries() {
    const auto paths = getVampPaths();
    return listLibraries(paths);
}

PathList listLibraries(const std::filesystem::path& path) {
    std::error_code ec;  // for noexcept versions
    if (!std::filesystem::is_directory(path, ec)) {
        return {};
    }
    std::vector<std::filesystem::path> result;
    for (auto&& entry : std::filesystem::directory_iterator(path, ec)) {
        if (isVampLibrary(entry)) {
            result.push_back(entry.path());
        }
    }
    return result;
}

PathList listLibraries(std::span<const std::filesystem::path> paths) {
    std::set<std::filesystem::path> result;  // use set to avoid duplicates (paths may have duplicates)
    for (auto&& path : paths) {
        const auto libraries = listLibraries(path);
        result.insert(libraries.begin(), libraries.end());
    }
    return {result.begin(), result.end()};
}

PluginLibrary loadLibrary(const std::filesystem::path& libraryPath) {
    return PluginLibrary(libraryPath);
}

std::vector<PluginKey> listPlugins() {
    const auto libraryPaths = listLibraries();
    return listPlugins(libraryPaths);
}

static std::vector<PluginKey> listPluginsInLibrary(const std::filesystem::path& path) {
    assert(std::filesystem::is_regular_file(path));
    try {
        const auto library = loadLibrary(path);
        return library.listPlugins();
    } catch (...) {
        return {};
    }
}

static std::vector<PluginKey> listPluginsInDirectory(const std::filesystem::path& path) {
    assert(std::filesystem::is_directory(path));
    std::vector<PluginKey> result;
    for (auto&& libraryPath : listLibraries(path)) {
        const auto plugins = listPluginsInLibrary(libraryPath);
        result.insert(result.end(), plugins.begin(), plugins.end());
    }
    return {result.begin(), result.end()};
}

std::vector<PluginKey> listPlugins(const std::filesystem::path& path) {
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        return listPluginsInDirectory(path);
    }
    if (std::filesystem::is_regular_file(path, ec)) {
        return listPluginsInLibrary(path);
    }
    return {};
}

std::vector<PluginKey> listPlugins(std::span<const std::filesystem::path> paths) {
    std::set<PluginKey> result;  // use set to avoid duplicates (paths may have duplicates)
    for (auto&& path : paths) {
        const auto plugins = listPlugins(path);
        result.insert(plugins.begin(), plugins.end());
    }
    return {result.begin(), result.end()};
}

static std::optional<std::filesystem::path> findLibrary(std::string_view stem) {
    std::error_code ec;
    for (auto&& path : getVampPaths()) {
        for (auto&& entry : std::filesystem::directory_iterator(path, ec)) {
            if (isVampLibrary(entry) && entry.path().stem() == stem) {
                return entry.path();
            }
        }
    }
    return std::nullopt;
}

static std::optional<std::filesystem::path> findLibrary(std::string_view stem, std::span<const std::filesystem::path> libraryPaths) {
    for (auto&& libraryPath : libraryPaths) {
        if (isVampLibrary(libraryPath) && libraryPath.stem() == stem) {
            return libraryPath;
        }
    }
    return std::nullopt;
}

std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate) {
    const auto libraryPath = findLibrary(key.getLibrary());
    if (!libraryPath) {
        throw std::invalid_argument(helper::concat("Plugin not found: ", key.get()));
    }
    const auto library = loadLibrary(libraryPath.value());
    return library.loadPlugin(key, inputSampleRate);
}

std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate, std::span<const std::filesystem::path> paths) {
    const auto libraryPath = findLibrary(key.getLibrary(), paths);
    if (!libraryPath) {
        throw std::invalid_argument(helper::concat("Plugin not found: ", key.get()));
    }
    const auto library = loadLibrary(libraryPath.value());
    return library.loadPlugin(key, inputSampleRate);
}

}  // namespace rtvamp::hostsdk
