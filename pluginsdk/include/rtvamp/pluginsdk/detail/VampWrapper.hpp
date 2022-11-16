#pragma once

#include <algorithm>  // transform
#include <array>
#include <cassert>
#include <span>
#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rtvamp/pluginsdk/Plugin.hpp"

namespace rtvamp::pluginsdk::detail {

/**
 * Generate plugin descriptor from plugin definition at compile time.
 */
template <detail::IsPlugin TPlugin>
class VampPluginDescriptorWrapper {
public:
    static consteval VampPluginDescriptor get() {
        VampPluginDescriptor d{};
        // static plugin information
        d.vampApiVersion = 2;
        d.identifier     = TPlugin::meta.identifier;
        d.name           = TPlugin::meta.name;
        d.description    = TPlugin::meta.description;
        d.maker          = TPlugin::meta.maker;
        d.pluginVersion  = TPlugin::meta.pluginVersion;
        d.copyright      = TPlugin::meta.copyright;
        d.parameterCount = static_cast<unsigned int>(TPlugin::parameters.size());
        d.parameters     = getParameters();
        d.programCount   = static_cast<unsigned int>(TPlugin::programs.size());
        d.programs       = getPrograms();
        d.inputDomain    = getVampInputDomain();
        // function pointers set to nullptr by aggregate initializer
        return d;
    }

private:
    static consteval auto getVampInputDomain() {
        return TPlugin::meta.inputDomain == TPlugin::InputDomain::Frequency
            ? vampFrequencyDomain
            : vampTimeDomain;
    }

    static consteval const VampParameterDescriptor** getParameters() {
        if (vampParametersPtr.empty()) return nullptr;
        return const_cast<const VampParameterDescriptor**>(vampParametersPtr.data());
    }

    static consteval const char** getPrograms() {
        if (TPlugin::programs.empty()) return nullptr;
        return const_cast<const char**>(TPlugin::programs.data());
    }

    static constexpr auto parameterCount = TPlugin::parameters.size();

    static constexpr auto vampParameters = [] {
        std::array<VampParameterDescriptor, parameterCount> result{};
        for (size_t i = 0; i < parameterCount; ++i) {
            const auto& p = TPlugin::parameters[i];
            auto& vp      = result[i];

            vp.identifier   = p.identifier;
            vp.name         = p.name;
            vp.description  = p.description;
            vp.unit         = p.unit;
            vp.defaultValue = p.defaultValue;
            vp.minValue     = p.minValue;
            vp.maxValue     = p.maxValue;
            vp.isQuantized  = static_cast<int>(p.quantizeStep.has_value());
            vp.quantizeStep = p.quantizeStep.value_or(0.0f);
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


struct VampOutputDescriptorWrapper{
    explicit VampOutputDescriptorWrapper(const PluginBase::OutputDescriptor& d)
        : identifier_(d.identifier),
          name_(d.name),
          description_(d.description),
          unit_(d.unit),
          binNames_(d.binNames)
    {
        if (!binNames_.empty()) {
            binNames_.resize(d.binCount);  // crop or fill missing names with empty strings
            binNamesConstChar_.resize(d.binCount);
        }

        // descriptor_.identifier       = identifier_.c_str();
        // descriptor_.name             = name_.c_str();
        // descriptor_.description      = description_.c_str();
        // descriptor_.unit             = unit_.c_str();
        descriptor_.hasFixedBinCount = 1;
        descriptor_.binCount         = d.binCount;
        // descriptor_.binNames         = binNames_.empty() ? nullptr : binNamesConstChar_.data();
        descriptor_.hasKnownExtents  = static_cast<int>(d.hasKnownExtents);
        descriptor_.minValue         = d.minValue;
        descriptor_.maxValue         = d.maxValue;
        descriptor_.isQuantized      = static_cast<int>(d.quantizeStep.has_value());
        descriptor_.quantizeStep     = d.quantizeStep.value_or(0.0f);
        descriptor_.sampleType       = vampOneSamplePerStep;
        descriptor_.sampleRate       = 0.0f;
        descriptor_.hasDuration      = 0;
    }

    VampOutputDescriptor* get() {
        initPointer();  // reinit pointer, might have been copied or moved
        return &descriptor_;
    }

private:
    void initPointer() {
        std::transform(
            binNames_.begin(),
            binNames_.end(),
            binNamesConstChar_.begin(),
            [] (const std::string& s) { return s.c_str(); }
        );

        descriptor_.identifier  = identifier_.c_str();
        descriptor_.name        = name_.c_str();
        descriptor_.description = description_.c_str();
        descriptor_.unit        = unit_.c_str();
        descriptor_.binNames    = binNames_.empty() ? nullptr : binNamesConstChar_.data();
    }

    VampOutputDescriptor     descriptor_;
    const std::string        identifier_;
    const std::string        name_;
    const std::string        description_;
    const std::string        unit_;
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

    // prevent copy/move (invalides pointer v1.values)
    VampFeatureUnionWrapper(const VampFeatureUnionWrapper&) = delete;
    VampFeatureUnionWrapper(VampFeatureUnionWrapper&&) = delete;
    VampFeatureUnionWrapper& operator=(const VampFeatureUnionWrapper&) = delete;
    VampFeatureUnionWrapper& operator=(VampFeatureUnionWrapper&&) = delete;

    size_t getValueCount() const noexcept { return values_.size(); }

    void setValueCount(size_t n) {
        assert(static_cast<unsigned int>(n) == n);
        values_.resize(n);
        auto& v1 = getV1();
        v1.valueCount = static_cast<unsigned int>(n);
        v1.values     = values_.empty() ? nullptr : values_.data();  // might got reallocated
    }

    void assignValues(const PluginBase::Feature& values) {
        if (const auto n = values.size(); getValueCount() != n) {
            setValueCount(n);
        }
        std::copy(values.begin(), values.end(), values_.begin());
    }

    VampFeatureUnion* get() noexcept { return featureUnion_.data(); }

private:
    VampFeature&   getV1() noexcept { return featureUnion_[0].v1; }
    VampFeatureV2& getV2() noexcept { return featureUnion_[1].v2; }

    std::vector<float>              values_;
    std::array<VampFeatureUnion, 2> featureUnion_{};
};


template <size_t NOutputs>
class VampFeatureListsWrapper {
public:
    constexpr VampFeatureListsWrapper() {
        for (size_t i = 0; i < NOutputs; ++i) {
            featureLists_[i] = {
                .featureCount = 1,
                .features     = featureUnionWrappers_[i].get(),
            };
        }
    }

    // prevent copy/move (invalides pointer .features)
    VampFeatureListsWrapper(const VampFeatureListsWrapper&) = delete;
    VampFeatureListsWrapper(VampFeatureListsWrapper&&) = delete;
    VampFeatureListsWrapper& operator=(const VampFeatureListsWrapper&) = delete;
    VampFeatureListsWrapper& operator=(VampFeatureListsWrapper&&) = delete;

    void assignValues(size_t outputNumber, const PluginBase::Feature& values) {
        assert(outputNumber < NOutputs);
        featureUnionWrappers_[outputNumber].assignValues(values);
    }

    void assignValues(std::span<const PluginBase::Feature> values) {
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

}  // namespace rtvamp::pluginsdk::detail
