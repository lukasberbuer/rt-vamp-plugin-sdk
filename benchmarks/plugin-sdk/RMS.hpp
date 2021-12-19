#pragma once

#include <cmath>
#include <numeric>  // accumulate

#include <vamp-sdk/Plugin.h>

#include "rt-vamp-plugin/Plugin.hpp"

using namespace rtvamp;

template <typename T>
struct square {
    T operator()(const T& left, const T& right) const {   
        return left + right * right;
    }
};

class RMS : public rtvamp::Plugin {
public:
    using Plugin::Plugin;

    const char* getIdentifier()    const override { return "rms"; }
    const char* getName()          const override { return "Root mean square"; }
    const char* getDescription()   const override { return ""; };
    const char* getMaker()         const override { return "Lukas Berbuer"; }
    const char* getCopyright()     const override { return "MIT"; }
    int         getPluginVersion() const override { return 1; };

    InputDomain getInputDomain() const { return InputDomain::TimeDomain; }

    OutputList getOutputDescriptors() const override {
        OutputDescriptor d;
        d.identifier  = "rms";
        d.name        = "RMS";
        d.description = "Root mean square of signal";
        d.unit        = "V";
        d.binCount    = 1;
        // use default values for extend and quantization
        return {d};
    }

    bool initialise(unsigned int /* stepSize */, unsigned int /* blockSize */) override {
        initialiseFeatureSet();
        return true;
    }

    void reset() override {}

    const FeatureSet& process(InputBuffer inputBuffer, uint64_t /* nsec */) override {
        auto  signal = std::get<TimeDomainBuffer>(inputBuffer);
        auto& result = getFeatureSet();

        const float sumSquares = std::accumulate(
            signal.begin(), signal.end(), 0.0f, square<float>()
        );
        const float rms = std::sqrt(sumSquares / signal.size());

        result[0][0] = rms;
        return result;
    }
};

class RMSvamp : public Vamp::Plugin {
public:
    explicit RMSvamp(float inputSampleRate) : Plugin(inputSampleRate) {}

    std::string getIdentifier()    const override { return "rms"; }
    std::string getName()          const override { return "Root mean square"; }
    std::string getDescription()   const override { return ""; };
    std::string getMaker()         const override { return "Lukas Berbuer"; };
    std::string getCopyright()     const override { return ""; };
    int         getPluginVersion() const override { return 1; };

    InputDomain getInputDomain() const { return TimeDomain; }

    OutputList getOutputDescriptors() const override {
        OutputDescriptor d;
        d.identifier       = "rms";
        d.name             = "RMS";
        d.description      = "Root mean square of signal";
        d.unit             = "V";
        d.hasFixedBinCount = true;
        d.binCount         = 1;
        d.hasKnownExtents  = false;
        d.isQuantized      = false;
        d.sampleType       = OutputDescriptor::OneSamplePerStep;
        return {d};
    }

    bool initialise(size_t /* channels */, size_t /* stepSize */, size_t blockSize) override {
        blockSize_ = blockSize;
        return true;
    }

    void reset() override {}

    FeatureSet process(const float* const* inputBuffers, Vamp::RealTime /* timestamp */) override {
        const float sumSquares = std::accumulate(
            inputBuffers[0], inputBuffers[0] + blockSize_, 0.0f, square<float>()
        );
        const float rms = std::sqrt(sumSquares / blockSize_);

        Feature feature;
        feature.values = {rms};

        FeatureSet featureSet;
        featureSet[0] = {feature};
        return featureSet;
    }

    FeatureSet getRemainingFeatures() override { return {}; }

private:
    size_t blockSize_ = 0;
};
