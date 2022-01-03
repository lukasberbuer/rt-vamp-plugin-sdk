#include "PluginLibrary.hpp"

#include <Windows.h>

#include "helper.hpp"

namespace rtvamp::hostsdk::dll {

void* load(const std::filesystem::path& path) {
    void* handle = LoadLibraryW(path.wstring().c_str());  // unicode support
    if (!handle) {
        throw std::runtime_error(
            helper::concat("Error loading dynamic library: ", path, " (", GetLastError(), ")")
        );
    }
    return handle;
}

void close(void* handle) {
    const int success = FreeLibrary(reinterpret_cast<HMODULE>(handle));
    if (success == 0) {
        throw std::runtime_error(
            helper::concat("Error closing dynamic library (", GetLastError(), ")")
        );
    }
}

void* getFunction(void* handle, const char* name) {
    void* funcPtr = GetProcAddress(reinterpret_cast<HMODULE>(handle), name);
    if (!funcPtr) {
        throw std::runtime_error(
            helper::concat("Undefined symbol in dynamic library: ", name)
        );
    }
    return funcPtr;
}

}  // namespace rtvamp::hostsdk::dll
