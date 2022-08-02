#include <catch2/catch_test_macros.hpp>

#include "rtvamp/pluginsdk.hpp"

#include "TestPlugin.hpp"

using rtvamp::pluginsdk::EntryPoint;

TEST_CASE("EntryPoint") {
    using EP = EntryPoint<TestPlugin, TestPlugin>;

    SECTION("Valid version range") {
        REQUIRE(EP::getDescriptor(0, 0) == nullptr);
        REQUIRE(EP::getDescriptor(1, 0) != nullptr);
        REQUIRE(EP::getDescriptor(2, 0) != nullptr);
        REQUIRE(EP::getDescriptor(3, 0) == nullptr);
    }

    SECTION("Valid index range") {
        REQUIRE(EP::getDescriptor(2, 0) != nullptr);
        REQUIRE(EP::getDescriptor(2, 1) != nullptr);
        REQUIRE(EP::getDescriptor(2, 2) == nullptr);
    }

    SECTION("Check descriptor") {
        // descriptors of the same plugin should point to the same memory location
        REQUIRE(EP::getDescriptor(2, 0) == EP::getDescriptor(2, 1));
    }
}
