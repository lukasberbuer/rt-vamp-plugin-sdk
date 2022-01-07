#pragma once

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

    virtual std::string_view getIdentifier( )   const = 0;
    virtual std::string_view getName()          const = 0;
    virtual std::string_view getDescription()   const = 0;
    virtual std::string_view getMaker()         const = 0;
    virtual std::string_view getCopyright()     const = 0;
    virtual int              getPluginVersion() const = 0;
    virtual InputDomain      getInputDomain()   const = 0;

    virtual ParameterList    getParameterList()                const { return {}; }
    virtual float            getParameter(std::string_view id) const { return 0.0f; }
    virtual void             setParameter(std::string_view id, float value) {} 

    virtual ProgramList      getProgramList()    const { return {}; }
    virtual std::string_view getCurrentProgram() const { return {}; }
    virtual void             selectProgram(std::string_view name) {}

    virtual uint32_t         getPreferredStepSize()  const { return 0; }
    virtual uint32_t         getPreferredBlockSize() const { return 0; }

    virtual uint32_t         getOutputCount()       const = 0;
    virtual OutputList       getOutputDescriptors() const = 0;

    virtual bool             initialise(uint32_t stepSize, uint32_t blockSize) = 0;
    virtual void             reset() = 0;
    virtual FeatureSet       process(InputBuffer buffer, uint64_t nsec) = 0;

    float                    getInputSampleRate() const noexcept { return inputSampleRate_; };

private:
    const float inputSampleRate_;
};

/* ------------------------------------------- Concept ------------------------------------------ */

template <typename T>
concept IsPlugin = std::is_base_of_v<Plugin, T>;

}  // namespace rtvamp::hostsdk
