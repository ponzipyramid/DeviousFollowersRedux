#include <SKSE/SKSE.h>
#include <DFF/DealManager.h>
#include <DFF/Deal.h>
#include "DFF/Settings.h"
#include <Config.h>
#include "UI.hpp"
#include "Serialization.hpp"

#include <articuno/archives/ryml/ryml.h>

#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

using namespace DFF;
using namespace SKSE;
using namespace articuno::ryml;

namespace {
    inline const auto ActiveDealsRecord = _byteswap_ulong('ACTD');
    inline const auto ExtendRulePath = "devious followers continued/extend";

    int PickRandom(int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, max);
        return distr(gen);
    }

    std::string Lowercase(std::string in) {
        std::string out = in;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
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

    log::info("InitDeals: Registered {} rules", rules.size());
}

void DealManager::InitQuests() {
    std::vector<std::string> toRemove;

    std::vector<std::string> ruleIds;
    for (auto& [key, rule] : rules) {
        rule.Init();
        if (rule.IsValid()) {
            log::info("InitQuests: Rule {} is valid", rule.GetName());
            ruleIds.push_back(rule.GetPath());     
        } else {
            log::info("InitQuests: Rule {} is invalid", rule.GetName());
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
                log::info("InitQuests: Conflict - {} and {}", r1->GetPath(), r2->GetPath());
                conflicts[r1].insert(r2);
                conflicts[r2].insert(r1);
            }
        }
    }
}

std::string DealManager::SelectRule(std::string lastRejected) {
    std::vector<Rule*> candidateRules;

    Rule* lastRejectedRule = nullptr;
    Rule* extendRule = nullptr;

    for (auto& [path, rule] : rules) {
        if (path == ExtendRulePath) {
            extendRule = &rule;
            continue;
        }

        if (path == lastRejected) {
            lastRejectedRule = &rule;
            continue;
        }

        if (!rule.IsEnabled()) {
            continue; 
        }

        // TODO: filter on severity

        bool compatible = true;
        for (auto& [_, deal] : deals) {
            for (auto activeRule : deal.rules) {

                if (conflicts[activeRule].contains(&rule)) {
                    compatible = false;
                }
            }
        }

        if (compatible) candidateRules.push_back(&rule);
    }

    if (candidateRules.size() == 1 && lastRejectedRule) candidateRules.push_back(lastRejectedRule);

    if (deals.size() > 0 || candidateRules.empty()) candidateRules.push_back(extendRule); 

    int index = PickRandom(candidateRules.size() - 1);
    auto rule = candidateRules[index];

    SKSE::log::info("SelectRule: Selected {}", rule->GetName());

    return rule->GetPath();
}

bool DealManager::CanEnableRule(std::string ruleName) {
    if (auto rule = GetRuleByPath(ruleName)) {
        return rule->CanEnable();
    }
    return false;
}

bool DealManager::CanDisableRule(std::string ruleName) {
    if (auto rule = GetRuleByPath(ruleName)) {
        return rule->CanDisable();
    }
    return false;
}

int DealManager::ActivateRule(std::string path) {
    if (path == ExtendRulePath) {
        log::info("ActivateRule: extending deal");
        std::vector<std::string> activeDeals;
        for (auto& [name, deal] : deals) {
            activeDeals.push_back(name);
        }

        if (!activeDeals.empty())
            ExtendDeal(activeDeals[PickRandom(activeDeals.size() - 1)], 0.0f);

        return -1;
    }

    if (auto rule = GetRuleByPath(path)) {
        log::info("ActivateRule: activating {}", path);

        std::vector<Deal*> candidateDeals;
        for (auto& [name, deal] : deals) {
            if (deal.IsOpen()) {
                candidateDeals.push_back(&deal);
            }
        }

        Deal* chosen;
        if (candidateDeals.empty()) { 
            // create new one if none are open
            auto name = GetNextDealName();
            Deal deal(name);

            auto key = Lowercase(name);

            deals[key] = deal;
            chosen = GetDealByName(key);
        } else {
            int index = PickRandom(candidateDeals.size());
            chosen = candidateDeals[index];
        }

        auto global = rule->GetGlobal();
        global->value = 3;
        chosen->UpdateTimer();

        chosen->rules.push_back(rule);
        return chosen->rules.size();
    } else {
        log::error("ActivateRule: Invalid id {} given", path);
        return -1;
    }
}


void DealManager::RemoveDeal(std::string name) {
    if (auto deal = GetDealByName(name)) {
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
    if (auto deal = GetDealByName(name)) {
        deal->Extend(by);
    }
}

std::string DealManager::GetRandomDeal() {
    std::vector<std::string> activeDeals;
    for (auto [name, _] : deals) activeDeals.push_back(name);
    return activeDeals[PickRandom(activeDeals.size() - 1)];
}

RE::TESGlobal* DealManager::GetRuleGlobal(std::string path) {
    if (Rule* rule = GetRuleByPath(path))
        return rule->GetGlobal();
    else
        return nullptr;
}

std::string DealManager::GetRuleName(std::string path) {
    if (Rule* rule = GetRuleByPath(path))
        return rule->GetName();
    else
        return "";
}

std::string DealManager::GetRuleDesc(std::string path) {
    if (Rule* rule = GetRuleByPath(path))
        return rule->GetDesc();
    else
        return "";
}

std::string DealManager::GetRuleHint(std::string path) {
    if (Rule* rule = GetRuleByPath(path))
        return rule->GetHint();
    else
        return "";
}

int DealManager::GetDealCost(std::string name) {
    if (auto deal = GetDealByName(name)) {
        return deal->GetCost();
    } else {
        return 0;
    }
}

std::vector<std::string> DealManager::GetDeals() { 
    std::vector<std::string> active;

    for (auto& [name, _] : deals) {
        active.push_back(name);
    }

    log::info("GetDeals: Num deals = {}", active.size());

    return active;
}

std::vector<std::string> DealManager::GetDealRules(std::string name) {
    Deal* deal = GetDealByName(name);

    std::vector<std::string> ruleNames;

    if (deal) {
        std::vector<Rule*>& rules = deal->rules;

        if (!rules.empty()) {
            ruleNames.reserve(rules.size());
            for (auto& rule : rules) ruleNames.push_back(rule->GetPath());
        } else {
            log::info("GetDealRules: no rules in deal {}", name);
        }
    } else {
        log::error("GetDealRules: no deal found {}", name);
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
        for (auto& key : rules) ruleNames.push_back(key->GetPath());
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

    log::info("Showing message box");


    menuChosen = false;

    TESMessageBox::Show(msg, options, [dealNames](int result) {
        log::info("Selected {}", result);
        std::string val;

        if (result < dealNames.size())
            val = dealNames[result];
        else
            log::info("User cancelled buyout");

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

    log::error("GetNextDealName - Failed to find open name");
    return "";
}

Rule* DealManager::GetRuleByPath(std::string path) { 
    path = Lowercase(path);
    return rules.count(path) ? &rules[path] : nullptr; 
}

Deal* DealManager::GetDealByName(std::string name) {
    name = Lowercase(name);
    return deals.count(name) ? &deals[name] : nullptr; 
}


void DealManager::OnRevert(SerializationInterface*) {
    std::unique_lock lock(GetSingleton()._lock);
    GetSingleton().deals.clear();
}

void DealManager::OnGameSaved(SerializationInterface* serde) {
    std::unique_lock lock(GetSingleton()._lock);
    if (!serde->OpenRecord(ActiveDealsRecord, 0)) {
        log::error("Unable to open record to write cosave data.");
        return;
    }

    // save active rules for each modular deal
    auto dealSize = GetSingleton().deals.size();
    serde->WriteRecordData(&dealSize, sizeof(dealSize));
    for (auto& [name, deal] : GetSingleton().deals) {
        log::info("OnGameSaved: Serializing {}", name);
        deal.Serialize(serde);
    }
}

void DealManager::OnGameLoaded(SerializationInterface* serde) {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t version;

    while (serde->GetNextRecordInfo(type, version, size)) {
        if (type == ActiveDealsRecord) {
            // load active deals
            std::size_t numDeals;
            serde->ReadRecordData(&numDeals, sizeof(numDeals));
            for (; numDeals > 0; --numDeals) {
                Deal deal(serde);
                log::info("OnGameLoaded: Deserializing {}", deal.GetName());
                auto key = Lowercase(deal.GetName());
                GetSingleton().deals[key] = deal;
            }
        }
    }
}