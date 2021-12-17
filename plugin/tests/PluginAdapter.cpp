#include <catch2/catch.hpp>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/PluginAdapter.h"

#include "TestPlugin.h"

using Catch::Matchers::Equals;
using namespace rtvamp;

TEST_CASE("PluginAdapter descriptor") {
    PluginAdapter<TestPlugin> adapter;
    const auto* d = adapter.getDescriptor();

    CHECK(d->vampApiVersion == 2);

    CHECK_THAT(d->identifier,  Equals("test"));
    CHECK_THAT(d->name,        Equals("Test plugin"));
    CHECK_THAT(d->description, Equals("Some random test plugin"));
    CHECK_THAT(d->maker,       Equals("LB"));
    CHECK_THAT(d->copyright,   Equals("MIT"));
    CHECK(d->pluginVersion == 1);

    CHECK(d->parameterCount == 1);

    const auto* p = d->parameters[0];
    CHECK_THAT(p->identifier,  Equals("param"));
    CHECK_THAT(p->name,        Equals("Parameter"));
    CHECK_THAT(p->description, Equals("Some random parameter"));
    CHECK_THAT(p->unit,        Equals(""));
    CHECK(p->minValue == 0.0f);
    CHECK(p->maxValue == 2.0f);
    CHECK(p->defaultValue == 1.0f);
    CHECK(p->isQuantized == true);
    CHECK_THAT(p->valueNames[0], Equals("a"));
    CHECK_THAT(p->valueNames[1], Equals("b"));
    CHECK_THAT(p->valueNames[2], Equals("c"));

    CHECK(d->programCount == 2);
    CHECK_THAT(d->programs[0], Equals("default"));
    CHECK_THAT(d->programs[1], Equals("new"));

    CHECK(d->inputDomain == vampTimeDomain);

    CHECK(d->instantiate != nullptr);
    CHECK(d->cleanup != nullptr);
    CHECK(d->initialise != nullptr);
    CHECK(d->reset != nullptr);
    CHECK(d->getParameter != nullptr);
    CHECK(d->setParameter != nullptr);
    CHECK(d->getCurrentProgram != nullptr);
    CHECK(d->selectProgram != nullptr);
    CHECK(d->getPreferredStepSize != nullptr);
    CHECK(d->getPreferredBlockSize != nullptr);
    CHECK(d->getMinChannelCount != nullptr);
    CHECK(d->getMaxChannelCount != nullptr);
    CHECK(d->getOutputCount != nullptr);
    CHECK(d->getOutputDescriptor != nullptr);
    CHECK(d->releaseOutputDescriptor != nullptr);
    CHECK(d->process != nullptr);
    CHECK(d->getRemainingFeatures != nullptr);
    CHECK(d->releaseFeatureSet != nullptr);
}

TEST_CASE("PluginAdapter instantiation") {
    PluginAdapter<TestPlugin> adapter;
    const VampPluginDescriptor* d = adapter.getDescriptor();

    VampPluginHandle h = d->instantiate(d, 48000);
    REQUIRE(h != nullptr);

    d->reset(h);

    SECTION("Parameter") {
        REQUIRE(d->getParameter(h, 0) == 1.0f);
        REQUIRE(d->getParameter(h, 1) == 0.0f);  // does not exist
        d->setParameter(h, 0, 2.0f);
        REQUIRE(d->getParameter(h, 0) == 2.0f);
    }
    
    SECTION("Program") {
        REQUIRE(d->getCurrentProgram(h) == 0);
        d->selectProgram(h, 1);
        REQUIRE(d->getCurrentProgram(h) == 1);
    }

    SECTION("Preferences / channel count") {
        CHECK(d->getPreferredStepSize(h) == 0);
        CHECK(d->getPreferredBlockSize(h) == 0);
        CHECK(d->getMinChannelCount(h) == 1);
        CHECK(d->getMaxChannelCount(h) == 1);
    }

    SECTION("Outputs") {
        REQUIRE(d->getOutputCount(h) == 1);
        REQUIRE(d->getOutputDescriptor(h, 99) == nullptr);  // invalid output index

        VampOutputDescriptor* o = d->getOutputDescriptor(h, 0);
        REQUIRE(o != nullptr);

        CHECK_THAT(o->identifier,  Equals("output"));
        CHECK_THAT(o->name,        Equals("Output"));
        CHECK_THAT(o->description, Equals("Some random output"));
        CHECK_THAT(o->unit,        Equals("V"));

        CHECK(o->hasFixedBinCount == 1);
        CHECK(o->binCount == 3);
        CHECK_THAT(o->binNames[0], Equals("a"));
        CHECK_THAT(o->binNames[1], Equals("b"));
        CHECK_THAT(o->binNames[2], Equals("c"));

        CHECK(o->hasKnownExtents == 1);
        CHECK(o->minValue == 0.0f);
        CHECK(o->maxValue == 10.0f);
        CHECK(o->isQuantized == 0);
        CHECK(o->quantizeStep == 0.0f);
        CHECK(o->sampleType == vampOneSamplePerStep);
        CHECK(o->sampleRate == 0);
        CHECK(o->hasDuration == 0);

        d->releaseOutputDescriptor(o);
    }

    SECTION("Shared resource of VampOutputDescriptor -> problem?") {
        VampOutputDescriptor* o1 = d->getOutputDescriptor(h, 0);
        VampOutputDescriptor *o2 = d->getOutputDescriptor(h, 0);

        REQUIRE(o1 == o2);
        o1->binCount = 99;
        REQUIRE(o2->binCount == 99);

        d->releaseOutputDescriptor(o1);
        d->releaseOutputDescriptor(o2);
    }

    SECTION("Initialise,  process and getRemainingFeatures") {
        const unsigned int              inputChannels = 1;
        const unsigned int              blockSize = 5;
        const unsigned int              stepSize = 5;
        const std::vector<float>        signal{1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
        const std::vector<const float*> inputBuffer{signal.data()};

        d->initialise(h, inputChannels, stepSize, blockSize);

        auto* result = d->process(h, inputBuffer.data(), 0, 0);

        CHECK(result != nullptr);
        CHECK(result[0].featureCount == 1);
        CHECK(result[0].features[0].v1.valueCount == 3);
        CHECK(result[0].features[0].v1.values[0] == 1.1f);
        CHECK(result[0].features[0].v1.values[1] == 2.2f);
        CHECK(result[0].features[0].v1.values[2] == 3.3f);

        d->releaseFeatureSet(result); 

        auto* remaining = d->getRemainingFeatures(h);

        REQUIRE(remaining != nullptr);
        REQUIRE(remaining[0].featureCount == 0);
        REQUIRE(remaining[0].features == nullptr);

        d->releaseFeatureSet(remaining); 
    }

    d->cleanup(h);
}
