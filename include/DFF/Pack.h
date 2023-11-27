#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace DFF {
    class Rule; 

    class Pack {
    public:
        [[nodiscard]] Pack() = default;
        Pack(std::string name) { this->name = name; }
        
        [[nodiscard]] inline RE::TESQuest* GetQuest() { return quest; }
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const std::string GetModName() { return modName; }
        [[nodiscard]] inline const std::vector<Rule*> GetRules() { return rules; }
        [[nodiscard]] inline const void AddRule(Rule* rule) { rules.push_back(rule); }

        bool Init(RE::TESDataHandler* handler);
    private:
        std::string name;
        RE::FormID formId;
        std::string modName;
        std::string author;
        std::vector<Rule*> rules;

        RE::TESQuest* quest;
    };
};  // namespace DFF