#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace rtvamp::hostsdk {

class PluginLoader {
public:
    static std::vector<std::filesystem::path> getPluginPaths();

    static std::vector<std::filesystem::path> listLibraries();

    static std::vector<std::string> listPlugins();
};


}  // namespace rtvamp::hostsdk
