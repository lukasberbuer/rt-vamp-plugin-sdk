#pragma once

#include <string>
#include <string_view>

namespace rtvamp::hostsdk {

/**
 * Identifier for a plugin uniquely within the scope of the current system.
 * 
 * Is is composed of the library name and the plugin identifier: `<library name>:<identifier>`.
 * 
 * Example `example-plugin:rms`:
 * - library path: `C:\Program Files\Vamp Plugins\example-plugin.dll`
 * - plugin identifier: `rms`
 */
class PluginKey {
public:
    PluginKey(const char* key);
    PluginKey(std::string key);
    PluginKey(std::string_view key);
    PluginKey(std::string_view library, std::string_view identifier);

    std::string_view get()           const noexcept { return key_; }
    std::string_view getLibrary()    const noexcept;
    std::string_view getIdentifier() const noexcept;

    auto operator<=>(const PluginKey&) const = default;  

private:
    std::string key_;
    size_t      pos_;
};

}  // namespace rtvamp::hostsdk
