#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

inline static std::filesystem::path getPluginPath(const std::string& stem) {
    const char* vampPath = std::getenv("VAMP_PATH");
    if (!vampPath) {
        throw std::runtime_error("VAMP_PATH environment variable must be set");
    }

    for (const auto& file : std::filesystem::directory_iterator(vampPath)) {
        if (file.is_regular_file() && file.path().stem() == stem) {
            return file.path();
        }
    }
    throw std::runtime_error("Plugin not found");
}
