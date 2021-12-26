#pragma once

#include "rtvamp/pluginsdk/PluginDefinition.hpp"

using namespace rtvamp::pluginsdk;

class RMS : public PluginDefinition<1> {
public:
    using PluginDefinition::PluginDefinition;  // inherit constructor

    static constexpr Meta meta {
        .identifier    = "rms",
        .name          = "Root mean square",
        .description   = "",
        .maker         = "LB",
        .copyright     = "MIT",
        .pluginVersion = 1,
        .inputDomain   = InputDomain::TimeDomain,
    };

    OutputList getOutputDescriptors() const override {
        return {
            OutputDescriptor{
                .identifier  = "rms",
                .name        = "RMS",
                .description = "Root mean square of signal",
                .unit        = "V",
                .binCount    = 1,
                // use default values for extend and quantization
            }
        };
    }

    bool initialise(unsigned int stepSize, unsigned int blockSize) override;
    void reset() override;

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) override;
};
