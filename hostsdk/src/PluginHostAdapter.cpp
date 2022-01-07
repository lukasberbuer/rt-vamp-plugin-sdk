#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>

#include "vamp/vamp.h"

#include "helper.hpp"

namespace rtvamp::hostsdk {

static const char* notNullptr(const char* s) {
    if (s == nullptr) return "";
    return s;
}

static std::vector<Plugin::ParameterDescriptor> getParameterDescriptors(
    const VampPluginDescriptor& descriptor
) {
    std::vector<Plugin::ParameterDescriptor> result(descriptor.parameterCount);

    for (size_t i = 0 ; i < descriptor.parameterCount; ++i) {
        auto* vampParameter = descriptor.parameters[i];
        auto& parameter     = result[i];

        parameter.identifier   = notNullptr(vampParameter->identifier);
        parameter.name         = notNullptr(vampParameter->name);
        parameter.description  = notNullptr(vampParameter->description);
        parameter.unit         = notNullptr(vampParameter->unit);
        parameter.minValue     = vampParameter->minValue;
        parameter.maxValue     = vampParameter->maxValue;
        parameter.defaultValue = vampParameter->defaultValue;
        parameter.isQuantized  = vampParameter->isQuantized == 1 ? true : false;
        parameter.quantizeStep = vampParameter->quantizeStep;
    }
    return result;
}

static std::optional<size_t> findParameterIndex(
    const VampPluginDescriptor& descriptor, std::string_view identifier
) {
    for (size_t i = 0; i < descriptor.parameterCount; ++i) {
        if (std::string_view(descriptor.parameters[i]->identifier) == identifier)
            return i;
    }
    return {};
}

static std::optional<size_t> findProgramIndex(
    const VampPluginDescriptor& descriptor, std::string_view program
) {
    for (size_t i = 0; i < descriptor.programCount; ++i) {
        if (std::string_view(descriptor.programs[i]) == program)
            return i;
    }
    return {};
}

PluginHostAdapter::PluginHostAdapter(
    const VampPluginDescriptor& descriptor, float inputSampleRate
) : Plugin(inputSampleRate), descriptor_{descriptor} {
    handle_ = descriptor_.instantiate(&descriptor_, inputSampleRate);
    if (!handle_) {
        throw std::runtime_error("Plugin instantiation failed");
    }

    parameters_ = getParameterDescriptors(descriptor_);
    programs_   = std::vector<const char*>(
        descriptor_.programs,
        descriptor_.programs + descriptor_.programCount
    );

    try {
        checkRequirements();
    } catch (const std::exception&) {
        descriptor_.cleanup(handle_);
        throw;
    }
}

PluginHostAdapter::~PluginHostAdapter() {
    descriptor_.cleanup(handle_);
}

std::string_view PluginHostAdapter::getIdentifier() const noexcept {
    return descriptor_.identifier;
}

std::string_view PluginHostAdapter::getName() const noexcept {
    return descriptor_.name;
}

std::string_view PluginHostAdapter::getDescription() const noexcept {
    return descriptor_.description;
}

std::string_view PluginHostAdapter::getMaker() const noexcept {
    return descriptor_.maker;
}

std::string_view PluginHostAdapter::getCopyright() const noexcept {
    return descriptor_.copyright;
}
int PluginHostAdapter::getPluginVersion() const noexcept {
    return descriptor_.pluginVersion;
}

Plugin::InputDomain PluginHostAdapter::getInputDomain() const noexcept {
    return descriptor_.inputDomain == vampFrequencyDomain
        ? InputDomain::Frequency
        : InputDomain::Time;
}

Plugin::ParameterList PluginHostAdapter::getParameterList() const noexcept {
    return parameters_;
}

float PluginHostAdapter::getParameter(std::string_view id) const {
    const auto optionalIndex = findParameterIndex(descriptor_, id);
    if (!optionalIndex) return 0.0f;
    return descriptor_.getParameter(handle_, optionalIndex.value());
}

void PluginHostAdapter::setParameter(std::string_view id, float value) {
    const auto optionalIndex = findParameterIndex(descriptor_, id);
    if (!optionalIndex) return;
    descriptor_.setParameter(handle_, optionalIndex.value(), value);
}

Plugin::ProgramList PluginHostAdapter::getProgramList() const noexcept {
    return programs_;
}

std::string_view PluginHostAdapter::getCurrentProgram() const {
    const auto index = descriptor_.getCurrentProgram(handle_);
    assert(index < descriptor_.programCount);
    return programs_[index];
}

void PluginHostAdapter::selectProgram(std::string_view name) {
    const auto optionalIndex = findProgramIndex(descriptor_, name);
    if (!optionalIndex) return;
    descriptor_.selectProgram(handle_, optionalIndex.value());
}

uint32_t PluginHostAdapter::getPreferredStepSize() const {
    return descriptor_.getPreferredStepSize(handle_);
}

uint32_t PluginHostAdapter::getPreferredBlockSize() const {
    return descriptor_.getPreferredBlockSize(handle_);
}

uint32_t PluginHostAdapter::getOutputCount() const {
    return descriptor_.getOutputCount(handle_);
}

Plugin::OutputList PluginHostAdapter::getOutputDescriptors() const {
    const auto outputCount = getOutputCount();
    std::vector<OutputDescriptor> outputs(outputCount);

    for (size_t i = 0; i < outputCount; ++i) {
        auto& output     = outputs[i];
        auto* vampOutput = descriptor_.getOutputDescriptor(handle_, i);
        assert(vampOutput != nullptr);

        output.identifier  = vampOutput->identifier;
        output.name        = vampOutput->name;
        output.description = vampOutput->description;
        output.unit        = vampOutput->unit;

        output.binCount = vampOutput->binCount;
        if (vampOutput->hasFixedBinCount && vampOutput->binNames) {
            bool validBinNames = false;
            output.binNames.resize(output.binCount);
            for (unsigned int j = 0; j < output.binCount; ++j) {
                if (const char* binName = vampOutput->binNames[j]) {
                    output.binNames[j] = binName;
                    validBinNames = true;
                }
            }
            if (!validBinNames) output.binNames.clear();
        }

        output.hasKnownExtents = vampOutput->hasKnownExtents == 1;
        output.minValue        = vampOutput->minValue;
        output.maxValue        = vampOutput->maxValue;
        output.isQuantized     = vampOutput->isQuantized == 1;
        output.quantizeStep    = vampOutput->quantizeStep;

        descriptor_.releaseOutputDescriptor(vampOutput);
    }

    return outputs;
}

bool PluginHostAdapter::initialise(uint32_t stepSize, uint32_t blockSize) {
    outputCount_ = getOutputCount();
    if (featureSet_.size() != outputCount_) {
        featureSet_.resize(outputCount_);
    }
    const auto success = descriptor_.initialise(handle_, 1, stepSize, blockSize);
    checkRequirements();  // output definitions might change dynamically
    initialised_ = true;
    return success;
}

void PluginHostAdapter::reset() {
    descriptor_.reset(handle_);
}

Plugin::FeatureSet PluginHostAdapter::process(InputBuffer buffer, uint64_t nsec) {
    assert(initialised_ && "Plugin must be initialised before process");

    const auto getInputBuffer = [&] {
        if (getInputDomain() == InputDomain::Frequency) {
            // casts between interleaved arrays and std::complex are guaranteed to be valid
            // https://en.cppreference.com/w/cpp/numeric/complex
            return reinterpret_cast<const float*>(
                std::get<FrequencyDomainBuffer>(buffer).data()
            );
        } else {
            return std::get<TimeDomainBuffer>(buffer).data();
        }
    };

    const float* const  inputBuffer  = getInputBuffer();
    const float* const* inputBuffers = &inputBuffer;

    auto* vampFeatureLists = descriptor_.process(
        handle_,
        inputBuffers,
        nsec / 1'000'000'000,
        nsec % 1'000'000'000
    );

    for (size_t i = 0; i < outputCount_; ++i) {
        const auto& vampFeatureList = vampFeatureLists[i];
        const auto& vampFeatureV1   = vampFeatureList.features[0].v1;
        auto&       feature         = featureSet_[i];

        assert(vampFeatureList.featureCount == 1);

        if (feature.size() != vampFeatureV1.valueCount) {
            feature.resize(vampFeatureV1.valueCount);
        }
        std::copy(
            vampFeatureV1.values,
            vampFeatureV1.values + vampFeatureV1.valueCount,
            feature.begin()
        );
    }

    descriptor_.releaseFeatureSet(vampFeatureLists);

    return featureSet_;
}

void PluginHostAdapter::checkRequirements() {
    if (descriptor_.getMinChannelCount(handle_) > 1) {
        throw std::runtime_error("Minimum channel count > 1 not supported");
    }

    for (size_t outputIndex = 0; outputIndex < getOutputCount(); ++outputIndex) {
        const auto* outputDescriptor = descriptor_.getOutputDescriptor(handle_, outputIndex);
        if (outputDescriptor->hasFixedBinCount != 1) {
            throw std::runtime_error(
                helper::concat(
                    "Dynamic bin count of output \"",
                    outputDescriptor->identifier,
                    "\" not supported"
                )
            );
        }
        if (outputDescriptor->sampleType != vampOneSamplePerStep) {
            throw std::runtime_error(
                helper::concat(
                    "Sample type of output \"",
                    outputDescriptor->identifier,
                    "\" not supported (OneSamplePerStep required)"
                )
            );
        }
    }
}

}  // namespace rtvamp::hostsdk
