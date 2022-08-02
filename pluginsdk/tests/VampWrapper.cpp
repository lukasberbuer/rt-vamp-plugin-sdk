#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "rtvamp/pluginsdk.hpp"

#include "TestPlugin.hpp"

using Catch::Matchers::Equals;

template <typename U, typename V>
static consteval bool strEqual(U&& u, V&& v) {
    return std::string_view(std::forward<U>(u))
        == std::string_view(std::forward<V>(v));
}

TEST_CASE("VampPluginDescriptorWrapper") {
    using rtvamp::pluginsdk::detail::VampPluginDescriptorWrapper;

    constexpr auto d = VampPluginDescriptorWrapper<TestPlugin>::get();

    STATIC_REQUIRE(d.vampApiVersion == 2);

    STATIC_REQUIRE(strEqual(d.identifier, "test"));
    STATIC_REQUIRE(strEqual(d.name, "Test plugin"));
    STATIC_REQUIRE(strEqual(d.description, "Some random test plugin"));
    STATIC_REQUIRE(strEqual(d.maker, "LB"));
    STATIC_REQUIRE(d.pluginVersion == 1);
    STATIC_REQUIRE(strEqual(d.copyright , "MIT"));
    STATIC_REQUIRE(d.parameterCount == 1);

    constexpr auto* p = d.parameters[0];
    STATIC_REQUIRE(strEqual(p->identifier, "param"));
    STATIC_REQUIRE(strEqual(p->name, "Parameter"));
    STATIC_REQUIRE(strEqual(p->description, "Some random parameter"));
    STATIC_REQUIRE(strEqual(p->unit, ""));
    STATIC_REQUIRE(p->defaultValue == 1.0f);
    STATIC_REQUIRE(p->minValue == 0.0f);
    STATIC_REQUIRE(p->maxValue == 2.0f);
    STATIC_REQUIRE(p->isQuantized == 1);
    STATIC_REQUIRE(p->quantizeStep == 1.0f);

    STATIC_REQUIRE(d.programCount == 2);
    STATIC_REQUIRE(strEqual(d.programs[0], "default"));
    STATIC_REQUIRE(strEqual(d.programs[1], "new"));
}

TEST_CASE("VampOutputDescriptorWrapper") {
    using rtvamp::pluginsdk::PluginBase;
    using rtvamp::pluginsdk::detail::VampOutputDescriptorWrapper;

    SECTION("Default values") {
        PluginBase::OutputDescriptor descriptor{};
        VampOutputDescriptorWrapper  wrapper(descriptor);
        VampOutputDescriptor*        d = wrapper.get();

        CHECK_THAT(d->identifier,  Equals(""));
        CHECK_THAT(d->name,        Equals(""));
        CHECK_THAT(d->description, Equals(""));
        CHECK_THAT(d->unit,        Equals(""));
        CHECK(d->hasFixedBinCount == 1);
        CHECK(d->binCount == 1);
        CHECK(d->binNames == nullptr);
        CHECK(d->hasKnownExtents == 0);
        CHECK(d->minValue == 0.0f);
        CHECK(d->maxValue == 0.0f);
        CHECK(d->isQuantized == 0);
        CHECK(d->quantizeStep == 0.0f);
        CHECK(d->sampleType == vampOneSamplePerStep);
        CHECK(d->sampleRate == 0.0f);
    }

    SECTION("Non-matching binCount and binNames size") {
        PluginBase::OutputDescriptor descriptor;
        descriptor.binCount = 3;
        descriptor.binNames = {"a", "b"};

        VampOutputDescriptorWrapper wrapper(descriptor);
        VampOutputDescriptor*       d = wrapper.get();

        CHECK_THAT(d->binNames[0], Equals("a"));
        CHECK_THAT(d->binNames[1], Equals("b"));
        CHECK_THAT(d->binNames[2], Equals(""));
    }

    SECTION("Copy") {
        PluginBase::OutputDescriptor descriptor;
        descriptor.identifier = "test";

        std::vector<VampOutputDescriptorWrapper> vec;
        vec.emplace_back(descriptor);
        vec.emplace_back(descriptor);  // will resize vector and force copy

        REQUIRE_THAT(vec.at(0).get()->identifier, Equals("test"));
    }
}

TEST_CASE("VampFeatureUnionWrapper") {
    using rtvamp::pluginsdk::detail::VampFeatureUnionWrapper;

    SECTION("Default values") {
        VampFeatureUnionWrapper wrapper{};
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
        VampFeatureUnionWrapper wrapper{};
        REQUIRE(wrapper.getValueCount() == 0);
        REQUIRE(wrapper.get()->v1.values == nullptr);

        wrapper.setValueCount(3);
        REQUIRE(wrapper.getValueCount() == 3);
        REQUIRE(wrapper.get()->v1.valueCount == 3);
        REQUIRE(wrapper.get()->v1.values != nullptr);
    }

    SECTION("Assign values") {
        VampFeatureUnionWrapper wrapper{};
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
    using rtvamp::pluginsdk::detail::VampFeatureListsWrapper;

    SECTION("Empty") {
        VampFeatureListsWrapper<0> wrapper{};
        REQUIRE(wrapper.get() == nullptr);
    }

    SECTION("Assign values per output") {
        VampFeatureListsWrapper<3> wrapper;

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
        VampFeatureListsWrapper<3> wrapper{};

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
