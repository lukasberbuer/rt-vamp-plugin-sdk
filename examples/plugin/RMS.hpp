#pragma once

#include "PluginBase.hpp"

using namespace rtvamp;

class RMS : public PluginBase {
public:
    using PluginBase::PluginBase;

    constexpr const char* getIdentifier()    const override { return "rms"; }
    constexpr const char* getName()          const override { return "Root mean square"; }
    constexpr const char* getDescription()   const override { return ""; };
    constexpr int         getPluginVersion() const override { return 1; };
    constexpr InputDomain getInputDomain()   const override { return InputDomain::TimeDomain; }

    OutputList getOutputDescriptors() const override;

    bool initialise(unsigned int stepSize, unsigned int blockSize) override;
    void reset() override;

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) override;
};
