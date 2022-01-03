#include "PluginLibrary.hpp"

#include <dlfcn.h>

#include "helper.hpp"

namespace rtvamp::hostsdk::dll {

void* load(const std::filesystem::path& path) {
    dlerror();  // clear any existing error
    void* handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        throw std::runtime_error(
            helper::concat("Error loading dynamic library: ", path, " (", dlerror(), ")")
        );
    }
    return handle;
}

void close(void* handle) {
    dlerror();  // clear any existing error
    const int ret = dlclose(handle);
    if (ret != 0) {
        throw std::runtime_error(
            helper::concat("Error closing dynamic library (", dlerror(), ")")
        );
    }
}

void* getFunction(void* handle, const char* name) {
    dlerror();  // clear any existing error
    void* funcPtr = dlsym(handle, name);
    if (const char* error = dlerror()) {
        throw std::runtime_error(
            helper::concat("Undefined symbol in dynamic library: ", name)
        );
    }
    return funcPtr;
}

}  // namespace rtvamp::hostsdk::dll
