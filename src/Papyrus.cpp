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

    void ActivateDeal(StaticFunctionTag*, int32_t id) { DealManager::GetSingleton().ActivateRule(id); }
    
    
    void RemoveDeal(StaticFunctionTag*, std::string name) { DealManager::GetSingleton().RemoveDeal(name); }

    void ActivateRule(StaticFunctionTag*, int id) {
        DealManager::GetSingleton().ActivateRule(id);
    }

    std::vector<std::string> GetDealRules(StaticFunctionTag*, std::string name) {
        return DealManager::GetSingleton().GetDealRules(name);
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

    void ShowBuyoutMenuInternal(StaticFunctionTag*) { 
        DealManager::GetSingleton().ShowBuyoutMenu();
    }

    bool IsBuyoutSelected(StaticFunctionTag*) { 
        return DealManager::GetSingleton().IsBuyoutSelected();
    }

    int GetBuyoutMenuResult(StaticFunctionTag*) { return DealManager::GetSingleton().GetBuyoutMenuResult(); }

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
    vm->RegisterFunction("RemoveDeal", PapyrusClass, RemoveDeal);
    vm->RegisterFunction("ActivateRule", PapyrusClass, ActivateRule);
    vm->RegisterFunction("GetDealRules", PapyrusClass, GetDealRules);

    vm->RegisterFunction("GetInventoryNamedObjects", PapyrusClass, GetInventoryNamedObjects);

    vm->RegisterFunction("GetGroupNames", PapyrusClass, GetGroupNames);;
    
    vm->RegisterFunction("IsBuyoutSelected", PapyrusClass, IsBuyoutSelected);
    vm->RegisterFunction("ShowBuyoutMenuInternal", PapyrusClass, ShowBuyoutMenuInternal);
    vm->RegisterFunction("GetBuyoutMenuResult", PapyrusClass, GetBuyoutMenuResult);
    
    vm->RegisterFunction("GetRandomDeviceByKeyword", PapyrusDeviceClass, GetRandomDeviceByKeyword);
    vm->RegisterFunction("GetWornItemByKeyword", PapyrusDeviceClass, GetWornItemByKeyword);

    return true;
}