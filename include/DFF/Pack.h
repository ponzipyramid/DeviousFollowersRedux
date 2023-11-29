#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace DFF {
    class Rule; 

    class Pack {
    public:
        [[nodiscard]] Pack() = default;
        Pack(std::string a_name, const YAML::Node& a_packFile) { 
            this->name = a_name;
            this->formId = a_packFile["formId"].as<int>();
            this->modName = a_packFile["modName"].as<std::string>();
            this->author = a_packFile["author"].as<std::string>();
        }

        [[nodiscard]] inline RE::TESQuest* GetQuest() { return quest; }
        [[nodiscard]] inline void SetName(std::string name) { this->name = name; }
        
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const std::string GetAuthor() { return author; }
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

        friend struct YAML::convert<Pack>;
    };
};  // namespace DFF