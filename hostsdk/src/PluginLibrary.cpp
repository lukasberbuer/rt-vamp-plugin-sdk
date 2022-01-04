#include "PluginLibrary.hpp"

#include <algorithm>  // transform

#include "rtvamp/pluginsdk/macros.hpp"
#include "helper.hpp"

namespace rtvamp::hostsdk {

namespace dll {

/**
 * OS uses reference counting for loading/closing dynamic libraries.
 * It's safe to load a library multiple times: the same handle will be returned.
 * 
 * References:
 * - https://man7.org/linux/man-pages/man3/dlopen.3.html
 * - https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryw
 */

extern void* load(const std::filesystem::path& path) noexcept;
extern void* getFunction(void* handle, const char* name) noexcept;
extern bool  close(void* handle) noexcept;

}  // namespace dll

PluginLibrary::PluginLibrary(const std::filesystem::path& libraryPath)
    : libraryPath_(libraryPath) {
    if (!std::filesystem::exists(libraryPath)) {
        throw std::runtime_error(helper::concat("Dynamic library does not exist: ", libraryPath));
    }

    handle_ = dll::load(libraryPath);
    if (!handle_) {
        throw std::runtime_error(helper::concat("Error loading dynamic library: ", libraryPath));
    }

    try {
        constexpr const char* symbol = "vampGetPluginDescriptor";
        const auto func = reinterpret_cast<VampGetPluginDescriptorFunction>(
            dll::getFunction(handle_, symbol)
        );
        if (!func) {
            throw std::runtime_error(
                helper::concat("Undefined symbol in dynamic library: ", symbol)
            );
        }

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
    if (!dll::close(handle_)) {
        RTVAMP_DEBUG("Error closing dynamic library: ", libraryPath_);
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
