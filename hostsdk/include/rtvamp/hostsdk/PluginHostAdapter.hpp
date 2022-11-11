#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "rtvamp/hostsdk/Plugin.hpp"

// forward declarations
struct _VampPluginDescriptor;
typedef _VampPluginDescriptor VampPluginDescriptor;
typedef void* VampPluginHandle;

namespace rtvamp::hostsdk {

class DynamicLibrary;

class PluginHostAdapter : public Plugin {
public:
    PluginHostAdapter(
        const VampPluginDescriptor&     descriptor,
        float                           inputSampleRate,
        std::shared_ptr<DynamicLibrary> library = nullptr  // extend lifetime of dl handle
    );
    ~PluginHostAdapter();

    std::filesystem::path getLibraryPath() const noexcept;

    uint32_t              getVampApiVersion() const noexcept override;

    std::string_view      getIdentifier()     const noexcept override;
    std::string_view      getName()           const noexcept override;
    std::string_view      getDescription()    const noexcept override;
    std::string_view      getMaker()          const noexcept override;
    std::string_view      getCopyright()      const noexcept override;
    int                   getPluginVersion()  const noexcept override;
    InputDomain           getInputDomain()    const noexcept override;

    ParameterList         getParameterDescriptors() const noexcept override;
    std::optional<float>  getParameter(std::string_view id) const override;
    bool                  setParameter(std::string_view id, float value) override; 

    ProgramList           getPrograms()       const noexcept override;
    CurrentProgram        getCurrentProgram() const override;
    bool                  selectProgram(std::string_view name) override;

    uint32_t              getPreferredStepSize()  const override;
    uint32_t              getPreferredBlockSize() const override;

    uint32_t              getOutputCount()       const override;
    OutputList            getOutputDescriptors() const override;

    bool                  initialise(uint32_t stepSize, uint32_t blockSize) override;
    void                  reset() override;
    FeatureSet            process(InputBuffer buffer, uint64_t nsec) override;

private:
    void checkRequirements();

    const VampPluginDescriptor&      descriptor_;
    std::shared_ptr<DynamicLibrary>  library_;
    VampPluginHandle                 handle_{nullptr};
    std::vector<ParameterDescriptor> parameters_;
    std::vector<std::string_view>    programs_;
    std::vector<Feature>             featureSet_;
    uint32_t                         outputCount_{0};
    bool                             initialised_{false};
    uint32_t                         initialisedBlockSize_{0};
};

}  // namespace rtvamp::hostsdk
