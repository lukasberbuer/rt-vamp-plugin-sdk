#include <catch2/catch.hpp>

#include "rtvamp/pluginsdk/EntryPoint.hpp"
#include "rtvamp/pluginsdk/PluginAdapter.hpp"

#include "TestPlugin.hpp"

using Catch::Matchers::Equals;
using namespace rtvamp::pluginsdk;

TEST_CASE("EntryPoint") {
    using EP = EntryPoint<TestPlugin, TestPlugin>;

    SECTION("Invalid version") {
        REQUIRE(EP::getDescriptor(0, 0) == nullptr);  // version 0
    }

    SECTION("Valid index range") {
        REQUIRE(EP::getDescriptor(2, 0) != nullptr);
        REQUIRE(EP::getDescriptor(2, 1) != nullptr);
        REQUIRE(EP::getDescriptor(2, 2) == nullptr);
    }

    SECTION("Check descriptor") {
        REQUIRE(EP::getDescriptor(2, 0) != nullptr);
        REQUIRE(EP::getDescriptor(2, 0) == PluginAdapter<TestPlugin>::getDescriptor());

        // descriptors of the same plugin should point to the same memory location
        REQUIRE(EP::getDescriptor(2, 0) == EP::getDescriptor(2, 1));
    }
}
