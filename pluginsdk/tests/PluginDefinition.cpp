#include <catch2/catch.hpp>

#include "rtvamp/pluginsdk/PluginDefinition.hpp"

#include "TestPlugin.hpp"

using rtvamp::pluginsdk::Plugin;

TEST_CASE("PluginDefinition") {
    TestPlugin plugin(48000);

    CHECK(plugin.getInputSampleRate() == 48000);
    CHECK(plugin.initialise(3, 3));

    SECTION("Input domain validation with variant") {
        CHECK_NOTHROW(plugin.process(Plugin::TimeDomainBuffer{}, 0));
        CHECK_THROWS(plugin.process(Plugin::FrequencyDomainBuffer{}, 0));
    }

    SECTION("Check result dimension") {
        const auto& featureSet = plugin.process(Plugin::TimeDomainBuffer{}, 0);
        CHECK(featureSet.size() == 1);
        CHECK(featureSet[0].size() == 3);
    }
}