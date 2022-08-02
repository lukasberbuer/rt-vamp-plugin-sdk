#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "rtvamp/hostsdk/PluginKey.hpp"

using Catch::Matchers::Equals;
using rtvamp::hostsdk::PluginKey;

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
