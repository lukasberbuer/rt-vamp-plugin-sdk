#include <catch2/catch.hpp>

#include "rtvamp/pluginsdk/EntryPoint.hpp"
#include "rtvamp/pluginsdk/PluginAdapter.hpp"

#include "TestPlugin.hpp"

using Catch::Matchers::Equals;
using namespace rtvamp::pluginsdk;

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
        REQUIRE(EP::getDescriptor(2, 0) != nullptr);
        REQUIRE(EP::getDescriptor(2, 0) == PluginAdapter<TestPlugin>::getDescriptor());

        // descriptors of the same plugin should point to the same memory location
        REQUIRE(EP::getDescriptor(2, 0) == EP::getDescriptor(2, 1));
    }
}
