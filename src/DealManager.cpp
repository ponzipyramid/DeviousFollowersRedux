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

using namespace RE;
using namespace DFF;
using namespace SKSE;
using namespace articuno::ryml;

namespace {
    inline const auto ActiveDealsRecord = _byteswap_ulong('ACTD');
}
DealManager& DealManager::GetSingleton() noexcept {
    static DealManager instance;
    return instance;
}

int PickRandom(int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, max - 1);
    return distr(gen);
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
    std::string dir("Data\\SKSE\\Plugins\\DFF");
   
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

    for (const auto& [key, value] : deals) {
        deals[key].InitQuest();
        RE::FormID formId = deals[key].GetQuest()->GetFormID();
        formMap[formId] = &deals[key];
    }

    for (auto& [key, rule] : rules) {
        rule.Init();
    }
}

void DealManager::InitQuestData() {
    log::info("Initializing deal quest data");

    for (const auto& [key, value] : deals) {
        deals[key].InitQuestData();
        if (deals[key].IsActive()) {
            log::info("{} is active", key);
            activeDeals.insert(key);
        }
    }
}

int DealManager::SelectDeal(int track, int maxModDeals, float bias) {
    RegeneratePotentialDeals(true);

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

    std::string name = candidateDeals[index];

    Deal& chosen = deals[name];
    int id;
    if (chosen.IsBuiltIn()) {        
        id_name_map.erase(chosen.GetBuiltInId() + chosen.GetStage());

        id = chosen.GetBuiltInId();

        if (chosen.IsModular()) { 
            std::vector<Rule*> leveledCandidates;
            log::info("{} rules available", candidateRules.size());
            for (Rule* rule : candidateRules) {
                if (rule->IsRuleForLevel(chosen.GetNextStage())) {
                    leveledCandidates.push_back(rule);
                }
            }
            int index = PickRandom(leveledCandidates.size());
            Rule* rule = leveledCandidates[index];
            log::info("Chose modular deal {} stage {} and rule {}", name, chosen.GetNextStage(), rule->GetFullName());
            id += rule->GetBuiltInId();
            return id;
        } else {
            id += chosen.GetNextStage();
        }

    } else if (name_id_map.count(name)) {
        id = name_id_map[name];
    } else {
        id = 1;
        while (id_name_map.count(id)) {
            id = PickRandom(1000000);
        }
    }
    
    name_id_map[name] = id;
    id_name_map[id] = name;

    if (track) selectedDeal = name;
    
    log::info("Chose deal {} stage {}", name, chosen.GetNextStage());

    return (chosen.IsBuiltIn() ? 1 : -1) * id;
}


Deal* DealManager::GetDealById(int id, bool modular) {
    std::vector<Deal*>& checkDeals = builtInDeals;

    int n = id;
    while (n >= 10) n /= 10;
    n *= 10;

    if (modular) {
        n *= 10;
        checkDeals = modularDeals;
    }

    Deal* deal;

    for (Deal* d : checkDeals) {
        if (d->GetBuiltInId() == n) {
            deal = d;
            break;
        }
    }

    return deal;
}


void DealManager::ActivateDeal(int id) {

    int actualId = abs(id);
    if (!id_name_map.count(actualId)) {
        if (id < 0) {
            // addon deal
            return;
        } else if (id >= 100) {
            // modular deal
            Deal* deal = GetDealById(actualId, true);
            activeDeals.insert(deal->GetFullName());
            deal->NextStage();
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

    log::info("Activating {}", id_name_map[actualId]);
}


void DealManager::RegeneratePotentialDeals(int maxModDeals) {
    std::vector<std::string> validatedClassic;
    std::vector<std::string> validatedModular;

    candidateRules.clear();
    for (auto& [key, rule] : rules) {
        if (!DoesActiveExclude(&rule) && rule.IsEnabled()) {
            candidateRules.push_back(&rule);
        }
    }

    int currActiveModularDeals = 0;

    for (std::string dealId : activeDeals) {
    
        if (deals[dealId].IsModular()) {
            currActiveModularDeals++;
        }
    
    }

    for (auto& [id, value] : deals) {
        Deal* deal = &deals[id];
        if (deal->IsModular() && deal->HasNextStage()) {
            if (!candidateRules.empty() &&
                (activeDeals.count(deal->GetFullName()) || (currActiveModularDeals + 1) <= maxModDeals))
                validatedModular.push_back(deal->GetFullName());
        } else if (deal->HasNextStage() && !DoesActiveExclude(deal)) {
            validatedClassic.push_back(deal->GetFullName());
        }
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

RE::TESQuest* DealManager::GetDealQuest(int id) { return deals[id_name_map[id]].GetQuest(); }

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


std::vector<RE::TESQuest*> DealManager::GetActiveDeals(bool classic) { 
    std::vector<RE::TESQuest*> quests;

    if (classic) {
        for (std::string dealId : activeDeals) {
            if (!deals[dealId].IsModular()) {
                quests.push_back(deals[dealId].GetQuest());
                log::info("Added deal {}", deals[dealId].GetName());
            }
        }
        
    } else {
        for (std::string dealId : activeDeals) {
            if (deals[dealId].IsModular()) {
                quests.push_back(deals[dealId].GetQuest());
                log::info("Added deal {}", dealId);
            }
        }
        
    }

    return quests;
}
std::string DealManager::GetDealName(RE::TESQuest* quest) { return formMap[quest->GetFormID()]->GetName(); }

std::vector<std::string> DealManager::GetDealRules(RE::TESQuest* quest) { 
    std::vector<std::string> ruleNames;
    Deal* deal = formMap[quest->GetFormID()];
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

    // save max stage for each classic deal
    auto dealSize = GetSingleton().deals.size();
    serde->WriteRecordData(&dealSize, sizeof(dealSize));
    for (auto& [id, deal] : GetSingleton().deals) {
        WriteString(serde, deal.GetFullName());
        auto maxStage = deal.GetMaxStage();
        serde->WriteRecordData(&maxStage, sizeof(maxStage));
    }

    // save toggled rules
    ruleSize = GetSingleton().rules.size();
    serde->WriteRecordData(&ruleSize, sizeof(ruleSize));
    for (auto& [id, rule] : GetSingleton().rules) {
        WriteString(serde, rule.GetFullName());
        auto enabled = rule.IsEnabled();
        serde->WriteRecordData(&enabled, sizeof(enabled));
    }

    // save toggled stage variations for classic deals
    dealSize = GetSingleton().deals.size() - GetSingleton().modularDeals.size();
    serde->WriteRecordData(&dealSize, sizeof(dealSize));

    log::info("Saving {} deals", dealSize);
    for (auto& [id, deal] : GetSingleton().deals) {
        if (deal.IsModular()) continue;

        WriteString(serde, deal.GetFullName());
        log::info("Saving deal {}", deal.GetFullName());
        std::size_t numStages = 0;
        std::vector<Stage> stages = deal.GetStages();
        numStages += stages.size();
        for (Stage& stage : stages) {
            numStages += stage.GetAltStages().size();
        }
        serde->WriteRecordData(&numStages, sizeof(numStages));
        log::info("Saving {} stages", numStages);
        for (Stage& stage : deal.GetStages()) {
            auto enabled = stage.IsEnabled();
            serde->WriteRecordData(&enabled, sizeof(enabled));

            log::info("Saving {} is {} ", stage.GetName(), enabled);

            auto numAltStages = stage.GetAltStages().size();
            serde->WriteRecordData(&numAltStages, sizeof(numAltStages));

            log::info("Saving {} alt stages", numAltStages);
            for (Stage& altStage : stage.GetAltStages()) {
                enabled = altStage.IsEnabled();
                serde->WriteRecordData(&enabled, sizeof(enabled));

                log::info("Saving {} is {} ", altStage.GetName(), enabled);
            }
        }
    }
}

void DealManager::OnGameLoaded(SerializationInterface* serde) {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t version;

    while (serde->GetNextRecordInfo(type, version, size)) {
        if (type == ActiveDealsRecord) {
            log::info("Loading in save data");
            GetSingleton().id_name_map.clear();
            GetSingleton().name_id_map.clear();

            std::size_t mapSize;
            serde->ReadRecordData(&mapSize, sizeof(mapSize));
            for (; mapSize > 0; --mapSize) {
                std::string name = ReadString(serde);

                int id;
                serde->ReadRecordData(&id, sizeof(id));

                log::info("Loaded deal mapping {} -> {}", id, name);

                GetSingleton().id_name_map[id] = name;
                GetSingleton().name_id_map[name] = id;
            }

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

            std::size_t dealSize;
            serde->ReadRecordData(&dealSize, sizeof(dealSize));
            for (; dealSize > 0; --dealSize) {
                std::string id = ReadString(serde);
                int maxStage;
                serde->ReadRecordData(&maxStage, sizeof(maxStage));
                GetSingleton().deals[id].SetMaxStage(maxStage);
            }

            serde->ReadRecordData(&ruleSize, sizeof(ruleSize));
            for (; ruleSize > 0; --ruleSize) {
                std::string id = ReadString(serde);
                bool enabled;
                serde->ReadRecordData(&enabled, sizeof(enabled));
                GetSingleton().rules[id].SetEnabled(enabled);
            }

            serde->ReadRecordData(&dealSize, sizeof(dealSize));
            log::info("Loading {} deals", dealSize);
            for (; dealSize > 0; --dealSize) {
                std::string id = ReadString(serde);

                Deal& deal = GetSingleton().deals[id];
                log::info("Loading deal {}", deal.GetFullName());

                std::size_t numStages;
                serde->ReadRecordData(&numStages, sizeof(numStages));
                log::info("Loading {} stages", numStages);
                int i = 0;
                for (; numStages > 0; --numStages) {

                    bool enabled;
                    serde->ReadRecordData(&enabled, sizeof(enabled));

                    deal.GetStages()[i].SetEnabled(enabled);
                    log::info("Loaded stage {} is {}", deal.GetStages()[i].GetName(), deal.GetStages()[i].IsEnabled());
                    std::size_t numAltStages;
                    serde->ReadRecordData(&numAltStages, sizeof(numAltStages));

                    numStages -= numAltStages; // prevent out of bounds

                    log::info("Loading {} alt stages", numAltStages);
                    int j = 0;
                    for (; numAltStages > 0; --numAltStages) {
                        serde->ReadRecordData(&enabled, sizeof(enabled));
                        
                        deal.GetStages()[i].GetAltStages()[j].SetEnabled(enabled);
                        log::info("Loaded alt stage {} is {}", deal.GetStages()[i].GetAltStages()[j].GetName(),
                                  deal.GetStages()[i].GetAltStages()[j].IsEnabled());
                        j++;
                    }

                    i++;
                }
            }
        }
    }
}