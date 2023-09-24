#include <SKSE/SKSE.h>
#include <DFF/DealManager.h>
#include <Config.h>
#include <DFF/Deal.h>
#include <articuno/archives/ryml/ryml.h>
#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <stdlib.h>
#include "UI.h"

using namespace RE;
using namespace DFF;
using namespace SKSE;
using namespace articuno::ryml;

namespace {
    inline const auto ActiveDealsRecord = _byteswap_ulong('ACTD');

    int PickRandom(int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, max - 1);
        return distr(gen);
    }

    int PickRandom(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(min, max - 1);
        return distr(gen);
    }

}
DealManager& DealManager::GetSingleton() noexcept {
    static DealManager instance;
    return instance;
}

bool IsDirValid(std::string dir) {
    bool valid = true;
    struct stat info;
    const char* cstr = dir.c_str();
    if (stat(cstr, &info) != 0) {
        return false;
    } else if (!(info.st_mode & S_IFDIR)) {
        return false;
    }

    return true;
}

void DealManager::InitDeals() {
    std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Addons");
   
    if (!IsDirValid(dir)) {
        log::info("DFR Directory not valid");
        return;
    }

    std::string ext1(".yaml");
    std::string ext2(".yml");

    std::vector<std::string> dealIds;
    std::vector<std::string> ruleIds;

    for (const auto& a : std::filesystem::directory_iterator(dir)) {

        if (!std::filesystem::is_directory(a)) {
            continue;
        }

        std::string dirName = a.path().stem().string();
        std::string groupName = dir + "\\" + dirName;

        log::info("Initializing add-on {}", dirName);

        std::string rulesDir = groupName + "\\Rules";

        if (IsDirValid(rulesDir)) {
            for (const auto& p : std::filesystem::directory_iterator(rulesDir)) {
                auto fileName = p.path();

                if (fileName.extension() == ext1 || fileName.extension() == ext2) {
                    const auto name = fileName.stem().string();

                    Rule entity(dirName, name);
                    bool issue = false;

                    try {
                        std::ifstream inputFile(fileName.string());
                        if (inputFile.good()) {
                            yaml_source ar(inputFile);

                            ar >> entity;
                            rules[entity.GetFullName()] = entity;
                            ruleIds.push_back(entity.GetFullName());
                            ruleGroups[dirName].push_back(&rules[entity.GetFullName()]);
                            log::info("Registered Rule {}", entity.GetName());

                        } else
                            log::error("Error: Failed to read file");
                    } catch (const std::exception& e) {
                        issue = true;
                        log::error("Error: {}", e.what());
                    }

                    if (issue) log::error("Failed to register rule: {}", entity.GetName());
                }
            }
        } else {
            log::info("No rules found");
        }
    }
    Rule* r1;
    Rule* r2;


    for (int i = 0; i < ruleIds.size(); i++) {
        r1 = &rules[ruleIds[i]];
        conflicts[r1].insert(r1);
        for (int j = i + 1; j < ruleIds.size(); j++) {
            r2 = &rules[ruleIds[j]];
            if (r1->ConflictsWith(r2)) {
                log::info("Rule-Rule Conflict: {} and {}", r1->GetFullName(), r2->GetFullName());
                conflicts[r1].insert(r2);
                conflicts[r2].insert(r1);
            }
        }
    }

    log::info("Finished initializing. Registered {} rules.", rules.size());
}

void DealManager::InitQuests() {
    std::vector<std::string> toRemove;

    // TODO: Check if rules' quest exists and property exists within it

    for (auto& [key, rule] : rules) {
        rule.Init();
    }
}

void DealManager::InitQuestData() {
    // TODO: Iterate through all rule properties to figure out which ones are active
}

int DealManager::SelectDeal(int lastRejectedId) {

    /*
    TODO:
        - get all non-conflicting rules
        - select one at random
        - generate id simply by iterating over map and picking next slot
        - return rule id
    */ 

    return 0;
}

void DealManager::ActivateRule(int id) {
    /*
    TODO:
        - use id map to get associate rule object
        - add to active rules
    */
}

void DealManager::RemoveDeal(std::string name) {
    /*
    TODO:
        - use id map to get associate rule object
        - remove from active rules
    */
}

std::vector<std::string> DealManager::GetActiveDeals() { 
    std::vector<std::string> active;

    for (auto& [name, _] : activeDeals) {
        active.push_back(name);
    }

    return active;
}

std::vector<std::string> DealManager::GetDealRules(std::string name) {
    Deal* deal = &activeDeals[name];

    std::vector<std::string> ruleNames;

    if (deal) {
        std::vector<Rule*>& rules = deal->rules;

        if (!rules.empty()) {
            ruleNames.reserve(rules.size());
            for (auto& key : rules) ruleNames.push_back(key->GetName());
        }
    }

    return ruleNames;
}

std::vector<std::string> DealManager::GetGroupNames() { 
    std::vector<std::string> groupNames; 

    for (auto& [name, deals] : ruleGroups) {
        groupNames.push_back(name);
    }

    return groupNames;
}

std::vector<std::string> DealManager::GetGroupRules(std::string groupName) { 
    std::vector<Rule*>& rules = ruleGroups[groupName];

    std::vector<std::string> ruleNames;

    if (!rules.empty()) {
        ruleNames.reserve(rules.size());
        for (auto& key : rules) ruleNames.push_back(key->GetName());
    }

    return ruleNames;
}

void DealManager::ShowBuyoutMenu() {
    std::string msg = "Buyout: Select a Deal";
    std::vector<std::string> options; 
    std::vector<int> costs;

    for (auto& [name, deal] : activeDeals) {
        auto cost = deal.GetDealCost();
        costs.push_back(cost);
        
        options.push_back(std::format("{} [{}]", name, cost));
    }

    options.push_back("Cancel");

    SKSE::log::info("Showing message box");


    menuChosen = false;

    TESMessageBox::Show(msg, options, [costs](int result) {
        SKSE::log::info("Selected {}", result);
        int val = -1;

        if (result < costs.size())
            val = costs[result];
        else
            SKSE::log::info("User cancelled buyout");

        DealManager::GetSingleton().chosenCost = val;
        DealManager::GetSingleton().menuChosen = true;
    });
}

int DealManager::GetBuyoutMenuResult() {
    DealManager::GetSingleton().menuChosen = false;
    return chosenCost;
}

void DealManager::OnRevert(SerializationInterface*) {
    std::unique_lock lock(GetSingleton()._lock);
    GetSingleton().id_map.clear();
    GetSingleton().activeDeals.clear();
}

std::string ReadString(SerializationInterface* serde) {
    std::size_t nameSize;
    serde->ReadRecordData(&nameSize, sizeof(nameSize));

    std::string name;
    name.reserve(nameSize);

    char c;
    for (int i = 0; i < nameSize; i++) {
        serde->ReadRecordData(&c, sizeof(c));
        name += c;
    }
    return name;
}

void WriteString(SerializationInterface* serde, std::string name) {
    std::size_t nameSize = name.size();
    serde->WriteRecordData(&nameSize, sizeof(nameSize));

    char c;
    for (int i = 0; i < nameSize; i++) {
        c = name[i];
        serde->WriteRecordData(&c, sizeof(c));
    }
}

std::string DealManager::GetNextDealName() {
    for (auto& name : allDealNames)
        if (!activeDeals.count(name)) return name;

    SKSE::log::error("GetNextDealName - Failed to find open name");
    return "";
}

void DealManager::OnGameSaved(SerializationInterface* serde) {
    std::unique_lock lock(GetSingleton()._lock);
    if (!serde->OpenRecord(ActiveDealsRecord, 0)) {
        log::error("Unable to open record to write cosave data.");
        return;
    }

    // save deal to id mapping
    auto mapSize = GetSingleton().id_map.size();
    serde->WriteRecordData(&mapSize, sizeof(mapSize));
    for (auto& count : GetSingleton().id_map) {
        WriteString(serde, count.second);
        int maxStage = count.first;

        log::info("Saved deal mapping {} -> {}", count.second, count.first);

        serde->WriteRecordData(&maxStage, sizeof(maxStage));
    }

    // save active rules for each modular deal
    auto ruleSize = GetSingleton().activeDeals.size();
    serde->WriteRecordData(&ruleSize, sizeof(ruleSize));
    for (auto& [name, deal] : GetSingleton().activeDeals) {
        WriteString(serde, deal.GetName());
        auto ruleCount = deal.rules.size();
        serde->WriteRecordData(&ruleCount, sizeof(ruleCount));
        for (Rule* rule : deal.rules) {
            WriteString(serde, rule->GetFullName());
        }
    }
}

void DealManager::OnGameLoaded(SerializationInterface* serde) {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t version;

    while (serde->GetNextRecordInfo(type, version, size)) {
        if (type == ActiveDealsRecord) {
            GetSingleton().id_map.clear();

            // load in deal id mappings
            std::size_t mapSize;
            serde->ReadRecordData(&mapSize, sizeof(mapSize));
            for (; mapSize > 0; --mapSize) {
                std::string name = ReadString(serde);

                int id;
                serde->ReadRecordData(&id, sizeof(id));

                GetSingleton().id_map[id] = name;
            }

            // load in active rules for modular deals
            std::size_t ruleSize;
            serde->ReadRecordData(&ruleSize, sizeof(ruleSize));
            for (; ruleSize > 0; --ruleSize) {
                std::string name = ReadString(serde);

                Deal deal(name);
                GetSingleton().activeDeals[name] = deal;

                std::size_t ruleCount;
                serde->ReadRecordData(&ruleCount, sizeof(ruleCount));
                for (; ruleCount > 0; --ruleCount) {
                    std::string ruleName = ReadString(serde);
                    GetSingleton().activeDeals[name].rules.push_back(&GetSingleton().rules[ruleName]);
                }
            }
        }
    }
}