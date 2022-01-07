#pragma once

#include <array>
#include <atomic>
#include <shared_mutex>
#include <utility>  // cmp_less

#include "rtvamp/pluginsdk/Plugin.hpp"
#include "rtvamp/pluginsdk/VampWrapper.hpp"

namespace rtvamp::pluginsdk {

template <IsPlugin TPlugin>
class PluginInstanceAdapter {
public:
    explicit PluginInstanceAdapter(float inputSampleRate) : plugin_(inputSampleRate) {
        updateOutputDescriptors();
    }

    int initialise(unsigned int /* inputChannels */, unsigned int stepSize, unsigned int blockSize) {
        blockSize_ = blockSize;
        const bool result = plugin_.initialise(stepSize, blockSize);
        outputsNeedUpdate_ = true;
        return result ? 1 : 0;
    }

    void reset() {
        plugin_.reset();
    }

    float getParameter(int index) const {
        // bounds checking in descriptor lambda
        return plugin_.getParameter(TPlugin::parameters[index].identifier); 
    }

    void setParameter(int index, float value) {
        // bounds checking in descriptor lambda
        plugin_.setParameter(TPlugin::parameters[index].identifier, value);
        outputsNeedUpdate_ = true;
    }

    unsigned int getCurrentProgram() const {
        const auto& programs = TPlugin::programs;
        const auto  program  = plugin_.getCurrentProgram();
        for (size_t i = 0; i < programs.size(); ++i) {
            if (programs[i] == program) return i;
        }
        return 0;
    }

    void selectProgram(unsigned int index) {
        // bounds checking in descriptor lambda
        plugin_.selectProgram(TPlugin::programs[index]);
        outputsNeedUpdate_ = true;
    }

    VampOutputDescriptor* getOutputDescriptor(unsigned int index) {
        updateOutputDescriptors();
        std::shared_lock readerLock(mutex_);
        // bounds checking in descriptor lambda
        return &outputDescriptorWrappers_[index].get();
    }

    VampFeatureList* process(const float* const* inputBuffers, int sec, int nsec) {
        const auto*   buffer    = inputBuffers[0];  // only first channel
        const int64_t timestamp = 1'000'000'000 * sec + nsec;

        const auto getInputBuffer = [&]() -> PluginBase::InputBuffer {
            if constexpr (TPlugin::meta.inputDomain == PluginBase::InputDomain::Time) {
                return std::span(buffer, blockSize_);
            } else {
                // casts between interleaved arrays and std::complex are guaranteed to be valid
                // https://en.cppreference.com/w/cpp/numeric/complex
                return std::span(reinterpret_cast<const std::complex<float>*>(buffer), blockSize_ + 2);
            }
        };

        const auto& result = plugin_.process(getInputBuffer(), timestamp);

        assert(result.size() == outputCount);

        featureListsWrapper_.assignValues(result);
        return featureListsWrapper_.get();
    }

    VampFeatureList* getRemainingFeatures() {
        static std::array<VampFeatureList, outputCount> empty{};  // aggregate initialization to set to {0, nullptr}
        return empty.data();
    }

    const TPlugin& get() const noexcept { return plugin_; }

private:
    void updateOutputDescriptors() {
        if (outputsNeedUpdate_) {
            std::unique_lock writerLock(mutex_);

            const auto descriptors = plugin_.getOutputDescriptors();

            // (re)generate vamp output descriptors
            outputDescriptorWrappers_.clear();
            for (const auto& d : descriptors) {
                outputDescriptorWrappers_.emplace_back(d);
            }

            outputsNeedUpdate_ = false;
        }
    }

    static constexpr auto outputCount = TPlugin::outputCount;

    TPlugin                                  plugin_;
    size_t                                   blockSize_{0};
    std::shared_mutex                        mutex_;
    std::atomic<bool>                        outputsNeedUpdate_{true};
    std::vector<VampOutputDescriptorWrapper> outputDescriptorWrappers_;
    VampFeatureListsWrapper<outputCount>     featureListsWrapper_;
};

template <IsPlugin TPlugin>
class PluginAdapter {
public:
    static consteval const VampPluginDescriptor* getDescriptor() { return &descriptor; }

private:
    using TPluginInstanceAdapter = PluginInstanceAdapter<TPlugin>;

    static VampPluginHandle vampInstantiate(
        const VampPluginDescriptor* desc, float inputSampleRate
    ) {
        // should the host create plugins with others descriptors? -> shared state
        if (desc != getDescriptor()) return nullptr;

        std::unique_lock writeLock(mutex);
        auto& adapter = plugins.emplace_back(
            std::make_unique<TPluginInstanceAdapter>(inputSampleRate)
        );
        return adapter.get();
    }

    static void vampCleanup(VampPluginHandle handle) {
        std::unique_lock writerLock(mutex);
        auto it = std::find_if(
            plugins.begin(),
            plugins.end(),
            [&](const auto& adapter) { return adapter.get() == handle; }
        );
        if (it != plugins.end()) {
            plugins.erase(it);
        }
    }

    static TPluginInstanceAdapter* findPlugin(VampPluginHandle handle) {
        std::shared_lock readerLock(mutex);
        auto it = std::find_if(
            plugins.begin(),
            plugins.end(),
            [&](const auto& adapter) { return adapter.get() == handle; }
        );
        return it == plugins.end() ? nullptr : (*it).get();
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
        auto d = VampPluginDescriptorWrapper<TPlugin>::get();

        d.instantiate = vampInstantiate;
        d.cleanup     = vampCleanup;

        d.initialise = [](
            VampPluginHandle handle, unsigned int inputChannels, unsigned int stepSize, unsigned int blockSize
        ) -> int {
            auto* adapter = findPlugin(handle);
            if (!adapter) return 0;
            return adapter->initialise(inputChannels, stepSize, blockSize);
        };

        d.reset = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            adapter->reset();
        };

        d.getParameter = [](VampPluginHandle handle, int index) {
            // check index before dispatching
            if (!isValidParameterIndex(index)) return 0.0f;
            auto* adapter = findPlugin(handle);
            if (!adapter) return 0.0f;
            return adapter->getParameter(index);
        };

        d.setParameter = [](VampPluginHandle handle, int index, float value) {
            // check index before dispatching
            if (!isValidParameterIndex(index)) return;
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            return adapter->setParameter(index, value);
        };

        d.getCurrentProgram = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getCurrentProgram() : 0;
        };

        d.selectProgram = [](VampPluginHandle handle, unsigned int index) {
            // check index before dispatching
            if (!isValidProgramIndex(index)) return;
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            adapter->selectProgram(index);
        };

        d.getPreferredStepSize = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->get().getPreferredStepSize() : 0;
        };

        d.getPreferredBlockSize = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->get().getPreferredBlockSize() : 0;
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
            if (!isValidOutputIndex(index)) return nullptr;
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getOutputDescriptor(index) : nullptr;
        };

        d.releaseOutputDescriptor = [](VampOutputDescriptor*) {};  // memory owned and released by plugin

        d.process = [](
            VampPluginHandle handle, const float* const* inputBuffers, int sec, int nsec
        ) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->process(inputBuffers, sec, nsec) : nullptr;
        };

        d.getRemainingFeatures = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getRemainingFeatures() : nullptr;
        };

        d.releaseFeatureSet = [](VampFeatureList*) {};  // memory owned and released by plugin

        return d;
    }();

    inline static std::shared_mutex mutex;
    inline static std::vector<std::unique_ptr<TPluginInstanceAdapter>> plugins;
};

}  // namespace rtvamp::pluginsdk
