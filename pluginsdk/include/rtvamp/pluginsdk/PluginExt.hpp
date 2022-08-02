#pragma once

#include <algorithm>  // clamp
#include <array>
#include <cassert>
#include <cmath>  // round
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "rtvamp/pluginsdk/Plugin.hpp"

namespace rtvamp::pluginsdk {

/**
 * Extended plugin base class with automatic parameter / program handling.
 * 
 * Two template arguments must be provided:
 * 1. the implementation type itself to access static data (parameter descriptors / programs)
 *    (this pattern is known as the curiously recurring template pattern (CRTP))
 * 2. number of outputs
 * 
 * Assumptions:
 * - first program is enabled by default -> default parameters should match program settings
 */
template <typename Self, uint32_t NOutputs>
class PluginExt : public Plugin<NOutputs> {
public:
    explicit PluginExt(float inputSampleRate) : Plugin<NOutputs>(inputSampleRate) {}

    std::optional<float> getParameter(std::string_view id) const override final;
    bool                 setParameter(std::string_view id, float value) override final;

    std::string_view     getCurrentProgram() const override final;
    bool                 selectProgram(std::string_view name) override final;

    // custom logic can be implemented with following callbacks
    virtual void         onParameterChange(std::string_view id, float newValue) {}
    virtual void         onProgramChange(std::string_view newProgram) {}

private:
    static           std::vector<float>    defaultParameterValues();
    static constexpr std::optional<size_t> findParameterIndex(std::string_view id);
    static constexpr std::optional<size_t> findProgramIndex(std::string_view name);

    std::vector<float> parameterValues_{defaultParameterValues()};
    size_t             programIndex_{0};
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename Self, uint32_t NOutputs>
std::optional<float> PluginExt<Self, NOutputs>::getParameter(std::string_view id) const {
    if (const auto index = findParameterIndex(id)) {
        return parameterValues_[index.value()];
    }
    return {};
}

template <typename Self, uint32_t NOutputs>
bool PluginExt<Self, NOutputs>::setParameter(std::string_view id, float value) {
    if (const auto index = findParameterIndex(id)) {
        const auto& descriptor = Self::parameters[index.value()];

        if (descriptor.quantizeStep) {
            const auto quantizeStep = descriptor.quantizeStep.value();
            value = std::round(value / quantizeStep) * quantizeStep;
        }
        value = std::clamp(value, descriptor.minValue, descriptor.maxValue);

        parameterValues_[index.value()] = value;
        onParameterChange(id, value);
        return true;
    }
    return false;
}

template <typename Self, uint32_t NOutputs>
std::string_view PluginExt<Self, NOutputs>::getCurrentProgram() const {
    assert(programIndex_ < Self::programs.size());
    return Self::programs[programIndex_];
}

template <typename Self, uint32_t NOutputs>
bool PluginExt<Self, NOutputs>::selectProgram(std::string_view name) {
    if (const auto index = findProgramIndex(name)) {
        programIndex_ = index.value();
        onProgramChange(name);
        return true;
    }
    return false;
}

template <typename Self, uint32_t NOutputs>
std::vector<float> PluginExt<Self, NOutputs>::defaultParameterValues() {
    std::vector<float> values(Self::parameters.size());
    for (size_t i = 0; i < Self::parameters.size(); ++i) {
        values[i] = Self::parameters[i].defaultValue;
    }
    return values;
}

template <typename Self, uint32_t NOutputs>
constexpr std::optional<size_t> PluginExt<Self, NOutputs>::findParameterIndex(std::string_view id) {
    for (size_t i = 0; i < Self::parameters.size(); ++i) {
        if (Self::parameters[i].identifier == id) return i;
    }
    return {};
}

template <typename Self, uint32_t NOutputs>
constexpr std::optional<size_t> PluginExt<Self, NOutputs>::findProgramIndex(std::string_view name) {
    for (size_t i = 0; i < Self::programs.size(); ++i) {
        if (Self::programs[i] == name) return i;
    }
    return {};
}

}  // namespace rtvamp::pluginsdk
