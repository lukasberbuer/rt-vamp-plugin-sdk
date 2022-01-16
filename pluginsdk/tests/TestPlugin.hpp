#pragma once

#include "rtvamp/pluginsdk.hpp"

class TestPlugin : public rtvamp::pluginsdk::Plugin<1> {
public:
    using Plugin::Plugin;  // inherit constructor

    static constexpr Meta meta{
        .identifier    = "test",
        .name          = "Test plugin",
        .description   = "Some random test plugin",
        .maker         = "LB",
        .copyright     = "MIT",
        .pluginVersion = 1,
        .inputDomain   = InputDomain::Time,
    };

    static constexpr std::array parameters{
        ParameterDescriptor{
            .identifier   = "param",
            .name         = "Parameter",
            .description  = "Some random parameter",
            .unit         = "",
            .defaultValue = 1.0f,
            .minValue     = 0.0f,
            .maxValue     = 2.0f,
            .quantizeStep = 1.0f,
        }
    };

    static constexpr std::array programs{"default", "new"};

    OutputList getOutputDescriptors() const override {
        return {
            OutputDescriptor{
                .identifier      = "output",
                .name            = "Output",
                .description     = "Some random output",
                .unit            = "V",
                .binCount        = 3,
                .binNames        = {"a", "b", "c"},
                .hasKnownExtents = true,
                .minValue        = 0.0f,
                .maxValue        = 10.0f,
            },
        };
    }

    std::optional<float> getParameter(std::string_view id) const override {
        if (id == "param") return param_;
        return {};
    }

    bool setParameter(std::string_view id, float value) override {
        if (id == "param") {
            param_ = value;
            return true;
        }
        return false;
    }

    std::string_view getCurrentProgram() const override {
        return programs[programIndex_];
    }

    bool selectProgram(std::string_view program) override {
        for (size_t i = 0; i < programs.size(); ++i) {
            if (programs[i] == program) {
                programIndex_ = i;
                return true;
            }
        }
        return false;
    }

    void reset() override {};

    bool initialise(uint32_t stepSize, uint32_t blockSize) override {
        initialiseFeatureSet();
        return true;
    };

    const FeatureSet& process(InputBuffer buffer, uint64_t nsec) override {
        auto  signal = std::get<TimeDomainBuffer>(buffer);
        auto& result = getFeatureSet();
        if (signal.size() >= 3) {
            result[0][0] = signal[0];
            result[0][1] = signal[1];
            result[0][2] = signal[2];
        }
        return result;
    };

private:
    float  param_        = 1.0f;
    size_t programIndex_ = 0;
};
