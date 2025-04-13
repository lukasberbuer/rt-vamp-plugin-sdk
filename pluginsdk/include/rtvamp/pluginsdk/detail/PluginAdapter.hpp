#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <complex>
#include <memory>
#include <mutex>
#include <utility>  // cmp_less
#include <vector>

#include "rtvamp/pluginsdk/Plugin.hpp"
#include "rtvamp/pluginsdk/detail/macros.hpp"
#include "rtvamp/pluginsdk/detail/VampWrapper.hpp"

namespace rtvamp::pluginsdk::detail {

template <IsPlugin TPlugin>
class PluginAdapter {
public:
    static constexpr const VampPluginDescriptor* getDescriptor() { return &descriptor; }

private:
    class Instance;
    class InstanceMap;

    inline static std::mutex mutex;
    inline static std::vector<std::unique_ptr<Instance>> instances;

    static Instance* getInstance(VampPluginHandle handle) {
        return reinterpret_cast<Instance*>(handle);  // NOLINT
    }

    static VampPluginHandle vampInstantiate(
        const VampPluginDescriptor* desc, float inputSampleRate
    ) {
        // should the host create instances with others descriptors? -> shared state
        // possible solution: overwrite function pointer in entry point and dispatch to adapters there
        if (desc != &descriptor) {
            return nullptr;
        }

        const std::unique_lock lock{mutex};
        auto& adapter = instances.emplace_back(
            std::make_unique<Instance>(inputSampleRate)
        );
        return adapter.get();
    }

    static void vampCleanup(VampPluginHandle handle) {
        const std::unique_lock lock{mutex};
        auto it = std::find_if(
            instances.begin(),
            instances.end(),
            [&](const auto& adapter) { return adapter.get() == handle; }
        );
        if (it != instances.end()) {
            instances.erase(it);
        }
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
            vp.quantizeStep = p.quantizeStep.value_or(0.0F);
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

    static constexpr const VampParameterDescriptor** getParameters() {
        return vampParametersPtr.empty()
            ? nullptr
            : const_cast<const VampParameterDescriptor**>(vampParametersPtr.data());
    }

    static constexpr const char** getPrograms() {
        return TPlugin::programs.empty()
            ? nullptr
            : const_cast<const char**>(TPlugin::programs.data());
    }

    static constexpr auto getVampInputDomain() {
        return TPlugin::meta.inputDomain == TPlugin::InputDomain::Frequency
            ? vampFrequencyDomain
            : vampTimeDomain;
    }

    static constexpr bool isValidParameterIndex(auto index) {
        return index >= 0 && std::cmp_less(index, TPlugin::parameters.size());
    }

    static constexpr bool isValidProgramIndex(auto index) {
        return index >= 0 && std::cmp_less(index, TPlugin::programs.size());
    }

    static constexpr bool isValidOutputIndex(auto index) {
        return index >= 0 && std::cmp_less(index, TPlugin::outputCount);
    }

    static constexpr VampPluginDescriptor descriptor = [] {
        VampPluginDescriptor d{};
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

        d.instantiate = vampInstantiate;
        d.cleanup     = vampCleanup;

        d.initialise = [](
            VampPluginHandle handle, unsigned int inputChannels, unsigned int stepSize, unsigned int blockSize
        ) -> int {
            return handle != nullptr ? getInstance(handle)->initialise(inputChannels, stepSize, blockSize) : 0;
        };

        d.reset = [](VampPluginHandle handle) {
            if (handle != nullptr) {
                getInstance(handle)->reset();
            }
        };

        d.getParameter = [](VampPluginHandle handle, int index) {
            if (!isValidParameterIndex(index)) {
                return 0.0F;
            }
            return handle != nullptr ? getInstance(handle)->getParameter(index) : 0.0F;
        };

        d.setParameter = [](VampPluginHandle handle, int index, float value) {
            if (!isValidParameterIndex(index)) {
                return;
            }
            if (handle != nullptr) {
                getInstance(handle)->setParameter(index, value);
            }
        };

        d.getCurrentProgram = [](VampPluginHandle handle) {
            return handle != nullptr ? getInstance(handle)->getCurrentProgram() : 0;
        };

        d.selectProgram = [](VampPluginHandle handle, unsigned int index) {
            if (!isValidProgramIndex(index)) {
                return;
            }
            if (handle != nullptr) {
                getInstance(handle)->selectProgram(index);
            }
        };

        d.getPreferredStepSize = [](VampPluginHandle handle) {
            return handle != nullptr ? getInstance(handle)->get().getPreferredStepSize() : 0;
        };

        d.getPreferredBlockSize = [](VampPluginHandle handle) {
            return handle != nullptr ? getInstance(handle)->get().getPreferredBlockSize() : 0;
        };

        d.getMinChannelCount = [](VampPluginHandle) -> unsigned int {
            return 1;
        };

        d.getMaxChannelCount = [](VampPluginHandle) -> unsigned int {
            return 1;
        };

        d.getOutputCount = [](VampPluginHandle) {
            return TPlugin::outputCount;
        };

        d.getOutputDescriptor = [](VampPluginHandle handle, unsigned int index) -> VampOutputDescriptor* {
            if (!isValidOutputIndex(index)) {
                return nullptr;
            }
            return handle != nullptr ? getInstance(handle)->getOutputDescriptor(index) : nullptr;
        };

        d.releaseOutputDescriptor = [](VampOutputDescriptor* descriptor) {
            if (descriptor != nullptr) {
                clear(*descriptor);
                delete descriptor;  // NOLINT
            }
        };

        d.process = [](
            VampPluginHandle handle, const float* const* inputBuffers, int sec, int nsec
        ) {
            return handle != nullptr ? getInstance(handle)->process(inputBuffers, sec, nsec) : nullptr;
        };

        d.getRemainingFeatures = [](VampPluginHandle handle) {
            return handle != nullptr ? getInstance(handle)->getRemainingFeatures() : nullptr;
        };

        d.releaseFeatureSet = [](VampFeatureList*) {};  // memory owned and released by plugin

        return d;
    }();
};

/* ------------------------------------------ Instance ------------------------------------------ */

template <IsPlugin TPlugin>
class PluginAdapter<TPlugin>::Instance {
public:
    explicit Instance(float inputSampleRate) : plugin_(inputSampleRate) {
        std::generate_n(
            featureLists_.begin(),
            outputCount,
            []() { return makeVampFeatureList(1); }
        );
    }

    int initialise(unsigned int /* inputChannels */, unsigned int stepSize, unsigned int blockSize) {
        blockSize_ = blockSize;
        try {
            const bool success = plugin_.initialise(stepSize, blockSize);
            return success ? 1 : 0;
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::initialise: ", e.what());
            return 0;
        }
    }

    void reset() {
        try {
            plugin_.reset();
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::reset: ", e.what());
        }
    }

    float getParameter(int index) const {
        // bounds checking in descriptor lambda
        try {
            return plugin_.getParameter(TPlugin::parameters[index].identifier).value_or(0.0F); 
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::getParameter: ", e.what());
            return 0.0F;
        }
    }

    void setParameter(int index, float value) {
        // bounds checking in descriptor lambda
        try {
            plugin_.setParameter(TPlugin::parameters[index].identifier, value);
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::setParameter: ", e.what());
        }
    }

    unsigned int getCurrentProgram() const {
        try {
            const auto& programs = TPlugin::programs;
            const auto  program  = plugin_.getCurrentProgram();
            for (unsigned int i = 0; i < static_cast<unsigned int>(programs.size()); ++i) {
                if (programs[i] == program) {
                    return i;
                }
            }
            return 0;
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::getCurrentProgram: ", e.what());
            return 0;
        }
    }

    void selectProgram(unsigned int index) {
        // bounds checking in descriptor lambda
        try {
            plugin_.selectProgram(TPlugin::programs[index]);
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::selectProgram: ", e.what());
        }
    }

    VampOutputDescriptor* getOutputDescriptor(unsigned int index) {
        const auto outputs = plugin_.getOutputDescriptors();
        if (index >= outputs.size()) {
            RTVAMP_ERROR("rtvamp::Plugin::getOutputDescriptor: index out of bounds");
            return nullptr;
        }
        return new VampOutputDescriptor{makeVampOutputDescriptor(outputs[index])};
    }

    VampFeatureList* process(const float* const* inputBuffers, int sec, int nsec) {
        const auto*   buffer    = *inputBuffers;  // only first channel
        const int64_t timestamp = static_cast<int64_t>(1'000'000'000) * sec + nsec;

        const auto getInputBuffer = [&]() -> typename TPlugin::InputBuffer {
            if constexpr (TPlugin::meta.inputDomain == TPlugin::InputDomain::Time) {
                return std::span(buffer, blockSize_);
            } else {
                // casts between interleaved arrays and std::complex are guaranteed to be valid
                // https://en.cppreference.com/w/cpp/numeric/complex
                // NOLINTNEXTLINE(*reinterpret-cast)
                return std::span(reinterpret_cast<const std::complex<float>*>(buffer), blockSize_ / 2 + 1);
            }
        };

        try {
            const auto& result = plugin_.process(getInputBuffer(), timestamp);
            assert(result.size() == outputCount);
            for (size_t i = 0; i < outputCount; ++i) {
                auto& featureList = featureLists_[i].get();
                assert(featureList.featureCount == 1);
                assert(featureList.features != nullptr);
                assignValues(*featureList.features, result[i]);
            }
        } catch (const std::exception& e) {
            RTVAMP_ERROR("rtvamp::Plugin::process: ", e.what());
        }

        return asNative(featureLists_.data());  // return last feature list if exception is thrown - better return nans?
    }

    VampFeatureList* getRemainingFeatures() {
        static std::array<VampFeatureList, outputCount> empty{};  // aggregate initialization to set to {0, nullptr}
        return empty.data();
    }

    const TPlugin& get() const noexcept { return plugin_; }

private:
    static constexpr auto outputCount = TPlugin::outputCount;

    TPlugin                                           plugin_;
    size_t                                            blockSize_{0};
    std::array<Wrapper<VampFeatureList>, outputCount> featureLists_{};
};

}  // namespace rtvamp::pluginsdk::detail
