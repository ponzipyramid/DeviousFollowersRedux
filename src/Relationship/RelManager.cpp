#include "Relationship/RelManager.h"

using namespace DFF;

RelManager& RelManager::GetSingleton() noexcept {
    static RelManager instance;
    return instance;
}

void RelManager::Init() {
    // get rel stage global
    // get deal manager singleton ptr
    // load configs into memory

    std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Relationship");

    for (int i = 1; i < max_stages + 1; i++) {
        std::string filePath = dir + "\\stage" + std::to_string(i) + ".yaml";

        RelStage stage;
        std::ifstream inputFile(filePath);
        if (inputFile.good()) {
            try {
                YAML::Node node = YAML::LoadFile(filePath);
                stage = node.as<RelStage>();
                SKSE::log::info("RelManager::Init: num lock in = {}", stage.GetLockInRules().size());
            }
            catch (std::exception e) {
                SKSE::log::info("RelManager::Init: Failed to read stage {} because {}", i, e.what());
            }
        }
        else {
            SKSE::log::info("Failed to read stage {}", i);
        }

        SKSE::log::info("Adding stage {}", stage.GetName());
        stages.push_back(stage);
    }

    int i = 0;
    for (auto& stage : stages) {
        SKSE::log::info("Stage {} = {}", i, stage.GetName());
        i++;
    }

    flowQuest = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(0xD62, "DeviousFollowers.esp");
    auto questStage = flowQuest->GetCurrentStageID();
    this->currentStageIndex = 0;
    if (questStage == 80) {
        this->currentStageIndex = 4;
    }
}