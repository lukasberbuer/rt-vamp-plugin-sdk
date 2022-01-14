#pragma once

#include "vamp/vamp.h"

#include "rtvamp/pluginsdk/Plugin.hpp"
#include "rtvamp/pluginsdk/detail/PluginAdapter.hpp"

namespace rtvamp::pluginsdk {

/**
 * Helper class to create the entry point for plugin libraries. 
 *
 * The plugins are defined as template parameters
 *
 * A Vamp plugin must export a function `vampGetPluginDescriptor`. The EntryPoint::getDescriptor()
 * method has the same signature as the required function. You have following options:
 * 1. Define, implement and export the `vampGetPluginDescriptor` manually
 * 2. Define, link and export the `vampGetPluginDescriptor` to the EntryPoint::getDescriptor()
 *    method
 * 3. Use the `RTVAMP_ENTRY_POINT(...) macro to automatically define and export the entry point.
 *
 *    Example: \include plugin/plugin.cpp
 */
template <IsPlugin... Plugins>
class EntryPoint {
public:
    static constexpr const VampPluginDescriptor* getDescriptor(
        unsigned int version,
        unsigned int index
    ) {
        if (version < 1 || version > VAMP_API_VERSION) return {};
        if (index >= pluginCount) return {};
        return descriptors[index];
    }

private:
    static constexpr auto pluginCount = sizeof...(Plugins);

    static constexpr std::array<const VampPluginDescriptor*, pluginCount> descriptors{
        {detail::PluginAdapter<Plugins>::getDescriptor()...}
    };
};

}  // namespace rtvamp::pluginsdk

/* -------------------------------------------- Macro ------------------------------------------- */

/**
 * Export entry point symbol with pragma.
 * Reference: https://docs.microsoft.com/de-de/cpp/build/reference/export-exports-a-function
 */
#ifdef _WIN32
    #define RTVAMP_EXPORT_ENTRY_POINT \
        _Pragma("comment(linker, \"/export:vampGetPluginDescriptor\")")
#else
    #define RTVAMP_EXPORT_ENTRY_POINT
#endif

/**
 * Generate entry point for given PluginDefintion types and export symbol with pragma.
 */
#define RTVAMP_ENTRY_POINT(...)                                                                    \
    RTVAMP_EXPORT_ENTRY_POINT                                                                      \
                                                                                                   \
    extern "C" const VampPluginDescriptor* vampGetPluginDescriptor(                                \
        unsigned int hostApiVersion,                                                                      \
        unsigned int index                                                                         \
    ) {                                                                                            \
        return ::rtvamp::pluginsdk::EntryPoint<__VA_ARGS__>::getDescriptor(hostApiVersion, index); \
    }
