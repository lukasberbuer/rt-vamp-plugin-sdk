#include <algorithm>  // transform

#include <catch2/catch.hpp>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginLoader.hpp"

using Catch::Matchers::Equals;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;
using rtvamp::hostsdk::Plugin;
using rtvamp::hostsdk::PluginKey;
using rtvamp::hostsdk::PluginLoader;

using namespace std::string_literals;

TEST_CASE("PluginKey") {
    SECTION("Valid") {
        REQUIRE_NOTHROW(PluginKey("x:y"));
        REQUIRE_NOTHROW(PluginKey("library:y"));
        REQUIRE_NOTHROW(PluginKey("x:identifier"));
    }

    SECTION("Invalid") {
        REQUIRE_THROWS(PluginKey(""));
        REQUIRE_THROWS(PluginKey("invalid"));
        REQUIRE_THROWS(PluginKey(":invalid"));
        REQUIRE_THROWS(PluginKey("invalid:"));
        REQUIRE_THROWS(PluginKey("", ""));
    }

    SECTION("Valid, decompose from key") {
        const PluginKey key("library:identifier");
        REQUIRE_THAT(std::string(key.get()),           Equals("library:identifier"));
        REQUIRE_THAT(std::string(key.getLibrary()),    Equals("library"));
        REQUIRE_THAT(std::string(key.getIdentifier()), Equals("identifier"));
    }

    SECTION("Valid, compose from parts") {
        const PluginKey key("library", "identifier");
        REQUIRE_THAT(std::string(key.get()),           Equals("library:identifier"));
        REQUIRE_THAT(std::string(key.getLibrary()),    Equals("library"));
        REQUIRE_THAT(std::string(key.getIdentifier()), Equals("identifier"));
    }

    SECTION("Comparison") {
        REQUIRE(PluginKey("a:b") == PluginKey("a", "b"));
        REQUIRE(PluginKey("a:b") < PluginKey("x:y"));
    }
}

TEST_CASE("PluginLoader getPluginPaths") {
    const auto paths = PluginLoader::getPluginPaths();

    REQUIRE(paths.size() >= 1);

    namespace fs = std::filesystem;

#ifdef _WIN32
    CHECK_THAT(paths, VectorContains(fs::path("C:/Program Files/Vamp Plugins")));
#elif __APPLE__
    CHECK_THAT(paths, VectorContains(fs::path("/Library/Audio/Plug-Ins/Vamp")));
#elif __linux__
    CHECK_THAT(paths, VectorContains(fs::path("/usr/lib/vamp")));
    CHECK_THAT(paths, VectorContains(fs::path("/usr/lib/x86_64-linux-gnu/vamp")));
    CHECK_THAT(paths, VectorContains(fs::path("/usr/local/lib/vamp")));
#endif

    for (const auto& path : paths) {
        if (fs::exists(path)) {
            CHECK(fs::is_directory(path));
        }
    }
}

TEST_CASE("PluginLoader listLibraries") {
    PluginLoader loader;
    const auto libraries = loader.listLibraries();
    REQUIRE(libraries.size() >= 1);

    std::vector<std::string> librariesStem(libraries.size());
    std::transform(
        libraries.begin(),
        libraries.end(),
        librariesStem.begin(),
        [] (const std::filesystem::path& path) { return path.stem().string(); }
    );

    REQUIRE_THAT(librariesStem, VectorContains("example-plugin"s));
    REQUIRE_THAT(librariesStem, !VectorContains("invalid-plugin"s));
}

TEST_CASE("PluginLoader listPlugins") {
    PluginLoader loader;
    const auto plugins = loader.listPlugins();
    REQUIRE(plugins.size() >= 1);
    REQUIRE_THAT(plugins, VectorContains(PluginKey("example-plugin:rms")));
}

TEST_CASE("PluginLoader loadPlugin") {
    PluginLoader loader;
    SECTION("Load valid plugin") {
        auto plugin = loader.loadPlugin("example-plugin:rms", 48000);

        REQUIRE(plugin != nullptr);
        REQUIRE_THAT(plugin->getIdentifier(), Equals("rms"));
    }

    SECTION("Load same plugin twice") {
        auto plugin1 = loader.loadPlugin("example-plugin:rms", 48000);
        auto plugin2 = loader.loadPlugin("example-plugin:rms", 48000);

        REQUIRE(plugin1.get() != plugin2.get());
    }

    SECTION("Invalid plugin key") {
        REQUIRE_THROWS_WITH(
            loader.loadPlugin("invalidkey", 48000),
            StartsWith("Invalid plugin key")
        );
    }

    SECTION("Non-existing plugin") {
        REQUIRE_THROWS_WITH(
            loader.loadPlugin("unknownlib:empty", 48000),
            StartsWith("Plugin not found")
        );
    }
}
