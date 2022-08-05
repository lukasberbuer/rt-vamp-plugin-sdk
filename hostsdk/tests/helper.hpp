#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

inline static std::filesystem::path getVampPath() {
    const char* vampPath = std::getenv("VAMP_PATH");
    if (!vampPath) {
        throw std::runtime_error("VAMP_PATH environment variable must be set");
    }
    return vampPath;
}

inline static std::filesystem::path getLibraryPath(const std::string& stem) {
    for (const auto& file : std::filesystem::directory_iterator(getVampPath())) {
        if (file.is_regular_file() && file.path().stem() == stem) {
            return file.path();
        }
    }
    throw std::runtime_error("Plugin not found");
}
