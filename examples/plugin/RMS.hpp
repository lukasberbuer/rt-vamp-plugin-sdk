#pragma once

#include "rtvamp/pluginsdk.hpp"

class RMS : public rtvamp::pluginsdk::Plugin<1> {
public:
    using Plugin::Plugin;  // inherit constructor

    static constexpr Meta meta {
        .identifier    = "rms",
        .name          = "RMS",
        .description   = "Root mean square",
        .maker         = "LB",
        .copyright     = "MIT",
        .pluginVersion = 1,
        .inputDomain   = InputDomain::Time,
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

    bool initialise(uint32_t stepSize, uint32_t blockSize) override;
    void reset() override;

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) override;
};
