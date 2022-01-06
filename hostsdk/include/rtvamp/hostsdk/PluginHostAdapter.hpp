#pragma once

#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"

// forward declarations
struct _VampPluginDescriptor;
typedef _VampPluginDescriptor VampPluginDescriptor;
typedef void* VampPluginHandle;

namespace rtvamp::hostsdk {

class PluginHostAdapter : public Plugin {
public:
    PluginHostAdapter(const VampPluginDescriptor& descriptor, float inputSampleRate);
    ~PluginHostAdapter();

    std::string_view getIdentifier()    const override;
    std::string_view getName()          const override;
    std::string_view getDescription()   const override;
    std::string_view getMaker()         const override;
    std::string_view getCopyright()     const override;
    int              getPluginVersion() const override;
    InputDomain      getInputDomain()   const override;

    ParameterList    getParameterList()                const override;
    float            getParameter(std::string_view id) const override;
    void             setParameter(std::string_view id, float value) override; 

    ProgramList      getProgramList()    const override;
    std::string_view getCurrentProgram() const override;
    void             selectProgram(std::string_view name) override;

    uint32_t         getPreferredStepSize()  const override;
    uint32_t         getPreferredBlockSize() const override;

    uint32_t         getOutputCount()       const override;
    OutputList       getOutputDescriptors() const override;

    bool             initialise(uint32_t stepSize, uint32_t blockSize) override;
    void             reset() override;
    FeatureSet       process(InputBuffer buffer, uint64_t nsec) override;

private:
    void checkRequirements();

    const VampPluginDescriptor&      descriptor_;
    VampPluginHandle                 handle_{nullptr};
    std::vector<ParameterDescriptor> parameters_;
    std::vector<const char*>         programs_;
    std::vector<Feature>             featureSet_;
    uint32_t                         outputCount_{0};
    bool                             initialised_{false};
};

}  // namespace rtvamp::hostsdk
