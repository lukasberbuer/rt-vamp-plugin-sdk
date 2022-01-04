#include "PluginLibrary.hpp"

#include <dlfcn.h>

namespace rtvamp::hostsdk::dll {

void* load(const std::filesystem::path& path) noexcept {
    dlerror();  // clear any existing error
    void* handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    return handle;
}

bool close(void* handle) noexcept {
    dlerror();  // clear any existing error
    const int err = dlclose(handle);
    return err == 0;
}

void* getFunction(void* handle, const char* name) noexcept {
    dlerror();  // clear any existing error
    void* funcPtr = dlsym(handle, name);
    if (dlerror()) return nullptr;
    return funcPtr;
}

}  // namespace rtvamp::hostsdk::dll
