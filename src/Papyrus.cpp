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

    int32_t SelectDeal(StaticFunctionTag*, int lastRejectedId) {
        return DealManager::GetSingleton().SelectDeal(lastRejectedId);
    }
    
    void RemoveDeal(StaticFunctionTag*, std::string name) { DealManager::GetSingleton().RemoveDeal(name); }
    
    void ResetAllDeals(StaticFunctionTag*) {
        DealManager::GetSingleton().ResetAllDeals();
    }

    void Pause(StaticFunctionTag*) { DealManager::GetSingleton().Pause(); }
    void Resume(StaticFunctionTag*) { DealManager::GetSingleton().Resume(); }

    int ActivateRule(StaticFunctionTag*, int id) {
        return DealManager::GetSingleton().ActivateRule(id);
    }

    int ActivateRuleByName(StaticFunctionTag*, std::string name) {
        return DealManager::GetSingleton().ActivateRule(name);
    }

    RE::TESGlobal* GetRuleGlobal(StaticFunctionTag*, int id) { 
        return DealManager::GetSingleton().GetRuleGlobal(id);
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

    double GetExpensiveDebtCount(StaticFunctionTag*) {
        return DealManager::GetSingleton().GetExpensiveDebtCount();
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

    std::vector<std::string> GetGroupNames(StaticFunctionTag*) {
        return DealManager::GetSingleton().GetGroupNames();
    }

    std::vector<std::string> GetGroupRules(StaticFunctionTag*, std::string groupName) {
        return DealManager::GetSingleton().GetGroupRules(groupName);
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
}

bool DFF::RegisterDealManager(IVirtualMachine* vm) {
    // Deal functions
    vm->RegisterFunction("SelectDeal", PapyrusClass, SelectDeal);
    vm->RegisterFunction("ActivateRule", PapyrusClass, ActivateRule);
    vm->RegisterFunction("ActivateRuleByName", PapyrusClass, ActivateRuleByName);
    vm->RegisterFunction("RemoveDeal", PapyrusClass, RemoveDeal);
    vm->RegisterFunction("ResetAllDeals", PapyrusClass, ResetAllDeals);
    
    vm->RegisterFunction("Pause", PapyrusClass, Pause);
    vm->RegisterFunction("Resume", PapyrusClass, Resume);

    vm->RegisterFunction("GetDeals", PapyrusClass, GetDeals);
    vm->RegisterFunction("GetRandomDeal", PapyrusClass, GetRandomDeal);
    vm->RegisterFunction("GetDealRules", PapyrusClass, GetDealRules);
    vm->RegisterFunction("GetRuleGlobal", PapyrusClass, GetRuleGlobal);
    vm->RegisterFunction("GetEnslavementRules", PapyrusClass, GetEnslavementRules);
    vm->RegisterFunction("GetDealCost", PapyrusClass, GetDealCost);
    vm->RegisterFunction("GetExpensiveDebtCount", PapyrusClass, GetExpensiveDebtCount);
    vm->RegisterFunction("ExtendDeal", PapyrusClass, ExtendDeal);

    vm->RegisterFunction("GetInventoryNamedObjects", PapyrusClass, GetInventoryNamedObjects);

    vm->RegisterFunction("GetGroupNames", PapyrusClass, GetGroupNames);
    vm->RegisterFunction("GetGroupRules", PapyrusClass, GetGroupRules);
    ;
    
    vm->RegisterFunction("IsBuyoutSelected", PapyrusClass, IsBuyoutSelected);
    vm->RegisterFunction("ShowBuyoutMenuInternal", PapyrusClass, ShowBuyoutMenu);
    vm->RegisterFunction("GetBuyoutMenuResult", PapyrusClass, GetBuyoutMenuResult);
    
    vm->RegisterFunction("GetRandomDeviceByKeyword", PapyrusDeviceClass, GetRandomDeviceByKeyword);
    vm->RegisterFunction("GetWornItemByKeyword", PapyrusDeviceClass, GetWornItemByKeyword);

    return true;
}