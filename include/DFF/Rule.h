#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class Rule {
    public:
        [[nodiscard]] Rule() = default;
        Rule(std::string group, std::string name);
        [[nodiscard]] inline const std::string GetType() { return type; }
        [[nodiscard]] inline const std::string GetFullName() { return fullName; }
        [[nodiscard]] inline const std::string GetName() { return name; }
        [[nodiscard]] inline const bool CheckSeverity(int level) { return level <= severity; }
        [[nodiscard]] inline const bool MetRequirements() { return reqsMet; }

        bool ConflictsWith(Rule* other);
        void Init();
    private:
        articuno_deserialize(ar) {
            ar <=> articuno::kv(questFormId, "questFormId");
            ar <=> articuno::kv(questEspName, "questEspName");

            ar <=> articuno::kv(type, "type");
            ar <=> articuno::kv(slots, "slots");
            ar <=> articuno::kv(negate, "negate");
            ar <=> articuno::kv(severity, "severity");

            ar <=> articuno::kv(description, "description");
            ar <=> articuno::kv(requirements, "requirements");
        }

        int questFormId;
        std::string questEspName;

        std::string name;
        std::string fullName;
        std::string description;

        std::string type;
        bool negate;
        std::unordered_set<int> slots;
        int severity;

        bool reqsMet = true;
        std::vector<std::string> requirements;

        friend class articuno::access;
    };
}  // namespace DFF
