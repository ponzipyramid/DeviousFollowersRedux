#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class Rule {
    public:
        [[nodiscard]] Rule() = default;
        Rule(std::string path);
        [[nodiscard]] inline const std::string GetType() { return type; }
        [[nodiscard]] inline const std::string GetPath() { return path; }
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const std::string GetDesc() { return description; }
        [[nodiscard]] inline const std::string GetHint() { return hint; }
        [[nodiscard]] inline const bool CheckSeverity(int maxLevel) { return maxLevel >= this->level; }
        [[nodiscard]] inline const bool CanEnable() { return reqsMet; }
        [[nodiscard]] inline const bool CanDisable() { return !preventDisable; }
        [[nodiscard]] inline const bool IsValid() { return statusGlobal != nullptr; }
        [[nodiscard]] inline const bool IsEnabled() { return statusGlobal->value > 0; }
        [[nodiscard]] inline RE::TESGlobal* GetGlobal() { return statusGlobal; }

        bool ConflictsWith(Rule* other);
        void Init();
    private:
        static bool RulesCompatible(Rule* r1, Rule* r2);

        articuno_deserialize(ar) {
            ar <=> articuno::kv(name, "name");

            ar <=> articuno::kv(formId, "formId");
            ar <=> articuno::kv(modName, "modName");

            ar <=> articuno::kv(type, "type");
            ar <=> articuno::kv(slots, "slots");
            ar <=> articuno::kv(negate, "negate");
            ar <=> articuno::kv(level, "level");

            ar <=> articuno::kv(description, "description");
            ar <=> articuno::kv(hint, "hint");
            
            ar <=> articuno::kv(preventDisable, "preventDisable");
            ar <=> articuno::kv(requirements, "requirements");
        }

        RE::FormID formId;
        std::string modName;

        std::string name;
        std::string path;
        std::string description;
        std::string hint;

        std::string type;
        bool negate;
        std::unordered_set<int> slots;
        int level;

        RE::TESGlobal* statusGlobal = nullptr;

        bool preventDisable = false;
        bool reqsMet = true;
        std::vector<std::string> requirements;

        friend class articuno::access;
    };
}  // namespace DFF
