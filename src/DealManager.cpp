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
    inline const auto ExtendRulePath = "devious followers/extend";

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

void DealManager::Init() {
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();
    std::vector<std::string> ruleIds;

    std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Addons");

    if (!IsDirValid(dir)) {
        log::info("Init: DFR Directory not valid");
        return;
    }

    std::string ext1(".yaml");
    std::string ext2(".yml");

    for (const auto& a : std::filesystem::directory_iterator(dir)) {
        if (!std::filesystem::is_directory(a)) {
            continue;
        }

        std::string packName = a.path().stem().string();

        std::string configFile = dir + "\\" + packName + "\\config.yaml";

        std::string packId = Lowercase(packName);

        Pack pack(packName);

        std::ifstream inputFile(configFile);
        if (inputFile.good()) {
            try {
                log::info("Init: Initializing add-on {}", packName);
                yaml_source ar(inputFile);
                ar >> pack;

                if (pack.Init(handler)) {
                    log::info("Init: Registered Pack {}", pack.GetName());
                    packs[packId] = pack; 
                } else {
                    continue;
                }
            } catch (...) {
                SKSE::log::info("Init: Failed to register {}", packName);
            }
        } else {
            SKSE::log::info("Init: Failed to register {}", packName);
            continue;
        }

        std::string rulesDir = dir + "\\" + packName + "\\Rules";

        if (IsDirValid(rulesDir)) {
            for (const auto& p : std::filesystem::directory_iterator(rulesDir)) {
                auto fileName = p.path();

                if (fileName.extension() == ext1 || fileName.extension() == ext2) {
                    const auto name = fileName.stem().string();

                    Rule rule(&packs[packId], name);
                    bool issue = false;

                    try {
                        std::ifstream inputFile(fileName.string());
                        if (inputFile.good()) {
                            yaml_source ar(inputFile);

                            ar >> rule;

                            if (rule.Init(handler)) {
                                log::info("Init: Registered Rule {}", rule.GetName());
                                rules[rule.GetId()] = rule;
                                packs[packId].AddRule(&rules[rule.GetId()]);
                            }
                        } else
                            log::error("Init Error - Failed to read file");
                    } catch (const std::exception& e) {
                        issue = true;
                        log::error("Init: Error - {}", e.what());
                    }

                    if (issue) log::error("Init: Failed to register rule: {}", rule.GetName());
                }
            }
        } else {
            log::info("Init: No rules found");
        }
    }

    log::info("Init: Registered {} pack(s)", packs.size());
    log::info("Init: Registered {} rule(s)", rules.size());

    Rule* r1 = nullptr;
    Rule* r2 = nullptr;

    for (int i = 0; i < ruleIds.size(); i++) {
        r1 = &rules[ruleIds[i]];
        conflicts[r1].insert(r1);
        for (int j = i + 1; j < ruleIds.size(); j++) {
            r2 = &rules[ruleIds[j]];
            if (r1->ConflictsWith(r2)) {
                log::info("Init: Conflict - {} and {}", r1->GetId(), r2->GetId());
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

    return rule->GetId();
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

        rule->Activate();
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
            if (rule->IsActive()) rule->Enable();
        }
    }

    deals.erase(name);
}

void DealManager::ResetAllDeals() {
    for (auto [_, deal] : deals) {
        for (auto& rule : deal.rules) {
            if (rule->IsActive()) rule->Enable();
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

std::string DealManager::GetRuleHint(std::string path) {
    log::info("GetRuleHint: {}", path);

    if (Rule* rule = GetRuleByPath(path)) {
        log::info("GetRuleHint: {} found", path);

        return rule->GetHint();
    } else
        return "";
}

std::string DealManager::GetRuleInfo(std::string path) {
    if (Rule* rule = GetRuleByPath(path)) {
        return rule->GetInfo();
    } else
        return "";
}

std::string DealManager::GetRulePack(std::string path) {
    if (Rule* rule = GetRuleByPath(path)) {
        return rule->GetPack()->GetName();
    } else
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
            for (auto& rule : rules) ruleNames.push_back(rule->GetId());
        } else {
            log::info("GetDealRules: no rules in deal {}", name);
        }
    } else {
        log::error("GetDealRules: no deal found {}", name);
    }

    return ruleNames;
}

std::vector<std::string> DealManager::GetPackNames() { 
    std::vector<std::string> packNames; 

    for (auto& [name, deals] : packs) {
        packNames.push_back(name);
    }

    return packNames;
}

std::vector<std::string> DealManager::GetPackRules(std::string name) {
    log::info("GetPackRules: getting rules for pack {}", name);
    std::vector<std::string> ruleNames;
        
    if (auto pack = GetPackByName(name)) {
        log::info("GetPackRules: fetched pack");
        auto& rules = pack->GetRules();
        for (auto& rule : rules) ruleNames.push_back(rule->GetId());
    } else {
        log::info("GetPackRules: could not find pack");
    }

    return ruleNames;
}

RE::TESQuest* DealManager::GetPackQuest(std::string name) {
    log::info("GetPackQuest: getting quest for {}", name);
    
    if (auto pack = GetPackByName(name)) {
        log::info("GetPackQuest: quest = {}", pack->GetQuest() != nullptr);
        return pack->GetQuest();
    } else {
        return nullptr;
    }
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

void DealManager::SetRuleValid(std::string path, bool valid) {
    SKSE::log::info("SetRuleValid: {} {}", path, valid);
    if (auto rule = GetRuleByPath(path)) {
        rule->valid = valid;
        if (!valid) rule->Disable();
    }else
        SKSE::log::info("SetRuleValid: could not set valid for {}", path);
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

Pack* DealManager::GetPackByName(std::string name) {
    name = Lowercase(name);
    return packs.count(name) ? &packs[name] : nullptr;
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