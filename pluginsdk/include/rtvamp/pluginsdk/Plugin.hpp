#pragma once

#include <array>
#include <complex>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Vamp C API uses unsigned int as size type (blockSize, stepSize, channelCount, outputCount, ...).
// Make sure it has at least 32 bit and use uint32_t as size type in C++ interfaces.
static_assert(sizeof(unsigned int) >= sizeof(uint32_t), "Size type must have at least 32 bit");

namespace rtvamp::pluginsdk {

/**
 * Non-templated plugin base class with type definitions.
 */
class PluginBase {
public:
    PluginBase() = default;
    virtual ~PluginBase() = default;

    PluginBase(const PluginBase&) = default;
    PluginBase(PluginBase&&) = default;
    PluginBase& operator=(const PluginBase&) = default;
    PluginBase& operator=(PluginBase&&) = default;

    /** Input domain of the plugin. */
    enum class InputDomain { Time, Frequency };

    struct ParameterDescriptor {
        // use const char* for compile-time evaluation and mapping to C API
        const char*          identifier   = "";
        const char*          name         = "";
        const char*          description  = "";
        const char*          unit         = "";
        float                defaultValue = 0.0F;
        float                minValue     = 0.0F;
        float                maxValue     = 0.0F;
        std::optional<float> quantizeStep = std::nullopt;
        // std::vector<const char*> valueNames{};  // currently not possible -> wait for constexpr vectors
    };

    struct OutputDescriptor {
        std::string               identifier;
        std::string               name;
        std::string               description;
        std::string               unit;
        uint32_t                  binCount = 1;
        std::vector<std::string>  binNames = {};  // NOLINT(*redundant-member-init)
        bool                      hasKnownExtents = false;
        float                     minValue        = 0.0F;
        float                     maxValue        = 0.0F;
        std::optional<float>      quantizeStep    = std::nullopt;
    };

    using TimeDomainBuffer      = std::span<const float>;  ///< Time domain buffer
    using FrequencyDomainBuffer = std::span<const std::complex<float>>;  ///< Frequency domain buffer (FFT)
    using InputBuffer           = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;  ///< Input domain variant
    using Feature               = std::vector<float>;  ///< Feature with one or more values (defined by OutputDescriptor::binCount)
};

/**
 * Base class to implement feature extraction plugins.
 *
 * The number of outputs is provided as a template parameter. This allows fixed-sized arrays for
 * both the process results (#FeatureSet) and output descriptors (#OutputList) and therefore enforce
 * the requirement of matching output size at compile time.
 *
 * Static plugin descriptors are provided as static constexpr variables to generate the C API
 * descriptor at compile time. Dynamic plugin descriptors and methods are defined as (pure) virtual
 * functions.
 */
template <uint32_t NOutputs>
class Plugin : public PluginBase {
public:
    explicit constexpr Plugin(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}

    using OutputList = std::array<OutputDescriptor, NOutputs>;  ///< List of output descriptors
    using FeatureSet = std::array<Feature, NOutputs>;           ///< Computed features for each output

    static constexpr uint32_t outputCount = NOutputs;  ///< Number of outputs (defined by template parameter)

    /** Static plugin descriptor */
    struct Meta {
        const char*  identifier    = "";
        const char*  name          = "";
        const char*  description   = "";
        const char*  maker         = "";
        const char*  copyright     = "";
        int          pluginVersion = 1;
        InputDomain  inputDomain   = InputDomain::Time;
    };

    static constexpr Meta                               meta{};        ///< Required static plugin descriptor
    static constexpr std::array<ParameterDescriptor, 0> parameters{};  ///< Optional parameter descriptors (default: none)
    static constexpr std::array<const char*, 0>         programs{};    ///< Optional program list (default: none)

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

    void initialiseFeatureSet() {
        const auto outputs     = getOutputDescriptors();
        auto&      featureSet  = getFeatureSet();
        for (size_t i = 0; i < outputCount; ++i) {
            featureSet[i].resize(outputs[i].binCount);
        }
    }

private:
    float      inputSampleRate_;
    FeatureSet featureSet_;
};

/* ------------------------------------------- Concept ------------------------------------------ */

template <typename T>
concept HasParameters = requires {
    { T::parameters } -> std::convertible_to<std::array<PluginBase::ParameterDescriptor, T::parameters.size()>>;
};

template <typename T>
concept HasPrograms = requires {
    { T::programs } -> std::convertible_to<std::array<const char*, T::programs.size()>>;
};

template <typename T>
concept IsPlugin = std::constructible_from<T, float> && requires(
    T plugin,
    std::string_view parameterName,
    float parameterValue,
    std::string_view programName,
    uint32_t stepSize,
    uint32_t blockSize,
    PluginBase::InputBuffer buffer,
    uint64_t nsec
) {
    { T::outputCount } -> std::convertible_to<uint32_t>;
    { T::meta } -> std::convertible_to<typename T::Meta>;

    requires (!HasParameters<T> || requires {
        { plugin.getParameter(parameterName) } -> std::same_as<std::optional<float>>;
        { plugin.setParameter(parameterName, parameterValue) } -> std::same_as<bool>;
    });

    requires (!HasPrograms<T> || requires {
        { plugin.getCurrentProgram() } -> std::same_as<std::string_view>;
        { plugin.selectProgram(programName) } -> std::same_as<bool>;
    });

    { plugin.getPreferredStepSize() } -> std::same_as<uint32_t>;
    { plugin.getPreferredBlockSize() } -> std::same_as<uint32_t>;
    { plugin.getOutputDescriptors() } -> std::same_as<std::array<PluginBase::OutputDescriptor, T::outputCount>>;
    { plugin.initialise(stepSize, blockSize) } -> std::same_as<bool>;
    { plugin.reset() } -> std::same_as<void>;
    { plugin.process(buffer, nsec) } -> std::convertible_to<const std::array<PluginBase::Feature, T::outputCount>&>;
};

}  // namespace rtvamp::pluginsdk
