#include "Papyrus.h"

#include <DFF/DealManager.h>

using namespace DFF;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "DealManager";

    int32_t SelectDeal(StaticFunctionTag*, int track, int maxModDeals, float_t prefersModular) {
        return DealManager::GetSingleton().SelectDeal(track, maxModDeals, prefersModular);
    }

    void ActivateDeal(StaticFunctionTag*, int32_t id) { DealManager::GetSingleton().ActivateDeal(id); }
    
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

    std::vector<RE::TESQuest*> GetActiveDeals(StaticFunctionTag*, bool classic) {
        return DealManager::GetSingleton().GetActiveDeals(classic);
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
}

bool DFF::RegisterDealManager(IVirtualMachine* vm) {
    vm->RegisterFunction("SelectDeal", PapyrusClass, SelectDeal);
    vm->RegisterFunction("ActivateDeal", PapyrusClass, ActivateDeal);
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

    return true;
}