#include <set>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "vamp/vamp.h"

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "TestPluginDescriptor.hpp"

using Catch::Matchers::Equals;
using rtvamp::hostsdk::Plugin;
using rtvamp::hostsdk::PluginHostAdapter;

TEST_CASE("PluginHostAdapter plugin requirements") {
    auto descriptor = TestPluginDescriptor::get();

    SECTION("Missing function pointer") {
        descriptor.getOutputCount = nullptr;
        REQUIRE_THROWS_WITH(
            PluginHostAdapter(descriptor, 48000),
            "Missing function pointer to getOutputCount"
        );
    }

    SECTION("Min channel count == 1") {
        descriptor.getMinChannelCount = [](VampPluginHandle) { return 2u; };
        REQUIRE_THROWS_WITH(
            PluginHostAdapter(descriptor, 48000),
            "Minimum channel count > 1 not supported"
        );
    }

    SECTION("hasFixedBinCount == false") {
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

    CHECK(plugin.getVampApiVersion() == 2);

    CHECK_THAT(std::string(plugin.getIdentifier()),  Equals(descriptor.identifier));
    CHECK_THAT(std::string(plugin.getName()),        Equals(descriptor.name));
    CHECK_THAT(std::string(plugin.getDescription()), Equals(descriptor.description));
    CHECK_THAT(std::string(plugin.getMaker()),       Equals(descriptor.maker));
    CHECK_THAT(std::string(plugin.getCopyright()),   Equals(descriptor.copyright));

    CHECK(plugin.getPluginVersion() == descriptor.pluginVersion);
    CHECK(plugin.getInputDomain()   == Plugin::InputDomain::Time);

    SECTION("Parameter") {
        auto parameters = plugin.getParameterDescriptors();
        REQUIRE(parameters.size() == TestPluginDescriptor::parameters.size());

        for (size_t i = 0; i < parameters.size(); ++i) {
            const auto& vampParameter = TestPluginDescriptor::parameters[i];
            auto& parameter           = parameters[i];

            CHECK_THAT(std::string(parameter.identifier),  Equals(vampParameter.identifier));
            CHECK_THAT(std::string(parameter.name),        Equals(vampParameter.name));
            CHECK_THAT(std::string(parameter.description), Equals(vampParameter.description));
            CHECK_THAT(std::string(parameter.unit),        Equals(vampParameter.unit));
            CHECK(parameter.defaultValue == vampParameter.defaultValue);
            CHECK(parameter.minValue     == vampParameter.minValue);
            CHECK(parameter.maxValue     == vampParameter.maxValue);
            CHECK(parameter.quantizeStep.has_value()    == (vampParameter.isQuantized == 1));
            CHECK(parameter.quantizeStep.value_or(0.0f) == vampParameter.quantizeStep);

            // if (vampParameter.valueNames) {
            //     CHECK_FALSE(parameter.valueNames.empty());
            //     for (size_t j = 0; j < parameter.valueNames.size(); ++j) {
            //         CHECK_THAT(
            //             std::string(parameter.valueNames[j]),
            //             Equals(vampParameter.valueNames[j])
            //         );
            //     }
            // }
        }
    };

    SECTION("Programs") {
        auto programs = plugin.getPrograms();
        REQUIRE(programs.size() == descriptor.programCount);

        for (size_t i = 0; i < descriptor.programCount; ++i) {
            CHECK_THAT(std::string(programs[i]), Equals(descriptor.programs[i]));
        }
    }
}

TEST_CASE("PluginHostAdapter handle nullptr in static plugin data") {
    auto descriptor = TestPluginDescriptor::get();
    
    SECTION("Nullptr string") {
        descriptor.identifier = nullptr;
        auto plugin = PluginHostAdapter(descriptor, 48000);
        REQUIRE(plugin.getIdentifier().empty());
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

    REQUIRE_FALSE(plugin.getParameter("invalid").has_value());
    REQUIRE_FALSE(plugin.setParameter("invalid", 0.0f));

    REQUIRE(plugin.getParameter("param1").value() == 1.0f);
    REQUIRE(plugin.setParameter("param1", 2.0f));
    REQUIRE(plugin.getParameter("param1").value() == 2.0f);

    REQUIRE(plugin.getParameter("param2").value() == -1.0f);
    REQUIRE(plugin.setParameter("param2", -5.0f));
    REQUIRE(plugin.getParameter("param2").value() == -5.0f);
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

    REQUIRE_THAT(std::string(plugin.getCurrentProgram().value()), Equals(descriptor.programs[0]));

    REQUIRE_FALSE(plugin.selectProgram("invalid"));
    REQUIRE_THAT(std::string(plugin.getCurrentProgram().value()), Equals(descriptor.programs[0]));

    REQUIRE(plugin.selectProgram(descriptor.programs[1]));
    REQUIRE_THAT(std::string(plugin.getCurrentProgram().value()), Equals(descriptor.programs[1]));
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

        CHECK(output.hasKnownExtents == bool(vampOutput.hasKnownExtents));
        CHECK(output.minValue        == vampOutput.minValue);
        CHECK(output.maxValue        == vampOutput.maxValue);
        CHECK(output.quantizeStep.has_value()    == (vampOutput.isQuantized == 1));
        CHECK(output.quantizeStep.value_or(0.0f) == vampOutput.quantizeStep);
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
    featureUnion[0].v1.valueCount = static_cast<unsigned int>(values.size());
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

    SECTION("Release feature set after process") {
        static bool released = false;
        descriptor.releaseFeatureSet = [](VampFeatureList*) { released = true; };
        plugin.process(Plugin::TimeDomainBuffer{}, 0);
        REQUIRE(released);
    };
}

TEST_CASE("PluginHostAdapter process with wrong input domain") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    plugin.initialise(0, 0);
    REQUIRE_THROWS_WITH(
        plugin.process(Plugin::FrequencyDomainBuffer{}, 0),
        "Wrong input buffer type: Time domain required"
    );
}

#ifdef RTVAMP_VALIDATE

TEST_CASE("PluginHostAdapter validation: process before initialise") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    REQUIRE_THROWS_WITH(
        plugin.process(Plugin::TimeDomainBuffer{}, 0),
        "Plugin must be initialised before process"
    );
}

TEST_CASE("PluginHostAdapter validation: process with wrong buffer size") {
    auto descriptor = TestPluginDescriptor::get();
    auto plugin     = PluginHostAdapter(descriptor, 48000);

    plugin.initialise(32, 32);
    REQUIRE_THROWS_WITH(
        plugin.process(Plugin::TimeDomainBuffer{}, 0),
        "Wrong input buffer size: Buffer size must match initialised block size of 32"
    );
}

#endif
