#pragma once

#include <filesystem>
#include <string_view>

#include "config.hpp"

inline std::filesystem::path getLibraryPath(std::string_view stem) {
    for (const auto& path : searchPaths) {
        for (const auto& file : std::filesystem::directory_iterator(path)) {
            if (file.is_regular_file() && file.path().stem() == stem) {
                return file.path();
            }
        }
    }
    throw std::runtime_error("Plugin not found");
}
