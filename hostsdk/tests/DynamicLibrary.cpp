#include <utility>  // move

#include <catch2/catch_test_macros.hpp>

#include "DynamicLibrary.hpp"

#include "helper.hpp"

using rtvamp::hostsdk::DynamicLibrary;

TEST_CASE("DynamicLibrary") {
    const auto  validPath   = getLibraryPath("example-plugin");
    const char* validSymbol = "vampGetPluginDescriptor";

    SECTION("Fail loading non-existing path") {
        DynamicLibrary dl;
        CHECK_FALSE(dl.load("nonexisting"));
        CHECK_FALSE(dl.isLoaded());
    }

    SECTION("Load/unload") {
        DynamicLibrary dl;
        CHECK(dl.load(validPath));
        CHECK(dl.isLoaded());
        CHECK(dl.path().value() == validPath);

        dl.unload();
        CHECK_FALSE(dl.isLoaded());
        CHECK_FALSE(dl.path());
    }

    SECTION("Load same library multiple times, expect same handles") {
        DynamicLibrary dl1(validPath);
        DynamicLibrary dl2(validPath);
        REQUIRE(dl1.handle() == dl2.handle());
    }

    SECTION("Get function") {
        DynamicLibrary dl;
        REQUIRE(dl.getFunction<void*>(validSymbol) == nullptr);

        dl.load(validPath);
        REQUIRE(dl.getFunction<void*>(validSymbol) != nullptr);
    }

    SECTION("Copy constructor") {
        DynamicLibrary dl1(validPath);
        DynamicLibrary dl2(dl1);

        CHECK(dl2.isLoaded());
        CHECK(dl2.path().value() == validPath);
    }

    SECTION("Copy assignment") {
        DynamicLibrary dl1(validPath);
        auto dl2 = dl1;

        CHECK(dl2.isLoaded());
        CHECK(dl2.path().value() == validPath);
    }

    SECTION("Move constructor") {
        DynamicLibrary dl1(validPath);
        DynamicLibrary dl2(std::move(dl1));

        CHECK(dl2.isLoaded());
        CHECK(dl2.path() == validPath);
    }

    SECTION("Move assignment") {
        DynamicLibrary dl1(validPath);
        auto dl2 = std::move(dl1);

        CHECK(dl2.isLoaded());
        CHECK(dl2.path() == validPath);
    }

    SECTION("Check if library stays open after copy is closed") {
        DynamicLibrary dl1(validPath);
        {
            DynamicLibrary dl2(dl1);
        }
        CHECK(dl1.isLoaded());
        CHECK(dl1.getFunction<void*>(validSymbol) != nullptr);
    }
}
