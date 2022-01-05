#include "DynamicLibrary.hpp"

#include <dlfcn.h>

namespace rtvamp::hostsdk {

bool DynamicLibrary::loadImpl(const std::filesystem::path& path) {
    unloadImpl();
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    return handle_ != nullptr;
}

void DynamicLibrary::unloadImpl() {
    if (handle_ == nullptr) return;
    dlclose(handle_);
    handle_ = nullptr;
}

void* DynamicLibrary::symbolImpl(const char* name) noexcept {
    if (handle_ == nullptr) return nullptr;
    return dlsym(handle_, name);
}

}  // namespace rtvamp::hostsdk
