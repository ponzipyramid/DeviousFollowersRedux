#include <SKSE/SKSE.h>
#include <DFF/DealManager.h>
#include <DFF/Deal.h>
#include <Config.h>
#include "UI.hpp"
#include "Serialization.hpp"

#include <articuno/archives/ryml/ryml.h>

#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

using namespace RE;
using namespace DFF;
using namespace SKSE;
using namespace articuno::ryml;

namespace {
    inline const auto ActiveDealsRecord = _byteswap_ulong('ACTD');

    int PickRandom(int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, max);
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
        log::info("InitDeals: DFR Directory not valid");
        return;
    }

    std::string ext1(".yaml");
    std::string ext2(".yml");

    for (const auto& a : std::filesystem::directory_iterator(dir)) {

        if (!std::filesystem::is_directory(a)) {
            continue;
        }

        std::string dirName = a.path().stem().string();
        std::string groupName = dir + "\\" + dirName;

        log::info("InitDeals: Initializing add-on {}", dirName);

        std::string rulesDir = groupName + "\\Rules";

        if (IsDirValid(rulesDir)) {
            for (const auto& p : std::filesystem::directory_iterator(rulesDir)) {
                auto fileName = p.path();

                if (fileName.extension() == ext1 || fileName.extension() == ext2) {
                    const auto name = fileName.stem().string();

                    Rule entity(dirName + "/" + name);
                    bool issue = false;

                    try {
                        std::ifstream inputFile(fileName.string());
                        if (inputFile.good()) {
                            yaml_source ar(inputFile);

                            ar >> entity;
                            rules[entity.GetPath()] = entity;
                            ruleGroups[dirName].push_back(&rules[entity.GetPath()]);
                            log::info("InitDeals: Registered Rule {}", entity.GetName());

                        } else
                            log::error("InitDeals: Error - Failed to read file");
                    } catch (const std::exception& e) {
                        issue = true;
                        log::error("InitDeals: Error - {}", e.what());
                    }

                    if (issue) log::error("InitDeals: Failed to register rule: {}", entity.GetName());
                }
            }
        } else {
            log::info("InitDeals: No rules found");
        }
    }

    log::info("InitDeals: Finished initializing. Registered {} rules.", rules.size());
}

void DealManager::InitQuests() {
    std::vector<std::string> toRemove;

    std::vector<std::string> ruleIds;
    for (auto& [key, rule] : rules) {
        rule.Init();
        if (rule.IsValid()) {
            ruleIds.push_back(rule.GetPath());     
        } else {
            toRemove.push_back(key);
        }
    }

    for (auto remove : toRemove) rules.erase(remove);

    Rule* r1 = nullptr;
    Rule* r2 = nullptr;

    for (int i = 0; i < ruleIds.size(); i++) {
        r1 = &rules[ruleIds[i]];
        conflicts[r1].insert(r1);
        for (int j = i + 1; j < ruleIds.size(); j++) {
            r2 = &rules[ruleIds[j]];
            if (r1->ConflictsWith(r2)) {
                SKSE::log::info("InitQuests: Conflict - {} and {}", r1->GetPath(), r2->GetPath());
                conflicts[r1].insert(r2);
                conflicts[r2].insert(r1);
            }
        }
    }
}

int DealManager::SelectDeal(int lastRejectedId) {
    std::string lastRejected = id_map[lastRejectedId];

    std::vector<Rule*> candidateRules;

    for (auto& [name, rule] : rules) {
        if (name == lastRejected) continue;
        for (auto& [_, deal] : deals) {
            for (auto activeRule : deal.rules) {
                // TODO: filter on severity
                if (!conflicts[activeRule].contains(&rule) && rule.IsEnabled()) {
                    candidateRules.push_back(&rule);
                }
            }
        }
    }

    int index = PickRandom(candidateRules.size());
    if (index == candidateRules.size()) return 0;
    else {
        auto rule = candidateRules[index];
        auto id = 1;
        auto path = rule->GetPath();

        if (name_map.count(path)) {
            id = name_map[path];
        } else {
            while (id_map.count(id)) {
                id += 1;
            }
        }

        id_map[id] = path;
        name_map[path] = id;

        return id;
    }
}

int DealManager::ActivateRule(int id) {
    if (id == 0) {
        ExtendDeal(GetRandomDeal(), 0.0f);
    }
    return ActivateRule(id_map[id]); 
}

int DealManager::ActivateRule(std::string ruleName) {
    if (ruleName.empty()) {
        SKSE::log::error("ActivateRule: Invalid id given");
        return -1;
    } else {
        std::vector<Deal*> candidateDeals;
        for (auto& [name, deal] : deals) {
            if (deal.IsOpen()) {
                candidateDeals.push_back(&deal);
            }
        }

        Deal* chosen;
        if (candidateDeals.empty()) { // create new one if none are open
            Deal deal(GetNextDealName());
            deals[deal.GetName()] = deal;
            chosen = &deals[deal.GetName()];
        } else {
            int index = PickRandom(candidateDeals.size());
            chosen = candidateDeals[index];
        }

        chosen->UpdateTimer();

        chosen->rules.push_back(&rules[ruleName]);
        return chosen->rules.size();
    }
}

void DealManager::RemoveDeal(std::string name) {
    if (auto deal = &deals[name]) {
        for (auto& rule : deal->rules) {
            if (rule->IsEnabled())
                rule->GetGlobal()->value = 1; // reset to 1 only if enabled or running
        }
    }

    deals.erase(name);
}

void DealManager::ResetAllDeals() {
    for (auto [_, deal] : deals) {
        for (auto& rule : deal.rules) {
            if (rule->IsEnabled()) rule->GetGlobal()->value = 1;  // reset to 1 only if enabled or running
        }
    }

    deals.clear();
}

void DealManager::Pause() {
    // TODO: cache everything and then reset everything
}

void DealManager::Resume() {
    // TODO: Read and apply cached data
}

void DealManager::ExtendDeal(std::string name, double by) {
    if (auto deal = &deals[name]) {
        deal->Extend(by);
    }
}

std::string DealManager::GetRandomDeal() {
    std::vector<std::string> activeDeals;
    for (auto [name, _] : deals) activeDeals.push_back(name);
    return activeDeals[PickRandom(activeDeals.size() - 1)];
}

RE::TESGlobal* DealManager::GetRuleGlobal(int id) {
    auto name = id_map[id];
    if (Rule* rule = &rules[name])
        return rule->GetGlobal();
    else
        return nullptr;
}

int DealManager::GetDealCost(std::string name) {
    if (auto deal = &deals[name]) {
        return deal->GetCost();
    } else {
        return 0;
    }
}

double DealManager::GetExpensiveDebtCount() {
    // TODO: replace with debt calc
    double expensiveDebtCount = 0.0f;
    
    for (auto& [_, deal] : deals) {
        int multiplier = 1;
        for (auto& rule : deal.rules) {
            if (rule->GetPath() == "devious followers continued/expensive") {
                expensiveDebtCount += multiplier;
            }
            multiplier *= 2;
        }
    }

    return expensiveDebtCount; 
}

std::vector<std::string> DealManager::GetDeals() { 
    std::vector<std::string> active;

    for (auto& [name, _] : deals) {
        active.push_back(name);
    }

    return active;
}

std::vector<std::string> DealManager::GetDealRules(std::string name) {
    Deal* deal = &deals[name];

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

std::vector<std::string> DealManager::GetEnslavementRules() {
    std::vector<std::string> ruleNames;

    // TODO: populate enslavement rules list

    return ruleNames;
}


void DealManager::ShowBuyoutMenu() {
    std::string msg = "Buyout: Select a Deal";
    std::vector<std::string> options; 
    std::vector<std::string> dealNames;

    for (auto& [name, deal] : deals) {
        auto cost = deal.GetCost();
        dealNames.push_back(deal.GetName());
        
        options.push_back(std::format("{} [{}]", name, cost));
    }

    options.push_back("Cancel");

    SKSE::log::info("Showing message box");


    menuChosen = false;

    TESMessageBox::Show(msg, options, [dealNames](int result) {
        SKSE::log::info("Selected {}", result);
        std::string val;

        if (result < dealNames.size())
            val = dealNames[result];
        else
            SKSE::log::info("User cancelled buyout");

        DealManager::GetSingleton().chosenDeal = val;
        DealManager::GetSingleton().menuChosen = true;
    });
}

std::string DealManager::GetBuyoutMenuResult() {
    DealManager::GetSingleton().menuChosen = false;
    return chosenDeal;
}

std::string DealManager::GetNextDealName() {
    for (auto& name : allDealNames)
        if (!deals.count(name)) return name;

    SKSE::log::error("GetNextDealName - Failed to find open name");
    return "";
}

void DealManager::OnRevert(SerializationInterface*) {
    std::unique_lock lock(GetSingleton()._lock);
    GetSingleton().id_map.clear();
    GetSingleton().name_map.clear();
    GetSingleton().deals.clear();
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
        Serialization::Write<std::string>(serde, count.second);
        int maxStage = count.first;

        log::info("Saved deal mapping {} -> {}", count.second, count.first);

        serde->WriteRecordData(&maxStage, sizeof(maxStage));
    }

    // save active rules for each modular deal
    auto dealSize = GetSingleton().deals.size();
    serde->WriteRecordData(&dealSize, sizeof(dealSize));
    for (auto& [name, deal] : GetSingleton().deals) {
        deal.Serialize(serde);
    }
}

void DealManager::OnGameLoaded(SerializationInterface* serde) {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t version;

    while (serde->GetNextRecordInfo(type, version, size)) {
        if (type == ActiveDealsRecord) {
            GetSingleton().id_map.clear();

            // load deal id mappings
            std::size_t mapSize;
            serde->ReadRecordData(&mapSize, sizeof(mapSize));
            for (; mapSize > 0; --mapSize) {
                std::string name = Serialization::Read<std::string>(serde);

                int id;
                serde->ReadRecordData(&id, sizeof(id));

                GetSingleton().id_map[id] = name;
                GetSingleton().name_map[name] = id;
            }

            // load active deals
            std::size_t numDeals;
            serde->ReadRecordData(&numDeals, sizeof(numDeals));
            for (; numDeals > 0; --numDeals) {
                Deal deal(serde);
                GetSingleton().deals[deal.GetName()] = deal;
            }
        }
    }
}