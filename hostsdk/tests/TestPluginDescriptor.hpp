#pragma once

#include <array>

#include "vamp/vamp.h"

struct TestPluginDescriptor {
    static VampPluginDescriptor get() {
        VampPluginDescriptor d{};

        d.vampApiVersion = 2;
        d.identifier     = "test";
        d.name           = "Test plugin";
        d.description    = "Some random test plugin";
        d.maker          = "LB";
        d.pluginVersion  = 1;
        d.copyright      = "MIT";
        d.parameterCount = static_cast<unsigned int>(parameters.size());
        d.parameters     = const_cast<const VampParameterDescriptor**>(parametersPtr.data());
        d.programCount   = static_cast<unsigned int>(programs.size());
        d.programs       = const_cast<const char**>(programs.data());
        d.inputDomain    = vampTimeDomain;

        d.instantiate = [](const VampPluginDescriptor*, float) -> VampPluginHandle {
            static int handle{};
            return &handle;
        };

        d.cleanup = [](VampPluginHandle) {};

        d.initialise = [](VampPluginHandle, unsigned int, unsigned int, unsigned int) { return 1; };
        d.reset      = [](VampPluginHandle) {};

        d.getParameter          = [](VampPluginHandle, int) { return 0.0f; };
        d.setParameter          = [](VampPluginHandle, int, float) {};
        d.getCurrentProgram     = [](VampPluginHandle) { return 0u; };
        d.selectProgram         = [](VampPluginHandle, unsigned int) {};
        d.getPreferredStepSize  = [](VampPluginHandle) { return 0u; };
        d.getPreferredBlockSize = [](VampPluginHandle) { return 0u; };
        d.getMinChannelCount    = [](VampPluginHandle) { return 0u; };
        d.getMaxChannelCount    = [](VampPluginHandle) { return 0u; };

        d.getOutputCount = [](VampPluginHandle) { return static_cast<unsigned int>(outputs.size()); };

        d.getOutputDescriptor = [](VampPluginHandle, unsigned int) {
            return const_cast<VampOutputDescriptor*>(outputs.data());
        };

        d.releaseOutputDescriptor = [](VampOutputDescriptor*) {};

        d.process = [](VampPluginHandle, const float* const*, int, int) -> VampFeatureList* {
            return nullptr;
        };

        d.getRemainingFeatures = [](VampPluginHandle) -> VampFeatureList* { return nullptr; };
        d.releaseFeatureSet    = [](VampFeatureList*) {};

        return d;
    }

    static constexpr std::array parameterValueNames{"a", "b", "c"};
    static constexpr std::array parameters{
        VampParameterDescriptor{
            .identifier   = "param1",
            .name         = "Parameter1",
            .description  = "Some random parameter",
            .unit         = "V",
            .minValue     = 0.0f,
            .maxValue     = 2.0f,
            .defaultValue = 1.0f,
            .isQuantized  = 1,
            .quantizeStep = 1.0f,
            .valueNames   = const_cast<const char**>(parameterValueNames.data()),
        },
        VampParameterDescriptor{
            .identifier   = "param2",
            .name         = "Parameter2",
            .description  = "Some random parameter",
            .unit         = "V",
            .minValue     = -10.0f,
            .maxValue     = 10.0f,
            .defaultValue = -1.0f,
            .isQuantized  = 0,
            .quantizeStep = 0.0f,
            .valueNames   = nullptr,
        }
    };
    static constexpr std::array parametersPtr{&parameters[0], &parameters[1]};
    static constexpr std::array programs{"default", "new"};
    static constexpr std::array outputBinNames{"a", "b", "c"};
    static constexpr std::array outputs{
        VampOutputDescriptor{
            .identifier       = "output",
            .name             = "Output",
            .description      = "Some random output description",
            .unit             = "V",
            .hasFixedBinCount = 1,
            .binCount         = 3,
            .binNames         = const_cast<const char**>(outputBinNames.data()),
            .hasKnownExtents  = 1,
            .minValue         = -10.0f,
            .maxValue         = 10.0f,
            .isQuantized      = 1,
            .quantizeStep     = 1.0f,
            .sampleType       = vampOneSamplePerStep,
            .sampleRate       = 0.0f,
            .hasDuration      = 0,
        }
    };
};
