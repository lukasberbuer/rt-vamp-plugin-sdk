#include <iostream>
#include <set>
#include <shared_mutex>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/PluginAdapter.h"

#include "PluginInstanceAdapter.hpp"
#include "VampFeatureWrapper.hpp"
#include "VampOutputDescriptorWrapper.h"
#include "VampPluginDescriptorCpp.h"

namespace rtvamp {

class PluginAdapterBase::Impl {
public:
    Impl(PluginAdapterBase&);
    ~Impl();

    const VampPluginDescriptor* getDescriptor();

private:
    static VampPluginHandle vampInstantiate(const VampPluginDescriptor*, float);
    static void             vampCleanup(VampPluginHandle);

    static PluginInstanceAdapter* findPlugin(VampPluginHandle);

    using Instances = std::set<Impl*>;
    using Plugins   = std::vector<std::unique_ptr<PluginInstanceAdapter>>;

    inline static Instances         instances_;
    inline static std::shared_mutex instancesMutex_;
    inline static Plugins           plugins_;
    inline static std::shared_mutex pluginsMutex_;

    std::mutex                               mutex_;  // guard non-static members
    PluginAdapterBase&                       base_;
    std::unique_ptr<VampPluginDescriptorCpp> descriptor_{nullptr};
};

/* ---------------------------------------------------------------------------------------------- */

PluginAdapterBase::PluginAdapterBase() : impl_{std::make_unique<Impl>(*this)} {}
PluginAdapterBase::~PluginAdapterBase() = default;

const VampPluginDescriptor* PluginAdapterBase::getDescriptor() {
    return impl_->getDescriptor();
}

/* ---------------------------------------------------------------------------------------------- */

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
            auto* adapter = findPlugin(handle);
            if (!adapter) return 0;
            return adapter->initialise(inputChannels, stepSize, blockSize);
        };

        descriptor_->reset = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            adapter->reset();
        };

        descriptor_->getParameter = [](VampPluginHandle handle, int index) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getParameter(index) : 0.0f;
        };

        descriptor_->setParameter = [](VampPluginHandle handle, int index, float value) {
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            adapter->setParameter(index, value);
        };

        descriptor_->getCurrentProgram = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getCurrentProgram() : 0;
        };

        descriptor_->selectProgram = [](VampPluginHandle handle, unsigned int index) {
            auto* adapter = findPlugin(handle);
            if (!adapter) return;
            adapter->selectProgram(index);
        };

        descriptor_->getPreferredStepSize = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->get()->getPreferredStepSize() : 0;
        };

        descriptor_->getPreferredBlockSize = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->get()->getPreferredBlockSize() : 0;
        };

        descriptor_->getMinChannelCount = [](VampPluginHandle) -> unsigned {
            return 1;
        };

        descriptor_->getMaxChannelCount = [](VampPluginHandle) -> unsigned {
            return 1;
        };

        descriptor_->getOutputCount = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getOutputCount() : 0;
        };

        descriptor_->getOutputDescriptor = [](VampPluginHandle handle, unsigned int index) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getOutputDescriptor(index) : nullptr;
        };

        descriptor_->releaseOutputDescriptor = [](VampOutputDescriptor*) {};  // memory owned and released by plugin

        descriptor_->process = [](
            VampPluginHandle handle, const float* const* inputBuffers, int sec, int nsec
        ) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->process(inputBuffers, sec, nsec) : nullptr;
        };

        descriptor_->getRemainingFeatures = [](VampPluginHandle handle) {
            auto* adapter = findPlugin(handle);
            return adapter ? adapter->getRemainingFeatures() : nullptr;
        };

        descriptor_->releaseFeatureSet = [](VampFeatureList*) {};  // memory owned and released by plugin
    }

    return descriptor_.get();
}

PluginInstanceAdapter* PluginAdapterBase::Impl::findPlugin(VampPluginHandle handle) {
    std::shared_lock readerLock(pluginsMutex_);
    auto it = std::find_if(
        plugins_.begin(),
        plugins_.end(),
        [&](const auto& adapter) { return adapter.get() == handle; }
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
        std::make_unique<PluginInstanceAdapter>((*it)->base_.createPlugin(inputSampleRate))
    );
    return pluginWrapper.get();
}

void PluginAdapterBase::Impl::vampCleanup(VampPluginHandle handle) {
    std::unique_lock writerLock(pluginsMutex_);
    auto it = std::find_if(
        plugins_.begin(),
        plugins_.end(),
        [&](const auto& adapter) { return adapter.get() == handle; }
    );
    if (it != plugins_.end()) {
        plugins_.erase(it);
    }
}

}  // namespace rtvamp
