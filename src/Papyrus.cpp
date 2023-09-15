#include "Papyrus.h"

#include <DFF/DealManager.h>
#include <Device/DeviceManager.h>

using namespace DFF;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "DealManager";
    constexpr std::string_view PapyrusDeviceClass = "DeviceManager";

    int32_t SelectDeal(StaticFunctionTag*, int track, int maxModDeals, float_t prefersModular, int lastRejectedId) {
        return DealManager::GetSingleton().SelectDeal(track, maxModDeals, prefersModular, lastRejectedId);
    }

    void ActivateDeal(StaticFunctionTag*, int32_t id) { DealManager::GetSingleton().ActivateDeal(id); }
    
    void ActivateDealByQuest(StaticFunctionTag*, RE::TESQuest* quest) {
        DealManager::GetSingleton().ActivateDeal(quest);
    }
    
    void RemoveDeal(StaticFunctionTag*, RE::TESQuest* quest) { DealManager::GetSingleton().RemoveDeal(quest); }
    
    TESQuest* GetDealQuest(StaticFunctionTag*, int32_t id) { return DealManager::GetSingleton().GetDealQuest(id); }
    
    int32_t GetStage(StaticFunctionTag*, int32_t id) { return DealManager::GetSingleton().GetStage(id); }
    
    bool IsDealActive(StaticFunctionTag*, int32_t id) { return DealManager::GetSingleton().IsDealActive(id); }

    void LoadActiveModularRules(StaticFunctionTag*, std::vector<RE::TESQuest*> quests, std::vector<int> ruleIds) {
        DealManager::GetSingleton().LoadActiveModularRules(quests, ruleIds);
    }

    void ActivateRule(StaticFunctionTag*, RE::TESQuest* quest, int id) {
        DealManager::GetSingleton().ActivateRule(quest, id);
    }

    void LoadRuleMaxStage(StaticFunctionTag*, RE::TESQuest* quest, int max) {
        DealManager::GetSingleton().LoadRuleMaxStage(quest, max);
    }

    std::vector<RE::TESQuest*> GetActiveDeals(StaticFunctionTag*, int filter) {
        return DealManager::GetSingleton().GetActiveDeals(filter);
    }
    std::string GetDealName(StaticFunctionTag*, RE::TESQuest* quest) {
        return DealManager::GetSingleton().GetDealName(quest);
    }
    std::vector<std::string> GetDealRules(StaticFunctionTag*, RE::TESQuest* quest) {
        return DealManager::GetSingleton().GetDealRules(quest);
    }
    void ToggleRule(StaticFunctionTag*, std::string group, int id, bool enabled) {
        DealManager::GetSingleton().ToggleRule(group, id, enabled);
    }
    void ToggleStageVariation(StaticFunctionTag*, std::string name, int stageIndex, int varIndex, bool enabled) {
        DealManager::GetSingleton().ToggleStageVariation(name, stageIndex, varIndex, enabled);
    }

    int GetDealNextQuestStage(StaticFunctionTag*, int id) { 
        return DealManager::GetSingleton().GetDealNextQuestStage(id);
    }

    RE::TESGlobal* GetDealCostGlobal(StaticFunctionTag*, RE::TESQuest* q) {
        return DealManager::GetSingleton().GetDealCostGlobal(q);

    }
    RE::TESGlobal* GetDealTimerGlobal(StaticFunctionTag*, RE::TESQuest* q) {
        return DealManager::GetSingleton().GetDealTimerGlobal(q);
    }
    int GetStageIndex(StaticFunctionTag*, RE::TESQuest* q) { 
        return DealManager::GetSingleton().GetStageIndex(q);
    }

    std::vector<TESForm*> GetInventoryNamedObjects(StaticFunctionTag*, RE::TESObjectREFR* a_container,
                                                                   std::vector<std::string> itemNames) {
        std::vector<TESForm*> refrs;
        
        if (!a_container) {
            return refrs;
        }

        auto inventory = a_container->GetInventory();
        for (const auto& [form, data] : inventory) {
            if (!form->GetPlayable() || form->GetName()[0] == '\0') continue;
            if (data.second->IsQuestObject()) continue;
            
            std::string formName = form->GetName();
            for (const auto& iName : itemNames) {

                std::string itemName(iName);

                std::transform(itemName.begin(), itemName.end(), itemName.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                std::transform(formName.begin(), formName.end(), formName.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                
                SKSE::log::info("Processing item {} with tag {}", formName, itemName);

                if (formName.find(itemName) != std::string::npos) {
                    refrs.push_back(form);
                }
            }
        }


        return refrs; 
    }

    std::vector<std::string> GetGroupNames(StaticFunctionTag*) {
        return DealManager::GetSingleton().GetGroupNames();
    }

    std::vector<RE::TESQuest*> GetGroupDeals(StaticFunctionTag*, std::string groupName, int filter) {
        return DealManager::GetSingleton().GetGroupDeals(groupName, filter);
    }

    RE::TESQuest* SelectRandomActiveDeal(StaticFunctionTag*) {
        return DealManager::GetSingleton().SelectRandomActiveDeal();
    }

    int GetDealNumStages(StaticFunctionTag*, RE::TESQuest* q) { 
        return DealManager::GetSingleton().GetDealNumStages(q);
    }

    std::vector<std::string> GetDealFinalStages(StaticFunctionTag*, RE::TESQuest* q) {
        return DealManager::GetSingleton().GetDealFinalStages(q);
    }

    std::vector<int> GetDealFinalStageIndexes(StaticFunctionTag*, RE::TESQuest* q) {
        return DealManager::GetSingleton().GetDealFinalStageIndexes(q);
    }

    bool IsDealValid(StaticFunctionTag*, RE::TESQuest* q) {
        return DealManager::GetSingleton().IsDealValid(q);
    }

    void ShowBuyoutMenuInternal(StaticFunctionTag*) { 
        DealManager::GetSingleton().ShowBuyoutMenu();
    }

    bool IsBuyoutSelected(StaticFunctionTag*) { 
        return DealManager::GetSingleton().IsBuyoutSelected();
    }

    RE::TESQuest* GetBuyoutMenuResult(StaticFunctionTag*) { return DealManager::GetSingleton().GetBuyoutMenuResult(); }

    RE::TESObjectARMO* GetRandomDeviceByKeyword(StaticFunctionTag*, RE::Actor* actor, RE::BGSKeyword* kwd) {
        return DeviceManager::GetSingleton().GetRandomDeviceByKeyword(actor, kwd);
    }

    RE::TESObjectARMO* GetWornItemByKeyword(StaticFunctionTag*, RE::Actor* actor, RE::BGSKeyword* kwd) {
        return DeviceManager::GetSingleton().GetWornItemByKeyword(actor, kwd);
    }
}

bool DFF::RegisterDealManager(IVirtualMachine* vm) {
    // Deal functions
    vm->RegisterFunction("SelectDeal", PapyrusClass, SelectDeal);
    vm->RegisterFunction("ActivateDeal", PapyrusClass, ActivateDeal);
    vm->RegisterFunction("ActivateDealByQuest", PapyrusClass, ActivateDealByQuest);
    vm->RegisterFunction("RemoveDeal", PapyrusClass, RemoveDeal);
    vm->RegisterFunction("GetDealQuest", PapyrusClass, GetDealQuest);
    vm->RegisterFunction("GetStage", PapyrusClass, GetStage);
    vm->RegisterFunction("IsDealActive", PapyrusClass, IsDealActive);
    vm->RegisterFunction("LoadActiveModularRules", PapyrusClass, LoadActiveModularRules);
    vm->RegisterFunction("ActivateRule", PapyrusClass, ActivateRule);
    vm->RegisterFunction("LoadRuleMaxStage", PapyrusClass, LoadRuleMaxStage);

    vm->RegisterFunction("GetActiveDeals", PapyrusClass, GetActiveDeals);
    vm->RegisterFunction("GetDealName", PapyrusClass, GetDealName);
    vm->RegisterFunction("GetDealRules", PapyrusClass, GetDealRules);

    vm->RegisterFunction("ToggleRule", PapyrusClass, ToggleRule);
    vm->RegisterFunction("ToggleStageVariation", PapyrusClass, ToggleStageVariation);
    vm->RegisterFunction("GetDealNextQuestStage", PapyrusClass, GetDealNextQuestStage);

    vm->RegisterFunction("GetDealCostGlobal", PapyrusClass, GetDealCostGlobal);
    vm->RegisterFunction("GetDealTimerGlobal", PapyrusClass, GetDealTimerGlobal);

    vm->RegisterFunction("GetStageIndex", PapyrusClass, GetStageIndex);
    vm->RegisterFunction("GetInventoryNamedObjects", PapyrusClass, GetInventoryNamedObjects);

    vm->RegisterFunction("GetGroupNames", PapyrusClass, GetGroupNames);
    vm->RegisterFunction("IsDealValid", PapyrusClass, IsDealValid);


    vm->RegisterFunction("SelectRandomActiveDeal", PapyrusClass, SelectRandomActiveDeal);
    
    vm->RegisterFunction("GetDealNumStages", PapyrusClass, GetDealNumStages);
    vm->RegisterFunction("GetDealFinalStages", PapyrusClass, GetDealFinalStages);
    
    vm->RegisterFunction("GetDealFinalStageIndexes", PapyrusClass, GetDealFinalStageIndexes);
    vm->RegisterFunction("GetDealFinalStageIndexes", PapyrusClass, GetDealFinalStageIndexes);
    
    vm->RegisterFunction("IsBuyoutSelected", PapyrusClass, IsBuyoutSelected);
    vm->RegisterFunction("ShowBuyoutMenuInternal", PapyrusClass, ShowBuyoutMenuInternal);
    vm->RegisterFunction("GetBuyoutMenuResult", PapyrusClass, GetBuyoutMenuResult);
    

    // Device functions
    vm->RegisterFunction("GetRandomDeviceByKeyword", PapyrusDeviceClass, GetRandomDeviceByKeyword);
    vm->RegisterFunction("GetWornItemByKeyword", PapyrusDeviceClass, GetWornItemByKeyword);

    return true;
}