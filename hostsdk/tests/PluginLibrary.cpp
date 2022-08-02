#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "PluginLibrary.hpp"

#include "helper.hpp"

using Catch::Matchers::Equals;
using rtvamp::hostsdk::PluginKey;
using rtvamp::hostsdk::PluginLibrary;

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

        REQUIRE_THAT(library.getLibraryPath(), Equals(path));
        REQUIRE_THAT(library.getLibraryName(), Equals("example-plugin"));

        const auto descriptors = library.getDescriptors();
        REQUIRE(descriptors.size() >= 2);
        REQUIRE_THAT(descriptors[0]->identifier, Equals("rms"));
        REQUIRE_THAT(descriptors[1]->identifier, Equals("spectralrolloff"));

        const auto keys = library.listPlugins();
        REQUIRE(keys.size() >= 2);
        REQUIRE(keys[0] == PluginKey("example-plugin", "rms"));
        REQUIRE(keys[1] == PluginKey("example-plugin", "spectralrolloff"));

        SECTION("Same handle for multiple library loads -> same plugin descriptors") {
            PluginLibrary library2(path);
            REQUIRE_THAT(library.getDescriptors(), Equals(library2.getDescriptors()));
        }
    }
}
