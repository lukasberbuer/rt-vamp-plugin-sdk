#include <catch2/catch.hpp>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/PluginAdapter.h"

#include "TestPlugin.h"

using Catch::Matchers::Equals;
using namespace rtvamp;

TEST_CASE("PluginAdapter descriptor") {
    PluginAdapter<TestPlugin> adapter;
    const auto* d = adapter.getDescriptor();

    REQUIRE(d->vampApiVersion == 2);

    REQUIRE_THAT(d->identifier,  Equals("test"));
    REQUIRE_THAT(d->name,        Equals("Test plugin"));
    REQUIRE_THAT(d->description, Equals("Some random test plugin"));
    REQUIRE_THAT(d->maker,       Equals("LB"));
    REQUIRE_THAT(d->copyright,   Equals("MIT"));
    REQUIRE(d->pluginVersion == 1);

    REQUIRE(d->parameterCount == 1);
    const auto* p = d->parameters[0];
    REQUIRE_THAT(p->identifier,  Equals("param"));
    REQUIRE_THAT(p->name,        Equals("Parameter"));
    REQUIRE_THAT(p->description, Equals("Some random parameter"));
    REQUIRE_THAT(p->unit,        Equals(""));
    REQUIRE(p->minValue == 0.0f);
    REQUIRE(p->maxValue == 2.0f);
    REQUIRE(p->defaultValue == 1.0f);
    REQUIRE(p->isQuantized == true);
    REQUIRE_THAT(p->valueNames[0], Equals("a"));
    REQUIRE_THAT(p->valueNames[1], Equals("b"));
    REQUIRE_THAT(p->valueNames[2], Equals("c"));

    REQUIRE(d->programCount == 2);
    REQUIRE_THAT(d->programs[0], Equals("default"));
    REQUIRE_THAT(d->programs[1], Equals("new"));

    REQUIRE(d->inputDomain == vampTimeDomain);

    REQUIRE(d->instantiate != nullptr);
    REQUIRE(d->cleanup != nullptr);
    REQUIRE(d->initialise != nullptr);
    REQUIRE(d->reset != nullptr);
    REQUIRE(d->getParameter != nullptr);
    REQUIRE(d->setParameter != nullptr);
    REQUIRE(d->getCurrentProgram != nullptr);
    REQUIRE(d->selectProgram != nullptr);
    REQUIRE(d->getPreferredStepSize != nullptr);
    REQUIRE(d->getPreferredBlockSize != nullptr);
    REQUIRE(d->getMinChannelCount != nullptr);
    REQUIRE(d->getMaxChannelCount != nullptr);
    REQUIRE(d->getOutputCount != nullptr);
    REQUIRE(d->getOutputDescriptor != nullptr);
    REQUIRE(d->releaseOutputDescriptor != nullptr);
    REQUIRE(d->process != nullptr);
    REQUIRE(d->getRemainingFeatures != nullptr);
    REQUIRE(d->releaseFeatureSet != nullptr);
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
        REQUIRE(d->getPreferredStepSize(h) == 0);
        REQUIRE(d->getPreferredBlockSize(h) == 0);
        REQUIRE(d->getMinChannelCount(h) == 1);
        REQUIRE(d->getMaxChannelCount(h) == 1);
    }

    SECTION("Outputs") {
        REQUIRE(d->getOutputCount(h) == 1);
        REQUIRE(d->getOutputDescriptor(h, 99) == nullptr);  // invalid output index

        VampOutputDescriptor* o = d->getOutputDescriptor(h, 0);
        REQUIRE(o != nullptr);

        REQUIRE_THAT(o->identifier,  Equals("output"));
        REQUIRE_THAT(o->name,        Equals("Output"));
        REQUIRE_THAT(o->description, Equals("Some random output"));
        REQUIRE_THAT(o->unit,        Equals("V"));

        REQUIRE(o->hasFixedBinCount == 1);
        REQUIRE(o->binCount == 3);
        REQUIRE_THAT(o->binNames[0], Equals("a"));
        REQUIRE_THAT(o->binNames[1], Equals("b"));
        REQUIRE_THAT(o->binNames[2], Equals("c"));

        REQUIRE(o->hasKnownExtents == 1);
        REQUIRE(o->minValue == 0.0f);
        REQUIRE(o->maxValue == 10.0f);
        REQUIRE(o->isQuantized == 0);
        REQUIRE(o->quantizeStep == 0.0f);
        REQUIRE(o->sampleType == vampOneSamplePerStep);
        REQUIRE(o->sampleRate == 0);
        REQUIRE(o->hasDuration == 0);

        d->releaseOutputDescriptor(o);
    }

    d->cleanup(h);
}
