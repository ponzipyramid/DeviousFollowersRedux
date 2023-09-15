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
        log::info("DFF Directory not valid");
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

        log::info("Initializing {}", dirName);

        std::string dealDir = groupName + "\\Deals";
        std::string rulesDir = groupName + "\\Rules";


        if (IsDirValid(dealDir)) {
            for (const auto& p : std::filesystem::directory_iterator(dealDir)) {
                auto fileName = p.path();

                if (fileName.extension() == ext1 || fileName.extension() == ext2) {                    
                    const auto name = fileName.stem().string();

                    Deal entity(dirName, name);
                    bool issue = false;

                    try {
                        std::ifstream inputFile(fileName.string());
                        if (inputFile.good()) {
                            yaml_source ar(inputFile);

                            ar >> entity;
                            deals[entity.GetFullName()] = entity;
                            dealGroups[dirName].push_back(entity.GetFullName());
                            dealIds.push_back(entity.GetFullName());
                            
                            log::info("Registered Deal {}", entity.GetName());

                        } else
                            log::error("Error: Failed to read file {}", fileName.string());
                    } catch (const std::exception& e) {
                        issue = true;
                        log::error("Error: {}", e.what());
                    }

                    if (issue) log::error("Failed to register deal: {}", entity.GetFullName());
                }
            }
        } else {
            log::info("No deals found");
        }

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

    for (const auto& [key, value] : deals) {
        std::string name = deals[key].GetFullName();
        if (deals[key].IsBuiltIn()) builtInDeals.push_back(&deals[key]);
        if (deals[key].IsModular()) modularDeals.push_back(&deals[key]);
    }

    log::info("{} modular deals", modularDeals.size());

    Deal* d1;
    Deal* d2;

    Rule* r1;
    Rule* r2;

    for (int i = 0; i < dealIds.size(); i++) {
        d1 = &deals[dealIds[i]];
        for (int j = i + 1; j < dealIds.size(); j++) {
            d2 = &deals[dealIds[j]];
            if (d1->ConflictsWith(d2)) {
                log::info("Deal-Deal Conflict: {} and {}", d1->GetFullName(), d2->GetFullName());
                conflicts[d1].insert(d2);
                conflicts[d2].insert(d1);
            }
        }

        for (auto& [key, r1] : rules) {
            if (d1->ConflictsWith(&r1)) {
                log::info("Deal-Rule Conflict: {} and {}", d1->GetFullName(), r1.GetFullName());

                conflicts[d1].insert(&r1);
                conflicts[&r1].insert(d1);
            }
        }
    }

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


    log::info("Finished initializing. Registered {} deals and {} rules.", deals.size(), rules.size());
}

void DealManager::InitQuests() {
    log::info("Initializing deal quests");

    std::vector<std::string> toRemove;

    for (const auto& [key, value] : deals) {
        if (deals[key].InitQuest()) {
            RE::FormID formId = deals[key].GetQuest()->GetFormID();
            formMap[formId] = &deals[key];
        } else {
            log::info("Could not initialize quest data for {} - unregistering", key);
            toRemove.push_back(key);
        }
    }

    for (auto& [key, rule] : rules) {
        rule.Init();
    }

    for (const auto& key : toRemove) {
        deals.erase(key);
    }
}

void DealManager::InitQuestData() {
    log::info("Initializing deal quest data");

    for (const auto& [key, value] : deals) {
        if (deals[key].InitQuestData() && deals[key].IsActive()) {
            log::info("{} is active", key);
            activeDeals.insert(key);
        }
    }
}

int DealManager::SelectDeal(int track, int maxModDeals, float bias, int lastRejectedId) {

    Deal* chosen = nullptr;
    std::string name;
    std::string forcedDealName = Config::GetSingleton().GetForcedDealName();

    if (forcedDealName != "") {
        SKSE::log::info("Attempting to select forced deal {}", forcedDealName);

        name = forcedDealName;
        Deal* forcedDeal = &deals[forcedDealName];
        if (forcedDeal != nullptr && forcedDeal->HasNextStage() && !DoesActiveExclude(forcedDeal)) {
            SKSE::log::info("Successfully selected forced deal");
            chosen = forcedDeal;
        } else {
            SKSE::log::info("Failed to select forced deal");
        }

    }
    if (chosen == nullptr) {
        RegeneratePotentialDeals(maxModDeals, lastRejectedId);

        std::vector<std::string> candidateDeals;
        if (candidateClassicDeals.empty()) {
            candidateDeals = candidateModularDeals;
        } else if (candidateModularDeals.empty()) {
            candidateDeals = candidateClassicDeals;
        } else {
            int biasTest = PickRandom(100);
            candidateDeals = biasTest < bias ? candidateClassicDeals : candidateModularDeals;
        }

        int numCandidates = candidateDeals.size();

        if (!numCandidates) {
            return 0;
        }
        int index = PickRandom(numCandidates);

        name = candidateDeals[index];

        chosen = &deals[name];
    
    }
    
    int id;
    if (chosen->IsModular()) {
        id = chosen->GetBuiltInId();
        
        std::vector<Rule*> leveledCandidates;
        log::info("{} rules available", candidateRules.size());
        for (Rule* rule : candidateRules) {
            if (rule->IsRuleForLevel(chosen->GetNextStage())) {
                leveledCandidates.push_back(rule);
            }
        }
        int index = PickRandom(leveledCandidates.size());
        Rule* rule = leveledCandidates[index];
        id += rule->GetBuiltInId();
        return id;
    } else if (name_id_map.count(name)) {
        id = name_id_map[name];
    } else {
        id = 1000;
        while (id_name_map.count(id)) {
            id++;
        }
    }
    
    name_id_map[name] = id;
    id_name_map[id] = name;

    if (track) selectedDeal = name;
    
    log::info("Chose deal {} stage {} - assigned id {}", name, chosen->GetNextStage(), id);

    return id;
}


Deal* DealManager::GetDealById(int id, bool modular) {
    if (id_name_map.count(id)) {
        return &deals[id_name_map[id]];
    }
    
    std::vector<Deal*>& checkDeals = builtInDeals;

    int n = id;
    while (n >= 10) n /= 10;
    n *= 10;

    if (modular) {
        n *= 10;
        checkDeals = modularDeals;
    }

    Deal* deal = nullptr;

    for (Deal* d : checkDeals) {
        if (d->GetBuiltInId() == n) {
            deal = d;
            break;
        }
    }

    return deal;
}


void DealManager::ActivateDeal(int id) {

    SKSE::log::info("ActivateDeal - Id START");

    int actualId = abs(id);
    if (!id_name_map.count(actualId)) {
        if (id < 0) {
            // invalid deal
            return;
        } else if (id >= 100) {
            // modular deal
            Deal* deal = GetDealById(actualId, true);
            activeDeals.insert(deal->GetFullName());
            deal->NextStage();

            SKSE::log::info("ActivateDeal END");

            return;
        } else {
            // classic deal
            Deal* deal = GetDealById(actualId);
            id_name_map[actualId] = deal->GetFullName();
            name_id_map[deal->GetFullName()] = actualId;
        }
    }

    activeDeals.insert(id_name_map[actualId]);
    deals[id_name_map[id]].NextStage();
    if (id_name_map[actualId] == selectedDeal) {
        selectedDeal = "";
    }
    SKSE::log::info("ActivateDeal END");

    log::info("Activating {}", id_name_map[actualId]);
}

void DealManager::ActivateDeal(RE::TESQuest* q) {
    Deal* deal = formMap[q->GetFormID()];
    
    SKSE::log::info("ActivateDeal - Quest START");

    if (deal) {
        activeDeals.insert(deal->GetFullName());
        deals[deal->GetFullName()].NextStage();
    }
}

Rule* DealManager::GetRuleById(int id) {
    while (id >= 100) {
        id -= 100;
    }
    for (auto& [key, rule] : rules) {
        if (rule.GetBuiltInId() == id) {
            return &rule;
        }
    }

    return nullptr;
}


void DealManager::RegeneratePotentialDeals(int maxModDeals, int lastRejectedId) {
    std::vector<std::string> validatedClassic;
    std::vector<std::string> validatedModular;

    Rule* lastRejectedRule = nullptr;
    Deal* lastRejectedDeal = nullptr;

    if (lastRejectedId > 100 && lastRejectedId < 1000) {
        lastRejectedRule = GetRuleById(lastRejectedId);
    } else {
        lastRejectedDeal = GetDealById(lastRejectedId);
    }

    // create list of rules
    candidateRules.clear();
    bool canAddRejected = false;
    for (auto& [key, rule] : rules) {
        if (!DoesActiveExclude(&rule) && rule.IsEnabled()) {
            if (&rule == lastRejectedRule) {
                canAddRejected = true;
            } else {
                candidateRules.push_back(&rule);
            }
        }
    }

    if (canAddRejected && candidateRules.empty()) {
        candidateRules.push_back(lastRejectedRule);
    }

    int currActiveModularDeals = 0;

    // get number of active deals
    for (std::string dealId : activeDeals) {
    
        if (deals[dealId].IsModular()) {
            currActiveModularDeals++;
        }
    
    }

    canAddRejected = false;
    for (auto& [id, value] : deals) {
        Deal* deal = &deals[id];
        if (deal->IsModular() && deal->HasNextStage()) {
            if (!candidateRules.empty() &&
                (activeDeals.count(deal->GetFullName()) || (currActiveModularDeals + 1) <= maxModDeals))
                validatedModular.push_back(deal->GetFullName());
        } else if (deal->HasNextStage() && !DoesActiveExclude(deal)) {
            if (deal == lastRejectedDeal) {
                canAddRejected = true;
            } else {
                validatedClassic.push_back(deal->GetFullName());
            }
        }
    }

     if (canAddRejected && validatedClassic.empty()) {
        validatedClassic.push_back(lastRejectedDeal->GetFullName());
     }

    log::info("RegeneratePotentialDeals: {} active deals, {} potential deals {} potential rules", activeDeals.size(),
              validatedClassic.size() + validatedModular.size(), candidateRules.size());
    candidateClassicDeals = validatedClassic;
    candidateModularDeals = validatedModular;
}



bool DealManager::DoesActiveExclude(Conflictor* entity) { 
    for (std::string dealId : activeDeals) {
        if (conflicts[&deals[dealId]].count(entity)) {
            return true;
        }
    }

    for (const auto& [key, rules] : activeRules) {
        for (Rule* rule : rules) {
            if (conflicts[rule].count(entity)) {
                return true;
            }
        }
    }
    return false;
}

void DealManager::RemoveDeal(RE::TESQuest* quest) {
    RE::FormID formId = quest->GetFormID();

    Deal* deal = formMap[formId];
    std::string actualId = deal->GetFullName();

    log::info("Removing deal {}", actualId);

    int id = name_id_map[actualId];

    activeDeals.erase(actualId);
    name_id_map.erase(actualId);
    id_name_map.erase(id);

    deals[actualId].Reset();

    if (deal->IsModular())
        activeRules[deal].clear();
}

int DealManager::GetStage(int id) { return deals[id_name_map[id]].GetStage(); }

int DealManager::GetStageIndex(RE::TESQuest* q) {
    Deal* deal = formMap[q->GetFormID()];

    if (deal) {
        return deal->GetStageIndex();
    } else {
        return -1;
    }
}

RE::TESQuest* DealManager::GetDealQuest(int id) {
    return deals[id_name_map[id]].GetQuest(); 
}

bool DealManager::IsDealActive(int id) { return activeDeals.count(id_name_map[id]); }

bool DealManager::IsDealSelected(std::string name) { return name == selectedDeal; }

void DealManager::LoadActiveModularRules(std::vector<RE::TESQuest*> quests, std::vector<int> activeRuleIds) {
    for (int i = 0; i < quests.size(); i++) {
        Deal* deal = formMap[quests[i]->GetFormID()];

        for (int j = 0; j < 3; j++) {
            int id = activeRuleIds[(i * 3) + j];

            for (auto& [key, rule] : rules) {
                if (rule.GetBuiltInId() == id) {
                    SKSE::log::info("Added rule {} to deal", rule.GetFullName(), deal->GetFullName());
                    activeRules[deal].push_back(&rule);
                    break;
                }
            }
        }
    }
    log::info("Loaded in {} mod rules for {} modular deals", activeRuleIds.size(), quests.size());
}

void DealManager::ActivateRule(RE::TESQuest* quest, int id) { 
    log::info("Activating rule with id {}", id);

    RE::FormID formId = quest->GetFormID();
    Deal* deal = formMap[formId];

    for (auto& [key, rule] : rules) {
        if (rule.GetBuiltInId() == id) {
            activeRules[deal].push_back(&rule);
            break;
        }
    }
    log::info("Activating rule with id {}", id);

}

void DealManager::LoadRuleMaxStage(RE::TESQuest* quest, int maxStage) {
    formMap[quest->GetFormID()]->SetMaxStage(maxStage);
}


std::vector<RE::TESQuest*> DealManager::GetActiveDeals(int filter) { 
    std::vector<RE::TESQuest*> quests;

    for (std::string dealId : activeDeals) {
        auto& deal = deals[dealId];

        if ((filter == 0) || (filter == 1 && !deal.IsModular()) || (filter == 2 && deal.IsModular())) {
            RE::TESQuest* q = deals[dealId].GetQuest();
            quests.push_back(q);
        }
    }

    return quests;
}
std::string DealManager::GetDealName(RE::TESQuest* quest) {
    if (quest == nullptr) {
        return "";
    }

    Deal* deal = formMap[quest->GetFormID()];

    bool isNull = formMap[quest->GetFormID()] == nullptr;
    
    if (deal != nullptr)
        return deal->GetName();
    else
        return "";
}

std::vector<std::string> DealManager::GetDealRules(RE::TESQuest* quest) { 
    std::vector<std::string> ruleNames;
    Deal* deal = formMap[quest->GetFormID()];
    if (deal == nullptr) return ruleNames;

    if (deal->IsModular()) {
        for (Rule* rule : activeRules[deal]) ruleNames.push_back(rule->GetName());
    } else {
        for (Stage* stage : deal->GetActiveStages()) ruleNames.push_back(stage->GetName());
    }
    
    log::info("Fetching {} rules for deal {}", ruleNames.size(), deal->GetName());
    return ruleNames;
}

void DealManager::ToggleRule(std::string group, int id, bool enabled) {
    std::string fullName;
    std::vector<Rule*> groupRules = ruleGroups[group];
    for (int i = 0; i < groupRules.size(); i++) {
        int currId = groupRules[i]->IsBuiltIn() ? groupRules[i]->GetBuiltInId() : i;
        if (id == i) {
            fullName = groupRules[i]->GetFullName();
            break;
        }
    }
    if (rules.count(fullName)) {
        rules[fullName].SetEnabled(enabled);
        log::info("Toggled {} to {}", fullName, enabled);
    }
}

void DealManager::ToggleStageVariation(std::string fullName, int stageIndex, int varIndex, bool enabled) {    
    if (deals.count(fullName)) {
        log::info("Toggling variation for deal {}", fullName);
        Deal* deal = &deals[fullName];
        deal->ToggleStageVariation(stageIndex, varIndex, enabled);
    } else {
        log::info("Couldn't find deal to toggle stage {}", fullName);
    }
}

int DealManager::GetDealNextQuestStage(int id) { 
    Deal* deal = GetDealById(id);
    int stage = deal->GetNextStage();
    log::info("GetDealNextQuestStage {} w/ stage", deal->GetName(), stage);
    return stage;
}

RE::TESGlobal* DealManager::GetDealCostGlobal(RE::TESQuest* q) { return formMap[q->GetFormID()]->GetCostGlobal(); }

RE::TESGlobal* DealManager::GetDealTimerGlobal(RE::TESQuest* q) { return formMap[q->GetFormID()]->GetTimerGlobal(); }


std::vector<std::string> DealManager::GetGroupNames() { 
    std::vector<std::string> groupNames; 

    for (auto& [name, deals] : dealGroups) {
        groupNames.push_back(name);
    }

    return groupNames;
}
std::vector<RE::TESQuest*> DealManager::GetGroupDeals(std::string groupName, int filter) { 
    std::vector<RE::TESQuest*> dealQuests;

    bool includeModular = false;
    bool includeClassic = false;

    if (filter == 0) {
        includeModular = true;
        includeClassic = true;
    } else if (filter == 1) {
        includeClassic = true;
    } else {
        includeModular = true;
    }

    for (auto& dealName : dealGroups[groupName]) {
        Deal* deal = &deals[dealName];

        if ((includeModular && deal->IsModular()) || (includeClassic && !deal->IsModular()))
            dealQuests.push_back(deals[dealName].GetQuest());
    }

    return dealQuests;
}

RE::TESQuest* DealManager::SelectRandomActiveDeal() {
    int numActive = activeDeals.size();

    int rand = PickRandom(numActive);
    std::vector<Deal*> activeDealList;
    activeDealList.reserve(numActive);

    for (const auto& activeDeal : activeDeals) {
        activeDealList.push_back(&deals[activeDeal]);
    }

    return activeDealList[rand]->GetQuest();
}

int DealManager::GetDealNumStages(RE::TESQuest* q) { 
    Deal* deal = formMap[q->GetFormID()];
    if (deal)
        return deal->GetNumStages();
    else
        return 0;
}

std::vector<std::string> DealManager::GetDealFinalStages(RE::TESQuest* q) {
    std::vector<std::string> stageIndices;

    Deal* deal = formMap[q->GetFormID()];

    if (!deal) return stageIndices;

    auto stages = deal->GetStages();

    auto lastStage = stages[stages.size() - 1];
    auto altStages = lastStage.GetAltStages();

    stageIndices.reserve(altStages.size() + 1);
    stageIndices.push_back(lastStage.GetName());

    for (auto& alt : altStages) {
        stageIndices.push_back(alt.GetName());
    }

    return stageIndices;
}

std::vector<int> DealManager::GetDealFinalStageIndexes(RE::TESQuest* q) {
    std::vector<int> stageIndices;
    
    Deal* deal = formMap[q->GetFormID()];
    if (!deal) return stageIndices;

    auto stages = deal->GetStages();

    auto lastStage = stages[stages.size() - 1];
    auto altStages = lastStage.GetAltStages();

    stageIndices.reserve(altStages.size() + 1);
    stageIndices.push_back(lastStage.GetIndex());

    for (auto& alt : altStages) {
        stageIndices.push_back(alt.GetIndex());
    }

    return stageIndices;
}

bool DealManager::IsDealValid(RE::TESQuest* q) { return formMap[q->GetFormID()] != nullptr; }

void DealManager::ShowBuyoutMenu() {
    std::string msg = "Buyout: Select a Deal";
    std::vector<std::string> options; 
    std::vector<std::string> dealList;
    for (auto deal : activeDeals) {
        auto cost = deals[deal].GetCostGlobal();
        dealList.push_back(deal);
        options.push_back(std::format("{} [{}]", deals[deal].GetName(), cost->value));
    }

    options.push_back("Cancel");

    SKSE::log::info("Showing message box");


    menuChosen = false;

    TESMessageBox::Show(msg, options, [dealList](int result) {
        SKSE::log::info("Selected {}", result);
        std::string val;

        if (result < dealList.size())
            val = dealList[result];
        else
            SKSE::log::info("User cancelled buyout");

        DealManager::GetSingleton().chosenDeal = val;
        DealManager::GetSingleton().menuChosen = true;

        SKSE::log::info("Set chosen deal to {} and menuChosen to {}", DealManager::GetSingleton().chosenDeal,
                        DealManager::GetSingleton().menuChosen);

    });
}

RE::TESQuest* DealManager::GetBuyoutMenuResult() {
    SKSE::log::info("Fetching chosen deal to {} and menuChosen to {}", DealManager::GetSingleton().chosenDeal,
                    DealManager::GetSingleton().menuChosen);

    DealManager::GetSingleton().menuChosen = false;

    if (chosenDeal.empty()) {
        SKSE::log::info("No chosen deal");
        return nullptr;
    } else {
        auto deal = &deals[chosenDeal];
        if (deal) {
            return deal->GetQuest();
        } else {
            return nullptr;
        } 
    }
}

void DealManager::OnRevert(SerializationInterface*) {
    std::unique_lock lock(GetSingleton()._lock);
    GetSingleton().id_name_map.clear();
    GetSingleton().name_id_map.clear();
    GetSingleton().activeRules.clear();
    GetSingleton().activeDeals.clear();

    for (auto& [key, deal] : GetSingleton().deals) {
        deal.Reset();
    }
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

void DealManager::OnGameSaved(SerializationInterface* serde) {
    std::unique_lock lock(GetSingleton()._lock);
    if (!serde->OpenRecord(ActiveDealsRecord, 0)) {
        log::error("Unable to open record to write cosave data.");
        return;
    }

    // save deal to id mapping
    auto mapSize = GetSingleton().id_name_map.size();
    serde->WriteRecordData(&mapSize, sizeof(mapSize));
    for (auto& count : GetSingleton().id_name_map) {
        WriteString(serde, count.second);
        int maxStage = count.first;

        log::info("Saved deal mapping {} -> {}", count.second, count.first);

        serde->WriteRecordData(&maxStage, sizeof(maxStage));
    }

    // save active rules for each modular deal
    auto ruleSize = GetSingleton().activeRules.size();
    serde->WriteRecordData(&ruleSize, sizeof(ruleSize));
    for (auto& [deal, rules] : GetSingleton().activeRules) {
        WriteString(serde, deal->GetFullName());
        auto ruleCount = rules.size();
        serde->WriteRecordData(&ruleCount, sizeof(ruleCount));
        for (Rule* rule : rules) {
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
            GetSingleton().id_name_map.clear();
            GetSingleton().name_id_map.clear();

            // load in deal id mappings
            std::size_t mapSize;
            serde->ReadRecordData(&mapSize, sizeof(mapSize));
            for (; mapSize > 0; --mapSize) {
                std::string name = ReadString(serde);

                int id;
                serde->ReadRecordData(&id, sizeof(id));

                GetSingleton().id_name_map[id] = name;
                GetSingleton().name_id_map[name] = id;
            }

            // load in active rules for modular deals
            std::size_t ruleSize;
            serde->ReadRecordData(&ruleSize, sizeof(ruleSize));
            for (; ruleSize > 0; --ruleSize) {
                std::string id = ReadString(serde);
                Deal& deal = GetSingleton().deals[id];
                std::size_t ruleCount;
                serde->ReadRecordData(&ruleCount, sizeof(ruleCount));
                for (; ruleCount > 0; --ruleCount) {
                    std::string ruleId = ReadString(serde);
                    GetSingleton().activeRules[&GetSingleton().deals[id]].push_back(&GetSingleton().rules[ruleId]);
                }
            }
        }
    }
}