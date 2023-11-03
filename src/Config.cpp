#include "Config.h"
#include <articuno/archives/ryml/ryml.h>

using namespace DFF;
using namespace SKSE;

const Config& Config::GetSingleton() noexcept {
    static Config instance;

    static std::atomic_bool initialized;
    static std::latch latch(1);
    if (!initialized.exchange(true)) {
        std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Config");
        std::string configDefaultFile = dir + "\\preferences.default.yaml";
        std::string configFile = dir + "\\preferences.yaml";

        if (!std::filesystem::exists(configDefaultFile)) {
            log::error("Error: preferences default file doesn't exist");
            return instance;
        }

        if (!std::filesystem::exists(configFile) && !std::filesystem::copy_file(configDefaultFile, configFile)) {
            log::error("Error: failed to generate preferences file");
            return instance;
        }

        std::ifstream inputFile(configFile);
        if (inputFile.good()) {
            articuno::ryml::yaml_source ar(inputFile);
            ar >> instance;
        }
        latch.count_down();
    }
    latch.wait();

    return instance;
}
