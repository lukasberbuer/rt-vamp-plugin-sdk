#pragma once

#include "rtvamp/pluginsdk.hpp"

class SpectralRolloff : public rtvamp::pluginsdk::PluginExt<SpectralRolloff, 1> {
public:
    using PluginExt::PluginExt;  // inherit constructor

    static constexpr Meta meta {
        .identifier    = "spectralrolloff",
        .name          = "Spectral roll-off",
        .description   = "",
        .maker         = "LB",
        .copyright     = "MIT",
        .pluginVersion = 1,
        .inputDomain   = InputDomain::Frequency,
    };

    static constexpr std::array parameters{
        ParameterDescriptor{
            .identifier   = "rolloff",
            .name         = "Roll-off factor",
            .description  = "Some random parameter",
            .unit         = "",
            .defaultValue = 0.9f,
            .minValue     = 0.0f,
            .maxValue     = 1.0f,
            // default values for quantization (disabled)
        }
    };

    OutputList getOutputDescriptors() const override {
        return {
            OutputDescriptor{
                .identifier  = "frequency",
                .name        = "Roll-off frequency",
                .description = "Frequency below which n% of the total energy is concentrated",
                .unit        = "Hz",
                .binCount    = 1,
                // use default values for extend and quantization
            }
        };
    }

    bool initialise(uint32_t stepSize, uint32_t blockSize) override;
    void reset() override;

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) override;

private:
    std::vector<float> magnitude_;
};
