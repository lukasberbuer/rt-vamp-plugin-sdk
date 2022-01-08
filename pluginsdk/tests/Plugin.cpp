#include <catch2/catch.hpp>

#include "rtvamp/pluginsdk/Plugin.hpp"

#include "TestPlugin.hpp"

using rtvamp::pluginsdk::PluginBase;

TEST_CASE("Plugin") {
    TestPlugin plugin(48000);

    CHECK(plugin.initialise(3, 3));

    SECTION("Input domain validation with variant") {
        CHECK_NOTHROW(plugin.process(PluginBase::TimeDomainBuffer{}, 0));
        CHECK_THROWS(plugin.process(PluginBase::FrequencyDomainBuffer{}, 0));
    }

    SECTION("Check result dimension") {
        const auto& featureSet = plugin.process(PluginBase::TimeDomainBuffer{}, 0);
        CHECK(featureSet.size() == 1);
        CHECK(featureSet[0].size() == 3);
    }
}