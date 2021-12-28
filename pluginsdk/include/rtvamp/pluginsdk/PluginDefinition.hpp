#pragma once

#include <array>
#include <complex>
#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "rtvamp/pluginsdk/Plugin.hpp"

namespace rtvamp::pluginsdk {

/**
 * Base class to implement feature extraction plugins.
 * 
 * The number of outputs is provided as a template parameter.
 */
template <uint32_t NOutputs>
class PluginDefinition : public Plugin {
public:
    using Plugin::Plugin;  // inherit constructor

    static constexpr uint32_t outputCount = NOutputs;

    struct Meta {
        const char* identifier;
        const char* name;
        const char* description;
        const char* maker;
        const char* copyright;
        int         pluginVersion;
        InputDomain inputDomain;
    };

    // required static plugin descriptor
    static constexpr Meta meta{};

    // optional static descriptors, default: 0 parameters, 0 programs
    static constexpr std::array<ParameterDescriptor, 0> parameters{};
    static constexpr std::array<const char*, 0>         programs{};

    constexpr const char*   getIdentifier()    const override final { return meta.identifier; }
    constexpr const char*   getName()          const override final { return meta.name; }
    constexpr const char*   getDescription()   const override final { return meta.description; }
    constexpr const char*   getMaker()         const override final { return meta.maker; }
    constexpr const char*   getCopyright()     const override final { return meta.copyright; }
    constexpr int           getPluginVersion() const override final { return meta.pluginVersion; }
    constexpr InputDomain   getInputDomain()   const override final { return meta.inputDomain; }

    constexpr ParameterList getParameterList() const override final { return parameters; }
    constexpr ProgramList   getProgramList()   const override final { return programs; }

    constexpr uint32_t      getOutputCount()   const override final { return NOutputs; }

protected:
    using FeatureArray = std::array<Feature, NOutputs>;

    FeatureArray& getFeatureSet() noexcept { return featureSet_; }
    void initialiseFeatureSet();

private:
    FeatureArray featureSet_;
};

/* --------------------------------------- Implementation --------------------------------------- */

template <uint32_t NOutputs>
void PluginDefinition<NOutputs>::initialiseFeatureSet() {
    const auto outputs     = getOutputDescriptors();
    auto&      featureSet  = getFeatureSet();
    for (size_t i = 0; i < outputCount; ++i) {
        featureSet[i].resize(outputs[i].binCount);
    }
}

/* ------------------------------------------- Concept ------------------------------------------ */

template <typename T>
concept IsPluginMeta = requires(T t) {
    { t.identifier }    -> std::convertible_to<const char*>;
    { t.name }          -> std::convertible_to<const char*>;
    { t.description }   -> std::convertible_to<const char*>;
    { t.maker }         -> std::convertible_to<const char*>;
    { t.copyright }     -> std::convertible_to<const char*>;
    { t.pluginVersion } -> std::convertible_to<int>;
    { t.inputDomain }   -> std::convertible_to<Plugin::InputDomain>;
};

template <typename T>
concept IsPluginDefinition = requires(T t) {
    requires std::is_base_of_v<Plugin, T>;
    { t.outputCount } -> std::convertible_to<uint32_t>;
    { t.meta }        -> IsPluginMeta;
    { t.parameters }  -> std::convertible_to<Plugin::ParameterList>;
    { t.programs }    -> std::convertible_to<Plugin::ProgramList>;
};

}  // namespace rtvamp::pluginsdk
