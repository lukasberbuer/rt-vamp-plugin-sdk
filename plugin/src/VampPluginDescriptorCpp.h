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
struct VampPluginDescriptorCpp : VampPluginDescriptor {
    explicit VampPluginDescriptorCpp(const Plugin& plugin)
        : identifier_(plugin.getIdentifier()),
          name_(plugin.getName()),
          description_(plugin.getDescription()),
          maker_(plugin.getMaker()),
          copyright_(plugin.getCopyright()),
          parameters_(plugin.getParameterDescriptors()),
          programs_(plugin.getPrograms())
    {
        vampApiVersion = plugin.getVampApiVersion();

        identifier    = identifier_.c_str();
        name          = name_.c_str();
        description   = description_.c_str();
        maker         = maker_.c_str();
        copyright     = copyright_.c_str();
        pluginVersion = plugin.getPluginVersion();

        parameterCount = parameters_.size();
        vampParameters_.resize(parameterCount);
        vampParametersPtr_.resize(parameterCount);
        vampParametersValueNames_.resize(parameterCount);
        for (size_t i = 0; i < parameterCount; ++i) {
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
                transformStringToConstChar(p.valueNames, vampParametersValueNames_[i]);
                v.valueNames = vampParametersValueNames_[i].data();
            } else {
                v.valueNames = nullptr;
            }
        }
        std::transform(vampParameters_.begin(), vampParameters_.end(), vampParametersPtr_.begin(),
            [](auto& param) { return &param; }
        );
        parameters = vampParametersPtr_.data();

        transformStringToConstChar(programs_, vampPrograms_);
        programCount = programs_.size();
        programs     = vampPrograms_.data();

        inputDomain = plugin.getInputDomain() == InputDomain::FrequencyDomain
            ? vampFrequencyDomain
            : vampTimeDomain;

        // function pointers have to be assigned externally
        instantiate = nullptr;
        cleanup = nullptr;
        initialise = nullptr;
        reset = nullptr;
        getParameter = nullptr;
        setParameter = nullptr;
        getCurrentProgram = nullptr;
        selectProgram = nullptr;
        getPreferredStepSize = nullptr;
        getPreferredBlockSize = nullptr;
        getMinChannelCount = nullptr;
        getMaxChannelCount = nullptr;
        getOutputCount = nullptr;
        getOutputDescriptor = nullptr;
        releaseOutputDescriptor = nullptr;
        process = nullptr;
        getRemainingFeatures = nullptr;
        releaseFeatureSet = nullptr;
    }

private:
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
