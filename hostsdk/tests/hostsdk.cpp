#include <algorithm>  // transform

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "rtvamp/hostsdk.hpp"

#include "helper.hpp"

using Catch::Matchers::Equals;
using Catch::Matchers::StartsWith;
using Catch::Matchers::Contains;
using rtvamp::hostsdk::PluginKey;

TEST_CASE("getVampPaths") {
    const auto paths = rtvamp::hostsdk::getVampPaths();

    REQUIRE(paths.size() >= 1);

    namespace fs = std::filesystem;

#ifdef _WIN32
    CHECK_THAT(paths, Contains(fs::path("C:/Program Files/Vamp Plugins")));
#elif __APPLE__
    CHECK_THAT(paths, Contains(fs::path("/Library/Audio/Plug-Ins/Vamp")));
#elif __linux__
    CHECK_THAT(paths, Contains(fs::path("/usr/lib/vamp")));
    CHECK_THAT(paths, Contains(fs::path("/usr/lib/x86_64-linux-gnu/vamp")));
    CHECK_THAT(paths, Contains(fs::path("/usr/local/lib/vamp")));
#endif

    for (const auto& path : paths) {
        if (fs::exists(path)) {
            CHECK(fs::is_directory(path));
        }
    }
}

TEST_CASE("isVampLibrary") {
    SECTION("Non-existing library") {
        REQUIRE_FALSE(rtvamp::hostsdk::isVampLibrary("nonexistinglibrary.dll"));
    }

    SECTION("Existing but invalid library") {
        const auto path = getLibraryPath("invalid-plugin");
        REQUIRE_FALSE(rtvamp::hostsdk::isVampLibrary(path));
    }

    SECTION("Valid library") {
        const auto path = getLibraryPath("example-plugin");
        REQUIRE(rtvamp::hostsdk::isVampLibrary(path));
    }
}

TEST_CASE("listLibraries") {
    auto getLibraryStems = [](std::span<const std::filesystem::path> libraryPaths) {
        std::vector<std::string> result(libraryPaths.size());
        std::transform(
            libraryPaths.begin(),
            libraryPaths.end(),
            result.begin(),
            [] (const std::filesystem::path& path) { return path.stem().string(); }
        );
        return result;
    };

    SECTION("Invalid path") {
        const auto libraries = rtvamp::hostsdk::listLibraries("thisshouldbeaninvalidpath");
        REQUIRE(libraries.size() == 0);
    }

    SECTION("VAMP_PATH") {
        const auto libraries = rtvamp::hostsdk::listLibraries(getVampPath());
        REQUIRE(libraries.size() >= 1);

        const auto stems = getLibraryStems(libraries);
        CHECK_THAT(stems, Contains("example-plugin"));
        CHECK_THAT(stems, !Contains("invalid-plugin"));
    }

    SECTION("Default paths including VAMP_PATH") {
        const auto libraries = rtvamp::hostsdk::listLibraries();
        REQUIRE(libraries.size() >= 1);
    }
}

TEST_CASE("loadLibrary") {
    REQUIRE_THROWS(
        rtvamp::hostsdk::loadLibrary("nonexistinglibrary.dll")
    );
    REQUIRE_THROWS(
        rtvamp::hostsdk::loadLibrary(getLibraryPath("invalid-plugin"))
    );
    REQUIRE_NOTHROW(
        rtvamp::hostsdk::loadLibrary(getLibraryPath("example-plugin"))
    );
}

TEST_CASE("listPlugins") {
    SECTION("Invalid path") {
        const auto plugins = rtvamp::hostsdk::listPlugins("thisshouldbeaninvalidpath");
        REQUIRE(plugins.size() == 0);
    }

    SECTION("Library path") {
        const auto plugins = rtvamp::hostsdk::listPlugins(getLibraryPath("example-plugin"));
        REQUIRE(plugins.size() >= 1);
        REQUIRE_THAT(plugins, Contains(PluginKey("example-plugin:rms")));
    }

    SECTION("Default paths including VAMP_PATH") {
        const auto plugins = rtvamp::hostsdk::listPlugins();
        REQUIRE(plugins.size() >= 1);
        REQUIRE_THAT(plugins, Contains(PluginKey("example-plugin:rms")));
    }
}

TEST_CASE("loadPlugin") {
    SECTION("Non-existing plugin") {
        REQUIRE_THROWS_WITH(
            rtvamp::hostsdk::loadPlugin("unknownlib:empty", 48000),
            StartsWith("Plugin not found")
        );
    }

    SECTION("Invalid plugin key") {
        REQUIRE_THROWS_WITH(
            rtvamp::hostsdk::loadPlugin("invalidkey", 48000),
            StartsWith("Invalid plugin key")
        );
    }

    SECTION("Load valid plugin") {
        auto plugin = rtvamp::hostsdk::loadPlugin("example-plugin:rms", 48000);
        REQUIRE(plugin != nullptr);
        REQUIRE_THAT(std::string(plugin->getIdentifier()), Equals("rms"));
    }

    SECTION("Load valid plugin with given library paths") {
        const std::vector<std::filesystem::path> libraryPaths{getLibraryPath("example-plugin")};
        auto plugin = rtvamp::hostsdk::loadPlugin("example-plugin:rms", 48000, libraryPaths);
        REQUIRE(plugin != nullptr);
    }
}
