#include <DFF/Deal.h>
#include "RE/T/TESDataHandler.h"
#include <SKSE/SKSE.h>
#include <functional>
#include <algorithm>
#include "Script.hpp"

using namespace DFF;

bool Deal::InitQuest() {
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();
    RE::FormID formId = handler->LookupFormID(this->formId, this->espName);
    this->quest = RE::TESForm::LookupByID<RE::TESQuest>(formId);
    if (!this->quest) return false;

    formId = handler->LookupFormID(this->costFormID, this->espName);
    this->costGlobal = RE::TESForm::LookupByID<RE::TESGlobal>(formId);

    formId = handler->LookupFormID(this->timerFormID, this->espName);
    this->timerGlobal = RE::TESForm::LookupByID<RE::TESGlobal>(formId);

    // TODO: validate self-conflicting rules within a single stage in each deal
    for (std::string requirement : requirements) {
        if (handler->LookupModByName(requirement) == nullptr) {
            SKSE::log::info("Missing mod {}", requirement);
            enabled = false;
            break;
        }
    }

    return true;
}

bool Deal::InitQuestData() {
    if (!this->quest) {
        return false;
    }

    if (this->quest->IsRunning()) {
        for (int i = 0; i < stages.size(); i++) {
            if (stages[i].index == this->quest->currentStage) {
                stageIndex = i;
                stages[i].altIndex = -1;
                break;
            }
            for (int j = 0; j < stages[i].alt.size(); j++) {
                if (stages[i].alt[j].index == this->quest->currentStage) {
                    stageIndex = i;
                    stages[i].altIndex = j;
                    break;
                }
            }
        }
    }

    return true;
}


void Rule::Init() { 
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton(); 
    for (std::string requirement : requirements) {
        if (handler->LookupModByName(requirement) == nullptr) {
            SKSE::log::info("{} is missing mod {}", name, requirement);
            enabled = false;
            break;
        }
    }
}
bool Deal::ConflictsWith(Deal* other) {
    if (excludeDeals.count(other->name)) {
        return true;
    }
    for (int i = 0; i < other->rules.size(); i++) {
        if (ConflictsWith(&other->rules[i])) {
            return true;
        }
    }

    return false;
}

bool Deal::ConflictsWith(Rule* other) {
    if (excludeRules.count(other->GetFullName())) {
        return true;
    }

    for (int i = 0; i < rules.size(); i++) {
        if (rules[i].ConflictsWith(other)) {
            return true;
        }
    }
    return false;
}

// checks whether at least one rule or its alts is compatible with another rule or its alts
bool Rule::RulesCompatible(Rule* r1, Rule* r2) {
    std::vector<Rule*> rules1;
    std::vector<Rule*> rules2;

    rules1.push_back(r1);
    for (auto& altRule : r1->alt) rules1.push_back(&altRule);

    rules2.push_back(r2);
    for (auto& altRule : r2->alt) rules2.push_back(&altRule);

    Rule* curr;
    Rule* other;
    for (int i = 0; i < rules1.size(); i++) {
        
        curr = rules1[i];
        for (int j = 0; j < rules2.size(); j++) {
            bool conflict = false;
            other = rules2[j];
            
            if (!InternalCheck(curr, other) || !InternalCheck(other, curr)) return false;
        }
    }
    return true;
}

bool Rule::InternalCheck(Rule* r1, Rule* r2) {
    if (r1->type == "naked" && r2->type == "naked") return false;

    if (r1->type == "wear" && r2->type == "wear") {
        for (int slot : r1->slots) {
            if (r2->slots.contains(slot)) return false;
        }
    }

    if (r1->type == "bathe" && r2->type == "no bathe") return false;

    return true;

}

bool Rule::IsEnabled() {
    try {
        auto quest = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(0x1BD274, "DeviousFollowers.esp");
        auto ptr = ScriptUtils::GetScriptObject(quest, "_DFlowModDealController");
        auto val = ScriptUtils::GetProperty<int>(ptr, statusProperty);

        return val != 0;
    } catch (...) {
        SKSE::log::info("Failed to retrieve rule status property {}", name);
        return false;
    }
}


bool Rule::ConflictsWith(Rule* other) {
    return !RulesCompatible(this, other); 
}

void Deal::NextStage() {
    int maxStage = GetMaxStage();
    stageIndex < std::max(maxStage, (int)stages.size()) - 1 ? ++stageIndex : stageIndex;
}

void Stage::Reset() {
    altIndex = -1;
}

void Deal::Reset() { 
    this->quest->Reset();
    for (Stage& stage : stages) stage.Reset();
    stageIndex = -1;
}

int Deal::GetMaxStage() {
    if (IsModular()) {
        return 3;
    } else {
        return GetProperty<int>("MaxStages");
    }
}

template <typename T>
T Deal::GetProperty(std::string name) {
    auto ptr = ScriptUtils::GetScriptObject(quest, "_ddeal");
    return ScriptUtils::GetProperty<T>(ptr, name);
}

bool Deal::HasNextStage() {
    int maxStage = GetMaxStage();
    bool hasNext = stageIndex < (std::min(maxStage, (int)stages.size()) - 1);
    
    SKSE::log::info("Deal {} has next stage = {}", name, hasNext);

    return hasNext;
}
Deal::Deal(std::string group, std::string name) { 
    this->name = name;
    this->fullName = group + '/' + name;
    std::transform(this->fullName.begin(), this->fullName.end(), this->fullName.begin(), ::tolower);
}

Rule::Rule(std::string group, std::string name) {
    this->name = name;
    this->fullName = group + '/' + name;
    std::transform(this->fullName.begin(), this->fullName.end(), this->fullName.begin(), ::tolower);
}

int Deal::GetNextStage() {

    if (!IsModular()) {
        try {
            SKSE::log::info("Randomizing alt stage for {}", name);
            auto enabledAlts = GetProperty<std::vector<bool>>("FinalStagesEnabled"); 
            stages[stageIndex + 1].RandomizeAltIndex(enabledAlts);
        } catch(...) {
            SKSE::log::info("Could not retrieve script object for {}", name);
        }
    }

    return stages[stageIndex + 1].GetQuestStageIndex(); 
}

void Stage::RandomizeAltIndex(std::vector<bool> enabled) {
    // run thru deals and if disabled don't add them to list

    if (alt.empty()) return;

    std::vector<int> indices;
    if (enabled[0]) {
        indices.push_back(-1);
    }

    for (int i = 0; i < alt.size(); i++) {
        if (enabled[i + 1]) indices.push_back(i);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, indices.size() - 1);
    altIndex = indices[distr(gen)];
    SKSE::log::info("Selected alt index {} out of {}", altIndex, indices.size());
}

int Stage::GetQuestStageIndex() { return altIndex < 0 ? index : alt[altIndex].GetQuestStageIndex(); }

std::vector<Rule*> Deal::GetActiveRules() {
    std::vector<Rule*> activeRules;
    for (int i = 0; i <= stageIndex; i++) {
        Stage& stage = stages[i];
        for (int j = 0; j < stage.rules.size(); j++) {
            activeRules.push_back(&stage.rules[j]);
        }
    }

    return activeRules;
}

std::vector<Stage*> Deal::GetActiveStages() {
    std::vector<Stage*> activeStages;
    for (int i = 0; i <= stageIndex; i++) {
        activeStages.push_back(&stages[i]);
    }

    return activeStages;
}


void Deal::ToggleStageVariation(int stageIndex, int varIndex, bool enabled) { 
    SKSE::log::info("Toggling stage {}", stages[stageIndex].GetName());

    varIndex--;
    if (varIndex == -1) {
        SKSE::log::info("Disabled regular stage");
        stages[stageIndex].enabled = enabled;
    } else if (varIndex < stages[stageIndex].alt.size()) {
        SKSE::log::info("Disabled alt stage with index {}", varIndex);
        stages[stageIndex].alt[varIndex].enabled = enabled;
    } else {
        SKSE::log::info("Failed to disable stage {} {} {} {}", stageIndex, varIndex, enabled,
                        stages[stageIndex].alt.size());
    }
}

void Deal::SetMaxStage(int max) {
    int maxStage = GetMaxStage();

    SKSE::log::info("Setting max stage for {} to {}", fullName, max);
    maxStage = max;
}
