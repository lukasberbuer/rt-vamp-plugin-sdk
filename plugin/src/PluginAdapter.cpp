#include <atomic>
#include <iostream>
#include <set>
#include <shared_mutex>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/PluginAdapter.h"

#include "VampOutputDescriptorCpp.h"
#include "VampPluginDescriptorCpp.h"

namespace rtvamp {

class PluginWrapper {
public:
    PluginWrapper(std::unique_ptr<Plugin> plugin)
        : plugin_{std::move(plugin)},
          parameters_(plugin_->getParameterDescriptors()),
          programs_(plugin_->getPrograms())
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
        const std::string program{plugin_->getCurrentProgram()};
        for (size_t i = 0; i < programs_.size(); ++i) {
            if (programs_[i] == program) return i;
        }
        return 0;
    }

    void selectProgram(unsigned int index) {
        try {
            plugin_->selectProgram(programs_.at(index));
            outputsNeedUpdate_ = true;
        } catch (const std::out_of_range&) {}
    }

    unsigned int getOutputCount() {
        updateOutputDescriptors();
        return outputs_.size();
    }

    VampOutputDescriptor* getOutputDescriptor(unsigned int index) {
        updateOutputDescriptors();
        std::shared_lock readerLock(mutex_);
        try {
            return new VampOutputDescriptorCpp(outputs_.at(index));
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
    }

    VampFeatureList* getRemainingFeatures() {

    }

    const Plugin* get() const noexcept { return plugin_.get(); }

private:
    void updateOutputDescriptors() {
        if (outputsNeedUpdate_) {
            std::unique_lock writerLock(mutex_);
            outputs_ = plugin_->getOutputDescriptors();
            outputsNeedUpdate_ = false;
        }
    }

    const std::unique_ptr<Plugin>            plugin_;
    const ParameterList                      parameters_;
    const ProgramList                        programs_;
    std::shared_mutex                        mutex_;
    size_t                                   blockSize_{0};
    OutputList                               outputs_;
    std::atomic<bool>                        outputsNeedUpdate_{true};
    std::unique_ptr<VampOutputDescriptorCpp> outputDescriptor_;
};

class PluginAdapterBase::Impl {
public:
    Impl(PluginAdapterBase&);
    ~Impl();

    const VampPluginDescriptor* getDescriptor();

private:
    static VampPluginHandle vampInstantiate(const VampPluginDescriptor*, float);
    static void             vampCleanup(VampPluginHandle);

    static PluginWrapper* findPluginWrapper(VampPluginHandle);

    using Instances = std::set<Impl*>;
    using Plugins   = std::vector<std::unique_ptr<PluginWrapper>>;

    inline static Instances         instances_;
    inline static std::shared_mutex instancesMutex_;
    inline static Plugins           plugins_;
    inline static std::shared_mutex pluginsMutex_;

    std::mutex                               mutex_;  // guard non-static members
    PluginAdapterBase&                       base_;
    std::unique_ptr<VampPluginDescriptorCpp> descriptor_{nullptr};
};

PluginAdapterBase::PluginAdapterBase() : impl_{std::make_unique<Impl>(*this)} {}
PluginAdapterBase::~PluginAdapterBase() = default;

const VampPluginDescriptor* PluginAdapterBase::getDescriptor() {
    return impl_->getDescriptor();
}

PluginAdapterBase::Impl::Impl(PluginAdapterBase& base) : base_(base) {
    std::unique_lock writeLock(instancesMutex_);
    instances_.insert(this);
}

PluginAdapterBase::Impl::~Impl() {
    std::unique_lock writeLock(instancesMutex_);
    instances_.erase(this);
}

const VampPluginDescriptor* PluginAdapterBase::Impl::getDescriptor() {
    std::lock_guard lock(mutex_);

    if (!descriptor_) {
        const auto plugin = base_.createPlugin(48000);

        if (plugin->getVampApiVersion() != VAMP_API_VERSION) {
            std::cerr << "API version of " << plugin->getVampApiVersion()
                      << " for plugin \"" << plugin->getIdentifier() << "\" differs from version "
                      << VAMP_API_VERSION << " for adapter.";
            return nullptr;
        }

        descriptor_ = std::make_unique<VampPluginDescriptorCpp>(*plugin);
        
        descriptor_->instantiate = vampInstantiate;
        descriptor_->cleanup     = vampCleanup;

        descriptor_->initialise = [](
            VampPluginHandle handle, unsigned int inputChannels, unsigned int stepSize, unsigned int blockSize
        ) {
            auto* wrapper = findPluginWrapper(handle);
            if (!wrapper) return 0;
            return wrapper->initialise(inputChannels, stepSize, blockSize);
        };

        descriptor_->reset = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            if (!wrapper) return;
            wrapper->reset();
        };

        descriptor_->getParameter = [](VampPluginHandle handle, int index) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->getParameter(index) : 0.0f;
        };

        descriptor_->setParameter = [](VampPluginHandle handle, int index, float value) {
            auto* wrapper = findPluginWrapper(handle);
            if (!wrapper) return;
            wrapper->setParameter(index, value);
        };

        descriptor_->getCurrentProgram = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->getCurrentProgram() : 0;
        };

        descriptor_->selectProgram = [](VampPluginHandle handle, unsigned int index) {
            auto* wrapper = findPluginWrapper(handle);
            if (!wrapper) return;
            wrapper->selectProgram(index);
        };

        descriptor_->getPreferredStepSize = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->get()->getPreferredStepSize() : 0;
        };

        descriptor_->getPreferredBlockSize = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->get()->getPreferredBlockSize() : 0;
        };

        descriptor_->getMinChannelCount = [](VampPluginHandle) -> unsigned {
            return 1;
        };

        descriptor_->getMaxChannelCount = [](VampPluginHandle) -> unsigned {
            return 1;
        };

        descriptor_->getOutputCount = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->getOutputCount() : 0;
        };

        descriptor_->getOutputDescriptor = [](VampPluginHandle handle, unsigned int index) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->getOutputDescriptor(index) : nullptr;
        };

        descriptor_->releaseOutputDescriptor = [](VampOutputDescriptor* desc) {
            if (desc == nullptr) return;
            delete static_cast<VampOutputDescriptorCpp*>(desc);
            desc = nullptr;
        };

        descriptor_->process = [](
            VampPluginHandle handle, const float* const* inputBuffers, int sec, int nsec
        ) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->process(inputBuffers, sec, nsec) : nullptr;
        };

        descriptor_->getRemainingFeatures = [](VampPluginHandle handle) {
            auto* wrapper = findPluginWrapper(handle);
            return wrapper ? wrapper->getRemainingFeatures() : nullptr;
        };

        descriptor_->releaseFeatureSet = [](VampFeatureList*) {};  // memory owned and released by plugin
    }

    return descriptor_.get();
}

PluginWrapper* PluginAdapterBase::Impl::findPluginWrapper(VampPluginHandle handle) {
    std::shared_lock readerLock(pluginsMutex_);
    auto it = std::find_if(
        plugins_.begin(),
        plugins_.end(),
        [&](const auto& wrapper) { return wrapper.get() == handle; }
    );
    return it == plugins_.end() ? nullptr : (*it).get();
}

VampPluginHandle PluginAdapterBase::Impl::vampInstantiate(
    const VampPluginDescriptor* desc, float inputSampleRate
) {
    std::shared_lock instancesReadLock(instancesMutex_);
    const auto it = std::find_if(
        instances_.begin(),
        instances_.end(),
        [&](Impl* instance) { return instance->getDescriptor() == desc; }
    );

    if (it == instances_.end()) return nullptr;

    std::unique_lock pluginsWriteLock(pluginsMutex_);
    auto& pluginWrapper = plugins_.emplace_back(
        std::make_unique<PluginWrapper>((*it)->base_.createPlugin(inputSampleRate))
    );
    return pluginWrapper.get();
}

void PluginAdapterBase::Impl::vampCleanup(VampPluginHandle handle) {
    std::unique_lock writerLock(pluginsMutex_);
    auto it = std::find_if(
        plugins_.begin(),
        plugins_.end(),
        [&](const auto& wrapper) { return wrapper.get() == handle; }
    );
    if (it != plugins_.end()) {
        plugins_.erase(it);
    }
}


}  // namespace rtvamp
