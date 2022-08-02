#include <catch2/catch_test_macros.hpp>

#include "rtvamp/pluginsdk.hpp"

#include "TestPlugin.hpp"

TEST_CASE("Plugin") {
    TestPlugin plugin(48000);

    CHECK(plugin.initialise(3, 3));

    SECTION("Input domain validation with variant") {
        CHECK_NOTHROW(plugin.process(TestPlugin::TimeDomainBuffer{}, 0));
        CHECK_THROWS(plugin.process(TestPlugin::FrequencyDomainBuffer{}, 0));
    }

    SECTION("Check result dimension") {
        const auto& featureSet = plugin.process(TestPlugin::TimeDomainBuffer{}, 0);
        CHECK(featureSet.size() == 1);
        CHECK(featureSet[0].size() == 3);
    }
}
