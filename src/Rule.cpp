#include <DFF/Rule.h>

using namespace DFF;

void Rule::Enable() {
    if (CanEnable()) statusGlobal->value = 1;
}

void Rule::Disable() {
    if (CanDisable()) statusGlobal->value = 0;
}

void Rule::Activate() {
    if (IsEnabled()) statusGlobal->value = 3;
}

void Rule::Reset() {
    if (IsEnabled()) statusGlobal->value = 1;
}

bool Rule::Init(RE::TESDataHandler* handler) {
    if (pack->GetQuest() == nullptr) return false;

    std::string modName = pack->GetModName();

    statusGlobal = handler->LookupForm<RE::TESGlobal>(formId, modName);

    if (statusGlobal == nullptr) return false;

    for (std::string req : requirements) {
        if (handler->LookupModByName(req) == nullptr) {
            SKSE::log::info("{} is missing mod {}", name, req);
            reqsMet = false;
            break;
        }
    }

    if (!reqsMet && CanDisable()) Disable();

    return true;
}

bool Rule::RulesCompatible(Rule* r1, Rule* r2) {
    if (r1->type == "naked" && r2->type == "naked") return false;
    if (r1->type == "bathe" && r2->type == "bathe") return false;
    if (r1->type == "wear" && r1->slots.contains(32) && r2->type == "naked") return false;

    if (r1->type == "wear" && r2->type == "wear") {
        for (int slot : r1->slots) {
            if (r2->slots.contains(slot)) return false;
        }
    }

    for (auto& sr1 : r1->subRules) {
        for (auto& sr2 : r2->subRules) {
            if (sr1.name == sr2.name) {
                return false;
            }

        }
    }

    return true;
}

bool Rule::ConflictsWith(Rule* other) { 
    return exclude.contains(other->GetId()) || other->exclude.contains(GetId()) ||
           !(RulesCompatible(this, other) && RulesCompatible(other, this));
}

Rule::Rule(Pack* pack, std::string name) {
    this->name = name;
    this->pack = pack;
    this->id = pack->GetName() + '/' + name;
    std::transform(this->id.begin(), this->id.end(), this->id.begin(), ::tolower);
}
