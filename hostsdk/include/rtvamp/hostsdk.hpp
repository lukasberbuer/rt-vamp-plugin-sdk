#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginKey.hpp"
#include "rtvamp/hostsdk/PluginLibrary.hpp"

namespace rtvamp::hostsdk {

using PathList = std::vector<std::filesystem::path>;

/**
 * Get default Vamp search paths for plugin libraries.
 * A custom path can be set with the `VAMP_PATH` environment variable.
 */
PathList getVampPaths();

/**
 * Check if the library is an existing and valid Vamp library.
 */
bool isVampLibrary(const std::filesystem::path& libraryPath);

/**
 * List all plugin libraries in default Vamp search paths.
 */
PathList listLibraries();

/**
 * List all plugin libraries in custom search path.
 */
PathList listLibraries(const std::filesystem::path& path);

/**
 * List all plugin libraries in custom search paths.
 */
PathList listLibraries(std::span<const std::filesystem::path> paths);

/**
 * Load plugin library by file path.
 */
PluginLibrary loadLibrary(const std::filesystem::path& libraryPath);

/**
 * List plugins in default Vamp search paths.
 */
std::vector<PluginKey> listPlugins();

/**
 * List plugins in path (either directory or library).
 */
std::vector<PluginKey> listPlugins(const std::filesystem::path& path);

/**
 * List plugins in given list of paths (either search paths or library paths).
 */
std::vector<PluginKey> listPlugins(std::span<const std::filesystem::path> paths);

/**
 * Load plugin.
 */
std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate);

/**
 * Load plugin from given list of paths (either search paths or library paths).
 * Providing the cached library paths speeds up the search.
 */
std::unique_ptr<Plugin> loadPlugin(const PluginKey& key, float inputSampleRate, std::span<const std::filesystem::path> paths);

}  // namespace rtvamp::hostsdk
