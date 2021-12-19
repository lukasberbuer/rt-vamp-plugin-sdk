#pragma once

#include "PluginBase.hpp"

using namespace rtvamp;

class RMS : public PluginBase {
public:
    using PluginBase::PluginBase;

    const char* getIdentifier()    const override { return "rms"; }
    const char* getName()          const override { return "Root mean square"; }
    const char* getDescription()   const override { return ""; };
    int         getPluginVersion() const override { return 1; };

    InputDomain getInputDomain() const { return InputDomain::TimeDomain; }

    OutputList getOutputDescriptors() const override;

    bool initialise(unsigned int stepSize, unsigned int blockSize) override;
    void reset() override;

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) override;
};
