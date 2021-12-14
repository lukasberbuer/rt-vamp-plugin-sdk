#include <catch2/catch.hpp>

#include "VampOutputDescriptorCpp.h"

using namespace Catch::Matchers;
using namespace rtvamp;

TEST_CASE("VampOutputDescriptorCpp") {
    SECTION("Default values") {
        OutputDescriptor        descriptor;
        VampOutputDescriptorCpp vampDescriptor(descriptor);

        REQUIRE_THAT(vampDescriptor.identifier,  Equals(""));
        REQUIRE_THAT(vampDescriptor.name,        Equals(""));
        REQUIRE_THAT(vampDescriptor.description, Equals(""));
        REQUIRE_THAT(vampDescriptor.unit,        Equals(""));
        REQUIRE(vampDescriptor.hasFixedBinCount == 1);
        REQUIRE(vampDescriptor.binCount == 0);
        REQUIRE(vampDescriptor.binNames == nullptr);
        REQUIRE(vampDescriptor.hasKnownExtents == 0);
        REQUIRE(vampDescriptor.minValue == 0.0f);
        REQUIRE(vampDescriptor.maxValue == 0.0f);
        REQUIRE(vampDescriptor.isQuantized == 0);
        REQUIRE(vampDescriptor.quantizeStep == 0.0f);
        REQUIRE(vampDescriptor.sampleType == vampOneSamplePerStep);
        REQUIRE(vampDescriptor.sampleRate == 0.0f);
    }

    SECTION("Non-matching binCount and binNames size") {
        OutputDescriptor descriptor;
        descriptor.binCount = 3;
        descriptor.binNames = {"a", "b"};

        VampOutputDescriptorCpp vampDescriptor(descriptor);

        REQUIRE_THAT(vampDescriptor.binNames[0], Equals("a"));
        REQUIRE_THAT(vampDescriptor.binNames[1], Equals("b"));
        REQUIRE_THAT(vampDescriptor.binNames[2], Equals(""));
    }
}
