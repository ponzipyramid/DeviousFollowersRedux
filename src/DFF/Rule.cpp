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


Rule::Rule(Pack* pack, std::string name, YAML::Node a_node) {
    if (pack) {
        this->name = name;
        this->pack = pack;
        this->id = pack->GetName() + '/' + name;
        std::transform(this->id.begin(), this->id.end(), this->id.begin(), ::tolower);
    }

    this->formId = a_node["formId"].as<int>();
    this->type = a_node["type"].as<std::string>();

    if (a_node["slots"].IsSequence()) {
        auto slotList = a_node["slots"].as<std::vector<int>>();;
        for (auto slot : slotList)
            this->slots.insert(slot);
    }

    this->level = a_node["level"].as<int>();
    this->hint = a_node["hint"].IsDefined() ? a_node["hint"].as<std::string>() : "";
    this->info = a_node["info"].IsDefined() ? a_node["info"].as<std::string>() : "";
    this->preventDisable = a_node["preventDisable"].IsDefined() ? a_node["preventDisable"].as<bool>() : false;
    this->requirements = a_node["requirements"].IsDefined() ? a_node["requirements"].as<std::vector<std::string>>() : std::vector<std::string>();
    
    if (a_node["exclude"].IsSequence()) {
        auto excludeList = a_node["exclude"].as<std::vector<std::string>>();;
        for (auto ruleId : excludeList)
            this->exclude.insert(ruleId);

    }

    if (a_node["subRules"].IsSequence()) {
        auto subRuleListNode = a_node["subRules"];

        for (std::size_t i = 0; i < subRuleListNode.size(); i++) {
            auto ruleNode = subRuleListNode[i];
            Rule subRule;

            subRule.type = ruleNode["type"].as<std::string>();

            if (ruleNode["slots"].IsSequence()) {
                auto slotList = ruleNode["slots"].as<std::vector<int>>();;
                for (auto slot : slotList)
                    subRule.slots.insert(slot);
            }

            subRules.push_back(subRule);
        }
    }
}
