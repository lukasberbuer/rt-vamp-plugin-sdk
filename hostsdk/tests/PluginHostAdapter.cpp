#include <set>

#include <catch2/catch.hpp>

#include "vamp/vamp.h"

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "TestPluginDescriptor.hpp"

using Catch::Matchers::Equals;
using rtvamp::hostsdk::Plugin;
using rtvamp::hostsdk::PluginHostAdapter;

TEST_CASE("PluginHostAdapter plugin requirements") {
    SECTION("Min channel count == 1") {
        auto descriptor = TestPluginDescriptor::get();
        descriptor.getMinChannelCount = [](VampPluginHandle) { return 2u; };
        REQUIRE_THROWS_WITH(
            PluginHostAdapter(descriptor, 48000),
            "Minimum channel count > 1 not supported"
        );
    }

    SECTION("hasFixedBinCount == false") {
        auto descriptor = TestPluginDescriptor::get();
        static auto outputs = TestPluginDescriptor::outputs;
        outputs[0].hasFixedBinCount = 0;
        descriptor.getOutputDescriptor = [](VampPluginHandle, unsigned int) {
            return const_cast<VampOutputDescriptor*>(outputs.data());
        };
        REQUIRE_THROWS_WITH(
            PluginHostAdapter(descriptor, 48000),
            "Dynamic bin count of output \"output\" not supported"
        );
    }

    SECTION("sampleType != vampOneSamplePerStep") {
        auto descriptor = TestPluginDescriptor::get();
        static auto outputs = TestPluginDescriptor::outputs;
        outputs[0].sampleType = GENERATE(vampFixedSampleRate, vampVariableSampleRate);
        descriptor.getOutputDescriptor = [](VampPluginHandle, unsigned int) {
            return const_cast<VampOutputDescriptor*>(outputs.data());
        };
        REQUIRE_THROWS_WITH(
            PluginHostAdapter(descriptor, 48000),
            "Sample type of output \"output\" not supported (OneSamplePerStep required)"
        );
    }
}

TEST_CASE("PluginHostAdapter static plugin data") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    CHECK_THAT(plugin.getIdentifier(),  Equals(descriptor.identifier));
    CHECK_THAT(plugin.getName(),        Equals(descriptor.name));
    CHECK_THAT(plugin.getDescription(), Equals(descriptor.description));
    CHECK_THAT(plugin.getMaker(),       Equals(descriptor.maker));
    CHECK_THAT(plugin.getCopyright(),   Equals(descriptor.copyright));

    CHECK(plugin.getPluginVersion() == descriptor.pluginVersion);
    CHECK(plugin.getInputDomain()   == Plugin::InputDomain::Time);

    SECTION("Parameter") {
        auto parameters = plugin.getParameterList();
        REQUIRE(parameters.size() == TestPluginDescriptor::parameters.size());

        for (size_t i = 0; i < parameters.size(); ++i) {
            const auto& vampParameter = TestPluginDescriptor::parameters[i];
            auto& parameter           = parameters[i];

            CHECK_THAT(parameter.identifier,  Equals(vampParameter.identifier));
            CHECK_THAT(parameter.name,        Equals(vampParameter.name));
            CHECK_THAT(parameter.description, Equals(vampParameter.description));
            CHECK_THAT(parameter.unit,        Equals(vampParameter.unit));
            CHECK(parameter.minValue     == vampParameter.minValue);
            CHECK(parameter.maxValue     == vampParameter.maxValue);
            CHECK(parameter.defaultValue == vampParameter.defaultValue);
            CHECK(parameter.isQuantized  == (vampParameter.isQuantized == 1));
            CHECK(parameter.quantizeStep == vampParameter.quantizeStep);
        }
    };

    SECTION("Programs") {
        auto programs = plugin.getProgramList();
        REQUIRE(programs.size() == descriptor.programCount);

        for (size_t i = 0; i < descriptor.programCount; ++i) {
            CHECK_THAT(programs[i], Equals(descriptor.programs[i]));
        }
    }
}

TEST_CASE("PluginHostAdapter get/set parameter") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static std::array parameters{
        TestPluginDescriptor::parameters[0].defaultValue,
        TestPluginDescriptor::parameters[1].defaultValue
    };

    descriptor.getParameter = [](VampPluginHandle, int index) {
        return parameters.at(index);
    };
    descriptor.setParameter = [](VampPluginHandle, int index, float value) {
        parameters.at(index) = value;
    };

    REQUIRE(plugin.getParameter("invalid") == 0.0f);
    REQUIRE_NOTHROW(plugin.setParameter("invalid", 0.0f));

    REQUIRE(plugin.getParameter("param1") == 1.0f);
    plugin.setParameter("param1", 2.0f);
    REQUIRE(plugin.getParameter("param1") == 2.0f);

    REQUIRE(plugin.getParameter("param2") == -1.0f);
    plugin.setParameter("param2", -5.0f);
    REQUIRE(plugin.getParameter("param2") == -5.0f);
}

TEST_CASE("PluginHostAdapter get/set programs") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static unsigned int programIndex = 0;

    descriptor.getCurrentProgram = [](VampPluginHandle) {
        return programIndex;
    };
    descriptor.selectProgram = [](VampPluginHandle, unsigned int index) {
        if (index < TestPluginDescriptor::programs.size()) {
            programIndex = index;
        }
    };

    REQUIRE_THAT(plugin.getCurrentProgram(), Equals(descriptor.programs[0]));

    plugin.selectProgram("invalid");
    REQUIRE_THAT(plugin.getCurrentProgram(), Equals(descriptor.programs[0]));

    plugin.selectProgram(descriptor.programs[1]);
    REQUIRE_THAT(plugin.getCurrentProgram(), Equals(descriptor.programs[1]));
}

TEST_CASE("PluginHostAdapter preferred step/block size") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    descriptor.getPreferredStepSize  = [](VampPluginHandle) { return 512u; };
    descriptor.getPreferredBlockSize = [](VampPluginHandle) { return 1024u; };

    REQUIRE(plugin.getPreferredStepSize()  == 512);
    REQUIRE(plugin.getPreferredBlockSize() == 1024);
}

TEST_CASE("PluginHostAdapter outputs") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    const auto outputCount = plugin.getOutputCount();
    REQUIRE(outputCount == TestPluginDescriptor::outputs.size());

    const auto outputs = plugin.getOutputDescriptors();
    REQUIRE(outputs.size() == outputCount);

    for (size_t i = 0; i < outputCount; ++i) {
        const auto& vampOutput = TestPluginDescriptor::outputs[i];
        const auto& output     = outputs[i];

        CHECK_THAT(output.identifier,  Equals(vampOutput.identifier));
        CHECK_THAT(output.name,        Equals(vampOutput.name));
        CHECK_THAT(output.description, Equals(vampOutput.description));
        CHECK_THAT(output.unit,        Equals(vampOutput.unit));

        CHECK(output.binCount == vampOutput.binCount);
        if (vampOutput.binNames) {
            for (size_t j = 0; j < vampOutput.binCount; ++j) {
                CHECK_THAT(output.binNames[j], Equals(vampOutput.binNames[j]));
            }
        }

        CHECK(output.hasKnownExtents == vampOutput.hasKnownExtents);
        CHECK(output.minValue        == vampOutput.minValue);
        CHECK(output.maxValue        == vampOutput.maxValue);
        CHECK(output.isQuantized     == (vampOutput.isQuantized == 1));
        CHECK(output.quantizeStep    == vampOutput.quantizeStep);
    }
}

TEST_CASE("PluginHostAdapter release output descriptors") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static std::set<const VampOutputDescriptor*> released;

    descriptor.releaseOutputDescriptor = [](VampOutputDescriptor* d) {
        released.insert(d);
    };

    plugin.getOutputDescriptors();

    for (const auto& d : TestPluginDescriptor::outputs) {
        CHECK(released.contains(&d));
    }
}

TEST_CASE("PluginHostAdapter initialise") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static unsigned int inputChannelsInit = 0;
    static unsigned int stepSizeInit      = 0;
    static unsigned int blockSizeInit     = 0;

    descriptor.initialise = [](
        VampPluginHandle,
        unsigned int inputChannels,
        unsigned int stepSize, 
        unsigned int blockSize
    )  -> int {
        inputChannelsInit = inputChannels;
        stepSizeInit      = stepSize;
        blockSizeInit     = blockSize;
        return 1;
    };

    REQUIRE(plugin.initialise(512, 1024));
    REQUIRE(inputChannelsInit == 1);
    REQUIRE(stepSizeInit      == 512);
    REQUIRE(blockSizeInit     == 1024);
}

TEST_CASE("PluginHostAdapter reset") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static bool reset = false;
    descriptor.reset = [](VampPluginHandle) { reset = true; };

    plugin.reset();
    REQUIRE(reset);
}

TEST_CASE("PluginHostAdapter process") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    static std::vector<float>              values{1.1f, 2.2f, 3.3f};
    static std::array<VampFeatureUnion, 2> featureUnion{};
    featureUnion[0].v1.valueCount = values.size();
    featureUnion[0].v1.values     = values.data();
    static VampFeatureList featureList{
        .featureCount = 1,
        .features     = featureUnion.data(),
    };

    static int secProcess  = 0;
    static int nsecProcess = 0;

    descriptor.process = [](
        VampPluginHandle, const float* const* inputBuffers, int sec, int nsec
    ) -> VampFeatureList* {
        secProcess  = sec;
        nsecProcess = nsec;
        return &featureList;
    };

    REQUIRE(plugin.initialise(0, 0));
    auto result = plugin.process(Plugin::TimeDomainBuffer{}, 1'000'000'123);

    REQUIRE(secProcess  == 1);
    REQUIRE(nsecProcess == 123);

    REQUIRE(result.size() == 1);
    REQUIRE_THAT(result[0], Equals(values));

    SECTION("Wrong input domain / variant type") {
        REQUIRE_THROWS(plugin.process(Plugin::FrequencyDomainBuffer{}, 0));
    }

    SECTION("Release feature set after process") {
        static bool released = false;
        descriptor.releaseFeatureSet = [](VampFeatureList*) { released = true; };
        plugin.process(Plugin::TimeDomainBuffer{}, 0);
        REQUIRE(released);
    };
}