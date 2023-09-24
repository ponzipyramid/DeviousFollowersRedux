#include <DFF/Rule.h>

using namespace DFF;

void Rule::Init() {
    RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();
    for (std::string req : requirements) {
        if (handler->LookupModByName(req) == nullptr) {
            SKSE::log::info("{} is missing mod {}", name, req);
            reqsMet = false;
            break;
        }
    }
}

bool Rule::ConflictsWith(Rule* other) { return false; }

Rule::Rule(std::string group, std::string name) {
    this->name = name;
    this->fullName = group + '/' + name;
    std::transform(this->fullName.begin(), this->fullName.end(), this->fullName.begin(), ::tolower);
}
