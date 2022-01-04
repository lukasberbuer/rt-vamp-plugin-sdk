#include "PluginLibrary.hpp"

#include <Windows.h>

namespace rtvamp::hostsdk::dll {

void* load(const std::filesystem::path& path) noexcept {
    void* handle = LoadLibraryW(path.wstring().c_str());  // unicode support
    return handle;
}

bool close(void* handle) noexcept {
    const int success = FreeLibrary(reinterpret_cast<HMODULE>(handle));
    return success != 0;
}

void* getFunction(void* handle, const char* name) noexcept {
    void* funcPtr = GetProcAddress(reinterpret_cast<HMODULE>(handle), name);
    return funcPtr;
}

}  // namespace rtvamp::hostsdk::dll
