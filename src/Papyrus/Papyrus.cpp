#include "Papyrus.h"

#include <Deals/DealManager.h>
#include <Device/DeviceManager.h>
#include <Relationship/RelManager.h>

using namespace DFF;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "DealManager";
    constexpr std::string_view PapyrusDeviceClass = "DeviceManager";

    // TODO: rename to SelectRule
    std::string SelectDeal(StaticFunctionTag*, std::string lastRejected) {
        return DealManager::GetSingleton().SelectRule(lastRejected);
    }
    
    void RemoveDeal(StaticFunctionTag*, std::string name) { DealManager::GetSingleton().RemoveDeal(name); }
    
    void SetRuleValid(StaticFunctionTag*, std::string name, bool valid) { DealManager::GetSingleton().SetRuleValid(name, valid); }
    
    void ResetAllDeals(StaticFunctionTag*) {
        DealManager::GetSingleton().ResetAllDeals();
    }

    void Pause(StaticFunctionTag*) { DealManager::GetSingleton().Pause(); }
    void Resume(StaticFunctionTag*) { DealManager::GetSingleton().Resume(); }

    int ActivateRule(StaticFunctionTag*, std::string a_path, std::string a_dealName, bool a_create) { 
        return DealManager::GetSingleton().ActivateRule(a_path, a_dealName, a_create);
    }

    RE::TESGlobal* GetRuleGlobal(StaticFunctionTag*, std::string path) { 
        return DealManager::GetSingleton().GetRuleGlobal(path);
    }

    std::string GetRuleName(StaticFunctionTag*, std::string path) {
        return DealManager::GetSingleton().GetRuleName(path);
    }

    std::string GetRuleHint(StaticFunctionTag*, std::string path) {
        return DealManager::GetSingleton().GetRuleHint(path);
    }

    std::string GetRuleInfo(StaticFunctionTag*, std::string path) {
        return DealManager::GetSingleton().GetRuleInfo(path);
    }

    std::string GetRulePack(StaticFunctionTag*, std::string path) {
        return DealManager::GetSingleton().GetRulePack(path);
    }

    bool CanEnableRule(StaticFunctionTag*, std::string path) { 
        return DealManager::GetSingleton().CanEnableRule(path);
    }

    bool CanDisableRule(StaticFunctionTag*, std::string path) {
        return DealManager::GetSingleton().CanDisableRule(path);
    }

    std::vector<std::string> GetEnslavementRules(StaticFunctionTag*) {
        return DealManager::GetSingleton().GetEnslavementRules();
    }

    std::string GetRandomDeal(StaticFunctionTag*) { return DealManager::GetSingleton().GetRandomDeal(); }

    std::vector<std::string> GetDeals(StaticFunctionTag*) { 
        return DealManager::GetSingleton().GetDeals();
    }

    std::vector<std::string> GetDealRules(StaticFunctionTag*, std::string name) {
        return DealManager::GetSingleton().GetDealRules(name);
    }

    int GetDealCost(StaticFunctionTag*, std::string name) { 
        return DealManager::GetSingleton().GetDealCost(name);
    }

    void ExtendDeal(StaticFunctionTag*, std::string name, double by = 0.0f) {
        DealManager::GetSingleton().ExtendDeal(name, by);
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

    std::vector<std::string> GetPackNames(StaticFunctionTag*) {
        return DealManager::GetSingleton().GetPackNames();
    }

    std::vector<std::string> GetPackRules(StaticFunctionTag*, std::string packName) {
        return DealManager::GetSingleton().GetPackRules(packName);
    }

    RE::TESQuest* GetPackQuest(StaticFunctionTag*, std::string packName) {
        return DealManager::GetSingleton().GetPackQuest(packName);
    }

    void ShowBuyoutMenu(StaticFunctionTag*) { 
        DealManager::GetSingleton().ShowBuyoutMenu();
    }

    bool IsBuyoutSelected(StaticFunctionTag*) { 
        return DealManager::GetSingleton().IsBuyoutSelected();
    }

    std::string GetBuyoutMenuResult(StaticFunctionTag*) { return DealManager::GetSingleton().GetBuyoutMenuResult(); }

    RE::TESObjectARMO* GetRandomDeviceByKeyword(StaticFunctionTag*, RE::Actor* actor, RE::BGSKeyword* kwd) {
        return DeviceManager::GetSingleton().GetRandomDeviceByKeyword(actor, kwd);
    }

    RE::TESObjectARMO* GetWornItemByKeyword(StaticFunctionTag*, RE::Actor* actor, RE::BGSKeyword* kwd) {
        return DeviceManager::GetSingleton().GetWornItemByKeyword(actor, kwd);
    }

    void ActivateRelationshipStage(StaticFunctionTag*, int stage) {
        RelManager::GetSingleton().ActivateRelationshipStage(stage);
    }

    std::string GetRelationshipStageDeal(StaticFunctionTag*) {
        return RelManager::GetSingleton().GetRelationshipStageDeal();
    }

    std::vector<std::string> GetRelationshipStageRules(StaticFunctionTag*) {
        return RelManager::GetSingleton().GetRelationshipStageRules();
    }

    bool GetRelStageSettingBool(StaticFunctionTag*, std::string setting) {
        return RelManager::GetSingleton().GetRelStageSetting<bool>(setting);
    }

    float GetRelStageSettingFloat(StaticFunctionTag*, std::string setting) {
        return RelManager::GetSingleton().GetRelStageSetting<float>(setting);
    }

    std::string GetRelStageSettingString(StaticFunctionTag*, std::string setting) {
        return RelManager::GetSingleton().GetRelStageSetting<std::string>(setting);
    }

    void SetDealLockIn(StaticFunctionTag*, std::string a_dealName, bool a_lockIn) {
        return DealManager::GetSingleton().SetDealLockIn(a_dealName, a_lockIn);
    }
}

bool DFF::RegisterDealManager(IVirtualMachine* vm) {
    // Deal functions
    vm->RegisterFunction("SelectDeal", PapyrusClass, SelectDeal);
    vm->RegisterFunction("ActivateRule", PapyrusClass, ActivateRule);
    vm->RegisterFunction("RemoveDeal", PapyrusClass, RemoveDeal);
    vm->RegisterFunction("ResetAllDeals", PapyrusClass, ResetAllDeals);
    vm->RegisterFunction("Pause", PapyrusClass, Pause);
    vm->RegisterFunction("Resume", PapyrusClass, Resume);
    vm->RegisterFunction("GetDeals", PapyrusClass, GetDeals);
    vm->RegisterFunction("GetRandomDeal", PapyrusClass, GetRandomDeal);
    vm->RegisterFunction("GetDealRules", PapyrusClass, GetDealRules);
    vm->RegisterFunction("GetRuleGlobal", PapyrusClass, GetRuleGlobal);
    vm->RegisterFunction("GetRuleName", PapyrusClass, GetRuleName);
    vm->RegisterFunction("GetRuleHint", PapyrusClass, GetRuleHint);
    vm->RegisterFunction("GetRuleInfo", PapyrusClass, GetRuleInfo);
    vm->RegisterFunction("GetRulePack", PapyrusClass, GetRulePack);
    vm->RegisterFunction("CanEnableRule", PapyrusClass, CanEnableRule);
    vm->RegisterFunction("CanDisableRule", PapyrusClass, CanDisableRule);
    vm->RegisterFunction("GetEnslavementRules", PapyrusClass, GetEnslavementRules);
    vm->RegisterFunction("GetDealCost", PapyrusClass, GetDealCost);
    vm->RegisterFunction("ExtendDeal", PapyrusClass, ExtendDeal);
    vm->RegisterFunction("GetInventoryNamedObjects", PapyrusClass, GetInventoryNamedObjects);
    vm->RegisterFunction("GetPackNames", PapyrusClass, GetPackNames);
    vm->RegisterFunction("GetPackRules", PapyrusClass, GetPackRules);
    vm->RegisterFunction("GetPackQuest", PapyrusClass, GetPackQuest);
    vm->RegisterFunction("IsBuyoutSelected", PapyrusClass, IsBuyoutSelected);
    vm->RegisterFunction("ShowBuyoutMenuInternal", PapyrusClass, ShowBuyoutMenu);
    vm->RegisterFunction("GetBuyoutMenuResult", PapyrusClass, GetBuyoutMenuResult);
    vm->RegisterFunction("SetRuleValid", PapyrusClass, SetRuleValid);
    vm->RegisterFunction("SetDealLockIn", PapyrusClass, SetDealLockIn);
    
    // Device functions
    vm->RegisterFunction("GetRandomDeviceByKeyword", PapyrusDeviceClass, GetRandomDeviceByKeyword);
    vm->RegisterFunction("GetWornItemByKeyword", PapyrusDeviceClass, GetWornItemByKeyword);

    // Relationship functions
    vm->RegisterFunction("ActivateRelationshipStage", PapyrusClass, ActivateRelationshipStage);
    vm->RegisterFunction("GetRelationshipStageRules", PapyrusClass, GetRelationshipStageRules);
    vm->RegisterFunction("GetRelationshipStageDeal", PapyrusClass, GetRelationshipStageDeal);
    vm->RegisterFunction("GetRelStageSettingBool", PapyrusClass, GetRelStageSettingBool);
    vm->RegisterFunction("GetRelStageSettingFloat", PapyrusClass, GetRelStageSettingFloat);
    vm->RegisterFunction("GetRelStageSettingString", PapyrusClass, GetRelStageSettingString);


    return true;
}