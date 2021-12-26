#pragma once

#include <array>
#include <complex>
#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <vector>

/**
 * Vamp C API uses unsigned int as size type (blockSize, stepSize, channelCount, outputCount, ...).
 * Make sure that has at least 32 bit and use uint32_t as type in C++ interfaces.
 */
static_assert(sizeof(unsigned int) >= sizeof(uint32_t), "Size type must have at least 32 bit");

namespace rtvamp {

struct ParameterDescriptor {
    const char* identifier  = "";
    const char* name        = "";
    const char* description = "";
    const char* unit        = "";

    float minValue     = 0.0f;
    float maxValue     = 0.0f;
    float defaultValue = 0.0f;
    bool  isQuantized  = false;
    float quantizeStep = 0.0f;
#if __cpp_lib_constexpr_vector
    std::vector<const char*> valueNames{};
#endif
};

struct OutputDescriptor {
    const char* identifier  = "";
    const char* name        = "";
    const char* description = "";
    const char* unit        = "";

    uint32_t                 binCount = 0;
    std::vector<std::string> binNames{};

    bool  hasKnownExtents = false;
    float minValue        = 0.0f;
    float maxValue        = 0.0f;
    bool  isQuantized     = false;
    float quantizeStep    = 0.0f;
};

enum class InputDomain { TimeDomain, FrequencyDomain };

using TimeDomainBuffer      = std::span<const float>;
using FrequencyDomainBuffer = std::span<const std::complex<float>>;
using InputBuffer           = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;

using Feature = std::vector<float>;

/**
 * Base class to implement feature extraction plugins.
 * 
 * The number of outputs is provided as an template parameter.
 */
template <uint32_t NOutputs>
class PluginDefinition {
public:
    explicit PluginDefinition(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}
    virtual ~PluginDefinition() = default;

    using OutputList = std::array<OutputDescriptor, NOutputs>;
    using FeatureSet = std::array<Feature, NOutputs>;

    static constexpr uint32_t vampApiVersion = 2;
    static constexpr uint32_t outputCount    = NOutputs;

    struct Meta {
        const char* identifier;
        const char* name;
        const char* description;
        const char* maker;
        const char* copyright;
        int         pluginVersion;
        InputDomain inputDomain;
    };

    // required static plugin descriptor
    static constexpr Meta meta{};

    // optional static descriptors, default: 0 parameters, 0 programs
    static constexpr std::array<ParameterDescriptor, 0> parameters{};
    static constexpr std::array<const char*, 0>         programs{};

    // virtual methods which may be implemented in the plugin
    virtual float       getParameter(std::string_view id) const { return 0.0f; }
    virtual void        setParameter(std::string_view id, float value) {} 

    virtual const char* getCurrentProgram() const { return {}; }
    virtual void        selectProgram(std::string_view name) {}

    virtual uint32_t    getPreferredStepSize()  const { return 0; }
    virtual uint32_t    getPreferredBlockSize() const { return 0; }

    // pure virtual methods which must be implemented in the plugin
    virtual OutputList  getOutputDescriptors() const = 0;

    virtual bool initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void reset() = 0;

    virtual const FeatureSet& process(InputBuffer buffer, uint64_t nsec) = 0;

    float getInputSampleRate() const noexcept { return inputSampleRate_; }

protected:
    FeatureSet& getFeatureSet() noexcept { return featureSet_; }
    void initialiseFeatureSet();

private:
    const float inputSampleRate_;
    FeatureSet  featureSet_;
};

/* --------------------------------------- Implementation --------------------------------------- */

template <uint32_t NOutputs>
void PluginDefinition<NOutputs>::initialiseFeatureSet() {
    const auto outputs     = getOutputDescriptors();
    auto&      featureSet  = getFeatureSet();
    for (size_t i = 0; i < outputCount; ++i) {
        featureSet[i].resize(outputs[i].binCount);
    }
}

}  // namespace rtvamp
