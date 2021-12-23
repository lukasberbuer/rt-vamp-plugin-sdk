#pragma once

#include <complex>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace rtvamp {

struct ParameterDescriptor {
    std::string identifier;
    std::string name;
    std::string description;
    std::string unit;

    float minValue     = 0.0f;
    float maxValue     = 0.0f;
    float defaultValue = 0.0f;
    bool  isQuantized  = false;
    float quantizeStep = 0.0f;

    std::vector<std::string> valueNames;
};

using ParameterList = std::vector<ParameterDescriptor>;
using ProgramList   = std::vector<std::string>;

struct OutputDescriptor {
    std::string identifier;
    std::string name;
    std::string description;
    std::string unit;

    unsigned int             binCount = 0;
    std::vector<std::string> binNames;

    bool  hasKnownExtents = false;
    float minValue        = 0.0f;
    float maxValue        = 0.0f;

    bool  isQuantized  = false;
    float quantizeStep = 0.0f;
};

using Feature    = std::vector<float>;
using FeatureSet = std::vector<Feature>;
using OutputList = std::vector<OutputDescriptor>;

using TimeDomainBuffer      = std::span<const float>;
using FrequencyDomainBuffer = std::span<const std::complex<float>>;
using InputBuffer           = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;

enum class InputDomain {
    TimeDomain,
    FrequencyDomain
};

class Plugin  {
public:
    explicit Plugin(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}
    virtual ~Plugin() {}

    constexpr unsigned int getVampApiVersion() const { return 2; }

    virtual constexpr const char* getIdentifier() const = 0;
    virtual constexpr const char* getName() const = 0;
    virtual constexpr const char* getDescription() const = 0;
    virtual constexpr const char* getMaker() const = 0;
    virtual constexpr const char* getCopyright() const = 0;
    virtual constexpr int         getPluginVersion() const = 0;

    virtual constexpr InputDomain  getInputDomain() const = 0;
    virtual constexpr unsigned int getPreferredBlockSize() const { return 0; }
    virtual constexpr unsigned int getPreferredStepSize()  const { return 0; }

    virtual ParameterList getParameterDescriptors() const { return {}; }
    virtual float         getParameter(std::string_view) const { return 0.0f; }
    virtual void          setParameter(std::string_view, float) {} 

    virtual ProgramList getPrograms() const { return {}; }
    virtual std::string getCurrentProgram() const { return {}; }
    virtual void        selectProgram(std::string_view) {}

    virtual OutputList getOutputDescriptors() const = 0;

    virtual bool initialise(unsigned int stepSize, unsigned int blockSize) = 0;
    virtual void reset() = 0;

    virtual const FeatureSet& process(InputBuffer inputBuffer, uint64_t nsec) = 0;
    const         FeatureSet& getResult() const { return featureSet_; }

    float getInputSampleRate() const { return inputSampleRate_; }

protected:
    FeatureSet& getFeatureSet() { return featureSet_; }

    void initialiseFeatureSet();

private:
    float      inputSampleRate_;
    FeatureSet featureSet_;
};

}  // namespace rtvamp
