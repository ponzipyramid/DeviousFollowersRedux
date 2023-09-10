#pragma once

#include <articuno/articuno.h>
#include <articuno/types/auto.h>

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>


namespace DFF {
    class Conflictor {

    };

    class Rule : public Conflictor {
    public:
        [[nodiscard]] inline Rule() {}
        Rule(std::string group, std::string name);
        [[nodiscard]] inline const std::string GetType() { return type; } 
        [[nodiscard]] inline const std::string GetFullName() { return fullName; }
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const int GetBuiltInId() { return builtInId; }
        [[nodiscard]] inline bool IsBuiltIn() { return builtInId >= 0; }
        [[nodiscard]] inline const bool IsRuleForLevel(int level) { return !!levels.count(level); }
        bool IsEnabled();
        inline void SetEnabled(bool enabled) { this->enabled = enabled; }

        bool ConflictsWith(Rule* other);
        static bool RulesCompatible(Rule* r1, Rule* r2);
        void Init();
    private:
        articuno_deserialize(ar) {
            std::vector<std::string> _slots;
            std::vector<std::string> _levels;
            std::string _negate;
            std::string _disabled;

            std::string _builtInId;

            ar <=> articuno::kv(type, "type");
            ar <=> articuno::kv(_slots, "slots");
            ar <=> articuno::kv(_negate, "negate");
            ar <=> articuno::kv(_builtInId, "builtInId");
            ar <=> articuno::kv(_levels, "levels");

            ar <=> articuno::kv(description, "description");
            ar <=> articuno::kv(requirements, "requirements");
            ar <=> articuno::kv(_disabled, "disabled");
            
            ar <=> articuno::kv(statusProperty, "statusProperty");


            ar <=> articuno::kv(alt, "alt");

            if (!_builtInId.empty()) {
                builtInId = std::stoi(_builtInId);
            }
            negate = _negate == "true";
            enabled = _disabled != "true";


            for (std::string slot : _slots) slots.insert(std::stoi(slot));
            for (std::string level : _levels) levels.insert(std::stoi(level));

            if (statusProperty.empty() && !name.empty()) {
                statusProperty = std::string(name);
                statusProperty.erase(std::remove_if(statusProperty.begin(), statusProperty.end(), ::isspace),
                                     statusProperty.end());
                statusProperty += "Rule";
            }

            if (!name.empty()) {
                SKSE::log::info("Stauts Property for {} is {}", name, statusProperty);
            }

        }

        std::string type;
        std::string name;
        std::string fullName;
        std::string description;

        int builtInId;

        bool negate;
        bool enabled;

        std::string statusProperty;

        std::unordered_set<int> slots;
        std::unordered_set<int> levels;
        std::vector<Rule> alt;

        std::vector<std::string> requirements;

        friend class articuno::access;
    };

    class Stage {
    public:
        [[nodiscard]] inline std::vector<Rule> GetRules() { return rules; }
        [[nodiscard]] inline std::string GetName() { return name; }
        [[nodiscard]] inline int GetIndex() { return index; }
        [[nodiscard]] inline bool IsEnabled() { return enabled; }
        [[nodiscard]] inline std::vector<Stage>& GetAltStages() { return alt; }
        inline void SetEnabled(bool enabled) { this->enabled = enabled; }

        void RandomizeAltIndex(std::vector<bool> enabled);
        void Reset();
        int GetQuestStageIndex();
    private:
        articuno_deserialize(ar) {
            std::string _index;
            ar <=> articuno::kv(_index, "index");
            ar <=> articuno::kv(rules, "rules");
            ar <=> articuno::kv(alt, "alt");
            ar <=> articuno::kv(name, "name");

            index = stoi(_index);

            if (rules.size() && name.empty()) {
                throw std::runtime_error(std::string("classic deal stages must have names"));
            }
        }

    protected:
        std::string name;
        bool enabled = true;

        int index;
        int altIndex = -1; // used to select stage variations by indexing regular stage = -1, alt stage 1 = 0, alt stage 2, ...

        std::vector<Rule> rules;
        std::vector<Stage> alt;

        friend class Deal;
        friend class articuno::access;
    };

    class Deal : public Conflictor {
    public:
        [[nodiscard]] inline Deal() { }
        Deal(std::string group, std::string name);
        [[nodiscard]] inline bool IsBuiltIn() { return builtInId >= 0; }
        [[nodiscard]] inline int GetBuiltInId() { return builtInId; }
        [[nodiscard]] inline bool IsActive() { return stageIndex > -1; }
        [[nodiscard]] inline RE::TESQuest* GetQuest() { return quest; }
        [[nodiscard]] inline std::string GetFullName() { return fullName; }
        [[nodiscard]] inline std::string GetName() { return name; }
        [[nodiscard]] inline bool IsModular() { return modular; }
        int GetMaxStage();
        [[nodiscard]] inline int GetNumStages() { return stages.size(); }
        [[nodiscard]] inline bool IsMaxStageDifferent() { return GetMaxStage() != stages.size(); }
        [[nodiscard]] inline std::vector<Stage>& GetStages() { return stages; }

        [[nodiscard]] inline RE::TESGlobal* GetCostGlobal() const { return costGlobal; }
        [[nodiscard]] inline RE::TESGlobal* GetTimerGlobal() const { return timerGlobal; }

        bool ConflictsWith(Deal* other);
        bool ConflictsWith(Rule* other);
        void SetMaxStage(int maxStage);

        bool InitQuest();
        bool InitQuestData();
        void NextStage();
        void Reset();
        bool HasNextStage();

        void ToggleStageVariation(int stageIndex, int varIndex, bool enabled);

        std::vector<Stage*> GetActiveStages();
        std::vector<Rule*> GetActiveRules();

        [[nodiscard]] inline int GetStage() {
            return stageIndex < 0 ? stageIndex : stages[stageIndex].GetQuestStageIndex();
        }
        [[nodiscard]] inline int GetStageIndex() { return stageIndex; }
        int GetNextStage();
    private:
        articuno_deserialize(ar) { 
            std::string _formId;
            std::string _builtInId;
            std::string _modular;

            ar <=> articuno::kv(espName, "espName");
            ar <=> articuno::kv(stages, "stages");
            ar <=> articuno::kv(_formId, "formId");
            ar <=> articuno::kv(_builtInId, "builtInId");
            ar <=> articuno::kv(description, "description");
            ar <=> articuno::kv(_modular, "modular");
            ar <=> articuno::kv(excludeDeals, "excludeDeals");
            ar <=> articuno::kv(excludeRules, "excludeRules");

            ar <=> articuno::kv(costFormID, "costFormID");
            ar <=> articuno::kv(timerFormID, "timerFormID");

            formId = std::stoul(_formId, nullptr, 16);
            modular = _modular == "true";

            if (!_builtInId.empty()) {
                builtInId = std::stoi(_builtInId);
            }

            for (int i = 0; i < stages.size(); i++) {
                for (int j = 0; j < stages[i].rules.size(); j++) {
                    rules.push_back(stages[i].rules[j]);

                    for (int k = 0; k < stages[i].alt.size(); k++) {
                        for (int l = 0; l < stages[i].alt[k].rules.size(); l++) {
                            rules.push_back(stages[i].alt[k].rules[l]);
                        }
                    }
                }
            }
        }

        template<typename T>
        T GetProperty(std::string name);

        std::string fullName;
        std::string name;

        std::string espName;
        std::string description;
        bool enabled = true;

        int formId;
        int builtInId = -1;
        int stageIndex = -1;

        std::vector<Stage> stages;
        std::vector<Rule> rules;

        std::vector<std::string> requirements;
        std::unordered_set<std::string> excludeRules;
        std::unordered_set<std::string> excludeDeals;

        RE::TESQuest* quest;

        RE::FormID costFormID;
        RE::FormID timerFormID;

        RE::TESGlobal* costGlobal;
        RE::TESGlobal* timerGlobal;

        bool modular;

        friend class articuno::access;
        friend class Rule;
    };
}

