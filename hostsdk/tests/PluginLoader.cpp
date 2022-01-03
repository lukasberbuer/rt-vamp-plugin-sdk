#include <algorithm>  // transform

#include <catch2/catch.hpp>

#include "rtvamp/hostsdk/PluginLoader.hpp"

using Catch::Matchers::Contains;
using Catch::Matchers::Equals;
using Catch::Matchers::VectorContains;

using namespace std::string_literals;
using namespace rtvamp::hostsdk;

TEST_CASE("PluginLoader::libraryPaths") {
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

TEST_CASE("PluginLoader::listLibraries") {
    const auto libraries = PluginLoader::listLibraries();
    REQUIRE(libraries.size() >= 1);

    std::vector<std::string> librariesStem(libraries.size());
    std::transform(
        libraries.begin(),
        libraries.end(),
        librariesStem.begin(),
        [] (const std::filesystem::path& path) { return path.stem().string(); }
    );

    REQUIRE_THAT(librariesStem, VectorContains("example-plugin"s));
    REQUIRE_THAT(librariesStem, VectorContains("invalid-plugin"s));
}

TEST_CASE("PluginLoader::listPlugins") {
    const auto plugins = PluginLoader::listPlugins();
    REQUIRE(plugins.size() >= 1);
    REQUIRE_THAT(plugins, VectorContains("example-plugin:rms"s));
}
