#pragma once

#include <filesystem>
#include <string>

namespace rtvamp::hostsdk {

/**
 * OS uses reference counting for loading/closing dynamic libraries.
 * It's safe to load a library multiple times: the same handle will be returned.
 * 
 * References:
 * - https://man7.org/linux/man-pages/man3/dlopen.3.html
 * - https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryw
 */

class DynamicLibrary {
public:
    DynamicLibrary() = default;
    explicit DynamicLibrary(const std::filesystem::path& path) { load(path); }

    ~DynamicLibrary() { unload(); }

    // non-copyable
    DynamicLibrary(const DynamicLibrary&)            = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    // moveable
    DynamicLibrary(DynamicLibrary&& other) noexcept {
        std::swap(handle_, other.handle_);
    }
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept {
        std::swap(handle_, other.handle_);
        return *this;
    }

    bool load(const std::filesystem::path& path);
    void unload();

    void* getFunction(const char* functionName) noexcept;

    template <typename FunctionSignature>
    FunctionSignature getFunction(const char* functionName) noexcept {
        return reinterpret_cast<FunctionSignature>(getFunction(functionName));
    }

    bool isLoaded() const noexcept { return handle_ != nullptr; }

private:
    void* handle_{nullptr};
};

}  // namespace rtvamp::hostsdk
