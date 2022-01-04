#pragma once

#include <algorithm>  // transform
#include <array>
#include <cassert>
#include <span>
#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rtvamp/pluginsdk/Plugin.hpp"
#include "rtvamp/pluginsdk/PluginDefinition.hpp"

namespace rtvamp::pluginsdk {

/**
 * Generate plugin descriptor from plugin definition at compile time.
 */
template <IsPluginDefinition TPluginDefinition>
class VampPluginDescriptorWrapper {
public:
    static consteval VampPluginDescriptor get() {
        VampPluginDescriptor d{};
        // static plugin information
        d.vampApiVersion = 2;
        d.identifier     = TPluginDefinition::meta.identifier;
        d.name           = TPluginDefinition::meta.name;
        d.description    = TPluginDefinition::meta.description;
        d.maker          = TPluginDefinition::meta.maker;
        d.pluginVersion  = TPluginDefinition::meta.pluginVersion;
        d.copyright      = TPluginDefinition::meta.copyright;
        d.parameterCount = TPluginDefinition::parameters.size();
        d.parameters     = getParameters();
        d.programCount   = TPluginDefinition::programs.size();
        d.programs       = getPrograms();
        d.inputDomain    = getVampInputDomain();
        // function pointers set to nullptr by aggregate initializer
        return d;
    }

private:
    static consteval auto getVampInputDomain() {
        return TPluginDefinition::meta.inputDomain == Plugin::InputDomain::Frequency
            ? vampFrequencyDomain
            : vampTimeDomain;
    }

    static consteval const VampParameterDescriptor** getParameters() {
        if (vampParametersPtr.empty()) return nullptr;
        return const_cast<const VampParameterDescriptor**>(vampParametersPtr.data());
    }

    static consteval const char** getPrograms() {
        if (TPluginDefinition::programs.empty()) return nullptr;
        return const_cast<const char**>(TPluginDefinition::programs.data());
    }

    static constexpr auto parameterCount = TPluginDefinition::parameters.size();

    static constexpr auto vampParameters = [] {
        std::array<VampParameterDescriptor, parameterCount> result{};
        for (size_t i = 0; i < parameterCount; ++i) {
            const auto& p = TPluginDefinition::parameters[i];
            auto& vp      = result[i];

            vp.identifier   = p.identifier;
            vp.name         = p.name;
            vp.description  = p.description;
            vp.unit         = p.unit;
            vp.minValue     = p.minValue;
            vp.maxValue     = p.maxValue;
            vp.defaultValue = p.defaultValue;
            vp.isQuantized  = p.isQuantized;
            vp.quantizeStep = p.quantizeStep;
        }
        return result;
    }();

    static constexpr auto vampParametersPtr = [] {
        std::array<const VampParameterDescriptor*, parameterCount> result{};
        std::transform(
            vampParameters.begin(),
            vampParameters.end(),
            result.begin(),
            [](auto&& e) { return &e; }
        );
        return result;
    }();
};

/**
 * RAII wrapper for VampOutputDescriptor.
 * 
 * Strings (identifier, name, description, unit) are assumed to be compile-time constants and
 * therefore not copied.
 */
struct VampOutputDescriptorWrapper{
    explicit VampOutputDescriptorWrapper(const Plugin::OutputDescriptor& d)
        : binNames_(d.binNames)
    {
        if (!binNames_.empty()) {
            binNames_.resize(d.binCount);  // crop or fill missing names with empty strings
            binNamesConstChar_.resize(d.binCount);
            std::transform(
                binNames_.begin(),
                binNames_.end(),
                binNamesConstChar_.begin(),
                [] (const std::string& s) { return s.c_str(); }
            );
        }

        descriptor_.identifier       = d.identifier;
        descriptor_.name             = d.name;
        descriptor_.description      = d.description;
        descriptor_.unit             = d.unit;
        descriptor_.hasFixedBinCount = 1;
        descriptor_.binCount         = d.binCount;
        descriptor_.binNames         = binNames_.empty() ? nullptr : binNamesConstChar_.data();
        descriptor_.hasKnownExtents  = static_cast<int>(d.hasKnownExtents);
        descriptor_.minValue         = d.minValue;
        descriptor_.maxValue         = d.maxValue;
        descriptor_.isQuantized      = static_cast<int>(d.isQuantized);
        descriptor_.quantizeStep     = d.quantizeStep;
        descriptor_.sampleType       = vampOneSamplePerStep;
        descriptor_.sampleRate       = 0.0f;
        descriptor_.hasDuration      = 0;
    }

    const VampOutputDescriptor& get() const noexcept { return descriptor_; }
    VampOutputDescriptor&       get()       noexcept { return descriptor_; }

private:
    VampOutputDescriptor descriptor_;

    std::vector<std::string> binNames_;
    std::vector<const char*> binNamesConstChar_;
};


class VampFeatureUnionWrapper {
public:
    VampFeatureUnionWrapper() {
        auto& v1 = getV1();
        v1.hasTimestamp = 0;
        v1.sec          = 0;
        v1.nsec         = 0;
        v1.valueCount   = 0;
        v1.values       = values_.data();
        v1.label        = nullptr;

        auto& v2 = getV2();
        v2.hasDuration  = 0;
        v2.durationSec  = 0;
        v2.durationNsec = 0;
    }

    size_t getValueCount() const noexcept { return values_.size(); }

    void setValueCount(size_t n) {
        values_.resize(n);
        auto& v1 = getV1();
        v1.valueCount = n;
        v1.values     = values_.empty() ? nullptr : values_.data();  // might got reallocated
    }

    void assignValues(const Plugin::Feature& values) {
        if (const auto n = values.size(); getValueCount() != n) {
            setValueCount(n);
        }
        std::copy(values.begin(), values.end(), values_.begin());
    }

    const VampFeatureUnion* get() const noexcept { return featureUnion_.data(); }
    VampFeatureUnion*       get()       noexcept { return featureUnion_.data(); }

private:
    VampFeature&   getV1() noexcept { return featureUnion_[0].v1; }
    VampFeatureV2& getV2() noexcept { return featureUnion_[1].v2; }

    std::vector<float>              values_;
    std::array<VampFeatureUnion, 2> featureUnion_{};
};


template <size_t NOutputs>
class VampFeatureListsWrapper {
public:
    VampFeatureListsWrapper() {
        for (size_t i = 0; i < NOutputs; ++i) {
            featureLists_[i] = {
                .featureCount = 1,
                .features     = featureUnionWrappers_[i].get(),
            };
        }
    }

    void assignValues(size_t outputNumber, const Plugin::Feature& values) {
        assert(outputNumber < NOutputs);
        featureUnionWrappers_[outputNumber].assignValues(values);
    }

    void assignValues(std::span<const Plugin::Feature> values) {
        assert(values.size() == NOutputs);
        for (size_t i = 0; i < NOutputs; ++i) {
            assignValues(i, values[i]);
        }
    }

    VampFeatureList* get() noexcept { return featureLists_.data(); }

private:
    std::array<VampFeatureUnionWrapper, NOutputs> featureUnionWrappers_{};
    std::array<VampFeatureList, NOutputs>         featureLists_{};
};

}  // namespace rtvamp::pluginsdk