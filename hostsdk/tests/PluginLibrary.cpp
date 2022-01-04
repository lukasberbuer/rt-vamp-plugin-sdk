#include <algorithm>  // find_if
#include <iostream>

#include <catch2/catch.hpp>

#include "PluginLibrary.hpp"

using Catch::Matchers::Equals;
using rtvamp::hostsdk::PluginLibrary;

static std::filesystem::path getPluginPath(const std::string& stem) {
    std::filesystem::path vampPath(std::getenv("VAMP_PATH"));

    for (const auto& file : std::filesystem::directory_iterator(vampPath)) {
        if (file.is_regular_file() && file.path().stem() == stem) {
            return file.path();
        }
    }
    throw std::runtime_error("Plugin not found");
}

TEST_CASE("PluginLibrary") {
    SECTION("Non-existing library") {
        REQUIRE_THROWS_WITH(
            PluginLibrary("non-existing.so"),
            "Dynamic library does not exist: \"non-existing.so\""
        );
    }

    SECTION("Invalid plugin") {
        const auto path = getPluginPath("invalid-plugin");
        REQUIRE_THROWS_WITH(
            PluginLibrary(path),
            "Undefined symbol in dynamic library: vampGetPluginDescriptor"
        );
    }

    SECTION("example-plugin") {
        const auto path = getPluginPath("example-plugin");
        PluginLibrary library(path);

        REQUIRE_THAT(library.getLibraryName(), Equals("example-plugin"));
        REQUIRE(library.getDescriptors().size() == 1);

        const auto plugins = library.getPlugins();
        REQUIRE(plugins.size() == 1);
        REQUIRE_THAT(plugins[0], Equals("rms"));

        SECTION("Same handle for multiple library loads -> same plugin descriptors") {
            PluginLibrary library2(path);
            REQUIRE_THAT(library.getDescriptors(), Equals(library2.getDescriptors()));
        }
    }
}
