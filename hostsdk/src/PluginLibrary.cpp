#include "PluginLibrary.hpp"

#include <algorithm>  // transform

#include "helper.hpp"

namespace rtvamp::hostsdk {

namespace dll {

extern void* load(const std::filesystem::path& path);
extern void  close(void* handle);
extern void* getFunction(void* handle, const char* name);

}  // namespace dll

PluginLibrary::PluginLibrary(const std::filesystem::path& libraryPath)
    : libraryPath_(libraryPath) {
    if (!std::filesystem::exists(libraryPath)) {
        throw std::runtime_error(helper::concat("Dynamic library does not exist: ", libraryPath));
    }

    handle_ = dll::load(libraryPath);

    try {
        const auto func = reinterpret_cast<VampGetPluginDescriptorFunction>(
            dll::getFunction(handle_, "vampGetPluginDescriptor")
        );

        unsigned int i = 0;
        while (const auto* descriptor = func(VAMP_API_VERSION, i++)) {
            descriptors_.push_back(descriptor);
        }
    } catch (...) {
        dll::close(handle_);
        throw;
    }
}

PluginLibrary::~PluginLibrary() {
    try {
        dll::close(handle_);
    } catch (const std::exception&) {
        // ...
    }
}

std::string PluginLibrary::getLibraryName() const {
    return libraryPath_.stem().string();
}

std::vector<std::string> PluginLibrary::getPlugins() const {
    std::vector<std::string> result(descriptors_.size());
    std::transform(
        descriptors_.begin(),
        descriptors_.end(),
        result.begin(),
        [](const VampPluginDescriptor* d) { return d->identifier; }
    );
    return result;
}

std::vector<const VampPluginDescriptor*> PluginLibrary::getDescriptors() const {
    return descriptors_;
}

}  // namespace rtvamp::hostsdk
