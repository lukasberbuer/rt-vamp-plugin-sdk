#pragma once

#include <complex>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <variant>
#include <vector>

namespace rtvamp::hostsdk {

class Plugin {
public:
    explicit Plugin(float inputSampleRate) : inputSampleRate_(inputSampleRate) {}
    virtual ~Plugin() = default;

    /** Input domain of the plugin. */
    enum class InputDomain { Time, Frequency };

    struct ParameterDescriptor {
        std::string_view         identifier;
        std::string_view         name;
        std::string_view         description;
        std::string_view         unit;
        float                    defaultValue;
        float                    minValue;
        float                    maxValue;
        std::optional<float>     quantizeStep;
        std::vector<std::string_view> valueNames;
    };

    struct OutputDescriptor {
        std::string              identifier;
        std::string              name;
        std::string              description;
        std::string              unit;
        uint32_t                 binCount;
        std::vector<std::string> binNames;
        bool                     hasKnownExtents;
        float                    minValue;
        float                    maxValue;
        std::optional<float>     quantizeStep;
    };

    using ParameterList          = std::span<const ParameterDescriptor>;  ///< List of parameter descriptors
    using ProgramList            = std::span<const std::string_view>;  ///< List of programs
    using CurrentProgram         = std::optional<std::string_view>;  ///< Current program (if programs avaiable)
    using OutputList             = std::vector<OutputDescriptor>;  ///< List of output descriptors
    using TimeDomainBuffer       = std::span<const float>;  ///< Time domain buffer
    using FrequencyDomainBuffer  = std::span<const std::complex<float>>;  ///< Frequency domain buffer (FFT)
    using InputBuffer            = std::variant<TimeDomainBuffer, FrequencyDomainBuffer>;  ///< Input buffer variant
    using Feature                = std::vector<float>;  ///< Feature with one or more values (defined by OutputDescriptor::binCount)
    using FeatureSet             = std::span<const Feature>;  ///< Computed features for each output

    virtual std::filesystem::path getLibraryPath() const noexcept = 0;

    virtual uint32_t              getVampApiVersion() const noexcept = 0;

    virtual std::string_view      getIdentifier( )    const noexcept = 0;
    virtual std::string_view      getName()           const noexcept = 0;
    virtual std::string_view      getDescription()    const noexcept = 0;
    virtual std::string_view      getMaker()          const noexcept = 0;
    virtual std::string_view      getCopyright()      const noexcept = 0;
    virtual int                   getPluginVersion()  const noexcept = 0;
    virtual InputDomain           getInputDomain()    const noexcept = 0;

    virtual ParameterList         getParameterDescriptors() const noexcept = 0;
    virtual std::optional<float>  getParameter(std::string_view id) const = 0;
    virtual bool                  setParameter(std::string_view id, float value) = 0;

    virtual ProgramList           getPrograms()       const noexcept = 0;
    virtual CurrentProgram        getCurrentProgram() const = 0;
    virtual bool                  selectProgram(std::string_view name) = 0;

    virtual uint32_t              getPreferredStepSize()  const = 0;
    virtual uint32_t              getPreferredBlockSize() const = 0;

    virtual uint32_t              getOutputCount()       const = 0;
    virtual OutputList            getOutputDescriptors() const = 0;

    virtual bool                  initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void                  reset() = 0;
    virtual FeatureSet            process(InputBuffer buffer, uint64_t nsec) = 0;

    float                         getInputSampleRate() const noexcept { return inputSampleRate_; };

private:
    const float inputSampleRate_;
};

}  // namespace rtvamp::hostsdk
