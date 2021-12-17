#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/Plugin.h"

#include "helper.h"

namespace rtvamp {

/**
 * Extended VampPluginDescriptor with RAII containers for easy cleanup.
 */
class VampPluginDescriptorWrapper : VampPluginDescriptor {
public:
    explicit VampPluginDescriptorWrapper(const Plugin& plugin)
        : identifier_(plugin.getIdentifier()),
          name_(plugin.getName()),
          description_(plugin.getDescription()),
          maker_(plugin.getMaker()),
          copyright_(plugin.getCopyright()),
          parameters_(plugin.getParameterDescriptors()),
          programs_(plugin.getPrograms())
    {
        auto& d = descriptor_;

        d.vampApiVersion = plugin.getVampApiVersion();
        d.identifier     = identifier_.c_str();
        d.name           = name_.c_str();
        d.description    = description_.c_str();
        d.maker          = maker_.c_str();
        d.copyright      = copyright_.c_str();
        d.pluginVersion  = plugin.getPluginVersion();
        d.parameterCount = parameters_.size();
        d.parameters     = getParameters();
        d.programCount   = programs_.size();
        d.programs       = getPrograms();
        d.inputDomain    = plugin.getInputDomain() == InputDomain::FrequencyDomain
            ? vampFrequencyDomain
            : vampTimeDomain;

        // function pointers have to be assigned externally
        d.instantiate             = nullptr;
        d.cleanup                 = nullptr;
        d.initialise              = nullptr;
        d.reset                   = nullptr;
        d.getParameter            = nullptr;
        d.setParameter            = nullptr;
        d.getCurrentProgram       = nullptr;
        d.selectProgram           = nullptr;
        d.getPreferredStepSize    = nullptr;
        d.getPreferredBlockSize   = nullptr;
        d.getMinChannelCount      = nullptr;
        d.getMaxChannelCount      = nullptr;
        d.getOutputCount          = nullptr;
        d.getOutputDescriptor     = nullptr;
        d.releaseOutputDescriptor = nullptr;
        d.process                 = nullptr;
        d.getRemainingFeatures    = nullptr;
        d.releaseFeatureSet       = nullptr;
    }

    const VampPluginDescriptor& get() const noexcept { return descriptor_; }
    VampPluginDescriptor&       get()       noexcept { return descriptor_; }

private:
    const VampParameterDescriptor** getParameters() {
        const size_t n = parameters_.size();

        vampParameters_.resize(n);
        vampParametersPtr_.resize(n);
        vampParametersValueNames_.resize(n);

        for (size_t i = 0; i < n; ++i) {
            auto& p = parameters_[i];
            auto& v = vampParameters_[i];

            v.identifier   = p.identifier.c_str();
            v.name         = p.name.c_str();
            v.description  = p.description.c_str();
            v.unit         = p.unit.c_str();
            v.minValue     = p.minValue;
            v.maxValue     = p.maxValue;
            v.defaultValue = p.defaultValue;
            v.isQuantized  = p.isQuantized;
            v.quantizeStep = p.quantizeStep;

            if (p.isQuantized && !p.valueNames.empty()) {
                transform::all(p.valueNames, vampParametersValueNames_[i], transform::ToConstChar{});
                v.valueNames = vampParametersValueNames_[i].data();
            } else {
                v.valueNames = nullptr;
            }
        }

        transform::all(vampParameters_, vampParametersPtr_, transform::ToPtr{});
        return vampParametersPtr_.data();
    }

    const char** getPrograms() {
        transform::all(programs_, vampPrograms_, transform::ToConstChar{});
        return vampPrograms_.data();
    }

    VampPluginDescriptor descriptor_;

    const std::string identifier_;
    const std::string name_;
    const std::string description_;
    const std::string maker_;
    const std::string copyright_;

    const std::vector<ParameterDescriptor>      parameters_;
    std::vector<VampParameterDescriptor>        vampParameters_;
    std::vector<const VampParameterDescriptor*> vampParametersPtr_;
    std::vector<std::vector<const char*>>       vampParametersValueNames_;

    const std::vector<std::string> programs_;
    std::vector<const char*>       vampPrograms_;
};

}  // namespace rtvamp
