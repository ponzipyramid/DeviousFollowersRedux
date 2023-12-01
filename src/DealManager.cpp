#include <SKSE/SKSE.h>
#include <DFF/DealManager.h>
#include <DFF/Deal.h>
#include <DFF/RulePath.h>
#include "DFF/Settings.h"
#include <Config.h>
#include "UI.hpp"
#include "Serialization.hpp"
#include <algorithm>
#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

using namespace DFF;
using namespace SKSE;

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
    std::vector<RulePath> paths;
    std::unordered_map<RE::FormID, std::string> seenGlobals;

    SKSE::log::info("Config {}", Config::GetSingleton().GetBaseScore());

    std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Packs");

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

        auto packName = a.path().stem().string();
        auto packId = Lowercase(packName);

        try {

            log::info("Init: Initializing add-on {}", packName);

            auto packFile = YAML::LoadFile((a.path() / "config.yaml").string());
            auto pack = Pack(packName, packFile);
            
            if (pack.Init(handler)) {
                log::info("Init: Registered Pack {} made by {}", pack.GetName(), pack.GetAuthor());
                packs[packId] = pack;
            }
            else {
                continue;
            }
        }
        catch (...) {
            SKSE::log::info("Init: Failed to register {}", packName);
        }

        std::string rulesDir = dir + "\\" + packName + "\\Rules";
        if (IsDirValid(rulesDir)) {
            for (const auto& p : std::filesystem::directory_iterator(rulesDir)) {
                auto filePath = p.path();

                if (filePath.extension() == ext1 || filePath.extension() == ext2) {
                    const auto name = filePath.stem().string();

                    try {
                        std::ifstream inputFile(filePath.string());
                        if (inputFile.good()) {
                            auto ruleFile = YAML::LoadFile(filePath.string());

                            Rule rule(&packs[packId], name, ruleFile);

                            if (rule.Init(handler)) {
                                auto globalId = rule.GetGlobal()->GetFormID();
                                
                                if (seenGlobals.count(globalId)) {
                                    log::info("Init: duplicate global found {} and {}", rule.GetId(),
                                              seenGlobals[globalId]);
                                    continue;
                                }

                                seenGlobals[globalId] = rule.GetId();

                                log::info("Init: Registered Rule {}", rule.GetName());
                                rules[rule.GetId()] = rule;
                                packs[packId].AddRule(&rules[rule.GetId()]);
                                ruleIds.push_back(rule.GetId());
                            } else {
                                log::info("Init: Rule {} is invalid", rule.GetName());
                            }
                        } else
                            log::error("Init Error - Failed to read rule file");
                    } catch (const std::exception& e) {
                        log::error("Init: Rule {} Registration Error - {}", name, e.what());
                    }
                }
            }
        } else {
            log::info("Init: No rules found");
        }

        std::string pathsDir = dir + "\\" + packName + "\\Paths";
        if (IsDirValid(pathsDir)) {
            for (const auto& p : std::filesystem::directory_iterator(pathsDir)) {
                auto filePath = p.path();

                if (filePath.extension() == ext1 || filePath.extension() == ext2) {
                    const auto name = filePath.stem().string();

                    try {
                        std::ifstream inputFile(filePath.string());
                        if (inputFile.good()) {
                            auto pathFile = YAML::LoadFile(filePath.string());
                            RulePath path(name, pathFile);
                            if (!path.GetRuleIds().empty()) {
                                log::info("Init: Registered Path {}", path.GetName());
                                paths.push_back(path);
                            }
                        } else
                            log::error("Init Error - Failed to read path file");
                    } catch (const std::exception& e) {
                        log::error("Init: Error - {}", e.what());
                    }
                }
            }
        } else {
            log::info("Init: No rules found");
        }
    }

    log::info("Init: Registered {} pack(s)", packs.size());
    log::info("Init: Registered {} paths(s)", paths.size());
    log::info("Init: Registered {} rule(s)", rules.size());

    Rule* r1 = nullptr;
    Rule* r2 = nullptr;

    for (int i = 0; i < ruleIds.size(); i++) {
        r1 = &rules[ruleIds[i]];
        conflicts[r1].insert(r1);
        for (int j = i + 1; j < ruleIds.size(); j++) {
            r2 = &rules[ruleIds[j]];
            if (r1->ConflictsWith(r2)) {
                //log::info("Init: Conflict between {} and {}", r1->GetId(), r2->GetId());
                conflicts[r1].insert(r2);
                conflicts[r2].insert(r1);
            }
        }
    }

    for (auto& path : paths) {
        std::vector<std::string> ruleIds = path.GetRuleIds();

        for (auto r1Id : ruleIds) {
            if (auto rule1 = GetRuleByPath(r1Id)) {
                for (auto r2Id : ruleIds) {
                    if (r1Id == r2Id) continue;

                    if (auto rule2 = GetRuleByPath(r2Id)) {
                        if (rule2->GetLevel() < rule1->GetLevel()) {
                            //log::info("Init: {} is a predecessor of {}", r2Id, r1Id);
                            predecessors[rule1].insert(rule2);
                        }
                    }
                }
            } else {
                //log::info("Init: {} in path {} not found", r1Id, path.GetName());
            }
        }
    }
}

std::string DealManager::SelectRule(std::string lastRejected) {
    log::info("SelectRule: Start {}", lastRejected);
    std::unordered_set<Rule*> candidateRules;

    Rule* lastRejectedRule = nullptr;
    Rule* extendRule = nullptr;

    std::unordered_map<int, std::vector<Rule*>> stratifiedRules;

    for (auto& [path, rule] : rules) {
        if (!rule.IsEnabled()) {
            continue;
        }

        if (!rule.IsActive()) {
            log::info("SelectRule: resetting {}", rule.GetId());
            rule.Reset();
        }

        if (Lowercase(path) == Lowercase(ExtendRulePath)) {
            extendRule = &rule;
            continue;
        }

        if (path == lastRejected && !rule.IsActive()) {
            lastRejectedRule = &rule;
            continue;
        }

        bool compatible = true;
        for (auto& [_, deal] : deals) {
            for (auto activeRule : deal.rules) {
                if (conflicts[activeRule].contains(&rule) && !rule.CanRuleIdReplace(activeRule->GetId())) {
                    log::info("SelectRule: excluding {} due to active rule {}", path, activeRule->GetId());
                    compatible = false;
                    break;
                }
            }
            if (!compatible) {
                break;
            }
        }

        if (compatible) {
            log::info("SelectRule: adding {} to available pool", rule.GetId());
            candidateRules.insert(&rule);
        }
    }

    Rule* selectedRule = nullptr;
    
    auto forcedRuleIds = Config::GetSingleton().GetForcedDealIds();

    for (auto& ruleId : forcedRuleIds) {
        if (auto rule = GetRuleByPath(ruleId)) {
            if (!rule->IsActive() && candidateRules.contains(rule)) {
                log::info("SelectRule: forcing rule to {}", ruleId);
                selectedRule = rule;
                break;
            }
        }
    }

    if (!selectedRule) {
        if (candidateRules.empty() && lastRejectedRule) candidateRules.insert(lastRejectedRule);
        if (deals.size() > 0 && candidateRules.empty()) candidateRules.insert(extendRule);

        std::vector<Rule*> finalRules;
        finalRules.reserve(candidateRules.size());

        std::vector<int> ruleWeights;
        ruleWeights.reserve(candidateRules.size());

        int targetSeverity = CalculateTargetSeverity();
        log::info("CalculateTargetSeverity: {}", targetSeverity);

        for (auto rule : candidateRules) {
            finalRules.push_back(rule);
            ruleWeights.push_back(CalculateRuleWeight(rule, targetSeverity));
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(ruleWeights.begin(), ruleWeights.end());
        selectedRule = finalRules[d(gen)];
    }

    log::info("SelectRule: Selected {}", selectedRule->GetName());

    selectedRule->SetSelected();

    return selectedRule->GetId();
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

        Deal* chosen = nullptr;

        int index = PickRandom(candidateDeals.size());
        log::info("ActivateRule: index = {} out of {}", index, deals.size());
        if (candidateDeals.empty() || index == candidateDeals.size()) {
            auto name = GetNextDealName();
            Deal deal(name);

            auto key = Lowercase(name);

            deals[key] = deal;
            chosen = GetDealByName(key);
        } else {
            chosen = candidateDeals[index];
        }

        log::info("ActivateRule: selected deal {} {}", chosen->GetName(), chosen->rules.size());

        rule->Activate();
        chosen->UpdateTimer();
        chosen->rules.push_back(rule);

        log::info("ActivateRule: num rules = {} num deals = {}", chosen->rules.size(), deals.size());

        return chosen->rules.size();
    } else {
        log::error("ActivateRule: Invalid id {} given", path);
        return -1;
    }
}


void DealManager::RemoveDeal(std::string name) {
    log::info("RemoveDeal: {}", name);
    if (auto deal = GetDealByName(name)) {
        for (auto& rule : deal->rules) {
            if (rule->IsActive()) rule->Enable();
        }
        log::info("RemoveDeal: removing {}", name);
        deals.erase(Lowercase(name));
        log::info("RemoveDeal: {} deals left", deals.size());

    } else {
        log::info("RemoveDeal: {} not found", name);     
    }
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
    for (auto& [_, deal] : deals) {
        for (auto rule : deal.rules) {
            rule->Reset();
        }
    }
}

void DealManager::Resume() {
    for (auto& [_, deal] : deals) {
        for (auto rule : deal.rules) {
            rule->Activate();
        }
    }
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
    log::info("GetDealCost: {}", name);
    if (auto deal = GetDealByName(name)) {
        auto cost = deal->GetCost();
        log::info("GetDealCost: {} = {}", name, cost);
        return deal->GetCost();
    } else {
        log::info("GetDealCost: {} not found", name);
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
    if (auto rule = GetRuleByPath(path)) {
        rule->valid = valid;
        if (!valid) rule->Disable();
    }else
        SKSE::log::info("SetRuleValid: could not set valid for {}", path);
}

std::string DealManager::GetNextDealName() {
    for (auto& name : allDealNames)
        if (!deals.count(Lowercase(name))) return name;

    log::error("GetNextDealName - Failed to find open name");
    return "";
}

int DealManager::CalculateTargetSeverity() {
    if (deals.empty()) return 1;

    int targetSeverity = 1;

    auto calcMode = Config::GetSingleton().GetTargetSeverityMode();

    switch (calcMode) { 
        case SeverityMode::Max: {

            for (const auto& [_, deal] : deals) {
                targetSeverity = std::max(targetSeverity, (int) deal.rules.size());
            }

            break;
        }
        case SeverityMode::Median: {
            std::vector<int> counts;

            for (const auto& [_, deal] : deals) {
                counts.push_back(deal.rules.size());
            }

            int midpoint = counts.size() / 2;

            std::nth_element(counts.begin(), counts.begin() + midpoint, counts.end());

            targetSeverity = counts[midpoint];

            break;
        }
        case SeverityMode::Mode: {
            std::unordered_map<int, int> counts;
            
            int mode = 0;
            for (auto& [_, deal] : deals) {
                counts[deal.rules.size()]++;

                if (mode < 1) {
                    mode = 1;
                } else if (mode < counts[deal.rules.size()]) {
                    mode = deal.rules.size();
                }
            }

            targetSeverity = mode;

            break;
        }
    }

    return std::clamp(targetSeverity, 1, 3);
}

int DealManager::CalculateRuleWeight(Rule* rule, int targetSeverity) {
    auto config = Config::GetSingleton();

    int willpower = Settings::GetWillpower();
    
    bool loWill = willpower <= config.GetLowWillpowerThreshold();
    bool hiWill = willpower >= config.GetHighWillpowerThreshold();

    int score = config.GetBaseScore();

    if (rule->GetLevel() < targetSeverity) score += config.GetBelowThresholdBoost();
    if (rule->GetLevel() == targetSeverity) score += config.GetExactThresholdBoost();

    if (loWill && rule->GetLevel() > targetSeverity) score += config.GetLowWillpowerBoost();
    if (hiWill && rule->GetLevel() < targetSeverity) score += config.GetHighWillpowerBoost(); 

    bool appliedBoost = false;
    for (auto& [_, deal] : deals) {
        for (auto& other : deal.rules) {
            if (predecessors[rule].contains(other)) {
                score += config.GetPathBoost();
                if (!config.GetApplyMultiplePathBoost()) {
                    appliedBoost = true;
                    break;
                }
            }
        }
        if (appliedBoost) break;
    };

    score = std::max(1, score);

    log::info("CalculateRuleWeight: {} = {}", rule->GetId(), score);

    return score;
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