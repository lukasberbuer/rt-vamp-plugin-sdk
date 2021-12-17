#include <catch2/catch.hpp>

#include "VampFeatureWrapper.hpp"

using namespace rtvamp;

TEST_CASE("VampFeatureUnionWrapper") {
    SECTION("Default values") {
        VampFeatureUnionWrapper wrapper;
        VampFeatureUnion*       feature = wrapper.get();
        VampFeature&            v1 = feature->v1;
        VampFeatureV2&          v2 = feature->v2;

        CHECK(v1.hasTimestamp == 0);
        CHECK(v1.sec == 0);
        CHECK(v1.nsec == 0);
        CHECK(v1.nsec == 0);
        CHECK(v1.valueCount == 0);
        CHECK(v1.values == nullptr);
        CHECK(v1.label == nullptr);

        CHECK(v2.hasDuration == 0);
        CHECK(v2.durationSec == 0);
        CHECK(v2.durationNsec == 0);
    }

    SECTION("Get/set value count") {
        VampFeatureUnionWrapper wrapper;
        REQUIRE(wrapper.getValueCount() == 0);
        REQUIRE(wrapper.get()->v1.values == nullptr);

        wrapper.setValueCount(3);
        REQUIRE(wrapper.getValueCount() == 3);
        REQUIRE(wrapper.get()->v1.valueCount == 3);
        REQUIRE(wrapper.get()->v1.values != nullptr);
    }

    SECTION("Assign values") {
        VampFeatureUnionWrapper  wrapper;
        const std::vector<float> values{1.0f, 2.0f, 3.0f};

        auto getRawValues = [&] {
            return wrapper.get()->v1.values;
        };

        wrapper.assignValues(values);
        REQUIRE(wrapper.getValueCount() == 3);
        REQUIRE(getRawValues() != nullptr);
        CHECK(getRawValues()[0] == values[0]);
        CHECK(getRawValues()[1] == values[1]);
        CHECK(getRawValues()[2] == values[2]);

        wrapper.assignValues({});
        REQUIRE(wrapper.getValueCount() == 0);
        REQUIRE(getRawValues() == nullptr);
    }
}

TEST_CASE("VampFeatureListsWrapper") {
    SECTION("Empty") {
        VampFeatureListsWrapper wrapper;
        REQUIRE(wrapper.get() == nullptr);
    }

    SECTION("Get/set output count") {
        VampFeatureListsWrapper wrapper;
        REQUIRE(wrapper.getOutputCount() == 0);

        wrapper.setOutputCount(3);
        REQUIRE(wrapper.getOutputCount() == 3);
        // check if size of raw array is valid -> access would fail with address sanitizer
        REQUIRE(wrapper.get()[0].features);
        REQUIRE(wrapper.get()[1].features);
        REQUIRE(wrapper.get()[2].features);
    }

    SECTION("Assign values per output") {
        VampFeatureListsWrapper  wrapper;
        wrapper.setOutputCount(3);

        auto getValueCount = [&](size_t outputNumber) {
            return wrapper.get()[outputNumber].features[0].v1.valueCount;
        };
        auto getRawValues = [&](size_t outputNumber) {
            return wrapper.get()[outputNumber].features[0].v1.values;
        };

        CHECK(getValueCount(0) == 0);
        CHECK(getValueCount(1) == 0);
        CHECK(getValueCount(2) == 0);

        const std::vector<float> values{1.1f, 2.2f, 3.3f};
        wrapper.assignValues(2, values);

        CHECK(getValueCount(0) == 0);
        CHECK(getValueCount(1) == 0);
        CHECK(getValueCount(2) == 3);

        CHECK(getRawValues(2)[0] == values[0]);
        CHECK(getRawValues(2)[1] == values[1]);
        CHECK(getRawValues(2)[2] == values[2]);
    }

    SECTION("Assign values") {
        VampFeatureListsWrapper  wrapper;
        // don't set output count manually
        // wrapper.setOutputCount(3);

        const std::vector<std::vector<float>> values{
            {0.0f},
            {1.0f, 1.1f},
            {2.0f, 2.1f, 2.2f},
        };
        wrapper.assignValues(values);

        auto getValueCount = [&](size_t outputNumber) {
            return wrapper.get()[outputNumber].features[0].v1.valueCount;
        };
        auto getRawValues = [&](size_t outputNumber) {
            return wrapper.get()[outputNumber].features[0].v1.values;
        };

        CHECK(getValueCount(0) == 1);
        CHECK(getValueCount(1) == 2);
        CHECK(getValueCount(2) == 3);

        CHECK(getRawValues(0)[0] == values[0][0]);
        CHECK(getRawValues(1)[0] == values[1][0]);
        CHECK(getRawValues(1)[1] == values[1][1]);
        CHECK(getRawValues(2)[0] == values[2][0]);
        CHECK(getRawValues(2)[1] == values[2][1]);
        CHECK(getRawValues(2)[2] == values[2][2]);
    }
}
