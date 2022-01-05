#include "DynamicLibrary.hpp"

#include <dlfcn.h>

namespace rtvamp::hostsdk {

bool DynamicLibrary::load(const std::filesystem::path& path) {
    unload();
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    return handle_ != nullptr;
}

void DynamicLibrary::unload() {
    if (handle_ == nullptr) return;
    dlclose(handle_);
    handle_ = nullptr;
}

void* DynamicLibrary::getFunction(const char* functionName) noexcept {
    if (handle_ == nullptr) return nullptr;
    return dlsym(handle_, functionName);
}

}  // namespace rtvamp::hostsdk
