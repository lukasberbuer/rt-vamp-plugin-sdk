#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <sndfile.hh>

#include "rtvamp/hostsdk/Plugin.hpp"
#include "rtvamp/hostsdk/PluginLoader.hpp"

#include "helper.hpp"

using rtvamp::hostsdk::Plugin;
using rtvamp::hostsdk::PluginLoader;

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt) {
    if (!opt) return os << "null";
    return os << opt.value();
}

std::ostream& operator<<(std::ostream& os, const Plugin::InputDomain& d) {
    return os << (d == Plugin::InputDomain::Frequency ? "Frequency" : "Time");
}

void listPlugins() {
    PluginLoader loader;

    for (auto&& lib : loader.listLibraries()) {
        for (auto&& key : loader.listPluginsInLibrary(lib)) {
            std::cout << std::boolalpha;
            std::cout << Escape::Blue << "Plugin " << Escape::Bold << key.get() << Escape::Reset 
                << " (" << lib.string() << ")\n";

            try {
                const auto plugin = loader.loadPlugin(key, 48000);
                std::cout << "- Identifier:           " << plugin->getIdentifier() << '\n';
                std::cout << "- Name:                 " << plugin->getName() << '\n';
                std::cout << "- Description:          " << plugin->getDescription() << '\n';
                std::cout << "- Maker:                " << plugin->getMaker() << '\n';
                std::cout << "- Copyright:            " << plugin->getCopyright() << '\n';
                std::cout << "- Plugin version:       " << plugin->getPluginVersion() << '\n';
                std::cout << "- Input domain:         " << plugin->getInputDomain() << '\n';
                std::cout << "- Preferred step size:  " << plugin->getPreferredStepSize() << '\n';
                std::cout << "- Preferred block size: " << plugin->getPreferredBlockSize() << '\n';
                std::cout << "- Programs:             " << join(plugin->getProgramList()) << '\n';

                std::cout << "- Parameters:\n";
                size_t parameterIndex = 0;
                for (auto&& p : plugin->getParameterList()) {
                    std::cout << "  - Parameter " << ++parameterIndex << ":\n";
                    std::cout << "    - Identifier:       " << p.identifier << '\n';
                    std::cout << "    - Name:             " << p.name << '\n';
                    std::cout << "    - Description:      " << p.description << '\n';
                    std::cout << "    - Unit:             " << p.unit << '\n';
                    std::cout << "    - Default value:    " << p.defaultValue << '\n';
                    std::cout << "    - Minimum value:    " << p.minValue << '\n';
                    std::cout << "    - Maximum value:    " << p.maxValue << '\n';
                    std::cout << "    - Quantize step:    " << p.quantizeStep << '\n';
                }

                std::cout << "- Outputs:\n";
                size_t outputIndex = 0;
                for (auto&& o : plugin->getOutputDescriptors()) {
                    std::cout << "  - Output " << ++outputIndex << ":\n";
                    std::cout << "    - Identifier:       " << o.identifier << '\n';
                    std::cout << "    - Name:             " << o.name << '\n';
                    std::cout << "    - Description:      " << o.description << '\n';
                    std::cout << "    - Unit:             " << o.unit << '\n';
                    std::cout << "    - Bin count:        " << o.binCount << '\n';
                    std::cout << "    - Bin names:        " << join(o.binNames) << '\n';
                    std::cout << "    - Known extends:    " << o.hasKnownExtents << '\n';
                    std::cout << "    - Minimum value:    " << o.minValue << '\n';
                    std::cout << "    - Maximum value:    " << o.maxValue << '\n';
                    std::cout << "    - Quantize step:    " << o.quantizeStep << '\n';
                }
            } catch (const std::exception& e) {
                std::cout << Escape::Red << "[ERROR] " << e.what() << Escape::Reset << '\n';
            }
            std::cout << '\n';
        }
    }
    std::cout << std::flush;
}

void listPaths() {
    for (auto&& path : PluginLoader::getPluginPaths()) {
        std::cout << path.string() << std::endl;
    }
}

void listPluginIds() {
    PluginLoader loader;
    for (auto&& key : loader.listPlugins()) {
        std::cout << key.get() << std::endl;
    }
}

void listPluginOutputs() {
    PluginLoader loader;
    for (auto&& key : loader.listPlugins()) {
        try {
            auto plugin = loader.loadPlugin(key, 48000);
            for (auto&& output : plugin->getOutputDescriptors()) {
                std::cout << key.get() << ':' << output.identifier << std::endl;
            }
        } catch (...) {}
    }
}

void process(std::string_view pluginKey, std::string_view audiofile) {
    // load audio file
    auto file = SndfileHandle(std::string(audiofile));
    if (file.error()) {
        throw std::runtime_error(
            concat("Failed to open audio file: ", audiofile, " (", file.strError(), ")")
        );
    }

    const auto sampleRate = file.samplerate(); 
    const auto channels   = file.channels();

    // load plugin
    PluginLoader loader;
    auto plugin = loader.loadPlugin(pluginKey, sampleRate);

    const size_t preferredBlockSize = plugin->getPreferredBlockSize();
    const size_t blockSize = preferredBlockSize ? preferredBlockSize : 1024;
    const size_t stepSize  = blockSize;

    const bool success = plugin->initialise(stepSize, blockSize);
    if (!success) throw std::runtime_error("Initialisation failed");

    const auto output = plugin->getOutputDescriptors().at(0);  // choose first output

    // print summary
    std::cout << "Audio file:      " << audiofile << '\n';
    std::cout << "- sampling rate: " << sampleRate << '\n';
    std::cout << "- channels:      " << channels << '\n';
    std::cout << "Plugin:          " << pluginKey << '\n';
    std::cout << "- output:        " << output.name << " (" << output.description << ")\n";
    std::cout << "- block size:    " << blockSize << '\n';
    std::cout << "- step size:     " << stepSize << '\n';
    std::cout << '\n';

    if (channels > 1) {
        std::cout << Escape::Yellow
            << "More than one channel provided. Only first channel will be processed!"
            << Escape::Reset << "\n\n";
    }

    // initialise buffer
    const sf_count_t   readSize = blockSize * channels;
    std::vector<float> bufferInterleavedChannels(readSize);
    std::vector<float> bufferChannel(blockSize);

    // process audio block-wise, print timestamps and features
    const uint64_t nsecIncrement = (1'000'000'000 * blockSize) / sampleRate;
    uint64_t       nsec = 0;

    std::cout << "Time [s]\t" << output.name << " [" << output.unit << "]\n";

    while (file.read(bufferInterleavedChannels.data(), readSize) == readSize) {
        // deinterleave channels buffer
        const size_t channel = 0;
        for (size_t i = 0; i < blockSize; ++i) {
            bufferChannel[i] = bufferInterleavedChannels[i * channels + channel];
        }

        auto featureSet = plugin->process(bufferChannel, nsec);

        std::cout << std::fixed << nsec / 1e9 << '\t';
        for (auto&& feature : featureSet[0]) {
            std::cout << feature << '\t';
        }
        std::cout << '\n';

        nsec += nsecIncrement;
    }
    std::cout << std::flush;
}

void usage(std::string_view program) {
    std::cout <<
        "usage: " << program << " [options] [plugin] [audiofile]\n"
        "\n"
        "positional arguments:\n"
        "  plugin          plugin identifier in the form library:plugin (as returned by --list-ids)\n"
        "  audiofile       path to audio file to extract features from\n"
        "\n"
        "options:\n"
        "  --list, -l      list plugin informations in human readable format\n"
        "  --list-paths    list plugin search paths\n"
        "  --list-ids      list plugins in the form library:plugin\n"
        "  --list-outputs  list plugins and outputs in the form library:plugin:output\n"
        "  --help, -h      show help\n"
        << std::flush;
}

int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);

    if (parser.nargs() < 2 || parser.hasFlag("-h") || parser.hasFlag("--help")) {
        usage(parser.program());
        return 2;
    }

    if (parser.hasFlag("-l") || parser.hasFlag("--list")) {
        listPlugins();
        return 0;
    }

    if (parser.hasFlag("--list-paths")) {
        listPaths();
        return 0;
    }

    if (parser.hasFlag("--list-ids")) {
        listPluginIds();
        return 0;
    }

    if (parser.hasFlag("--list-outputs")) {
        listPluginOutputs();
        return 0;
    }

    if (parser.nargs() == 3) {
        const auto plugin    = parser.args()[1];
        const auto audiofile = parser.args()[2];
        try {
            process(plugin, audiofile);
        } catch (const std::exception& e) {
            std::cerr << Escape::Red << "[ERROR] " << e.what() << Escape::Reset << std::endl;
            return 1;
        }
        return 0;
    }

    usage(parser.program());
    return 2;
}
