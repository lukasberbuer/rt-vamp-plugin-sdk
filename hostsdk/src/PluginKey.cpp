#include "rtvamp/hostsdk/PluginKey.hpp"

#include <stdexcept>

#include "helper.hpp"

namespace rtvamp::hostsdk {

PluginKey::PluginKey(const char* key) : PluginKey(std::string(key)) {}

PluginKey::PluginKey(std::string_view key) : PluginKey(std::string(key)) {}

PluginKey::PluginKey(std::string key) : key_(std::move(key)) {
    if (key_.empty()) {
        throw std::invalid_argument("Plugin key empty");
    }

    pos_ = key_.find(':');
    if (
        pos_ == std::string::npos ||  // not found
        pos_ == 0 ||                  // first character -> empty library name
        pos_ == key_.size() - 1       // last character -> empty identifier
    ) {
        throw std::invalid_argument(helper::concat("Invalid plugin key: ", key_));
    }
}

PluginKey::PluginKey(std::string_view library, std::string_view identifier)
    : PluginKey(std::string(library).append(":").append(identifier)) {}

std::string_view PluginKey::getLibrary() const noexcept {
    return std::string_view(key_).substr(0, pos_);
}

std::string_view PluginKey::getIdentifier() const noexcept {
    return std::string_view(key_).substr(pos_ + 1);
}

}  // namespace rtvamp::hostsdk
