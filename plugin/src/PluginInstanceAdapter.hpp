#pragma once

#include <atomic>
#include <shared_mutex>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/Plugin.hpp"

#include "VampFeatureWrapper.hpp"
#include "VampOutputDescriptorWrapper.hpp"

namespace rtvamp {

class PluginInstanceAdapter {
public:
    explicit PluginInstanceAdapter(std::unique_ptr<Plugin> plugin)
        : plugin_{std::move(plugin)},
          parameters_(plugin_->getParameterDescriptors())
    {
        updateOutputDescriptors();
    }

    int initialise(unsigned int /* inputChannels */, unsigned int stepSize, unsigned int blockSize) {
        blockSize_ = blockSize;
        const bool result = plugin_->initialise(stepSize, blockSize);
        outputsNeedUpdate_ = true;
        return result ? 1 : 0;
    }

    void reset() {
        plugin_->reset();
    }

    float getParameter(int index) const {
        try {
            return plugin_->getParameter(parameters_.at(index).identifier); 
        } catch (const std::out_of_range&) {}
        return 0.0f;
    }

    void setParameter(int index, float value) {
        try {
            plugin_->setParameter(parameters_.at(index).identifier, value);
            outputsNeedUpdate_ = true;
        } catch (const std::out_of_range&) {}
    }

    unsigned int getCurrentProgram() {
        const auto programs = plugin_->getPrograms();
        const auto program  = plugin_->getCurrentProgram();
        for (size_t i = 0; i < programs.size(); ++i) {
            if (programs[i] == program) return i;
        }
        return 0;
    }

    void selectProgram(unsigned int index) {
        const auto programs = plugin_->getPrograms();
        if (index >= programs.size()) return;
        plugin_->selectProgram(programs[index]);
    }

    unsigned int getOutputCount() {
        updateOutputDescriptors();
        return outputs_.size();
    }

    VampOutputDescriptor* getOutputDescriptor(unsigned int index) {
        updateOutputDescriptors();
        std::shared_lock readerLock(mutex_);
        try {
            return &outputDescriptorWrappers_.at(index).get();
        } catch (const std::out_of_range&) {}
        return nullptr;
    }

    VampFeatureList* process(const float* const* inputBuffers, int sec, int nsec) {
        const auto*   buffer    = inputBuffers[0];  // only first channel
        const int64_t timestamp = 1'000'000'000 * sec + nsec;

        auto getInputBuffer = [&]() -> InputBuffer {
            if (plugin_->getInputDomain() == InputDomain::TimeDomain) {
                return std::span(buffer, blockSize_);
            } else {
                // casts between interleaved arrays and std::complex are guaranteed to be valid
                // https://en.cppreference.com/w/cpp/numeric/complex
                return std::span(reinterpret_cast<const std::complex<float>*>(buffer), blockSize_ + 2);
            }
        };

        const auto& result = plugin_->process(getInputBuffer(), timestamp);

        assert(result.size() == outputs_.size());

        featureListsWrapper_.assignValues(result);
        return featureListsWrapper_.get();
    }

    VampFeatureList* getRemainingFeatures() {
        return featureListsEmpty_.data();  // list with featureCount = 0 and features = nullptr
    }

    const Plugin* get() const noexcept { return plugin_.get(); }

private:
    void updateOutputDescriptors() {
        if (outputsNeedUpdate_) {
            std::unique_lock writerLock(mutex_);

            outputs_     = plugin_->getOutputDescriptors();

            const size_t outputCount = outputs_.size();
            featureListsWrapper_.setOutputCount(outputCount);
            featureListsEmpty_.resize(outputCount, VampFeatureList{0, nullptr});

            // (re)generate vamp output descriptors
            outputDescriptorWrappers_.clear();
            for (const auto& output : outputs_) {
                outputDescriptorWrappers_.emplace_back(output);
            }

            outputsNeedUpdate_ = false;
        }
    }

    const std::unique_ptr<Plugin>            plugin_;
    const ParameterList                      parameters_;
    std::shared_mutex                        mutex_;
    size_t                                   blockSize_{0};
    std::atomic<bool>                        outputsNeedUpdate_{true};
    OutputList                               outputs_;
    std::vector<VampOutputDescriptorWrapper> outputDescriptorWrappers_;
    std::vector<VampFeatureList>             featureListsEmpty_;  // for getRemainingFeatures
    VampFeatureListsWrapper                  featureListsWrapper_;
};

}  // namespace rtvamp
