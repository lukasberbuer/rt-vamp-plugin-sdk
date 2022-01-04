#include <utility>  // move

#include <catch2/catch.hpp>

#include "DynamicLibrary.hpp"

#include "helper.hpp"

using rtvamp::hostsdk::DynamicLibrary;

TEST_CASE("DynamicLibrary") {
    const auto validPath = getPluginPath("example-plugin");

    SECTION("Fail loading non-existing path") {
        DynamicLibrary dl;
        REQUIRE_FALSE(dl.load("nonexisting"));
        REQUIRE_FALSE(dl.isLoaded());
    }

    SECTION("Load/unload") {
        DynamicLibrary dl;
        REQUIRE(dl.load(validPath));
        REQUIRE(dl.isLoaded());

        dl.unload();
        REQUIRE_FALSE(dl.isLoaded());
    }

    SECTION("Move constructor") {
        DynamicLibrary dl1(validPath);
        DynamicLibrary dl2(std::move(dl1));

        CHECK_FALSE(dl1.isLoaded());
        CHECK(dl2.isLoaded());
    }

    SECTION("Get function") {
        DynamicLibrary dl;
        REQUIRE(dl.getFunction("vampGetPluginDescriptor") == nullptr);

        dl.load(validPath);
        REQUIRE(dl.getFunction("vampGetPluginDescriptor") != nullptr);
    }
}
