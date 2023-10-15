#pragma once

#include <RE/Skyrim.h>
#include <DFF/Pack.h>
#include <SKSE/SKSE.h>
#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class Rule {
    public:
        [[nodiscard]] Rule() = default;
        Rule(Pack* pack, std::string name);
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const std::string GetType() { return type; }
        [[nodiscard]] inline const std::string GetId() { return id; }
        [[nodiscard]] inline const std::string GetHint() { return hint.empty() ? info : hint; }
        [[nodiscard]] inline const std::string GetInfo() { return info.empty() ? hint : info; }
        [[nodiscard]] inline Pack* GetPack() { return pack; }
        [[nodiscard]] inline const bool CheckSeverity(int maxLevel) { return maxLevel >= this->level; }
        [[nodiscard]] inline const bool CanEnable() { return reqsMet && valid && HasGlobal(); }
        [[nodiscard]] inline const bool CanDisable() { return !preventDisable && HasGlobal(); }
        [[nodiscard]] inline const bool HasGlobal() { return statusGlobal != nullptr; }
        [[nodiscard]] inline const bool IsEnabled() { return statusGlobal->value > 0; }
        [[nodiscard]] inline const bool IsSelected() { return statusGlobal->value == 2; }
        [[nodiscard]] inline void SetSelected() { statusGlobal->value = 2; }
        [[nodiscard]] inline void ResetIfSelected() {
            if (IsSelected()) statusGlobal->value = 1;
        }
        [[nodiscard]] inline const bool IsActive() { return statusGlobal->value == 3; }
        [[nodiscard]] inline RE::TESGlobal* GetGlobal() { return statusGlobal; }
        [[nodiscard]] inline int GetLevel() { return level; }

        void Enable();
        void Disable();
        void Activate();
        bool ConflictsWith(Rule* other);
        bool Init(RE::TESDataHandler* handler);

        bool valid = true;
        RE::BGSRefAlias* alias = nullptr;
    private:
        static bool RulesCompatible(Rule* r1, Rule* r2);

        articuno_deserialize(ar) {
            ar <=> articuno::kv(formId, "formId");

            ar <=> articuno::kv(type, "type");
            ar <=> articuno::kv(slots, "slots");
            ar <=> articuno::kv(negate, "negate");
            ar <=> articuno::kv(level, "level");

            ar <=> articuno::kv(hint, "hint");
            ar <=> articuno::kv(info, "info");
            
            ar <=> articuno::kv(preventDisable, "preventDisable");
            ar <=> articuno::kv(requirements, "requirements");
        }

        RE::FormID formId;

        std::string name;
        std::string id;
        std::string hint;
        std::string info;

        Pack* pack;

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
