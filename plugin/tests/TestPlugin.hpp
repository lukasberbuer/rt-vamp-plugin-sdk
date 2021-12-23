#pragma once

#include "rt-vamp-plugin/Plugin.hpp"

using namespace rtvamp;

class TestPlugin : public Plugin {
public:
    TestPlugin(float inputSampleRate) : Plugin(inputSampleRate) {}

    constexpr const char* getIdentifier()    const override { return "test"; }
    constexpr const char* getName()          const override { return "Test plugin"; };
    constexpr const char* getDescription()   const override { return "Some random test plugin"; };
    constexpr const char* getMaker()         const override { return "LB"; };
    constexpr const char* getCopyright()     const override { return "MIT"; };
    constexpr int         getPluginVersion() const override { return 1; };
    constexpr InputDomain getInputDomain()   const override { return InputDomain::TimeDomain; };

    ParameterList getParameterDescriptors() const {
        ParameterList result;
        ParameterDescriptor d;
        d.identifier   = "param";
        d.name         = "Parameter";
        d.description  = "Some random parameter";
        d.unit         = "";
        d.minValue     = 0.0f;
        d.maxValue     = 2.0f;
        d.defaultValue = 1.0f;
        d.isQuantized  = true;
        d.valueNames   = {"a", "b", "c"};
        result.push_back(d);
        return result;
    }

    float getParameter(std::string_view id) const {
        return id == "param" ? param_ : 0.0f;
    }

    void setParameter(std::string_view id, float value) {
        if (id == "param") param_ = value;
    } 

    ProgramList getPrograms()       const { return programs_; }
    std::string getCurrentProgram() const { return programs_[programIndex_]; }

    void selectProgram(std::string_view program) {
        for (size_t i = 0; i < programs_.size(); ++i) {
            if (programs_[i] == program) {
                programIndex_ = i;
                return;
            }
        }
    }

    bool initialise(unsigned int, unsigned int) override {
        initialiseFeatureSet();
        return true;
    };

    void reset() override {};

    OutputList getOutputDescriptors() const override {
        OutputList result;
        OutputDescriptor d;
        d.identifier      = "output";
        d.name            = "Output";
        d.description     = "Some random output";
        d.unit            = "V";
        d.binCount        = 3;
        d.binNames        = {"a", "b", "c"};
        d.hasKnownExtents = true;
        d.minValue        = 0.0f;
        d.maxValue        = 10.0f;
        result.push_back(d);
        return result;
    }

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t) override {
        auto  signal = std::get<TimeDomainBuffer>(inputBuffer);
        auto& result = getFeatureSet();
        if (signal.size() >= 3) {
            result[0][0] = signal[0];
            result[0][1] = signal[1];
            result[0][2] = signal[2];
        }
        return result;
    }

private:
    float param_ = 1.0f;
    std::vector<std::string> programs_{"default", "new"};
    size_t programIndex_ = 0;
};
