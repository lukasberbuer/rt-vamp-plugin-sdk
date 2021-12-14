#include <catch2/catch.hpp>

#include "rt-vamp-plugin/Plugin.h"

#include "TestPlugin.h"

using namespace rtvamp;

TEST_CASE("Plugin") {
    TestPlugin plugin(48000);

    REQUIRE(plugin.getInputSampleRate() == 48000);
    REQUIRE(plugin.initialise(3, 3));

    SECTION("Input domain validation with variant") {
        REQUIRE_NOTHROW(plugin.process(TimeDomainBuffer{}, 0));
        REQUIRE_THROWS(plugin.process(FrequencyDomainBuffer{}, 0));
    }

    SECTION("Check result dimension") {
        const auto& featureSet = plugin.process(TimeDomainBuffer{}, 0);
        REQUIRE(featureSet.size() == 1);
        REQUIRE(featureSet[0].size() == 3);
    }

    SECTION("Get stored result with getResult") {
        const auto& featureSet = plugin.process(TimeDomainBuffer{}, 0);
        REQUIRE(&featureSet == &plugin.getResult());
    }
}
