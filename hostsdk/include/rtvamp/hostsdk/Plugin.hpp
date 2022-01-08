#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "rtvamp/pluginsdk/Plugin.hpp"

namespace rtvamp::hostsdk {

using PluginBase = rtvamp::pluginsdk::PluginBase;  // share same base class with pluginsdk

class Plugin : public PluginBase {
public:
    explicit Plugin(float inputSampleRate)
        : inputSampleRate_(inputSampleRate) {}

    using ParameterList = std::span<const ParameterDescriptor>;
    using ProgramList   = std::span<const char* const>;
    using OutputList    = std::vector<OutputDescriptor>;
    using FeatureSet    = std::span<const Feature>;

    virtual std::string_view     getIdentifier( )   const noexcept = 0;
    virtual std::string_view     getName()          const noexcept = 0;
    virtual std::string_view     getDescription()   const noexcept = 0;
    virtual std::string_view     getMaker()         const noexcept = 0;
    virtual std::string_view     getCopyright()     const noexcept = 0;
    virtual int                  getPluginVersion() const noexcept = 0;
    virtual InputDomain          getInputDomain()   const noexcept = 0;

    virtual ParameterList        getParameterList() const noexcept = 0;
    virtual std::optional<float> getParameter(std::string_view id) const = 0;
    virtual void                 setParameter(std::string_view id, float value) = 0;

    virtual ProgramList          getProgramList()    const noexcept = 0;
    virtual std::string_view     getCurrentProgram() const = 0;
    virtual void                 selectProgram(std::string_view name) = 0;

    virtual uint32_t             getPreferredStepSize()  const = 0;
    virtual uint32_t             getPreferredBlockSize() const = 0;

    virtual uint32_t             getOutputCount()       const = 0;
    virtual OutputList           getOutputDescriptors() const = 0;

    virtual bool                 initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void                 reset() = 0;
    virtual FeatureSet           process(InputBuffer buffer, uint64_t nsec) = 0;

    float                        getInputSampleRate() const noexcept { return inputSampleRate_; };

private:
    const float inputSampleRate_;
};

/* ------------------------------------------- Concept ------------------------------------------ */

template <typename T>
concept IsPlugin = std::is_base_of_v<Plugin, T>;

}  // namespace rtvamp::hostsdk
