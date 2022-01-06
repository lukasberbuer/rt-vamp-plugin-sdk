#pragma once

#include <complex>
#include <span>
#include <string>
#include <variant>
#include <vector>

/**
 * Vamp C API uses unsigned int as size type (blockSize, stepSize, channelCount, outputCount, ...).
 * Make sure it has at least 32 bit and use uint32_t as size type in C++ interfaces.
 */
static_assert(sizeof(unsigned int) >= sizeof(uint32_t), "Size type must have at least 32 bit");

namespace rtvamp::pluginsdk {

/**
 * Base class for feature extraction plugins.
 * 
 * Implementations must derive from the PluginDefinition class.
 */
class Plugin {
public:
    explicit constexpr Plugin(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}
    virtual ~Plugin() = default;

    enum class InputDomain { Time, Frequency };

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
        // std::vector<const char*> valueNames{};  // currently not possible -> wait for constexpr vectors
    };

    struct OutputDescriptor {
        std::string identifier;
        std::string name;
        std::string description;
        std::string unit;

        uint32_t                 binCount = 0;
        std::vector<std::string> binNames{};

        bool  hasKnownExtents = false;
        float minValue        = 0.0f;
        float maxValue        = 0.0f;
        bool  isQuantized     = false;
        float quantizeStep    = 0.0f;
    };

    using ParameterList           = std::span<const ParameterDescriptor>;
    using ProgramList             = std::span<const char* const>;
    using OutputList              = std::vector<OutputDescriptor>;

    using TimeDomainBuffer        = std::span<const float>;
    using FrequencyDomainBuffer   = std::span<const std::complex<float>>;
    using InputBuffer             = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;

    using Feature                 = std::vector<float>;
    using FeatureSet              = std::span<const Feature>;

    virtual constexpr const char*   getIdentifier( )   const = 0;
    virtual constexpr const char*   getName()          const = 0;
    virtual constexpr const char*   getDescription()   const = 0;
    virtual constexpr const char*   getMaker()         const = 0;
    virtual constexpr const char*   getCopyright()     const = 0;
    virtual constexpr int           getPluginVersion() const = 0;
    virtual constexpr InputDomain   getInputDomain()   const = 0;

    virtual constexpr ParameterList getParameterList()                const { return {}; }
    virtual float                   getParameter(std::string_view id) const { return 0.0f; }
    virtual void                    setParameter(std::string_view id, float value) {} 

    virtual constexpr ProgramList   getProgramList()    const { return {}; }
    virtual const char*             getCurrentProgram() const { return {}; }
    virtual void                    selectProgram(std::string_view name) {}

    virtual float                   getInputSampleRate()    const { return inputSampleRate_; };
    virtual uint32_t                getPreferredStepSize()  const { return 0; }
    virtual uint32_t                getPreferredBlockSize() const { return 0; }

    virtual constexpr uint32_t      getOutputCount()       const = 0;
    virtual OutputList              getOutputDescriptors() const = 0;

    virtual bool                    initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void                    reset() = 0;
    virtual FeatureSet              process(InputBuffer buffer, uint64_t nsec) = 0;

private:
    const float inputSampleRate_ = 0.0f;
};

/* ------------------------------------------- Concept ------------------------------------------ */

template <typename T>
concept IsPlugin = std::is_base_of_v<Plugin, T>;

}  // namespace rtvamp::pluginsdk
