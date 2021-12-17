#include <catch2/catch.hpp>

#include "rt-vamp-plugin/Plugin.h"

#include "TestPlugin.h"

using namespace rtvamp;

TEST_CASE("Plugin") {
    TestPlugin plugin(48000);

    CHECK(plugin.getInputSampleRate() == 48000);
    CHECK(plugin.initialise(3, 3));

    SECTION("Input domain validation with variant") {
        CHECK_NOTHROW(plugin.process(TimeDomainBuffer{}, 0));
        CHECK_THROWS(plugin.process(FrequencyDomainBuffer{}, 0));
    }

    SECTION("Check result dimension") {
        const auto& featureSet = plugin.process(TimeDomainBuffer{}, 0);
        CHECK(featureSet.size() == 1);
        CHECK(featureSet[0].size() == 3);
    }

    SECTION("Get stored result with getResult") {
        const auto& featureSet = plugin.process(TimeDomainBuffer{}, 0);
        CHECK(&featureSet == &plugin.getResult());
    }
}
