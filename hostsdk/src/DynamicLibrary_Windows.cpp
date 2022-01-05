#include "DynamicLibrary.hpp"

#include <Windows.h>

namespace rtvamp::hostsdk {

bool DynamicLibrary::loadImpl(const std::filesystem::path& path) {
    unloadImpl();
    handle_ = LoadLibraryW(path.wstring().c_str());  // unicode support
    return handle_ != nullptr;
}

void DynamicLibrary::unloadImpl() {
    if (handle_ == nullptr) return;
    FreeLibrary(reinterpret_cast<HMODULE>(handle_));
    handle_ = nullptr;
}

void* DynamicLibrary::symbolImpl(const char* name) noexcept {
    if (handle_ == nullptr) return nullptr;
    return GetProcAddress(reinterpret_cast<HMODULE>(handle_), name);
}

}  // namespace rtvamp::hostsdk
