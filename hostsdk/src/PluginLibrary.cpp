#include "rtvamp/hostsdk/PluginLibrary.hpp"

#include <stdexcept>

#include "vamp/vamp.h"

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "DynamicLibrary.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

PluginLibrary::PluginLibrary(const std::filesystem::path& libraryPath) {
    if (!std::filesystem::exists(libraryPath)) {
        throw std::runtime_error(helper::concat("Dynamic library does not exist: ", libraryPath));
    }

    dl_ = std::make_shared<DynamicLibrary>();

    if (!dl_->load(libraryPath)) {
        throw std::runtime_error(helper::concat("Error loading dynamic library: ", libraryPath));
    }

    constexpr const char* symbol = "vampGetPluginDescriptor";
    const auto func = dl_->getFunction<VampGetPluginDescriptorFunction>(symbol);
    if (!func) {
        throw std::runtime_error(
            helper::concat("Undefined symbol in dynamic library: ", symbol)
        );
    }

    unsigned int i = 0;
    while (const auto* descriptor = func(VAMP_API_VERSION, i++)) {
        descriptors_.push_back(descriptor);
    }
}

std::filesystem::path PluginLibrary::getLibraryPath() const noexcept {
    return dl_->path().value();
}

std::string PluginLibrary::getLibraryName() const {
    return getLibraryPath().stem().string();
}

size_t PluginLibrary::getPluginCount() const noexcept {
    return descriptors_.size();
}

std::vector<PluginKey> PluginLibrary::listPlugins() const {
    std::vector<PluginKey> result;
    result.reserve(descriptors_.size());

    for (auto&& descriptor : descriptors_) {
        result.emplace_back(getLibraryName(), descriptor->identifier);
    }

    return result;
}

std::unique_ptr<Plugin> PluginLibrary::loadPlugin(const PluginKey& key, float inputSampleRate) const {
    const auto* descriptor = [&] {
        for (const auto* d : descriptors_) {
            if (d->identifier == key.getIdentifier()) return d;
        }
        throw std::invalid_argument(
            helper::concat("Plugin not found: ", key.get())
        );
    }();

    return std::make_unique<PluginHostAdapter>(*descriptor, inputSampleRate, dl_);
}

std::unique_ptr<Plugin> PluginLibrary::loadPlugin(size_t index, float inputSampleRate) const {
    const auto count = getPluginCount();
    if (index >= count) {
        throw std::invalid_argument(
            helper::concat("Invalid plugin index: ", index, " >= ", count, " (available plugins)")
        );
    }

    return std::make_unique<PluginHostAdapter>(*descriptors_[index], inputSampleRate, dl_);
}

}  // namespace rtvamp::hostsdk
