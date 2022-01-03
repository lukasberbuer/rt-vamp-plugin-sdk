#pragma once

#include <array>
#include <cmath>
#include <numeric>  // accumulate

#include "vamp/vamp.h"

#include "helper.hpp"

struct DescriptorRMS {
    static consteval VampPluginDescriptor get() {
        VampPluginDescriptor d{};

        d.vampApiVersion = 2;
        d.identifier     = "rms";
        d.name           = "RMS";
        d.description    = "Root mean square";
        d.maker          = "LB";
        d.pluginVersion  = 1;
        d.copyright      = "MIT";
        d.parameterCount = 0;
        d.parameters     = nullptr;
        d.programCount   = 0;
        d.programs       = nullptr;
        d.inputDomain    = vampTimeDomain;

        d.instantiate = [](const VampPluginDescriptor*, float) -> VampPluginHandle {
            static int handle{};
            return &handle;
        };

        d.cleanup = [](VampPluginHandle) {};

        d.initialise = [](
            VampPluginHandle,
            unsigned int channelCount,
            unsigned int stepSize,
            unsigned int blockSize
        ) -> int {
            blockSize_ = blockSize;
            return 1;
        };
    
        d.reset = [](VampPluginHandle) {};

        d.getParameter          = [](VampPluginHandle, int) { return 0.0f; };
        d.setParameter          = [](VampPluginHandle, int, float) {};
        d.getCurrentProgram     = [](VampPluginHandle) { return 0u; };
        d.selectProgram         = [](VampPluginHandle, unsigned int) {};
        d.getPreferredStepSize  = [](VampPluginHandle) { return 0u; };
        d.getPreferredBlockSize = [](VampPluginHandle) { return 0u; };
        d.getMinChannelCount    = [](VampPluginHandle) { return 0u; };
        d.getMaxChannelCount    = [](VampPluginHandle) { return 0u; };

        d.getOutputCount = [](VampPluginHandle) -> unsigned int { return outputs.size(); };

        d.getOutputDescriptor = [](VampPluginHandle, unsigned int) {
            return const_cast<VampOutputDescriptor*>(outputs.data());
        };

        d.releaseOutputDescriptor = [](VampOutputDescriptor*) {};

        d.process = [](
            VampPluginHandle, const float* const* inputBuffers, int, int
        ) -> VampFeatureList* {
            const float sumSquares = std::accumulate(
                inputBuffers[0], inputBuffers[0] + blockSize_, 0.0f, square<float>()
            );
            const float rms = std::sqrt(sumSquares / blockSize_);

            static std::vector<float>              featureValues(1, 0.0f);
            static std::array<VampFeatureUnion, 2> featureUnion = [&] {
                std::array<VampFeatureUnion, 2> result{};
                result[0].v1.hasTimestamp = 0;
                result[0].v1.sec          = 0;
                result[0].v1.nsec         = 0;
                result[0].v1.valueCount   = 1;
                result[0].v1.values       = featureValues.data();
                result[0].v1.label        = nullptr;
                result[1].v2.hasDuration  = 0;
                result[1].v2.durationSec  = 0;
                result[1].v2.durationNsec = 0;
                return result;
            }();
            static VampFeatureList featureList{
                .featureCount = 1,
                .features     = featureUnion.data()
            };
            
            featureValues[0] = rms;
            return &featureList;
        };

        d.getRemainingFeatures = [](VampPluginHandle) -> VampFeatureList* { return nullptr; };
        d.releaseFeatureSet    = [](VampFeatureList*) {};

        return d;
    }

    static constexpr std::array outputs{
        VampOutputDescriptor{
            .identifier       = "rms",
            .name             = "RMS",
            .description      = "Root mean square of signal",
            .unit             = "V",
            .hasFixedBinCount = 1,
            .binCount         = 1,
            .binNames         = nullptr,
            .hasKnownExtents  = 0,
            .minValue         = 0.0f,
            .maxValue         = 0.0f,
            .isQuantized      = 0,
            .quantizeStep     = 0.0f,
            .sampleType       = vampOneSamplePerStep,
            .sampleRate       = 0.0f,
            .hasDuration      = 0,
        }
    };

private:
    inline static unsigned int blockSize_ = 0;
};
