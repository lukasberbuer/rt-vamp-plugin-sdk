#pragma once

#include <algorithm>  // transform
#include <span>
#include <string>
#include <string_view>
#include <utility>  // exchange

#include "vamp/vamp.h"

#include "rtvamp/pluginsdk/Plugin.hpp"

namespace rtvamp::pluginsdk::detail {

[[nodiscard]] inline char* copy(std::string_view str) {
    if (str.empty()) {
        return nullptr;
    }
    char* copy = new char[str.size() + 1];  // NOLINT
    std::copy_n(str.data(), str.size(), copy);
    copy[str.size()] = '\0';  // NOLINT
    return copy;
}

inline void clear(const char* str) {
    delete[] str;  // NOLINT
}

inline void clear(VampOutputDescriptor& desc) {
    clear(desc.identifier);
    clear(desc.name);
    clear(desc.description);
    clear(desc.unit);
    if (desc.binNames != nullptr) {
        std::for_each_n(
            desc.binNames,
            desc.binCount,
            [](const char* name) { clear(name); }
        );
        delete[] desc.binNames;  // NOLINT
    }
    desc = {};
}

inline void clear(VampFeature& feature) {
    delete[] feature.values;  // NOLINT
    delete[] feature.label;  // NOLINT
    feature = {};
}

inline void clear(VampFeatureUnion& featureUnion) {
    clear(featureUnion.v1);
    featureUnion = {};
}

inline void clear(VampFeatureList& featureList) {
    if (featureList.features != nullptr) {
        std::for_each_n(
            featureList.features,
            featureList.featureCount,
            [](VampFeatureUnion& featureUnion) { clear(featureUnion); }
        );
        delete[] featureList.features;  // NOLINT
    }
    featureList = {};
}

inline void assignValues(VampFeatureUnion& featureUnion, std::span<const float> values) {
    auto& v1 = featureUnion.v1;
    if (v1.valueCount != values.size()) {
        delete[] v1.values;  // NOLINT
        v1.values = new float[values.size()]{};  // NOLINT
    }
    v1.valueCount = static_cast<unsigned int>(values.size());
    std::copy_n(values.data(), v1.valueCount, v1.values);
}

[[nodiscard]] inline VampFeatureList makeVampFeatureList(size_t featureCount) {
    VampFeatureList featureList{};
    featureList.featureCount = static_cast<unsigned int>(featureCount);
    featureList.features     = new VampFeatureUnion[featureCount]{};  // NOLINT
    return featureList;
}

[[nodiscard]] inline VampOutputDescriptor makeVampOutputDescriptor(
    const PluginBase::OutputDescriptor& d
) {
    VampOutputDescriptor native{};
    native.identifier       = copy(d.identifier);
    native.name             = copy(d.name);
    native.description      = copy(d.description);
    native.unit             = copy(d.unit);
    native.hasFixedBinCount = 1;
    native.binCount         = d.binCount;
    native.binNames         = d.binNames.empty() ? nullptr : new const char*[d.binCount]{};
    if (native.binNames != nullptr) {
        std::transform(
            d.binNames.begin(),
            d.binNames.end(),
            native.binNames,
            [](const std::string& s) { return copy(s); }
        );
    }
    native.hasKnownExtents  = static_cast<int>(d.hasKnownExtents);
    native.minValue         = d.minValue;
    native.maxValue         = d.maxValue;
    native.isQuantized      = static_cast<int>(d.quantizeStep.has_value());
    native.quantizeStep     = d.quantizeStep.value_or(0.0F);
    native.sampleType       = vampOneSamplePerStep;
    native.sampleRate       = 0.0F;
    native.hasDuration      = 0;
    return native;
}

}  // namespace rtvamp::pluginsdk::detail
