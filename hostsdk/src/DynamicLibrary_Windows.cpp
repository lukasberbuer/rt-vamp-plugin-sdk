#include "DynamicLibrary.hpp"

#include <Windows.h>

namespace rtvamp::hostsdk {

bool DynamicLibrary::load(const std::filesystem::path& path) {
    unload();
    handle_ = LoadLibraryW(path.wstring().c_str());  // unicode support
    return handle_ != nullptr;
}

void DynamicLibrary::unload() {
    if (handle_ == nullptr) return;
    FreeLibrary(reinterpret_cast<HMODULE>(handle_));
    handle_ = nullptr;
}

void* DynamicLibrary::getFunction(const char* functionName) noexcept {
    if (handle_ == nullptr) return nullptr;
    return GetProcAddress(reinterpret_cast<HMODULE>(handle_), functionName);
}

}  // namespace rtvamp::hostsdk
