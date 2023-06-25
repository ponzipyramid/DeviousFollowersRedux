#include <DFF/Deal.h>
#include "RE/T/TESDataHandler.h"
#include <SKSE/SKSE.h>
#include <functional>
#include <algorithm>

using namespace DFF;

bool Deal::InitQuest() {
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();
    RE::FormID formId = handler->LookupFormID(this->formId, this->espName);
    this->quest = RE::TESForm::LookupByID<RE::TESQuest>(formId);
    if (!this->quest) return false;

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
            if (curr->type == "wear" && other->type == "wear" && !(curr->negate && other->negate)) {
                for (int slot : curr->slots) {
                    if (other->slots.contains(slot)) {
                        std::string slotString1;
                        std::string slotString2;

                        for (auto slot : curr->slots) {
                            slotString1 += " " + std::to_string(slot);
                        }
                        for (auto slot : other->slots) {
                            slotString2 += " " + std::to_string(slot);
                        }
                        conflict = true;
                        break;
                    }
                }
            } else if (curr->type == "bathe" && other->type == "bathe" && other->negate != curr->negate) {
                conflict = true;
            }
            if (!conflict) {
                return true;
            }
        }
    }

    return false;
}

bool Rule::ConflictsWith(Rule* other) {
    return !RulesCompatible(this, other); 
}

void Deal::NextStage() { 
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

bool Deal::HasNextStage() { return stageIndex < std::max(maxStage, (int) stages.size()) - 1; }

void Deal::SetMaxStage(int max) { 
    SKSE::log::info("Setting max stage for {} to {}", fullName, max);
    maxStage = max; 
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

void Stage::RandomizeAltIndex() {
    // run thru deals and if disabled don't add them to list
    std::vector<int> indices;
    if (enabled) indices.push_back(-1);

    for (int i = 0; i < alt.size(); i++) 
        if (alt[i].enabled) indices.push_back(i);
    

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, indices.size() - 1);
    altIndex = indices[distr(gen)];
    SKSE::log::info("Selected alt index {}", altIndex);
}

int Deal::GetNextStage() {
    stages[stageIndex + 1].RandomizeAltIndex();
    return stages[stageIndex + 1].GetQuestStageIndex(); 
}

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

int Stage::GetQuestStageIndex() { return altIndex < 0 ? index : alt[altIndex].GetQuestStageIndex(); }

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