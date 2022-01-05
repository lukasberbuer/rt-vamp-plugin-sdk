#pragma once

#include <filesystem>
#include <optional>
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

    // copy will increment the reference count of the underlying dynamic library (handled by OS)
    DynamicLibrary(const DynamicLibrary& other)            { assign(other); }
    DynamicLibrary& operator=(const DynamicLibrary& other) { assign(other); return *this; }

    DynamicLibrary(DynamicLibrary&& other)            noexcept { swap(other); }
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept { swap(other); return *this; }

    bool load(const std::filesystem::path& path) {
        if (loadImpl(path)) {
            path_ = path;
            return true;
        }
        return false;
    }

    void unload() {
        unloadImpl();
        path_ = std::nullopt;
    }

    bool isLoaded() const noexcept { return handle_ != nullptr; }

    template <typename FunctionSignature>
    FunctionSignature getFunction(const char* functionName) noexcept {
        return reinterpret_cast<FunctionSignature>(symbolImpl(functionName));
    }

    std::optional<std::filesystem::path> path() const noexcept { return path_; }

    void assign(const DynamicLibrary& other) {
        if (other.isLoaded()) {
            load(other.path().value());
        }
    }

    void swap(DynamicLibrary& other) noexcept {
        std::swap(path_, other.path_);
        std::swap(handle_, other.handle_);
    }

    void* handle() const noexcept { return handle_; }

private:
    // platform specific implementations
    bool  loadImpl(const std::filesystem::path& path);
    void  unloadImpl();
    void* symbolImpl(const char* name) noexcept;

    std::optional<std::filesystem::path> path_{};
    void* handle_{nullptr};
};

}  // namespace rtvamp::hostsdk
