#pragma once

#include <DFF/Rule.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class RulePath {
    public:
        inline RulePath(std::string name) { this->name = name; }

        inline std::string GetName() { return name; }
        inline std::vector<std::string> GetRuleIds() { return ruleIds; }

    private:
        articuno_deserialize(ar) { ar <=> articuno::kv(ruleIds, "rules"); }

        std::vector<std::string> ruleIds;
        std::string name;

        friend class articuno::access;
    };

}  // namespace DFF