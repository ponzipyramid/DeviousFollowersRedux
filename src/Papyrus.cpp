#include "Papyrus.h"

#include <DFF/HitCounterManager.h>
#include <DFF/DealManager.h>

using namespace DFF;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "DealManager";

    int32_t* PopulateHitData(Actor* target, char* unk0);

    Relocation<decltype(PopulateHitData)>& GetHookedFunction() noexcept {
        static Relocation<decltype(PopulateHitData)> value(RELOCATION_ID(42832, 44001), 0x42);
        return value;
    }

    Relocation<decltype(PopulateHitData)> OriginalPopulateHitData;

    bool StartCounting(StaticFunctionTag*, Actor* actor) {
        return HitCounterManager::GetSingleton().Track(actor);
    }

    bool StopCounting(StaticFunctionTag*, Actor* actor) {
        return HitCounterManager::GetSingleton().Untrack(actor);
    }

    int32_t GetTotalHitCounters(StaticFunctionTag*) {
        return 0;
    }

    void Increment(StaticFunctionTag*, Actor* actor, int32_t by) {
        if (actor) {
            HitCounterManager::GetSingleton().Increment(actor, by);
        }
    }

    int32_t GetCount(StaticFunctionTag*, Actor* actor) {
        if (!actor) {
            return 0;
        }
        return HitCounterManager::GetSingleton().GetHitCount(actor).value_or(0);
    }

    int32_t* PopulateHitData(Actor* target, char* unk0) {
        HitCounterManager::GetSingleton().RegisterHit(target);
        return OriginalPopulateHitData(target, unk0);
    }

    int32_t SelectDeal(StaticFunctionTag*, int track, float_t prefersModular) {
        return DealManager::GetSingleton().SelectDeal(track, prefersModular);
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

    void SetMaxModularDeals(StaticFunctionTag*, int maxDeals) {
        DealManager::GetSingleton().SetMaxModularDeals(maxDeals);
    }
}

/**
 * This is the function that acts as a registration callback for Papyrus functions. Within you can register functions
 * to bind to native code. The first argument of such bindings is the function name, the second is the class name, and
 * third is the function that will be invoked in C++ to handle it. The callback should return <code>true</code> on
 * success.
 */
bool DFF::RegisterHitCounter(IVirtualMachine* vm) {
    return true;
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

    vm->RegisterFunction("SetMaxModularDeals", PapyrusClass, SetMaxModularDeals);

    return true;
}

void DFF::InitializeHook(Trampoline& trampoline) {
    // The trampoline can be used to write a new call instruction at a given address (here the start of the function for
    // HitData::Populate). We use write_code<5> to indicate this is a 5-byte call instruction (rather than the much
    // rarer 6-byte call). We pass in the address of our function that will be called, and a pointer to the trampoline
    // function is returned.
    //
    // The trampoline pointed to contains any instructions from the original function we overwrote and a call to the
    // instruction that comes after, so that if we call that address as a function, we are in effect calling the
    // original code.
    OriginalPopulateHitData = trampoline.write_call<5>(GetHookedFunction().address(),
                                                       reinterpret_cast<uintptr_t>(PopulateHitData));
    log::debug("Hit data hook written.");
}