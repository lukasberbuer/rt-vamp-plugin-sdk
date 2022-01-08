#pragma once

#include <array>
#include <complex>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

/**
 * Vamp C API uses unsigned int as size type (blockSize, stepSize, channelCount, outputCount, ...).
 * Make sure it has at least 32 bit and use uint32_t as size type in C++ interfaces.
 */
static_assert(sizeof(unsigned int) >= sizeof(uint32_t), "Size type must have at least 32 bit");

namespace rtvamp::pluginsdk {

/**
 * Plugin base class with common type definitions of pluginsdk and hostsdk.
 */
class PluginBase {
public:
    virtual ~PluginBase() = default;

    enum class InputDomain { Time, Frequency };

    struct ParameterDescriptor {
        // use const char* for compile-time evaluation and mapping to C API
        const char*          identifier   = "";
        const char*          name         = "";
        const char*          description  = "";
        const char*          unit         = "";
        float                defaultValue = 0.0f;
        float                minValue     = 0.0f;
        float                maxValue     = 0.0f;
        std::optional<float> quantizeStep = std::nullopt;
        // std::vector<const char*> valueNames{};  // currently not possible -> wait for constexpr vectors
    };

    struct OutputDescriptor {
        std::string              identifier;
        std::string              name;
        std::string              description;
        std::string              unit;
        uint32_t                 binCount = 0;
        std::vector<std::string> binNames{};
        bool                     hasKnownExtents = false;
        float                    minValue        = 0.0f;
        float                    maxValue        = 0.0f;
        std::optional<float>     quantizeStep    = std::nullopt;
    };

    using TimeDomainBuffer      = std::span<const float>;
    using FrequencyDomainBuffer = std::span<const std::complex<float>>;
    using InputBuffer           = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;
    using Feature               = std::vector<float>;
};

/**
 * Base class to implement feature extraction plugins.
 * 
 * The number of outputs is provided as a template parameter.
 */
template <uint32_t NOutputs>
class Plugin : public PluginBase {
public:
    explicit constexpr Plugin(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}

    using OutputList = std::array<OutputDescriptor, NOutputs>;
    using FeatureSet = std::array<Feature, NOutputs>;

    static constexpr uint32_t outputCount = NOutputs;

    // required static plugin descriptor
    struct Meta {
        const char* identifier    = "";
        const char* name          = "";
        const char* description   = "";
        const char* maker         = "";
        const char* copyright     = "";
        int         pluginVersion = 1;
        InputDomain inputDomain   = InputDomain::Time;
    };

    static constexpr Meta meta{};

    // optional static descriptors, default: 0 parameters, 0 programs
    static constexpr std::array<ParameterDescriptor, 0> parameters{};
    static constexpr std::array<const char*, 0>         programs{};

    virtual std::optional<float> getParameter(std::string_view id) const { return {}; }
    virtual bool                 setParameter(std::string_view id, float value) { return false; } 

    virtual std::string_view     getCurrentProgram() const { return {}; }
    virtual bool                 selectProgram(std::string_view name) { return false; }

    virtual uint32_t             getPreferredStepSize()  const { return 0; }
    virtual uint32_t             getPreferredBlockSize() const { return 0; }

    virtual OutputList           getOutputDescriptors() const = 0;

    virtual bool                 initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void                 reset() = 0;
    virtual const FeatureSet&    process(InputBuffer buffer, uint64_t nsec) = 0;

protected:
    float       getInputSampleRate() const noexcept { return inputSampleRate_; };
    FeatureSet& getFeatureSet() noexcept { return featureSet_; }
    void        initialiseFeatureSet();

private:
    const float inputSampleRate_;
    FeatureSet  featureSet_;
};

/* --------------------------------------- Implementation --------------------------------------- */

template <uint32_t NOutputs>
void Plugin<NOutputs>::initialiseFeatureSet() {
    const auto outputs     = getOutputDescriptors();
    auto&      featureSet  = getFeatureSet();
    for (size_t i = 0; i < outputCount; ++i) {
        featureSet[i].resize(outputs[i].binCount);
    }
}

/* ------------------------------------------- Concept ------------------------------------------ */

namespace detail {

/**
 * Check if type is derived from Plugin.
 * Some workaround to make it work with integer template parameter for output count.
 */
template <typename T, size_t MaxOutputCount = 32>
consteval bool isPlugin() {
    return []<std::size_t... Ns>(std::index_sequence<Ns...>) {
        return std::disjunction<
            std::is_base_of<Plugin<Ns>, T>...
        >::value;
    }(std::make_index_sequence<MaxOutputCount>{});
}

}  // namespace detail

template <typename T>
concept IsPlugin = detail::isPlugin<T>();

}  // namespace rtvamp::pluginsdk
