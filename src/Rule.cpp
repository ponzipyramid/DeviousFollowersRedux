#include <DFF/Rule.h>

using namespace DFF;

void Rule::Init() {
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

    statusGlobal = handler->LookupForm<RE::TESGlobal>(formId, modName);

    for (std::string req : requirements) {
        if (handler->LookupModByName(req) == nullptr) {
            SKSE::log::info("{} is missing mod {}", name, req);
            reqsMet = false;
            break;
        }
    }

    if (!reqsMet) statusGlobal->value = 0;
}

bool Rule::RulesCompatible(Rule* r1, Rule* r2) {
    if (r1->type == "naked" && r2->type == "naked") return false;
    if (r1->type == "bathe" && r2->type == "bathe") return false;
    if (r1->type == "wear" && r2->type == "naked") return false;

    if (r1->type == "wear" && r2->type == "wear") {
        for (int slot : r1->slots) {
            if (r2->slots.contains(slot)) return false;
        }
    }

    return true;
}

bool Rule::ConflictsWith(Rule* other) { return !(RulesCompatible(this, other) && RulesCompatible(other, this)); }

Rule::Rule(std::string path) {
    this->path = path;
    std::transform(this->path.begin(), this->path.end(), this->path.begin(), ::tolower);
}
