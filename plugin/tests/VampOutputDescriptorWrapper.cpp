#include <catch2/catch.hpp>

#include "VampOutputDescriptorWrapper.hpp"

using namespace Catch::Matchers;
using namespace rtvamp;

TEST_CASE("VampOutputDescriptorWrapper") {
    SECTION("Default values") {
        OutputDescriptor            descriptor;
        VampOutputDescriptorWrapper wrapper(descriptor);
        VampOutputDescriptor&       d = wrapper.get();

        CHECK_THAT(d.identifier,  Equals(""));
        CHECK_THAT(d.name,        Equals(""));
        CHECK_THAT(d.description, Equals(""));
        CHECK_THAT(d.unit,        Equals(""));
        CHECK(d.hasFixedBinCount == 1);
        CHECK(d.binCount == 0);
        CHECK(d.binNames == nullptr);
        CHECK(d.hasKnownExtents == 0);
        CHECK(d.minValue == 0.0f);
        CHECK(d.maxValue == 0.0f);
        CHECK(d.isQuantized == 0);
        CHECK(d.quantizeStep == 0.0f);
        CHECK(d.sampleType == vampOneSamplePerStep);
        CHECK(d.sampleRate == 0.0f);
    }

    SECTION("Non-matching binCount and binNames size") {
        OutputDescriptor descriptor;
        descriptor.binCount = 3;
        descriptor.binNames = {"a", "b"};

        VampOutputDescriptorWrapper wrapper(descriptor);
        VampOutputDescriptor&       d = wrapper.get();

        CHECK_THAT(d.binNames[0], Equals("a"));
        CHECK_THAT(d.binNames[1], Equals("b"));
        CHECK_THAT(d.binNames[2], Equals(""));
    }
}
