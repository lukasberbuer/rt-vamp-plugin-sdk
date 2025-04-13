#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "rtvamp/pluginsdk.hpp"

using namespace rtvamp::pluginsdk;
using Catch::Matchers::Equals;

TEST_CASE("VampOutputDescriptor") {
    SECTION("Default values") {
        PluginBase::OutputDescriptor descriptor{};

        auto d = detail::makeVampOutputDescriptor(descriptor);
        CHECK(d.identifier == nullptr);
        CHECK(d.name == nullptr);
        CHECK(d.description == nullptr);
        CHECK(d.unit == nullptr);
        CHECK(d.hasFixedBinCount == 1);
        CHECK(d.binCount == 1);
        CHECK(d.binNames == nullptr);
        CHECK(d.hasKnownExtents == 0);
        CHECK(d.minValue == 0.0F);
        CHECK(d.maxValue == 0.0F);
        CHECK(d.isQuantized == 0);
        CHECK(d.quantizeStep == 0.0F);
        CHECK(d.sampleType == vampOneSamplePerStep);
        CHECK(d.sampleRate == 0.0F);
        detail::clear(d);
    }

    SECTION("Random values") {
        PluginBase::OutputDescriptor descriptor{};
        descriptor.identifier = "identifier";
        descriptor.name = "name";
        descriptor.description = "description";
        descriptor.unit = "unit";
        // ...

        auto d = detail::makeVampOutputDescriptor(descriptor);
        CHECK_THAT(d.identifier, Equals("identifier"));
        CHECK_THAT(d.name, Equals("name"));
        CHECK_THAT(d.description, Equals("description"));
        CHECK_THAT(d.unit, Equals("unit"));
        detail::clear(d);
    }

    SECTION("Non-matching binCount and binNames size") {
        PluginBase::OutputDescriptor descriptor;
        descriptor.binCount = 3;
        descriptor.binNames = {"a", "b"};

        auto d = detail::makeVampOutputDescriptor(descriptor);
        CHECK_THAT(d.binNames[0], Equals("a"));
        CHECK_THAT(d.binNames[1], Equals("b"));
        CHECK(d.binNames[2] == nullptr);
        detail::clear(d);
    }
}

TEST_CASE("VampFeatureUnion") {
    SECTION("assignValues") {
        VampFeatureUnion featureUnion{};
        const std::vector<float> values{1.0F, 2.0F, 3.0F};
        detail::assignValues(featureUnion, values);
        CHECK(featureUnion.v1.valueCount == 3);
        CHECK(featureUnion.v1.values != nullptr);
        CHECK(featureUnion.v1.values != values.data());
        CHECK(featureUnion.v1.values[0] == values[0]);
        CHECK(featureUnion.v1.values[1] == values[1]);
        CHECK(featureUnion.v1.values[2] == values[2]);
        detail::clear(featureUnion);
    }
}
