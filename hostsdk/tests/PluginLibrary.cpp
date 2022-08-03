#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "rtvamp/hostsdk/PluginLibrary.hpp"

#include "helper.hpp"

using Catch::Matchers::Equals;
using Catch::Matchers::StartsWith;
using rtvamp::hostsdk::Plugin;
using rtvamp::hostsdk::PluginKey;
using rtvamp::hostsdk::PluginLibrary;

TEST_CASE("PluginLibrary") {
    SECTION("Non-existing library") {
        REQUIRE_THROWS_WITH(
            PluginLibrary("non-existing.so"),
            "Dynamic library does not exist: \"non-existing.so\""
        );
    }

    SECTION("Invalid library") {
        const auto path = getLibraryPath("invalid-plugin");
        REQUIRE_THROWS_WITH(
            PluginLibrary(path),
            "Undefined symbol in dynamic library: vampGetPluginDescriptor"
        );
    }

    SECTION("Valid library (example-plugin)") {
        const auto path = getLibraryPath("example-plugin");
        PluginLibrary library(path);

        REQUIRE(library.getLibraryPath() == path);

        REQUIRE_THAT(library.getLibraryName(), Equals("example-plugin"));

        const auto count = library.getPluginCount();
        REQUIRE(count >= 2);

        const auto keys = library.listPlugins();
        REQUIRE(keys.size() == count);
        REQUIRE(keys[0] == PluginKey("example-plugin", "rms"));
        REQUIRE(keys[1] == PluginKey("example-plugin", "spectralrolloff"));

        SECTION("Load with invalid plugin key") {
            REQUIRE_THROWS_WITH(
                library.loadPlugin("invalidkey", 48000),
                StartsWith("Invalid plugin key")
            );
        }

        SECTION("Load with invalid plugin index") {
            REQUIRE_THROWS_WITH(
                library.loadPlugin(111, 48000),
                StartsWith("Invalid plugin index")
            );
        }

        SECTION("Load all available plugins") {
            // by key
            for (auto&& key : library.listPlugins()) {
                REQUIRE_NOTHROW(library.loadPlugin(key, 48000));
            }
            // by index
            for (size_t i = 0; i < library.getPluginCount(); ++i) {
                REQUIRE_NOTHROW(library.loadPlugin(i, 48000));
            }
        }

        SECTION("Load same plugin twice") {
            auto plugin1 = library.loadPlugin(0, 48000);
            auto plugin2 = library.loadPlugin(0, 48000);
            REQUIRE(plugin1.get() != plugin2.get());
        }
    }

    SECTION("Load plugin & check lifetime of library handle") {
        std::unique_ptr<Plugin> plugin;

        {
            const auto path = getLibraryPath("example-plugin");
            PluginLibrary library(path);
            plugin = library.loadPlugin("example-plugin:rms", 48000);

            CHECK(plugin->getOutputCount() == 1);
        }

        // PluginLibrary destructed, but library handle should be alive

        CHECK(plugin->getOutputCount() == 1);
    }
}
