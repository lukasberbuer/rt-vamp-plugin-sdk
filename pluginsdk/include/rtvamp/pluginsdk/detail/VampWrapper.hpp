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

template <typename T>
class Wrapper {
public:
    constexpr Wrapper() noexcept = default;
    constexpr Wrapper(T&& native) noexcept : native_(std::exchange(native, {})) {}  // NOLINT

    constexpr Wrapper(const Wrapper&) = delete;
    constexpr Wrapper(Wrapper&& other) noexcept
        : native_(std::exchange(other.native(), {})) {};

    constexpr Wrapper& operator=(const Wrapper&) = delete;
    constexpr Wrapper& operator=(Wrapper&& other) noexcept {
        if (this != &other) {
            clear(native_);
            native_ = std::exchange(other.get(), {});
        }
        return *this;
    }

    constexpr ~Wrapper() {
        clear(native_);
    }

    constexpr T& get() noexcept { return native_; }
    constexpr const T& get() const noexcept { return native_; }

private:
    T native_{};
};

template <typename T>
constexpr T* asNative(Wrapper<T>* wrapper) noexcept {
    return static_cast<T*>(static_cast<void*>(wrapper));
}

template <typename T>
constexpr const T* asNative(const Wrapper<T>* wrapper) noexcept {
    return static_cast<T*>(static_cast<void*>(wrapper));
}

template <typename T>
constexpr T& asNative(Wrapper<T>& wrapper) noexcept {
    return *asNative(&wrapper);
}

template <typename T>
constexpr const T& asNative(const Wrapper<T>& wrapper) noexcept {
    return *asNative(&wrapper);
}

[[nodiscard]] inline VampFeatureUnion makeVampFeatureUnion(size_t valueCount) {
    VampFeatureUnion featureUnion{};
    featureUnion.v1.valueCount = static_cast<unsigned int>(valueCount);
    featureUnion.v1.values     = new float[valueCount];  // NOLINT
    std::fill_n(featureUnion.v1.values, valueCount, 0.0F);
    return featureUnion;
}

[[nodiscard]] inline VampFeatureList makeVampFeatureList(size_t featureCount) {
    VampFeatureList featureList{};
    featureList.featureCount = static_cast<unsigned int>(featureCount);
    featureList.features     = new VampFeatureUnion[featureCount];  // NOLINT
    std::fill_n(featureList.features, featureCount, VampFeatureUnion{});
    return featureList;
}

inline void assignValues(VampFeatureUnion& featureUnion, std::span<const float> values) {
    auto& v1 = featureUnion.v1;
    if (v1.valueCount != values.size()) {
        delete[] v1.values;  // NOLINT
        v1.values = new float[values.size()];  // NOLINT
    }
    v1.valueCount = static_cast<unsigned int>(values.size());
    std::copy_n(values.data(), v1.valueCount, v1.values);
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
    native.binNames         = d.binNames.empty() ? nullptr : new const char*[d.binCount];
    std::transform(
        d.binNames.begin(),
        d.binNames.end(),
        native.binNames,
        [](const std::string& s) { return copy(s); }
    );
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
