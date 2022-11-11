#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>  // move

#include "vamp/vamp.h"

#include "DynamicLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

inline static const char* notNull(const char* str) {
    return str ? str : "";
}

template <typename T>
inline static std::optional<T> createOptional(T value, bool hasValue) {
    if (hasValue) return value;
    return {};
}

static std::vector<Plugin::ParameterDescriptor> convertParameterDescriptors(
    const VampPluginDescriptor& descriptor
) {
    std::vector<Plugin::ParameterDescriptor> result(descriptor.parameterCount);

    for (size_t i = 0 ; i < descriptor.parameterCount; ++i) {
        auto* vampParameter = descriptor.parameters[i];
        auto& parameter     = result[i];

        parameter.identifier   = notNull(vampParameter->identifier);
        parameter.name         = notNull(vampParameter->name);
        parameter.description  = notNull(vampParameter->description);
        parameter.unit         = notNull(vampParameter->unit);
        parameter.defaultValue = vampParameter->defaultValue;
        parameter.minValue     = vampParameter->minValue;
        parameter.maxValue     = vampParameter->maxValue;
        parameter.quantizeStep = createOptional(vampParameter->quantizeStep, vampParameter->isQuantized == 1);

        // if (vampParameter->isQuantized && vampParameter->valueNames) {
        //     size_t count = 0;
        //     for (auto ptr = vampParameter->valueNames; *ptr != nullptr; ++ptr) { ++count; }
        //     parameter.valueNames = std::vector<std::string_view>(
        //         vampParameter->valueNames,
        //         vampParameter->valueNames + count
        //     );
        // }
    }
    return result;
}

inline static std::optional<int> findParameterIndex(
    const VampPluginDescriptor& descriptor, std::string_view identifier
) {
    for (int i = 0; i < static_cast<int>(descriptor.parameterCount); ++i) {
        if (std::string_view(descriptor.parameters[i]->identifier) == identifier)
            return i;
    }
    return {};
}

static std::vector<std::string_view> convertPrograms(const VampPluginDescriptor& descriptor) {
    std::vector<std::string_view> result;
    result.reserve(descriptor.programCount);
    for (size_t i = 0; i < descriptor.programCount; ++i) {
        result.emplace_back(descriptor.programs[i]);
    }
    return result;
}

inline static std::optional<int> findProgramIndex(
    const VampPluginDescriptor& descriptor, std::string_view program
) {
    for (int i = 0; i < static_cast<int>(descriptor.programCount); ++i) {
        if (std::string_view(descriptor.programs[i]) == program)
            return i;
    }
    return {};
}

static void checkPluginDescriptor(const VampPluginDescriptor& d) {
    using Error = std::runtime_error;

    if (!d.instantiate)
        throw Error("Missing function pointer to instantiate");
    if (!d.cleanup)
        throw Error("Missing function pointer to clean");
    if (!d.initialise)
        throw Error("Missing function pointer to initialise");
    if (!d.reset)
        throw Error("Missing function pointer to reset");
    if (!d.getParameter)
        throw Error("Missing function pointer to getParameter");
    if (!d.setParameter)
        throw Error("Missing function pointer to setParameter");
    if (!d.getCurrentProgram)
        throw Error("Missing function pointer to getCurrentProgram");
    if (!d.selectProgram)
        throw Error("Missing function pointer to selectProgram");
    if (!d.getPreferredStepSize)
        throw Error("Missing function pointer to getPreferredStepSize");
    if (!d.getPreferredBlockSize)
        throw Error("Missing function pointer to getPreferredBlockSize");
    if (!d.getMinChannelCount)
        throw Error("Missing function pointer to getMinChannelCount");
    if (!d.getMaxChannelCount)
        throw Error("Missing function pointer to getMaxChannelCount");
    if (!d.getOutputCount)
        throw Error("Missing function pointer to getOutputCount");
    if (!d.getOutputDescriptor)
        throw Error("Missing function pointer to getOutputDescriptor");
    if (!d.releaseOutputDescriptor)
        throw Error("Missing function pointer to releaseOutputDescriptor");
    if (!d.process)
        throw Error("Missing function pointer to process");
    if (!d.getRemainingFeatures)
        throw Error("Missing function pointer to getRemainingFeatures");
    if (!d.releaseFeatureSet)
        throw Error("Missing function pointer to releaseFeatureSet");
}

PluginHostAdapter::PluginHostAdapter(
    const VampPluginDescriptor&     descriptor,
    float                           inputSampleRate,
    std::shared_ptr<DynamicLibrary> library
) : Plugin(inputSampleRate), descriptor_(descriptor), library_(std::move(library)) {
    checkPluginDescriptor(descriptor_);

    handle_ = descriptor_.instantiate(&descriptor_, inputSampleRate);
    if (!handle_) {
        throw std::runtime_error("Plugin instantiation failed");
    }

    parameters_ = convertParameterDescriptors(descriptor_);
    programs_   = convertPrograms(descriptor_);

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

std::filesystem::path PluginHostAdapter::getLibraryPath() const noexcept {
    if (library_ && library_->path()) {
        return library_->path().value();
    }
    return {};
}

uint32_t PluginHostAdapter::getVampApiVersion() const noexcept {
    return descriptor_.vampApiVersion;
}

std::string_view PluginHostAdapter::getIdentifier() const noexcept {
    return notNull(descriptor_.identifier);
}

std::string_view PluginHostAdapter::getName() const noexcept {
    return notNull(descriptor_.name);
}

std::string_view PluginHostAdapter::getDescription() const noexcept {
    return notNull(descriptor_.description);
}

std::string_view PluginHostAdapter::getMaker() const noexcept {
    return notNull(descriptor_.maker);
}

std::string_view PluginHostAdapter::getCopyright() const noexcept {
    return notNull(descriptor_.copyright);
}
int PluginHostAdapter::getPluginVersion() const noexcept {
    return descriptor_.pluginVersion;
}

Plugin::InputDomain PluginHostAdapter::getInputDomain() const noexcept {
    return descriptor_.inputDomain == vampFrequencyDomain
        ? InputDomain::Frequency
        : InputDomain::Time;
}

Plugin::ParameterList PluginHostAdapter::getParameterDescriptors() const noexcept {
    return parameters_;
}

std::optional<float> PluginHostAdapter::getParameter(std::string_view id) const {
    const auto optionalIndex = findParameterIndex(descriptor_, id);
    if (!optionalIndex) return {};
    return descriptor_.getParameter(handle_, optionalIndex.value());
}

bool PluginHostAdapter::setParameter(std::string_view id, float value) {
    const auto optionalIndex = findParameterIndex(descriptor_, id);
    if (!optionalIndex) return false;
    descriptor_.setParameter(handle_, optionalIndex.value(), value);
    return true;
}

Plugin::ProgramList PluginHostAdapter::getPrograms() const noexcept {
    return programs_;
}

Plugin::CurrentProgram PluginHostAdapter::getCurrentProgram() const {
    if (descriptor_.programCount == 0) return {};
    const auto index = descriptor_.getCurrentProgram(handle_);
    assert(index < descriptor_.programCount);
    return programs_[index];
}

bool PluginHostAdapter::selectProgram(std::string_view name) {
    const auto optionalIndex = findProgramIndex(descriptor_, name);
    if (!optionalIndex) return false;
    descriptor_.selectProgram(handle_, optionalIndex.value());
    return true;
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

    for (uint32_t i = 0; i < outputCount; ++i) {
        auto& output     = outputs[i];
        auto* vampOutput = descriptor_.getOutputDescriptor(handle_, static_cast<int>(i));

        if (!vampOutput) {
            throw std::runtime_error(helper::concat("Output descriptor ", i, " is null"));
        }

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
        output.quantizeStep    = createOptional(vampOutput->quantizeStep, vampOutput->isQuantized == 1);

        descriptor_.releaseOutputDescriptor(vampOutput);
    }

    return outputs;
}

bool PluginHostAdapter::initialise(uint32_t stepSize, uint32_t blockSize) {
    outputCount_ = getOutputCount();
    if (featureSet_.size() != outputCount_) {
        featureSet_.resize(outputCount_);
    }
    initialised_ = descriptor_.initialise(handle_, 1, stepSize, blockSize);
    initialisedBlockSize_ = blockSize;
    checkRequirements();  // output definitions might change dynamically
    return initialised_;
}

void PluginHostAdapter::reset() {
    descriptor_.reset(handle_);
}

Plugin::FeatureSet PluginHostAdapter::process(InputBuffer buffer, uint64_t nsec) {
#ifdef RTVAMP_VALIDATE
    if (!initialised_) {
        throw std::logic_error("Plugin must be initialised before process");
    }
#else
    assert(initialised_ && "Plugin must be initialised before process");
#endif

    const bool isTimeDomain = getInputDomain() == InputDomain::Time;
    const bool validType    = std::holds_alternative<TimeDomainBuffer>(buffer) == isTimeDomain;

    if (!validType && isTimeDomain) {
        throw std::invalid_argument("Wrong input buffer type: Time domain required");
    }
    if (!validType && !isTimeDomain) {
        throw std::invalid_argument("Wrong input buffer type: Frequency domain required");
    }

#ifdef RTVAMP_VALIDATE
    const auto blockSize         = std::visit([] (auto&& buf) { return buf.size(); }, buffer);
    const auto expectedBlockSize = isTimeDomain
        ? initialisedBlockSize_
        : initialisedBlockSize_ / 2 + 1;
    if (blockSize != expectedBlockSize) {
        throw std::invalid_argument(
            helper::concat(
                "Wrong input buffer size: Buffer size must match initialised block size of ",
                initialisedBlockSize_
            )
        );
    }
#endif

    const auto getInputBuffer = [&] {
        if (isTimeDomain) {
            return std::get<TimeDomainBuffer>(buffer).data();
        } else {
            // casts between interleaved arrays and std::complex are guaranteed to be valid
            // https://en.cppreference.com/w/cpp/numeric/complex
            return reinterpret_cast<const float*>(std::get<FrequencyDomainBuffer>(buffer).data());
        }
    };

    const float* const  inputBuffer  = getInputBuffer();
    const float* const* inputBuffers = &inputBuffer;

    auto* vampFeatureLists = descriptor_.process(
        handle_,
        inputBuffers,
        static_cast<int>(nsec / 1'000'000'000),
        static_cast<int>(nsec % 1'000'000'000)
    );

    if (!vampFeatureLists) {
        throw std::runtime_error("Returned feature list is null");
    }

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
    using Error = std::runtime_error;

    if (descriptor_.vampApiVersion < 1 || descriptor_.vampApiVersion > 2) {
        throw Error("Only Vamp API versions 1 and 2 supported");
    }

    if (descriptor_.getMinChannelCount(handle_) > 1) {
        throw Error("Minimum channel count > 1 not supported");
    }

    for (uint32_t outputIndex = 0; outputIndex < getOutputCount(); ++outputIndex) {
        const auto* outputDescriptor = descriptor_.getOutputDescriptor(handle_, outputIndex);

        if (!outputDescriptor) {
            throw Error(helper::concat("Output descriptor ", outputIndex, " is null"));
        }

        if (outputDescriptor->hasFixedBinCount != 1) {
            throw Error(
                helper::concat(
                    "Dynamic bin count of output \"",
                    outputDescriptor->identifier,
                    "\" not supported"
                )
            );
        }
        if (outputDescriptor->sampleType != vampOneSamplePerStep) {
            throw Error(
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
