#include "PluginLibrary.hpp"

#include "helper.hpp"

namespace rtvamp::hostsdk {

PluginLibrary::PluginLibrary(const std::filesystem::path& libraryPath) {
    if (!std::filesystem::exists(libraryPath)) {
        throw std::runtime_error(helper::concat("Dynamic library does not exist: ", libraryPath));
    }

    if (!dl_.load(libraryPath)) {
        throw std::runtime_error(helper::concat("Error loading dynamic library: ", libraryPath));
    }

    constexpr const char* symbol = "vampGetPluginDescriptor";
    const auto func = dl_.getFunction<VampGetPluginDescriptorFunction>(symbol);
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

std::string PluginLibrary::getLibraryName() const {
    return dl_.path().value().stem().string();
}

std::vector<const VampPluginDescriptor*> PluginLibrary::getDescriptors() const {
    return descriptors_;
}

}  // namespace rtvamp::hostsdk
